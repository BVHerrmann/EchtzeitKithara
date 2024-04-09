#include "AdminSettingsDialog.h"
#include "GlobalConst.h"
#include "ImageData.h"
#include "MainAppCrystalT2.h"
#include "OverviewDialog.h"
#include "ProductData.h"
#include "SettingsData.h"
#include "VideoDialog.h"

#include <audittrail.h>

AdminSettingsDialog::AdminSettingsDialog(MainAppCrystalT2* pMainAppCrystalT2) : QWidget(pMainAppCrystalT2), m_WindowSetup(false)
{
    ui.setupUi(this);
    QString ObjName = QString("MainTabAdminSettings");
    setObjectName(ObjName);
    m_MainAppCrystalT2 = pMainAppCrystalT2;

    ui.comboBoxUsedTriggerOutputs->insertItem(USE_BOTH_VALVES, tr("Use Both"));
    ui.comboBoxUsedTriggerOutputs->insertItem(USE_ONLY_LEFT_VALVE, tr("Use Only Left"));
    ui.comboBoxUsedTriggerOutputs->insertItem(USE_ONLY_RIGHT_VALVE, tr("Use Only Right"));

    ui.comboBoxUseSpeedFromISCalcEjectionTime->insertItem(0, tr("Speed From IS Maschine"));
    ui.comboBoxUseSpeedFromISCalcEjectionTime->insertItem(1, tr("Speed From Camera"));

    ui.doubleSpinBoxInfoLevel->setValue(INFO_LEVEL_OFF);
    SetAuditTrailProperties();
    connect(ui.comboBoxUsedTriggerOutputs, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AdminSettingsDialog::SlotUsedTriggerChanged);
    connect(ui.comboBoxUseSpeedFromISCalcEjectionTime, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AdminSettingsDialog::SlotUseSpeedCalcEjectionTimeChanged);
    connect(ui.doubleSpinBoxInfoLevel, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotInfoLevelChanged);
    connect(ui.doubleSpinBoxAcceptanceThreshold, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotAcceptanceThresholdChanged);

    connect(ui.doubleSpinBoxWarnLevelHeatingPipe, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotWarnLevelHeatingPipeChanged);
    connect(ui.doubleSpinBoxAlarmLevelHeatingPipe, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotAlarmLevelHeatingPipeChanged);
    connect(ui.doubleSpinBoxMaschineStopHeatingPipe, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotMaschineStopHeatingPipeChanged);
    // Liquid tank Level alarms
    connect(ui.doubleSpinBoxTankFillingLevelWarning, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotTankFillingLevelWarningChanged);
    connect(ui.doubleSpinBoxTankFillingLevelAlarm, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotTankFillingLevelAlarmChanged);
    connect(ui.doubleSpinBoxTankFillingLevelMaschineStop, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotTankFillingLevelMaschineStopChanged);
    connect(ui.doubleSpinBoxTankFillingLevelMaxValue, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotTankFillingLevelMaxValueChanged);

    connect(ui.doubleSpinBoxTankHeaterLevelWarning, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotTankHeaterLevelWarningChanged);
    connect(ui.doubleSpinBoxTankHeaterLevelAlarm, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotTankHeaterLevelAlarmChanged);
    connect(ui.doubleSpinBoxTankHeaterLevelMaschineStop, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotTankHeaterLevelMaschineStopChanged);

    connect(ui.doubleSpinBoxTankPreasureLevelWarning, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotTankPreasureLevelWarningChanged);
    connect(ui.doubleSpinBoxTankPreasureLevelAlarm, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotTankPreasureLevelAlarmChanged);
    connect(ui.doubleSpinBoxTankPreasureLevelMaschineStop, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotTankPreasureLevelMaschineStopChanged);

    connect(ui.doubleSpinBoxDegreeOfPollutionMaschineStop, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotDegreeOfPollutionMaschineStopChanged);
    connect(ui.doubleSpinBoxDegreeOfPollutionLevelAlarm, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotDegreeOfPollutionLevelAlarmChanged);
    connect(ui.doubleSpinBoxDegreeOfPollutionLevelWarning, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotDegreeOfPollutionLevelWarningChanged);

    connect(ui.doubleSpinBoxValveChamberTemperatureWarningLevel, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotValveChamberTemperatureWarningLevelChanged);
    connect(ui.doubleSpinBoxValveChamberTemperatureAlarmLevel, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotValveChamberTemperatureAlarmLevelChanged);
    connect(ui.doubleSpinBoxValveChamberTemperatureMaschineStopInSec, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotValveChamberTemperatureMaschineStopInSecChanged);

    connect(ui.doubleSpinBoxValvePiezoTemperatureWarningLevel, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotValvePiezoTemperatureWarningLevelChanged);
    connect(ui.doubleSpinBoxValvePiezoTemperatureAlarmLevel, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotValvePiezoTemperatureAlarmLevelChanged);
    connect(ui.doubleSpinBoxValvePiezoTemperatureMaschineStopInSec, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotValvePiezoTemperatureMaschineStopInSecChanged);

    connect(ui.doubleSpinBoxMaxSpeedDeviationWarning, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotMaxSpeedDeviationWarningChanged);
    connect(ui.doubleSpinBoxMaxSpeedDeviationAlarm, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotMaxSpeedDeviationAlarmChanged);
    connect(ui.doubleSpinBoxMaxSpeedDeviationMaschineStop, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotMaxSpeedDeviationMaschineStopChanged);

    connect(ui.doubleSpinBoxCounterProducktNotFilledWarning, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotCounterProducktNotFilledWarningChanged);
    connect(ui.doubleSpinBoxCounterProducktNotFilledAlarm, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotCounterProducktNotFilledAlarmChanged);
    connect(ui.doubleSpinBoxCounterProducktNotFilledMaschineStop, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotCounterProducktNotFilledMaschineStopChanged);

    connect(ui.doubleSpinBoxMaxCounterBottleEjectedOneAfterTheOther, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotMaxEjectedBottlesChanged);
    connect(ui.doubleSpinBoxMaxCounterMiddleTooLowOneAfterTheOther, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotSpinBoxMaxBottlesNotFilledChanged);

    connect(ui.doubleSpinBoxMaxNumberFilesTrendGraph, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotMaxNumberFilesTrendGraphChanged);

    connect(ui.doubleSpinBoxMixerSpeedRotationPerMinute, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotSpeedMixerChanged);
    connect(this, &AdminSettingsDialog::SignalMachineStateChanged, this, &AdminSettingsDialog::SlotMaschineStateChanged);

    connect(ui.doubleSpinBoxPressureIncreaseWhenFlushing, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotPreasureIncreaseWhenFlushingChanged);

    connect(ui.doubleSpinBoxAirCoolingWarning, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotAirCoolingWarningChanged);
    connect(ui.doubleSpinBoxAirCoolingAlarm, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotAirCoolingAlarmChanged);
    connect(ui.doubleSpinBoxAirCoolingMaschineStop, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotAirCoolingMaschineStopChanged);

    connect(ui.doubleSpinBoxWaterCoolingWarning, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotWaterCoolingWarningChanged);
    connect(ui.doubleSpinBoxWaterCoolingAlarm, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotWaterCoolingAlarmChanged);
    connect(ui.doubleSpinBoxWaterCoolingMaschineStop, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotWaterCoolingMaschineStopChanged);

    connect(ui.doubleSpinBoxAlarmLevelMixerVelocity, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotAlarmLevelMixerVelocityChanged);
    connect(ui.doubleSpinBoxWarnLevelMixerVelocity, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotWarnLevelMixerVelocityChanged);
    connect(ui.doubleSpinBoxMaschineStopMixerVelocity, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotMaschineStopMixerVelocityChanged);

    // ui.checkBoxTankPreasureLevel->hide();
    m_WindowSetup = true;
}

