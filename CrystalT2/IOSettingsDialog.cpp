#include "IOSettingsDialog.h"
#include "ControlsWithColorStatus.h"
#include "GlobalConst.h"
#include "ImageData.h"
#include "MainAppCrystalT2.h"
#include "SettingsData.h"
#include "bswitch.h"

#include <audittrail.h>

IOSettingsDialog::IOSettingsDialog(MainAppCrystalT2* pMainAppCrystalT2)
    : QWidget(pMainAppCrystalT2),

      m_TimerShowDigitalOutputs(NULL),
      m_SwitchCameraLight(NULL),
      m_SwitchSetTrigger1Valve(NULL),
      m_SwitchSetTrigger2Valve(NULL),
      m_SwitchSlotErrorLight(NULL),
      m_SwitchSlotErrorTransfer(NULL),
      m_SwitchCounterEjectionTransfer(NULL),
      m_SwitchPressureTanksHeater(NULL),
      m_SwitchPressureTanksValve(NULL),
      m_SwitchValveController(NULL),
      m_SwitchBottleEjection(NULL),
      m_SwitchSetControlVoltage(NULL),
      m_SwitchEjetionIS(NULL),
      m_SwitchOrangeLightWarning(NULL),
      m_SwitchRealEjection(NULL),
      m_LastOffsetAnalogInputTankFillingLevel(0.0),
      m_LastFactorAnalogInputTankFillingLevel(1.0),
      m_LastOffsetAnalogInputAirCoolingCamera(0.0),
      m_LastFactorAnalogInputAirCoolingCamera(1.0),
      m_LastOffsetAnalogInputAirCoolingCameraLight(0.0),
      m_LastFactorAnalogInputAirCoolingCameraLight(1.0),
      m_LastOffsetAnalogInputAirCoolingValves(0.0),
      m_LastFactorAnalogInputAirCoolingValves(1.0),
      m_LastOffsetAnalogInputWaterCooling(0.0),
      m_LastFactorAnalogInputWaterCooling(1.0),
      m_LastOffsetAnalogInputFlowTransmitterWaterCoolingCircuit(0.0),
      m_LastFactorAnalogInputFlowTransmitterWaterCoolingCircuit(1.0),
      m_LastOffsetAnalogInputTemperaturWaterCoolingReturn(0.0),
      m_LastFactorAnalogInputTemperaturWaterCoolingReturn(1.0),
      m_LastOffsetAnalogInputCurrentPreasure(0.0),
      m_LastFactorAnalogInputCurrentPreasure(1.0),
      m_ControlMixerVelocity(NULL),
      m_StatusControlMixerVelocity(ALARM_LEVEL_OK),
      m_NumberErrorMixer(0)
{
    ui.setupUi(this);
    m_testBool = false;
    m_SetupWindow = false;
    m_MainAppCrystalT2 = pMainAppCrystalT2;
    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();

    m_CurrentCalibrateAnalogInputIndex = AnalogInputCh::AI_ACTUAL_TANK_FILLING_LEVEL;
    ui.comboBoxSelectAnalogInputType->insertItem(0, AI_NAME_ACTUAL_AIR_COOLING_CAMERA_AND_BACK_LIGHT);
    ui.comboBoxSelectAnalogInputType->insertItem(1, AI_NAME_ACTUAL_AIR_COOLING_GLASS);
    ui.comboBoxSelectAnalogInputType->insertItem(2, AI_NAME_ACTUAL_AIR_COOLING_VALVE);
    ui.comboBoxSelectAnalogInputType->insertItem(3, AI_NAME_ACTUAL_WATER_COOLING);
    ui.comboBoxSelectAnalogInputType->insertItem(4, AI_NAME_ACTUAL_FLOW_TRANSMITTER_WATER_COOLING_CIRCUIT);
    ui.comboBoxSelectAnalogInputType->insertItem(5, AI_NAME_ACTUAL_TEMPERATURE_WATER_COOLING_RETURN);
    ui.comboBoxSelectAnalogInputType->insertItem(6, AI_NAME_ACTUAL_PREASURE);
    ui.comboBoxSelectAnalogInputType->insertItem(7, AI_NAME_ACTUAL_TANK_FILLING_LEVEL);

    ui.comboBoxSelectAnalogInputType->setCurrentIndex(m_CurrentCalibrateAnalogInputIndex);

    m_SwitchSetControlVoltage = new BSwitch(ui.widgetSetControlVoltage);
    m_SwitchCameraLight = new BSwitch(ui.widgetSetCameraLight);
    m_SwitchSetTrigger1Valve = new BSwitch(ui.widgetSetTrigger1Valve);
    m_SwitchSetTrigger2Valve = new BSwitch(ui.widgetSetTrigger2Valve);
    m_SwitchSlotErrorLight = new BSwitch(ui.widgetSetErrorLightOnOff);
    m_SwitchSlotErrorTransfer = new BSwitch(ui.widgetSetErrorTransfer);
    m_SwitchCounterEjectionTransfer = new BSwitch(ui.widgetSetCounterEjection);
    m_SwitchPressureTanksHeater = new BSwitch(ui.widgetSetPressureTanksHeater);
    m_SwitchPressureTanksValve = new BSwitch(ui.widgetSetPreasureTankValve);
    // m_SwitchPressureTanksValve2 = new BSwitch(ui.widgetSetPreasureTankValve_2);
    m_SwitchValveController = new BSwitch(ui.widgetSetValveController);
    m_SwitchBottleEjection = new BSwitch(ui.widgetSetBottleEjection);
    m_SwitchBlueLightPermanentEjectionIS = new BSwitch(ui.widgetBlueLightPermanentEjectionIS);
    m_SwitchWhiteLightBottleEjection = new BSwitch(ui.widgetWhiteLightBottleEjection);
    m_SwitchEjetionIS = new BSwitch(ui.widgetEjectionIS);
    m_SwitchOrangeLightWarning = new BSwitch(ui.widgetOrangeLightWarning);
    m_SwitchRealEjection = new BSwitch(ui.widgetRealEjection);
    m_MixerPowerOnOffSimulation = new BSwitch(ui.widgetMixerControlPowerOnOffSimulation);
    m_MixerStartStopSimulation = new BSwitch(ui.widgetMixerStartStopSimulation);

    if (pSettingsData) {
        ui.doubleSpinBoxCycleTimeIOTask->setValue(pSettingsData->m_CycleTimeIOTask);
        ui.doubleSpinBoxTimePeriodTriggerOutputActive->setValue(pSettingsData->m_TimePeriodTriggerOutputOnInms);
        ui.doubleSpinBoxTimePeriodDigitalOutputActive->setValue(pSettingsData->m_TimePeriodDigitalOutputOnInms);
        if (!pSettingsData->m_WorkWithoutEtherCat) ui.groupBoxEtherCatSimulation->hide();
    }

    // nur für die Simulation
    ui.doubleSpinBoxActualValvePreasure->setValue(0.95);
    ui.doubleSpinBoxPreasureTankFillingLevel->setValue(3.0);
    ui.doubleSpinBoxPreasureTankTemperature->setValue(33.0);
    ui.doubleSpinBoxHeatingPipeTemperature->setValue(35.0);
    ui.doubleSpinBoxAirCoolingCameraSimulation->setValue(65.0);
    ui.doubleSpinBoxAirCoolingCameraLightSimulation->setValue(63.0);
    ui.doubleSpinBoxAirCoolingValvesSimulation->setValue(61.0);
    ui.doubleSpinBoxWaterCoolingSimulation->setValue(70.0);
    ui.doubleSpinBoxFlowTransmitterWaterCoolingCircuitSimulation->setValue(72.0);
    ui.doubleSpinBoxTemperaturWaterCoolingReturnSimulation->setValue(43.0);
    ui.doubleSpinBoxMixerSpeedSimulation->setValue(18.0);
    // nur für die Simulation
    m_MixerPowerOnOffSimulation->setChecked(true);
    m_SwitchSetControlVoltage->setChecked(true);

    SetAuditTrailProperties();

    connect(ui.comboBoxSelectAnalogInputType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &IOSettingsDialog::SlotSelectAnalogInputTypeChanged);

    connect(ui.doubleSpinBoxCycleTimeIOTask, &QDoubleSpinBox::editingFinished, this, &IOSettingsDialog::SlotCycleTimeIOTaskChanged);
    connect(ui.doubleSpinBoxTimePeriodTriggerOutputActive, &QDoubleSpinBox::editingFinished, this, &IOSettingsDialog::SlotTimePeriodTriggerOutputActive);
    connect(ui.doubleSpinBoxTimePeriodDigitalOutputActive, &QDoubleSpinBox::editingFinished, this, &IOSettingsDialog::SlotTimePeriodDigitalOutputActive);

    connect(m_SwitchCameraLight, &BSwitch::clicked, this, &IOSettingsDialog::SlotCameraLight);
    connect(m_SwitchSetTrigger1Valve, &BSwitch::clicked, this, &IOSettingsDialog::SlotTrigger1Valve);
    connect(m_SwitchSetTrigger2Valve, &BSwitch::clicked, this, &IOSettingsDialog::SlotTrigger2Valve);
    connect(m_SwitchSlotErrorLight, &BSwitch::clicked, this, &IOSettingsDialog::SlotErrorLight);
    connect(m_SwitchSlotErrorTransfer, &BSwitch::clicked, this, &IOSettingsDialog::SlotErrorTransfer);
    connect(m_SwitchCounterEjectionTransfer, &BSwitch::clicked, this, &IOSettingsDialog::SlotCounterEjectionTransfer);
    connect(m_SwitchPressureTanksHeater, &BSwitch::clicked, this, &IOSettingsDialog::SlotPressureTanksHeater);
    connect(m_SwitchPressureTanksValve, &BSwitch::clicked, this, &IOSettingsDialog::SlotPressureTanksValve);
    // connect(m_SwitchPressureTanksValve2, &BSwitch::clicked, this, &IOSettingsDialog::SlotPressureTanksValve2);
    connect(m_SwitchValveController, &BSwitch::clicked, this, &IOSettingsDialog::SlotValveController);
    connect(m_SwitchBottleEjection, &BSwitch::clicked, this, &IOSettingsDialog::SlotBottleEjection);
    connect(m_SwitchBlueLightPermanentEjectionIS, &BSwitch::clicked, this, &IOSettingsDialog::SlotBlueLightPermanentEjectionIS);
    connect(m_SwitchWhiteLightBottleEjection, &BSwitch::clicked, this, &IOSettingsDialog::SlotWhiteLightBottleEjection);
    connect(m_SwitchEjetionIS, &BSwitch::clicked, this, &IOSettingsDialog::SlotEjectionIS);
    connect(m_SwitchOrangeLightWarning, &BSwitch::clicked, this, &IOSettingsDialog::SlotWarningLight);
    // connect(ui.pushButtonGetStatusOutputs, &QPushButton::clicked, this, &IOSettingsDialog::SlotShowDigitalOutputs);

    // Nur für die Simulation
    connect(ui.doubleSpinBoxActualValvePreasure, &QDoubleSpinBox::editingFinished, this, &IOSettingsDialog::SlotActualValvePreasureSimulation);
    connect(ui.doubleSpinBoxPreasureTankFillingLevel, &QDoubleSpinBox::editingFinished, this, &IOSettingsDialog::SlotPreasureTankFillingLevelSimulation);
    connect(ui.doubleSpinBoxPreasureTankTemperature, &QDoubleSpinBox::editingFinished, this, &IOSettingsDialog::SlotPreasureTankTemperatureSimulation);
    connect(ui.doubleSpinBoxHeatingPipeTemperature, &QDoubleSpinBox::editingFinished, this, &IOSettingsDialog::SlotHeatingPipeTemperatureSimulation);
    connect(ui.doubleSpinBoxAirCoolingCameraSimulation, &QDoubleSpinBox::editingFinished, this, &IOSettingsDialog::SlotActualAirCoolingCameraSimulation);
    connect(ui.doubleSpinBoxAirCoolingCameraLightSimulation, &QDoubleSpinBox::editingFinished, this, &IOSettingsDialog::SlotActualAirCoolingCameraLightSimulation);
    connect(ui.doubleSpinBoxAirCoolingValvesSimulation, &QDoubleSpinBox::editingFinished, this, &IOSettingsDialog::SlotActualAirCoolingValvesSimulation);
    connect(ui.doubleSpinBoxWaterCoolingSimulation, &QDoubleSpinBox::editingFinished, this, &IOSettingsDialog::SlotActualWaterCoolingSimulation);
    connect(ui.doubleSpinBoxFlowTransmitterWaterCoolingCircuitSimulation, &QDoubleSpinBox::editingFinished, this, &IOSettingsDialog::SlotActualFlowTransmitterWaterCoolingCircuitSimulation);
    connect(ui.doubleSpinBoxTemperaturWaterCoolingReturnSimulation, &QDoubleSpinBox::editingFinished, this, &IOSettingsDialog::SlotActualTemperaturWaterCoolingReturnSimulation);
    connect(m_SwitchSetControlVoltage, &BSwitch::clicked, this, &IOSettingsDialog::SlotSetControlVotageSimulation);
    connect(m_SwitchRealEjection, &BSwitch::clicked, this, &IOSettingsDialog::SlotSetRealEjectionSimulation);
    connect(m_MixerPowerOnOffSimulation, &BSwitch::clicked, this, &IOSettingsDialog::SlotSetPowerMixerSimulation);
    // connect(ui.doubleSpinBoxMixerSpeedSimulation, &QDoubleSpinBox::editingFinished, this, &IOSettingsDialog::SlotSetMixerSpeedSimulation);
    connect(m_MixerStartStopSimulation, &BSwitch::clicked, this, &IOSettingsDialog::SlotSetMixerStartStopSimulation);
    // Nur für die Simulation

    connect(ui.pushButtonAddValueToListCalibrate, &BSwitch::clicked, this, &IOSettingsDialog::SlotAddToListCalibrate);
    connect(ui.pushButtonCalibrate, &BSwitch::clicked, this, &IOSettingsDialog::SlotCalibrateFillingLevel);
    connect(ui.pushButtonRestoreLastCalibrationData, &BSwitch::clicked, this, &IOSettingsDialog::SlotRestoreLastCalibrationData);
    ui.pushButtonRestoreLastCalibrationData->setEnabled(false);

    connect(ui.pushButtonStartMixer, &QPushButton::clicked, this, &IOSettingsDialog::SlotStartMixer);
    connect(ui.pushButtonStopMixer, &QPushButton::clicked, this, &IOSettingsDialog::SlotStopMixer);

    m_TimerShowDigitalOutputs = new QTimer(this);
    connect(m_TimerShowDigitalOutputs, &QTimer::timeout, this, &IOSettingsDialog::SlotShowDigitalOutputs);
    m_TimerShowDigitalOutputs->setInterval(100);
    bool SmallerAsThreshold = false;
    m_ControlMixerVelocity = new ControlsWithColorStatus(ui.doubleSpinBoxMixerVelocity, SmallerAsThreshold);

    ui.pushButtonStartMixer->hide();
    ui.pushButtonStopMixer->hide();

    m_SetupWindow = true;
}

