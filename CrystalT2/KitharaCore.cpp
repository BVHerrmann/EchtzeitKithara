#include "KrtsBertram.h"
#define ulong NotUse
#include <windows.h>
#include "EtherCatSlaveData.h"
#include "GlobalConst.h"
#include "ImageData.h"
#include "KitharaCore.h"
#include "MainAppCrystalT2.h"
#include "SettingsData.h"
#include "VideoHeader.h"

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"

KitharaCore::KitharaCore(MainAppCrystalT2* pMainAppCrystalT2)
    : m_MainAppCrystalT2(NULL),
      m_IsKitharaDriverOpen(false),
      m_PointerImageBlock(NULL),
      m_PointerVideoBlock(NULL),
      m_PointerCleanImageBlock(NULL),
      m_PointerVideoBlockFillingProcess(NULL),
      m_PointerVideoBlockCameraSimulation(NULL),
      m_KernelParameter(NULL),
      m_ExchangeMemory(NULL),
      m_MaxImageWidth(USED_CAMERA_WIDTH),
      m_MaxImageHeight(USED_CAMERA_HEIGHT),
      m_WorkWithoutCamera(false),
      m_WorkWithoutEtherCat(false),
      m_LiquidFlowSimulationOn(false),
      m_IsInitWent(true),
      m_UseUSBCameraInterface(true),
      m_IsCameraResolutionGreaterThanDefaultResolution(true),
      m_HandleKernel(KS_INVALID_HANDLE)
{
    m_MainAppCrystalT2 = pMainAppCrystalT2;
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        m_WorkWithoutCamera = GetMainAppCrystalT2()->GetSettingsData()->m_WorkWithoutCamera;
        m_WorkWithoutEtherCat = GetMainAppCrystalT2()->GetSettingsData()->m_WorkWithoutEtherCat;
        m_LiquidFlowSimulationOn = GetMainAppCrystalT2()->GetSettingsData()->m_LiquidFlowSimulationOn;
    }
}

KitharaCore::~KitharaCore()
{
}

int KitharaCore::FreeKithara()
{
    int rv = ERROR_CODE_NO_ERROR;
    int flags;
    void* pExchangeMemory = NULL;
    QString ErrorMsg, InfoMsg;
    KSError ksError = KS_OK;

    if (m_ExchangeMemory) {
#if _DEBUG
        flags = KSF_NO_CONTEXT | KSF_USER_EXEC;
        pExchangeMemory = (void*)(m_ExchangeMemory);
#else
        flags = KSF_NO_CONTEXT | KSF_KERNEL_EXEC;
        pExchangeMemory = (void*)(m_ExchangeMemory->pSysPtr);
#endif
        rv = StopCameraAcquisition();
        if (rv == ERROR_CODE_NO_ERROR) {
            InfoMsg = tr("Info! StopCameraAcquisition: OK");
            SendMessageToGUI(InfoMsg, QtMsgType::QtInfoMsg);
        }
        Sleep(500);
        if (m_ExchangeMemory->m_HandleCameraSimulationTask != KS_INVALID_HANDLE) {
            rv = StopAndRemoveCameraSimulationTask();
            if (rv == ERROR_CODE_NO_ERROR) {
                InfoMsg = tr("Info! StopAndRemoveCameraSimulationTask: OK");
                SendMessageToGUI(InfoMsg, QtMsgType::QtInfoMsg);
            }
        }
        rv = StopAndRemoveMeasureTask();
        if (rv == ERROR_CODE_NO_ERROR) {
            InfoMsg = tr("Info! StopAndRemoveMeasureTask: OK");
            SendMessageToGUI(InfoMsg, QtMsgType::QtInfoMsg);
        }

        // clear Ethercat
        rv = StopAndRemoveReadEtherCatDataTask();
        if (rv == ERROR_CODE_NO_ERROR) {
            InfoMsg = tr("Info! StopAndRemoveReadEtherCatDataTask: OK");
            SendMessageToGUI(InfoMsg, QtMsgType::QtInfoMsg);
        }

        rv = StopAndRemoveIOTask();
        if (rv == ERROR_CODE_NO_ERROR) {
            InfoMsg = tr("Info! StopAndRemoveIOTask: OK");
            SendMessageToGUI(InfoMsg, QtMsgType::QtInfoMsg);
        }
        // free Ethercat Kernel
        ksError = KS_execKernelFunction(m_HandleKernel, "FreeEtherCatDataSet", pExchangeMemory, NULL, flags);
        if (ksError == KS_OK) {
            InfoMsg = tr("Info! FreeEtherCatDataSet In %1: OK").arg(m_NameKernelDLL);
            SendMessageToGUI(InfoMsg, QtMsgType::QtInfoMsg);
        } else {
            InfoMsg = tr("Info! KS_execKernelFunction FreeEtherCatDataSet Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, InfoMsg);
        }
        rv = FreeEtherCatMasterAndSlave();
        if (rv == ERROR_CODE_NO_ERROR) {
            InfoMsg = tr("Info! FreeEtherCatMasterAndSlave: OK");
            SendMessageToGUI(InfoMsg, QtMsgType::QtInfoMsg);
        }
        // end clear Ethercat
        rv = FreeCameraHandler();
        if (rv == ERROR_CODE_NO_ERROR) {
            InfoMsg = tr("Info! FreeCameraHandler: OK");
            SendMessageToGUI(InfoMsg, QtMsgType::QtInfoMsg);
        }
        rv = CloseRealTimeCamera();
        if (rv == ERROR_CODE_NO_ERROR) {
            InfoMsg = tr("Info! CloseRealTimeCamera: OK");
            SendMessageToGUI(InfoMsg, QtMsgType::QtInfoMsg);
        }
        if (m_UseUSBCameraInterface) {
            rv = CloseXHCIController();
            if (rv == ERROR_CODE_NO_ERROR) {
                InfoMsg = tr("Info! CloseXHCIController: OK");
                SendMessageToGUI(InfoMsg, QtMsgType::QtInfoMsg);
            }
        } else {
            rv = CloseNetworkAdapter();
            if (rv == ERROR_CODE_NO_ERROR) {
                InfoMsg = tr("Info! CloseNetworkAdapter: OK");
                SendMessageToGUI(InfoMsg, QtMsgType::QtInfoMsg);
            }
        }
        rv = RemoveCallBacks();
        if (rv == ERROR_CODE_NO_ERROR) {
            InfoMsg = tr("Info! RemoveCallBacks: OK");
            SendMessageToGUI(InfoMsg, QtMsgType::QtInfoMsg);
        }
        rv = RemoveEvents();
        if (rv == ERROR_CODE_NO_ERROR) {
            InfoMsg = tr("Info! RemoveEvents: OK");
            SendMessageToGUI(InfoMsg, QtMsgType::QtInfoMsg);
        }
        rv = FreeImageSharedMemory();
        if (rv == ERROR_CODE_NO_ERROR) {
            InfoMsg = tr("Info! FreeImageSharedMemory: OK");
            SendMessageToGUI(InfoMsg, QtMsgType::QtInfoMsg);
        }
        rv = FreeImageSharedMemoryForCameraSimulation();
        if (rv == ERROR_CODE_NO_ERROR) {
            InfoMsg = tr("Info! FreeImageSharedMemoryForCameraSimulation: OK");
            SendMessageToGUI(InfoMsg, QtMsgType::QtInfoMsg);
        }
        rv = FreeExchangeSharedMemory();
        if (rv == ERROR_CODE_NO_ERROR) {
            InfoMsg = tr("Info! FreeExchangeSharedMemory: OK");
            SendMessageToGUI(InfoMsg, QtMsgType::QtInfoMsg);
        }
    }  // end if (m_ExchangeMemory)
    rv = FreeKernelDLL();
    if (rv == ERROR_CODE_NO_ERROR) {
        InfoMsg = tr("Info! FreeKernelDLL: OK");
        SendMessageToGUI(InfoMsg, QtMsgType::QtInfoMsg);
    }

    return rv;
}

void KitharaCore::DebugFormat(const char* format, ...)
{
    static char s_printf_buf[512];
    va_list args;
    va_start(args, format);
    _vsnprintf_s(s_printf_buf, sizeof(s_printf_buf), format, args);
    va_end(args);
    OutputDebugStringA(s_printf_buf);
}

void KitharaCore::SendMessageToGUI(QString& Msg, QtMsgType MsgType)
{
    if (GetMainAppCrystalT2()) {
        GetMainAppCrystalT2()->SlotAddNewMessage(Msg, MsgType);
    }
}

unsigned char KitharaCore::GetDigitalInputs(int FirstByte)
{
    unsigned char DigtalInputs = 0;
    if (m_ExchangeMemory) {
        DigtalInputs = m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[FirstByte];  // .m_El1008;
    }
    return DigtalInputs;
}

bool KitharaCore::GetDigitalInput(EtherCATConfigData::IOChannels Channel)
{
    if (m_ExchangeMemory) {
        if (m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[Channel].m_FirstByte] &
            m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[Channel].m_ChannelNumber)
            return true;
        else
            return false;
    } else
        return false;
}

double KitharaCore::GetEtherCatCycelTimeInms()
{
    if (m_ExchangeMemory)
        return (m_ExchangeMemory->m_EtherCATConfigData.m_CurrentTimeStampIn100nsUnits - m_ExchangeMemory->m_EtherCATConfigData.m_LastTimeStampIn100nsUnits) / ((double)(10000.0));
    else
        return 0.0;
}

unsigned long long KitharaCore::GetEtherCATTimeStampIn100nsUnits()
{
    if (m_ExchangeMemory)
        return (m_ExchangeMemory->m_EtherCATConfigData.m_CurrentTimeStampIn100nsUnits);
    else
        return 0;
}

double KitharaCore::GetDeltaTFromISInms()
{
    if (m_ExchangeMemory)
        return (m_ExchangeMemory->m_EtherCATConfigData.m_DeltaTFromIS) / ((double)(10000.0));
    else
        return 0.0;
}

int KitharaCore::MakeCameraDeviceReset()
{
    int rv = ERROR_CODE_NO_ERROR;
    QString ErrorMsg, AddErrorMsg;
    QString NodeName = "DeviceReset";
    QString Value = "Execute";
    KSError ksError = KS_OK;

    ksError = SetStringNodeCameraValue(NodeName, Value);
    if (ksError != KS_OK) {
        AddErrorMsg = tr("Camera Device Reset Failed!");
        rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
    }
    return rv;
}

// void KitharaCore::SetComplationDateRealTimeDLL(QString& NameKernelDLL)
//{
//    QString CompilationTime;
//    QString AppPath = QCoreApplication::applicationDirPath() + QString("/") + NameKernelDLL;
//    QFileInfo fileInfo(AppPath);
//    QString time_format = "MMMM d yyyy  HH:mm:ss";
//    QLocale locale(QLocale("en_US"));
//
//    QDateTime dateTime = fileInfo.lastModified();
//    m_CompilationTimeRealTimeDLL = locale.toString(dateTime, time_format);
//}

int KitharaCore::OpenKitharaDriver(QString& CustomNumber, QString& NameKernelDLL, QString& StartUpInfo, QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    QString AddErrorMsg;
    KSError ksError = KS_OK;
    char* pCustomerNumber = NULL;
    char* pNameKernelDLL = NULL;
    // const char _pCustomerNumber[] = "7BDT-ETQV-BPZ1";

    m_NameKernelDLL = NameKernelDLL;
    pCustomerNumber = new char[CustomNumber.length() + 1];
    strcpy(pCustomerNumber, CustomNumber.toStdString().c_str());
    pCustomerNumber[CustomNumber.length()] = '\0';
    pNameKernelDLL = new char[NameKernelDLL.length() + 1];
    strcpy(pNameKernelDLL, NameKernelDLL.toStdString().c_str());
    pNameKernelDLL[NameKernelDLL.length()] = '\0';

    m_IsKitharaDriverOpen = false;
    ksError = KS_openDriver(pCustomerNumber);  // Opening the driver is always the first step! After that, we can use other functions. If the opening fails, no other function can be called.
    if (ksError == KS_OK) {
        rv = LoadKernelDLL(pNameKernelDLL, ErrorMsg);  // load the Real Time Kernel DLL(KitharaVisionDLL.dll)
        if (rv == ERROR_CODE_NO_ERROR) {
            rv = CreateExchangeMemory(ErrorMsg);  // Create shared memory holds data between application and real time context
            if (rv == ERROR_CODE_NO_ERROR) {
                rv = GetNumberDedicatedCPUs(StartUpInfo, ErrorMsg);
                if (rv == ERROR_CODE_NO_ERROR) {
                    m_IsKitharaDriverOpen = true;
                }
            }
        }
    } else {
        AddErrorMsg = tr("KS_openDriver Failed");
        rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
    }
    if (pCustomerNumber != NULL) delete pCustomerNumber;
    if (pNameKernelDLL != NULL) delete pNameKernelDLL;
    return rv;
}

int KitharaCore::GetKitharaErrorCodeFromRealTimeContext()
{
    int rv = INFO_CODE_INPUT_OUTPUT_DEVICE_NO_INFO;
    if (m_ExchangeMemory)
        return m_ExchangeMemory->m_KS_ErrorCode;
    else
        return rv;
}

bool KitharaCore::GetInfoCodeIsLeftTriggerSet()
{
    bool rv = false;
    if (m_ExchangeMemory)
        return m_ExchangeMemory->m_InfoLeftTriggerIsSet;
    else
        return rv;
}

void KitharaCore::SetInfoCodeIsLeftTriggerSet(bool set)
{
    if (m_ExchangeMemory) {
        m_ExchangeMemory->m_InfoLeftTriggerIsSet = set;
    }
}

bool KitharaCore::GetInfoCodeIsRightTriggerSet()
{
    bool rv = false;
    if (m_ExchangeMemory)
        return m_ExchangeMemory->m_InfoRightTriggerIsSet;
    else
        return rv;
}

void KitharaCore::SetInfoCodeIsRightTriggerSet(bool set)
{
    if (m_ExchangeMemory) {
        m_ExchangeMemory->m_InfoRightTriggerIsSet = set;
    }
}

int KitharaCore::GetInfoCodeIODevice()
{
    int rv = INFO_CODE_INPUT_OUTPUT_DEVICE_NO_INFO;
    if (m_ExchangeMemory)
        return m_ExchangeMemory->m_InfoCodeInputOutputDevice;
    else
        return rv;
}

void KitharaCore::SetInfoCodeIODevice(int code)
{
    if (m_ExchangeMemory) {
        m_ExchangeMemory->m_InfoCodeInputOutputDevice = code;
    }
}

int KitharaCore::CloseKitharaDriver()
{
    if (m_IsKitharaDriverOpen) {
        QString ErrorMsg;
        KSError ksError = KS_closeDriver();
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_closeDriver Failed");
            return GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        } else {
            m_IsKitharaDriverOpen = false;
            return ERROR_CODE_NO_ERROR;
        }

    } else
        return ERROR_CODE_NO_ERROR;
}