AdminSettingsDialog::~AdminSettingsDialog()
{
}

void AdminSettingsDialog::SetAuditTrailProperties()
{
    ui.doubleSpinBoxAcceptanceThreshold->setProperty(kAuditTrail, ui.labelContentBoardAcceptanceThreshold->text());
    ui.comboBoxUsedTriggerOutputs->setProperty(kAuditTrail, ui.labelContentBoardUsedTriggerOutputs->text());
    ui.comboBoxUseSpeedFromISCalcEjectionTime->setProperty(kAuditTrail, ui.labelContentBoardUseSpeedFromISCalcEjectionTime->text());

    // Liquid tank alarm levels
    ui.doubleSpinBoxTankFillingLevelWarning->setProperty(kAuditTrail, ui.labelContentBoardTankFillingLevelWarning->text());
    ui.doubleSpinBoxTankFillingLevelAlarm->setProperty(kAuditTrail, ui.labelContentBoardTankFillingLevelAlarm->text());
    ui.doubleSpinBoxTankFillingLevelMaschineStop->setProperty(kAuditTrail, ui.labelContentBoardTankFillingLevelMaschineStop->text());
    ui.doubleSpinBoxTankFillingLevelMaxValue->setProperty(kAuditTrail, ui.labelContentBoardTankFillingLevelMaxValue->text());

    ui.doubleSpinBoxTankHeaterLevelWarning->setProperty(kAuditTrail, ui.labelContentBoardTankHeaterLevelWarning->text());
    ui.doubleSpinBoxTankHeaterLevelAlarm->setProperty(kAuditTrail, ui.labelContentBoardTankHeaterLevelAlarm->text());
    ui.doubleSpinBoxTankHeaterLevelMaschineStop->setProperty(kAuditTrail, ui.labelContentBoardTankHeaterLevelMaschineStop->text());

    ui.doubleSpinBoxTankPreasureLevelWarning->setProperty(kAuditTrail, ui.labelContentBoardTankPreasureLevelWarning->text());
    ui.doubleSpinBoxTankPreasureLevelAlarm->setProperty(kAuditTrail, ui.labelContentBoardTankPreasureLevelAlarm->text());
    ui.doubleSpinBoxTankPreasureLevelMaschineStop->setProperty(kAuditTrail, ui.labelContentBoardTankPreasureLevelMaschineStop->text());

    ui.doubleSpinBoxDegreeOfPollutionMaschineStop->setProperty(kAuditTrail, ui.labelContentBoardDegreeOfPollutionMaschineStop->text());
    ui.doubleSpinBoxDegreeOfPollutionLevelAlarm->setProperty(kAuditTrail, ui.labelContentBoardDegreeOfPollutionLevelAlarm->text());
    ui.doubleSpinBoxDegreeOfPollutionLevelWarning->setProperty(kAuditTrail, ui.labelContentBoardDegreeOfPollutionLevelWarning->text());

    ui.doubleSpinBoxValveChamberTemperatureWarningLevel->setProperty(kAuditTrail, ui.labelContentBoardValveChamberTemperatureWarningLevel->text());
    ui.doubleSpinBoxValveChamberTemperatureAlarmLevel->setProperty(kAuditTrail, ui.labelContentBoardValveChamberTemperatureAlarmLevel->text());
    ui.doubleSpinBoxValveChamberTemperatureMaschineStopInSec->setProperty(kAuditTrail, ui.labelContentBoardValveChamberTemperatureMaschineStopInSec->text());

    ui.doubleSpinBoxMaxSpeedDeviationWarning->setProperty(kAuditTrail, ui.labelContentBoardMaxSpeedDeviationWarning->text());
    ui.doubleSpinBoxMaxSpeedDeviationAlarm->setProperty(kAuditTrail, ui.labelContentBoardMaxSpeedDeviationAlarm->text());
    ui.doubleSpinBoxMaxSpeedDeviationMaschineStop->setProperty(kAuditTrail, ui.labelContentBoardMaxSpeedDeviationMaschineStop->text());

    ui.doubleSpinBoxMaxCounterBottleEjectedOneAfterTheOther->setProperty(kAuditTrail, ui.groupBoxMaxEjectedBottles->title());
    ui.doubleSpinBoxMaxCounterMiddleTooLowOneAfterTheOther->setProperty(kAuditTrail, ui.groupBoxMaxBottlesNotFilled->title());

    ui.doubleSpinBoxInfoLevel->setProperty(kAuditTrail, ui.labelContentBoardInfoLevel->text());

    ui.doubleSpinBoxWarnLevelHeatingPipe->setProperty(kAuditTrail, ui.labelContentBoardWarnLevelHeatingPipe->text());
    ui.doubleSpinBoxAlarmLevelHeatingPipe->setProperty(kAuditTrail, ui.labelContentBoardAlarmLevelHeatingPipe->text());
    ui.doubleSpinBoxMaschineStopHeatingPipe->setProperty(kAuditTrail, ui.labelContentBoardMaschineStopHeatingPipe->text());

    ui.doubleSpinBoxAirCoolingWarning->setProperty(kAuditTrail, ui.groupBoxAirCooling->title());
    ui.doubleSpinBoxAirCoolingAlarm->setProperty(kAuditTrail, ui.groupBoxAirCooling->title());
    ui.doubleSpinBoxAirCoolingMaschineStop->setProperty(kAuditTrail, ui.groupBoxAirCooling->title());

    ui.doubleSpinBoxWaterCoolingWarning->setProperty(kAuditTrail, ui.groupBoxWaterCooling->title());
    ui.doubleSpinBoxWaterCoolingAlarm->setProperty(kAuditTrail, ui.groupBoxWaterCooling->title());
    ui.doubleSpinBoxWaterCoolingMaschineStop->setProperty(kAuditTrail, ui.groupBoxWaterCooling->title());

    ui.doubleSpinBoxAlarmLevelMixerVelocity->setProperty(kAuditTrail, ui.groupBoxMixerVelocity->title());
    ui.doubleSpinBoxWarnLevelMixerVelocity->setProperty(kAuditTrail, ui.groupBoxMixerVelocity->title());
    ui.doubleSpinBoxMaschineStopMixerVelocity->setProperty(kAuditTrail, ui.groupBoxMixerVelocity->title());

    ui.doubleSpinBoxMixerSpeedRotationPerMinute->setProperty(kAuditTrail, ui.labelContentBoardMixerSpeedRotationPerMinute->text());

    ui.doubleSpinBoxPressureIncreaseWhenFlushing->setProperty(kAuditTrail, ui.labelPressureIncreaseWhenFlushing->text());
}