IOSettingsDialog::~IOSettingsDialog()
{
}

void IOSettingsDialog::SetAuditTrailProperties()
{
    ui.doubleSpinBoxCycleTimeIOTask->setProperty(kAuditTrail, ui.labelCycleTimeIOTask->text());
    ui.doubleSpinBoxTimePeriodTriggerOutputActive->setProperty(kAuditTrail, ui.labelTimePeriodTriggerOutputActive->text());
    ui.doubleSpinBoxTimePeriodDigitalOutputActive->setProperty(kAuditTrail, ui.labelTimePeriodDigitalOutputActive->text());

    m_SwitchCameraLight->setProperty(kAuditTrail, ui.labelCameraLight->text());
    m_SwitchSetTrigger1Valve->setProperty(kAuditTrail, ui.labelTrigger1Valve->text());
    m_SwitchSetTrigger2Valve->setProperty(kAuditTrail, ui.labelTrigger2Valve->text());
    m_SwitchSlotErrorLight->setProperty(kAuditTrail, ui.labelErrorLight->text());
    m_SwitchSlotErrorTransfer->setProperty(kAuditTrail, ui.labelErrorTransfer->text());
    m_SwitchCounterEjectionTransfer->setProperty(kAuditTrail, ui.labelCounterEjectionTransfer->text());
    m_SwitchPressureTanksHeater->setProperty(kAuditTrail, ui.labelPressureTanksHeater->text());
    m_SwitchPressureTanksValve->setProperty(kAuditTrail, ui.labelPressureTanksValve->text());
    // m_SwitchPressureTanksValve2->setProperty(kAuditTrail, ui.labelPressureTanksValve2->text());
    m_SwitchValveController->setProperty(kAuditTrail, ui.labelValveController->text());
    m_SwitchBottleEjection->setProperty(kAuditTrail, ui.labelBottleEjection->text());
    m_SwitchBlueLightPermanentEjectionIS->setProperty(kAuditTrail, ui.labelBlueLightPermanentEjectionIS->text());
    m_SwitchWhiteLightBottleEjection->setProperty(kAuditTrail, ui.labelWhiteLightBottleEjection->text());
    m_SwitchEjetionIS->setProperty(kAuditTrail, ui.labelEjectionIS->text());
    m_SwitchOrangeLightWarning->setProperty(kAuditTrail, ui.labelOrangeLightWarning->text());
}
// wird all 500ms aufgerufen
void IOSettingsDialog::SetActualVelocityMixerRPM(double ActualVelocityMixerRPM)
{
    if (m_ControlMixerVelocity && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        bool MaschineStopInSec = true;
        double DefaultMixerSpeedRPM = GetMainAppCrystalT2()->GetMixerTerminalValueToRotationPerMinute(GetMainAppCrystalT2()->GetSettingsData()->m_SpeedMixerStepperValue);
        double MaschineStopThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_MixerMovingMaschineStopTimeInSec;
        double WarnThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_MixerMovingWarningInRPM;
        double AlarmThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_MixerMovingAlarmInRPM;

        int StatusControlMixerVelocity = m_ControlMixerVelocity->SetValueAndAlarmStatus(ActualVelocityMixerRPM, ActualVelocityMixerRPM, DefaultMixerSpeedRPM, WarnThreshold, AlarmThreshold,
                                                                                   MaschineStopThreshold, MaschineStopInSec);
        if (StatusControlMixerVelocity != ALARM_LEVEL_OK) {
            if (GetMainAppCrystalT2()->GetCurrentMaschineState() == PluginInterface::MachineState::Off) {
                // In off state mixer is not running for check pressure is the same
                m_NumberErrorMixer = 0;
                m_StatusControlMixerVelocity = ALARM_LEVEL_OK;
                m_ControlMixerVelocity->SetColorStatus(ALARM_LEVEL_OK);
            } else {
                m_NumberErrorMixer++;  // Zähler damit es in der Beschleunigungsphase oder Bremsphase nicht zu einer Fehlermeldung kommt
                if (m_NumberErrorMixer < GetMainAppCrystalT2()->GetSettingsData()->m_MaximumErrorCount) {
                    m_StatusControlMixerVelocity = ALARM_LEVEL_OK;
                    m_ControlMixerVelocity->SetColorStatus(ALARM_LEVEL_OK);
                } else {
                    m_StatusControlMixerVelocity = StatusControlMixerVelocity;  // jetzt erst Fehlermeldung
                }
            }
        } else {
            m_NumberErrorMixer = 0;
            m_StatusControlMixerVelocity = StatusControlMixerVelocity;  // Drehzahl ok
        }

    } else {
        m_StatusControlMixerVelocity = ALARM_LEVEL_OK;
        ui.doubleSpinBoxMixerVelocity->setValue(ActualVelocityMixerRPM);
        if (m_ControlMixerVelocity) {
            m_ControlMixerVelocity->StopTimerColorStatus();
            m_ControlMixerVelocity->SetBackgroundColor(HMIColor::ContentBoard);
        }
    }
}