int KitharaCore::SearchAndOpenXHCIDeviceForUSBCamera(int DeviceXHCIIndex, QString& StartUpInfo, QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    QString AddErrorMsg;
    char DeviceName[256];

    KSError ksError = KS_enumDevicesEx("XHCI", "PCI", KS_INVALID_HANDLE, DeviceXHCIIndex, DeviceName, 0);  // 'XHCI' searches for XHCI controllers search on PCI Bus
    if (ksError == KS_OK) {
        ksError = KS_openXhciController(&(m_ExchangeMemory->m_HandleController), DeviceName, 0);
        if (ksError == KS_OK) {
            StartUpInfo += tr("XHCI Controller Is Open: %1: OK").arg(DeviceName);
        } else {
            AddErrorMsg = tr("KS_openXhciController Failed: Device Name: %1").arg(DeviceName);
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
    } else {
        AddErrorMsg = tr("KS_enumDevicesEx Failed");
        rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
    }
    return rv;
}
// nur für tests
int KitharaCore::SearchAndOpenNetworkAdapterForNetworkCamera(int DeviceNetworkIndex, QString& StartUpInfo, QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    QString AddErrorMsg;
    char DeviceName[256];
    KSNetworkAdapterConfig adapterConfig = {0};
    adapterConfig.structSize = sizeof(KSNetworkAdapterConfig);
    adapterConfig.configFlags = KS_NETWORK_SUPPORT_JUMBO_FRAMES;
    KSIPConfig ipConfig = {0};
    KS_makeIPv4(&ipConfig.localAddress, 10, 0, 41, 201);
    KS_makeIPv4(&ipConfig.subnetMask, 255, 255, 0, 0);
    KS_makeIPv4(&ipConfig.gatewayAddress, 0, 0, 0, 0);

    KS_ntohl(ipConfig.localAddress);
    KS_ntohl(ipConfig.subnetMask);
    KS_ntohl(ipConfig.gatewayAddress);

    KSError ksError = KS_enumDevicesEx("NET", "PCI", KS_INVALID_HANDLE, DeviceNetworkIndex, DeviceName, 0);
    if (ksError == KS_OK) {
        ksError = KS_openNetworkAdapter(&(m_ExchangeMemory->m_HandleController), DeviceName, &adapterConfig, 0);
        if (ksError == KS_OK) {
            ksError = KS_execNetworkCommand(m_ExchangeMemory->m_HandleController, KS_NETWORK_SET_IP_CONFIG, &ipConfig, 0);
            if (ksError != KS_OK) {
                AddErrorMsg = tr("KS_execNetworkCommand", "Failed To Set IP Config!Name: %1").arg(DeviceName);
                rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
            }
        } else {
            AddErrorMsg = tr("KS_openNetworkAdapter Failed: Device Name: %1").arg(DeviceName);
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
    } else {
        AddErrorMsg = tr("KS_enumDevicesEx Failed");
        rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
    }

    return rv;
}

int KitharaCore::LoadKernelDLL(const char* NameKernelDLL, QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    int Flags = KSF_KERNEL_EXEC;

#if _DEBUG
    Flags = 0;  // KSF_USER_EXEC;
#endif
    KSError ksError = KS_loadVisionKernel(&(m_HandleKernel), NameKernelDLL, NULL, NULL, Flags);
    if (ksError != KS_OK) {
        QString AddErrorMsg = tr("KS_loadVisionKernel Failed. FileName:%1").arg(NameKernelDLL);
        rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
    }

    return rv;
}

int KitharaCore::FreeKernelDLL()
{
    int rv = ERROR_CODE_NO_ERROR;
    QString ErrorMsg;
    KSError ksError = KS_OK;

    if (m_HandleKernel != KS_INVALID_HANDLE) {
        ksError = KS_freeKernel(m_HandleKernel);
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_freeKernel Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
        m_HandleKernel = KS_INVALID_HANDLE;
    }
    return rv;
}

int KitharaCore::WaitForNextImage(int TimeOutInms, unsigned __int64& counterCameraImages, QString& ErrorMsg)
{
    if (m_ExchangeMemory) {
        counterCameraImages = m_ExchangeMemory->m_CounterCameraFrames;
        KSError ksError = KS_waitForEvent(m_ExchangeMemory->m_HandleImageReceivedEvent, 0, TimeOutInms * 10000);
        if (ksError != KS_OK) {
            ErrorMsg = tr("Error! Time Out! No Image From Real Time Task");
            return ERROR_CODE_ANY_ERROR;
        } else
            return ERROR_CODE_NO_ERROR;
    } else {
        ErrorMsg = tr("Error! Can Not Wait For Next Image. No Shared Data");
        return ERROR_CODE_ANY_ERROR;
    }
}

int KitharaCore::WaitForBottleEjected(int TimeOutInms, QString& ErrorMsg)
{
    if (m_ExchangeMemory) {
        KSError ksError = KS_waitForEvent(m_ExchangeMemory->m_HandleEventBottleEjected, 0, TimeOutInms * 10000);
        if (ksError != KS_OK) {
            ErrorMsg = tr("Error! Time Out! No Bottles Ejected");
            return ERROR_CODE_ANY_ERROR;
        } else {
            return ERROR_CODE_NO_ERROR;
        }
    } else {
        ErrorMsg = tr("Error! Can Not Wait For Bottle Ejected");
        return ERROR_CODE_ANY_ERROR;
    }
}

int KitharaCore::WaitForCanCopyNewVideo(int TimeOutInms, QString& ErrorMsg)
{
    if (m_ExchangeMemory) {
        if (m_ExchangeMemory->m_HandleEventCanCopyVideoData != KS_INVALID_HANDLE) {
            KSError ksError = KS_waitForEvent(m_ExchangeMemory->m_HandleEventCanCopyVideoData, 0, TimeOutInms * 10000);
            if (ksError != KS_OK) {
                ErrorMsg = tr("Timeout. Copy New Video");
                return ERROR_CODE_TIME_OUT;
            } else
                return ERROR_CODE_NO_ERROR;
        } else
            return ERROR_CODE_ANY_ERROR;
    } else {
        ErrorMsg = tr("Error! Can Not Wait For Copy New Video. No Shared Data");
        return ERROR_CODE_ANY_ERROR;
    }
}

int KitharaCore::WaitForNextInpuOutput(int TimeOutInms, QString& ErrorMsg)
{
    if (m_ExchangeMemory) {
        if (m_ExchangeMemory->m_HandleEventInputOutput != KS_INVALID_HANDLE) {
            KSError ksError = KS_waitForEvent(m_ExchangeMemory->m_HandleEventInputOutput, 0, TimeOutInms * 10000);
            if (ksError != KS_OK) {
                ErrorMsg = tr("Timeout Inputoutput Task");
                return ERROR_CODE_TIME_OUT;
            } else
                return ERROR_CODE_NO_ERROR;
        } else
            return ERROR_CODE_ANY_ERROR;
    } else {
        ErrorMsg = tr("Error! Can Not Wait For Next InputOutput. No Shared Data");
        return ERROR_CODE_ANY_ERROR;
    }
}

int KitharaCore::SearchCameraDevices(int CameraIndex, int DeviceIndex, QString& CameraHarwareID)
{
    int rv = ERROR_CODE_NO_ERROR;
    QString CameraStartUpInfo, ErrorMsg, AddErrorMsg;
    KSError ksError = KS_OK;
    KSCameraInfo ksCamerInfo;
    ksCamerInfo.structSize = sizeof(KSCameraInfo);

    if (m_UseUSBCameraInterface) {
        rv = SearchAndOpenXHCIDeviceForUSBCamera(DeviceIndex, CameraStartUpInfo, ErrorMsg);
        if (rv == 0) {
            CameraStartUpInfo = tr("Found PCI Device And Open XHCI Controller: Ok");
        }
    } else {
        rv = SearchAndOpenNetworkAdapterForNetworkCamera(DeviceIndex, CameraStartUpInfo, ErrorMsg);
        if (rv == 0) {
            CameraStartUpInfo = tr("Found Network Device And Open Network Adapter: Ok");
        }
    }
    if (rv == 0) {
        SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
        if (m_ExchangeMemory->m_HandleController != KS_INVALID_HANDLE) {
            if (m_UseUSBCameraInterface) {
                ksError = KS_enumCameras(m_ExchangeMemory->m_HandleController, CameraIndex, &ksCamerInfo, 0);
            } else {
                int count = 0;
                for (;;) {
                    count++;
                    ksError = KS_enumCameras(m_ExchangeMemory->m_HandleController, CameraIndex, &ksCamerInfo, 0);
                    if (ksError == KS_OK) {
                        break;
                    }
                    QThread::msleep(300);
                    if (count == 20) {
                        break;
                    }
                }
                ksError = KS_enumCameras(m_ExchangeMemory->m_HandleController, CameraIndex, &ksCamerInfo, KSF_NO_DISCOVERY);
            }
            if (ksError != KS_OK) {
                AddErrorMsg = tr("KS_enumCameras Failed");
                rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
            } else {
                m_CameraVendor = QString("%1").arg(ksCamerInfo.vendor);
                m_CameraName = QString("%1").arg(ksCamerInfo.name);
                m_CameraSerialNumber = QString("%1").arg(ksCamerInfo.serialNumber);
                CameraStartUpInfo = tr("Camera Found:%1 OK").arg(m_CameraName);
                CameraHarwareID = ksCamerInfo.hardwareId;
                SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                // Camera Reset that makes sure thas camera works at the first time on windows
                //  1. Connect the  camera
                //  2. Make Camera DeviceReset, after device reset the camera is disconnected
                //  3. Close the XHCI controller
                //  4. Open the XHCI controller again
                if (m_UseUSBCameraInterface) {
                    rv = OpenRealTimeCamera(CameraHarwareID);
                    if (rv == 0) {
                        CameraStartUpInfo = tr("Info! Make Camera Device Reset. Please Wait...");
                        SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                        rv = MakeCameraDeviceReset();
                        if (rv == 0) {
                            ksError = KS_closeXhci(m_ExchangeMemory->m_HandleController, 0);
                            if (ksError == KS_OK) {
                                rv = SearchAndOpenXHCIDeviceForUSBCamera(DeviceIndex, CameraStartUpInfo, ErrorMsg);
                            } else {
                                AddErrorMsg = tr("KS_closeXhci Failed");
                                rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                            }
                        }
                    }
                }
            }
        } else {
            ErrorMsg = tr("Error! XHCI Controller Invalid Handle");
            SendMessageToGUI(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
            rv = ERROR_CODE_ANY_ERROR;
        }
    }
    return rv;
}

int KitharaCore::InitEtherCatMasterAndSlave(QString& NetworkStartUpInfo, int NetworkDeviceIndex)
{
    int rv = ERROR_CODE_NO_ERROR;
    KSError ksError = KS_OK;
    char NetworkAdapterName[128];
    char ConfigXMLPath[512];
    QString AddErrorMsg, ErrorMsg;
    QString XMLPath = GetMainAppCrystalT2()->GetXMLCofigurationFileLocation();

    if (m_ExchangeMemory) {
        ReadDesignBusTerminals();
        m_ExchangeMemory->m_EtherCATConfigData.m_NumberSlaves = m_ListEtherCatSlaveData.count();
        strcpy(ConfigXMLPath, XMLPath.toStdString().c_str());
        ksError = KS_enumDevices("NET", NetworkDeviceIndex, NetworkAdapterName, 0);
        if (ksError == KS_OK) {
            NetworkStartUpInfo = tr("Info! RealTime Network Hardware ID: %1: OK").arg(NetworkAdapterName);
            SendMessageToGUI(NetworkStartUpInfo, QtMsgType::QtInfoMsg);
            ksError = KS_openNetworkAdapter(&(m_ExchangeMemory->m_EtherCATConfigData.m_HandleNetworkAdapter), NetworkAdapterName, NULL, 0);
            if (ksError == KS_OK) {
                ksError = KS_createEcatMaster(&(m_ExchangeMemory->m_EtherCATConfigData.m_HandleEtherCatMaster), m_ExchangeMemory->m_EtherCATConfigData.m_HandleNetworkAdapter, ConfigXMLPath, "", 0);
                if (ksError == KS_OK) {
                    NetworkStartUpInfo = tr("Info! KS_createEcatMaster: OK");
                    SendMessageToGUI(NetworkStartUpInfo, QtMsgType::QtInfoMsg);
                    if (ConnectToEcatMaster()) {
                        NetworkStartUpInfo = tr("Info! Connect To EtherCat Master: OK");
                        SendMessageToGUI(NetworkStartUpInfo, QtMsgType::QtInfoMsg);
                        if (!m_WorkWithoutEtherCat) {
                            for (int i = 0; i < m_ExchangeMemory->m_EtherCATConfigData.m_NumberSlaves; i++) {
                                ksError = KS_createEcatSlave(m_ExchangeMemory->m_EtherCATConfigData.m_HandleEtherCatMaster, &m_ExchangeMemory->m_EtherCATConfigData.m_HandleEtherCatSlave[i], 0, 0,
                                                             m_ListEtherCatSlaveData.at(i).m_VendorID, m_ListEtherCatSlaveData.at(i).m_ProductID, 0, 0);
                                if (ksError != KS_OK) {
                                    AddErrorMsg = tr("KS_createEcatSlave Failed With Index:%1 ProductID:%2").arg(i + 1).arg(m_ListEtherCatSlaveData.at(i).m_ProductID, 0, 16);
                                    rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                                    break;
                                }
                            }
                            /*if (ksError == KS_OK) {
                                rv = StepperMotorSetPDOAssignVelocity(ErrorMsg);
                                
                            }*/
                        }
                    } else {
                        ErrorMsg = tr("Can Not Connect To EtherCat Master");
                        SendMessageToGUI(ErrorMsg, QtMsgType::QtFatalMsg);
                        rv = ERROR_CODE_ANY_ERROR;
                    }
                } else {
                    AddErrorMsg = tr("KS_createEcatMaster Failed");
                    rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                }
            } else {
                AddErrorMsg = tr("KS_openNetworkAdapter Failed");
                rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
            }
        } else {
            AddErrorMsg = tr("KS_enumDevices Network Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
    }
    if (m_WorkWithoutEtherCat) {
        return ERROR_CODE_NO_ERROR;  // wenn Simulation an dann Fehlermeldung unterdrücken
    } else {
        return rv;
    }
}

KSError KitharaCore::StepperMotorSetPDOAssignVelocity(QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    KSError ksError = KS_OK;
    int IndexSlave = -1;

    for (int i = 0; i < m_ExchangeMemory->m_EtherCATConfigData.m_NumberSlaves; i++) {
        if (m_ExchangeMemory->m_EtherCATConfigData.m_IOTerminals[i].m_IsMotor) {
            IndexSlave = i;
            break;
        }
    }
    if (IndexSlave != -1) {
        // ksError = KS_changeEcatState(m_ExchangeMemory->m_EtherCATConfigData.m_HandleEtherCatSlave[IndexSlave], KS_ECAT_STATE_PREOP, 0);
        if (ksError == KS_OK) {
            KSEcatDcParams params;
            ksError = KS_lookupEcatDcOpMode(m_ExchangeMemory->m_EtherCATConfigData.m_HandleEtherCatSlave[IndexSlave],  // Slave handle
                                            "DC",                                                                      // OpMode name
                                            &params,                                                                   // Address of KSEcatDcParams structure
                                            0);                                                                        // Flags, here none
            if (ksError != KS_OK) return ksError;

            ksError = KS_configEcatDcOpMode(m_ExchangeMemory->m_EtherCATConfigData.m_HandleEtherCatSlave[IndexSlave],  // Slave handle
                                            "DC",                                                                      // OpMode name
                                            &params,                                                                   // Address of KSEcatDcParams structure
                                            0);                                                                        // Flags, here none
            if (ksError != KS_OK) return ksError;

            ksError = KS_setEcatPdoAssign(m_ExchangeMemory->m_EtherCATConfigData.m_HandleEtherCatSlave[IndexSlave], KS_ECAT_SYNC_INPUT, -1, 0);  // reset
            if (ksError == KS_OK) {
                ksError = KS_setEcatPdoAssign(m_ExchangeMemory->m_EtherCATConfigData.m_HandleEtherCatSlave[IndexSlave], KS_ECAT_SYNC_INPUT, 0x1A00, 0);
                if (ksError == KS_OK) {
                    ksError = KS_setEcatPdoAssign(m_ExchangeMemory->m_EtherCATConfigData.m_HandleEtherCatSlave[IndexSlave], KS_ECAT_SYNC_INPUT, 0x1A03, 0);
                    if (ksError == KS_OK) {
                        ksError = KS_setEcatPdoAssign(m_ExchangeMemory->m_EtherCATConfigData.m_HandleEtherCatSlave[IndexSlave], KS_ECAT_SYNC_INPUT, 0x1A04, 0);
                    }
                    /*ksError = KS_changeEcatSlaveState(m_ExchangeMemory->m_EtherCATConfigData.m_HandleEtherCatSlave[IndexSlave], KS_ECAT_STATE_SAFEOP, 0);
                    ksError = KS_changeEcatState(m_ExchangeMemory->m_EtherCATConfigData.m_HandleEtherCatSlave[IndexSlave], KS_ECAT_STATE_OP, 0);*/
                }
            }
        }
    }
    if (ksError != KS_OK) {
        QString AddErrorMsg = tr("StepperMotorSetPDOAssignVelocity");
        rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
    }
    return rv;
}

bool KitharaCore::ConnectToEcatMaster()
{
    bool ConnectionOk = false;

    if (!m_WorkWithoutEtherCat) {
        KSError ksError = KS_OK;
        KSEcatMasterState masterState;
        int ConnectionCounter = 0;

        masterState.structSize = sizeof(KSEcatMasterState);
        while (true) {
            ksError = KS_queryEcatMasterState(m_ExchangeMemory->m_EtherCATConfigData.m_HandleEtherCatMaster, &masterState, 0);
            if (ksError != KS_OK)
                break;
            else {
                if (masterState.connected) {
                    ConnectionOk = true;
                    break;
                } else
                    Sleep(500);
            }
            ConnectionCounter++;
            if (ConnectionCounter > 5) break;
        }
    } else
        ConnectionOk = true;  // Hier Simulation
    return ConnectionOk;
}

bool KitharaCore::IsEtherCATMasterConnected()
{
    if (m_ExchangeMemory && m_ExchangeMemory->m_EtherCATConfigData.m_HandleEtherCatMaster != KS_INVALID_HANDLE) {
        KSEcatMasterState masterState;
        if (KS_queryEcatMasterState(m_ExchangeMemory->m_EtherCATConfigData.m_HandleEtherCatMaster, &masterState, 0) == KS_OK) {
            if (masterState.connected)
                return true;
            else
                return false;
        } else
            return false;
    } else
        return false;
}

void KitharaCore::ReadDesignBusTerminals()
{
    QString IniFileLocation = GetMainAppCrystalT2()->GetXMLCofigurationFileLocation() + QString("/") + FILE_NAME_ETHERCAT_BUS_TERMINALS + ".ini";
    QString Name, AccessName;
    QSettings settings(IniFileLocation, QSettings::IniFormat);
    QStringList keys = settings.childGroups();
    QString IgnorSection = "ignor";
    bool ok;

    m_ListEtherCatSlaveData.clear();
    for (int i = 0; i < keys.count(); i++) {
        if (keys[i].toLower() != IgnorSection.toLower()) {
            EtherCatSlaveData SlaveData;
            settings.beginGroup(keys[i]);
            SlaveData.m_TerminalType = settings.value("TerminalType", "").toString();
            SlaveData.m_Position = settings.value("Position", 0).toInt();
            SlaveData.m_VendorID = settings.value("VendorID", 0).toInt();
            SlaveData.m_ProductID = settings.value("ProductID", 0).toString().toInt(&ok, 16);
            SlaveData.m_TerminalDisplayName = keys[i];
            SlaveData.m_NumberChannels = settings.value("NumberChannels", 0).toInt();
            SlaveData.m_FirstByte = settings.value("FirstByte", 0).toInt();
            SlaveData.m_NumberBitsPerChannel = settings.value("BitsPerChannel", 0).toInt();
            for (int j = 0; j < SlaveData.m_NumberChannels; j++) {
                Name = QString("Channel_%1_DisplayName").arg(j + 1);
                AccessName = settings.value(Name, "").toString();
                if (!AccessName.isEmpty()) SlaveData.m_ListChannelDisplayNames.append(AccessName);

                Name = QString("Channel_%1_AccessName").arg(j + 1);
                AccessName = settings.value(Name, "").toString();
                if (!AccessName.isEmpty()) SlaveData.m_ListChannelNumbers.insert(AccessName, j);  // m_ListAccessNames.append(Value);
            }
            settings.endGroup();
            m_ListEtherCatSlaveData.append(SlaveData);
        }
    }
    qSort(m_ListEtherCatSlaveData.begin(), m_ListEtherCatSlaveData.end(), KitharaCore::PositionLessThan);
    if (m_ExchangeMemory) {
        for (int TerminalIndex = 0; TerminalIndex < m_ListEtherCatSlaveData.count(); TerminalIndex++) {
            m_ExchangeMemory->m_EtherCATConfigData.m_IOTerminals[TerminalIndex].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
            m_ExchangeMemory->m_EtherCATConfigData.m_IOTerminals[TerminalIndex].m_LenghtInBytes =
                (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
            if (m_ListEtherCatSlaveData.at(TerminalIndex).m_TerminalType == QString("DigitalOutput") || m_ListEtherCatSlaveData.at(TerminalIndex).m_TerminalType == QString("AnalogOutput"))
                m_ExchangeMemory->m_EtherCATConfigData.m_IOTerminals[TerminalIndex].m_IsInput = false;  // is an output terminal
            else {
                if (m_ListEtherCatSlaveData.at(TerminalIndex).m_TerminalType == QString("StepperMotor")) {
                    m_ExchangeMemory->m_EtherCATConfigData.m_IOTerminals[TerminalIndex].m_IsMotor = true;
                } else {
                    m_ExchangeMemory->m_EtherCATConfigData.m_IOTerminals[TerminalIndex].m_IsInput = true;
                }
            }
        }
        ConfigureDigitalOutputs();
        ConfigureDigitalInputs();
        ConfigureAnalogOutputs();
        ConfigureAnalogInputs();
    }
}

/*DO_ERROR_LIGHT
  DO_ERROR_TRANSFER
  DO_COUNTER_EJECTION_TRANSFER
  DO_PREASURE_TANK_HEATER
  DO_HEATING_PIPE
  DO_CAMERA_LIGHT
  DO_VALVE_CONTROLLER
  DO_BOTTLE_EJECTION
  DO_TRIGGER1_VALVE,
  DO_TRIGGER2_VALVE
  DO_WHITE_LIGHT
  DO_BLUE_LIGHT
  DO_PREASURE_TANK_VALVE
  DO_EJECTION_BY_IS_MASCHINE
  */
void KitharaCore::ConfigureDigitalOutputs()
{
    QString TerminalTypeName = "DigitalOutput";
    int TerminalIndex, BitPosition;

    TerminalIndex = GetDigitalIOIndex(TerminalTypeName, QString(DO_NAME_ERROR_LIGHT), BitPosition);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_ERROR_LIGHT].m_ChannelNumber = BitPosition;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_ERROR_LIGHT].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_ERROR_LIGHT].m_IsInput = false;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_ERROR_LIGHT].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetDigitalIOIndex(TerminalTypeName, QString(DO_NAME_ERROR_TRANSFER), BitPosition);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_ERROR_TRANSFER].m_ChannelNumber = BitPosition;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_ERROR_TRANSFER].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_ERROR_TRANSFER].m_IsInput = false;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_ERROR_TRANSFER].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetDigitalIOIndex(TerminalTypeName, QString(DO_NAME_COUNTER_EJECTION_TRANSFER), BitPosition);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_COUNTER_EJECTION_TRANSFER].m_ChannelNumber = BitPosition;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_COUNTER_EJECTION_TRANSFER].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_COUNTER_EJECTION_TRANSFER].m_IsInput = false;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_COUNTER_EJECTION_TRANSFER].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetDigitalIOIndex(TerminalTypeName, QString(DO_NAME_PREASURE_TANK_HEATER), BitPosition);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_PREASURE_TANK_HEATER].m_ChannelNumber = BitPosition;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_PREASURE_TANK_HEATER].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_PREASURE_TANK_HEATER].m_IsInput = false;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_PREASURE_TANK_HEATER].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetDigitalIOIndex(TerminalTypeName, QString(DO_NAME_HEATING_PIPE), BitPosition);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_HEATING_PIPE].m_ChannelNumber = BitPosition;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_HEATING_PIPE].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_HEATING_PIPE].m_IsInput = false;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_HEATING_PIPE].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetDigitalIOIndex(TerminalTypeName, QString(DO_NAME_CAMERA_LIGHT), BitPosition);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_CAMERA_LIGHT].m_ChannelNumber = BitPosition;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_CAMERA_LIGHT].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_CAMERA_LIGHT].m_IsInput = false;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_CAMERA_LIGHT].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetDigitalIOIndex(TerminalTypeName, QString(DO_NAME_VALVE_CONTROLLER), BitPosition);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_VALVE_CONTROLLER].m_ChannelNumber = BitPosition;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_VALVE_CONTROLLER].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_VALVE_CONTROLLER].m_IsInput = false;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_VALVE_CONTROLLER].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetDigitalIOIndex(TerminalTypeName, QString(DO_NAME_BOTTLE_EJECTION), BitPosition);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_BOTTLE_EJECTION].m_ChannelNumber = BitPosition;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_BOTTLE_EJECTION].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_BOTTLE_EJECTION].m_IsInput = false;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_BOTTLE_EJECTION].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetDigitalIOIndex(TerminalTypeName, QString(DO_NAME_TRIGGER1_VALVE), BitPosition);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_TRIGGER1_VALVE].m_ChannelNumber = BitPosition;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_TRIGGER1_VALVE].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_TRIGGER1_VALVE].m_IsInput = false;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_TRIGGER1_VALVE].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetDigitalIOIndex(TerminalTypeName, QString(DO_NAME_TRIGGER2_VALVE), BitPosition);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_TRIGGER2_VALVE].m_ChannelNumber = BitPosition;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_TRIGGER2_VALVE].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_TRIGGER2_VALVE].m_IsInput = false;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_TRIGGER2_VALVE].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetDigitalIOIndex(TerminalTypeName, QString(DO_NAME_WHITE_LIGHT), BitPosition);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_WHITE_LIGHT].m_ChannelNumber = BitPosition;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_WHITE_LIGHT].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_WHITE_LIGHT].m_IsInput = false;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_WHITE_LIGHT].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetDigitalIOIndex(TerminalTypeName, QString(DO_NAME_BLUE_LIGHT), BitPosition);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_BLUE_LIGHT].m_ChannelNumber = BitPosition;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_BLUE_LIGHT].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_BLUE_LIGHT].m_IsInput = false;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_BLUE_LIGHT].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetDigitalIOIndex(TerminalTypeName, QString(DO_NAME_PREASURE_TANK_VALVE), BitPosition);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_PREASURE_TANK_VALVE].m_ChannelNumber = BitPosition;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_PREASURE_TANK_VALVE].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_PREASURE_TANK_VALVE].m_IsInput = false;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_PREASURE_TANK_VALVE].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetDigitalIOIndex(TerminalTypeName, QString(DO_NAME_EJECTION_BY_IS_MASCHINE), BitPosition);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_EJECTION_BY_IS_MASCHINE].m_ChannelNumber = BitPosition;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_EJECTION_BY_IS_MASCHINE].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_EJECTION_BY_IS_MASCHINE].m_IsInput = false;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_EJECTION_BY_IS_MASCHINE].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetDigitalIOIndex(TerminalTypeName, QString(DO_NAME_ORANGE_LIGHT), BitPosition);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_ORANGE_LIGHT].m_ChannelNumber = BitPosition;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_ORANGE_LIGHT].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_ORANGE_LIGHT].m_IsInput = false;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DO_ORANGE_LIGHT].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
}

// DI_CONTROL_VOLTAGE
// DI_CLOCK_SIGNAL_FROM_IS
// DI_EJECTION_CONTROL
void KitharaCore::ConfigureDigitalInputs()
{
    QString TerminalTypeName = "DigitalInput";
    int ChannelIndex;
    /////////////////   Start Beckhoffklemme EL1008  Schaltplan -> K10 Klemme Nummer 1  am Buskoppler/////////////////
    int TerminalIndex = GetDigitalIOIndex(TerminalTypeName, QString(DI_NAME_CONTROL_VOLTAGE), ChannelIndex);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DI_CONTROL_VOLTAGE].m_ChannelNumber = ChannelIndex;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DI_CONTROL_VOLTAGE].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DI_CONTROL_VOLTAGE].m_IsInput = true;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DI_CONTROL_VOLTAGE].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetDigitalIOIndex(TerminalTypeName, QString(DI_NAME_CLOCK_SIGNAL_FROM_IS), ChannelIndex);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DI_CLOCK_SIGNAL_FROM_IS].m_ChannelNumber = ChannelIndex;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DI_CLOCK_SIGNAL_FROM_IS].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DI_CLOCK_SIGNAL_FROM_IS].m_IsInput = true;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DI_CLOCK_SIGNAL_FROM_IS].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetDigitalIOIndex(TerminalTypeName, QString(DI_NAME_EJECTION_CONTROL), ChannelIndex);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DI_EJECTION_CONTROL].m_ChannelNumber = ChannelIndex;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DI_EJECTION_CONTROL].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DI_EJECTION_CONTROL].m_IsInput = true;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::DI_EJECTION_CONTROL].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    /////////////////   Ende Beckhoffklemme EL1008  Schaltplan -> K10 Klemme Nummer 1  /////////////////
}