void AdminSettingsDialog::SlotMaxNumberFilesTrendGraphChanged()
{
    if (GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_MaxNumberFilesTrendGraph = ui.doubleSpinBoxMaxNumberFilesTrendGraph->value();
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void AdminSettingsDialog::SlotPreasureIncreaseWhenFlushingChanged()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_PreasureIncreaseWhenFlushing = ui.doubleSpinBoxPressureIncreaseWhenFlushing->value();
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void AdminSettingsDialog::SlotUseSpeedCalcEjectionTimeChanged(int index)
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        if (GetMainAppCrystalT2()->GetSettingsData()) {
            SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
            if (pSettingsData) {
                if (index == 0)
                    pSettingsData->m_UseSpeedFromISCalcEjectionTime = true;
                else
                    pSettingsData->m_UseSpeedFromISCalcEjectionTime = false;
                GetMainAppCrystalT2()->SaveSettings();
                GetMainAppCrystalT2()->GetImageData()->SetUseSpeedFromISCalcEjectionTime(pSettingsData->m_UseSpeedFromISCalcEjectionTime);
            }
        }
    }
}

void AdminSettingsDialog::SetupWindow()
{
    m_WindowSetup = false;
    if (GetMainAppCrystalT2()->GetSettingsData()) {
        ui.doubleSpinBoxWarnLevelHeatingPipe->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_HeatingPipeTemperatureLevelWarningInPercent);
        ui.doubleSpinBoxAlarmLevelHeatingPipe->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_HeatingPipeTemperatureLevelAlarmInPercent);
        ui.doubleSpinBoxMaschineStopHeatingPipe->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_HeatingPipeLevelMaschineStopTimeInSec);

        ui.doubleSpinBoxTankFillingLevelWarning->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_TankFillingLevelWarningInLiter);
        ui.doubleSpinBoxTankFillingLevelAlarm->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_TankFillingLevelAlarmInLiter);
        ui.doubleSpinBoxTankFillingLevelMaschineStop->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_TankFillingLevelMaschineStopInLiter);
        ui.doubleSpinBoxTankFillingLevelMaxValue->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_TankFillingLevelMaxValue);

        ui.doubleSpinBoxTankHeaterLevelWarning->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_TankTemperatureLevelWarningInDegree);
        ui.doubleSpinBoxTankHeaterLevelAlarm->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_TankTemperatureLevelAlarmInDegree);
        ui.doubleSpinBoxTankHeaterLevelMaschineStop->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_TankTemperatureLevelMaschineStopTimeInSec);

        ui.doubleSpinBoxTankPreasureLevelWarning->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_TankPreasureLevelWarningInPercent);
        ui.doubleSpinBoxTankPreasureLevelAlarm->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_TankPreasureLevelAlarmInPercent);
        ui.doubleSpinBoxTankPreasureLevelMaschineStop->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_TankPreasureLevelMaschineStopTimeInSec);

        ui.doubleSpinBoxDegreeOfPollutionMaschineStop->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_DegreePollutionMaschineStopTimeInSec);
        ui.doubleSpinBoxDegreeOfPollutionLevelAlarm->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_DegreePollutionAlarmInPercent);
        ui.doubleSpinBoxDegreeOfPollutionLevelWarning->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_DegreePollutionWarningInPercent);

        ui.doubleSpinBoxValveChamberTemperatureWarningLevel->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_ValveChamberTemperatureWarningLevelInDegree);
        ui.doubleSpinBoxValveChamberTemperatureAlarmLevel->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_ValveChamberTemperatureAlarmLevelInDegree);
        ui.doubleSpinBoxValveChamberTemperatureMaschineStopInSec->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_ValveChamberTemperatureMaschineStopTimeInSec);

        ui.doubleSpinBoxValvePiezoTemperatureWarningLevel->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_ValvePiezoTemperatureWarningLevelInDegree);
        ui.doubleSpinBoxValvePiezoTemperatureAlarmLevel->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_ValvePiezoTemperatureAlarmLevelInDegree);
        ui.doubleSpinBoxValvePiezoTemperatureMaschineStopInSec->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_ValvePiezoTemperatureMaschineStopTimeInSec);

        ui.doubleSpinBoxMaxSpeedDeviationWarning->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_SpeedDeviationWarningLevelInMPerMin);
        ui.doubleSpinBoxMaxSpeedDeviationAlarm->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_SpeedDeviationAlarmLevelInMPerMin);
        ui.doubleSpinBoxMaxSpeedDeviationMaschineStop->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_SpeedDeviationMaschineStopTimeInSec);

        ui.doubleSpinBoxCounterProducktNotFilledWarning->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_CounterProductNotFilledWarningLevel);
        ui.doubleSpinBoxCounterProducktNotFilledAlarm->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_CounterProductNotFilledAlarmLevel);
        ui.doubleSpinBoxCounterProducktNotFilledMaschineStop->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_CounterProductNotFilledMaschineStopTimeInSec);

        ui.doubleSpinBoxAirCoolingWarning->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_AirCoolingWarningInPercent);
        ui.doubleSpinBoxAirCoolingAlarm->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_AirCoolingAlarmInPercent);
        ui.doubleSpinBoxAirCoolingMaschineStop->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_AirCoolingMaschineStopTimeInSec);

        ui.doubleSpinBoxWaterCoolingWarning->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_WaterCoolingWarning);
        ui.doubleSpinBoxWaterCoolingAlarm->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_WaterCoolingAlarm);
        ui.doubleSpinBoxWaterCoolingMaschineStop->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_WaterCoolingMaschineStopTimeInSec);

        ui.doubleSpinBoxAlarmLevelMixerVelocity->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_MixerMovingAlarmInRPM);
        ui.doubleSpinBoxWarnLevelMixerVelocity->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_MixerMovingWarningInRPM);
        ui.doubleSpinBoxMaschineStopMixerVelocity->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_MixerMovingMaschineStopTimeInSec);

        ui.doubleSpinBoxAcceptanceThreshold->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_EdgeAcceptanceThresholdInPercent);

        ui.doubleSpinBoxMaxCounterBottleEjectedOneAfterTheOther->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_MaxCounterBottleEjectedOneAfterTheOther);
        ui.doubleSpinBoxMaxCounterMiddleTooLowOneAfterTheOther->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_MaxCounterMiddleTooLowOneAfterTheOther);

        ui.doubleSpinBoxMaxNumberFilesTrendGraph->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_MaxNumberFilesTrendGraph);

        double RotationPerMinute = GetMainAppCrystalT2()->GetMixerTerminalValueToRotationPerMinute(GetMainAppCrystalT2()->GetSettingsData()->m_SpeedMixerStepperValue);
        ui.doubleSpinBoxMixerSpeedRotationPerMinute->setValue(RotationPerMinute);

        if (!GetMainAppCrystalT2()->GetSettingsData()->m_WorkWithTwoValves) {
            ui.ContenBoardUsedTriggerOutputs->hide();  // wird nur mit einem Ventil gearbeitet, wird die Auswahlbox nicht benötigt
        }

        ui.comboBoxUseSpeedFromISCalcEjectionTime->setProperty(kAuditTrail, QVariant());
        if (GetMainAppCrystalT2()->GetSettingsData()->m_UseSpeedFromISCalcEjectionTime)
            ui.comboBoxUseSpeedFromISCalcEjectionTime->setCurrentIndex(0);
        else
            ui.comboBoxUseSpeedFromISCalcEjectionTime->setCurrentIndex(1);
        ui.comboBoxUseSpeedFromISCalcEjectionTime->setProperty(kAuditTrail, ui.labelContentBoardUseSpeedFromISCalcEjectionTime->text());

        ui.comboBoxUsedTriggerOutputs->setProperty(kAuditTrail, QVariant());
        if (GetMainAppCrystalT2()->GetCurrentProductData()->m_UsedTriggerOutputs == USE_BOTH_VALVES) {
            ui.comboBoxUsedTriggerOutputs->setCurrentIndex(USE_BOTH_VALVES);
            GetMainAppCrystalT2()->GetVideoDialogShowTriggerPos()->ShowFirstSlider(true);
            GetMainAppCrystalT2()->GetVideoDialogShowTriggerPos()->ShowSecondSlider(true);
        } else {
            if (GetMainAppCrystalT2()->GetCurrentProductData()->m_UsedTriggerOutputs == USE_ONLY_LEFT_VALVE) {
                ui.comboBoxUsedTriggerOutputs->setCurrentIndex(USE_ONLY_LEFT_VALVE);
                GetMainAppCrystalT2()->GetVideoDialogShowTriggerPos()->ShowFirstSlider(true);
                GetMainAppCrystalT2()->GetVideoDialogShowTriggerPos()->ShowSecondSlider(false);
            } else {
                ui.comboBoxUsedTriggerOutputs->setCurrentIndex(USE_ONLY_RIGHT_VALVE);
                GetMainAppCrystalT2()->GetVideoDialogShowTriggerPos()->ShowFirstSlider(false);
                GetMainAppCrystalT2()->GetVideoDialogShowTriggerPos()->ShowSecondSlider(true);
            }
        }
        ui.comboBoxUsedTriggerOutputs->setProperty(kAuditTrail, ui.labelContentBoardUsedTriggerOutputs->text());

        ui.doubleSpinBoxPressureIncreaseWhenFlushing->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_PreasureIncreaseWhenFlushing);
    }
    m_WindowSetup = true;
}