void IOSettingsDialog::SetPOSStausMixer(double Status)
{
    QString TextInfo = QString("%1").arg(Status, 0, 'f', 1);
    ui.labelTextInfo->setText(TextInfo);
}

void IOSettingsDialog::SetENCStatusMixer(ushort ENCStatusMixer)
{
    if (ENCStatusMixer & 0x1000) {  // Sync error
        ui.radioButtonENCStatusSyncError->setDown(true);
    } else {
        ui.radioButtonENCStatusSyncError->setDown(false);
    }
    if (ENCStatusMixer & 0x800) {  // Status of extern latch
        ui.radioButtonENCStatusStatusExternLatch->setDown(true);
    } else {
        ui.radioButtonENCStatusStatusExternLatch->setDown(false);
    }
    if (ENCStatusMixer & 0x400) {  // Status of input C
        ui.radioButtonENCStatusStatusInputC->setDown(true);
    } else {
        ui.radioButtonENCStatusStatusInputC->setDown(false);
    }
    if (ENCStatusMixer & 0x200) {  // Status of input B
        ui.radioButtonENCStatusStatusInputB->setDown(true);
    } else {
        ui.radioButtonENCStatusStatusInputB->setDown(false);
    }
    if (ENCStatusMixer & 0x100) {  // Status of input A
        ui.radioButtonENCStatusStatusInputA->setDown(true);
    } else {
        ui.radioButtonENCStatusStatusInputA->setDown(false);
    }
    if (ENCStatusMixer & 0x80) {  // Extrapolation stall
        ui.radioButtonENCStatusExtrapolationStall->setDown(true);
    } else {
        ui.radioButtonENCStatusExtrapolationStall->setDown(false);
    }
    if (ENCStatusMixer & 0x10) {  // Counter overflow
        ui.radioButtonENCStatusCounterOverflow->setDown(true);
    } else {
        ui.radioButtonENCStatusCounterOverflow->setDown(false);
    }
    if (ENCStatusMixer & 0x8) {  // Counter underflow
        ui.radioButtonENCStatusCounterUnderflow->setDown(true);
    } else {
        ui.radioButtonENCStatusCounterUnderflow->setDown(false);
    }
    if (ENCStatusMixer & 0x4) {  // Set counter done
        ui.radioButtonENCStatusSetConterDone->setDown(true);
    } else {
        ui.radioButtonENCStatusSetConterDone->setDown(false);
    }
    if (ENCStatusMixer & 0x2) {  // Latch extern valid
        ui.radioButtonENCStatusLatchExternValid->setDown(true);
    } else {
        ui.radioButtonENCStatusLatchExternValid->setDown(false);
    }
    if (ENCStatusMixer & 0x1) {  // Latch C valid
        ui.radioButtonENCStatusLatchCValid->setDown(true);
    } else {
        ui.radioButtonENCStatusLatchCValid->setDown(false);
    }
}

