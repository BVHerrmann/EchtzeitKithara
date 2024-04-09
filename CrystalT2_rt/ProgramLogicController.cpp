#include "ProgramLogicController.h"
#include <Windows.h>
#include "ImageData.h"

ProgramLogicController::ProgramLogicController(ImageData* pImageData) : m_ImageData(NULL), m_LastClockSignaleFromIS(false), m_LastTimeStamp(0), m_LastStateEjectionControl(false)
{
    m_ImageData = pImageData;
}

ProgramLogicController::~ProgramLogicController()
{
}

void ProgramLogicController::RunPLC(ExchangeMemory* SharedData, unsigned long long TimeStampIn100NanoUnits)
{
    bool StateControlVoltage = GetDigitalIOState(SharedData, EtherCATConfigData::DI_CONTROL_VOLTAGE);
    bool EjectionControl = GetDigitalIOState(SharedData, EtherCATConfigData::DI_EJECTION_CONTROL);

    // Check Control Voltage
    if (!StateControlVoltage) {                                                    // steuerspannung ist ausgefallen
        if (!GetDigitalIOState(SharedData, EtherCATConfigData::DO_ERROR_LIGHT)) {  // Störmeldung ist nicht an
            // set error light on, no control voltage
            TimeStampsSetDO Data1, Data2;
            Data1.m_TimeStamp = TimeStampIn100NanoUnits;
            Data1.m_SetOn = true;
            Data1.m_ChannelIndex = EtherCATConfigData::DO_ERROR_LIGHT;
            m_ListSetDigtalOutputs.push_back(Data1);
            // set error transfer on, no control voltage
            Data2.m_ChannelIndex = EtherCATConfigData::DO_ERROR_TRANSFER;
            Data2.m_TimeStamp = TimeStampIn100NanoUnits;
            Data2.m_SetOn = false;
            m_ListSetDigtalOutputs.push_back(Data2);
        }
    }
    // Check Ejection Control
    if (EjectionControl) {
        if (!m_LastStateEjectionControl) {
            TimeStampsSetDO Data;
            Data.m_TimeStamp = TimeStampIn100NanoUnits;
            Data.m_SetOn = true;
            Data.m_ChannelIndex = EtherCATConfigData::DO_COUNTER_EJECTION_TRANSFER;
            m_ListSetDigtalOutputs.push_back(Data);
            Data.m_TimeStamp = TimeStampIn100NanoUnits + static_cast<unsigned long long>(SharedData->m_EtherCATConfigData.m_TimePeriodDigitalOutputOnInms * ms);
            Data.m_SetOn = false;
            m_ListSetDigtalOutputs.push_back(Data);
            SharedData->m_CounterNumberBottlesRealEjected++;
        }
    }
    m_LastStateEjectionControl = EjectionControl;
    // Check Trigger
    TriggerTimeReached(SharedData, TimeStampIn100NanoUnits);  // set trigger and bottle injection
    // Check ejection
    EjectionTimeReached(SharedData, TimeStampIn100NanoUnits);  // wird nur gesetzt wenn SharedData->m_MaschineState == OPERATION_STATUS_RELEASE_PRODUCTION
    // Check Speed from IS
    CalculateSpeedFromIS(SharedData, TimeStampIn100NanoUnits);
    // Set other digital outputs
    SetDigitalOutputTimeReached(SharedData, TimeStampIn100NanoUnits);

    if (SharedData->m_EtherCATConfigData.m_ExecuteMixer) {
        SharedData->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerSTMControl.stmControl = 1;  // Velocity control
    } else {
        SharedData->m_EtherCATConfigData.m_EtherCatDataSet.m_MixerSTMControl.stmControl = 0;  // Velocity control
    }
}

void ProgramLogicController::CalculateSpeedFromIS(ExchangeMemory* SharedData, unsigned long long TimeStamp)
{
    bool ClockSignalFromIS = GetDigitalIOState(SharedData, EtherCATConfigData::DI_CLOCK_SIGNAL_FROM_IS);

    if (m_LastClockSignaleFromIS == false && ClockSignalFromIS)  // rising flank
    {
        if (m_LastTimeStamp == 0)
            m_LastTimeStamp = TimeStamp;
        else {
            SharedData->m_EtherCATConfigData.m_DeltaTFromIS = TimeStamp - m_LastTimeStamp;
            m_LastTimeStamp = 0;
        }
    }
    m_LastClockSignaleFromIS = ClockSignalFromIS;
}

bool ProgramLogicController::GetDigitalIOState(ExchangeMemory* SharedData, EtherCATConfigData::IOChannels Channel)
{
    if (SharedData->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[SharedData->m_EtherCATConfigData.m_IOChannels[Channel].m_FirstByte] &
        SharedData->m_EtherCATConfigData.m_IOChannels[Channel].m_ChannelNumber)
        return true;
    else
        return false;
}

