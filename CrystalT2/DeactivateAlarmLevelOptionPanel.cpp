#include "DeactivateAlarmLevelOptionPanel.h"
#include "MainAppCrystalT2.h"
#include "Settingsdata.h"

#include <audittrail.h>

DeactivateAlarmLevelOptionPanel::DeactivateAlarmLevelOptionPanel(MainAppCrystalT2* pMainAppCrystalT2) : OptionPanel(pMainAppCrystalT2)
{
    m_MainAppCrystalT2 = pMainAppCrystalT2;
    QFormLayout* formLayout = new QFormLayout();
    QGroupBox* groupBoxDeactivateAlarms = new QGroupBox(tr("Deactivate Alarms"));

    m_CheckBoxEnableStatusDegreeOfPolution = new QCheckBox();
    m_CheckBoxEnableStatusCurrentLiquidTankPreasure = new QCheckBox();
    m_CheckBoxEnableStatusCurrentLiquidTankFilling = new QCheckBox();
    m_CheckBoxEnableStatusCurrentLiquidTankTemp = new QCheckBox();
    m_CheckBoxEnableStatusPiezoTempLeftValve = new QCheckBox();
    m_CheckBoxEnableStatusChamperTempLeftValve = new QCheckBox();
    m_CheckBoxEnableStatusPiezoTempRightValve = new QCheckBox();
    m_CheckBoxEnableStatusChamperTempRightValve = new QCheckBox();
    m_CheckBoxEnableStatusSpeedDeviationBetweenCameraAndIS = new QCheckBox();
    m_CheckBoxEnableStatusHeatingPipeTemp = new QCheckBox();
    m_CheckBoxEnableStatusCounterProducNotFilled = new QCheckBox();
    m_CheckBoxEnableStatusCounterBottleEjectedOneAfterTheOther = new QCheckBox();
    m_CheckBoxEnableStatusCounterMiddleTooLowOneAfterTheOther = new QCheckBox();
    m_CheckBoxEnableStatusCounterProducNotFilled = new QCheckBox();
    m_CheckBoxEnableStatusAirCooling = new QCheckBox();
    m_CheckBoxEnableStatusWaterCooling = new QCheckBox();
    m_CheckBoxEnableStatusMixer = new QCheckBox();

    m_TextEnableStatusDegreeOfPolution = new QLabel(tr("Pollution"));
    m_TextEnableStatusCurrentLiquidTankPreasure = new QLabel(tr("Pressure"));
    m_TextEnableStatusCurrentLiquidTankFilling = new QLabel(tr("Filling Level"));
    m_TextEnableStatusCurrentLiquidTankTemp = new QLabel(tr("Tank Temperatur"));
    m_TextEnableStatusPiezoTempLeftValve = new QLabel(tr("Piezo Temp Left"));
    m_TextEnableStatusChamperTempLeftValve = new QLabel(tr("Chamber Temp Left"));
    m_TextEnableStatusPiezoTempRightValve = new QLabel(tr("Piezo Temp Right"));
    m_TextEnableStatusChamperTempRightValve = new QLabel(tr("Chamber Temp Right"));
    m_TextEnableStatusSpeedDeviationBetweenCameraAndIS = new QLabel(tr("Speed Deviation Camera - IS"));
    m_TextEnableStatusHeatingPipeTemp = new QLabel(tr("Heating Pipe Temp"));
    m_TextEnableStatusCounterProducNotFilled = new QLabel(tr("Ejection Sensor"));
    m_TextEnableStatusCounterBottleEjectedOneAfterTheOther = new QLabel(tr("Bottle Ejected One After The Other"));
    m_TextEnableStatusCounterMiddleTooLowOneAfterTheOther = new QLabel(tr("Middle Too Low One After The Other"));
    m_TextEnableStatusAirCooling = new QLabel(tr("Air Cooling"));
    m_TextEnableStatusWaterCooling = new QLabel(tr("Water Cooling"));
    m_TextEnableStatusMixer = new QLabel(tr("Mixer Running"));

    connect(m_CheckBoxEnableStatusDegreeOfPolution, &QCheckBox::stateChanged, this, &DeactivateAlarmLevelOptionPanel::SlotEnableStatusDegreeOfPolution);
    connect(m_CheckBoxEnableStatusCurrentLiquidTankPreasure, &QCheckBox::stateChanged, this, &DeactivateAlarmLevelOptionPanel::SlotEnableStatusCurrentLiquidTankPreasure);
    connect(m_CheckBoxEnableStatusCurrentLiquidTankFilling, &QCheckBox::stateChanged, this, &DeactivateAlarmLevelOptionPanel::SlotEnableStatusCurrentLiquidTankFilling);
    connect(m_CheckBoxEnableStatusCurrentLiquidTankTemp, &QCheckBox::stateChanged, this, &DeactivateAlarmLevelOptionPanel::SlotEnableStatusCurrentLiquidTankTemp);
    connect(m_CheckBoxEnableStatusPiezoTempLeftValve, &QCheckBox::stateChanged, this, &DeactivateAlarmLevelOptionPanel::SlotEnableStatusPiezoTempLeftValve);
    connect(m_CheckBoxEnableStatusChamperTempLeftValve, &QCheckBox::stateChanged, this, &DeactivateAlarmLevelOptionPanel::SlotEnableStatusChamperTempLeftValve);
    connect(m_CheckBoxEnableStatusPiezoTempRightValve, &QCheckBox::stateChanged, this, &DeactivateAlarmLevelOptionPanel::SlotEnableStatusPiezoTempRightValve);
    connect(m_CheckBoxEnableStatusChamperTempRightValve, &QCheckBox::stateChanged, this, &DeactivateAlarmLevelOptionPanel::SlotEnableStatusChamperTempRightValve);
    connect(m_CheckBoxEnableStatusSpeedDeviationBetweenCameraAndIS, &QCheckBox::stateChanged, this, &DeactivateAlarmLevelOptionPanel::SlotEnableStatusSpeedDeviationBetweenCameraAndIS);
    connect(m_CheckBoxEnableStatusHeatingPipeTemp, &QCheckBox::stateChanged, this, &DeactivateAlarmLevelOptionPanel::SlotEnableStatusHeatingPipeTemp);
    connect(m_CheckBoxEnableStatusCounterProducNotFilled, &QCheckBox::stateChanged, this, &DeactivateAlarmLevelOptionPanel::SlotEnableStatusCounterProducNotFilled);
    connect(m_CheckBoxEnableStatusCounterBottleEjectedOneAfterTheOther, &QCheckBox::stateChanged, this, &DeactivateAlarmLevelOptionPanel::SlotEnableStatusCounterBottleEjectedOneAfterTheOther);
    connect(m_CheckBoxEnableStatusCounterMiddleTooLowOneAfterTheOther, &QCheckBox::stateChanged, this, &DeactivateAlarmLevelOptionPanel::SlotEnableStatusCounterMiddleTooLowOneAfterTheOther);
    connect(m_CheckBoxEnableStatusAirCooling, &QCheckBox::stateChanged, this, &DeactivateAlarmLevelOptionPanel::SlotEnableStatusAirCooling);
    connect(m_CheckBoxEnableStatusWaterCooling, &QCheckBox::stateChanged, this, &DeactivateAlarmLevelOptionPanel::SlotEnableStatusWaterCooling);
    connect(m_CheckBoxEnableStatusMixer, &QCheckBox::stateChanged, this, &DeactivateAlarmLevelOptionPanel::SlotEnableStatusMixerRunning);

    formLayout->insertRow(0, m_TextEnableStatusDegreeOfPolution, m_CheckBoxEnableStatusDegreeOfPolution);
    formLayout->insertRow(1, m_TextEnableStatusCurrentLiquidTankPreasure, m_CheckBoxEnableStatusCurrentLiquidTankPreasure);
    formLayout->insertRow(2, m_TextEnableStatusCurrentLiquidTankFilling, m_CheckBoxEnableStatusCurrentLiquidTankFilling);
    formLayout->insertRow(3, m_TextEnableStatusCurrentLiquidTankTemp, m_CheckBoxEnableStatusCurrentLiquidTankTemp);
    formLayout->insertRow(4, m_TextEnableStatusPiezoTempLeftValve, m_CheckBoxEnableStatusPiezoTempLeftValve);
    formLayout->insertRow(5, m_TextEnableStatusChamperTempLeftValve, m_CheckBoxEnableStatusChamperTempLeftValve);
    formLayout->insertRow(6, m_TextEnableStatusPiezoTempRightValve, m_CheckBoxEnableStatusPiezoTempRightValve);
    formLayout->insertRow(7, m_TextEnableStatusChamperTempRightValve, m_CheckBoxEnableStatusChamperTempRightValve);
    formLayout->insertRow(8, m_TextEnableStatusSpeedDeviationBetweenCameraAndIS, m_CheckBoxEnableStatusSpeedDeviationBetweenCameraAndIS);
    formLayout->insertRow(9, m_TextEnableStatusHeatingPipeTemp, m_CheckBoxEnableStatusHeatingPipeTemp);
    formLayout->insertRow(10, m_TextEnableStatusCounterProducNotFilled, m_CheckBoxEnableStatusCounterProducNotFilled);
    formLayout->insertRow(11, m_TextEnableStatusCounterBottleEjectedOneAfterTheOther, m_CheckBoxEnableStatusCounterBottleEjectedOneAfterTheOther);
    formLayout->insertRow(12, m_TextEnableStatusCounterMiddleTooLowOneAfterTheOther, m_CheckBoxEnableStatusCounterMiddleTooLowOneAfterTheOther);
    formLayout->insertRow(13, m_TextEnableStatusAirCooling, m_CheckBoxEnableStatusAirCooling);
    formLayout->insertRow(14, m_TextEnableStatusWaterCooling, m_CheckBoxEnableStatusWaterCooling);
    formLayout->insertRow(15, m_TextEnableStatusMixer, m_CheckBoxEnableStatusMixer);

    groupBoxDeactivateAlarms->setLayout(formLayout);

    QVBoxLayout* mainLayout = new QVBoxLayout();

    mainLayout->addWidget(groupBoxDeactivateAlarms);
    setLayout(mainLayout);
    SetAuditTrailProperties();
    SetRequiredAccessLevel();
}