/*AO_SET_POINT_PREASURE_VALUE
AO_SET_POINT_WATER_COOLING_SENSOR
AO_SET_POINT_AIR_COOLING_CAMERA
AO_SET_POINT_AIR_COOLING_LIGHT
AO_SET_POINT_AIR_COOLING_VALVES
AO_SET_POINT_WATER_COOLING
*/
void KitharaCore::ConfigureAnalogOutputs()
{
    QString TerminalTypeName = "AnalogOutput";
    int ChannelOffsetInBytes;
    int TerminalIndex = GetAnalogIOIndex(TerminalTypeName, QString(AO_NAME_SET_POINT_PREASURE_VALUE), ChannelOffsetInBytes);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AO_SET_POINT_PREASURE_VALUE].m_ChannelNumber = ChannelOffsetInBytes;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AO_SET_POINT_PREASURE_VALUE].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AO_SET_POINT_PREASURE_VALUE].m_IsInput = false;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AO_SET_POINT_PREASURE_VALUE].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetAnalogIOIndex(TerminalTypeName, QString(AO_NAME_SET_POINT_WATER_COOLING_SENSOR), ChannelOffsetInBytes);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AO_SET_POINT_WATER_COOLING_SENSOR].m_ChannelNumber = ChannelOffsetInBytes;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AO_SET_POINT_WATER_COOLING_SENSOR].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AO_SET_POINT_WATER_COOLING_SENSOR].m_IsInput = false;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AO_SET_POINT_WATER_COOLING_SENSOR].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetAnalogIOIndex(TerminalTypeName, QString(AO_NAME_SET_POINT_AIR_COOLING_CAMERA_AND_BACK_LIGHT), ChannelOffsetInBytes);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AO_SET_POINT_AIR_COOLING_CAMERA].m_ChannelNumber = ChannelOffsetInBytes;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AO_SET_POINT_AIR_COOLING_CAMERA].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AO_SET_POINT_AIR_COOLING_CAMERA].m_IsInput = false;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AO_SET_POINT_AIR_COOLING_CAMERA].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetAnalogIOIndex(TerminalTypeName, QString(AO_NAME_SET_POINT_AIR_COOLING_GLASS), ChannelOffsetInBytes);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AO_SET_POINT_AIR_COOLING_LIGHT].m_ChannelNumber = ChannelOffsetInBytes;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AO_SET_POINT_AIR_COOLING_LIGHT].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AO_SET_POINT_AIR_COOLING_LIGHT].m_IsInput = false;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AO_SET_POINT_AIR_COOLING_LIGHT].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetAnalogIOIndex(TerminalTypeName, QString(AO_NAME_SET_POINT_AIR_COOLING_VALVES), ChannelOffsetInBytes);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AO_SET_POINT_AIR_COOLING_VALVES].m_ChannelNumber = ChannelOffsetInBytes;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AO_SET_POINT_AIR_COOLING_VALVES].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AO_SET_POINT_AIR_COOLING_VALVES].m_IsInput = false;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AO_SET_POINT_AIR_COOLING_VALVES].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetAnalogIOIndex(TerminalTypeName, QString(AO_NAME_SET_POINT_WATER_COOLING), ChannelOffsetInBytes);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AO_SET_POINT_WATER_COOLING].m_ChannelNumber = ChannelOffsetInBytes;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AO_SET_POINT_WATER_COOLING].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AO_SET_POINT_WATER_COOLING].m_IsInput = false;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AO_SET_POINT_WATER_COOLING].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
}

/*    AI_TANK_TEMPERATURE
     AI_HEATING_PIPE_TEMPERATURE
     AI_ACTUAL_AIR_COOLING_CAMERA
     AI_ACTUAL_AIR_COOLING_CAMERA_LIGHT
     AI_ACTUAL_AIR_COOLING_VALVE
     AI_ACTUAL_WATER_COOLING
     AI_ACTUAL_FLOW_TRANSMITTER_WATER_COOLING_CIRCUIT
     AI_ACTUAL_TEMPERATURE_WATER_COOLING_RETURN
     AI_ACTUAL_PREASURE_VALUE
     AI_PREASURE_TANK_FILLING_LEVEL_VALUE*/
void KitharaCore::ConfigureAnalogInputs()
{
    QString TerminalTypeName = "AnalogInput";
    int ChannelOffsetInBytes;

    int TerminalIndex = GetAnalogIOIndex(TerminalTypeName, QString(AI_NAME_TANK_TEMPERATURE), ChannelOffsetInBytes);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_TANK_TEMPERATURE].m_ChannelNumber = ChannelOffsetInBytes;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_TANK_TEMPERATURE].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_TANK_TEMPERATURE].m_IsInput = true;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_TANK_TEMPERATURE].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetAnalogIOIndex(TerminalTypeName, QString(AI_NAME_HEATING_PIPE_TEMPERATURE), ChannelOffsetInBytes);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_HEATING_PIPE_TEMPERATURE].m_ChannelNumber = ChannelOffsetInBytes;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_HEATING_PIPE_TEMPERATURE].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_HEATING_PIPE_TEMPERATURE].m_IsInput = true;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_HEATING_PIPE_TEMPERATURE].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetAnalogIOIndex(TerminalTypeName, QString(AI_NAME_ACTUAL_AIR_COOLING_CAMERA_AND_BACK_LIGHT), ChannelOffsetInBytes);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_AIR_COOLING_CAMERA].m_ChannelNumber = ChannelOffsetInBytes;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_AIR_COOLING_CAMERA].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_AIR_COOLING_CAMERA].m_IsInput = true;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_AIR_COOLING_CAMERA].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetAnalogIOIndex(TerminalTypeName, QString(AI_NAME_ACTUAL_AIR_COOLING_GLASS), ChannelOffsetInBytes);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_AIR_COOLING_CAMERA_LIGHT].m_ChannelNumber = ChannelOffsetInBytes;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_AIR_COOLING_CAMERA_LIGHT].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_AIR_COOLING_CAMERA_LIGHT].m_IsInput = true;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_AIR_COOLING_CAMERA_LIGHT].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetAnalogIOIndex(TerminalTypeName, QString(AI_NAME_ACTUAL_AIR_COOLING_VALVE), ChannelOffsetInBytes);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_AIR_COOLING_VALVE].m_ChannelNumber = ChannelOffsetInBytes;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_AIR_COOLING_VALVE].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_AIR_COOLING_VALVE].m_IsInput = true;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_AIR_COOLING_VALVE].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetAnalogIOIndex(TerminalTypeName, QString(AI_NAME_ACTUAL_WATER_COOLING), ChannelOffsetInBytes);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_WATER_COOLING].m_ChannelNumber = ChannelOffsetInBytes;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_WATER_COOLING].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_WATER_COOLING].m_IsInput = true;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_WATER_COOLING].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetAnalogIOIndex(TerminalTypeName, QString(AI_NAME_ACTUAL_FLOW_TRANSMITTER_WATER_COOLING_CIRCUIT), ChannelOffsetInBytes);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_FLOW_TRANSMITTER_WATER_COOLING_CIRCUIT].m_ChannelNumber = ChannelOffsetInBytes;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_FLOW_TRANSMITTER_WATER_COOLING_CIRCUIT].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_FLOW_TRANSMITTER_WATER_COOLING_CIRCUIT].m_IsInput = true;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_FLOW_TRANSMITTER_WATER_COOLING_CIRCUIT].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetAnalogIOIndex(TerminalTypeName, QString(AI_NAME_ACTUAL_TEMPERATURE_WATER_COOLING_RETURN), ChannelOffsetInBytes);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_TEMPERATURE_WATER_COOLING_RETURN].m_ChannelNumber = ChannelOffsetInBytes;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_TEMPERATURE_WATER_COOLING_RETURN].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_TEMPERATURE_WATER_COOLING_RETURN].m_IsInput = true;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_TEMPERATURE_WATER_COOLING_RETURN].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetAnalogIOIndex(TerminalTypeName, QString(AI_NAME_ACTUAL_PREASURE), ChannelOffsetInBytes);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_PREASURE_VALUE].m_ChannelNumber = ChannelOffsetInBytes;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_PREASURE_VALUE].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_PREASURE_VALUE].m_IsInput = true;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_ACTUAL_PREASURE_VALUE].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
    TerminalIndex = GetAnalogIOIndex(TerminalTypeName, QString(AI_NAME_ACTUAL_TANK_FILLING_LEVEL), ChannelOffsetInBytes);
    if (TerminalIndex >= 0) {
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_PREASURE_TANK_FILLING_LEVEL_VALUE].m_ChannelNumber = ChannelOffsetInBytes;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_PREASURE_TANK_FILLING_LEVEL_VALUE].m_FirstByte = m_ListEtherCatSlaveData.at(TerminalIndex).m_FirstByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_PREASURE_TANK_FILLING_LEVEL_VALUE].m_IsInput = true;
        m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[EtherCATConfigData::AI_PREASURE_TANK_FILLING_LEVEL_VALUE].m_LengthInByte =
            (m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberChannels * m_ListEtherCatSlaveData.at(TerminalIndex).m_NumberBitsPerChannel) / 8;
    }
}

bool KitharaCore::PositionLessThan(EtherCatSlaveData& SlaveData1, EtherCatSlaveData& SlaveData2)
{
    return (SlaveData1.m_Position < SlaveData2.m_Position);
}

int KitharaCore::FreeEtherCatMasterAndSlave()
{
    int rv = ERROR_CODE_NO_ERROR;
    QString ErrorMsg, AddMsg;
    KSError ksError = KS_OK;

    // Leaving OP with slave
    /*for (int i = 0; i < m_ExchangeMemory->m_EtherCATConfigData.m_NumberSlaves; i++) {
        if (m_ExchangeMemory->m_EtherCATConfigData.m_HandleEtherCatSlave[i] != KS_INVALID_HANDLE) {
            ksError = KS_changeEcatState(m_ExchangeMemory->m_EtherCATConfigData.m_HandleEtherCatSlave[i], KS_ECAT_STATE_INIT, 0);
            if (ksError != KS_OK) {
                AddMsg = tr("KS_changeEcatState Position:%1 Leaving OP with slave failed").arg(i + 1);
                rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddMsg);
            }
        }
    }*/

    for (int i = 0; i < m_ExchangeMemory->m_EtherCATConfigData.m_NumberSlaves; i++) {
        if (m_ExchangeMemory->m_EtherCATConfigData.m_HandleEtherCatSlave[i] != KS_INVALID_HANDLE) {
            ksError = KS_deleteEcatSlave(m_ExchangeMemory->m_EtherCATConfigData.m_HandleEtherCatSlave[i]);
            if (ksError != KS_OK) {
                AddMsg = tr("KS_deleteEcatSlave Position:%1 Failed").arg(i + 1);
                rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddMsg);
            }
        }
    }
    if (m_ExchangeMemory->m_EtherCATConfigData.m_HandleEtherCatMaster != KS_INVALID_HANDLE) {
        ksError = KS_closeEcatMaster(m_ExchangeMemory->m_EtherCATConfigData.m_HandleEtherCatMaster);
        if (ksError != KS_OK) {
            AddMsg = tr("KS_closeEcatMaster Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddMsg);
        }
    }
    if (m_ExchangeMemory->m_EtherCATConfigData.m_HandleNetworkAdapter != KS_INVALID_HANDLE) {
        ksError = KS_closeNetwork(m_ExchangeMemory->m_EtherCATConfigData.m_HandleNetworkAdapter, 0);
        if (ksError != KS_OK) {
            AddMsg = tr("KS_closeNetwork Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddMsg);
        }
    }
    return rv;
}

int KitharaCore::GetKitharaErrorMsg(int ksError, QString& ErrorMsg, QString& AdditionalErrorMsg)
{
    const char* pMsg;
    if (KS_getErrorString((KSError)(ksError), &pMsg, KSLNG_DEFAULT) == KS_OK)
        ErrorMsg = tr("Error! Code: 0x%1 Text: %2 Info: %3").arg(ksError, 0, 16).arg(pMsg).arg(AdditionalErrorMsg);
    else
        ErrorMsg = tr("Error! KS_getErrorString %1").arg(AdditionalErrorMsg);
    SendMessageToGUI(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
    return ERROR_CODE_ANY_ERROR;
}

QString KitharaCore::GetKitharaVersion()
{
    char pMsg[128];
    int lenght = 128;

    KSError ksError = KS_getDriverVersionAsString(pMsg, &lenght, 0);
    if (ksError == KS_OK)
        return QString("%1").arg(pMsg);
    else
        return QString();
}

int KitharaCore::SetCameraViewPort(int offsetX, int offsetY, int width, int height, QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    KSError ksError = KS_OK;
    int64 i64widthMax, i64heightMax, i64offsetX, i64offsetY, i64height, i64width;
    int64 i64IncrementWidth, i64IncrementHeight;
    int64 ROITemp = 0;
    int64 diff;
    QString AddErrorMsg;
    QRect NewRect;

    if (m_ExchangeMemory->m_HandleCamera != KS_INVALID_HANDLE) {
        ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_GET, "WidthMax", 0, &i64widthMax, 0, 0);
        if (ksError == KS_OK) {
            ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_GET, "HeightMax", 0, &i64heightMax, 0, 0);
            if (ksError == KS_OK) {
                m_MaxImageWidth = static_cast<int>(i64widthMax);
                m_MaxImageHeight = static_cast<int>(i64heightMax);
                // first set ROI offset data to zero before set new values
                ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_SET, "OffsetX", 0, &ROITemp, 0, 0);
                if (ksError != KS_OK) {
                    GetKitharaErrorMsg(ksError, ErrorMsg, QString("Can not set Offsetx to zero"));
                    qDebug() << ErrorMsg;
                }
                ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_SET, "OffsetY", 0, &ROITemp, 0, 0);
                if (ksError != KS_OK) {
                    GetKitharaErrorMsg(ksError, ErrorMsg, QString("Can not set Offsety to zero"));
                    qDebug() << ErrorMsg;
                }
                CheckCameraROIPosition(offsetX, offsetY, width, height, m_MaxImageWidth, m_MaxImageHeight);
                NewRect.setX(static_cast<int>(offsetX));
                NewRect.setY(static_cast<int>(offsetY));
                NewRect.setWidth(static_cast<int>(width));    // Bildbreite und Bildhöhe sind immer fix egal welche Kameraauflösung verfügbar ist
                NewRect.setHeight(static_cast<int>(height));  // Ist die Auflösung größer als die Vorgegebene kann der Kamera-ROI in der Oberfläche verschoben werden
                GetImageData()->SetMeasureWindowRect(ROI_ID_CAMERA, NewRect);
                if (width < static_cast<int>(i64widthMax) || height < static_cast<int>(i64heightMax)) {
                    m_IsCameraResolutionGreaterThanDefaultResolution = true;  // dann kann in der Bedienoberfläche der Kamera ROI verschoben werden
                } else {
                    m_IsCameraResolutionGreaterThanDefaultResolution = false;
                }
                i64offsetX = offsetX;
                i64offsetY = offsetY;
                i64height = height;
                i64width = width;

                qDebug() << QString("offx: %1 offy: %2 w: %3 h: %4 wmax: %5 hmax: %6").arg(i64offsetX).arg(i64offsetY).arg(i64width).arg(i64height).arg(m_MaxImageWidth).arg(m_MaxImageHeight);

                rv = WorkaroundSetCameraWidht(width, ErrorMsg);
                // ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_SET, "Width", 0, &i64width, 0, 0);
                if (rv == ERROR_CODE_NO_ERROR) {
                    ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_SET, "OffsetX", 0, &i64offsetX, 0, 0);
                    if (ksError == KS_OK) {
                        ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_SET, "Height", 0, &i64height, 0, 0);
                        if (ksError == KS_OK) {
                            ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_SET, "OffsetY", 0, &i64offsetY, 0, 0);
                            if (ksError != KS_OK) {
                                AddErrorMsg = tr("Set Camera OffsetY Failed");
                                rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                            }
                        } else {
                            AddErrorMsg = tr("Set Camera Height Failed");
                            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                        }
                    } else {
                        AddErrorMsg = tr("Set Camera OffsetX Failed");
                        rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                    }
                }
                /* else {
                     AddErrorMsg = tr("Set Camera Width Failed");
                     rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                 }*/
            } else {
                AddErrorMsg = tr("Get Camera Max Height Failed");
                rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
            }
        } else {
            AddErrorMsg = tr("Get Camera Max Width Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
    } else {
        if (!m_WorkWithoutCamera) {
            ErrorMsg = tr("Can Not Set Camera ROI, No Camera Connection");
            rv = ERROR_CODE_ANY_ERROR;
        } else {
            if (m_PointerVideoBlockCameraSimulation && GetImageData()) {
                // Hier nur Simulation wenn im Videospeicher die Bilder Größer sind als die vorgegebene Auflösung
                VideoHeader videoHeader;
                memcpy(&videoHeader, m_PointerVideoBlockCameraSimulation, sizeof(VideoHeader));
                CheckCameraROIPosition(offsetX, offsetY, width, height, videoHeader.m_ImageWidth, videoHeader.m_ImageHeight);
                NewRect.setX(static_cast<int>(offsetX));
                NewRect.setY(static_cast<int>(offsetY));
                NewRect.setWidth(static_cast<int>(width));
                NewRect.setHeight(static_cast<int>(height));
                GetImageData()->SetMeasureWindowRect(ROI_ID_CAMERA, NewRect);
            }
        }
    }
    return rv;
}

int KitharaCore::WorkaroundSetCameraWidht(int width, QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    int64 i64width = width;
    QString AddErrorMsg;

    if (m_IsCameraResolutionGreaterThanDefaultResolution) {
        // if (GetMainAppCrystalT2()->GetSettingsData()->m_UseCameraHighRes) {
        KSError ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_SET, "convWidth", 0, &i64width, 0, 0);
        if (ksError == KS_OK) {
            ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_SET, "convCopyToAutoFeatureWidthBrightnessAuto", 0, &i64width, 0, 0);
            if (ksError == KS_OK) {
                ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_SET, "convCopyToAutoFeatureWidthAWB", 0, &i64width, 0, 0);
                if (ksError != KS_OK) {
                    AddErrorMsg = tr("Set Camera convCopyToAutoFeatureWidthAWB Failed");
                    rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                }
            } else {
                AddErrorMsg = tr("Set Camera convCopyToAutoFeatureWidthBrightnessAuto Failed");
                rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
            }
        } else {
            AddErrorMsg = tr("Set Camera convWidth Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
    } else {
        KSError ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_SET, "Width", 0, &i64width, 0, 0);
        if (ksError != KS_OK) {
            AddErrorMsg = tr("Set Camera convCopyToAutoFeatureWidthAWB Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
    }
    return rv;
}

int KitharaCore::SetCameraXOffset(int newOffsetX, QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    int64 i64offsetX;
    QRect NewCameraROI;
    QRect cameraROI = GetImageData()->GetMeasureWindowRect(ROI_ID_CAMERA);
    int offsetX = cameraROI.x();
    int offsetY = cameraROI.y();
    int width = cameraROI.width();
    int height = cameraROI.height();

    if (m_ExchangeMemory->m_HandleCamera != KS_INVALID_HANDLE) {
        CheckCameraROIPosition(newOffsetX, offsetY, width, height, m_MaxImageWidth, m_MaxImageHeight);
        i64offsetX = newOffsetX;
        KSError ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_SET, "OffsetX", 0, &i64offsetX, 0, 0);
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("Set Camera OffsetX Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        } else {
            NewCameraROI.setX(static_cast<int>(newOffsetX));
            NewCameraROI.setY(static_cast<int>(offsetY));
            NewCameraROI.setWidth(static_cast<int>(width));  // Bildbreite und Bildhöhe sind immer fix egal welche Kameraauflösung verfügbar ist
            NewCameraROI.setHeight(static_cast<int>(height));
            GetImageData()->SetMeasureWindowRect(ROI_ID_CAMERA, NewCameraROI);
        }
    }
    return rv;
}

int KitharaCore::SetCameraYOffset(int newOffsetY, QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    int64 i64offsetY;
    QRect NewCameraROI;
    QRect cameraROI = GetImageData()->GetMeasureWindowRect(ROI_ID_CAMERA);
    int offsetX = cameraROI.x();
    int offsetY = cameraROI.y();
    int width = cameraROI.width();
    int height = cameraROI.height();

    if (m_ExchangeMemory->m_HandleCamera != KS_INVALID_HANDLE) {
        CheckCameraROIPosition(offsetX, newOffsetY, width, height, m_MaxImageWidth, m_MaxImageHeight);
        i64offsetY = newOffsetY;
        KSError ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_SET, "OffsetY", 0, &i64offsetY, 0, 0);
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("Set Camera OffsetY Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        } else {
            NewCameraROI.setX(static_cast<int>(offsetX));
            NewCameraROI.setY(static_cast<int>(newOffsetY));
            NewCameraROI.setWidth(static_cast<int>(width));  // Bildbreite und Bildhöhe sind immer fix egal welche Kameraauflösung verfügbar ist
            NewCameraROI.setHeight(static_cast<int>(height));
            GetImageData()->SetMeasureWindowRect(ROI_ID_CAMERA, NewCameraROI);
        }
    }
    return rv;
}

bool KitharaCore::CheckCameraROIPosition(int& offsetX, int& offsetY, int& width, int& height, int widthMax, int heightMax)
{
    bool ROIcorrection = false;
    if (offsetX < 0) {
        offsetX = 0;
        ROIcorrection = true;
    }
    if (offsetY < 0) {
        offsetY = 0;
        ROIcorrection = true;
    }
    // tritt nie auf da eine Auflösung die geringer als 640 x 480 nie benutzt wird
    if (width > widthMax) {
        int64 diff = width - widthMax;
        width = width - diff;
        ROIcorrection = true;
    }
    if (height > heightMax) {
        int64 diff = height - heightMax;
        height = height - diff;
        ROIcorrection = true;
    }

    if ((offsetX + width) > widthMax) {
        int64 diff = (offsetX + width) - widthMax;
        offsetX = offsetX - diff;
        ROIcorrection = true;
    }
    if ((offsetY + height) > heightMax) {
        int64 diff = (offsetY + height) - heightMax;
        offsetY = offsetY - diff;
        ROIcorrection = true;
    }
    return ROIcorrection;
}

