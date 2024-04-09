#include "InputOutputEvents.h"
#include "GlobalConst.h"
#include "KitharaCore.h"
#include "MainAppCrystalT2.h"
#include "ProductData.h"
#include "SettingsData.h"

InputOutputEvents::InputOutputEvents(MainAppCrystalT2* pMainAppCrystalT2) : QThread(), m_TerminateEventsInpuOutput(false), m_MainAppCrystalT2(NULL)
{
    m_MainAppCrystalT2 = pMainAppCrystalT2;
    m_previousInternalPosition = 0;
    m_previousTimestampInternalPositionInms = 0;

    m_counter_sum = new accumulator_set<int32_t, stats<tag::rolling_sum>>(tag::rolling_window::window_size = 5);
    m_delay_sum = new accumulator_set<int64_t, stats<tag::rolling_sum>>(tag::rolling_window::window_size = 5);
}

KitharaCore* InputOutputEvents::GetKitharaCore()
{
    if (GetMainAppCrystalT2())
        return GetMainAppCrystalT2()->GetKitharaCore();
    else
        return NULL;
}

InputOutputEvents::~InputOutputEvents()
{
    FinishedThread();
}

void InputOutputEvents::FinishedThread()
{
    if (isRunning()) {  // thread läuft noch
        bool rv;
        m_TerminateEventsInpuOutput = true;
        if (GetKitharaCore()) GetKitharaCore()->ForceSetEventInputOutput();
        m_MutexForEventInputOutput.lock();
        rv = m_WaitConditionEventInputOutput.wait(&m_MutexForEventInputOutput, 2000);
        m_MutexForEventInputOutput.unlock();
        msleep(0);
        if (isRunning()) terminate();
    }
}