void IOSettingsDialog::SetStatusMixer(ushort StatusMixer)
{
    if (StatusMixer & 0x1) {  //       if (StatusMixer & 0x0001) {
        ui.radioButtonReadyToEnable->setDown(true);
    } else {
        ui.radioButtonReadyToEnable->setDown(false);
    }
    if (StatusMixer & 0x2) {  // if (StatusMixer & 0x0002) {
        ui.radioButtonReady->setDown(true);
    } else {
        ui.radioButtonReady->setDown(false);
    }
    if (StatusMixer & 0x4) {  // if (StatusMixer & 0x0003) {
        ui.radioButtonWarning->setDown(true);
    } else {
        ui.radioButtonWarning->setDown(false);
    }
    if (StatusMixer & 0x8) {  // if (StatusMixer & 0x0004) {
        ui.radioButtonError->setDown(true);
    } else {
        ui.radioButtonError->setDown(false);
    }
    if (StatusMixer & 0x10) {  // if (StatusMixer & 0x0005) {
        ui.radioButtonMovingPositiv->setDown(true);
    } else {
        ui.radioButtonMovingPositiv->setDown(false);
    }
    if (StatusMixer & 0x20) {  // if (StatusMixer & 0x0006) {
        ui.radioButtonMovingNegativ->setDown(true);
    } else {
        ui.radioButtonMovingNegativ->setDown(false);
    }
    if (StatusMixer & 0x40) {  // if (StatusMixer & 0x0007) {
        ui.radioButtonTorqueReduced->setDown(true);
    } else {
        ui.radioButtonTorqueReduced->setDown(false);
    }
    if (StatusMixer & 0x80) {  // if (StatusMixer & 0x000C) {
        ui.radioButtonDigitalInput1->setDown(true);
    } else {
        ui.radioButtonDigitalInput1->setDown(false);
    }
    if (StatusMixer & 0x1000) {  // if (StatusMixer & 0x000D) {
        ui.radioButtonDigitalInput2->setDown(true);
    } else {
        ui.radioButtonDigitalInput2->setDown(false);
    }
    if (StatusMixer & 0x2000) {  // if (StatusMixer & 0x000E) {
        ui.radioButtonSyncError->setDown(true);
    } else {
        ui.radioButtonSyncError->setDown(false);
    }
}

void IOSettingsDialog::SlotStartMixer()
{
    if (GetMainAppCrystalT2()->GetImageData()) {
        GetMainAppCrystalT2()->GetImageData()->SetMixerOn(true);
    }
}

void IOSettingsDialog::SlotStopMixer()
{
    if (GetMainAppCrystalT2()->GetImageData()) {
        GetMainAppCrystalT2()->GetImageData()->SetMixerOn(false);
    }
}

void IOSettingsDialog::SlotSetPowerMixerSimulation(bool State)
{
    // Simulation Spannung auf den Spulen
    ushort ENCStatusMixer = 0;
    if (State) {
        ENCStatusMixer = ENCStatusMixer | 0x200;  // input b
        ENCStatusMixer = ENCStatusMixer | 0x100;  // input a
    }
    if (GetMainAppCrystalT2()->GetImageData()) {
        GetMainAppCrystalT2()->GetImageData()->SetENCStatusMixerSimulation(ENCStatusMixer);
    }
}

void IOSettingsDialog::SlotSetMixerStartStopSimulation(bool State)
{
    m_StatusMixer = 0;
    m_StatusMixer = m_StatusMixer | 0x1;
    m_StatusMixer = m_StatusMixer | 0x2;
    if (State) {
        m_StatusMixer = m_StatusMixer | 0x10;  // Moving positiv
    }
    if (GetMainAppCrystalT2()->GetImageData()) {
        GetMainAppCrystalT2()->GetImageData()->SetStatusMixerSimulation(m_StatusMixer);
    }
}

double IOSettingsDialog::GetMixerSpeedSimulationInRPM()
{
    return ui.doubleSpinBoxMixerSpeedSimulation->value();
}

// void IOSettingsDialog::SlotSetMixerSpeedSimulation()
//{
//    if (GetMainAppCrystalT2()) {
//        ushort mixerSpeedSimu = static_cast<ushort>(GetMainAppCrystalT2()->GetMixerRotationPerMinuteToTerminalValue(ui.doubleSpinBoxMixerSpeedSimulation->value()));
//        if (GetMainAppCrystalT2()->GetImageData()) {
//            GetMainAppCrystalT2()->GetImageData()->SetActualMixerVelocity(mixerSpeedSimu);
//        }
//    }
//}

void IOSettingsDialog::SlotSetRealEjectionSimulation(bool State)
{
    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
    if (pSettingsData) {
        if (pSettingsData->m_WorkWithoutEtherCat) {
            SetDigitalOutput(EtherCATConfigData::DI_EJECTION_CONTROL, State);
        }
    }
}

void IOSettingsDialog::SlotSetControlVotageSimulation(bool State)
{
    // m_ControlVoltageOnOffTestValue = State;
    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
    if (pSettingsData) {
        if (pSettingsData->m_WorkWithoutEtherCat) {
            SetDigitalOutput(EtherCATConfigData::DI_CONTROL_VOLTAGE, State);
        }
    }
}

void IOSettingsDialog::SlotActualValvePreasureSimulation()
{
    double dValue = ui.doubleSpinBoxActualValvePreasure->value();
    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
    if (pSettingsData) {
        if (pSettingsData->m_WorkWithoutEtherCat) {
            short iValue = GetMainAppCrystalT2()->PhysicalSizeToTerminalValue(MainAppCrystalT2::AnalogTerminals::PREASURE_VALUE, dValue);
            GetMainAppCrystalT2()->GetImageData()->SetAnalogInputValue2Byte(QString(AI_NAME_ACTUAL_PREASURE), iValue);
        }
    }
}

void IOSettingsDialog::SlotPreasureTankFillingLevelSimulation()
{
    double dvalue = ui.doubleSpinBoxPreasureTankFillingLevel->value();
    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
    if (pSettingsData) {
        if (pSettingsData->m_WorkWithoutEtherCat) {
            short iValue = GetMainAppCrystalT2()->PhysicalSizeToTerminalValue(MainAppCrystalT2::AnalogTerminals::FILLING_LEVEL, dvalue);
            GetMainAppCrystalT2()->GetImageData()->SetAnalogInputValue2Byte(QString(AI_NAME_ACTUAL_TANK_FILLING_LEVEL), iValue);
        }
    }
}

void IOSettingsDialog::SlotPreasureTankTemperatureSimulation()
{
    double dvalue = ui.doubleSpinBoxPreasureTankTemperature->value();
    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
    if (pSettingsData) {
        if (pSettingsData->m_WorkWithoutEtherCat) {
            short iValue = GetMainAppCrystalT2()->PhysicalSizeToTerminalValue(MainAppCrystalT2::AnalogTerminals::PREASURE_TANK_TEMPERATURE, dvalue);
            GetMainAppCrystalT2()->GetImageData()->SetAnalogInputValue2Byte(QString(AI_NAME_TANK_TEMPERATURE), iValue);
        }
    }
}

void IOSettingsDialog::SlotHeatingPipeTemperatureSimulation()
{
    double dvalue = ui.doubleSpinBoxHeatingPipeTemperature->value();
    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
    if (pSettingsData) {
        if (pSettingsData->m_WorkWithoutEtherCat) {
            short iValue = GetMainAppCrystalT2()->PhysicalSizeToTerminalValue(MainAppCrystalT2::AnalogTerminals::HEATING_PIPE_TEMPERATURE, dvalue);
            GetMainAppCrystalT2()->GetImageData()->SetAnalogInputValue2Byte(QString(AI_NAME_HEATING_PIPE_TEMPERATURE), iValue);
        }
    }
}

void IOSettingsDialog::SlotActualAirCoolingCameraSimulation()
{
    double dValue = ui.doubleSpinBoxAirCoolingCameraSimulation->value();
    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
    if (pSettingsData) {
        if (pSettingsData->m_WorkWithoutEtherCat) {
            short iValue = GetMainAppCrystalT2()->PhysicalSizeToTerminalValue(MainAppCrystalT2::AnalogTerminals::AIR_COOLING_CAMERA, dValue);
            GetMainAppCrystalT2()->GetImageData()->SetAnalogInputValue2Byte(QString(AI_NAME_ACTUAL_AIR_COOLING_CAMERA_AND_BACK_LIGHT), iValue);
        }
    }
}