int KitharaCore::_ConnectAndConfigureCamera(QString& CameraHardWareID)
{
    int rv = ERROR_CODE_NO_ERROR;
    double ExposureTimeInmus = 1000.0;
    QString CameraStartUpInfo, AddErrorMsg, ErrorMsg;
    KSError ksError = KS_OK;
    int OffsetX, OffsetY, width, height;

    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        ExposureTimeInmus = GetMainAppCrystalT2()->GetSettingsData()->m_ExposureTime;
    }
    if (m_ExchangeMemory && m_ExchangeMemory->m_HandleController != KS_INVALID_HANDLE) {
        rv = OpenRealTimeCamera(CameraHardWareID);
        if (rv == ERROR_CODE_NO_ERROR) {
            CameraStartUpInfo += tr("Info! KS_openCamera: OK");
            SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
            char PixelFormatName[] = "Mono8";
            ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_SET, "PixelFormat", 0, &PixelFormatName, 0, 0);
            if (ksError == KS_OK) {
                CameraStartUpInfo = tr("Info! KS_configCamera Set Camera PixelFormat: OK");
                SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                width = GetImageData()->GetImageWidth();
                height = GetImageData()->GetImageHeight();
                OffsetX = GetImageData()->GetImageOffsetX();
                OffsetY = GetImageData()->GetImageOffsetY();
                qDebug() << QString("offx: %1 offy: %2 w: %3 h: %4").arg(OffsetX).arg(OffsetY).arg(width).arg(height);

                rv = SetCameraViewPort(OffsetX, OffsetY, width, height, ErrorMsg);
                if (rv == ERROR_CODE_NO_ERROR) {
                    CameraStartUpInfo = tr("Info! Set Camera View Port: OK");
                    SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                    rv = SetFloatNodeCameraValue(QString("ExposureTime"), ExposureTimeInmus);
                    if (rv == ERROR_CODE_NO_ERROR) {
                        CameraStartUpInfo = tr("Info! Set Camera Exposure Time:%1: OK").arg(ExposureTimeInmus);
                        SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                        rv = SetInt64NodeCameraValue(QString("AcquisitionFrameRateEnable"), 0);  // Sicherstellen das die Framerate auf Maximum steht
                        if (rv == ERROR_CODE_NO_ERROR) {
                            CameraStartUpInfo = tr("Info! Disable Set Camera Framerate: OK");
                            SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                            char TriggerModeName[] = "Off";  // Sicherstellen das Trigger Mode auf off steht
                            ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_SET, "TriggerMode", 0, &TriggerModeName, 0, 0);
                            if (ksError != KS_OK) {
                                AddErrorMsg = tr("Set Camera Camera Trigger Mode -> Off");
                                rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                            }
                        }
                    }
                }

            } else {
                AddErrorMsg = tr("Set Camera Camera PixelFormat: %1 Failed").arg(PixelFormatName);
                rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
            }
        }
    } else {
        if (!m_ExchangeMemory) {
            ErrorMsg = tr("No Shared Data Allocated");
            SendMessageToGUI(ErrorMsg, QtMsgType::QtFatalMsg);
        }
        if (m_ExchangeMemory->m_HandleController == KS_INVALID_HANDLE) {
            ErrorMsg = tr("Can Not Open/Found XHCI Conroller");
            SendMessageToGUI(ErrorMsg, QtMsgType::QtFatalMsg);
        }
        rv = ERROR_CODE_ANY_ERROR;
    }

    return rv;
}

int KitharaCore::ConnectAndConfigureCamera(QString& CameraHardWareID)
{
    int rv = ERROR_CODE_NO_ERROR;
    double ExposureTimeInmus = 1000.0;
    QString CameraStartUpInfo, AddErrorMsg, ErrorMsg;
    char PixelFormatName[] = "Mono8";
    KSError ksError = KS_OK;
    int64 OffsetX, OffsetY, width, height, widthMax, heightMax;
    int64 ROITemp = 0;

    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        ExposureTimeInmus = GetMainAppCrystalT2()->GetSettingsData()->m_ExposureTime;
    }

    if (m_ExchangeMemory && m_ExchangeMemory->m_HandleController != KS_INVALID_HANDLE) {
        rv = OpenRealTimeCamera(CameraHardWareID);
        if (rv == 0) {
            CameraStartUpInfo += tr("Info! KS_openCamera: OK");
            SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
            ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_SET, "PixelFormat", 0, &PixelFormatName, 0, 0);
            if (ksError == KS_OK) {
                CameraStartUpInfo = tr("Info! KS_configCamera Set Camera Pixel Format: OK");
                SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_GET, "WidthMax", 0, &widthMax, 0, 0);
                if (ksError == KS_OK) {
                    CameraStartUpInfo = tr("Info! KS_configCamera Get Camera  Width Max: OK");
                    SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                    ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_GET, "HeightMax", 0, &heightMax, 0, 0);
                    if (ksError == KS_OK) {
                        CameraStartUpInfo = tr("Info! KS_configCamera Get Camera Height Max: OK");
                        SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                        m_MaxImageWidth = static_cast<int>(widthMax);
                        m_MaxImageHeight = static_cast<int>(heightMax);
                        if (GetImageData()->GetImageWidth() > widthMax || GetImageData()->GetImageHeight() > heightMax) {
                            QRect newCameraROI(0, 0, static_cast<int>(widthMax), static_cast<int>(heightMax));
                            GetImageData()->SetMeasureWindowRect(ROI_ID_CAMERA, newCameraROI);
                        }
                        width = GetImageData()->GetImageWidth();
                        height = GetImageData()->GetImageHeight();
                        OffsetX = GetImageData()->GetImageOffsetX();
                        OffsetY = GetImageData()->GetImageOffsetY();
                        // first set ROI offset data to zero before set new values
                        KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_SET, "OffsetX", 0, &ROITemp, 0, 0);
                        KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_SET, "OffsetY", 0, &ROITemp, 0, 0);

                        ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_SET, "Width", 0, &width, 0, 0);
                        if (ksError == KS_OK) {
                            CameraStartUpInfo = tr("Info! KS_configCamera Set Camera Width: OK");
                            SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                            ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_SET, "Height", 0, &height, 0, 0);
                            if (ksError == KS_OK) {
                                CameraStartUpInfo = tr("Info! KS_configCamera Set Camera Height: OK");
                                SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                                ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_SET, "OffsetX", 0, &OffsetX, 0, 0);
                                if (ksError == KS_OK) {
                                    CameraStartUpInfo = tr("Info! KS_configCamera Set Camera OffsetX: OK");
                                    SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                                    ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_SET, "OffsetY", 0, &OffsetY, 0, 0);
                                    if (ksError == KS_OK) {
                                        CameraStartUpInfo = tr("Info! KS_configCamera Set Camera OffsetY: OK");
                                        SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                                        rv = SetFloatNodeCameraValue(QString("ExposureTime"), ExposureTimeInmus);
                                        if (rv == 0) {
                                            CameraStartUpInfo = tr("Info! Set Camera Exposure Time:%1: OK").arg(ExposureTimeInmus);
                                            SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                                            rv = SetInt64NodeCameraValue(QString("AcquisitionFrameRateEnable"), 0);  // Sicherstellen das die Framerate auf maximum steht
                                            if (rv == 0) {
                                                CameraStartUpInfo = tr("Info! Disable Set Camera Framerate: OK");
                                                SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                                                // rv = SetInt64NodeCameraValue(QString("TriggerMode"),0);
                                            }
                                        }
                                    } else {
                                        AddErrorMsg = tr("Set Camera YOffset Failed");
                                        rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                                    }
                                } else {
                                    AddErrorMsg = tr("Set Camera XOffset Failed");
                                    rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                                }
                            } else {
                                AddErrorMsg = tr("Set Camera Height Failed");
                                rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                            }
                        } else {
                            AddErrorMsg = tr("Set Camera Width Failed");
                            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                        }
                    } else {
                        AddErrorMsg = tr("Get Camera Height Max Failed");
                        rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                    }
                } else {
                    AddErrorMsg = tr("Get Camera Width Max Failed");
                    rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                }
            } else {
                AddErrorMsg = tr("Set Camera Camera Pixel Format: %1 Failed").arg(PixelFormatName);
                rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
            }
        }
    } else {
        if (!m_ExchangeMemory) {
            ErrorMsg = tr("No Shared Data Allocated");
            SendMessageToGUI(ErrorMsg, QtMsgType::QtFatalMsg);
        }
        if (m_ExchangeMemory->m_HandleController == KS_INVALID_HANDLE) {
            ErrorMsg = tr("Can Not Open/Found XHCI Conroller");
            SendMessageToGUI(ErrorMsg, QtMsgType::QtFatalMsg);
        }
        rv = ERROR_CODE_ANY_ERROR;
    }
    return rv;
}

