#include "CoolingDialog.h"
#include "ControlsWithColorStatus.h"
#include "GlobalConst.h"
#include "ImageData.h"
#include "MainAppCrystalT2.h"
#include "ProductData.h"
#include "colors.h"
#include "settingsdata.h"

#include <audittrail.h>

CoolingDialog::CoolingDialog(MainAppCrystalT2* pMainAppCrystalT2)
    : QWidget(pMainAppCrystalT2),
      m_WindowsStartUp(false),
      m_ControlAirCoolingCameraActualValue(NULL),
      m_ControlAirCoolingCameraLightActualValue(NULL),
      m_ControlAirCoolingValvesActualValue(NULL),
      m_ControlWaterCoolingActualValue(NULL),
      m_ControlFlowTransmitterWaterCoolingCircuitActualValue(NULL),
      m_ControlWaterCoolingTemperatureReturnActualValue(NULL),
      m_StatusAirCoolingCameraAndLightActualValue(ALARM_LEVEL_OK),
      m_StatusAirCoolingGlassActualValue(ALARM_LEVEL_OK),
      m_StatusAirCoolingValvesActualValue(ALARM_LEVEL_OK),
      m_StatusWaterCoolingActualValue(ALARM_LEVEL_OK),
      m_StatusFlowTransmitterWaterCoolingCircuitActualValue(ALARM_LEVEL_OK),
      m_StatusWaterCoolingTemperatureReturnActualValue(ALARM_LEVEL_OK)
{
    ui.setupUi(this);
    m_MainAppCrystalT2 = pMainAppCrystalT2;
    connect(ui.doubleSpinBoxAirCoolingCameraAndLightDefaultValue, &QDoubleSpinBox::editingFinished, this, &CoolingDialog::SlotAirCoolingCameraDefaultValueChanged);
    connect(ui.doubleSpinBoxAirCoolingGlassDefaultValue, &QDoubleSpinBox::editingFinished, this, &CoolingDialog::SlotAirCoolingCameraLightDefaultValueChanged);
    connect(ui.doubleSpinBoxAirCoolingValvesDefaultValue, &QDoubleSpinBox::editingFinished, this, &CoolingDialog::SlotAirCoolingValvesDefaultValueChanged);

    connect(ui.doubleSpinBoxWaterCoolingCircuitDefaultSensor, &QDoubleSpinBox::editingFinished, this, &CoolingDialog::SlotWaterCoolingCircuitDefaultSensorChanged);
    connect(ui.doubleSpinBoxWaterCoolingTemperatureDefault, &QDoubleSpinBox::editingFinished, this, &CoolingDialog::SlotWaterCoolingTemperatureDefaultChanged);
    bool SmallerAsThreshold = false;
    // Air cooling current values
    m_ControlAirCoolingCameraActualValue = new ControlsWithColorStatus(ui.doubleSpinBoxAirCoolingCameraAndLightActualValue, SmallerAsThreshold);
    m_ControlAirCoolingCameraLightActualValue = new ControlsWithColorStatus(ui.doubleSpinBoxAirCoolingGlassActualValue, SmallerAsThreshold);
    m_ControlAirCoolingValvesActualValue = new ControlsWithColorStatus(ui.doubleSpinBoxAirCoolingValvesActualValue, SmallerAsThreshold);
    // Water cooling current values
     m_ControlWaterCoolingTemperatureReturnActualValue = new ControlsWithColorStatus(ui.doubleSpinBoxWaterCoolingTemperatureReturn, SmallerAsThreshold);
    SetAuditTrailProperties();
    SetRequiredAccessLevel();

    ui.labelDashBoardWaterCoolingDefaultValue->hide();
    ui.doubleSpinBoxWaterCoolingDefaultValue->hide();
    ui.labelDashBoardWaterCoolingCircuitDefaultSensor->hide();
    ui.doubleSpinBoxWaterCoolingCircuitDefaultSensor->hide();
}

CoolingDialog::~CoolingDialog()
{
}

void CoolingDialog::SetRequiredAccessLevel()
{
    ui.groupBoxAirCoolingDefaultValues->setProperty(kRequiredAccessLevel, kAccessLevelAdmin);
    ui.groupBoxWaterCoolingDefaultValues->setProperty(kRequiredAccessLevel, kAccessLevelAdmin);
}