void InputOutputEvents::run()
{
    QString KitharaErrorMsg, Msg;
    int rv;
    int count = 0;
    double LastEtherCATCycleTime = 0.0;

    while (true) {
        QApplication::processEvents();
        if (m_TerminateEventsInpuOutput) break;
        if (GetKitharaCore()) {
            rv = GetKitharaCore()->WaitForNextInpuOutput(2000, Msg);
            if (rv == ERROR_CODE_NO_ERROR) {
                if (m_TerminateEventsInpuOutput) break;
                // check is EtherCAT network cable connected
                if (LastEtherCATCycleTime != GetKitharaCore()->GetEtherCatCycelTimeInms()) {
                    LastEtherCATCycleTime = GetKitharaCore()->GetEtherCatCycelTimeInms();
                    Msg = QString("%1").arg(LastEtherCATCycleTime, 0, 'f', 5);
                    emit SignalShowInfo(Msg, INFO_CODE_ETHER_CAT_CYCLUS);
                    count = 0;
                } else {  // error EtherCAT is disconnected
                    count++;
                    if (count > 10) {
                        Msg = tr("EtherCAT IO Device Is Not Connected");
                        emit SignalShowInfo(Msg, INFO_CODE_INPUT_OUTPUT_DEVICE_ANY_ERROR);
                        count = 0;
                    }
                }

                if (GetKitharaCore()->GetInfoCodeIsLeftTriggerSet()) {
                    emit SignalShowInfo(tr("On"), INFO_CODE_INPUT_OUTPUT_DEVICE_LEFT_TRIGGER_IS_SET);
                    GetKitharaCore()->SetInfoCodeIsLeftTriggerSet(false);
                }
                if (GetKitharaCore()->GetInfoCodeIsRightTriggerSet()) {
                    emit SignalShowInfo(tr("On"), INFO_CODE_INPUT_OUTPUT_DEVICE_RIGHT_TRIGGER_IS_SET);
                    GetKitharaCore()->SetInfoCodeIsRightTriggerSet(false);
                }
                switch (GetKitharaCore()->GetInfoCodeIODevice()) {
                    case INFO_CODE_INPUT_OUTPUT_DEVICE_BLOW_OUT_EJECTOR_IS_ACTIVE:
                        emit SignalShowInfo(tr("On"), INFO_CODE_INPUT_OUTPUT_DEVICE_BLOW_OUT_EJECTOR_IS_ACTIVE);
                        break;
                   /* case INFO_CODE_INPUT_OUTPUT_DEVICE_LEFT_TRIGGER_IS_SET:
                        emit SignalShowInfo(tr("On"), INFO_CODE_INPUT_OUTPUT_DEVICE_LEFT_TRIGGER_IS_SET);
                        break;
                    case INFO_CODE_INPUT_OUTPUT_DEVICE_RIGHT_TRIGGER_IS_SET:
                        emit SignalShowInfo(tr("On"), INFO_CODE_INPUT_OUTPUT_DEVICE_RIGHT_TRIGGER_IS_SET);
                        break;*/
                    case INFO_CODE_INPUT_OUTPUT_DEVICE_ANY_ERROR:
                        GetKitharaCore()->GetInfoMsgIODevice(Msg);
                        emit SignalShowInfo(Msg, INFO_CODE_INPUT_OUTPUT_DEVICE_ANY_ERROR);
                        break;
                    case INFO_CODE_INPUT_DEVICE_KITHARA_ERROR_CODE:
                        GetKitharaCore()->GetInfoMsgIODevice(Msg);
                        GetKitharaCore()->GetKitharaErrorMsg(GetKitharaCore()->GetKitharaErrorCodeFromRealTimeContext(), KitharaErrorMsg, Msg);
                        emit SignalShowInfo(KitharaErrorMsg, INFO_CODE_INPUT_DEVICE_KITHARA_ERROR_CODE);
                        break;
                    case INFO_CODE_INPUT_DATA_STATUS:
                        if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
                            ShowIsControlPowerOn();
                            ShowSpeedFromISMaschine();
                            ShowPreasureTankTemperature();
                            ShowHeatingPipeTemperature();
                            ShowPreasure();
                            ShowFillingLevel();
                            ShowAirCoolingCamera();
                            ShowlAirCoolingCameraLight();
                            ShowlAirCoolingValves();
                            ShowWaterCooling();
                            ShowFlowTransmitterWaterCoolingCircuit();
                            ShowTemperaturWaterCoolingReturn();
                            ShowCounterNumberBottlesRealEjected();
                            ShowStatusMixer();
                        }
                        if (GetKitharaCore()->IsMeasuringTaskStarted()) {
                            Msg = tr("Measuring Task OK!");
                            emit SignalShowInfo(Msg, INFO_CODE_INPUT_OUTPUT_REAL_TIME_TASKS_RUNS);
                            GetKitharaCore()->SetMeasuringTaskStarted(false);
                        }
                        // if (GetKitharaCore()->IsIOTaskStarted()) {
                        //    Msg = tr("IO Task OK!");
                        //    //emit SignalShowInfo(Msg, INFO_CODE_INPUT_OUTPUT_REAL_TIME_TASKS_RUNS);
                        //    //GetKitharaCore()->SetIOTaskStarted(false);
                        //}
                        break;
                    case INFO_CODE_DEBUG_INFO:
                        GetKitharaCore()->GetInfoMsgIODevice(Msg);
                        emit SignalShowDebugInfo(Msg, ERROR_CODE_NO_ERROR);
                        break;
                    case INFO_CODE_MANUAL_EJECTION_READY:
                        emit SignalShowInfo(Msg, INFO_CODE_MANUAL_EJECTION_READY);
                        break;
                    case INFO_CODE_NO_PREASURE:  //Ínfo muss nicht weiterverarbeitet werden, da bei Druckabfall schon ein Alarm ausgelöst worden ist.
                        // emit SignalShowInfo(Msg, INFO_CODE_NO_PREASURE);
                        break;
                    default:
                        break;
                        GetKitharaCore()->SetInfoCodeIODevice(INFO_CODE_INPUT_OUTPUT_DEVICE_NO_INFO);
                }
            } else {
                if (rv == ERROR_CODE_ANY_ERROR) {
                    msleep(500);
                } else {  // timeout
                    if (!GetMainAppCrystalT2()->GetSettingsData()->m_WorkWithoutEtherCat && GetMainAppCrystalT2()->IsStartupInitReady()) {
                        Msg = tr("Error! IO Task Is Not Running");
                        emit SignalShowInfo(Msg, INFO_CODE_INPUT_OUTPUT_DEVICE_ANY_ERROR);
                    }
                }
            }
        } else {
            emit SignalShowInfo(tr("Real Time Application Is Not Active! Error Pointer Zero"), INFO_CODE_INPUT_OUTPUT_DEVICE_ANY_ERROR);
            break;
        }
    }
    m_MutexForEventInputOutput.lock();
    m_WaitConditionEventInputOutput.wakeAll();
    m_MutexForEventInputOutput.unlock();
}