void AdminSettingsDialog::showEvent(QShowEvent*)
{
    SetupWindow();
}

void AdminSettingsDialog::SlotUsedTriggerChanged(int index)
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        GetMainAppCrystalT2()->GetImageData()->SetUsedTriggerOutput(index);
        if (GetMainAppCrystalT2()->GetVideoDialogShowTriggerPos()) {
            if (index == USE_BOTH_VALVES)  // use both
            {
                GetMainAppCrystalT2()->GetVideoDialogShowTriggerPos()->ShowFirstSlider(true);
                GetMainAppCrystalT2()->GetVideoDialogShowTriggerPos()->ShowSecondSlider(true);
                if (GetMainAppCrystalT2()->GetOverviewDialog()) {
                    GetMainAppCrystalT2()->GetOverviewDialog()->ShowCheckBoxFirstTrigger(true);
                    GetMainAppCrystalT2()->GetOverviewDialog()->ShowCheckBoxSecondTrigger(true);
                    GetMainAppCrystalT2()->GetOverviewDialog()->CheckCheckBoxFirstTrigger(true);
                }
            } else {
                if (index == USE_ONLY_LEFT_VALVE)  // use first
                {
                    GetMainAppCrystalT2()->GetVideoDialogShowTriggerPos()->ShowFirstSlider(true);
                    GetMainAppCrystalT2()->GetVideoDialogShowTriggerPos()->ShowSecondSlider(false);
                    if (GetMainAppCrystalT2()->GetOverviewDialog()) {
                        GetMainAppCrystalT2()->GetOverviewDialog()->ShowCheckBoxFirstTrigger(true);
                        GetMainAppCrystalT2()->GetOverviewDialog()->ShowCheckBoxSecondTrigger(false);
                        GetMainAppCrystalT2()->GetOverviewDialog()->CheckCheckBoxFirstTrigger(true);
                    }
                } else {
                    if (index == USE_ONLY_RIGHT_VALVE)  // use second
                    {
                        GetMainAppCrystalT2()->GetVideoDialogShowTriggerPos()->ShowFirstSlider(false);
                        GetMainAppCrystalT2()->GetVideoDialogShowTriggerPos()->ShowSecondSlider(true);
                        if (GetMainAppCrystalT2()->GetOverviewDialog()) {
                            GetMainAppCrystalT2()->GetOverviewDialog()->ShowCheckBoxFirstTrigger(false);
                            GetMainAppCrystalT2()->GetOverviewDialog()->ShowCheckBoxSecondTrigger(true);
                            GetMainAppCrystalT2()->GetOverviewDialog()->CheckCheckBoxSecondTrigger(true);
                        }
                    }
                }
            }
        }
    }
}