void CoolingDialog::SetAuditTrailProperties()
{
    ui.doubleSpinBoxAirCoolingCameraAndLightDefaultValue->setProperty(kAuditTrail, ui.labelDashBoardAirCoolingCameraDefaultValue->text());
    ui.doubleSpinBoxAirCoolingGlassDefaultValue->setProperty(kAuditTrail, ui.labelDashBoardAirCoolingCameraLightDefaultValue->text());
    ui.doubleSpinBoxAirCoolingValvesDefaultValue->setProperty(kAuditTrail, ui.labelDashBoardAirCoolingValvesDefaultValue->text());

    ui.doubleSpinBoxWaterCoolingDefaultValue->setProperty(kAuditTrail, ui.labelDashBoardWaterCoolingDefaultValue->text());
    ui.doubleSpinBoxWaterCoolingCircuitDefaultSensor->setProperty(kAuditTrail, ui.labelDashBoardWaterCoolingCircuitDefaultSensor->text());
    ui.doubleSpinBoxWaterCoolingTemperatureDefault->setProperty(kAuditTrail, tr("Default Water Cooling Temperature"));

}

void CoolingDialog::showEvent(QShowEvent* ev)
{
    m_WindowsStartUp = false;
    if (GetMainAppCrystalT2()->GetCurrentProductData()) {
        ui.doubleSpinBoxAirCoolingCameraAndLightDefaultValue->setValue(GetMainAppCrystalT2()->GetCurrentProductData()->m_ValueAirCoolingCameraLitersPerMinute);
        ui.doubleSpinBoxAirCoolingGlassDefaultValue->setValue(GetMainAppCrystalT2()->GetCurrentProductData()->m_ValueAirCoolingLightLitersPerMinute);
        ui.doubleSpinBoxAirCoolingValvesDefaultValue->setValue(GetMainAppCrystalT2()->GetCurrentProductData()->m_ValueAirCoolingValveLitersPerMinute);

        ui.doubleSpinBoxWaterCoolingDefaultValue->setValue(GetMainAppCrystalT2()->GetCurrentWaterCoolingStrokeValue());
        ui.doubleSpinBoxWaterCoolingCircuitDefaultSensor->setValue(GetMainAppCrystalT2()->GetCurrentProductData()->m_ValueWaterCoolingCircuitDefaultSensor);
        ui.doubleSpinBoxWaterCoolingTemperatureDefault->setValue(GetMainAppCrystalT2()->GetCurrentProductData()->m_ValveWaterCoolingDefaultTemperature);
    }
    m_WindowsStartUp = true;
}

void CoolingDialog::SlotAirCoolingCameraDefaultValueChanged()
{
    if (m_WindowsStartUp && GetMainAppCrystalT2()->GetCurrentProductData()) {
        double dvalue = ui.doubleSpinBoxAirCoolingCameraAndLightDefaultValue->value();
        GetMainAppCrystalT2()->GetCurrentProductData()->m_ValueAirCoolingCameraLitersPerMinute = dvalue;
        GetMainAppCrystalT2()->SaveProductData();
        GetMainAppCrystalT2()->SetAirCoolingCamera();  // write to terminal
    }
}

void CoolingDialog::SlotAirCoolingCameraLightDefaultValueChanged()
{
    if (m_WindowsStartUp && GetMainAppCrystalT2()->GetCurrentProductData()) {
        double dvalue = ui.doubleSpinBoxAirCoolingGlassDefaultValue->value();
        GetMainAppCrystalT2()->GetCurrentProductData()->m_ValueAirCoolingLightLitersPerMinute = dvalue;
        GetMainAppCrystalT2()->SaveProductData();
        GetMainAppCrystalT2()->SetAirCoolingLight();  // write to terminal
    }
}

void CoolingDialog::SlotAirCoolingValvesDefaultValueChanged()
{
    if (m_WindowsStartUp && GetMainAppCrystalT2()->GetCurrentProductData()) {
        double dvalue = ui.doubleSpinBoxAirCoolingValvesDefaultValue->value();
        GetMainAppCrystalT2()->GetCurrentProductData()->m_ValueAirCoolingValveLitersPerMinute = dvalue;
        GetMainAppCrystalT2()->SaveProductData();
        GetMainAppCrystalT2()->SetAirCoolingValve();  // write to terminal
    }
}