void ProgramLogicController::SetDigitalOutput(ExchangeMemory* SharedData, bool Value, EtherCATConfigData::IOChannels Channel)
{
    if (SharedData->m_EtherCATConfigData.m_IOChannels[Channel].m_LengthInByte > 0) {
        if (Value)
            SharedData->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[SharedData->m_EtherCATConfigData.m_IOChannels[Channel].m_FirstByte] |=
                (static_cast<unsigned char>(SharedData->m_EtherCATConfigData.m_IOChannels[Channel].m_ChannelNumber));
        else
            SharedData->m_EtherCATConfigData.m_EtherCatDataSet.EtherCatData[SharedData->m_EtherCATConfigData.m_IOChannels[Channel].m_FirstByte] &=
                ~(static_cast<unsigned char>(SharedData->m_EtherCATConfigData.m_IOChannels[Channel].m_ChannelNumber));
    }
}

void ProgramLogicController::SetDigitalOutputTimeReached(ExchangeMemory* SharedData, unsigned long long TimeStamp)
{
    for (std::list<TimeStampsSetDO>::iterator it = m_ListSetDigtalOutputs.begin(); it != m_ListSetDigtalOutputs.end();) {
        if (TimeStamp >= it->m_TimeStamp) {
            SetDigitalOutput(SharedData, it->m_SetOn, it->m_ChannelIndex);
            it = m_ListSetDigtalOutputs.erase(it);
        } else
            ++it;
    }

    for (std::list<TimeStampsSetDO>::iterator it = m_ListDigtalOutputsWhiteLight.begin(); it != m_ListDigtalOutputsWhiteLight.end();) {
        if (TimeStamp >= it->m_TimeStamp) {
            SetDigitalOutput(SharedData, it->m_SetOn, it->m_ChannelIndex);
            it = m_ListDigtalOutputsWhiteLight.erase(it);
        } else
            ++it;
    }
}

void ProgramLogicController::EjectionTimeReached(ExchangeMemory* SharedData, unsigned long long TimeStampIn100ns)
{
    if (m_ImageData) {
        bool EjectionValue = !(SharedData->m_MeasuringParameter.m_BlowOutEjectorNormallyClosed);  // wenn Auswerfer NotClosed dann Ausgang auf false(0) wenn Produktion nicht freigegeben
        if (SharedData->m_MaschineState == OPERATION_STATUS_RELEASE_PRODUCTION) {
            unsigned long long EjectionPointTimeInns = m_ImageData->GetFrontEjectionPointInTimeInns();
            if (EjectionPointTimeInns > 0) {
                unsigned long long CurrentTimeStampInns = TimeStampIn100ns * 100;
                if (CurrentTimeStampInns >= EjectionPointTimeInns) {
                    EjectionValue = m_ImageData->GetFrontEjectionSetOrReset();                            // flag we want set digital output(1/true) or reset(0/false)
                    SetDigitalOutput(SharedData, EjectionValue, EtherCATConfigData::DO_BOTTLE_EJECTION);  // set digital output channel on (1 or 0) in shared memory
                    m_ImageData->DeleteFrontEjectionPointInTime();                                        // remove from FIFO
                }
            }
        } else {
            SetDigitalOutput(SharedData, EjectionValue, EtherCATConfigData::DO_BOTTLE_EJECTION);
        }
        if (EjectionValue) {  // nicht blasen
            if (m_ListDigtalOutputsWhiteLight.size() > 0) m_ListDigtalOutputsWhiteLight.clear();

            SetDigitalOutput(SharedData, false, EtherCATConfigData::DO_WHITE_LIGHT);  // white light off
            TimeStampsSetDO Data;
            Data.m_ChannelIndex = EtherCATConfigData::DO_WHITE_LIGHT;
            Data.m_TimeStamp = TimeStampIn100ns + static_cast<unsigned long long>(SharedData->m_MeasuringParameter.m_TimePeriodNotBlowInms * ms);  // Time how long off
            Data.m_SetOn = true;                                                                                                                   // Aus bedeutet hier Digital an, da Normally Closed
            m_ListDigtalOutputsWhiteLight.push_back(Data);

            SharedData->m_InfoCodeInputOutputDevice = INFO_CODE_INPUT_OUTPUT_DEVICE_BLOW_OUT_EJECTOR_IS_NOT_ACTIVE;
            KS_setEvent((KSHandle)(SharedData)->m_HandleEventInputOutput);  // Info an die GUI
        }
    }
    return;
}