int KitharaCore::CloseRealTimeCamera()
{
    KSError ksError = KS_OK;
    QString ErrorMsg, AddErrorMsg;
    int rv = ERROR_CODE_NO_ERROR;

    if (m_ExchangeMemory->m_HandleCamera != KS_INVALID_HANDLE) {
        ksError = KS_closeCamera(m_ExchangeMemory->m_HandleCamera, 0);
        if (ksError != KS_OK) {
            AddErrorMsg = tr("KS_closeCamera Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
        m_ExchangeMemory->m_HandleCamera = KS_INVALID_HANDLE;
    }
    return rv;
}

int KitharaCore::CloseXHCIController()
{
    KSError ksError = KS_OK;
    QString ErrorMsg, AddErrorMsg;
    int rv = ERROR_CODE_NO_ERROR;

    if (m_ExchangeMemory->m_HandleController != KS_INVALID_HANDLE) {
        ksError = KS_closeXhci(m_ExchangeMemory->m_HandleController, 0);
        if (ksError != KS_OK) {
            AddErrorMsg = tr("KS_closeXhci Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
        m_ExchangeMemory->m_HandleController = KS_INVALID_HANDLE;
    }
    return rv;
}
// not in use
int KitharaCore::CloseNetworkAdapter()
{
    KSError ksError = KS_OK;
    QString ErrorMsg, AddErrorMsg;
    int rv = ERROR_CODE_NO_ERROR;

    if (m_ExchangeMemory->m_HandleController != KS_INVALID_HANDLE) {
        ksError = KS_closeNetwork(m_ExchangeMemory->m_HandleController, 0);
        if (ksError != KS_OK) {
            AddErrorMsg = tr("KS_closeNetwork Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
        m_ExchangeMemory->m_HandleController = KS_INVALID_HANDLE;
    }
    return rv;
}

int KitharaCore::CreateExchangeMemory(QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    QString AddErrorMsg;
    KSError ksError = KS_OK;
    ExchangeMemory* pSysPtr;

    ksError = KS_createSharedMem((void**)&m_ExchangeMemory, (void**)&pSysPtr, 0, sizeof(ExchangeMemory), 0);
    if (ksError == KS_OK) {
        m_ExchangeMemory->pAppPtr = m_ExchangeMemory;
        m_ExchangeMemory->pSysPtr = pSysPtr;
        m_ExchangeMemory->m_InfoCodeInputOutputDevice = INFO_CODE_INPUT_OUTPUT_DEVICE_NO_INFO;
#if _DEBUG
        m_ExchangeMemory->m_RealTimeModusOnRing0 = false;  // no real time task
#else
        m_ExchangeMemory->m_RealTimeModusOnRing0 = true;
#endif
        m_ExchangeMemory->m_CounterNumberBottlesRealEjected = 0;
        m_ExchangeMemory->m_DebugCounter = 0;
        m_ExchangeMemory->m_CounterCameraFrames = 0;
        m_ExchangeMemory->m_MaschineState = OPERATION_STATUS_STARTUP;
        m_ExchangeMemory->m_StopSimulation = true;
        m_ExchangeMemory->m_MeasuringTaskIsStarted = false;
        m_ExchangeMemory->m_IOTaskIsStatred = false;
        m_ExchangeMemory->m_CameraSimulationOn = false;
        m_ExchangeMemory->m_EtherCatSimulation = false;
        m_ExchangeMemory->m_AbortCameraSimulation = false;
        m_ExchangeMemory->m_LiquidFlowSimulationOn = false;
        m_ExchangeMemory->m_VideoStateCameraSimulation = STOP_VIDEO_CAMERA_SIMULATION;
        m_ExchangeMemory->m_EnableTrigger = true;
        m_ExchangeMemory->m_SetManualTrigger = false;
        m_ExchangeMemory->m_UseManualTriggerOutputs = USE_BOTH_VALVES;
        m_ExchangeMemory->m_NumberEjectedBottlesByUser = 0;
        m_ExchangeMemory->m_ButtonIsClickedEjectTheNextnBottles = false;
        m_ExchangeMemory->m_InfoLeftTriggerIsSet = false;
        m_ExchangeMemory->m_InfoRightTriggerIsSet = false;
        m_ExchangeMemory->m_MeasuringParameter.m_ResetAllCounters = true;
        m_ExchangeMemory->m_MeasuringParameter.m_ResetCountersBottlesEjectionAndLiquidTooLow = true;
        m_ExchangeMemory->m_MeasuringParameter.m_InfoLevel = INFO_LEVEL_OFF;
        m_ExchangeMemory->m_MeasuringParameter.m_CurrentFifoSize = 0;
        m_ExchangeMemory->m_MeasuringParameter.m_TimeStampOffsetIn100nsUnits = 0;
        m_ExchangeMemory->m_MeasuringParameter.m_AbortMeasuringTask = false;
        m_ExchangeMemory->m_MeasuringParameter.m_AbortInputOutputTask = false;
        m_ExchangeMemory->m_MeasuringParameter.m_ReadyMeasuringTask = false;
        m_ExchangeMemory->m_MeasuringParameter.m_TriggerGetNewVideoFromRealTimeContext = false;
        m_ExchangeMemory->m_MeasuringParameter.m_ReadVideoFromRealTimeContext = false;
        m_ExchangeMemory->m_MeasuringParameter.m_ClearVideoBuffer = false;
        m_ExchangeMemory->m_MeasuringParameter.m_CalibrateModus = false;  // not in use
        m_ExchangeMemory->m_MeasuringParameter.m_DeaktivateCheckBottleUnderValve = false;
        SetKernelParameterToRealTime();
    } else {
        AddErrorMsg = tr("KS_createSharedMemEx Camera Failed");
        rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
    }
    return rv;
}

void KitharaCore::SetKernelParameterToRealTime()
{
    if (m_ExchangeMemory && m_KernelParameter) {
        m_ExchangeMemory->m_MeasuringParameter.m_TimePeriodNotBlowInms = m_KernelParameter->m_MeasuringParameter.m_TimePeriodNotBlowInms;
        m_ExchangeMemory->m_MeasuringParameter.m_MaxMeasurementsProductIsOutOfTol = m_KernelParameter->m_MeasuringParameter.m_MaxMeasurementsProductIsOutOfTol;
        m_ExchangeMemory->m_MeasuringParameter.m_NumberProductsAverageBottleNeckAndPixelSize = m_KernelParameter->m_MeasuringParameter.m_NumberProductsAverageBottleNeckAndPixelSize;
        m_ExchangeMemory->m_MeasuringParameter.m_InjectonWindowMiddleWidthInMm = m_KernelParameter->m_MeasuringParameter.m_InjectonWindowMiddleWidthInMm;
        m_ExchangeMemory->m_MeasuringParameter.m_MinSpeedInMMPerMs = m_KernelParameter->m_MeasuringParameter.m_MinSpeedInMMPerMs;
        m_ExchangeMemory->m_MeasuringParameter.m_DistancesBetweenValves = m_KernelParameter->m_MeasuringParameter.m_DistancesBetweenValves;
        m_ExchangeMemory->m_MeasuringParameter.m_AcceptanceThresholdLiquidLeftAndRightROI = m_KernelParameter->m_MeasuringParameter.m_AcceptanceThresholdLiquidLeftAndRightROI;
        m_ExchangeMemory->m_MeasuringParameter.m_MinAcceptanceThresholdLiquidMiddleROI = m_KernelParameter->m_MeasuringParameter.m_MinAcceptanceThresholdLiquidMiddleROI;
        m_ExchangeMemory->m_MeasuringParameter.m_MaxAcceptanceThresholdLiquidMiddleROI = m_KernelParameter->m_MeasuringParameter.m_MaxAcceptanceThresholdLiquidMiddleROI;
        m_ExchangeMemory->m_MeasuringParameter.m_ThresholdBinaryImageLiquid = m_KernelParameter->m_MeasuringParameter.m_ThresholdBinaryImageLiquid;
        m_ExchangeMemory->m_MeasuringParameter.m_MinNumberFoundedInROI = m_KernelParameter->m_MeasuringParameter.m_MinNumberFoundedInROI;
        for (int i = 0; i < NUMBER_MEASUREWINDOWS; i++) {
            m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowWidth[i] = m_KernelParameter->m_MeasuringParameter.m_MeasureWindowWidth[i];
            m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowHeight[i] = m_KernelParameter->m_MeasuringParameter.m_MeasureWindowHeight[i];
            m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowOffsetX[i] = m_KernelParameter->m_MeasuringParameter.m_MeasureWindowOffsetX[i];
            m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowOffsetY[i] = m_KernelParameter->m_MeasuringParameter.m_MeasureWindowOffsetY[i];
        }
        m_ExchangeMemory->m_MeasuringParameter.m_ReferenceInjectionPositionInPixel = m_KernelParameter->m_MeasuringParameter.m_ReferenceInjectionPositionInPixel;
        m_ExchangeMemory->m_MeasuringParameter.m_ReferenceMeasurePositionInPixel = m_KernelParameter->m_MeasuringParameter.m_ReferenceMeasurePositionInPixel;
        m_ExchangeMemory->m_MeasuringParameter.m_DistanceBetweenMeasureMiddlePosAndTriggerPosInPixel = m_KernelParameter->m_MeasuringParameter.m_DistanceBetweenMeasureMiddlePosAndTriggerPosInPixel;
        m_ExchangeMemory->m_MeasuringParameter.m_EdgeThreshold = m_KernelParameter->m_MeasuringParameter.m_EdgeThreshold;
        m_ExchangeMemory->m_MeasuringParameter.m_FilterKernelSize = m_KernelParameter->m_MeasuringParameter.m_FilterKernelSize;
        m_ExchangeMemory->m_MeasuringParameter.m_PixelSizeInMMPerPixel = m_KernelParameter->m_MeasuringParameter.m_PixelSizeInMMPerPixel;
        m_ExchangeMemory->m_MeasuringParameter.m_TriggerOffsetFirstValveInmm = m_KernelParameter->m_MeasuringParameter.m_TriggerOffsetFirstValveInmm;
        m_ExchangeMemory->m_MeasuringParameter.m_TriggerOffsetSecondValveInmm = m_KernelParameter->m_MeasuringParameter.m_TriggerOffsetSecondValveInmm;
        m_ExchangeMemory->m_MeasuringParameter.m_BottleneckDiameter = m_KernelParameter->m_MeasuringParameter.m_BottleneckDiameter;
        m_ExchangeMemory->m_MeasuringParameter.m_BandDirectional = m_KernelParameter->m_MeasuringParameter.m_BandDirectional;
        m_ExchangeMemory->m_MeasuringParameter.m_ImageBackgroundContrast = m_KernelParameter->m_MeasuringParameter.m_ImageBackgroundContrast;
        m_ExchangeMemory->m_MeasuringParameter.m_BotteleNeckDiameterToleranceInmm = m_KernelParameter->m_MeasuringParameter.m_BotteleNeckDiameterToleranceInmm;
        m_ExchangeMemory->m_MeasuringParameter.m_DistanceBottleEjectionInmm = m_KernelParameter->m_MeasuringParameter.m_DistanceBottleEjectionInmm;
        m_ExchangeMemory->m_MeasuringParameter.m_ProductWidthInmm = m_KernelParameter->m_MeasuringParameter.m_ProductWidthInmm;
        m_ExchangeMemory->m_MeasuringParameter.m_BlowOutEjectorNormallyClosed = m_KernelParameter->m_MeasuringParameter.m_BlowOutEjectorNormallyClosed;
        m_ExchangeMemory->m_MeasuringParameter.m_RightTriggerIsFirst = m_KernelParameter->m_MeasuringParameter.m_RightTriggerIsFirst;
        m_ExchangeMemory->m_MeasuringParameter.m_TargetProcessorIOTask = m_KernelParameter->m_MeasuringParameter.m_TargetProcessorIOTask;
        m_ExchangeMemory->m_MeasuringParameter.m_TargetProcessorMeasureTask = m_KernelParameter->m_MeasuringParameter.m_TargetProcessorMeasureTask;
        m_ExchangeMemory->m_MeasuringParameter.m_UsedTriggerOutputs = m_KernelParameter->m_MeasuringParameter.m_UsedTriggerOutputs;
        m_ExchangeMemory->m_MeasuringParameter.m_WorkWithTwoValves = m_KernelParameter->m_MeasuringParameter.m_WorkWithTwoValves;
        m_ExchangeMemory->m_MeasuringParameter.m_RollingMeanValueLiquid = m_KernelParameter->m_MeasuringParameter.m_RollingMeanValueLiquid;
        m_ExchangeMemory->m_MeasuringParameter.m_BottleOffsetOutOfROIInmm = m_KernelParameter->m_MeasuringParameter.m_BottleOffsetOutOfROIInmm;
        m_ExchangeMemory->m_MeasuringParameter.m_FormatFromISInmm = m_KernelParameter->m_MeasuringParameter.m_FormatFromISInmm;
        m_ExchangeMemory->m_MeasuringParameter.m_UseSpeedFromISCalcEjectionTime = m_KernelParameter->m_MeasuringParameter.m_UseSpeedFromISCalcEjectionTime;
        m_ExchangeMemory->m_EtherCATConfigData.m_CyclusTimeIOTaskInms = m_KernelParameter->m_EtherCATConfigData.m_CyclusTimeIOTaskInms;
        m_ExchangeMemory->m_EtherCATConfigData.m_TimePeriodTriggerOutputOnInms = m_KernelParameter->m_EtherCATConfigData.m_TimePeriodTriggerOutputOnInms;
        m_ExchangeMemory->m_EtherCATConfigData.m_TimePeriodDigitalOutputOnInms = m_KernelParameter->m_EtherCATConfigData.m_TimePeriodDigitalOutputOnInms;
        m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerSTMVelocity.Velocity = m_KernelParameter->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerSTMVelocity.Velocity;
    }
}

void KitharaCore::SetMixerVelocity(int velocity)
{
    if (m_ExchangeMemory) {
        m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerSTMVelocity.Velocity = velocity;
    }
}

void KitharaCore::SetMixerOn(bool on)
{
    if (m_ExchangeMemory) {
        m_ExchangeMemory->m_EtherCATConfigData.m_ExecuteMixer = on;
    }
}

bool KitharaCore::IsMixerOn()
{
    if (m_ExchangeMemory) {
        return m_ExchangeMemory->m_EtherCATConfigData.m_ExecuteMixer;
    } else {
        return false;
    }
}

void KitharaCore::ResetAllCounters()
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_MeasuringParameter.m_ResetAllCounters = true;
}

void KitharaCore::ResetCountersBottlesEjectionAndLiquidTooLow()
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_MeasuringParameter.m_ResetCountersBottlesEjectionAndLiquidTooLow = true;
}

bool KitharaCore::IsMeasuringTaskStarted()
{
    if (m_ExchangeMemory)
        return m_ExchangeMemory->m_MeasuringTaskIsStarted;
    else
        return false;
}

bool KitharaCore::IsIOTaskStarted()
{
    if (m_ExchangeMemory)
        return m_ExchangeMemory->m_IOTaskIsStatred;
    else
        return false;
}

void KitharaCore::SetMeasuringTaskStarted(bool set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_MeasuringTaskIsStarted = set;
}

void KitharaCore::SetIOTaskStarted(bool set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_IOTaskIsStatred = set;
}

int KitharaCore::GetCounterNumberBottlesRealEjected()
{
    if (m_ExchangeMemory) {
        return m_ExchangeMemory->m_CounterNumberBottlesRealEjected;
    } else {
        return 0;
    }
}

// Die Events für den Signalaustauch zwischen Echtzeit und Windowsapplikation erzeuegen
int KitharaCore::CreateEvents()
{
    int rv = ERROR_CODE_NO_ERROR;
    QString ErrorMsg;
    KSError ksError = KS_OK;

    // Event wird in der Echtzeittask auf signalisierend gesetzt, wenn von der GUI aus ein Video angefordert wird. In der Klass WaitForNewVideo wartet ein Thread aus dieses Signal/Event
    ksError = KS_createEvent(&(m_ExchangeMemory->m_HandleEventCanCopyVideoData), NULL, 0);
    if (ksError == KS_OK) {
        // Event wird in der Echtzeittask auf signalisierend gesetzt(Alle 100ms) wenn ein neues Livebild angezeigt werden soll
        ksError = KS_createEvent(&(m_ExchangeMemory->m_HandleImageReceivedEvent), NULL, 0);
        if (ksError == KS_OK) {
            // Event wird in der Echtzeittask auf signalisierend gesetzt, wenn die Messtask beendet ist
            ksError = KS_createEvent(&(m_ExchangeMemory->m_HandleEventMeasuringTaskFinished), NULL, 0);
            if (ksError == KS_OK) {
                // Event wird wird in der Echtzeittask auf signalisierend gesetzt, wenn ein neues Bild die Bildeinzugstask erreicht, wenn auf signalisierend gesetz ist, wird die Messung gestartet
                ksError = KS_createEvent(&(m_ExchangeMemory->m_HandleImageReceivedEventMeasuring), NULL, KSF_KERNEL_EXEC);
                if (ksError == KS_OK) {
                    // Event wird in der Echtzeittask auf signalisierend gesetzt, wenn die IO Task beendet ist
                    ksError = KS_createEvent(&(m_ExchangeMemory->m_HandleEventInputOutputTaskFinished), NULL, 0);
                    if (ksError == KS_OK) {
                        // Event wird in der Echtzeittask auf signalisierend gesetzt(Alle 500ms) um die aktuellen Statusdaten vom IO Device zu lesen un in der GUI anzuzeigen
                        ksError = KS_createEvent(&(m_ExchangeMemory->m_HandleEventInputOutput), NULL, 0);
                        if (ksError == KS_OK) {
                            // Event wird in der Echtzeittask auf signalisierend gesetzt zum Zeitpunkt der Entscheidung das Flasche demnächst ausgeworfen wird
                            ksError = KS_createEvent(&(m_ExchangeMemory->m_HandleEventBottleEjected), NULL, 0);
                            if (ksError != KS_OK) {
                                QString AddErrorMsg = tr("KS_createEvent m_HandleEventBottleEjected Failed");
                                rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                                m_ExchangeMemory->m_HandleEventBottleEjected = KS_INVALID_HANDLE;
                            }
                        } else {
                            QString AddErrorMsg = tr("KS_createEvent m_HandleEventInputOutput Failed");
                            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                            m_ExchangeMemory->m_HandleEventInputOutput = KS_INVALID_HANDLE;
                        }
                    } else {
                        QString AddErrorMsg = tr("KS_createEvent EventInputOutputTaskFinished Failed");
                        rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                        m_ExchangeMemory->m_HandleEventInputOutputTaskFinished = KS_INVALID_HANDLE;
                    }
                } else {
                    QString AddErrorMsg = tr("KS_createEvent Measuring Failed");
                    rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                    m_ExchangeMemory->m_HandleImageReceivedEventMeasuring = KS_INVALID_HANDLE;
                }
            } else {
                QString AddErrorMsg = tr("KS_createEvent MeasuringTaskFinished Failed");
                rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                m_ExchangeMemory->m_HandleEventMeasuringTaskFinished = KS_INVALID_HANDLE;
            }
        } else {
            QString AddErrorMsg = tr("KS_createEvent ImageReceivedEvent Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
            m_ExchangeMemory->m_HandleImageReceivedEvent = KS_INVALID_HANDLE;
        }
    } else {
        QString AddErrorMsg = tr("KS_createEvent EventCanCopyVideoData Failed");
        rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        m_ExchangeMemory->m_HandleEventCanCopyVideoData = KS_INVALID_HANDLE;
    }
    return rv;
}

int KitharaCore::RemoveEvents()
{
    int rv = ERROR_CODE_NO_ERROR;
    QString ErrorMsg;
    KSError ksError = KS_OK;

    if (m_ExchangeMemory->m_HandleEventBottleEjected != KS_INVALID_HANDLE) {
        ksError = KS_closeEvent(m_ExchangeMemory->m_HandleEventBottleEjected);
        m_ExchangeMemory->m_HandleEventBottleEjected = KS_INVALID_HANDLE;
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_closeEvent HandleEventBottleEjected Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
    }
    if (m_ExchangeMemory->m_HandleEventCanCopyVideoData != KS_INVALID_HANDLE) {
        ksError = KS_closeEvent(m_ExchangeMemory->m_HandleEventCanCopyVideoData);
        m_ExchangeMemory->m_HandleEventCanCopyVideoData = KS_INVALID_HANDLE;
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_closeEvent EventCanCopyVideoData Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
    }
    if (m_ExchangeMemory->m_HandleImageReceivedEvent != KS_INVALID_HANDLE) {
        ksError = KS_closeEvent(m_ExchangeMemory->m_HandleImageReceivedEvent);
        m_ExchangeMemory->m_HandleImageReceivedEvent = KS_INVALID_HANDLE;
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_closeEvent ImageReceivedEvent Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
    }
    if (m_ExchangeMemory->m_HandleEventMeasuringTaskFinished != KS_INVALID_HANDLE) {
        ksError = KS_closeEvent(m_ExchangeMemory->m_HandleEventMeasuringTaskFinished);
        m_ExchangeMemory->m_HandleEventMeasuringTaskFinished = KS_INVALID_HANDLE;
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_closeEvent EventMeasuringTaskFinished Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
    }
    if (m_ExchangeMemory->m_HandleEventInputOutputTaskFinished != KS_INVALID_HANDLE) {
        ksError = KS_closeEvent(m_ExchangeMemory->m_HandleEventInputOutputTaskFinished);
        m_ExchangeMemory->m_HandleEventInputOutputTaskFinished = KS_INVALID_HANDLE;
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_closeEvent EventInputOutputTaskFinished Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
    }
    if (m_ExchangeMemory->m_HandleImageReceivedEventMeasuring != KS_INVALID_HANDLE) {
        ksError = KS_closeEvent(m_ExchangeMemory->m_HandleImageReceivedEventMeasuring);
        m_ExchangeMemory->m_HandleImageReceivedEventMeasuring = KS_INVALID_HANDLE;
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_closeEvent ImageReceivedEventMeasuring Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
    }
    if (m_ExchangeMemory->m_HandleEventInputOutput != KS_INVALID_HANDLE) {
        ksError = KS_closeEvent(m_ExchangeMemory->m_HandleEventInputOutput);
        m_ExchangeMemory->m_HandleEventInputOutput = KS_INVALID_HANDLE;
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_closeEvent HandleEventInputOutput Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
    }
    return rv;
}

// Anlegen der einzelen Taskfunktionen
int KitharaCore::CreateCallBacks()
{
    int rv = ERROR_CODE_NO_ERROR;
    int flags;
    void* pExchangeMemory = NULL;
    QString ErrorMsg;
    KSError ksError = KS_OK;

#if _DEBUG
    flags = KSF_USER_EXEC | KSF_NO_CONTEXT | KSF_SAVE_FPU;
    pExchangeMemory = (void*)(m_ExchangeMemory);
#else
    flags = KSF_DIRECT_EXEC | KSF_NO_CONTEXT | KSF_SAVE_FPU;
    pExchangeMemory = (void*)(m_ExchangeMemory->pSysPtr);
#endif
    // Task für den Bildeinzug, wird jedesmal aufgerufen wenn ein neues Bild von der Kamera aufgenommen
    ksError = KS_createKernelCallBack(&(m_ExchangeMemory->m_HandleImageReceivedCallBack), m_HandleKernel, "ImageReceivedTaskCallBack", pExchangeMemory, flags, 0);
    if (ksError == KS_OK) {
        // Wird aufgerufen wenn neues bild im speicher
        ksError = KS_createKernelCallBack(&(m_ExchangeMemory->m_HandleMeasureTaskCallBack), m_HandleKernel, "MeasureTaskCallBack", pExchangeMemory, flags, 0);
        if (ksError == KS_OK) {
            // Schreibt die Ausgangsdaten
            ksError = KS_createKernelCallBack(&(m_ExchangeMemory->m_EtherCATConfigData.m_HandleIOTaskCallBack), m_HandleKernel, "IOTaskCallBack", pExchangeMemory, flags, 0);
            if (ksError == KS_OK) {
                // Liest die Eingangsdaten zyklisch ein
                ksError = KS_createKernelCallBack(&(m_ExchangeMemory->m_EtherCATConfigData.m_HandleReadEtherCatDataCallBack), m_HandleKernel, "ReadEtherCatDataCallBack", pExchangeMemory, flags, 0);
                if (ksError != KS_OK) {
                    QString AddErrorMsg = tr("KS_createKernelCallBack  ReadEtherCatDataCallBack Failed");
                    rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                }
            } else {
                QString AddErrorMsg = tr("KS_createKernelCallBack  IOTaskCallBack Failed");
                rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
            }
        } else {
            QString AddErrorMsg = tr("KS_createKernelCallBack  MeasureTaskCallBack Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
    } else {
        QString AddErrorMsg = tr("KS_createKernelCallBack ImageReceivedTaskCallBack Failed");
        rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
    }
    return rv;
}

int KitharaCore::RemoveCallBacks()
{
    int rv = ERROR_CODE_NO_ERROR;
    QString ErrorMsg;
    KSError ksError = KS_OK;

    if (m_ExchangeMemory->m_EtherCATConfigData.m_HandleReadEtherCatDataCallBack != KS_INVALID_HANDLE) {
        ksError = KS_removeCallBack(m_ExchangeMemory->m_EtherCATConfigData.m_HandleReadEtherCatDataCallBack);
        m_ExchangeMemory->m_EtherCATConfigData.m_HandleReadEtherCatDataCallBack = KS_INVALID_HANDLE;
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_removeCallBack ReadEtherCatDataCallBack Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
    }
    if (m_ExchangeMemory->m_EtherCATConfigData.m_HandleIOTaskCallBack != KS_INVALID_HANDLE) {
        ksError = KS_removeCallBack(m_ExchangeMemory->m_EtherCATConfigData.m_HandleIOTaskCallBack);
        m_ExchangeMemory->m_EtherCATConfigData.m_HandleIOTaskCallBack = KS_INVALID_HANDLE;
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_removeCallBack InputOutput Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
    }
    if (m_ExchangeMemory->m_HandleMeasureTaskCallBack != KS_INVALID_HANDLE) {
        ksError = KS_removeCallBack(m_ExchangeMemory->m_HandleMeasureTaskCallBack);
        m_ExchangeMemory->m_HandleMeasureTaskCallBack = KS_INVALID_HANDLE;
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_removeCallBack Measuring Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
    }
    if (m_ExchangeMemory->m_HandleImageReceivedCallBack != KS_INVALID_HANDLE) {
        ksError = KS_removeCallBack(m_ExchangeMemory->m_HandleImageReceivedCallBack);
        m_ExchangeMemory->m_HandleImageReceivedCallBack = KS_INVALID_HANDLE;
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_removeCallBack Camera Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
    }
    return rv;
}

// Erzeugen der einzelen Taskfunktionen
int KitharaCore::CreateRealTimeTasks()
{
    int rv = ERROR_CODE_NO_ERROR;
    QString ErrorMsg;
    KSError ksError = KS_OK;

    ksError = KS_setTaskStackSize(0x200000, 0);
    if (ksError != KS_OK) {
        QString AddErrorMsg = tr("KS_setTaskStackSize Failed");
        rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
    } else {
        ksError = KS_createTask(&(m_ExchangeMemory->m_HandleMeasuringTask), m_ExchangeMemory->m_HandleMeasureTaskCallBack, 254, KSF_CUSTOM_STACK_SIZE | KSF_DONT_START);
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_createTask MeasuringTask Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
            m_ExchangeMemory->m_HandleMeasuringTask = KS_INVALID_HANDLE;
        } else {
            if (m_WorkWithoutCamera) {
                ksError = KS_createTask(&(m_ExchangeMemory->m_HandleCameraSimulationTask), m_ExchangeMemory->m_HandleImageReceivedCallBack, 249, KSF_CUSTOM_STACK_SIZE | KSF_DONT_START);
                if (ksError != KS_OK) {
                    QString AddErrorMsg = tr("KS_createTask Camera Simulation Task Failed");
                    rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                    m_ExchangeMemory->m_HandleCameraSimulationTask = KS_INVALID_HANDLE;
                }
            }
            // erst start wenn init komplett durchgelaufen
            ksError =
                KS_createTimer(&(m_ExchangeMemory->m_EtherCATConfigData.m_HandleIOTask), 0.1 * ms, m_ExchangeMemory->m_EtherCATConfigData.m_HandleIOTaskCallBack, KSF_REALTIME_EXEC | KSF_DONT_START);
            // ksError =
            //    KS_createTask(&(m_ExchangeMemory->m_EtherCATConfigData.m_HandleIOTask), m_ExchangeMemory->m_EtherCATConfigData.m_HandleIOTaskCallBack, 250, KSF_CUSTOM_STACK_SIZE | KSF_DONT_START);
            if (ksError != KS_OK) {
                QString AddErrorMsg = tr("KS_createTask IOTask Failed");
                rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                m_ExchangeMemory->m_EtherCATConfigData.m_HandleIOTask = KS_INVALID_HANDLE;
            } else {
                ksError = KS_createTask(&(m_ExchangeMemory->m_EtherCATConfigData.m_HandleReadEtherCatDataTask), m_ExchangeMemory->m_EtherCATConfigData.m_HandleReadEtherCatDataCallBack, 251,
                                        KSF_CUSTOM_STACK_SIZE | KSF_DONT_START);
                if (ksError != KS_OK) {
                    QString AddErrorMsg = tr("KS_createTask ReadEtherCatDataTask Failed");
                    rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                    m_ExchangeMemory->m_EtherCATConfigData.m_HandleReadEtherCatDataTask = KS_INVALID_HANDLE;
                }
            }
        }
    }

    return rv;
}

int KitharaCore::StopAndRemoveMeasureTask()
{
    int rv = ERROR_CODE_NO_ERROR;
    uint TaskState;
    QString ErrorMsg;
    KSError ksError = KS_OK;

    if (m_ExchangeMemory->m_HandleMeasuringTask != KS_INVALID_HANDLE) {
        KS_getTaskState(m_ExchangeMemory->m_HandleMeasuringTask, &TaskState, 0);
        if (TaskState != KS_TASK_DORMANT && TaskState != KS_TASK_TERMINATED) {
            m_ExchangeMemory->m_MeasuringParameter.m_AbortMeasuringTask = true;
            if (m_ExchangeMemory->m_HandleImageReceivedEventMeasuring != KS_INVALID_HANDLE) {
                ksError = KS_setEvent(m_ExchangeMemory->m_HandleImageReceivedEventMeasuring);
                if (ksError != KS_OK) {
                    QString AddErrorMsg = tr("KS_setEvent Failed");
                    rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                }
            }
            if (ksError == KS_OK) {
                ksError = KS_waitForEvent(m_ExchangeMemory->m_HandleEventMeasuringTaskFinished, 0, 1000 * ms);
                if (ksError != KS_OK) {
                    QString AddErrorMsg = tr("KS_waitForEvent TimeOut Wait AbortMeasuring Task");
                    rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                }
            }
        }
        ksError = KS_removeTask(m_ExchangeMemory->m_HandleMeasuringTask);
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_removeTask MeasuringTask Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
        m_ExchangeMemory->m_HandleMeasuringTask = KS_INVALID_HANDLE;
    }
    return rv;
}

int KitharaCore::StopAndRemoveCameraSimulationTask()
{
    int rv = ERROR_CODE_NO_ERROR;
    uint TaskState;
    QString ErrorMsg;
    KSError ksError = KS_OK;

    if (m_ExchangeMemory->m_HandleCameraSimulationTask != KS_INVALID_HANDLE) {
        KS_getTaskState(m_ExchangeMemory->m_HandleCameraSimulationTask, &TaskState, 0);
        if (TaskState != KS_TASK_DORMANT && TaskState != KS_TASK_TERMINATED) {
            m_ExchangeMemory->m_AbortCameraSimulation = true;
            Sleep(500);
        }
        ksError = KS_removeTask(m_ExchangeMemory->m_HandleCameraSimulationTask);
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_removeTask CameraSimulation Task Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
        m_ExchangeMemory->m_HandleCameraSimulationTask = KS_INVALID_HANDLE;
    }
    return rv;
}

int KitharaCore::StopAndRemoveIOTask()
{
    int rv = ERROR_CODE_NO_ERROR;
    uint TaskState;
    QString ErrorMsg;
    KSError ksError = KS_OK;

    if (m_ExchangeMemory->m_EtherCATConfigData.m_HandleIOTask != KS_INVALID_HANDLE) {
        /*m_ExchangeMemory->m_MeasuringParameter.m_AbortInputOutputTask = true;
        ksError = KS_waitForEvent(m_ExchangeMemory->m_HandleEventInputOutputTaskFinished, 0, 2000 * ms);
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_waitForEvent TimeOut Wait Abort InputOutputTask Task");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
        ksError = KS_removeTask(m_ExchangeMemory->m_EtherCATConfigData.m_HandleIOTask);
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_removeTask InputoutputTask Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }*/

        ksError = KS_stopTimer(m_ExchangeMemory->m_EtherCATConfigData.m_HandleIOTask);  // Timer handle
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_stopTimer InputoutputTask Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }

        ksError = KS_removeTimer(m_ExchangeMemory->m_EtherCATConfigData.m_HandleIOTask);  // Timer handle
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_removeTimer InputoutputTask Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }

        m_ExchangeMemory->m_EtherCATConfigData.m_HandleIOTask = KS_INVALID_HANDLE;
    }
    return rv;
}

int KitharaCore::StopAndRemoveReadEtherCatDataTask()
{
    int rv = ERROR_CODE_NO_ERROR;
    QString ErrorMsg;
    KSError ksError = KS_OK;

    if (m_ExchangeMemory->m_EtherCATConfigData.m_HandleReadEtherCatDataTask != KS_INVALID_HANDLE) {
        ksError = KS_removeTask(m_ExchangeMemory->m_EtherCATConfigData.m_HandleReadEtherCatDataTask);
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_removeTask  ReadEtherCatDataTask Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
    }
    return rv;
}

int KitharaCore::CreateSharedMemoryForCameraSimulation(QString& VideoFileName)
{
    int rv = ERROR_CODE_NO_ERROR;
    VideoHeader videoHeader;
    QString ErrorMsg, CameraStartUpInfo, PathNameVideoFile = GetMainAppCrystalT2()->GetVideoFileLocation() + QString("/") + VideoFileName;
    int MB = 1024 * 1024;
    cv::VideoCapture cap;
    cv::Mat VideoFrame;
    int ImageBlockSize;

    if (cap.open(PathNameVideoFile.toLatin1().data())) {
        double fps = cap.get(cv::CAP_PROP_FPS);
        videoHeader.m_MaxNumberFrames = static_cast<unsigned __int64>(cap.get(cv::CAP_PROP_FRAME_COUNT));
        videoHeader.m_ImageWidth = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
        videoHeader.m_ImageHeight = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
        ImageBlockSize = videoHeader.m_ImageWidth * videoHeader.m_ImageHeight;
        videoHeader.m_MAXVideoBockSize = sizeof(VideoHeader) + (videoHeader.m_MaxNumberFrames * ImageBlockSize);
        if (videoHeader.m_MaxNumberFrames > 0) {
            m_ExchangeMemory->m_CameraSimulationOn = true;                                                                                             // info an real time task simulate camera
            KSError ksError = KS_createSharedMemEx(&(m_ExchangeMemory->m_HandleVideoBlockCameraSimulation), NULL, videoHeader.m_MAXVideoBockSize, 0);  // KSF_ALTERNATIVE | KSF_CONTINUOUS);
            if (ksError == KS_OK) {
                CameraStartUpInfo = tr("Info! KS_createSharedMemEx Get Pointer To Video Block Camera Simulation. Size: %1 MB: OK").arg((videoHeader.m_MAXVideoBockSize) / (double)(MB), 0, 'f', 2);
                SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);  // ERROR_CODE_NO_ERROR);
                ksError = KS_getSharedMemEx(m_ExchangeMemory->m_HandleVideoBlockCameraSimulation, (void**)&m_PointerVideoBlockCameraSimulation, 0);
                if (ksError == KS_OK) {
                    memcpy(m_PointerVideoBlockCameraSimulation, &videoHeader, sizeof(VideoHeader));  // copy videoHeader initial value
                    CameraStartUpInfo = tr("Info! KS_getSharedMemEx Get Pointer To Video Block Camera Simulation: OK");
                    SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);  // ERROR_CODE_NO_ERROR);
                    unsigned char* pImageRawData = m_PointerVideoBlockCameraSimulation + sizeof(VideoHeader);
                    int ImageOffsetX = GetImageData()->GetImageOffsetX();
                    int ImageOffsetY = GetImageData()->GetImageOffsetY();
                    int ImageWidth = GetImageData()->GetImageWidth();
                    int ImageHeight = GetImageData()->GetImageHeight();
                    if (CheckCameraROIPosition(ImageOffsetX, ImageOffsetY, ImageWidth, ImageHeight, videoHeader.m_ImageWidth, videoHeader.m_ImageHeight)) {
                        QRect newCameraROIRect(ImageOffsetX, ImageOffsetY, ImageWidth, ImageHeight);
                        GetImageData()->SetMeasureWindowRect(ROI_ID_CAMERA, newCameraROIRect);
                    }
                    if (ImageWidth < videoHeader.m_ImageWidth || ImageHeight < videoHeader.m_ImageHeight) {
                        m_IsCameraResolutionGreaterThanDefaultResolution = true;  // dann kann in der Bedienoberfläche, der Kamera ROI verschoben werden
                    } else {
                        m_IsCameraResolutionGreaterThanDefaultResolution = false;
                    }
                    for (int i = 0; i < videoHeader.m_MaxNumberFrames; i++) {
                        cap >> VideoFrame;
                        cvtColor(VideoFrame, VideoFrame, cv::COLOR_BGR2GRAY);
                        if (!VideoFrame.empty()) {
                            // flip(VideoFrame, VideoFrame, 1);
                            memcpy(pImageRawData, (unsigned char*)(VideoFrame.data), ImageBlockSize);
                            pImageRawData = pImageRawData + ImageBlockSize;
                        }
                    }
                } else {
                    QString AddMsg = tr("KS_getSharedMemEx For Video Block Camera Simulation Failed");
                    rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddMsg);
                }
            } else {
                QString AddMsg = tr("KS_createSharedMemEx For Video Block Camera Simulation Failed");
                rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddMsg);
            }
        } else {
            ErrorMsg = tr("Error! Can Not Open Video File: %1").arg(PathNameVideoFile);
            SendMessageToGUI(ErrorMsg, QtMsgType::QtFatalMsg);
            rv = ERROR_CODE_ANY_ERROR;
        }
    } else {
        ErrorMsg = tr("Error! Open Video File Camera Simulation Failed: %1").arg(PathNameVideoFile);
        SendMessageToGUI(ErrorMsg, QtMsgType::QtFatalMsg);
        rv = ERROR_CODE_ANY_ERROR;
    }
    return rv;
}

