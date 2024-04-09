// dllmain.cpp : Definiert den Einstiegspunkt für die DLL-Anwendung.

#include <Windows.h>

#include "ImageData.h"
#include "KrtsBertram.h"
#include "ProgramLogicController.h"
#include "VideoHeader.h"

void* pMixerSTMControl = NULL;                // 0x1602
void* pMixerSTMVelocity = NULL;               // 0x1604
void* pMixerSTMStatus = NULL;                 // 0x1A03
void* pMixerENCStatusCompact = NULL;          // 0x1A00
void* pMixerENCControlCompact = NULL;         // 0x1600
EtherCatDataSet* m_EtherCatDataRing0 = NULL;  // Real time access
EtherCatDataSet* m_EtherCatDataRing3 = NULL;  // Application access
ImageData* m_ImageData = NULL;                // includes the measuring function and handle the image data
ProgramLogicController* m_ProgramlogicController = NULL;
unsigned char* m_SharedMemoryImageBlock = NULL;       // pointer shared memory for live image view
unsigned char* m_SharedMemoryVideoBlock = NULL;       // pointer shared meomry video data
unsigned char* m_SharedMemoryCleanImageBlock = NULL;  // pointer shared meomry video data
unsigned char* m_SharedmemoryVideoBlockCameraSimulation = NULL;
unsigned char* m_SharedmemoryVideoBlockFillingProcess = NULL;
bool m_DebugOn = false;  // only for tests
unsigned long long m_LastTimeStampIOTaskIn100nsUnits = 0;
cv::Mat m_VideoImage;  // only used for camera simulation
KSError GetCameraRawDataAndStartMeasure(ExchangeMemory* SharedData);
void DebugFormat(const char* format, ...);
void GetCurrentTimeStampIn100nsUnits(ExchangeMemory* SharedData, unsigned long long& TimeStamp);
void SetInputOutputInfoData(ExchangeMemory* SharedData, std::string& InfoString, int InfoCode);
KSError PostEtherCatData(ExchangeMemory* SharedData);
void CopyEtherCatDataInputDataToSharedMemoryData(ExchangeMemory* SharedData);
void CopySharedMemoryDataToEtherCatDataOutputData(ExchangeMemory* SharedData);
KSError ConfigStepperMotor(ExchangeMemory* SharedData);
KSError StepperMotorSetPDOAssignVelocity(ExchangeMemory* SharedData);
KSError SetEcatDataObjAddressMixerVelocity(ExchangeMemory* SharedData);
int GetIndexMixer(ExchangeMemory* SharedData);
void GetInternPosition(ExchangeMemory* SharedData, unsigned long long CurrentTimeStamp);
void GetExternalPosition(ExchangeMemory* SharedData, unsigned long long CurrentTimeStamp);
void SetStatusToGUI(ExchangeMemory* SharedData, unsigned long long CurrentTimeStamp = 0);
void RunProgramLogicController(ExchangeMemory* SharedData, unsigned long long TimeStamp);

// einlesen der Adressen der Speicherblöcke für die unterschiedlichen Bilddaten, wird in der Windowsanwendung aufgereufen nachdem der Speicher angelegt wurde
// Die Speicherbereiche werden auf der Windowsseite angelegt
extern "C" KSError __declspec(dllexport) __stdcall SetSharedMemoryPointer(void* pArgs)
{
    ExchangeMemory* SharedData = reinterpret_cast<ExchangeMemory*>(pArgs);
    KSError ksError = KS_OK;
    std::string OpenCV_Version = CV_VERSION;

    ksError = KS_getSharedMemEx((KSHandle)(SharedData->m_HandleImageBlock), (void**)&m_SharedMemoryImageBlock, 0);  // Adresse für das Livebild
    if (ksError == KS_OK) {
        ksError = KS_getSharedMemEx((KSHandle)(SharedData->m_HandleVideoBlock), (void**)&m_SharedMemoryVideoBlock, 0);  // Adresse für das Video
        if (ksError == KS_OK) {
            // Adresse für das Bild für die Überprüfung des Grades der Verschmutzung
            ksError = KS_getSharedMemEx((KSHandle)(SharedData->m_HandleCleanImageBlock), (void**)&m_SharedMemoryCleanImageBlock, 0);
            if (ksError == KS_OK) {
                ksError = KS_getSharedMemEx((KSHandle)(SharedData->m_HandleVideoBlockFillingProcess), (void**)&m_SharedmemoryVideoBlockFillingProcess, 0);
            }
            // Wenn keine Kamera angeschlossen, wird hier ein Video gespeichert um die Kamera zu simulieren
            if ((KSHandle)(SharedData->m_HandleVideoBlockCameraSimulation) != KS_INVALID_HANDLE) {
                ksError = KS_getSharedMemEx((KSHandle)(SharedData->m_HandleVideoBlockCameraSimulation), (void**)&m_SharedmemoryVideoBlockCameraSimulation, 0);
            }
        }
    }
    // get th OpenCV Version
    strncpy_s(SharedData->m_BufferInfoText, OpenCV_Version.c_str(), sizeof(SharedData->m_BufferInfoText));
    SharedData->m_BufferInfoText[sizeof(SharedData->m_BufferInfoText) - 1] = 0;
    return ksError;
}