void ProgramLogicController::TriggerTimeReached(ExchangeMemory* SharedData, unsigned long long TimeStamp)
{
    double TriggerPointTimeInms, CurrentTimeStampInms, TriggerDriftInms;  // TriggerDriftInMM;
    double HalfPollingTimeIOTaskInms = SharedData->m_EtherCATConfigData.m_CyclusTimeIOTaskInms / 2.0;
    bool TriggerSetOrReset;
    int ChannelNumber;

    if (m_ImageData) {
        TriggerPointTimeInms = m_ImageData->GetFrontTriggerPointInTimeInms();
        if (TriggerPointTimeInms >= 0.0) {
            CurrentTimeStampInms = TimeStamp / ((double)(ms));
            TriggerDriftInms = CurrentTimeStampInms - TriggerPointTimeInms;
            if (CurrentTimeStampInms >= TriggerPointTimeInms || fabs(TriggerDriftInms) < HalfPollingTimeIOTaskInms) {  // Set trigger
                TriggerSetOrReset = m_ImageData->TriggerSetOrReset();                                                  // flag we want set digital output(1/true) or reset(0/false)
                ChannelNumber = m_ImageData->GetFrontTriggerChannelNumber();
                if (SharedData->m_EnableTrigger) {
                    if (SharedData->m_MaschineState == OPERATION_STATUS_RELEASE_PRODUCTION || SharedData->m_MaschineState == OPERATION_STATUS_SETUP_ENABLE_TRIGGER ||
                        SharedData->m_MaschineState == OPERATION_STATUS_ERROR)  // auch im Fehlerfall versuchen das Ventil anzusteueren
                    {
                        if (ChannelNumber == 1) {
                            SetDigitalOutput(SharedData, TriggerSetOrReset, EtherCATConfigData::DO_TRIGGER1_VALVE);  // set digital output channel on (1 or 0) in shared memory
                        } else {
                            if (ChannelNumber == 2) {
                                SetDigitalOutput(SharedData, TriggerSetOrReset, EtherCATConfigData::DO_TRIGGER2_VALVE);
                            }
                        }
                        if (TriggerSetOrReset) {
                            if (ChannelNumber == 1) {
                                m_ImageData->SetLeftTriggerIsSet(true);  // info an Bildverarbeitungstask Startpunkt um die Flüssigkeitsmenge zu bestimmen
                                SharedData->m_InfoLeftTriggerIsSet = true;// info an Windows App GUI
                            } else {
                                if (ChannelNumber == 2) {
                                    m_ImageData->SetRightTriggerIsSet(true);  // info an Bildverarbeitungstask Startpunkt um die Flüssigkeitsmenge zu bestimmen
                                    SharedData->m_InfoRightTriggerIsSet = true;// info an Windows App GUI
                                }
                            }
                            if (ChannelNumber == 1 || ChannelNumber == 2) KS_setEvent((KSHandle)(SharedData)->m_HandleEventInputOutput);  // event for GUI App info trigger is set
                        }
                    }
                } else {
                    SharedData->m_InfoCodeInputOutputDevice = INFO_CODE_NO_PREASURE;
                    KS_setEvent((KSHandle)(SharedData)->m_HandleEventInputOutput);  // Info for GUI can not trigger no preasure is on
                }
                m_ImageData->DeleteFrontTriggerPointInTime();  // remove trigger data from FIFO
            }
        }
        if (SharedData->m_MaschineState == OPERATION_STATUS_SETUP_DISABLE_TRIGGER) {
            SetDigitalOutput(SharedData, false, EtherCATConfigData::DO_TRIGGER1_VALVE);  // set Trigger off
            SetDigitalOutput(SharedData, false, EtherCATConfigData::DO_TRIGGER2_VALVE);  // set Trigger off
        }
    }
    return;
}

void ProgramLogicController::GetCurrentTimeStampIn100nsUnits(ExchangeMemory* SharedData, unsigned long long& TimeStamp)
{
    int64 CurrentTimeStampIn100nsUnits = 0;
    KS_getClock(&CurrentTimeStampIn100nsUnits, KS_CLOCK_MEASURE_HIGHEST);
    KS_convertClock(&CurrentTimeStampIn100nsUnits, KS_CLOCK_MEASURE_HIGHEST, KS_CLOCK_MACHINE_TIME, 0);
    TimeStamp = static_cast<unsigned long long>(CurrentTimeStampIn100nsUnits) + SharedData->m_MeasuringParameter.m_TimeStampOffsetIn100nsUnits;
}

void ProgramLogicController::DebugFormat(const char* format, ...)
{
    static char s_printf_buf[512];
    va_list args;
    va_start(args, format);
    _vsnprintf_s(s_printf_buf, sizeof(s_printf_buf), format, args);
    va_end(args);
    OutputDebugStringA(s_printf_buf);
}