void IOSettingsDialog::SlotActualAirCoolingCameraLightSimulation()
{
    double dValue = ui.doubleSpinBoxAirCoolingCameraLightSimulation->value();
    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
    if (pSettingsData) {
        if (pSettingsData->m_WorkWithoutEtherCat) {
            short iValue = GetMainAppCrystalT2()->PhysicalSizeToTerminalValue(MainAppCrystalT2::AnalogTerminals::AIR_COOLING_CAMERA_LIGHT, dValue);
            GetMainAppCrystalT2()->GetImageData()->SetAnalogInputValue2Byte(QString(AI_NAME_ACTUAL_AIR_COOLING_GLASS), iValue);
        }
    }
}

void IOSettingsDialog::SlotActualAirCoolingValvesSimulation()
{
    double dValue = ui.doubleSpinBoxAirCoolingValvesSimulation->value();
    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
    if (pSettingsData) {
        if (pSettingsData->m_WorkWithoutEtherCat) {
            short iValue = GetMainAppCrystalT2()->PhysicalSizeToTerminalValue(MainAppCrystalT2::AnalogTerminals::AIR_COOLING_VALVES, dValue);
            GetMainAppCrystalT2()->GetImageData()->SetAnalogInputValue2Byte(QString(AI_NAME_ACTUAL_AIR_COOLING_VALVE), iValue);
        }
    }
}

void IOSettingsDialog::SlotActualWaterCoolingSimulation()
{
    double dValue = ui.doubleSpinBoxWaterCoolingSimulation->value();
    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
    if (pSettingsData) {
        if (pSettingsData->m_WorkWithoutEtherCat) {
            short iValue = GetMainAppCrystalT2()->PhysicalSizeToTerminalValue(MainAppCrystalT2::AnalogTerminals::WATER_COOLING, dValue);
            GetMainAppCrystalT2()->GetImageData()->SetAnalogInputValue2Byte(QString(AI_NAME_ACTUAL_WATER_COOLING), iValue);
        }
    }
}

void IOSettingsDialog::SlotActualFlowTransmitterWaterCoolingCircuitSimulation()
{
    double dValue = ui.doubleSpinBoxFlowTransmitterWaterCoolingCircuitSimulation->value();
    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
    if (pSettingsData) {
        if (pSettingsData->m_WorkWithoutEtherCat) {
            short iValue = GetMainAppCrystalT2()->PhysicalSizeToTerminalValue(MainAppCrystalT2::AnalogTerminals::FLOW_TRANSMITTER_WATER_COOLING_CIRCUIT, dValue);
            GetMainAppCrystalT2()->GetImageData()->SetAnalogInputValue2Byte(QString(AI_NAME_ACTUAL_FLOW_TRANSMITTER_WATER_COOLING_CIRCUIT), iValue);
        }
    }
}

void IOSettingsDialog::SlotActualTemperaturWaterCoolingReturnSimulation()
{
    double dValue = ui.doubleSpinBoxTemperaturWaterCoolingReturnSimulation->value();
    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
    if (pSettingsData) {
        if (pSettingsData->m_WorkWithoutEtherCat) {
            short iValue = GetMainAppCrystalT2()->PhysicalSizeToTerminalValue(MainAppCrystalT2::AnalogTerminals::TEMPERATURE_WATER_COOLING_RETURN, dValue);
            GetMainAppCrystalT2()->GetImageData()->SetAnalogInputValue2Byte(QString(AI_NAME_ACTUAL_TEMPERATURE_WATER_COOLING_RETURN), iValue);
        }
    }
}

void IOSettingsDialog::showEvent(QShowEvent*)
{
    m_SetupWindow = false;
    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
    if (pSettingsData) {
        if (!pSettingsData->m_WorkWithoutEtherCat) ui.groupBoxEtherCatSimulation->hide();
    }
    if (m_TimerShowDigitalOutputs) m_TimerShowDigitalOutputs->start();
    m_SetupWindow = true;
}

void IOSettingsDialog::hideEvent(QHideEvent*)
{
    if (m_TimerShowDigitalOutputs) m_TimerShowDigitalOutputs->stop();
}

void IOSettingsDialog::SetTestDefaultParameter()
{
    SlotActualValvePreasureSimulation();
    SlotPreasureTankTemperatureSimulation();
    SlotPreasureTankFillingLevelSimulation();
    SlotSetControlVotageSimulation(true);
    SlotHeatingPipeTemperatureSimulation();
    SlotActualAirCoolingCameraLightSimulation();
    SlotActualAirCoolingCameraSimulation();
    SlotActualAirCoolingValvesSimulation();
    SlotActualWaterCoolingSimulation();
    SlotActualFlowTransmitterWaterCoolingCircuitSimulation();
    SlotActualTemperaturWaterCoolingReturnSimulation();
    // SlotSetMixerSpeedSimulation();
    SlotSetPowerMixerSimulation(true);
    SlotSetMixerStartStopSimulation(true);
}

void IOSettingsDialog::SetDigitalOutput(EtherCATConfigData::IOChannels ChannelID, bool State)
{
    if (GetMainAppCrystalT2()) {
        GetMainAppCrystalT2()->SetDigitalOutputValue(ChannelID, State);
    }
}

void IOSettingsDialog::SetEtherCatCyclus(const QString& Info)
{
    double Value = Info.toDouble();
    ui.doubleSpinBoxEtherCatCycle->setValue(Value);
}

void IOSettingsDialog::SetActualTankFillingLevel(double dvalue)
{
    if (m_CurrentCalibrateAnalogInputIndex == AnalogInputCh::AI_ACTUAL_TANK_FILLING_LEVEL) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            short iValue = GetMainAppCrystalT2()->PhysicalSizeToTerminalValue(MainAppCrystalT2::AnalogTerminals::FILLING_LEVEL, dvalue);
            ui.doubleSpinBoxCorrespondingTerminalValue->setValue((double)(iValue));
        }
    }
}

void IOSettingsDialog::SetActualAirCoolingCamera(double dvalue)
{
    if (m_CurrentCalibrateAnalogInputIndex == AnalogInputCh::AI_ACTUAL_AIR_COOLING_CAMERA) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            short iValue = GetMainAppCrystalT2()->PhysicalSizeToTerminalValue(MainAppCrystalT2::AnalogTerminals::AIR_COOLING_CAMERA, dvalue);
            ui.doubleSpinBoxCorrespondingTerminalValue->setValue((double)(iValue));
        }
    }
}

void IOSettingsDialog::SetActualAirCoolingCameraLight(double dvalue)
{
    if (m_CurrentCalibrateAnalogInputIndex == AnalogInputCh::AI_ACTUAL_AIR_COOLING_CAMERA_LIGHT) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            short iValue = GetMainAppCrystalT2()->PhysicalSizeToTerminalValue(MainAppCrystalT2::AnalogTerminals::AIR_COOLING_CAMERA_LIGHT, dvalue);
            ui.doubleSpinBoxCorrespondingTerminalValue->setValue((double)(iValue));
        }
    }
}

void IOSettingsDialog::SetActualAirCoolingValves(double dvalue)
{
    if (m_CurrentCalibrateAnalogInputIndex == AnalogInputCh::AI_ACTUAL_AIR_COOLING_VALVE) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            short iValue = GetMainAppCrystalT2()->PhysicalSizeToTerminalValue(MainAppCrystalT2::AnalogTerminals::AIR_COOLING_VALVES, dvalue);
            ui.doubleSpinBoxCorrespondingTerminalValue->setValue((double)(iValue));
        }
    }
}