extern "C" KSError __declspec(dllexport) __stdcall InitEtherCatDataSet(void* pArgs)
{
    ExchangeMemory* SharedData = reinterpret_cast<ExchangeMemory*>(pArgs);
    KSError ksError = KS_OK;
    int IndexMixer = GetIndexMixer(SharedData);

    if (SharedData && IndexMixer != -1) {
        ksError = KS_createQuickMutex(&(SharedData->m_EtherCATConfigData.m_HandleReadWriteMutex), KS_RTX_LEVEL, KSF_KERNEL_EXEC);
        if (ksError == KS_OK) {
            // ksError = StepperMotorSetPDOAssignVelocity(SharedData);
            if (ksError == KS_OK) {
                ksError = KS_createEcatDataSet(SharedData->m_EtherCATConfigData.m_HandleEtherCatMaster, &(SharedData->m_EtherCATConfigData.m_HandleEtherCatDataSet), (void**)&m_EtherCatDataRing3,
                                               (void**)&m_EtherCatDataRing0, 0);
                if (ksError == KS_OK) {
                    for (int i = 0; i < SharedData->m_EtherCATConfigData.m_NumberSlaves; i++) {
                        ksError = KS_assignEcatDataSet(SharedData->m_EtherCATConfigData.m_HandleEtherCatDataSet, SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[i], KS_ECAT_SYNC_ALL, 0, 0);
                    }
                    if (ksError == KS_OK) {
                        ksError = SetEcatDataObjAddressMixerVelocity(SharedData);
                        if (ksError == KS_OK) {
                            ksError = ConfigStepperMotor(SharedData);
                            if (ksError == KS_OK) {
                                ksError = KS_installEcatHandler(SharedData->m_EtherCATConfigData.m_HandleEtherCatDataSet, KS_DATASET_SIGNAL,
                                                                SharedData->m_EtherCATConfigData.m_HandleReadEtherCatDataTask, 0);
                                if (ksError == KS_OK) {
                                    // ksError = KS_changeEcatSlaveState(SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[IndexMixer], KS_ECAT_STATE_SAFEOP, 0);
                                    if (ksError == KS_OK) {
                                        //int64 startTime = 0;
                                        //uint cycleTime = 100000;  // cycleTime in ns
                                        //int shiftTime = 0;
                                        // Startet automatisch den Timer
                                        /*ksError = KS_activateEcatDcMode(SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[IndexMixer], startTime, cycleTime, shiftTime,
                                                                        SharedData->m_EtherCATConfigData.m_HandleIOTask, 0);*/
                                        if (ksError == KS_OK) {
                                            ksError = KS_changeEcatState(SharedData->m_EtherCATConfigData.m_HandleEtherCatDataSet, KS_ECAT_STATE_SAFEOP, 0);
                                            if (ksError == KS_OK) {
                                                ksError = KS_changeEcatState(SharedData->m_EtherCATConfigData.m_HandleEtherCatDataSet, KS_ECAT_STATE_OP, 0);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return ksError;
}
//------------------------------------------------------------------------------------------------------------
// Because the 0x1A06 isn't assigned to a sync manager by default we need to do this on
// our own.
// Because this requires to reset the output sync manager first, we will also have to add the PDO mapping
// of those PDO's that would have been mapped by default (0x1A00 and 0x1A03).
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
// Finally bring slave to state KS_ECAT_STATE_SAFEOP, so that the master can download the new mapping,
// if you want to alter the mapping, the slave has to be in state KS_ECAT_STATE_PREOP or lower,
// a new mapping will be committed in state transition from KS_ECAT_STATE_PREOP to KS_ECAT_STATE_SAFEOP.
//------------------------------------------------------------------------------------------------------------
// Volocity control compact with info data
KSError StepperMotorSetPDOAssignVelocity(ExchangeMemory* SharedData)
{
    KSError ksError = KS_OK;
    int IndexMixer = GetIndexMixer(SharedData);

    if (IndexMixer != -1) {
        ksError = KS_changeEcatSlaveState(SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[IndexMixer], KS_ECAT_STATE_PREOP, 0);
        if (ksError == KS_OK) {
            KSEcatDcParams params;
            ksError = KS_lookupEcatDcOpMode(SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[IndexMixer],  // Slave handle
                                            "DC",                                                                // OpMode name
                                            &params,                                                             // Address of KSEcatDcParams structure
                                            0);                                                                  // Flags, here none
            if (ksError != KS_OK) return ksError;

            ksError = KS_configEcatDcOpMode(SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[IndexMixer],  // Slave handle
                                            "DC",                                                                // OpMode name
                                            &params,                                                             // Address of KSEcatDcParams structure
                                            0);                                                                  // Flags, here none
            if (ksError != KS_OK) return ksError;

            // ksError = KS_setEcatPdoAssign(SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[IndexMixer], KS_ECAT_SYNC_INPUT, -1, 0);  // reset
            // if (ksError == KS_OK) {
            //    ksError = KS_setEcatPdoAssign(SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[IndexMixer], KS_ECAT_SYNC_INPUT, 0x1A00, 0);
            //    if (ksError == KS_OK) {
            //        ksError = KS_setEcatPdoAssign(SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[IndexMixer], KS_ECAT_SYNC_INPUT, 0x1A03, 0);
            //
            //    }
            //}
        }
    }
    return ksError;
}

int GetIndexMixer(ExchangeMemory* SharedData)
{
    int IndexMixer = -1;

    if (SharedData) {
        for (int i = 0; i < SharedData->m_EtherCATConfigData.m_NumberSlaves; i++) {
            if (SharedData->m_EtherCATConfigData.m_IOTerminals[i].m_IsMotor) {
                IndexMixer = i;
                break;
            }
        }
    }
    return IndexMixer;
}

KSError SetEcatDataObjAddressMixerVelocity(ExchangeMemory* SharedData)
{
    int bitOffset = 0;
    int bitLength = 0;
    KSError ksError = KS_OK;
    bool setAdressEMCControlCompact = false;
    bool setAdressSTMControl = false;
    bool setAdressSTMVelocity = false;
    bool setAdressSTMSTMStatus = false;
    bool setAdressENCStatusCompact = false;
    int index = GetIndexMixer(SharedData);

    if (index != -1) {
        ksError = KS_getEcatDataObjAddress(SharedData->m_EtherCATConfigData.m_HandleEtherCatDataSet, SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[index], 0x1600, -1, NULL,
                                           &pMixerENCControlCompact, &bitOffset, &bitLength, 0);
        if (ksError == KS_OK) {
            setAdressEMCControlCompact = true;
            memcpy(pMixerENCControlCompact, SharedData->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerENCControlCompact.m_ENCControlCompact, sizeof(MixerENCControlCompact));
            ksError = KS_getEcatDataObjAddress(SharedData->m_EtherCATConfigData.m_HandleEtherCatDataSet, SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[index], 0x1602, -1, NULL,
                                               &pMixerSTMControl, &bitOffset, &bitLength, 0);
            if (ksError == KS_OK) {
                setAdressSTMControl = true;
                memcpy(pMixerSTMControl, &(SharedData->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerSTMControl), sizeof(MixerSTMControl));
                ksError = KS_getEcatDataObjAddress(SharedData->m_EtherCATConfigData.m_HandleEtherCatDataSet, SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[index], 0x1604, -1, NULL,
                                                   &pMixerSTMVelocity, &bitOffset, &bitLength, 0);
                if (ksError == KS_OK) {
                    setAdressSTMVelocity = true;
                    memcpy(pMixerSTMVelocity, &(SharedData->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerSTMVelocity), sizeof(MixerSTMVelocity));
                    ksError = KS_getEcatDataObjAddress(SharedData->m_EtherCATConfigData.m_HandleEtherCatDataSet, SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[index], 0x1A03, -1, NULL,
                                                       &pMixerSTMStatus, &bitOffset, &bitLength, 0);
                    if (ksError == KS_OK) {
                        setAdressSTMSTMStatus = true;
                        memcpy(pMixerSTMStatus, &(SharedData->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerSTMStatus), sizeof(MixerSTMStatus));
                        ksError = KS_getEcatDataObjAddress(SharedData->m_EtherCATConfigData.m_HandleEtherCatDataSet, SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[index], 0x1A00, -1, NULL,
                                                           &pMixerENCStatusCompact, &bitOffset, &bitLength, 0);
                        if (ksError == KS_OK) {
                            setAdressENCStatusCompact = true;
                            memcpy(pMixerENCStatusCompact, (SharedData->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerENCStatusCompact.m_ENCStatusCompact), sizeof(MixerENCStatusCompact));
                        }
                    }
                }
            }
        }
    }

    if (!setAdressSTMControl) {
        ksError = SharedData->m_EtherCATConfigData.m_ErrorCodesetAdressSTMControl;
    } else {
        if (!setAdressSTMSTMStatus) {
            ksError = SharedData->m_EtherCATConfigData.m_ErrorCodesetAdressSTMSTMStatus;
        } else {
            if (!setAdressSTMVelocity) {
                ksError = SharedData->m_EtherCATConfigData.m_ErrorCodesetAdressSTMVelocity;
            } else {
                if (!setAdressENCStatusCompact) {
                    ksError = SharedData->m_EtherCATConfigData.m_ErrorCodesetAdressENCStatusCompact;
                } else {
                    if (!setAdressEMCControlCompact) {
                        ksError = SharedData->m_EtherCATConfigData.m_ErrorCodesetAdressENCStatusCompact;
                    }
                }
            }
        }
    }
    return ksError;
}

KSError ConfigStepperMotor(ExchangeMemory* SharedData)
{
    KSError ksError = KS_OK;
    KSEcatSlaveInfo* pSlaveInfo = NULL;
    KSEcatDataObjInfo* pObjInfo = NULL;
    bool RestoreMotorSettings = false;
    bool setMotorVotage = false;
    bool setMotorCurrent = false;
    bool setStartType = false;
    bool setOperationMode = false;
    bool setMinVelocityMin = false;

    int index = GetIndexMixer(SharedData);
    if (index != -1 && SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[index] != KS_INVALID_HANDLE) {
        // First restore configuration
        int restoreValue = 0x64616F6C;
        int size = 4;
        int varIndex = 0x1011;
        int varSubIndex = 0x01;
        ksError = KS_postEcatDataObj(SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[index], varIndex, varSubIndex, &restoreValue, size, KSF_SDO);
        if (ksError == KS_OK) {
            RestoreMotorSettings = true;
            ksError = KS_queryEcatSlaveInfo(SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[index], &pSlaveInfo, KSF_SDO | KSF_PDO);
            if (ksError == KS_OK) {
                for (int i = 0; i < pSlaveInfo->objCount; ++i) {
                    if (pSlaveInfo->objs[i]->index == 0x8010) {  //  STM Motor Settings Ch .1
                        pObjInfo = pSlaveInfo->objs[i];
                        for (int varIndex = 0; varIndex < pObjInfo->varCount; varIndex++) {
                            KSEcatDataVarInfo* pVarInfo = pObjInfo->vars[varIndex];
                            std::string StringName = pVarInfo->name;
                            if (StringName == "Nominal voltage") {
                                short NominalVoltageInmV = 24000;
                                int size = 2;
                                ksError = KS_postEcatDataObj(SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[index], pObjInfo->index, pObjInfo->vars[varIndex]->subIndex, &NominalVoltageInmV,
                                                             size, KSF_SDO);
                                if (ksError == KS_OK) {
                                    setMotorVotage = true;
                                }
                                break;
                            }
                        }
                        for (int varIndex = 0; varIndex < pObjInfo->varCount; varIndex++) {
                            KSEcatDataVarInfo* pVarInfo = pObjInfo->vars[varIndex];
                            std::string StringName = pVarInfo->name;
                            if (StringName == "Maximal current") {
                                short MaximalCurrentInmA = 2000;
                                int size = 2;
                                ksError = KS_postEcatDataObj(SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[index], pObjInfo->index, pObjInfo->vars[varIndex]->subIndex, &MaximalCurrentInmA,
                                                             size, KSF_SDO);
                                if (ksError == KS_OK) {
                                    setMotorCurrent = true;
                                }
                                break;
                            }
                        }
                    }
                    if (pSlaveInfo->objs[i]->index == 0x8021) {  // 0 POS Settings Ch.1
                        pObjInfo = pSlaveInfo->objs[i];
                        for (int varIndex = 0; varIndex < pObjInfo->varCount; varIndex++) {
                            KSEcatDataVarInfo* pVarInfo = pObjInfo->vars[varIndex];
                            std::string StringName = pVarInfo->name;
                            if (StringName == "Start type") {
                                short StartType = 0x3;  // Endless plus
                                int size = 2;
                                ksError =
                                    KS_postEcatDataObj(SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[index], pObjInfo->index, pObjInfo->vars[varIndex]->subIndex, &StartType, size, KSF_SDO);
                                if (ksError == KS_OK) setStartType = true;
                                break;
                            }
                        }
                    }
                    if (pSlaveInfo->objs[i]->index == 0x8020) {  // POS Features Ch.1
                        pObjInfo = pSlaveInfo->objs[i];
                        for (int varIndex = 0; varIndex < pObjInfo->varCount; varIndex++) {
                            KSEcatDataVarInfo* pVarInfo = pObjInfo->vars[varIndex];
                            std::string StringName = pVarInfo->name;
                            if (StringName == "Velocity min.") {
                                short VelocityMin = 0x0000;
                                int size = 2;
                                ksError =
                                    KS_postEcatDataObj(SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[index], pObjInfo->index, pObjInfo->vars[varIndex]->subIndex, &VelocityMin, size, KSF_SDO);
                                if (ksError == KS_OK) setMinVelocityMin = true;
                                break;
                            }
                        }
                    }
                    if (pSlaveInfo->objs[i]->index == 0x8012) {  // STM Features Ch.1
                        pObjInfo = pSlaveInfo->objs[i];
                        for (int varIndex = 0; varIndex < pObjInfo->varCount; varIndex++) {
                            KSEcatDataVarInfo* pVarInfo = pObjInfo->vars[varIndex];
                            std::string StringName = pVarInfo->name;
                            if (StringName == "Operation mode") {
                                char OperationMode = 0x1;  // direct speed
                                int size = 1;
                                ksError = KS_postEcatDataObj(SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[index], pObjInfo->index, pObjInfo->vars[varIndex]->subIndex, &OperationMode, size,
                                                             KSF_SDO);
                                if (ksError == KS_OK) setOperationMode = true;
                                break;
                            }
                        }
                    }
                }
            } else {
                RestoreMotorSettings = false;
            }
        }
    }
    if (!RestoreMotorSettings) {
        ksError = SharedData->m_EtherCATConfigData.m_ErrorCodeRestoreMotorSettings;
    } else {
        if (!setMotorVotage) {
            ksError = SharedData->m_EtherCATConfigData.m_ErrorCodesetMotorVoltage;
        } else {
            if (!setStartType) {
                ksError = SharedData->m_EtherCATConfigData.m_ErrorCodesetStartType;
            } else {
                if (!setOperationMode) {
                    ksError = SharedData->m_EtherCATConfigData.m_ErrorCodesetOperationMode;
                } else {
                    if (!setMotorCurrent) {
                        ksError = SharedData->m_EtherCATConfigData.m_ErrorCodesetMotorCurrent;
                    } else {
                        if (!setMinVelocityMin) {
                            ksError = SharedData->m_EtherCATConfigData.m_ErrorCodeSetVelocityMin;
                        }
                    }
                }
            }
        }
    }
    return ksError;
}

void GetInternPosition(ExchangeMemory* SharedData, unsigned long long CurrentTimeStamp)
{
    KSEcatSlaveInfo* pSlaveInfo = NULL;
    KSEcatDataObjInfo* pObjInfo = NULL;

    int index = GetIndexMixer(SharedData);
    KSError ksError = KS_queryEcatSlaveInfo(SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[index], &pSlaveInfo, KSF_SDO | KSF_PDO);
    if (ksError == KS_OK) {
        for (int i = 0; i < pSlaveInfo->objCount; ++i) {
            if (pSlaveInfo->objs[i]->index == 0x9010) {  //  STM Motor Settings Ch .1
                pObjInfo = pSlaveInfo->objs[i];
                for (int varIndex = 0; varIndex < pObjInfo->varCount; varIndex++) {
                    KSEcatDataVarInfo* pVarInfo = pObjInfo->vars[varIndex];
                    if (pVarInfo && pVarInfo->subIndex == 0x09) {
                        int size = 4;
                        ksError = KS_readEcatDataObj(SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[index], pObjInfo->index, pObjInfo->vars[varIndex]->subIndex,
                                                     SharedData->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerPositionData.m_InternalPosition, &size, KSF_SDO);
                        if (ksError == KS_OK) {
                            SharedData->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerPositionData.m_CurrentTimeStamp = CurrentTimeStamp;
                        }
                        break;
                    }
                }
                break;
            }
        }
    }
}

void GetExternalPosition(ExchangeMemory* SharedData, unsigned long long CurrentTimeStamp)
{
    KSEcatSlaveInfo* pSlaveInfo = NULL;
    KSEcatDataObjInfo* pObjInfo = NULL;

    int index = GetIndexMixer(SharedData);
    KSError ksError = KS_queryEcatSlaveInfo(SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[index], &pSlaveInfo, KSF_SDO | KSF_PDO);
    if (ksError == KS_OK) {
        for (int i = 0; i < pSlaveInfo->objCount; ++i) {
            if (pSlaveInfo->objs[i]->index == 0x9010) {  //  STM Motor Settings Ch .1
                pObjInfo = pSlaveInfo->objs[i];
                for (int varIndex = 0; varIndex < pObjInfo->varCount; varIndex++) {
                    KSEcatDataVarInfo* pVarInfo = pObjInfo->vars[varIndex];
                    if (pVarInfo && pVarInfo->subIndex == 0x13) {
                        unsigned char ExternalPosition[4];
                        int size = 4;
                        ksError =
                            KS_readEcatDataObj(SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[index], pObjInfo->index, pObjInfo->vars[varIndex]->subIndex, ExternalPosition, &size, KSF_SDO);
                        if (ksError == KS_OK) {
                            memcpy(SharedData->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerPositionData.m_ExternalPosition, ExternalPosition, size);
                            SharedData->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerPositionData.m_CurrentTimeStamp = CurrentTimeStamp;
                        }
                    }
                }
                break;
            }
        }
    }
}

extern "C" KSError __declspec(dllexport) __stdcall FreeEtherCatDataSet(void* pArgs)
{
    ExchangeMemory* SharedData = reinterpret_cast<ExchangeMemory*>(pArgs);
    KSError ksError = KS_OK;

    if (SharedData) {
        if (SharedData->m_EtherCATConfigData.m_HandleReadWriteMutex != KS_INVALID_HANDLE) {
            ksError = KS_removeQuickMutex(SharedData->m_EtherCATConfigData.m_HandleReadWriteMutex);
        }
        if (SharedData->m_EtherCATConfigData.m_HandleEtherCatDataSet != KS_INVALID_HANDLE) {
            ksError = KS_changeEcatState(SharedData->m_EtherCATConfigData.m_HandleEtherCatDataSet, KS_ECAT_STATE_SAFEOP, 0);
            if (ksError == KS_OK) {
                ksError = KS_installEcatHandler(SharedData->m_EtherCATConfigData.m_HandleEtherCatDataSet, KS_DATASET_SIGNAL, KS_INVALID_HANDLE, 0);
                if (ksError == KS_OK) {
                    if (SharedData->m_EtherCATConfigData.m_HandleEtherCatDataSet != KS_INVALID_HANDLE) {
                        ksError = KS_deleteEcatDataSet(SharedData->m_EtherCATConfigData.m_HandleEtherCatDataSet);
                        for (int i = 0; i < SharedData->m_EtherCATConfigData.m_NumberSlaves; i++) {
                            ksError = KS_changeEcatState(SharedData->m_EtherCATConfigData.m_HandleEtherCatSlave[i], KS_ECAT_STATE_INIT, 0);
                        }
                    }
                }
            }
        }
    }
    return ksError;
}

extern "C" KSError __declspec(dllexport) __stdcall ImageReceivedTaskCallBack(void* pArgs, void* /*pContext*/)
{
    KSError ksError = KS_OK;
    ExchangeMemory* SharedData = reinterpret_cast<ExchangeMemory*>(pArgs);

    if (SharedData) {
        if (!SharedData->m_CameraSimulationOn) {
            SharedData->m_CounterCameraFrames++;                          // counter is used to check is camera running
            if (SharedData->m_MeasuringParameter.m_ReadyMeasuringTask) {  // Nur dann das Event setzen wenn letzte Messung beendet
                SharedData->m_MeasuringParameter.m_ReadyMeasuringTask = false;
                KS_setEvent((KSHandle)(SharedData)->m_HandleImageReceivedEventMeasuring);
            } else {  // läuft die Messung noch, dann Bild löschen
                KSCameraBlock* pKsCameraBlock;
                char* pBits;
                while (KS_recvCameraImage((KSHandle)(SharedData)->m_HandleStream, (void**)&pBits, &pKsCameraBlock, 0) == KS_OK)
                    KS_releaseCameraImage((KSHandle)(SharedData)->m_HandleStream, (void*)pBits, 0);
            }
        } else {  // here camera simulation und ethercat simulation
            if (m_SharedmemoryVideoBlockCameraSimulation) {
                VideoHeader Header;
                unsigned char* pVideoData = m_SharedmemoryVideoBlockCameraSimulation;
                memcpy(&Header, pVideoData, sizeof(VideoHeader));  // read video header
                SharedData->m_CounterCameraFrames = 0;
                pVideoData = m_SharedmemoryVideoBlockCameraSimulation + sizeof(VideoHeader);
                unsigned long long CurrentTimeStamp = 0;
                double WaitTime = 1.12;  // 1.12ms -> simulate 890 frames per second

                //GetCurrentTimeStampIn100nsUnits(SharedData, m_LastTimeStampIOTaskIn100nsUnits);
                while (true) {
                    cv::Rect CameraROI;
                    CameraROI.x = SharedData->m_MeasuringParameter.m_MeasureWindowOffsetX[ROI_INDEX_CAMERA];
                    CameraROI.y = SharedData->m_MeasuringParameter.m_MeasureWindowOffsetY[ROI_INDEX_CAMERA];
                    CameraROI.width = SharedData->m_MeasuringParameter.m_MeasureWindowWidth[ROI_INDEX_CAMERA];
                    CameraROI.height = SharedData->m_MeasuringParameter.m_MeasureWindowHeight[ROI_INDEX_CAMERA];
                    if (SharedData->m_AbortCameraSimulation) break;
                    switch (SharedData->m_VideoStateCameraSimulation) {
                        case PLAY_ONE_VIDEO_CAMERA_SIMULATION:
                        case PLAY_VIDEO_CAMERA_SIMULATION:
                        case STEP_VIDEO_CAMERA_SIMULATION:
                            if (SharedData->m_MeasuringParameter.m_ReadyMeasuringTask) {
                                pVideoData = m_SharedmemoryVideoBlockCameraSimulation + sizeof(VideoHeader) + (Header.m_ImageHeight * Header.m_ImageWidth * SharedData->m_CounterCameraFrames);
                                cv::Mat CameraImage(Header.m_ImageHeight, Header.m_ImageWidth, CV_8UC1, (void*)pVideoData, Header.m_ImageWidth);
                                m_VideoImage = CameraImage(CameraROI).clone();
                                SharedData->m_MeasuringParameter.m_ReadyMeasuringTask = false;
                                KS_setEvent((KSHandle)(SharedData)->m_HandleImageReceivedEventMeasuring);
                                SharedData->m_CounterCameraFrames++;
                            }
                            break;
                        case STOP_VIDEO_CAMERA_SIMULATION:
                            pVideoData = m_SharedmemoryVideoBlockCameraSimulation;
                            memcpy(&Header, pVideoData, sizeof(VideoHeader));  // read video header
                            Header.m_CurrentNumberFrames = 0;                  // Clear all videodata
                            Header.m_FrameIndex = 0;
                            break;
                        default:
                            break;
                    }
                    if (SharedData->m_VideoStateCameraSimulation == STEP_VIDEO_CAMERA_SIMULATION) {
                        SharedData->m_VideoStateCameraSimulation = STOP_VIDEO_CAMERA_SIMULATION;
                    }
                    GetCurrentTimeStampIn100nsUnits(SharedData, CurrentTimeStamp);
                    KS_sleepTask(static_cast<int>(WaitTime * ms));  // 890 Frames per second
                    if (SharedData->m_CounterCameraFrames >= Header.m_MaxNumberFrames) {
                        SharedData->m_CounterCameraFrames = 0;  // video wieder von vorne laufen lassen
                        if (SharedData->m_VideoStateCameraSimulation == PLAY_ONE_VIDEO_CAMERA_SIMULATION) {
                            SharedData->m_VideoStateCameraSimulation = STOP_VIDEO_CAMERA_SIMULATION;
                        }
                    }
                    //if ((CurrentTimeStamp - m_LastTimeStampIOTaskIn100nsUnits) > (500 * ms)) {
                    //    m_LastTimeStampIOTaskIn100nsUnits = CurrentTimeStamp;
                    //    SetStatusToGUI(SharedData);  // Statusdaten alle 500ms an die Applikation
                    //}
                }
            }
        }
    } else
        ksError = KSERROR_FUNCTION_NOT_AVAILABLE;
    return ksError;
}

extern "C" KSError __declspec(dllexport) __stdcall MeasureTaskCallBack(void* pArgs)
{
    ExchangeMemory* SharedData = reinterpret_cast<ExchangeMemory*>(pArgs);
    KSError ksError = KS_OK;
    std::string ErrorMsg;
    std::string InfoText;

    ksError = KS_setTargetProcessor(SharedData->m_MeasuringParameter.m_TargetProcessorMeasureTask, 0);
    if (ksError == KS_OK) {
        if (SharedData && (SharedData->m_HandleStream != KS_INVALID_HANDLE || SharedData->m_CameraSimulationOn)) {
            if (m_ImageData == NULL)
                m_ImageData = new ImageData(SharedData, m_SharedMemoryImageBlock, m_SharedMemoryVideoBlock, m_SharedMemoryCleanImageBlock, m_SharedmemoryVideoBlockFillingProcess);
            else
                m_ImageData->SetExchangeMemory(SharedData);
            cv::setNumThreads(0);
            SharedData->m_MeasuringTaskIsStarted = true;  // info an GUI
            SetStatusToGUI(SharedData);
            SharedData->m_MeasuringParameter.m_ReadyMeasuringTask = true;
            while (true) {
                KS_waitForEvent((KSHandle)(SharedData)->m_HandleImageReceivedEventMeasuring, 0, 0);  // Wait for notification of image reception or abortion
                if (SharedData->m_MeasuringParameter.m_AbortMeasuringTask) break;
                GetCameraRawDataAndStartMeasure(SharedData);
                SharedData->m_MeasuringParameter.m_ReadyMeasuringTask = true;
                KS_sleepTask(100 * us);  // sicherheitswert damit beim Messausfall oder extrem kleinen Messzeiten die Task nicht staendig aufgerufen wird
            }
        } else {
            ErrorMsg = "Error Invalid Camera Stream Handle. Measure Task Is Not Running!\n";
            SetInputOutputInfoData(SharedData, ErrorMsg, INFO_CODE_INPUT_OUTPUT_DEVICE_ANY_ERROR);
            ksError = KSERROR_FUNCTION_NOT_AVAILABLE;
        }
    } else {
        // ErrorMsg = "Can Not Set Targetprocessor. Measure Task Is Not Running!\n";
        ErrorMsg = cv::format("Can Not Set Targetprocessor. Measure Task Is Not Running. Processor:%d\n", SharedData->m_MeasuringParameter.m_TargetProcessorMeasureTask);
        SetInputOutputInfoData(SharedData, ErrorMsg, INFO_CODE_INPUT_OUTPUT_DEVICE_ANY_ERROR);
        ksError = KSERROR_FUNCTION_NOT_AVAILABLE;
    }
    if (m_ImageData) {
        delete m_ImageData;
        m_ImageData = NULL;
    }
    SharedData->m_MeasuringTaskIsStarted = false;
    KS_setEvent((KSHandle)(SharedData)->m_HandleEventMeasuringTaskFinished);
    return ksError;
}

// Task für den Datenaustausch mit den Beckhoffklemmen
extern "C" KSError __declspec(dllexport) __stdcall IOTaskCallBack(void* pArgs)
{
    ExchangeMemory* SharedData = reinterpret_cast<ExchangeMemory*>(pArgs);
    KSError ksError = KS_OK;
    std::string ErrorMsg;
    unsigned long long TimeInterSetIOStatusToGUIIn100NanoSecUnits = ms * 500;  // 500ms Interval show status to GUI
    unsigned long long CurrentTimeStamp = 0;

    ksError = KS_setTargetProcessor(SharedData->m_MeasuringParameter.m_TargetProcessorIOTask, 0);
    if (ksError == KS_OK) {
        if (SharedData) {
            // GetCurrentTimeStampIn100nsUnits(SharedData, m_LastTimeStampIOTaskIn100nsUnits);
            if (!SharedData->m_IOTaskIsStatred) SharedData->m_IOTaskIsStatred = true;  // info an GUI
            // SetStatusToGUI(SharedData);
            // PostEtherCatData(SharedData);  // Einmal daten senden um einmal die Eingangsdaten zu lesen
            // KS_sleepTask(10 * ms);         // Warte bis Eingangsdaten gelesen

            // while (true) {
            // if (SharedData->m_MeasuringParameter.m_AbortInputOutputTask) break;
            GetCurrentTimeStampIn100nsUnits(SharedData, CurrentTimeStamp);
            RunProgramLogicController(SharedData, CurrentTimeStamp);  // SPS ablauf
            PostEtherCatData(SharedData);                             // senden der Daten an die IO Module, dieser Aufruf erzwingt dann ein einlesen der Eingangsdaten
            // KS_sleepTask(static_cast<int>(SharedData->m_EtherCATConfigData.m_CyclusTimeIOTaskInms * ms));  // Wartezeit
            if ((CurrentTimeStamp - m_LastTimeStampIOTaskIn100nsUnits) > TimeInterSetIOStatusToGUIIn100NanoSecUnits) {
                m_LastTimeStampIOTaskIn100nsUnits = CurrentTimeStamp;
                SetStatusToGUI(SharedData, CurrentTimeStamp);  // Statusdaten alle "TimeInterSetIOStatusToGUIIn100NanoSecUnits" an die Applikation
            }

            //}
        } else {
            ErrorMsg = "Error!! No Shareddata. IO Task Is Not Running";
            SetInputOutputInfoData(SharedData, ErrorMsg, INFO_CODE_INPUT_OUTPUT_DEVICE_ANY_ERROR);
            ksError = KSERROR_FUNCTION_NOT_AVAILABLE;
        }
    } else {
        // ErrorMsg = "Can Not Set Targetprocessor in IO Task!\n";
        ErrorMsg = cv::format("Can Not Set Targetprocessor IO Task. Processor:%d\n", SharedData->m_MeasuringParameter.m_TargetProcessorIOTask);
        SetInputOutputInfoData(SharedData, ErrorMsg, INFO_CODE_INPUT_OUTPUT_DEVICE_ANY_ERROR);
        ksError = KSERROR_FUNCTION_NOT_AVAILABLE;
    }
    // SharedData->m_IOTaskIsStatred = false;
    // KS_setEvent((KSHandle)(SharedData)->m_HandleEventInputOutputTaskFinished);  // event to no real time application IO Task is finished
    return ksError;
}

void SetStatusToGUI(ExchangeMemory* SharedData, unsigned long long CurrentTimeStamp)
{
    if (CurrentTimeStamp > 0) {
        GetInternPosition(SharedData, CurrentTimeStamp);
    }
    if (m_DebugOn)
        SharedData->m_InfoCodeInputOutputDevice = INFO_CODE_DEBUG_INFO;  // hier nur für testzwecke
    else
        SharedData->m_InfoCodeInputOutputDevice = INFO_CODE_INPUT_DATA_STATUS;  // Infocode für die GUI um welche Art von Daten es sich handelt
    // Event auf signalisierend setzen. In der Application wartet ein Thread auf diese Signal und zeigt dann die Aktuellen Statusdaten in der GUI
    KS_setEvent((KSHandle)(SharedData)->m_HandleEventInputOutput);
}

void RunProgramLogicController(ExchangeMemory* SharedData, unsigned long long TimeStamp)
{
    if (m_ImageData) {
        if (m_ProgramlogicController == NULL) {
            m_ProgramlogicController = new ProgramLogicController(m_ImageData);
        }
        m_ProgramlogicController->RunPLC(SharedData, TimeStamp);
    }
}
//--------------------------------------------------------------------------------------------------------------
// This callback is called when the DataSet returns from the topology to the master.
// Slaves will have written their process data to it.
// KS_readEcatDataSet is used to retrieve the DataSet to access the data.
//--------------------------------------------------------------------------------------------------------------
extern "C" KSError __declspec(dllexport) __stdcall ReadEtherCatDataCallBack(void* pArgs)
{
    ExchangeMemory* SharedData = reinterpret_cast<ExchangeMemory*>(pArgs);
    KSError ksError = KS_OK;
    std::string InfoText;
    unsigned long long TimeStampIn100nsUnits = 0;
    static bool running = false;

    KS_requestQuickMutex(SharedData->m_EtherCATConfigData.m_HandleReadWriteMutex);
    ksError = KS_readEcatDataSet(SharedData->m_EtherCATConfigData.m_HandleEtherCatDataSet, KSF_ACCEPT_INCOMPLETE);
    if (ksError == KS_OK) {
        // aufnahme der Timestamps dient der Überprüfung ob der Ethercat verbunden ist
        GetCurrentTimeStampIn100nsUnits(SharedData, TimeStampIn100nsUnits);
        SharedData->m_EtherCATConfigData.m_LastTimeStampIn100nsUnits = SharedData->m_EtherCATConfigData.m_CurrentTimeStampIn100nsUnits;
        SharedData->m_EtherCATConfigData.m_CurrentTimeStampIn100nsUnits = TimeStampIn100nsUnits;
        running = true;
        SharedData->m_KS_ErrorCode = ksError;
        CopyEtherCatDataInputDataToSharedMemoryData(SharedData);
    } else {
        if (!running && KSERROR_CODE(ksError) == KSERROR_NO_RESPONSE) {
            InfoText = "KS_readEcatDataSet Failed";
            SetInputOutputInfoData(SharedData, InfoText, INFO_CODE_INPUT_DEVICE_KITHARA_ERROR_CODE);
            SharedData->m_KS_ErrorCode = ksError;
            // This error indicates that a slave did not answer to KS_postEcatDataSet.
            // Some slaves need more time to fully arrive in SAFEOP and answer to the DataSet.
            // So we are simply ignoring this error here.
            // A real world application should handle this error differently during startup and running phase.
        }
    }
    KS_releaseQuickMutex(SharedData->m_EtherCATConfigData.m_HandleReadWriteMutex);
    return ksError;
}

void CopyEtherCatDataInputDataToSharedMemoryData(ExchangeMemory* SharedData)
{
    int FirstByte;
    for (int i = 0; i < SharedData->m_EtherCATConfigData.m_NumberSlaves; i++) {
        if (!SharedData->m_EtherCATConfigData.m_IOTerminals[i].m_IsMotor) {
            if (SharedData->m_EtherCATConfigData.m_IOTerminals[i].m_IsInput) {  // output data
                FirstByte = SharedData->m_EtherCATConfigData.m_IOTerminals[i].m_FirstByte;
                if (SharedData->m_RealTimeModusOnRing0)
                    memcpy(&(SharedData->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[FirstByte]), &(m_EtherCatDataRing0->EtherCatData[FirstByte]),
                           SharedData->m_EtherCATConfigData.m_IOTerminals[i].m_LenghtInBytes);
                else
                    memcpy(&(SharedData->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[FirstByte]), &(m_EtherCatDataRing3->EtherCatData[FirstByte]),
                           SharedData->m_EtherCATConfigData.m_IOTerminals[i].m_LenghtInBytes);
            }
        } else {
            if (pMixerSTMStatus) {  // 0x1A03 nur aktiv wenn  Geschwindikeitsinterface aktiv
                memcpy(&(SharedData->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerSTMStatus), pMixerSTMStatus, sizeof(MixerSTMStatus));
            }
            if (pMixerENCStatusCompact) {  // 0x1A00 nur aktiv wenn Geschwindikeitsinterface aktiv
                memcpy(SharedData->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerENCStatusCompact.m_ENCStatusCompact, pMixerENCStatusCompact, sizeof(MixerENCStatusCompact));
            }
        }
    }
}

void CopySharedMemoryDataToEtherCatDataOutputData(ExchangeMemory* SharedData)
{
    int FirstByte;
    for (int i = 0; i < SharedData->m_EtherCATConfigData.m_NumberSlaves; i++) {
        if (!SharedData->m_EtherCATConfigData.m_IOTerminals[i].m_IsMotor) {
            if (!SharedData->m_EtherCATConfigData.m_IOTerminals[i].m_IsInput) {  // output data
                FirstByte = SharedData->m_EtherCATConfigData.m_IOTerminals[i].m_FirstByte;
                if (SharedData->m_RealTimeModusOnRing0)
                    memcpy(&(m_EtherCatDataRing0->EtherCatData[FirstByte]), &(SharedData->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[FirstByte]),
                           SharedData->m_EtherCATConfigData.m_IOTerminals[i].m_LenghtInBytes);
                else
                    memcpy(&(m_EtherCatDataRing3->EtherCatData[FirstByte]), &(SharedData->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[FirstByte]),
                           SharedData->m_EtherCATConfigData.m_IOTerminals[i].m_LenghtInBytes);
            }
        } else {
            if (pMixerSTMControl) {  // 0x1602 aktiv wenn Positionierinterface oder Geschwindikeitsinterface aktiv
                memcpy(pMixerSTMControl, &(SharedData->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerSTMControl), sizeof(MixerSTMControl));
            }
            if (pMixerENCControlCompact) {  // 0x1600 nur aktiv wenn Geschwindigkeitsinterface aktiv
                memcpy(pMixerENCControlCompact, SharedData->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerENCControlCompact.m_ENCControlCompact, sizeof(MixerENCControlCompact));
            }
            if (pMixerSTMVelocity) {  // 0x1604 nur aktiv wenn Geschwindigkeitsinterface aktiv
                memcpy(pMixerSTMVelocity, &(SharedData->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerSTMVelocity), sizeof(MixerSTMVelocity));
            }
        }
    }
}

// sendet die Ausgangsdaten an den Ethercat und ruft dann die Function ReadEtherCatDataCallBack auf die einmal die Eingangsdaten liest
KSError PostEtherCatData(ExchangeMemory* SharedData)
{
    KSError ksError = KS_OK;
    std::string InfoText;

    if (SharedData->m_EtherCATConfigData.m_HandleEtherCatDataSet != KS_INVALID_HANDLE) {
        KS_requestQuickMutex(SharedData->m_EtherCATConfigData.m_HandleReadWriteMutex);
        CopySharedMemoryDataToEtherCatDataOutputData(SharedData);
        ksError = KS_postEcatDataSet(SharedData->m_EtherCATConfigData.m_HandleEtherCatDataSet, 0);  // post data to slave and calls ReadEtherCatDataCallBack
        if (ksError != KS_OK) {
            InfoText = "KS_postEcatDataSet Failed";
            SetInputOutputInfoData(SharedData, InfoText, INFO_CODE_INPUT_DEVICE_KITHARA_ERROR_CODE);
            SharedData->m_KS_ErrorCode = ksError;
        }
        KS_releaseQuickMutex(SharedData->m_EtherCATConfigData.m_HandleReadWriteMutex);
        if (SharedData->m_EtherCatSimulation) {  // Sind die Klemmen nicht angeschlossen wird ReadEtherCatDataCallBack nicht aufgerufen KS_postEcatDataSet hat keine Wirkung. Wenn
                                                 // m_EtherCatSimulation==true wird in der Applikation kein Fehler angezeigt
            unsigned long long TimeStampIn100nsUnits = 0;
            GetCurrentTimeStampIn100nsUnits(SharedData, TimeStampIn100nsUnits);
            SharedData->m_EtherCATConfigData.m_LastTimeStampIn100nsUnits = SharedData->m_EtherCATConfigData.m_CurrentTimeStampIn100nsUnits;
            SharedData->m_EtherCATConfigData.m_CurrentTimeStampIn100nsUnits = TimeStampIn100nsUnits;
        }
    }
    return ksError;
}

void SetInputOutputInfoData(ExchangeMemory* SharedData, std::string& InfoString, int InfoCode)
{
    SharedData->m_InfoCodeInputOutputDevice = InfoCode;
    strncpy_s(SharedData->m_BufferInfoText, InfoString.c_str(), sizeof(SharedData->m_BufferInfoText));
    SharedData->m_BufferInfoText[sizeof(SharedData->m_BufferInfoText) - 1] = 0;
    // Event auf signalisierend setzen. In der Application wartet ein Thread auf diese Signal und zeigt dann die Aktuellen Statusdaten in der GUI
    KS_setEvent((KSHandle)(SharedData)->m_HandleEventInputOutput);
}

// Bild aus der Kamera auslesen und dann die Messung starten
KSError GetCameraRawDataAndStartMeasure(ExchangeMemory* SharedData)
{
    KSError ksError = KS_OK;
    char* pBits = NULL;
    unsigned long long TimeStampBefore = 0;
    unsigned long long TimeStampAfter = 0;
    unsigned long long TimeStampInns;
    KSCameraBlock* pKsCameraBlock = NULL;

    GetCurrentTimeStampIn100nsUnits(SharedData, TimeStampBefore);  // Startzeitpunkt der Messung um die Messzeit zu bestimmen
    if (!SharedData->m_CameraSimulationOn) {                       // real time camera is active
        ksError = KS_recvCameraImage((KSHandle)(SharedData)->m_HandleStream, (void**)&pBits, &pKsCameraBlock, 0);
        if (ksError == KS_OK) {
            // We can only accept pure image blocks, after we have detected the block type we can cast the block type pointer to the appropriate structure.
            if (pKsCameraBlock->blockType == KS_CAMERA_BLOCKTYPE_IMAGE) {
                KSCameraImage* pKsCameraImage = (KSCameraImage*)pKsCameraBlock;
                if (pKsCameraImage->pixelFormat == KS_CAMERA_PIXEL_MONO_8) {
                    cv::Mat CameraImage((int)(pKsCameraImage->height), (int)(pKsCameraImage->width), CV_8UC1, (void*)pBits, pKsCameraImage->width + pKsCameraImage->linePadding);
                    CameraImage = CameraImage.clone();  // copy camera raw data into mat class
                    // cv::flip(CameraImage, CameraImage, 1);//test simulate another band direction
                    if (m_ImageData) {
                        m_ImageData->SetImageWidth(pKsCameraImage->width);
                        m_ImageData->SetImageHeight(pKsCameraImage->height);
                        TimeStampInns = TimeStampBefore * 100;  // convert in nsec
                        m_ImageData->SetCurrentTimeStampInNanoSec(TimeStampInns);
                        m_ImageData->Execute(CameraImage);  // Starte Messung
                    }
                }
            }
        }
        ksError = KS_releaseCameraImage((KSHandle)(SharedData)->m_HandleStream, (void*)pBits, 0);
    } else {
        if (m_ImageData && !m_VideoImage.empty()) {  // m_VideoImage comes from Camerasimulation
            // cv::flip(m_VideoImage, m_VideoImage, 1);
            m_ImageData->SetImageWidth(m_VideoImage.cols);
            m_ImageData->SetImageHeight(m_VideoImage.rows);
            TimeStampInns = TimeStampBefore * 100;
            m_ImageData->SetCurrentTimeStampInNanoSec(TimeStampInns);
            m_ImageData->Execute(m_VideoImage);
        }
    }
    GetCurrentTimeStampIn100nsUnits(SharedData, TimeStampAfter);
    double MeasuringTimeInms = (TimeStampAfter - TimeStampBefore) / ((double)(ms));  // calculate inspection time in ms
    if (m_ImageData) {
        m_ImageData->SetMeasuringTimeInms(MeasuringTimeInms);  // Messzeit speichern
    }
    return ksError;
}

void GetCurrentTimeStampIn100nsUnits(ExchangeMemory* SharedData, unsigned long long& TimeStamp)
{
    int64 CurrentTimeStampIn100nsUnits = 0;
    KS_getClock(&CurrentTimeStampIn100nsUnits, KS_CLOCK_MEASURE_HIGHEST);
    KS_convertClock(&CurrentTimeStampIn100nsUnits, KS_CLOCK_MEASURE_HIGHEST, KS_CLOCK_MACHINE_TIME, 0);
    TimeStamp = static_cast<unsigned long long>(CurrentTimeStampIn100nsUnits);
}

void DebugFormat(const char* format, ...)
{
    static char s_printf_buf[512];
    va_list args;
    va_start(args, format);
    _vsnprintf_s(s_printf_buf, sizeof(s_printf_buf), format, args);
    va_end(args);
    OutputDebugStringA(s_printf_buf);
}

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

BOOL WINAPI DllMain(HINSTANCE hInstDll, DWORD reason, LPVOID pReserved)
{
    return TRUE;
}