void AdminSettingsDialog::SlotAirCoolingWarningChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_AirCoolingWarningInPercent = ui.doubleSpinBoxAirCoolingWarning->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotAirCoolingAlarmChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_AirCoolingAlarmInPercent = ui.doubleSpinBoxAirCoolingAlarm->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotAirCoolingMaschineStopChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_AirCoolingMaschineStopTimeInSec = ui.doubleSpinBoxAirCoolingMaschineStop->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotWaterCoolingWarningChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_WaterCoolingWarning = ui.doubleSpinBoxWaterCoolingWarning->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotWaterCoolingAlarmChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_WaterCoolingAlarm = ui.doubleSpinBoxWaterCoolingAlarm->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotWaterCoolingMaschineStopChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_WaterCoolingMaschineStopTimeInSec = ui.doubleSpinBoxWaterCoolingMaschineStop->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotSpeedMixerChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        pSettingsData->m_SpeedMixerStepperValue = static_cast<int>(GetMainAppCrystalT2()->GetMixerRotationPerMinuteToTerminalValue(ui.doubleSpinBoxMixerSpeedRotationPerMinute->value()));
        GetMainAppCrystalT2()->SaveSettings();
        if (GetMainAppCrystalT2()->GetImageData()) {
            GetMainAppCrystalT2()->GetImageData()->SetMixerVelocity(pSettingsData->m_SpeedMixerStepperValue);
        }
    }
}