void IOSettingsDialog::SetActualWaterCooling(double dvalue)
{
    if (m_CurrentCalibrateAnalogInputIndex == AnalogInputCh::AI_ACTUAL_WATER_COOLING) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            short iValue = GetMainAppCrystalT2()->PhysicalSizeToTerminalValue(MainAppCrystalT2::AnalogTerminals::WATER_COOLING, dvalue);
            ui.doubleSpinBoxCorrespondingTerminalValue->setValue((double)(iValue));
        }
    }
}

void IOSettingsDialog::SetActualFlowTransmitterWaterCoolingCircuit(double dvalue)
{
    if (m_CurrentCalibrateAnalogInputIndex == AnalogInputCh::AI_ACTUAL_FLOW_TRANSMITTER_WATER_COOLING_CIRCUIT) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            short iValue = GetMainAppCrystalT2()->PhysicalSizeToTerminalValue(MainAppCrystalT2::AnalogTerminals::FLOW_TRANSMITTER_WATER_COOLING_CIRCUIT, dvalue);
            ui.doubleSpinBoxCorrespondingTerminalValue->setValue((double)(iValue));
        }
    }
}

void IOSettingsDialog::SetActualTemperaturWaterCoolingReturn(double dvalue)
{
    if (m_CurrentCalibrateAnalogInputIndex == AnalogInputCh::AI_ACTUAL_TEMPERATURE_WATER_COOLING_RETURN) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            short iValue = GetMainAppCrystalT2()->PhysicalSizeToTerminalValue(MainAppCrystalT2::AnalogTerminals::TEMPERATURE_WATER_COOLING_RETURN, dvalue);
            ui.doubleSpinBoxCorrespondingTerminalValue->setValue((double)(iValue));
        }
    }
}

void IOSettingsDialog::SetActualPreasureValue(double value)
{
    if (m_CurrentCalibrateAnalogInputIndex == AnalogInputCh::AI_ACTUAL_PREASURE) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            short iValue = static_cast<short>(((value - pSettingsData->m_OffsetAnalogInputCurrentPreasure) / pSettingsData->m_FactorAnalogInputCurrentPreasure) + 0.5);
            ui.doubleSpinBoxCorrespondingTerminalValue->setValue((double)(iValue));
        }
    }
}

void IOSettingsDialog::SlotAddToListCalibrate()
{
    int num = ui.listWidgetDataCalibrate->count();
    QChar Correspond = QChar(0x2259);

    QString InsertItem = QString("%1 %2 %3").arg(ui.doubleSpinBoxCorrespondingAnalogValue->value(), 0, 'f', 2).arg(Correspond).arg(ui.doubleSpinBoxCorrespondingTerminalValue->value());
    if (num < 2) {
        ui.listWidgetDataCalibrate->addItem(InsertItem);
    } else {
        QListWidgetItem* it = ui.listWidgetDataCalibrate->takeItem(0);
        delete it;
        ui.listWidgetDataCalibrate->addItem(InsertItem);
    }
}

void IOSettingsDialog::SlotCalibrateFillingLevel()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        double TankRestLiquidInLiter = ui.doubleSpinBoxTankRestLiquid->value();  // wenn keine Flüssigkeit zum Ventil hin fließt, dann sind im Tank noch ca. 2.3 Liter
        double y1, y2, x1, x2;
        int num = ui.listWidgetDataCalibrate->count();
        QChar Correspond = QChar(0x2259);
        if (m_CurrentCalibrateAnalogInputIndex != AnalogInputCh::AI_ACTUAL_TANK_FILLING_LEVEL) {
            TankRestLiquidInLiter = 0.0;
        }
        if (num == 2) {
            QString dataSet1 = ui.listWidgetDataCalibrate->item(0)->text();
            QString dataSet2 = ui.listWidgetDataCalibrate->item(1)->text();
            QStringList listDataset1 = dataSet1.split(Correspond);
            QStringList listDataset2 = dataSet2.split(Correspond);
            if (listDataset1.count() == 2) {
                y1 = listDataset1[0].toDouble() - TankRestLiquidInLiter;
                x1 = listDataset1[1].toDouble();
                if (listDataset2.count() == 2) {
                    y2 = listDataset2[0].toDouble() - TankRestLiquidInLiter;
                    x2 = listDataset2[1].toDouble();
                    if ((x1 - x2) != 0.0) {
                        double factor = (y1 - y2) / (x1 - x2);
                        double offset = y1 - factor * x1;
                        if (factor != 0.0) {
                            SaveFactorOffsetSettings(factor, offset);
                        } else {
                            qDebug() << QString("Calibrate Analog Input Failed y1:%1 == y2:%2").arg(y1).arg(y2);
                        }
                    } else {
                        qDebug() << QString("Calibrate Analog Input Failed x1:%1 == X2:%2").arg(x1).arg(x2);
                    }
                }
            }
        }
    }
}