void InputOutputEvents::ShowStatusMixer()
{
    if (GetKitharaCore() && m_delay_sum && m_counter_sum) {
        MixerSTMStatus STMStaus;                 // 0x1A03
        MixerENCStatusCompact ENCStatusCompact;  // 0x1A00
        uint16_t ENCStatusMixer = 0;
        uint32_t CurrentInternalPosition;
        unsigned long long TimeStampIn100nsUnit = GetKitharaCore()->GetMixerInternalPositionTimeStamp();
        int64_t TimestampInternalPositionInms = static_cast<uint64_t>(TimeStampIn100nsUnit / 10000.0);

        STMStaus.stmStatus = GetKitharaCore()->GetStatusMixer();
        memcpy(ENCStatusCompact.m_ENCStatusCompact, GetKitharaCore()->GetENCStatusCompactMixer(), sizeof(MixerENCStatusCompact));
        memcpy(&ENCStatusMixer, &(ENCStatusCompact.m_ENCStatusCompact[0]), sizeof(uint16_t));
        memcpy(&CurrentInternalPosition, GetKitharaCore()->GetMixerInternalPosition(), sizeof(uint32_t));
        
        /*if (CurrentInternalPosition < m_previousInternalPosition) {
            uint32_t delta = (65535 - m_previousInternalPosition) + CurrentInternalPosition;
            m_counter_sum->operator()(delta);
            
        } else {
            m_counter_sum->operator()((CurrentInternalPosition - m_previousInternalPosition));
        }*/
        m_counter_sum->operator()((CurrentInternalPosition - m_previousInternalPosition));
        m_previousInternalPosition = CurrentInternalPosition;
        m_delay_sum->operator()((int64_t)(TimestampInternalPositionInms - m_previousTimestampInternalPositionInms));
        m_previousTimestampInternalPositionInms = TimestampInternalPositionInms;
        double speed = GetMixerSpeed();
        QString Msg = QString("%1|%2|%3|%4").arg(STMStaus.stmStatus).arg(ENCStatusMixer).arg(speed).arg(speed);
        emit SignalShowInfo(Msg, INFO_CODE_STATUS_MIXER);
    }
}

double InputOutputEvents::GetMixerSpeed()
{
    double speed = 0.0;
    int64_t delay_sum = rolling_sum(*m_delay_sum);
    int32_t counter_sum = rolling_sum(*m_counter_sum);

    if (delay_sum > 0) {
        speed = ((double)counter_sum / delay_sum) *256.0;
    }
    // if (delay_sum > 0 && _increments_per_mm > 0) speed = rolling_sum(*_counter_sum) * 60 / delay_sum / _increments_per_mm;
    return speed;
}