int KitharaCore::FreeImageSharedMemoryForCameraSimulation()
{
    int rv = ERROR_CODE_NO_ERROR;

    if (m_ExchangeMemory->m_HandleVideoBlockCameraSimulation != KS_INVALID_HANDLE) {
        QString ErrorMsg;
        KSError ksError = KS_freeSharedMemEx(m_ExchangeMemory->m_HandleVideoBlockCameraSimulation, 0);
        m_ExchangeMemory->m_HandleVideoBlockCameraSimulation = KS_INVALID_HANDLE;
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_freeSharedMem Image Block Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
        m_PointerVideoBlockCameraSimulation = NULL;
    }
    return rv;
}

// Anlegen der verschiedenen Speicherblöcke für die unterschiedlichen Bilddaten(Livebild, Video und das Bild weleches den Verschmutzunggrad anzeigt)
int KitharaCore::CreateSharedMemory()
{
    int rv = ERROR_CODE_NO_ERROR;
    int MB = 1024 * 1024;
    int MaxNumberImagesInVideoFillingProcess = 125;
    VideoHeader videoHeader;
    VideoHeader videoHeaderFillingProcess;  // Videospeicher/Header seperater Vidospeicher in dem nur die Bilder des letzten Befüllvorganges gespeichert werden
    ImageHeader imageHeader;
    MeasuringResults errorImageMeasuringResultsHeader;
    QString ErrorMsg, CameraStartUpInfo;
    int64 SizeImageBlock = (GetImageData()->GetImageWidth() * GetImageData()->GetImageHeight()) + sizeof(ImageHeader);
    KSError ksError;

    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        videoHeader.m_MAXVideoBockSize = GetMainAppCrystalT2()->GetSettingsData()->m_SizeVideoMemoryInMB * MB;
    } else {
        videoHeader.m_MAXVideoBockSize = 256 * MB;
    }
    videoHeader.m_MaxNumberFrames = static_cast<unsigned __int64>(videoHeader.m_MAXVideoBockSize / ((double)(SizeImageBlock)));
    videoHeader.m_ImageWidth = GetImageData()->GetImageWidth();
    videoHeader.m_ImageHeight = GetImageData()->GetImageHeight();

    videoHeaderFillingProcess.m_ImageWidth = GetImageData()->GetImageWidth();
    videoHeaderFillingProcess.m_ImageHeight = GetImageData()->GetImageHeight();
    videoHeaderFillingProcess.m_MaxNumberFrames = MaxNumberImagesInVideoFillingProcess * 2;  // Anzahl Bilder für den Befüllvorgang (Beide Ventile)
    videoHeaderFillingProcess.m_MAXVideoBockSize = sizeof(VideoHeader) + SizeImageBlock * videoHeaderFillingProcess.m_MaxNumberFrames;

    CameraStartUpInfo = tr("Info! Max Frames In Video Buffer:%1").arg(static_cast<int>(videoHeader.m_MaxNumberFrames));
    SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);

    CameraStartUpInfo = tr("Info! KS_getSharedMemEx Get Pointer To Error Image Block: OK");
    SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
    // Sharedmemory anlegen für das Bild welches den Verschmutzunggrad anzeigt. Dieses Dild wird immer dann von der Echzeittask in den Speicher geschrieben wenn eine
    // Flasch erfogreich gefüllt wurde
    ksError = KS_createSharedMemEx(&(m_ExchangeMemory->m_HandleCleanImageBlock), NULL, SizeImageBlock + sizeof(ImageHeader), 0);
    if (ksError == KS_OK) {
        CameraStartUpInfo = tr("Info! KS_createSharedMemEx Get Pointer To Clean Image Block. Size: %1 MB: OK").arg((double)(SizeImageBlock + sizeof(ImageHeader)) / MB, 0, 'f', 2);
        SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
        ksError = KS_getSharedMemEx(m_ExchangeMemory->m_HandleCleanImageBlock, (void**)&m_PointerCleanImageBlock, 0);
        if (ksError == KS_OK) {
            CameraStartUpInfo = tr("Info! KS_getSharedMemEx Get Pointer To Clean Image Block: OK");
            SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
            ksError = KS_createSharedMemEx(&(m_ExchangeMemory->m_HandleImageBlock), NULL, SizeImageBlock, 0);
            if (ksError == KS_OK) {  // Sharedmemory anlegen für das Livebild. Ein Livebild wird nur alle 100ms angzeigt
                CameraStartUpInfo = tr("Info! KS_createSharedMemEx Get Pointer To Image Block. Size: %1 MB: OK").arg((double)(SizeImageBlock) / MB, 0, 'f', 2);
                SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                ksError = KS_getSharedMemEx(m_ExchangeMemory->m_HandleImageBlock, (void**)&m_PointerImageBlock, 0);
                if (ksError == KS_OK) {
                    CameraStartUpInfo = tr("Info! KS_getSharedMemEx Get Pointer To Image Block: OK");
                    SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                    ksError = KS_createSharedMemEx(&(m_ExchangeMemory->m_HandleVideoBlock), NULL, videoHeader.m_MAXVideoBockSize + sizeof(VideoHeader), 0);
                    if (ksError == KS_OK) {  // Sharedmemory anlegen für das Video
                        CameraStartUpInfo =
                            tr("Info! KS_createSharedMemEx Get Pointer To Video Block. Size:%1MB: OK").arg((videoHeader.m_MAXVideoBockSize + sizeof(VideoHeader)) / (double)(MB), 0, 'f', 2);
                        SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                        ksError = KS_getSharedMemEx(m_ExchangeMemory->m_HandleVideoBlock, (void**)&m_PointerVideoBlock, 0);
                        if (ksError == KS_OK) {
                            memcpy(m_PointerVideoBlock, &videoHeader, sizeof(VideoHeader));  // copy videoHeader initial value
                            CameraStartUpInfo = tr("Info! KS_getSharedMemEx Get Pointer To Video Block: OK");
                            SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                            ksError = KS_createSharedMemEx(&(m_ExchangeMemory->m_HandleVideoBlockFillingProcess), NULL, videoHeaderFillingProcess.m_MAXVideoBockSize, 0);
                            if (ksError == KS_OK) {  // seperater Videospeicher für den Flaschenauswurf. Hier wereden nur die letzten 125 Bilder gespeichert nach Beendingung des Befüllvorganges
                                CameraStartUpInfo = tr("Info! KS_createSharedMemEx Filling Process. Size: %1 MB: OK").arg((videoHeaderFillingProcess.m_MAXVideoBockSize) / (double)(MB), 0, 'f', 2);
                                SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                                ksError = KS_getSharedMemEx(m_ExchangeMemory->m_HandleVideoBlockFillingProcess, (void**)&m_PointerVideoBlockFillingProcess, 0);
                                if (ksError == KS_OK) {
                                    memcpy(m_PointerVideoBlockFillingProcess, &videoHeaderFillingProcess, sizeof(VideoHeader));  // copy videoHeader initial value
                                    CameraStartUpInfo = tr("Info! KS_getSharedMemEx Get Pointer To Video Block Bottles Ejected: OK");
                                    SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                                } else {
                                    QString AddMsg = tr("KS_getSharedMemEx For Video Block Bottles Ejected Failed");
                                    rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddMsg);
                                }
                            } else {
                                QString AddMsg = tr("KS_createSharedMemEx For Video Block Bottles Ejected Failed");
                                rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddMsg);
                            }
                        } else {
                            QString AddMsg = tr("KS_getSharedMemEx For Video Block Failed");
                            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddMsg);
                        }
                    } else {
                        QString AddMsg = tr("KS_createSharedMemEx For Video Block Failed");
                        rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddMsg);
                    }
                } else {
                    QString AddMsg = tr("KS_getSharedMemEx Pointer To Image Block Failed");
                    rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddMsg);
                }
            } else {
                QString AddMsg = tr("KS_createSharedMemEx For Image Block Failed");
                rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddMsg);
            }
        } else {
            QString AddMsg = tr("KS_getSharedMemEx Pointer To Clean Image Block Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddMsg);
        }
    } else {
        QString AddMsg = tr("KS_createSharedMemEx For Clean Image Block Failed");
        rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddMsg);
    }
    return rv;
}

int KitharaCore::FreeImageSharedMemory()
{
    int rv = ERROR_CODE_NO_ERROR;
    QString ErrorMsg;
    KSError ksError;

    if (m_ExchangeMemory->m_HandleCleanImageBlock != KS_INVALID_HANDLE) {
        ksError = KS_freeSharedMemEx(m_ExchangeMemory->m_HandleCleanImageBlock, 0);
        m_ExchangeMemory->m_HandleCleanImageBlock = KS_INVALID_HANDLE;
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_freeSharedMem Clean Image Block Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
        m_PointerCleanImageBlock = NULL;
    }
    if (m_ExchangeMemory->m_HandleImageBlock != KS_INVALID_HANDLE) {
        ksError = KS_freeSharedMemEx(m_ExchangeMemory->m_HandleImageBlock, 0);
        m_ExchangeMemory->m_HandleImageBlock = KS_INVALID_HANDLE;
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_freeSharedMem Image Block Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
        m_PointerImageBlock = NULL;
    }
    if (m_ExchangeMemory->m_HandleVideoBlock != KS_INVALID_HANDLE) {
        ksError = KS_freeSharedMemEx(m_ExchangeMemory->m_HandleVideoBlock, 0);
        m_ExchangeMemory->m_HandleVideoBlock = KS_INVALID_HANDLE;
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_freeSharedMem Video Block Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
        m_PointerVideoBlock = NULL;
    }
    if (m_ExchangeMemory->m_HandleVideoBlockFillingProcess != KS_INVALID_HANDLE) {
        ksError = KS_freeSharedMemEx(m_ExchangeMemory->m_HandleVideoBlockFillingProcess, 0);
        m_ExchangeMemory->m_HandleVideoBlockFillingProcess = KS_INVALID_HANDLE;
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_freeSharedMem Video Block Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
        m_PointerVideoBlock = NULL;
    }
    return rv;
}

int KitharaCore::FreeExchangeSharedMemory()
{
    int rv = ERROR_CODE_NO_ERROR;
    QString ErrorMsg;
    KSError ksError;

    ksError = KS_freeSharedMem(m_ExchangeMemory);
    if (ksError != KS_OK) {
        QString AddErrorMsg = tr("KS_freeSharedMem Failed");
        rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
    }
    m_ExchangeMemory = NULL;
    return rv;
}

int KitharaCore::GetFloatNodeCameraValue(const QString& NodeName, double& Value)
{
    int rv = ERROR_CODE_NO_ERROR;
    QString ErrorMsg;
    KSError ksError = KS_OK;

    if (m_ExchangeMemory->m_HandleCamera != KS_INVALID_HANDLE) {
        ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_GET, NodeName.toStdString().c_str(), 0, &Value, 0, 0);
        if (ksError != KS_OK) {
            QString AddMsg = tr("Can Not Read Camera String Node Name: %1  Value:%2").arg(NodeName).arg(Value, 0, 'f', 3);
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddMsg);
        }
    } else {
        ErrorMsg = tr("Error! Can Not Read Camera String Node Name: %1  Value:%2. Camera Is Not Open").arg(NodeName).arg(Value, 0, 'f', 3);
        SendMessageToGUI(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
        rv = ERROR_CODE_ANY_ERROR;
    }
    return rv;
}

int KitharaCore::SetStringNodeCameraValue(const QString& NodeName, QString& Value)
{
    int rv = ERROR_CODE_NO_ERROR;
    QString ErrorMsg;
    KSError ksError = KS_OK;

    if (m_ExchangeMemory->m_HandleCamera != KS_INVALID_HANDLE) {
        ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_SET, NodeName.toStdString().c_str(), 0, &Value, Value.size(), 0);
        if (ksError != KS_OK) {
            QString AddMsg = tr("Can Not Write Camera String Node Name: %1  Value:%2").arg(NodeName).arg(Value);
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddMsg);
        }
    } else {
        ErrorMsg = tr("Error! Can Not Write Camera String Node Name: %1  Value:%2.  Camera Is Not Open").arg(NodeName).arg(Value);
        SendMessageToGUI(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
        rv = ERROR_CODE_ANY_ERROR;
    }
    return rv;
}

int KitharaCore::SetFloatNodeCameraValue(const QString& NodeName, double Value)
{
    int rv = ERROR_CODE_NO_ERROR;
    QString ErrorMsg;
    KSError ksError = KS_OK;

    if (m_ExchangeMemory->m_HandleCamera != KS_INVALID_HANDLE) {
        ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_SET, NodeName.toStdString().c_str(), 0, &Value, 0, 0);
        if (ksError != KS_OK) {
            QString AddMsg = tr("Can Not Write Camera Float Node Name: %1  Value:%2.").arg(NodeName).arg(Value);
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddMsg);
        }
    } else {
        ErrorMsg = tr("Error! Can Not Write Camera Float Node Name: %1  Value:%2.  Camera Is Not Open").arg(NodeName).arg(Value);
        SendMessageToGUI(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
        rv = ERROR_CODE_ANY_ERROR;
    }
    return rv;
}

int KitharaCore::GetBoolNodeCameraValue(const QString& NodeName, bool& Value)
{
    int rv = ERROR_CODE_NO_ERROR;
    QString ErrorMsg;
    KSError ksError = KS_OK;

    if (m_ExchangeMemory->m_HandleCamera != KS_INVALID_HANDLE) {
        ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_GET, NodeName.toStdString().c_str(), 0, &Value, 0, 0);
        if (ksError != KS_OK) {
            QString AddMsg = tr("Can Not Read Camera Bool Node Name: %1  Value:%2.").arg(NodeName).arg(Value);
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddMsg);
        }
    } else {
        ErrorMsg = tr("Error! Can Not Read Camera Bool Node Name: %1  Value:%2.  Camera Is Not Open").arg(NodeName).arg(Value);
        SendMessageToGUI(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
        rv = ERROR_CODE_ANY_ERROR;
    }
    return rv;
}

int KitharaCore::SetBoolNodeCameraValue(const QString& NodeName, bool Value)
{
    int rv = ERROR_CODE_NO_ERROR;
    QString ErrorMsg;
    KSError ksError = KS_OK;

    if (m_ExchangeMemory->m_HandleCamera != KS_INVALID_HANDLE) {
        ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_SET, NodeName.toStdString().c_str(), 0, &Value, 0, 0);
        if (ksError != KS_OK) {
            QString AddMsg = tr("Can Not Write Camera Bool Node Name: %1  Value:%2.").arg(NodeName).arg(Value);
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddMsg);
        }
    } else {
        ErrorMsg = tr("Error! Can Not Write Camera Bool Node Name: %1  Value:%2.  Camera Is Not Open").arg(NodeName).arg(Value);
        SendMessageToGUI(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
        rv = ERROR_CODE_ANY_ERROR;
    }
    return rv;
}

int KitharaCore::SetEnumarateCameraValue(const QString& NodeName, int Value)
{
    int rv = ERROR_CODE_NO_ERROR;
    QString ErrorMsg;
    KSError ksError = KS_OK;
    int64 i64Value = Value;

    if (m_ExchangeMemory->m_HandleCamera != KS_INVALID_HANDLE) {
        ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_ENUMERATE, NodeName.toStdString().c_str(), 1, &i64Value, 0, 0);
        if (ksError != KS_OK) {
            QString AddMsg = tr("Can Not Write Camera Int Node Name: %1  Value:%2.").arg(NodeName).arg(Value);
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddMsg);
        }
    } else {
        ErrorMsg = tr("Error! Can Not Write Camera Int Node Name: %1  Value:%2.  Camera Is Not Open").arg(NodeName).arg(Value);
        SendMessageToGUI(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
        rv = ERROR_CODE_ANY_ERROR;
    }
    return rv;
}

int KitharaCore::SetInt64NodeCameraValue(const QString& NodeName, int Value)
{
    int rv = ERROR_CODE_NO_ERROR;
    QString ErrorMsg;
    KSError ksError = KS_OK;
    int64 i64Value = Value;

    if (m_ExchangeMemory->m_HandleCamera != KS_INVALID_HANDLE) {
        ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_SET, NodeName.toStdString().c_str(), 0, &i64Value, 0, 0);
        if (ksError != KS_OK) {
            QString AddMsg = tr("Can Not Write Camera Int Node Name: %1  Value:%2.").arg(NodeName).arg(Value);
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddMsg);
        }
    } else {
        ErrorMsg = tr("Error! Can Not Write Camera Int Node Name: %1  Value:%2.  Camera Is Not Open").arg(NodeName).arg(Value);
        SendMessageToGUI(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
        rv = ERROR_CODE_ANY_ERROR;
    }
    return rv;
}

int KitharaCore::GetInt64NodeCameraValue(const QString& NodeName, int64& Value)
{
    int rv = ERROR_CODE_NO_ERROR;
    QString ErrorMsg;
    KSError ksError = KS_OK;

    if (m_ExchangeMemory->m_HandleCamera != KS_INVALID_HANDLE) {
        ksError = KS_configCamera(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONFIG_GET, NodeName.toStdString().c_str(), 0, &Value, 0, 0);
        if (ksError != KS_OK) {
            QString AddMsg = tr("Can Not Read Camera Int Node Name: %1  Value:%2.").arg(NodeName).arg(Value);
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddMsg);
        }
    } else {
        ErrorMsg = tr("Error! Can Not Read Camera Int Node Name: %1  Value:%2.  Camera Is Not Open").arg(NodeName).arg(Value);
        SendMessageToGUI(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
        rv = ERROR_CODE_ANY_ERROR;
    }
    return rv;
}

int KitharaCore::InitCameraHandler()
{
    int rv = ERROR_CODE_NO_ERROR;
    QString ErrorMsg;
    KSError ksError = KS_OK;

    if (m_ExchangeMemory->m_HandleCamera != KS_INVALID_HANDLE) {
        ksError = KS_createCameraStream(&(m_ExchangeMemory->m_HandleStream), m_ExchangeMemory->m_HandleCamera, 0, 3, 0, 0);
        if (ksError == KS_OK) {
            ksError = KS_installCameraHandler(m_ExchangeMemory->m_HandleStream, KS_CAMERA_IMAGE_RECEIVED, m_ExchangeMemory->m_HandleImageReceivedCallBack, 0);
            if (ksError != KS_OK) {
                QString AddErrorMsg = tr("KS_installCameraHandler Failed");
                rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
            }
        } else {
            QString AddErrorMsg = tr("KS_createCameraStream Failed");
            rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        }
    }
    return rv;
}

int KitharaCore::FreeCameraHandler()
{
    int rv = ERROR_CODE_NO_ERROR;
    QString ErrorMsg;
    KSError ksError = KS_OK;

    if (m_ExchangeMemory->m_HandleStream != KS_INVALID_HANDLE) {
        ksError = KS_installCameraHandler(m_ExchangeMemory->m_HandleStream, KS_CAMERA_IMAGE_RECEIVED, KS_INVALID_HANDLE, 0);
    }
    if (ksError != KS_OK) {
        QString AddErrorMsg = tr("KS_installCameraHandler Failed");
        rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
    }
    if (m_ExchangeMemory->m_HandleStream != KS_INVALID_HANDLE) {
        ksError = KS_closeCameraStream(m_ExchangeMemory->m_HandleStream, 0);
        m_ExchangeMemory->m_HandleStream = KS_INVALID_HANDLE;
    }
    if (ksError != KS_OK) {
        QString AddErrorMsg = tr("KS_closeCameraStream Failed");
        rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
    }
    return rv;
}