void IOSettingsDialog::SaveFactorOffsetSettings(double factor, double offset)
{
    if (GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            switch (m_CurrentCalibrateAnalogInputIndex) {
                case AnalogInputCh::AI_ACTUAL_AIR_COOLING_CAMERA:
                    m_LastOffsetAnalogInputAirCoolingCamera = pSettingsData->m_OffsetAnalogInputAirCoolingCamera;
                    m_LastFactorAnalogInputAirCoolingCamera = pSettingsData->m_FactorAnalogInputAirCoolingCamera;
                    pSettingsData->m_OffsetAnalogInputAirCoolingCamera = offset;
                    pSettingsData->m_FactorAnalogInputAirCoolingCamera = factor;
                    break;
                case AnalogInputCh::AI_ACTUAL_AIR_COOLING_CAMERA_LIGHT:
                    m_LastOffsetAnalogInputAirCoolingCameraLight = pSettingsData->m_OffsetAnalogInputAirCoolingCameraLight;
                    m_LastFactorAnalogInputAirCoolingCameraLight = pSettingsData->m_FactorAnalogInputAirCoolingCameraLight;
                    pSettingsData->m_OffsetAnalogInputAirCoolingCameraLight = offset;
                    pSettingsData->m_FactorAnalogInputAirCoolingCameraLight = factor;
                    break;
                case AnalogInputCh::AI_ACTUAL_AIR_COOLING_VALVE:
                    m_LastOffsetAnalogInputAirCoolingValves = pSettingsData->m_OffsetAnalogInputAirCoolingValves;
                    m_LastFactorAnalogInputAirCoolingValves = pSettingsData->m_FactorAnalogInputAirCoolingValves;
                    pSettingsData->m_OffsetAnalogInputAirCoolingValves = offset;
                    pSettingsData->m_FactorAnalogInputAirCoolingValves = factor;
                    break;
                case AnalogInputCh::AI_ACTUAL_WATER_COOLING:
                    m_LastOffsetAnalogInputWaterCooling = pSettingsData->m_OffsetAnalogInputWaterCooling;
                    m_LastFactorAnalogInputWaterCooling = pSettingsData->m_FactorAnalogInputWaterCooling;
                    pSettingsData->m_OffsetAnalogInputWaterCooling = offset;
                    pSettingsData->m_FactorAnalogInputWaterCooling = factor;
                    break;
                case AnalogInputCh::AI_ACTUAL_FLOW_TRANSMITTER_WATER_COOLING_CIRCUIT:
                    m_LastOffsetAnalogInputFlowTransmitterWaterCoolingCircuit = pSettingsData->m_OffsetAnalogInputFlowTransmitterWaterCoolingCircuit;
                    m_LastFactorAnalogInputFlowTransmitterWaterCoolingCircuit = pSettingsData->m_FactorAnalogInputFlowTransmitterWaterCoolingCircuit;
                    pSettingsData->m_OffsetAnalogInputFlowTransmitterWaterCoolingCircuit = offset;
                    pSettingsData->m_FactorAnalogInputFlowTransmitterWaterCoolingCircuit = factor;
                    break;
                case AnalogInputCh::AI_ACTUAL_TEMPERATURE_WATER_COOLING_RETURN:
                    m_LastOffsetAnalogInputTemperaturWaterCoolingReturn = pSettingsData->m_OffsetAnalogInputTemperaturWaterCoolingReturn;
                    m_LastFactorAnalogInputTemperaturWaterCoolingReturn = pSettingsData->m_FactorAnalogInputTemperaturWaterCoolingReturn;
                    pSettingsData->m_OffsetAnalogInputTemperaturWaterCoolingReturn = offset;
                    pSettingsData->m_FactorAnalogInputTemperaturWaterCoolingReturn = factor;
                    break;
                case AnalogInputCh::AI_ACTUAL_PREASURE:
                    m_LastOffsetAnalogInputCurrentPreasure = pSettingsData->m_OffsetAnalogInputCurrentPreasure;
                    m_LastFactorAnalogInputCurrentPreasure = pSettingsData->m_FactorAnalogInputCurrentPreasure;
                    pSettingsData->m_OffsetAnalogInputCurrentPreasure = offset;
                    pSettingsData->m_FactorAnalogInputCurrentPreasure = factor;
                    break;
                case AnalogInputCh::AI_ACTUAL_TANK_FILLING_LEVEL:
                    m_LastOffsetAnalogInputTankFillingLevel = pSettingsData->m_OffsetAnalogInputTankFillingLevel;
                    m_LastFactorAnalogInputTankFillingLevel = pSettingsData->m_FactorAnalogInputTankFillingLevel;
                    pSettingsData->m_OffsetAnalogInputTankFillingLevel = offset;
                    pSettingsData->m_FactorAnalogInputTankFillingLevel = factor;
                    break;
                default:

                    break;
            }
        }
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void IOSettingsDialog::SlotRestoreLastCalibrationData()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            switch (m_CurrentCalibrateAnalogInputIndex) {
                case AnalogInputCh::AI_ACTUAL_AIR_COOLING_CAMERA:
                    pSettingsData->m_OffsetAnalogInputAirCoolingCamera = m_LastOffsetAnalogInputAirCoolingCamera;
                    pSettingsData->m_FactorAnalogInputAirCoolingCamera = m_LastFactorAnalogInputAirCoolingCamera;
                    break;
                case AnalogInputCh::AI_ACTUAL_AIR_COOLING_CAMERA_LIGHT:
                    pSettingsData->m_OffsetAnalogInputAirCoolingCameraLight = m_LastOffsetAnalogInputAirCoolingCameraLight;
                    pSettingsData->m_FactorAnalogInputAirCoolingCameraLight = m_LastFactorAnalogInputAirCoolingCameraLight;
                    break;
                case AnalogInputCh::AI_ACTUAL_AIR_COOLING_VALVE:
                    pSettingsData->m_OffsetAnalogInputAirCoolingValves = m_LastOffsetAnalogInputAirCoolingValves;
                    pSettingsData->m_FactorAnalogInputAirCoolingValves = m_LastFactorAnalogInputAirCoolingValves;
                    break;
                case AnalogInputCh::AI_ACTUAL_WATER_COOLING:
                    pSettingsData->m_OffsetAnalogInputWaterCooling = m_LastOffsetAnalogInputWaterCooling;
                    pSettingsData->m_FactorAnalogInputWaterCooling = m_LastFactorAnalogInputWaterCooling;
                    break;
                case AnalogInputCh::AI_ACTUAL_FLOW_TRANSMITTER_WATER_COOLING_CIRCUIT:
                    pSettingsData->m_OffsetAnalogInputFlowTransmitterWaterCoolingCircuit = m_LastOffsetAnalogInputFlowTransmitterWaterCoolingCircuit;
                    pSettingsData->m_FactorAnalogInputFlowTransmitterWaterCoolingCircuit = m_LastFactorAnalogInputFlowTransmitterWaterCoolingCircuit;
                    break;
                case AnalogInputCh::AI_ACTUAL_TEMPERATURE_WATER_COOLING_RETURN:
                    pSettingsData->m_OffsetAnalogInputTemperaturWaterCoolingReturn = m_LastOffsetAnalogInputTemperaturWaterCoolingReturn;
                    pSettingsData->m_FactorAnalogInputTemperaturWaterCoolingReturn = m_LastFactorAnalogInputTemperaturWaterCoolingReturn;
                    break;
                case AnalogInputCh::AI_ACTUAL_PREASURE:
                    pSettingsData->m_OffsetAnalogInputCurrentPreasure = m_LastOffsetAnalogInputCurrentPreasure;
                    pSettingsData->m_FactorAnalogInputCurrentPreasure = m_LastFactorAnalogInputCurrentPreasure;
                    break;
                case AnalogInputCh::AI_ACTUAL_TANK_FILLING_LEVEL:
                    pSettingsData->m_OffsetAnalogInputTankFillingLevel = m_LastOffsetAnalogInputTankFillingLevel;
                    pSettingsData->m_FactorAnalogInputTankFillingLevel = m_LastFactorAnalogInputTankFillingLevel;
                    break;
                default:

                    break;
            }
        }
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void IOSettingsDialog::SlotSelectAnalogInputTypeChanged(int index)
{
    m_CurrentCalibrateAnalogInputIndex = index;
    switch (index) {
        case AnalogInputCh::AI_ACTUAL_AIR_COOLING_CAMERA:
            ui.doubleSpinBoxCorrespondingAnalogValue->setSuffix(tr(" l/min"));
            ui.widgetDashBoardTankRestLiquid->hide();
            break;
        case AnalogInputCh::AI_ACTUAL_AIR_COOLING_CAMERA_LIGHT:
            ui.doubleSpinBoxCorrespondingAnalogValue->setSuffix(tr(" l/min"));
            ui.widgetDashBoardTankRestLiquid->hide();
            break;
        case AnalogInputCh::AI_ACTUAL_AIR_COOLING_VALVE:
            ui.doubleSpinBoxCorrespondingAnalogValue->setSuffix(tr(" l/min"));
            ui.widgetDashBoardTankRestLiquid->hide();
            break;
        case AnalogInputCh::AI_ACTUAL_WATER_COOLING:
            ui.doubleSpinBoxCorrespondingAnalogValue->setSuffix(tr(" l/min"));
            ui.widgetDashBoardTankRestLiquid->hide();
            break;
        case AnalogInputCh::AI_ACTUAL_FLOW_TRANSMITTER_WATER_COOLING_CIRCUIT:
            ui.doubleSpinBoxCorrespondingAnalogValue->setSuffix(tr(" l/min"));
            ui.widgetDashBoardTankRestLiquid->hide();
            break;
        case AnalogInputCh::AI_ACTUAL_TEMPERATURE_WATER_COOLING_RETURN:
            ui.doubleSpinBoxCorrespondingAnalogValue->setSuffix(QString(" C"));
            ui.widgetDashBoardTankRestLiquid->hide();
            break;
        case AnalogInputCh::AI_ACTUAL_PREASURE:
            ui.doubleSpinBoxCorrespondingAnalogValue->setSuffix(tr(" bar"));
            ui.widgetDashBoardTankRestLiquid->hide();
            break;
        case AnalogInputCh::AI_ACTUAL_TANK_FILLING_LEVEL:
            ui.doubleSpinBoxCorrespondingAnalogValue->setSuffix(tr(" Liter"));
            ui.widgetDashBoardTankRestLiquid->show();
            break;
        default:
            ui.widgetDashBoardTankRestLiquid->hide();
            break;
    }
}