// void OverviewDialog::SetLiquidTankPreasure(double set)
//{
//    if (m_ControlLiquidTankPreasure && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData() && GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusCurrentLiquidTankPreasure &&
//        GetMainAppCrystalT2()->GetCurrentProductData()) {
//        bool MaschineStopInMin = true;
//        double DefaultPreasureInBar = GetMainAppCrystalT2()->GetCurrentProductData()->m_DefaultPreasureInBar;
//        if (DefaultPreasureInBar != 0.0) {
//            double ValueInPercent = (100.0 / DefaultPreasureInBar) * set;
//            double MaschineStopThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_TankPreasureLevelMaschineStopTimeInSec;
//            double WarnThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_TankPreasureLevelWarningInPercent;
//            double AlarmThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_TankPreasureLevelAlarmInPercent;
//            double Default = 100.0;
//            m_StatusLiquidTankPreasure = m_ControlLiquidTankPreasure->SetValueAndAlarmStatus(set, ValueInPercent, Default, WarnThreshold, AlarmThreshold, MaschineStopThreshold, MaschineStopInMin);
//        } else {
//            m_StatusLiquidTankPreasure = ALARM_LEVEL_OK;
//            m_ControlLiquidTankPreasure->SetShowValue(DefaultPreasureInBar);
//            m_ControlLiquidTankPreasure->StopTimerColorStatus();
//            m_ControlLiquidTankPreasure->SetBackgroundColor(HMIColor::ContentBoard);
//        }
//    } else {
//        m_StatusLiquidTankPreasure = ALARM_LEVEL_OK;
//        ui.doubleSpinBoxLiquidTankPreasure->setValue(set);
//        if (m_ControlLiquidTankPreasure) {
//            m_ControlLiquidTankPreasure->StopTimerColorStatus();
//            m_ControlLiquidTankPreasure->SetBackgroundColor(HMIColor::ContentBoard);
//        }
//    }
//}

void CoolingDialog::SetActualAirCoolingCamera(double set)
{
    if (m_ControlAirCoolingCameraActualValue && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetCurrentProductData() && GetMainAppCrystalT2()->GetSettingsData() &&
        GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusAirCoolingCameraActualValue) {
        bool MaschineStopInSec = true;
        double DefaultAirCoolingCamera = GetMainAppCrystalT2()->GetCurrentProductData()->m_ValueAirCoolingCameraLitersPerMinute;
        if (DefaultAirCoolingCamera != 0.0) {
            double ValueInPercent = (100.0 / DefaultAirCoolingCamera) * set;
            double MaschineStopThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_AirCoolingMaschineStopTimeInSec;
            double WarnThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_AirCoolingWarningInPercent;
            double AlarmThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_AirCoolingAlarmInPercent;
            double Default = 100.0;
            m_StatusAirCoolingCameraAndLightActualValue =
                m_ControlAirCoolingCameraActualValue->SetValueAndAlarmStatus(set, ValueInPercent, Default, WarnThreshold, AlarmThreshold, MaschineStopThreshold, MaschineStopInSec);
        } else {
            m_StatusAirCoolingCameraAndLightActualValue = ALARM_LEVEL_OK;
            m_ControlAirCoolingCameraActualValue->SetShowValue(DefaultAirCoolingCamera);
            m_ControlAirCoolingCameraActualValue->StopTimerColorStatus();
            m_ControlAirCoolingCameraActualValue->SetBackgroundColor(HMIColor::ContentBoard);
        }
    } else {
        m_StatusAirCoolingCameraAndLightActualValue = ALARM_LEVEL_OK;
        ui.doubleSpinBoxAirCoolingCameraAndLightActualValue->setValue(set);
        if (m_ControlAirCoolingCameraActualValue) {
            m_ControlAirCoolingCameraActualValue->StopTimerColorStatus();
            m_ControlAirCoolingCameraActualValue->SetBackgroundColor(HMIColor::ContentBoard);
        }
    }
}