void AdminSettingsDialog::SlotAlarmLevelMixerVelocityChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_MixerMovingAlarmInRPM = ui.doubleSpinBoxAlarmLevelMixerVelocity->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotWarnLevelMixerVelocityChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_MixerMovingWarningInRPM = ui.doubleSpinBoxWarnLevelMixerVelocity->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotMaschineStopMixerVelocityChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_MixerMovingMaschineStopTimeInSec = ui.doubleSpinBoxMaschineStopMixerVelocity->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotCounterProducktNotFilledWarningChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_CounterProductNotFilledWarningLevel = ui.doubleSpinBoxCounterProducktNotFilledWarning->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotCounterProducktNotFilledAlarmChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_CounterProductNotFilledAlarmLevel = ui.doubleSpinBoxCounterProducktNotFilledAlarm->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotCounterProducktNotFilledMaschineStopChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_CounterProductNotFilledMaschineStopTimeInSec = ui.doubleSpinBoxCounterProducktNotFilledMaschineStop->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotTankFillingLevelWarningChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_TankFillingLevelWarningInLiter = ui.doubleSpinBoxTankFillingLevelWarning->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotTankFillingLevelAlarmChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_TankFillingLevelAlarmInLiter = ui.doubleSpinBoxTankFillingLevelAlarm->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotTankFillingLevelMaschineStopChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_TankFillingLevelMaschineStopInLiter = ui.doubleSpinBoxTankFillingLevelMaschineStop->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotTankFillingLevelMaxValueChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_TankFillingLevelMaxValue = ui.doubleSpinBoxTankFillingLevelMaxValue->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotWarnLevelHeatingPipeChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_HeatingPipeTemperatureLevelWarningInPercent = ui.doubleSpinBoxWarnLevelHeatingPipe->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotAlarmLevelHeatingPipeChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_HeatingPipeTemperatureLevelAlarmInPercent = ui.doubleSpinBoxAlarmLevelHeatingPipe->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotMaschineStopHeatingPipeChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_HeatingPipeLevelMaschineStopTimeInSec = ui.doubleSpinBoxMaschineStopHeatingPipe->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotTankHeaterLevelWarningChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_TankTemperatureLevelWarningInDegree = ui.doubleSpinBoxTankHeaterLevelWarning->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotTankHeaterLevelAlarmChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_TankTemperatureLevelAlarmInDegree = ui.doubleSpinBoxTankHeaterLevelAlarm->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotTankHeaterLevelMaschineStopChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_TankTemperatureLevelMaschineStopTimeInSec = ui.doubleSpinBoxTankHeaterLevelMaschineStop->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotTankPreasureLevelWarningChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_TankPreasureLevelWarningInPercent = ui.doubleSpinBoxTankPreasureLevelWarning->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotTankPreasureLevelAlarmChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_TankPreasureLevelAlarmInPercent = ui.doubleSpinBoxTankPreasureLevelAlarm->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotTankPreasureLevelMaschineStopChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_TankPreasureLevelMaschineStopTimeInSec = ui.doubleSpinBoxTankPreasureLevelMaschineStop->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotDegreeOfPollutionMaschineStopChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_DegreePollutionMaschineStopTimeInSec = ui.doubleSpinBoxDegreeOfPollutionMaschineStop->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotDegreeOfPollutionLevelAlarmChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_DegreePollutionAlarmInPercent = ui.doubleSpinBoxDegreeOfPollutionLevelAlarm->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotDegreeOfPollutionLevelWarningChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_DegreePollutionWarningInPercent = ui.doubleSpinBoxDegreeOfPollutionLevelWarning->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotValveChamberTemperatureWarningLevelChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_ValveChamberTemperatureWarningLevelInDegree = ui.doubleSpinBoxValveChamberTemperatureWarningLevel->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotValveChamberTemperatureAlarmLevelChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_ValveChamberTemperatureAlarmLevelInDegree = ui.doubleSpinBoxValveChamberTemperatureAlarmLevel->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotValveChamberTemperatureMaschineStopInSecChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_ValveChamberTemperatureMaschineStopTimeInSec = ui.doubleSpinBoxValveChamberTemperatureMaschineStopInSec->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotValvePiezoTemperatureWarningLevelChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_ValvePiezoTemperatureWarningLevelInDegree = ui.doubleSpinBoxValvePiezoTemperatureWarningLevel->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotValvePiezoTemperatureAlarmLevelChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_ValvePiezoTemperatureAlarmLevelInDegree = ui.doubleSpinBoxValvePiezoTemperatureAlarmLevel->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotValvePiezoTemperatureMaschineStopInSecChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_ValvePiezoTemperatureMaschineStopTimeInSec = ui.doubleSpinBoxValvePiezoTemperatureMaschineStopInSec->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotMaxSpeedDeviationWarningChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_SpeedDeviationWarningLevelInMPerMin = ui.doubleSpinBoxMaxSpeedDeviationWarning->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotMaxSpeedDeviationAlarmChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_SpeedDeviationAlarmLevelInMPerMin = ui.doubleSpinBoxMaxSpeedDeviationAlarm->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotMaxSpeedDeviationMaschineStopChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_SpeedDeviationMaschineStopTimeInSec = ui.doubleSpinBoxMaxSpeedDeviationMaschineStop->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotMaxEjectedBottlesChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_MaxCounterBottleEjectedOneAfterTheOther = ui.doubleSpinBoxMaxCounterBottleEjectedOneAfterTheOther->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