void IOSettingsDialog::SlotShowDigitalOutputs()
{
    bool rv;
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) {
        m_SetupWindow = false;
        rv = GetMainAppCrystalT2()->GetImageData()->GetDigitalInput(EtherCATConfigData::DO_ERROR_LIGHT);
        m_SwitchSlotErrorLight->setChecked(rv);
        rv = GetMainAppCrystalT2()->GetImageData()->GetDigitalInput(EtherCATConfigData::DO_CAMERA_LIGHT);
        m_SwitchCameraLight->setChecked(rv);
        rv = GetMainAppCrystalT2()->GetImageData()->GetDigitalInput(EtherCATConfigData::DO_TRIGGER1_VALVE);
        m_SwitchSetTrigger1Valve->setChecked(rv);
        rv = GetMainAppCrystalT2()->GetImageData()->GetDigitalInput(EtherCATConfigData::DO_TRIGGER2_VALVE);
        m_SwitchSetTrigger2Valve->setChecked(rv);
        rv = GetMainAppCrystalT2()->GetImageData()->GetDigitalInput(EtherCATConfigData::DO_ERROR_TRANSFER);
        m_SwitchSlotErrorTransfer->setChecked(rv);
        rv = GetMainAppCrystalT2()->GetImageData()->GetDigitalInput(EtherCATConfigData::DO_COUNTER_EJECTION_TRANSFER);
        m_SwitchCounterEjectionTransfer->setChecked(rv);
        rv = GetMainAppCrystalT2()->GetImageData()->GetDigitalInput(EtherCATConfigData::DO_PREASURE_TANK_HEATER);
        m_SwitchPressureTanksHeater->setChecked(rv);
        rv = GetMainAppCrystalT2()->GetImageData()->GetDigitalInput(EtherCATConfigData::DO_PREASURE_TANK_VALVE);
        m_SwitchPressureTanksValve->setChecked(rv);
        rv = GetMainAppCrystalT2()->GetImageData()->GetDigitalInput(EtherCATConfigData::DO_BOTTLE_EJECTION);
        m_SwitchBottleEjection->setChecked(rv);
        rv = GetMainAppCrystalT2()->GetImageData()->GetDigitalInput(EtherCATConfigData::DO_VALVE_CONTROLLER);
        m_SwitchValveController->setChecked(rv);
        rv = GetMainAppCrystalT2()->GetImageData()->GetDigitalInput(EtherCATConfigData::DO_BLUE_LIGHT);
        m_SwitchBlueLightPermanentEjectionIS->setChecked(rv);
        rv = GetMainAppCrystalT2()->GetImageData()->GetDigitalInput(EtherCATConfigData::DO_WHITE_LIGHT);
        m_SwitchWhiteLightBottleEjection->setChecked(rv);
        rv = GetMainAppCrystalT2()->GetImageData()->GetDigitalInput(EtherCATConfigData::DO_EJECTION_BY_IS_MASCHINE);
        m_SwitchEjetionIS->setChecked(rv);
        rv = GetMainAppCrystalT2()->GetImageData()->GetDigitalInput(EtherCATConfigData::DO_ORANGE_LIGHT);
        m_SwitchOrangeLightWarning->setChecked(rv);

        m_SetupWindow = true;
    }
}
/*enum IOChannels {
        CONTROL_VOLTAGE = 0, CLOCK_SIGNALE_FROM_IS, EJECTION_CONTROL, ACTUAL_PREASUERE_VALUE, PREASURE_TANK_FILLING_LEVEL, PREASURE_TANK_TEMPERATURE, REFERENCE_PREASURE_VALUE,
        ERROR_LIGHT, ERROR_TRANSFER, COUNTER_EJECTION_TRANSFER, PREASURE_TANK_HEATER, PREASURE_TANK_VALUE, CAMERA_LIGHT, VALVE_CONTROLLER, BOTTLE_EJECTION, TRIGGER_VALUE, END_VALUE
*/

void IOSettingsDialog::SlotWarningLight(bool State)
{
    if (m_SetupWindow) SetDigitalOutput(EtherCATConfigData::DO_ORANGE_LIGHT, State);
}

void IOSettingsDialog::SlotErrorLight(bool State)
{
    if (m_SetupWindow) SetDigitalOutput(EtherCATConfigData::DO_ERROR_LIGHT, State);
}

void IOSettingsDialog::SlotCameraLight(bool State)
{
    if (m_SetupWindow) SetDigitalOutput(EtherCATConfigData::DO_CAMERA_LIGHT, State);
}

void IOSettingsDialog::SlotTrigger1Valve(bool State)
{
    if (m_SetupWindow) SetDigitalOutput(EtherCATConfigData::DO_TRIGGER1_VALVE, State);
}

void IOSettingsDialog::SlotTrigger2Valve(bool State)
{
    if (m_SetupWindow) SetDigitalOutput(EtherCATConfigData::DO_TRIGGER2_VALVE, State);
}

void IOSettingsDialog::SlotErrorTransfer(bool State)
{
    if (m_SetupWindow) SetDigitalOutput(EtherCATConfigData::DO_ERROR_TRANSFER, State);
}

void IOSettingsDialog::SlotCounterEjectionTransfer(bool State)
{
    if (m_SetupWindow) SetDigitalOutput(EtherCATConfigData::DO_COUNTER_EJECTION_TRANSFER, State);
}

void IOSettingsDialog::SlotPressureTanksHeater(bool State)
{
    if (m_SetupWindow) SetDigitalOutput(EtherCATConfigData::DO_PREASURE_TANK_HEATER, State);
}

void IOSettingsDialog::SlotPressureTanksValve(bool State)
{
    if (m_SetupWindow) SetDigitalOutput(EtherCATConfigData::DO_PREASURE_TANK_VALVE, State);
}

void IOSettingsDialog::SlotEjectionIS(bool State)
{
    if (m_SetupWindow) SetDigitalOutput(EtherCATConfigData::DO_EJECTION_BY_IS_MASCHINE, State);
}

void IOSettingsDialog::SlotValveController(bool State)
{
    if (m_SetupWindow) SetDigitalOutput(EtherCATConfigData::DO_VALVE_CONTROLLER, State);
}

void IOSettingsDialog::SlotBottleEjection(bool State)
{
    if (m_SetupWindow) SetDigitalOutput(EtherCATConfigData::DO_BOTTLE_EJECTION, State);
}

void IOSettingsDialog::SlotBlueLightPermanentEjectionIS(bool State)
{
    if (m_SetupWindow) SetDigitalOutput(EtherCATConfigData::DO_BLUE_LIGHT, State);
}

void IOSettingsDialog::SlotWhiteLightBottleEjection(bool State)
{
    if (m_SetupWindow) SetDigitalOutput(EtherCATConfigData::DO_WHITE_LIGHT, State);
}

void IOSettingsDialog::SlotCycleTimeIOTaskChanged()
{
    if (GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_CycleTimeIOTask = ui.doubleSpinBoxCycleTimeIOTask->value();
        GetMainAppCrystalT2()->SaveSettings();
        GetMainAppCrystalT2()->GetImageData()->SetPollingTimeIOTaskInms(ui.doubleSpinBoxCycleTimeIOTask->value());  // into real time task
    }
}

void IOSettingsDialog::SlotTimePeriodTriggerOutputActive()
{
    if (GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_TimePeriodTriggerOutputOnInms = ui.doubleSpinBoxTimePeriodTriggerOutputActive->value();
        GetMainAppCrystalT2()->SaveSettings();
        GetMainAppCrystalT2()->GetImageData()->SetTimePeriodTriggerOutputOnInms(ui.doubleSpinBoxTimePeriodTriggerOutputActive->value());  // into real time task
    }
}

void IOSettingsDialog::SlotTimePeriodDigitalOutputActive()
{
    if (GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_TimePeriodDigitalOutputOnInms = ui.doubleSpinBoxTimePeriodDigitalOutputActive->value();
        GetMainAppCrystalT2()->SaveSettings();
        GetMainAppCrystalT2()->GetImageData()->SetTimePeriodDigitalOutputOnInms(ui.doubleSpinBoxTimePeriodDigitalOutputActive->value());  // into real time task
    }
}