int KitharaCore::GetCurrentFifoSize()
{
    if (m_ExchangeMemory)
        return m_ExchangeMemory->m_MeasuringParameter.m_CurrentFifoSize;
    else
        return 0;
}

MeasuringResults* KitharaCore::GetCurrentMeasuringResults()
{
    if (m_ExchangeMemory)
        return &(m_ExchangeMemory->m_MeasuringParameter.m_CurrentMeasuringResults);
    else
        return NULL;
}

MeasuringResultsLiquid* KitharaCore::GetCurrentMeasuringResultsLiqiud()
{
    if (m_ExchangeMemory)
        return &(m_ExchangeMemory->m_MeasuringParameter.m_CurrentMeasuringResultsLiqiud);
    else
        return NULL;
}

MeasuringResults* KitharaCore::GetCurrentAveragedMeasuringResults()
{
    if (m_ExchangeMemory)
        return &(m_ExchangeMemory->m_MeasuringParameter.m_AveragedMeasuringResults);
    else
        return NULL;
}

double KitharaCore::GetCurrentSpeedInMMPerms()
{
    if (m_ExchangeMemory)
        return m_ExchangeMemory->m_MeasuringParameter.m_CurrentMeasuringResults.m_CurrentSpeedInmmPerms;
    else
        return 0.0;
}

void KitharaCore::SetKernelParameter(ExchangeMemory* pKernelParameter)
{
    m_KernelParameter = pKernelParameter;
}