// Droplet volume ist too small
void AdminSettingsDialog::SlotSpinBoxMaxBottlesNotFilledChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_MaxCounterMiddleTooLowOneAfterTheOther = ui.doubleSpinBoxMaxCounterMiddleTooLowOneAfterTheOther->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdminSettingsDialog::SlotInfoLevelChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) {
        GetMainAppCrystalT2()->GetImageData()->SetInfoLevel(static_cast<int>(ui.doubleSpinBoxInfoLevel->value()));
    }
}

void AdminSettingsDialog::SetCurrentMaschineState(PluginInterface::MachineState set)
{
    emit SignalMachineStateChanged((int)set);
}

void AdminSettingsDialog::SlotMaschineStateChanged(int set)
{
    if (set == PluginInterface::MachineState::Production) SetInfoLevel(0);
}

void AdminSettingsDialog::SetInfoLevel(int set)
{
    disconnect(ui.doubleSpinBoxInfoLevel, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotInfoLevelChanged);
    ui.doubleSpinBoxInfoLevel->setValue(set);
    connect(ui.doubleSpinBoxInfoLevel, &QDoubleSpinBox::editingFinished, this, &AdminSettingsDialog::SlotInfoLevelChanged);
}

void AdminSettingsDialog::SlotAcceptanceThresholdChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_EdgeAcceptanceThresholdInPercent = ui.doubleSpinBoxAcceptanceThreshold->value();
            GetMainAppCrystalT2()->SaveSettings();
            GetMainAppCrystalT2()->GetImageData()->SetEdgeThreshold(static_cast<int>(ui.doubleSpinBoxAcceptanceThreshold->value()));
        }
    }
}