void CoolingDialog::SetActualAirCoolingCameraLight(double set)
{
    if (m_ControlAirCoolingCameraLightActualValue && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetCurrentProductData() && GetMainAppCrystalT2()->GetSettingsData() &&
        GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusAirCoolingCameraLightActualValue) {
        bool MaschineStopInSec = true;
        double DefaultAirCoolingCameraLight = GetMainAppCrystalT2()->GetCurrentProductData()->m_ValueAirCoolingLightLitersPerMinute;
        if (DefaultAirCoolingCameraLight != 0.0) {
            double ValueInPercent = (100.0 / DefaultAirCoolingCameraLight) * set;
            double MaschineStopThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_AirCoolingMaschineStopTimeInSec;
            double WarnThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_AirCoolingWarningInPercent;
            double AlarmThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_AirCoolingAlarmInPercent;
            double Default = 100.0;
            m_StatusAirCoolingGlassActualValue =
                m_ControlAirCoolingCameraLightActualValue->SetValueAndAlarmStatus(set, ValueInPercent, Default, WarnThreshold, AlarmThreshold, MaschineStopThreshold, MaschineStopInSec);
        } else {
            m_StatusAirCoolingGlassActualValue = ALARM_LEVEL_OK;
            m_ControlAirCoolingCameraLightActualValue->SetShowValue(DefaultAirCoolingCameraLight);
            m_ControlAirCoolingCameraLightActualValue->StopTimerColorStatus();
            m_ControlAirCoolingCameraLightActualValue->SetBackgroundColor(HMIColor::ContentBoard);
        }

    } else {
        m_StatusAirCoolingGlassActualValue = ALARM_LEVEL_OK;
        ui.doubleSpinBoxAirCoolingGlassActualValue->setValue(set);
        if (m_ControlAirCoolingCameraLightActualValue) {
            m_ControlAirCoolingCameraLightActualValue->StopTimerColorStatus();
            m_ControlAirCoolingCameraLightActualValue->SetBackgroundColor(HMIColor::ContentBoard);
        }
    }
}

void CoolingDialog::SetActualAirCoolingValves(double set)
{
    if (m_ControlAirCoolingValvesActualValue && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetCurrentProductData() && GetMainAppCrystalT2()->GetSettingsData() &&
        GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusAirCoolingValvesActualValue) {
        bool MaschineStopInSec = true;
        double DefaultAirCoolingValves = GetMainAppCrystalT2()->GetCurrentProductData()->m_ValueAirCoolingValveLitersPerMinute;
        if (DefaultAirCoolingValves != 0.0) {
            double ValueInPercent = (100.0 / DefaultAirCoolingValves) * set;
            double MaschineStopThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_AirCoolingMaschineStopTimeInSec;
            double WarnThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_AirCoolingWarningInPercent;
            double AlarmThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_AirCoolingAlarmInPercent;
            double Default = 100.0;
            m_StatusAirCoolingValvesActualValue =
                m_ControlAirCoolingValvesActualValue->SetValueAndAlarmStatus(set, ValueInPercent, Default, WarnThreshold, AlarmThreshold, MaschineStopThreshold, MaschineStopInSec);
        } else {
            m_StatusAirCoolingValvesActualValue = ALARM_LEVEL_OK;
            m_ControlAirCoolingValvesActualValue->SetShowValue(DefaultAirCoolingValves);
            m_ControlAirCoolingValvesActualValue->StopTimerColorStatus();
            m_ControlAirCoolingValvesActualValue->SetBackgroundColor(HMIColor::ContentBoard);
        }

    } else {
        m_StatusAirCoolingValvesActualValue = ALARM_LEVEL_OK;
        ui.doubleSpinBoxAirCoolingValvesActualValue->setValue(set);
        if (m_ControlAirCoolingValvesActualValue) {
            m_ControlAirCoolingValvesActualValue->StopTimerColorStatus();
            m_ControlAirCoolingValvesActualValue->SetBackgroundColor(HMIColor::ContentBoard);
        }
    }
}

//void CoolingDialog::SetWaterCoolingStrokeValue(double set)
//{
//    ui.doubleSpinBoxWaterCoolingDefaultValue->setValue(set);
//    if (GetMainAppCrystalT2()) {
//        GetMainAppCrystalT2()->SetWaterCoolingDefault(set);
//    }
//}