void InputOutputEvents::ShowIsControlPowerOn()
{
    bool DigitalInput = GetKitharaCore()->GetDigitalInput(EtherCATConfigData::DI_CONTROL_VOLTAGE);
    if (!DigitalInput) {
        if (GetMainAppCrystalT2()->IsStartupInitReady()) {
            emit SignalShowInfo(tr("Error: Control Voltage Is Off"), INFO_CODE_INPUT_CONTROL_VOLTAGE_NOT_OK);
        }
    }
}
void InputOutputEvents::ShowSpeedFromISMaschine()
{
    double TimeInMs = GetKitharaCore()->GetDeltaTFromISInms();
    if (TimeInMs > 0.0 && GetMainAppCrystalT2()->GetCurrentProductData()) {
        // im m/min mal zwei da die Zeit noch halbiert werden muss, halbe Periodendauer
        QString Msg = QString("%1").arg(((GetMainAppCrystalT2()->GetCurrentProductData()->m_FormatFromISInmm / TimeInMs) * 60.0) * 2.0);
        emit SignalShowInfo(Msg, INFO_CODE_SPEED_FROM_IS);
    } else {
        QString Msg = QString("-1.0");
        emit SignalShowInfo(Msg, INFO_CODE_SPEED_FROM_IS);
    }
}
void InputOutputEvents::ShowPreasure()
{
    double dValue = -1.0;
    short sValue;
    bool Ok = GetKitharaCore()->GetAnalogInputValue2Byte(QString(AI_NAME_ACTUAL_PREASURE), sValue);
    if (Ok) {
        dValue = GetMainAppCrystalT2()->TerminalValueToPhysicalSize(MainAppCrystalT2::AnalogTerminals::PREASURE_VALUE, sValue);
    }

    QString Msg = QString("%1").arg(dValue);
    emit SignalShowInfo(Msg, INFO_CODE_ACTUAL_LIQUID_TANK_PREASURE);
}
void InputOutputEvents::ShowFillingLevel()
{
    double dValue = -1.0;
    short sValue;
    bool Ok = GetKitharaCore()->GetAnalogInputValue2Byte(QString(AI_NAME_ACTUAL_TANK_FILLING_LEVEL), sValue);
    if (Ok) {
        dValue = GetMainAppCrystalT2()->TerminalValueToPhysicalSize(MainAppCrystalT2::AnalogTerminals::FILLING_LEVEL, sValue);
    }

    QString Msg = QString("%1").arg(dValue);
    emit SignalShowInfo(Msg, INFO_CODE_PREASURE_TANK_FILLING_LEVEL);
}
void InputOutputEvents::ShowPreasureTankTemperature()
{
    double dValue = -1.0;
    short sValue;
    bool Ok = GetKitharaCore()->GetAnalogInputValue2Byte(QString(AI_NAME_TANK_TEMPERATURE), sValue);
    if (Ok) {
        dValue = GetMainAppCrystalT2()->TerminalValueToPhysicalSize(MainAppCrystalT2::AnalogTerminals::PREASURE_TANK_TEMPERATURE, sValue);
    }

    QString Msg = QString("%1").arg(dValue);
    emit SignalShowInfo(Msg, INFO_CODE_PREASURE_TANK_TEMP);
}
void InputOutputEvents::ShowHeatingPipeTemperature()
{
    double dValue = -1.0;
    short sValue;
    bool Ok = GetKitharaCore()->GetAnalogInputValue2Byte(QString(AI_NAME_HEATING_PIPE_TEMPERATURE), sValue);
    if (Ok) {
        dValue = GetMainAppCrystalT2()->TerminalValueToPhysicalSize(MainAppCrystalT2::AnalogTerminals::HEATING_PIPE_TEMPERATURE, sValue);
    }

    QString Msg = QString("%1").arg(dValue);
    emit SignalShowInfo(Msg, INFO_CODE_HEATING_PIPE_TEMP);
}
void InputOutputEvents::ShowAirCoolingCamera()
{
    double dValue = -1.0;
    short sValue;
    bool Ok = GetKitharaCore()->GetAnalogInputValue2Byte(QString(AI_NAME_ACTUAL_AIR_COOLING_CAMERA_AND_BACK_LIGHT), sValue);
    if (Ok) {
        dValue = GetMainAppCrystalT2()->TerminalValueToPhysicalSize(MainAppCrystalT2::AnalogTerminals::AIR_COOLING_CAMERA, sValue);
    }

    QString Msg = QString("%1").arg(dValue);
    emit SignalShowInfo(Msg, INFO_CODE_ACTUAL_AIR_COOLING_CAMERA);
}
void InputOutputEvents::ShowlAirCoolingCameraLight()
{
    double dValue = -1.0;
    short sValue;
    bool Ok = GetKitharaCore()->GetAnalogInputValue2Byte(QString(AI_NAME_ACTUAL_AIR_COOLING_GLASS), sValue);
    if (Ok) {
        dValue = GetMainAppCrystalT2()->TerminalValueToPhysicalSize(MainAppCrystalT2::AnalogTerminals::AIR_COOLING_CAMERA_LIGHT, sValue);
    }

    QString Msg = QString("%1").arg(dValue);
    emit SignalShowInfo(Msg, INFO_CODE_ACTUAL_AIR_COOLING_LIGHT);
}
void InputOutputEvents::ShowlAirCoolingValves()
{
    double dValue = -1.0;
    short sValue;
    bool Ok = GetKitharaCore()->GetAnalogInputValue2Byte(QString(AI_NAME_ACTUAL_AIR_COOLING_VALVE), sValue);
    if (Ok) {
        dValue = GetMainAppCrystalT2()->TerminalValueToPhysicalSize(MainAppCrystalT2::AnalogTerminals::AIR_COOLING_VALVES, sValue);
    }

    QString Msg = QString("%1").arg(dValue);
    emit SignalShowInfo(Msg, INFO_CODE_ACTUAL_AIR_COOLING_VALVE);
}
void InputOutputEvents::ShowWaterCooling()
{
    double dValue = -1.0;
    short sValue;
    bool Ok = GetKitharaCore()->GetAnalogInputValue2Byte(QString(AI_NAME_ACTUAL_WATER_COOLING), sValue);
    if (Ok) {
        dValue = GetMainAppCrystalT2()->TerminalValueToPhysicalSize(MainAppCrystalT2::AnalogTerminals::WATER_COOLING, sValue);
    }

    QString Msg = QString("%1").arg(dValue);
    emit SignalShowInfo(Msg, INFO_CODE_ACTUAL_WATER_COOLING);
}
void InputOutputEvents::ShowFlowTransmitterWaterCoolingCircuit()
{
    double dValue = -1.0;
    short sValue;
    bool Ok = GetKitharaCore()->GetAnalogInputValue2Byte(QString(AI_NAME_ACTUAL_FLOW_TRANSMITTER_WATER_COOLING_CIRCUIT), sValue);
    if (Ok) {
        dValue = GetMainAppCrystalT2()->TerminalValueToPhysicalSize(MainAppCrystalT2::AnalogTerminals::FLOW_TRANSMITTER_WATER_COOLING_CIRCUIT, sValue);
    }
    QString Msg = QString("%1").arg(dValue);
    emit SignalShowInfo(Msg, INFO_CODE_ACTUAL_FLOW_TRANSMITTER_WATER_COOLING_CIRCUIT);
}
void InputOutputEvents::ShowTemperaturWaterCoolingReturn()
{
    double dValue = -1.0;
    short sValue;
    bool Ok = GetKitharaCore()->GetAnalogInputValue2Byte(QString(AI_NAME_ACTUAL_TEMPERATURE_WATER_COOLING_RETURN), sValue);
    if (Ok) {
        dValue = GetMainAppCrystalT2()->TerminalValueToPhysicalSize(MainAppCrystalT2::AnalogTerminals::TEMPERATURE_WATER_COOLING_RETURN, sValue);
    }
    QString Msg = QString("%1").arg(dValue);
    emit SignalShowInfo(Msg, INFO_CODE_ACTUAL_TEMPERATURE_WATER_COOLING_RETURN);
}

void InputOutputEvents::ShowCounterNumberBottlesRealEjected()
{
    QString Msg = QString("%1").arg(GetKitharaCore()->GetCounterNumberBottlesRealEjected());
    emit SignalShowInfo(Msg, INFO_CODE_COUNTER_REAL_EJECTED);
}