void DeactivateAlarmLevelOptionPanel::SetRequiredAccessLevel()
{
}

void DeactivateAlarmLevelOptionPanel::SetAuditTrailProperties()
{
}

void DeactivateAlarmLevelOptionPanel::ShowRow(int row, bool sh)
{
}

void DeactivateAlarmLevelOptionPanel::SlotEnableStatusMixerRunning(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        if (set == Qt::Checked)
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusMixer = false;
        else
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusMixer = true;
    }
}

void DeactivateAlarmLevelOptionPanel::SlotEnableStatusDegreeOfPolution(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        if (set == Qt::Checked)
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusDegreeOfPolution = false;
        else
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusDegreeOfPolution = true;
    }
}
void DeactivateAlarmLevelOptionPanel::SlotEnableStatusCurrentLiquidTankPreasure(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        if (set == Qt::Checked)
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusCurrentLiquidTankPreasure = false;
        else
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusCurrentLiquidTankPreasure = true;
    }
}
void DeactivateAlarmLevelOptionPanel::SlotEnableStatusCurrentLiquidTankFilling(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        if (set == Qt::Checked)
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusCurrentLiquidTankFilling = false;
        else
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusCurrentLiquidTankFilling = true;
    }
}
void DeactivateAlarmLevelOptionPanel::SlotEnableStatusCurrentLiquidTankTemp(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        if (set == Qt::Checked)
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusCurrentLiquidTankTemp = false;
        else
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusCurrentLiquidTankTemp = true;
    }
}
void DeactivateAlarmLevelOptionPanel::SlotEnableStatusPiezoTempLeftValve(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        if (set == Qt::Checked)
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusPiezoTempLeftValve = false;
        else
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusPiezoTempLeftValve = true;
    }
}
void DeactivateAlarmLevelOptionPanel::SlotEnableStatusChamperTempLeftValve(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        if (set == Qt::Checked)
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusChamperTempLeftValve = false;
        else
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusChamperTempLeftValve = true;
    }
}
void DeactivateAlarmLevelOptionPanel::SlotEnableStatusPiezoTempRightValve(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        if (set == Qt::Checked)
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusPiezoTempRightValve = false;
        else
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusPiezoTempRightValve = true;
    }
}
void DeactivateAlarmLevelOptionPanel::SlotEnableStatusChamperTempRightValve(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        if (set == Qt::Checked)
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusChamperTempRightValve = false;
        else
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusChamperTempRightValve = true;
    }
}
void DeactivateAlarmLevelOptionPanel::SlotEnableStatusSpeedDeviationBetweenCameraAndIS(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        if (set == Qt::Checked)
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusSpeedDeviationBetweenCameraAndIS = false;
        else
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusSpeedDeviationBetweenCameraAndIS = true;
    }
}
void DeactivateAlarmLevelOptionPanel::SlotEnableStatusHeatingPipeTemp(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        if (set == Qt::Checked)
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusHeatingPipeTemp = false;
        else
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusHeatingPipeTemp = true;
    }
}
void DeactivateAlarmLevelOptionPanel::SlotEnableStatusCounterProducNotFilled(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        if (set == Qt::Checked)
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusCounterProducNotFilled = false;
        else
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusCounterProducNotFilled = true;
    }
}
void DeactivateAlarmLevelOptionPanel::SlotEnableStatusCounterBottleEjectedOneAfterTheOther(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        if (set == Qt::Checked)
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusCounterBottleEjectedOneAfterTheOther = false;
        else
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusCounterBottleEjectedOneAfterTheOther = true;
    }
}
void DeactivateAlarmLevelOptionPanel::SlotEnableStatusCounterMiddleTooLowOneAfterTheOther(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        if (set == Qt::Checked)
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusCounterMiddleTooLowOneAfterTheOther = false;
        else
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusCounterMiddleTooLowOneAfterTheOther = true;
    }
}
void DeactivateAlarmLevelOptionPanel::SlotEnableStatusAirCooling(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        if (set == Qt::Checked)
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusAirCooling = false;
        else
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusAirCooling = true;
    }
}
void DeactivateAlarmLevelOptionPanel::SlotEnableStatusWaterCooling(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        if (set == Qt::Checked)
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusWaterCooling = false;
        else
            GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusWaterCooling = true;
    }
}