// Vorgabewerte vom Wasserkühlkreislauf
// Volumenstromregler Wasserkühlkreislauf Istwert
//void CoolingDialog::SlotWaterCoolingDefaultValueChanged()
//{
//    if (m_WindowsStartUp && GetMainAppCrystalT2()->GetCurrentProductData()) {
//        double dvalue = ui.doubleSpinBoxWaterCoolingDefaultValue->value();
//        GetMainAppCrystalT2()->GetCurrentProductData()->m_ValueWaterCoolingDefaultLitersPerMinute = dvalue;
//        GetMainAppCrystalT2()->SaveProductData();
//        GetMainAppCrystalT2()->SetWaterCoolingDefault();  // write to terminal
//    }
//}
// only for tests
void CoolingDialog::SlotWaterCoolingCircuitDefaultSensorChanged()
{
    if (m_WindowsStartUp && GetMainAppCrystalT2()->GetCurrentProductData()) {
        double dvalue = ui.doubleSpinBoxWaterCoolingCircuitDefaultSensor->value();
        GetMainAppCrystalT2()->SetActualTemperaturWaterCoolingReturn(dvalue);
      
    }
}

void CoolingDialog::SlotWaterCoolingTemperatureDefaultChanged()
{
    if (m_WindowsStartUp && GetMainAppCrystalT2()->GetCurrentProductData()) {
        double dvalue = ui.doubleSpinBoxWaterCoolingTemperatureDefault->value();
        GetMainAppCrystalT2()->GetCurrentProductData()->m_ValveWaterCoolingDefaultTemperature = dvalue;
        GetMainAppCrystalT2()->SaveProductData();
    }
}

// Aktuelle Eingangsdaten vom Wasserkühlkreislauf
// Volumenstromregler Wasserkühlkreislauf Sollwert
void CoolingDialog::SetActualWaterCooling(double value)
{
    if (m_ControlWaterCoolingActualValue) {
        if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetCurrentProductData()) {
            bool MaschineStopInSec = true;
            double WaterCoolingDefaultLitersPerMinute = GetMainAppCrystalT2()->GetCurrentProductData()->m_ValueWaterCoolingDefaultLitersPerMinute;
            if (WaterCoolingDefaultLitersPerMinute != 0.0) {
                double MaschineStopThreshold = 30.0;
                double WarnThreshold = 5.0;
                double AlarmThreshold = 15.0;
                m_StatusWaterCoolingActualValue =
                    m_ControlWaterCoolingActualValue->SetValueAndAlarmStatus(value, value, WaterCoolingDefaultLitersPerMinute, WarnThreshold, AlarmThreshold, MaschineStopThreshold, MaschineStopInSec);
            } else {
                m_StatusWaterCoolingActualValue = ALARM_LEVEL_OK;
                m_ControlWaterCoolingActualValue->SetBackgroundColor(HMIColor::ContentBoard);
            }
        }
    } else {
        ui.doubleSpinBoxWaterCoolingActualValue->setValue(value);
    }
}
// Durchflusstransmitter Wasserkühlkreislauf
// Wert vom Durchflussmesser in Liter pro Minute
void CoolingDialog::SetActualFlowTransmitterWaterCoolingCircuit(double value)
{
    if (m_ControlFlowTransmitterWaterCoolingCircuitActualValue) {
        if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
            bool MaschineStopInSec = true;
        }
    } else {
        ui.doubleSpinBoxFlowTransmitterWaterCoolingCircuitActualValue->setValue(value);
    }
}
// Volumenstromregler Wasserkühlkreislauf Istwert Sensor
// Temperaturwert Rücklauf in Grad Celius von 0 bis 150 Grad
void CoolingDialog::SetActualTemperaturWaterCoolingReturn(double value)
{
    m_TemperaturWaterCooling = value;
    if (m_ControlWaterCoolingTemperatureReturnActualValue) {
        if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
            bool MaschineStopInSec = true;
            double MaschineStopThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_WaterCoolingMaschineStopTimeInSec;
            double WarnThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_WaterCoolingWarning;
            double AlarmThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_WaterCoolingAlarm;
            double Default = 0.0;
            m_StatusWaterCoolingTemperatureReturnActualValue =
                m_ControlWaterCoolingTemperatureReturnActualValue->SetValueAndAlarmStatus(value, value, Default, WarnThreshold, AlarmThreshold, MaschineStopThreshold, MaschineStopInSec);
        }
    } else {
        ui.doubleSpinBoxWaterCoolingTemperatureReturn->setValue(value);
    }
}

double CoolingDialog::GetActualTemperaturWaterCoolingReturn()
{
    return m_TemperaturWaterCooling;
}