// Wird beim Starten der Software einmalig aufgerufen über die Klasse ImageData
int KitharaCore::InitRealTimeSystem(int CameraID, int DiviceIndex, int NetWorkAdapterID, QString& FileNameCameraSimulation)
{
    int flags, rv = ERROR_CODE_NO_ERROR;
    KSError ksError = KS_OK;
    QString ErrorMsg, CameraStartUpInfo, CameraHarwareID, InfoText, AddErrorMsg;
    void* pExchangeMemory = NULL;

#if _DEBUG
    flags = KSF_NO_CONTEXT | KSF_USER_EXEC;
    pExchangeMemory = (void*)(m_ExchangeMemory);
#else
    flags = KSF_NO_CONTEXT | KSF_KERNEL_EXEC;
    pExchangeMemory = (void*)(m_ExchangeMemory->pSysPtr);
#endif
    // set error code text for mixer
    m_ExchangeMemory->m_EtherCATConfigData.m_ErrorCodesetMotorVoltage = KSERROR_CATEGORY_USER + 0x00770000;
    m_ExchangeMemory->m_EtherCATConfigData.m_ErrorCodesetAdressSTMControl = KSERROR_CATEGORY_USER + 0x00710000;
    m_ExchangeMemory->m_EtherCATConfigData.m_ErrorCodesetAdressSTMSTMStatus = KSERROR_CATEGORY_USER + 0x00720000;
    m_ExchangeMemory->m_EtherCATConfigData.m_ErrorCodesetAdressSTMVelocity = KSERROR_CATEGORY_USER + 0x00730000;
    m_ExchangeMemory->m_EtherCATConfigData.m_ErrorCodesetOperationMode = KSERROR_CATEGORY_USER + 0x00740000;
    m_ExchangeMemory->m_EtherCATConfigData.m_ErrorCodesetStartType = KSERROR_CATEGORY_USER + 0x00750000;
    m_ExchangeMemory->m_EtherCATConfigData.m_ErrorCodeRestoreMotorSettings = KSERROR_CATEGORY_USER + 0x00760000;
    m_ExchangeMemory->m_EtherCATConfigData.m_ErrorCodeSetMotorWatchDog = KSERROR_CATEGORY_USER + 0x00780000;
    m_ExchangeMemory->m_EtherCATConfigData.m_ErrorCodesetMotorCurrent = KSERROR_CATEGORY_USER + 0x00790000;
    m_ExchangeMemory->m_EtherCATConfigData.m_ErrorCodeSetVelocityMin = KSERROR_CATEGORY_USER + 0x00700000;
    m_ExchangeMemory->m_EtherCATConfigData.m_ErrorCodesetAdressENCStatusCompact = KSERROR_CATEGORY_USER + 0x00800000;

    KS_addErrorString(m_ExchangeMemory->m_EtherCATConfigData.m_ErrorCodesetMotorVoltage, "Error Set Motor Voltage", KSLNG_DEFAULT);
    KS_addErrorString(m_ExchangeMemory->m_EtherCATConfigData.m_ErrorCodesetOperationMode, "Error Set Operation Mode", KSLNG_DEFAULT);
    KS_addErrorString(m_ExchangeMemory->m_EtherCATConfigData.m_ErrorCodesetStartType, "Error Set StartType", KSLNG_DEFAULT);
    KS_addErrorString(m_ExchangeMemory->m_EtherCATConfigData.m_ErrorCodeRestoreMotorSettings, "Error Can Not Restore Motor Settings", KSLNG_DEFAULT);
    KS_addErrorString(m_ExchangeMemory->m_EtherCATConfigData.m_ErrorCodeSetMotorWatchDog, "Error Can Not Set Mixer WatchDog", KSLNG_DEFAULT);  // not in use
    KS_addErrorString(m_ExchangeMemory->m_EtherCATConfigData.m_ErrorCodesetMotorCurrent, "Error Set Motor Current", KSLNG_DEFAULT);
    KS_addErrorString(m_ExchangeMemory->m_EtherCATConfigData.m_ErrorCodeSetVelocityMin, "Error Set Velocity Min", KSLNG_DEFAULT);

    KS_addErrorString(m_ExchangeMemory->m_EtherCATConfigData.m_ErrorCodesetAdressSTMControl, "Error Set Adress STM Control", KSLNG_DEFAULT);
    KS_addErrorString(m_ExchangeMemory->m_EtherCATConfigData.m_ErrorCodesetAdressSTMSTMStatus, "Error Set  Adress STM Status", KSLNG_DEFAULT);
    KS_addErrorString(m_ExchangeMemory->m_EtherCATConfigData.m_ErrorCodesetAdressSTMVelocity, "Error Set  Adress STM Velocity", KSLNG_DEFAULT);
    KS_addErrorString(m_ExchangeMemory->m_EtherCATConfigData.m_ErrorCodesetAdressENCStatusCompact, "Error Set Adress ENC Status Compact", KSLNG_DEFAULT);

    if (m_IsKitharaDriverOpen) {  // 1. Schritt: Kamera Initialisieren, Verbinden und dann die Kameraparameter setzen
        if (!m_WorkWithoutCamera) {
            rv = SearchConnectConfigureRealTimeCamera(CameraID, DiviceIndex);
        } else {
            rv = CreateSharedMemoryForCameraSimulation(FileNameCameraSimulation);  // Speicherbereich für die Kamerasimulation anlegen und mit der Videodatei füllen
        }
        m_ExchangeMemory->m_CameraSimulationOn = m_WorkWithoutCamera;           // Simulation Camera
        m_ExchangeMemory->m_LiquidFlowSimulationOn = m_LiquidFlowSimulationOn;  // Simulation Injektion
        m_ExchangeMemory->m_EtherCatSimulation = m_WorkWithoutEtherCat;         // Simulation Ethercat
        if (rv == ERROR_CODE_NO_ERROR) {
            if (GetImageData()) {
                CameraStartUpInfo = tr("Info! Used Camera Resolution: %1 x %2  ").arg(GetImageData()->GetImageWidth()).arg(GetImageData()->GetImageHeight());
                CameraStartUpInfo += tr("Max Camera Resolution: %1 x %2").arg(m_MaxImageWidth).arg(m_MaxImageHeight);
            }
            SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
            // 2. Schritt: Anlegen der verschiedenen Speicherblöcke für die unterschiedlichen Bilddaten(Livebild, Video und Bild Verschmutzunggrad
            rv = CreateSharedMemory();
            if (rv == ERROR_CODE_NO_ERROR) {
                // 3. Schritt: Die Adressen der angelegten Bildspeicherbereiche(Zusätzlich noch der Speicherbereich für die Kamerasimulation, wenn gefordert) werden jetzt in der
                // Echtzeittask über die Funktion SetSharedMemoryPointer gesetzt, in der Klasse KitharaVisionDLL.cpp
                CameraStartUpInfo = tr("Info! CreateSharedMemory: OK");
                SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                ksError = KS_execKernelFunction(m_HandleKernel, "SetSharedMemoryPointer", pExchangeMemory, NULL, flags);
                if (ksError == KS_OK) {
                    m_OpenCvVersion = m_ExchangeMemory->m_BufferInfoText;  // Auslesen der Aktuellen OpenCV Version aus dem Echtzeitteil
                    CameraStartUpInfo = tr("Info! SetSharedMemoryPointer In Real Time Context: OK");
                    rv = CreateEvents();  // Die Events für den Signalaustauch zwischen Echtzeit und Windows erzeuegen
                    if (rv == ERROR_CODE_NO_ERROR) {
                        CameraStartUpInfo = tr("Info! CreateEvents: OK");
                        SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                        rv = CreateCallBacks();  // Erzeugen der einzelenen Funktionen in dem Echtzeitteil für den Bildeinzug für das Messen, Schreiben und Lesen der Daten
                        if (rv == ERROR_CODE_NO_ERROR) {
                            CameraStartUpInfo = tr("Info! CreateCallBacks: OK");
                            SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                            rv = InitCameraHandler();
                            if (rv == ERROR_CODE_NO_ERROR) {
                                CameraStartUpInfo = tr("Info! InitCameraHandler: OK");
                                SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                                rv = CreateRealTimeTasks();
                                if (rv == ERROR_CODE_NO_ERROR) {
                                    CameraStartUpInfo = tr("Info! CreateRealTimeTasks: OK");
                                    SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                                    rv = InitEtherCatMasterAndSlave(InfoText, NetWorkAdapterID);  // Ethercat master and slave
                                    if (rv == ERROR_CODE_NO_ERROR) {
                                        CameraStartUpInfo = tr("Info! InitEtherCatMasterAndSlave: OK");
                                        SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                                        ksError = KS_execKernelFunction(m_HandleKernel, "InitEtherCatDataSet", pExchangeMemory, NULL, flags);
                                        if (ksError == KS_OK) {
                                            CameraStartUpInfo = tr("Info! InitEtherCatDataSet: OK");
                                            SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);
                                        } else {  // Hier keine Verbindung zu den Klemmen, wenn Simulation an dann Fehlermeldung unterdrücken
                                            if (!m_WorkWithoutEtherCat) {
                                                AddErrorMsg = tr("KS_execKernelFunction InitEtherCatDataSet Failed");
                                                rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                } else {
                    AddErrorMsg = tr("KS_execKernelFunction SetSharedMemoryPointer Failed");
                    rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
                }
            }
        }
    } else {
        ErrorMsg = tr("Error: Can Not Init Real Time Camera, Kithara Driver Is Not Open");
        SendMessageToGUI(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
        rv = ERROR_CODE_ANY_ERROR;
    }
    m_IsInitWent = true;
    return rv;
}

void KitharaCore::StartRealTimeTasks()
{
    StartRealTimeIOTask();
    if (m_WorkWithoutCamera) StartRealTimeCameraSimulationTask();
    StartRealTimeMeasureTask();
}

void KitharaCore::StartRealTimeIOTask()
{
    if (m_ExchangeMemory->m_EtherCATConfigData.m_HandleIOTask != KS_INVALID_HANDLE) {
        // KS_triggerTask(m_ExchangeMemory->m_EtherCATConfigData.m_HandleIOTask);
        KS_startTimer(m_ExchangeMemory->m_EtherCATConfigData.m_HandleIOTask, 0, 0.1 * ms);
    }
}

void KitharaCore::StartRealTimeMeasureTask()
{
    if (m_ExchangeMemory->m_HandleMeasuringTask != KS_INVALID_HANDLE) KS_triggerTask(m_ExchangeMemory->m_HandleMeasuringTask);
}

void KitharaCore::StartRealTimeCameraSimulationTask()
{
    if (m_ExchangeMemory->m_HandleCameraSimulationTask != KS_INVALID_HANDLE) KS_triggerTask(m_ExchangeMemory->m_HandleCameraSimulationTask);
}

int KitharaCore::SearchConnectConfigureRealTimeCamera(int CameraNumber, int DeviceIndex)
{
    int rv = ERROR_CODE_NO_ERROR;
    QString CameraHardwareID;

    rv = SearchCameraDevices(CameraNumber, DeviceIndex, CameraHardwareID);
    if (rv == ERROR_CODE_NO_ERROR) {
        rv = _ConnectAndConfigureCamera(CameraHardwareID);
    }
    return rv;
}

int KitharaCore::StartCameraAcquisition()
{
    QString ErrorMsg;
    KSError ksError = KS_OK;

    if (m_ExchangeMemory) {
        if (m_ExchangeMemory->m_HandleCamera != KS_INVALID_HANDLE) ksError = KS_startCameraAcquisition(m_ExchangeMemory->m_HandleCamera, KS_CAMERA_CONTINUOUS, 0);
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_startCameraAcquisition Failed");
            return GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        } else
            return ERROR_CODE_NO_ERROR;
    } else {
        ErrorMsg = tr("Error! Can Not Start Image Acquisition. No Shared Data");
        SendMessageToGUI(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
        return ERROR_CODE_ANY_ERROR;
    }
    return ERROR_CODE_NO_ERROR;
}

int KitharaCore::StopCameraAcquisition()
{
    QString ErrorMsg;
    KSError ksError = KS_OK;

    if (m_ExchangeMemory) {
        if (m_ExchangeMemory->m_HandleCamera != KS_INVALID_HANDLE) ksError = KS_stopCameraAcquisition(m_ExchangeMemory->m_HandleCamera, 0);
        if (ksError != KS_OK) {
            QString AddErrorMsg = tr("KS_stopCameraAcquisition Failed");
            return GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        } else
            return ERROR_CODE_NO_ERROR;
    } else {
        ErrorMsg = tr("Error! Can Not Stop Image Acquisition. No Shared Data");
        SendMessageToGUI(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
        return ERROR_CODE_ANY_ERROR;
    }
    return ERROR_CODE_NO_ERROR;
}

void KitharaCore::ForceSetEventBottleEjected()
{
    if (m_ExchangeMemory && m_ExchangeMemory->m_HandleEventBottleEjected != KS_INVALID_HANDLE) KS_setEvent(m_ExchangeMemory->m_HandleEventBottleEjected);
}

void KitharaCore::ForceSetEventGetNewImage()
{
    if (m_ExchangeMemory && m_ExchangeMemory->m_HandleImageReceivedEvent != KS_INVALID_HANDLE) KS_setEvent(m_ExchangeMemory->m_HandleImageReceivedEvent);
}

void KitharaCore::ForceSetEventInputOutput()
{
    if (m_ExchangeMemory && m_ExchangeMemory->m_HandleEventInputOutput != KS_INVALID_HANDLE) KS_setEvent(m_ExchangeMemory->m_HandleEventInputOutput);
}

void KitharaCore::ForceSetEventCanCopyVideoData()
{
    if (m_ExchangeMemory && m_ExchangeMemory->m_HandleEventCanCopyVideoData != KS_INVALID_HANDLE) KS_setEvent(m_ExchangeMemory->m_HandleEventCanCopyVideoData);
}

int KitharaCore::OpenRealTimeCamera(QString& CameraHardwareID)
{
    int rv = ERROR_CODE_NO_ERROR;
    QString ErrorMsg, AddErrorMsg;
    std::string CameraHardWarIDSTDString = CameraHardwareID.toStdString();
    const char* CameraName = CameraHardWarIDSTDString.c_str();
    KSError ksError = KS_OK;

    ksError = KS_openCamera(&(m_ExchangeMemory->m_HandleCamera), m_ExchangeMemory->m_HandleController, CameraName, 0);
    if (ksError != KS_OK) {
        AddErrorMsg = tr("KS_openCamera failed! Camera Name:%1").arg(CameraName);
        rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
        m_ExchangeMemory->m_HandleCamera = KS_INVALID_HANDLE;
    }
    return rv;
}

int KitharaCore::GetNumberDedicatedCPUs(QString& CameraStartUpInfo, QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    KSError ksError = KS_OK;
    KSSystemInformation systemInfo;
    systemInfo.structSize = sizeof(KSSystemInformation);

    ksError = KS_getSystemInformation(&systemInfo, 0);
    if (ksError == KS_OK) {
        CameraStartUpInfo += tr("Info! Number Of CPUs In The PC: %1").arg(systemInfo.numberOfCPUs);
        CameraStartUpInfo += tr(" Which Are Used By Windows: %1 Number Of Dedicated CPUs: %2").arg(systemInfo.numberOfSharedCPUs).arg(systemInfo.numberOfCPUs - systemInfo.numberOfSharedCPUs);

    } else {
        QString AddErrorMsg = tr("KS_getSystemInformation Failed");
        rv = GetKitharaErrorMsg(ksError, ErrorMsg, AddErrorMsg);
    }
    return rv;
}

int KitharaCore::CalculateTimeStampOffsetBetweenCameraAndRealTimeClock()
{
    int rv = ERROR_CODE_NO_ERROR;
    int64 TimeStampCamera;
    QString NodeName = "TimestampLatch";
    QString Value = "Execute";
    KSError ksError = KS_OK;

    m_ExchangeMemory->m_MeasuringParameter.m_TimeStampOffsetIn100nsUnits = 0;
    rv = SetStringNodeCameraValue(NodeName, Value);
    if (rv == 0) {
        NodeName = "TimestampLatchValue";
        rv = GetInt64NodeCameraValue(NodeName, TimeStampCamera);
        if (rv == 0) {
            int64 TimeStampSystem;
            int64 TimeStampCamInt64;
            KS_getClock(&TimeStampSystem, KS_CLOCK_MEASURE_HIGHEST);
            KS_convertClock(&TimeStampSystem, KS_CLOCK_MEASURE_HIGHEST, KS_CLOCK_MACHINE_TIME, 0);
            TimeStampCamInt64 = static_cast<int64>(TimeStampCamera / 100.0);  // in 100ns units
            m_ExchangeMemory->m_MeasuringParameter.m_TimeStampOffsetIn100nsUnits = TimeStampCamInt64 - TimeStampSystem;

            QString CameraStartUpInfo = tr("Info! CalculateTimeStampOffset TimestampOffset[100ns]:%1: OK").arg(m_ExchangeMemory->m_MeasuringParameter.m_TimeStampOffsetIn100nsUnits);
            SendMessageToGUI(CameraStartUpInfo, QtMsgType::QtInfoMsg);  // ERROR_CODE_NO_ERROR);
        }
    }
    return rv;
}

void KitharaCore::GetInfoMsgIODevice(QString& InfoMsg)
{
    if (m_ExchangeMemory) {
        InfoMsg = m_ExchangeMemory->m_BufferInfoText;
    }
}

ImageData* KitharaCore::GetImageData()
{
    return GetMainAppCrystalT2()->GetImageData();
}

void KitharaCore::FreeAllKithara()
{
    FreeKithara();
    Sleep(100);
    if (m_IsKitharaDriverOpen) {
        if (CloseKitharaDriver() == 0) {
            QString InfoMsg = tr("Info! Close Real Time Driver: OK");
            SendMessageToGUI(InfoMsg, QtMsgType::QtInfoMsg);  // ERROR_CODE_NO_ERROR);
        }
    }
    m_KernelParameter = NULL;
    m_PointerImageBlock = NULL;
    m_PointerVideoBlock = NULL;
    m_PointerCleanImageBlock = NULL;
}

void KitharaCore::SetEdgeThreshold(int set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_MeasuringParameter.m_EdgeThreshold = set;
}

void KitharaCore::SetFilterKernelSize(int set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_MeasuringParameter.m_FilterKernelSize = set;
}

int KitharaCore::SetExposureTime(double set)
{
    return SetFloatNodeCameraValue(QString("ExposureTime"), set);
}

void KitharaCore::SetReferenceMeasurePositionInPixel(int set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_MeasuringParameter.m_ReferenceMeasurePositionInPixel = set;
}

void KitharaCore::SetReferenceInjectionPositionInPixel(int set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_MeasuringParameter.m_ReferenceInjectionPositionInPixel = set;
}

void KitharaCore::SetDistanceBetweenMeasurePosAndTriggerPosInPixel(double set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_MeasuringParameter.m_DistanceBetweenMeasureMiddlePosAndTriggerPosInPixel = set;
}

void KitharaCore::SetRollingMeanValueLiquid(int set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_MeasuringParameter.m_RollingMeanValueLiquid = set;
}

void KitharaCore::SetImageBackgroundContrast(int set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_MeasuringParameter.m_ImageBackgroundContrast = set;
}

void KitharaCore::SetPollingTimeIOTaskInms(double set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_EtherCATConfigData.m_CyclusTimeIOTaskInms = set;
}

void KitharaCore::SetTimePeriodTriggerOutputOnInms(double set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_EtherCATConfigData.m_TimePeriodTriggerOutputOnInms = set;
}

void KitharaCore::SetTimePeriodDigitalOutputOnInms(double set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_EtherCATConfigData.m_TimePeriodDigitalOutputOnInms = set;
}

void KitharaCore::SetBotteleNeckDiameterToleranceInmm(double set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_MeasuringParameter.m_BotteleNeckDiameterToleranceInmm = set;
}

void KitharaCore::SetCurrentPixelSizeInMMPerPixel(double set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_MeasuringParameter.m_PixelSizeInMMPerPixel = set;
}

void KitharaCore::SetTriggerOffsetInmm(double set, int ValveID)
{
    if (m_ExchangeMemory) {
        if (ValveID == LEFT_VALVE_ID) {
            if (m_ExchangeMemory->m_MeasuringParameter.m_BandDirectional == BAND_DIRECTIONAL_RIGHT_TO_LEFT) {
                m_ExchangeMemory->m_MeasuringParameter.m_TriggerOffsetSecondValveInmm = set;
            } else {
                m_ExchangeMemory->m_MeasuringParameter.m_TriggerOffsetFirstValveInmm = set;
            }
        } else {
            if (ValveID == RIGHT_VALVE_ID) {
                if (m_ExchangeMemory->m_MeasuringParameter.m_BandDirectional == BAND_DIRECTIONAL_RIGHT_TO_LEFT) {
                    m_ExchangeMemory->m_MeasuringParameter.m_TriggerOffsetFirstValveInmm = set;
                } else {
                    m_ExchangeMemory->m_MeasuringParameter.m_TriggerOffsetSecondValveInmm = set;
                }
            }
        }
    }
}

void KitharaCore::SetUseSpeedFromISCalcEjectionTime(bool set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_MeasuringParameter.m_UseSpeedFromISCalcEjectionTime = set;
}

void KitharaCore::SetFormatFromISInmm(double set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_MeasuringParameter.m_FormatFromISInmm = set;
}

void KitharaCore::SetBandDirectional(int set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_MeasuringParameter.m_BandDirectional = set;
}

void KitharaCore::SetChangeTriggerOutputOrder(bool set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_MeasuringParameter.m_RightTriggerIsFirst = set;
}

void KitharaCore::SetCurrentMaschineState(int set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_MaschineState = set;
}

void KitharaCore::SetEnableTrigger(bool set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_EnableTrigger = set;
}

void KitharaCore::SetInfoLevel(int set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_MeasuringParameter.m_InfoLevel = set;
}

int KitharaCore::GetDebugCounter()
{
    if (m_ExchangeMemory)
        return m_ExchangeMemory->m_DebugCounter;
    else
        return -1;
}

int KitharaCore::GetInfoLevel()
{
    if (m_ExchangeMemory)
        return m_ExchangeMemory->m_MeasuringParameter.m_InfoLevel;
    else
        return 0;
}

void KitharaCore::SetVideoStateCameraSimulation(int set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_VideoStateCameraSimulation = set;
}

void KitharaCore::SetMeasureWindowPosToRealTime(int Index, int xoff, int yoff, int w, int h)
{
    if (m_ExchangeMemory) {
        m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowOffsetX[Index] = xoff;
        m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowOffsetY[Index] = yoff;
        m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowWidth[Index] = w;
        m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowHeight[Index] = h;
    }
}

void KitharaCore::SetCalibrateModus(bool set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_MeasuringParameter.m_CalibrateModus = set;
}

bool KitharaCore::GetCalibrateModus()
{
    if (m_ExchangeMemory)
        return m_ExchangeMemory->m_MeasuringParameter.m_CalibrateModus;
    else
        return false;
}

void KitharaCore::SetUsedTriggerOutput(int Value)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_MeasuringParameter.m_UsedTriggerOutputs = Value;
}

void KitharaCore::SetTargetBottleneckDiameter(double set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_MeasuringParameter.m_BottleneckDiameter = set;
}

void KitharaCore::SetProductWidth(double set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_MeasuringParameter.m_ProductWidthInmm = set;
}

void KitharaCore::SetAcceptanceThresholdLiquidLeftAndRightROI(int set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_MeasuringParameter.m_AcceptanceThresholdLiquidLeftAndRightROI = set;
}

void KitharaCore::SetMinAcceptanceThresholdLiquidMiddleROI(int set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_MeasuringParameter.m_MinAcceptanceThresholdLiquidMiddleROI = set;
}

void KitharaCore::SetMaxAcceptanceThresholdLiquidMiddleROI(int set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_MeasuringParameter.m_MaxAcceptanceThresholdLiquidMiddleROI = set;
}

void KitharaCore::SetDigitalOutput(EtherCATConfigData::IOChannels Channel, bool Value)
{
    if (m_ExchangeMemory && m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[Channel].m_LengthInByte > 0) {
        if (Value)
            m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[Channel].m_FirstByte] |=
                (static_cast<unsigned char>(m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[Channel].m_ChannelNumber));
        else
            m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[Channel].m_FirstByte] &=
                ~(static_cast<unsigned char>(m_ExchangeMemory->m_EtherCATConfigData.m_IOChannels[Channel].m_ChannelNumber));
    }
}
// nur für die Simulation
void KitharaCore::SetENCStatusMixerSimulation(ushort set)
{
    if (m_ExchangeMemory) {
        memcpy(m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerENCStatusCompact.m_ENCStatusCompact, &set, sizeof(ushort));
        // m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerENCStatusCompact.m_ENCStatusCompact[0] = set;
    }
}

// unsigned char* KitharaCore::GetENCStatusMixer()
//{
//    if (m_ExchangeMemory) {
//        return m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerENCStatus.m_ENCStatus;
//    }
//    return NULL;
//}

unsigned char* KitharaCore::GetENCStatusCompactMixer()
{
    if (m_ExchangeMemory) {
        return m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerENCStatusCompact.m_ENCStatusCompact;
    }
    return NULL;
}
// nur für die Simulation
void KitharaCore::SetStatusMixerSimulation(ushort set)
{
    if (m_ExchangeMemory) {
        m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerSTMStatus.stmStatus = set;
    }
}
ushort KitharaCore::GetStatusMixer()
{
    if (m_ExchangeMemory) {
        return m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerSTMStatus.stmStatus;
    }
    return 0;
}

// unsigned char* KitharaCore::GetPOSStatusCompact()
//{
//    if (m_ExchangeMemory) {
//        return m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerPOSStatus.POSStatus;
//    }
//    return NULL;
//}

// ushort KitharaCore::GetCounterMixer()
//{
//    if (m_ExchangeMemory) {
//        return m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerENCStatusCompact.CounterValue;
//    }
//    return 0;
//}

// ushort KitharaCore::GetActualVelocityMixer()
//{
//    if (m_ExchangeMemory) {
//        return m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerPOSStatus.ActualVelocity;
//    }
//    return 0;
//}
//
// ushort KitharaCore::GetPOSStatusMixer()
//{
//    if (m_ExchangeMemory) {
//        return m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerPOSStatus.POSStatus;
//    }
//    return 0;
//}

// unsigned char* KitharaCore::GetMixerPOSStatus()
//{
//    if (m_ExchangeMemory) {
//        return m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerPOSStatus.POSStatus;
//    }
//    return NULL;
//}

// unsigned char* KitharaCore::GetMixerInfoData()
//{
//    if (m_ExchangeMemory) {
//        return m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerInfoData.m_infoData;
//    }
//    return NULL;
//}

unsigned char* KitharaCore::GetMixerInternalPosition()
{
    if (m_ExchangeMemory) {
        return m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerPositionData.m_InternalPosition;
    }
    return NULL;
}

unsigned long long KitharaCore::GetMixerInternalPositionTimeStamp()
{
    if (m_ExchangeMemory) {
        return m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerPositionData.m_CurrentTimeStamp;
    }
    return 0;
}

unsigned char* KitharaCore::GetMixerExternalPosition()
{
    if (m_ExchangeMemory) {
        return m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerPositionData.m_ExternalPosition;
    }
    return NULL;
}

// short KitharaCore::GetMixerInfoData1()
//{
//    if (m_ExchangeMemory) {
//        return m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerInfoData.m_infoData1;
//    }
//    return 0;
//}
//
// short KitharaCore::GetMixerInfoData2()
//{
//    if (m_ExchangeMemory) {
//        return m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerInfoData.m_infoData2;
//    }
//    return 0;
//}

// nur für die Simulation
void KitharaCore::SetActualVelocityMixer(ushort set)
{
    /*if (m_ExchangeMemory) {
        m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerPOSStatus.ActualVelocity = set;
    }*/
}

int KitharaCore::GetDigitalIOIndex(const QString& TerminalTypeName, const QString& ChannelName, int& BitPosition)
{
    BitPosition = 1;
    for (int i = 0; i < m_ListEtherCatSlaveData.count(); i++) {
        if (m_ListEtherCatSlaveData.at(i).m_TerminalType == TerminalTypeName) {
            int ChannelNumber = m_ListEtherCatSlaveData.at(i).m_ListChannelNumbers.value(ChannelName, -1);
            if (ChannelNumber != -1) {
                BitPosition = BitPosition << ChannelNumber;
                return i;
            }
        }
    }
    return -1;
}

int KitharaCore::GetAnalogIOIndex(const QString& TerminalTypeName, const QString& ChannelName, int& ChannelOffset)
{
    ChannelOffset = 0;
    for (int i = 0; i < m_ListEtherCatSlaveData.count(); i++) {
        if (m_ListEtherCatSlaveData.at(i).m_TerminalType == TerminalTypeName) {
            int ChannelNumber = m_ListEtherCatSlaveData.at(i).m_ListChannelNumbers.value(ChannelName, -1);
            if (ChannelNumber != -1) {
                int BytesPerChannel = m_ListEtherCatSlaveData.at(i).m_NumberBitsPerChannel / 8;
                ChannelOffset = BytesPerChannel * ChannelNumber;
                return i;
            }
        }
    }
    return -1;
}

bool KitharaCore::SetAnalogOutputValue(const QString& ChannelName, short Value)
{
    QString TerminaleTypeName = "AnalogOutput";
    int ChannelOffset;
    int index = GetAnalogIOIndex(TerminaleTypeName, ChannelName, ChannelOffset);
    bool rv = true;

    if (m_ExchangeMemory && index >= 0) {
        unsigned char HighByte = static_cast<unsigned char>(Value & 0x00FF);
        unsigned char LowByte = static_cast<unsigned char>(Value >> 8);
        m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[m_ListEtherCatSlaveData.at(index).m_FirstByte + ChannelOffset] = HighByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[m_ListEtherCatSlaveData.at(index).m_FirstByte + ChannelOffset + 1] = LowByte;
    } else
        rv = false;
    return rv;
}

bool KitharaCore::SetAnalogInputValue(const QString& ChannelName, short Value)  // Nur für Tests
{
    QString TerminaleTypeName = "AnalogInput";
    int ChannelOffset;
    int index = GetAnalogIOIndex(TerminaleTypeName, ChannelName, ChannelOffset);
    bool rv = true;

    if (m_ExchangeMemory && index >= 0) {
        unsigned char HighByte = static_cast<unsigned char>(Value & 0x00FF);
        unsigned char LowByte = static_cast<unsigned char>(Value >> 8);
        m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[m_ListEtherCatSlaveData.at(index).m_FirstByte + ChannelOffset] = HighByte;
        m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[m_ListEtherCatSlaveData.at(index).m_FirstByte + ChannelOffset + 1] = LowByte;
    } else
        rv = false;
    return rv;
}

bool KitharaCore::GetAnalogInputValue2Byte(const QString& ChannelName, short& Value)
{
    QString TerminaleTypeName = "AnalogInput";
    int ChannelOffset;
    int index = GetAnalogIOIndex(TerminaleTypeName, ChannelName, ChannelOffset);
    bool rv = true;
    short Status = 0;

    if (m_ExchangeMemory && index >= 0) {
        Value = 0;
        unsigned char Byte1 = m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[m_ListEtherCatSlaveData.at(index).m_FirstByte + ChannelOffset];
        unsigned char Byte2 = m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[m_ListEtherCatSlaveData.at(index).m_FirstByte + ChannelOffset + 1];
        unsigned char Byte3 = m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[m_ListEtherCatSlaveData.at(index).m_FirstByte + ChannelOffset + 2];
        unsigned char Byte4 = m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[m_ListEtherCatSlaveData.at(index).m_FirstByte + ChannelOffset + 3];

        if (m_ListEtherCatSlaveData.at(index).m_NumberBitsPerChannel == 24) {  // 3-Byte Interface
            Value = (Value << 8) + Byte3;
            Value = (Value << 8) + Byte2;
        } else {  // 4-Byte Interface
            Status = (Status << 8) + Byte2;
            Status = (Status << 8) + Byte1;
            Value = (Value << 8) + Byte4;
            Value = (Value << 8) + Byte3;
            if (Status & 0x2) rv = false;  // Messbereich überschritten, Leitungsbruch, keine Verbindung, nicht angeschlossen
        }
    } else
        rv = false;
    return rv;
}

bool KitharaCore::SetAnalogOutputValue2Byte(const QString& ChannelName, short& Value)
{
    QString TerminaleTypeName = "AnalogOutput";
    int ChannelOffset;
    int index = GetAnalogIOIndex(TerminaleTypeName, ChannelName, ChannelOffset);
    bool rv = true;

    if (m_ExchangeMemory && index >= 0) {
        unsigned char Byte1, Byte2, Byte3, Byte4;
        unsigned char LowByte = static_cast<unsigned char>(Value & 0x00FF);
        unsigned char HighByte = static_cast<unsigned char>(Value >> 8);
        Byte1 = Byte2 = Byte3 = Byte4 = 0;
        if (m_ListEtherCatSlaveData.at(index).m_NumberBitsPerChannel == 24) {  // 3-Byte Interface
            // Value = (Value << 8) + Byte3;
            // Value = (Value << 8) + Byte2;

            Byte2 = LowByte;
            Byte3 = HighByte;
        } else {  // 4-Byte Interface
            // Value = (Value << 8) + Byte4;
            // Value = (Value << 8) + Byte3;
            Byte3 = LowByte;
            Byte4 = HighByte;
        }
        m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[m_ListEtherCatSlaveData.at(index).m_FirstByte + ChannelOffset] = Byte1;
        m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[m_ListEtherCatSlaveData.at(index).m_FirstByte + ChannelOffset + 1] = Byte2;
        m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[m_ListEtherCatSlaveData.at(index).m_FirstByte + ChannelOffset + 2] = Byte3;
        if (m_ListEtherCatSlaveData.at(index).m_NumberBitsPerChannel == 32) {
            m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[m_ListEtherCatSlaveData.at(index).m_FirstByte + ChannelOffset + 3] = Byte4;
        }
    } else
        rv = false;
    return rv;
}

bool KitharaCore::SetAnalogInputValue2Byte(const QString& ChannelName, short& Value)  // Nur für Tests
{
    QString TerminaleTypeName = "AnalogInput";
    int ChannelOffset;
    int index = GetAnalogIOIndex(TerminaleTypeName, ChannelName, ChannelOffset);
    bool rv = true;

    if (m_ExchangeMemory && index >= 0) {
        unsigned char Byte1, Byte2, Byte3, Byte4;
        unsigned char LowByte = static_cast<unsigned char>(Value & 0x00FF);
        unsigned char HighByte = static_cast<unsigned char>(Value >> 8);
        Byte1 = Byte2 = Byte3 = Byte4 = 0;
        if (m_ListEtherCatSlaveData.at(index).m_NumberBitsPerChannel == 24) {  // 3-Byte Interface
            // Value = (Value << 8) + Byte3;
            // Value = (Value << 8) + Byte2;

            Byte2 = LowByte;
            Byte3 = HighByte;
        } else {  // 4-Byte Interface
            // Value = (Value << 8) + Byte4;
            // Value = (Value << 8) + Byte3;
            Byte3 = LowByte;
            Byte4 = HighByte;
        }
        m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[m_ListEtherCatSlaveData.at(index).m_FirstByte + ChannelOffset] = Byte1;
        m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[m_ListEtherCatSlaveData.at(index).m_FirstByte + ChannelOffset + 1] = Byte2;
        m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[m_ListEtherCatSlaveData.at(index).m_FirstByte + ChannelOffset + 2] = Byte3;
        if (m_ListEtherCatSlaveData.at(index).m_NumberBitsPerChannel == 32) {
            m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[m_ListEtherCatSlaveData.at(index).m_FirstByte + ChannelOffset + 3] = Byte4;
        }
    } else
        rv = false;
    return rv;
}

bool KitharaCore::GetAnalogInputValue4Byte(QString& ChannelName, int& Value)
{
    QString TerminaleTypeName = "AnalogInput";
    int ChannelOffset;
    int index = GetAnalogIOIndex(TerminaleTypeName, ChannelName, ChannelOffset);
    bool rv = true;

    if (m_ExchangeMemory && index >= 0) {
        Value = 0;
        unsigned char HighByte = m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[m_ListEtherCatSlaveData.at(index).m_FirstByte + ChannelOffset + 2];
        unsigned char LowByte = m_ExchangeMemory->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[m_ListEtherCatSlaveData.at(index).m_FirstByte + ChannelOffset + 3];

        Value = (Value << 8) + LowByte;
        Value = (Value << 8) + HighByte;
    } else
        rv = false;
    return rv;
}
// Livebild
unsigned char* KitharaCore::GetRawImagedata()
{
    KSError ksError = KS_getSharedMemEx(m_ExchangeMemory->m_HandleImageBlock, (void**)&m_PointerImageBlock, 0);
    if (ksError == KS_OK)
        return m_PointerImageBlock;
    else
        return NULL;
}
// Bild um Verschmutzungsgrad zu bestimmen
unsigned char* KitharaCore::GetRawCleanImagedata()
{
    KSError ksError = KS_getSharedMemEx(m_ExchangeMemory->m_HandleCleanImageBlock, (void**)&m_PointerCleanImageBlock, 0);
    if (ksError == KS_OK)
        return m_PointerCleanImageBlock;
    else
        return NULL;
}
// Laufende Kamerabilder/Video Triggerbilder
unsigned char* KitharaCore::GetRawVideodata()
{
    if (m_ExchangeMemory->m_HandleVideoBlock != KS_INVALID_HANDLE) {
        KSError ksError = KS_getSharedMemEx(m_ExchangeMemory->m_HandleVideoBlock, (void**)&m_PointerVideoBlock, 0);
        if (ksError == KS_OK)
            return m_PointerVideoBlock;
        else
            return NULL;
    } else
        return NULL;
}
// videospeicher für Flaschen die Ausgeworfen wurden, Zeigt den Befüllvorgan
unsigned char* KitharaCore::GetRawVideodataBottleEjected()
{
    if (m_ExchangeMemory->m_HandleVideoBlockFillingProcess != KS_INVALID_HANDLE) {
        KSError ksError = KS_getSharedMemEx(m_ExchangeMemory->m_HandleVideoBlockFillingProcess, (void**)&m_PointerVideoBlockFillingProcess, 0);
        if (ksError == KS_OK)
            return m_PointerVideoBlockFillingProcess;
        else
            return NULL;
    } else
        return NULL;
}

bool KitharaCore::IsReadVideoFromRealTimeContextOk()
{
    if (m_ExchangeMemory)
        return m_ExchangeMemory->m_MeasuringParameter.m_ReadVideoFromRealTimeContext;
    else
        return false;
}

void KitharaCore::SetReadVideoFromRealTimeContextOk(bool set)
{
    if (m_ExchangeMemory) {
        m_ExchangeMemory->m_MeasuringParameter.m_ReadVideoFromRealTimeContext = set;
    }
}

void KitharaCore::TriggerGetNewVideoFromRealTimeContext()
{  // info an den real time context aufnahme in den videospeicher stoppen um video vom realtime context zu kopieren
    if (m_ExchangeMemory) {
        m_ExchangeMemory->m_MeasuringParameter.m_TriggerGetNewVideoFromRealTimeContext = true;
    }
}

double KitharaCore::GetProductPresentTime(double SpeedInMMPerSecond)
{
    double PresentTime = -1.0;                          // can not calculate, no band movement
    if (m_ExchangeMemory && SpeedInMMPerSecond > 16.6)  // Minimale Geschwindigkei 1 m/min
    {
        double DistanceInMM = m_ExchangeMemory->m_MeasuringParameter.m_PixelSizeInMMPerPixel * GetImageData()->GetImageWidth();
        double TimeInSec = DistanceInMM / SpeedInMMPerSecond;
        PresentTime = TimeInSec * 1000.0;
    }
    return PresentTime;
}

void KitharaCore::SetClearVideoBuffer()
{
    if (m_ExchangeMemory) {
        m_ExchangeMemory->m_MeasuringParameter.m_ClearVideoBuffer = true;
    }
}

void KitharaCore::SetDistanceBottleEjection(double set)
{
    if (m_ExchangeMemory) {
        m_ExchangeMemory->m_MeasuringParameter.m_DistanceBottleEjectionInmm = set;
    }
}

void KitharaCore::SetInjectonWindowMiddleWidthInMm(double set)
{
    if (m_ExchangeMemory) {
        m_ExchangeMemory->m_MeasuringParameter.m_InjectonWindowMiddleWidthInMm = set;
    }
}

void KitharaCore::SetDistancesBetweenValves(double set)
{
    if (m_ExchangeMemory) m_ExchangeMemory->m_MeasuringParameter.m_DistancesBetweenValves = set;
}

void KitharaCore::SetMinSpeedInMMPerMs(double set)
{
    if (m_ExchangeMemory) {
        m_ExchangeMemory->m_MeasuringParameter.m_MinSpeedInMMPerMs = set;
    }
}

void KitharaCore::SetMaxMeasurementsProductIsOutOfTol(int set)
{
    if (m_ExchangeMemory) {
        m_ExchangeMemory->m_MeasuringParameter.m_MaxMeasurementsProductIsOutOfTol = set;
    }
}

void KitharaCore::SetThresholdBinaryImageLiquid(int set)
{
    if (m_ExchangeMemory) {
        m_ExchangeMemory->m_MeasuringParameter.m_ThresholdBinaryImageLiquid = set;
    }
}

void KitharaCore::SetNumberProductsAverageBottleNeckAndPixelSize(int set)
{
    if (m_ExchangeMemory) {
        m_ExchangeMemory->m_MeasuringParameter.m_NumberProductsAverageBottleNeckAndPixelSize = set;
    }
}

void KitharaCore::SetMinNumberFoundedInROI(int set)
{
    if (m_ExchangeMemory) {
        m_ExchangeMemory->m_MeasuringParameter.m_MinNumberFoundedInROI = set;
    }
}

void KitharaCore::SetManualTrigger(int UsedTriggerOutput)
{
    if (m_ExchangeMemory) {
        m_ExchangeMemory->m_SetManualTrigger = true;
        m_ExchangeMemory->m_UseManualTriggerOutputs = UsedTriggerOutput;
    }
}

void KitharaCore::SetButtonIsClickedEjectTheNextnBottles()
{
    if (m_ExchangeMemory) {
        m_ExchangeMemory->m_ButtonIsClickedEjectTheNextnBottles = true;
    }
}

void KitharaCore::SetNumberEjectedBottlesByUser(int set)
{
    if (m_ExchangeMemory) {
        m_ExchangeMemory->m_NumberEjectedBottlesByUser = set;
    }
}

// void KitharaCore::EnableTriggerOutputs(bool on)
//{
//    if (m_ExchangeMemory) {
//        m_ExchangeMemory->m_EnableTriggerOutputs = true;
//    }
//}
