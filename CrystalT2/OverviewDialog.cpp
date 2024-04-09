#include "OverviewDialog.h"
#include "ControlsWithColorStatus.h"
#include "GlobalConst.h"
#include "MainAppCrystalT2.h"
#include "Productdata.h"
#include "SettingsData.h"
#include "ValveDialog.h"
#include "VideoDialog.h"
#include "colors.h"
#include "qstyle.h"

#include <audittrail.h>

OverviewDialog::OverviewDialog(MainAppCrystalT2* pMainAppCrystalT2, QWidget* parent)
    : QWidget(parent),
      m_ControlDegreeOfPollution(NULL),
      m_ControlLiquidTankFilling(NULL),
      m_ControlLiquidTankPreasure(NULL),
      m_ControlLiquidTankTemperature(NULL),
      m_ControlPiezoTemperatureLeftValve(NULL),
      m_ControlPiezoTemperatureRightValve(NULL),
      m_ControlChamberTemperatureLeftValve(NULL),
      m_ControlChamberTemperatureRightValve(NULL),
      m_ControlChamberHeatingPipeTemperatur(NULL),
      m_ControlCounterProducNotFilled(NULL),
      m_StatusLiquidTankPreasure(ALARM_LEVEL_OK),
      m_StatusLiquidTankFilling(ALARM_LEVEL_OK),
      m_StatusLiquidTankTemperature(ALARM_LEVEL_OK),
      m_StatusDegreeOfPollution(ALARM_LEVEL_OK),
      m_StatusPiezoTempLeftValve(ALARM_LEVEL_OK),
      m_StatusChamperTempLeftValve(ALARM_LEVEL_OK),
      m_StatusPiezoTempRightValve(ALARM_LEVEL_OK),
      m_StatusChamperTempRightValve(ALARM_LEVEL_OK),
      m_StatusCounterProducNotFilled(ALARM_LEVEL_OK),
      m_NumberErrorLiquidTankPressure(0)
{
    ui.setupUi(this);
    m_MainAppCrystalT2 = pMainAppCrystalT2;
    ui.doubleSpinBoxLiquidTankTemperature->setValue(INVALID_TEMPERATURE_VALUE);
    ui.checkBoxShowFirst->setChecked(true);
    ui.checkBoxShowSecond->setChecked(false);
    ui.checkBoxShowBoth->setChecked(false);

    ui.doubleSpinBoxHeatersStackTemp2->setMinimum(ui.doubleSpinBoxHeatersStackTemp2->minimum());
    ui.doubleSpinBoxHeatersStackTemp2->setSpecialValueText(tr("---"));
    ui.doubleSpinBoxHeatersCurrentTemp2->setMinimum(ui.doubleSpinBoxHeatersCurrentTemp2->minimum());
    ui.doubleSpinBoxHeatersCurrentTemp2->setSpecialValueText(tr("---"));

    ui.doubleSpinBoxHeatersStackTemp1->setMinimum(ui.doubleSpinBoxHeatersStackTemp1->minimum());
    ui.doubleSpinBoxHeatersStackTemp1->setSpecialValueText(tr("---"));
    ui.doubleSpinBoxHeatersCurrentTemp1->setMinimum(ui.doubleSpinBoxHeatersCurrentTemp1->minimum());
    ui.doubleSpinBoxHeatersCurrentTemp1->setSpecialValueText(tr("---"));

    ui.doubleSpinBoxLiquidTankTemperature->setSpecialValueText(tr("---"));

    connect(ui.checkBoxShowFirst, &QCheckBox::stateChanged, this, &OverviewDialog::SlotCheckBoxShowFirstChanged);
    connect(ui.checkBoxShowSecond, &QCheckBox::stateChanged, this, &OverviewDialog::SlotCheckBoxShowSecondChanged);
    connect(ui.checkBoxShowBoth, &QCheckBox::stateChanged, this, &OverviewDialog::SlotCheckBoxShowBothChanged);

    bool SmallerAsThreshold = true;
    m_ControlLiquidTankFilling = new ControlsWithColorStatus(ui.doubleSpinBoxLiquidTankFillingLevel, SmallerAsThreshold);
    SmallerAsThreshold = false;

    m_ControlCounterProducNotFilled = new ControlsWithColorStatus(ui.doubleSpinBoxCounterProducNotFilled, SmallerAsThreshold);
    m_ControlDegreeOfPollution = new ControlsWithColorStatus(ui.doubleSpinBoxDegreeOfPollution, SmallerAsThreshold);
    m_ControlLiquidTankPreasure = new ControlsWithColorStatus(ui.doubleSpinBoxLiquidTankPreasure, SmallerAsThreshold);
    m_ControlLiquidTankTemperature = new ControlsWithColorStatus(ui.doubleSpinBoxLiquidTankTemperature, SmallerAsThreshold);
    m_ControlChamberHeatingPipeTemperatur = new ControlsWithColorStatus(ui.doubleSpinBoxHeatingPipeTemperatur, SmallerAsThreshold);

    m_ControlChamberTemperatureLeftValve = new ControlsWithColorStatus(ui.doubleSpinBoxHeatersCurrentTemp1, SmallerAsThreshold);
    m_ControlChamberTemperatureRightValve = new ControlsWithColorStatus(ui.doubleSpinBoxHeatersCurrentTemp2, SmallerAsThreshold);
    m_ControlPiezoTemperatureLeftValve = new ControlsWithColorStatus(ui.doubleSpinBoxHeatersStackTemp1, SmallerAsThreshold);
    m_ControlPiezoTemperatureRightValve = new ControlsWithColorStatus(ui.doubleSpinBoxHeatersStackTemp2, SmallerAsThreshold);

    SetAuditTrailProperties();
}

void OverviewDialog::SetAuditTrailProperties()
{
    ui.checkBoxShowFirst->setProperty(kAuditTrail, ui.labelShowFirst->text());
    ui.checkBoxShowSecond->setProperty(kAuditTrail, ui.labelShowSecond->text());
    ui.checkBoxShowBoth->setProperty(kAuditTrail, ui.labelShowBoth->text());
}

void OverviewDialog::ShowCheckBoxFirstTrigger(bool show)
{
    if (show) {
        ui.checkBoxShowFirst->show();
        ui.labelShowFirst->show();
    } else {
        ui.checkBoxShowFirst->hide();
        ui.labelShowFirst->hide();
    }
}

void OverviewDialog::ShowCheckBoxSecondTrigger(bool show)
{
    if (show) {
        ui.checkBoxShowSecond->show();
        ui.labelShowSecond->show();
    } else {
        ui.checkBoxShowSecond->hide();
        ui.labelShowSecond->hide();
    }
}

void OverviewDialog::CheckCheckBoxFirstTrigger(bool check)
{
    ui.checkBoxShowFirst->setChecked(check);
}

void OverviewDialog::CheckCheckBoxSecondTrigger(bool check)
{
    ui.checkBoxShowSecond->setChecked(check);
}

void OverviewDialog::AddMultiImageWidget(QWidget* w)
{
    if (ui.OverviewImageFrame->layout()) {
        ui.OverviewImageFrame->layout()->addWidget(w);
        int ShowOnOverview = static_cast<VideoDialog*>(w)->GetShowImagesOnOverview();
        switch (ShowOnOverview) {
            case SHOW_ON_OVERVIEW_LEFT_TRIGGER:
                ui.checkBoxShowFirst->setChecked(true);
                ui.checkBoxShowSecond->setChecked(false);
                ui.checkBoxShowBoth->setChecked(false);
                break;
            case SHOW_ON_OVERVIEW_RIGHT_TRIGGER:
                ui.checkBoxShowFirst->setChecked(false);
                ui.checkBoxShowSecond->setChecked(true);
                ui.checkBoxShowBoth->setChecked(false);
                break;
            case SHOW_ON_OVERVIEW_BOTH_TRIGGER:
                ui.checkBoxShowFirst->setChecked(false);
                ui.checkBoxShowSecond->setChecked(false);
                ui.checkBoxShowBoth->setChecked(true);
                break;
            default:
                ui.checkBoxShowFirst->setChecked(true);
                ui.checkBoxShowSecond->setChecked(false);
                ui.checkBoxShowBoth->setChecked(false);
                break;
        }
    }
}

void OverviewDialog::SlotCheckBoxShowFirstChanged(int state)
{
    VideoDialog* pVideoDialog = GetMainAppCrystalT2()->GetVideoDialogShowProductVideos();
    if (pVideoDialog) {
        disconnect(ui.checkBoxShowBoth, &QCheckBox::stateChanged, this, &OverviewDialog::SlotCheckBoxShowBothChanged);
        ui.checkBoxShowBoth->setChecked(false);
        connect(ui.checkBoxShowBoth, &QCheckBox::stateChanged, this, &OverviewDialog::SlotCheckBoxShowBothChanged);
        disconnect(ui.checkBoxShowSecond, &QCheckBox::stateChanged, this, &OverviewDialog::SlotCheckBoxShowSecondChanged);
        if (state == Qt::Checked) {
            ui.checkBoxShowSecond->setChecked(false);
            pVideoDialog->SetShowImagesOnOverview(SHOW_ON_OVERVIEW_LEFT_TRIGGER);
        } else {
            ui.checkBoxShowSecond->setChecked(true);
            pVideoDialog->SetShowImagesOnOverview(SHOW_ON_OVERVIEW_RIGHT_TRIGGER);
        }
        connect(ui.checkBoxShowSecond, &QCheckBox::stateChanged, this, &OverviewDialog::SlotCheckBoxShowSecondChanged);
    }
}

void OverviewDialog::SlotCheckBoxShowSecondChanged(int state)
{
    VideoDialog* pVideoDialog = GetMainAppCrystalT2()->GetVideoDialogShowProductVideos();
    if (pVideoDialog) {
        disconnect(ui.checkBoxShowBoth, &QCheckBox::stateChanged, this, &OverviewDialog::SlotCheckBoxShowBothChanged);
        ui.checkBoxShowBoth->setChecked(false);
        connect(ui.checkBoxShowBoth, &QCheckBox::stateChanged, this, &OverviewDialog::SlotCheckBoxShowBothChanged);
        disconnect(ui.checkBoxShowFirst, &QCheckBox::stateChanged, this, &OverviewDialog::SlotCheckBoxShowFirstChanged);
        if (state == Qt::Checked) {
            ui.checkBoxShowFirst->setChecked(false);
            pVideoDialog->SetShowImagesOnOverview(SHOW_ON_OVERVIEW_RIGHT_TRIGGER);
        } else {
            ui.checkBoxShowFirst->setChecked(true);
            pVideoDialog->SetShowImagesOnOverview(SHOW_ON_OVERVIEW_LEFT_TRIGGER);
        }
        connect(ui.checkBoxShowFirst, &QCheckBox::stateChanged, this, &OverviewDialog::SlotCheckBoxShowFirstChanged);
    }
}

void OverviewDialog::SlotCheckBoxShowBothChanged(int state)
{
    VideoDialog* pVideoDialog = GetMainAppCrystalT2()->GetVideoDialogShowProductVideos();
    if (pVideoDialog) {
        disconnect(ui.checkBoxShowFirst, &QCheckBox::stateChanged, this, &OverviewDialog::SlotCheckBoxShowFirstChanged);
        disconnect(ui.checkBoxShowSecond, &QCheckBox::stateChanged, this, &OverviewDialog::SlotCheckBoxShowSecondChanged);
        if (state == Qt::Checked) {
            ui.checkBoxShowFirst->setChecked(false);
            ui.checkBoxShowSecond->setChecked(false);
            pVideoDialog->SetShowImagesOnOverview(SHOW_ON_OVERVIEW_BOTH_TRIGGER);
        } else {
            if (ui.checkBoxShowFirst->isVisible()) {
                ui.checkBoxShowFirst->setChecked(true);
                ui.checkBoxShowSecond->setChecked(false);
            } else {
                ui.checkBoxShowFirst->setChecked(false);
                ui.checkBoxShowSecond->setChecked(true);
            }
            pVideoDialog->SetShowImagesOnOverview(SHOW_ON_OVERVIEW_LEFT_TRIGGER);
        }
        connect(ui.checkBoxShowFirst, &QCheckBox::stateChanged, this, &OverviewDialog::SlotCheckBoxShowFirstChanged);
        connect(ui.checkBoxShowSecond, &QCheckBox::stateChanged, this, &OverviewDialog::SlotCheckBoxShowSecondChanged);
    }
}

void OverviewDialog::SetBottlesPerMiniute(double BottlesPerMin)
{
    ui.doubleSpinBoxBottlesPerMinute->setValue(BottlesPerMin);
}

void OverviewDialog::SetValveChamberTemp(double set, int ValveID)
{
    if (GetMainAppCrystalT2()->GetSettingsData()) {
        bool MaschineStopInSec = true;
        double MaschineStopThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_ValveChamberTemperatureMaschineStopTimeInSec;
        double WarnThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_ValveChamberTemperatureWarningLevelInDegree;
        double AlarmThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_ValveChamberTemperatureAlarmLevelInDegree;
        double DefaultValue = 0.0;
        if (ValveID == LEFT_VALVE_ID) {
            if (m_ControlChamberTemperatureLeftValve && GetMainAppCrystalT2()->GetValveDialogLeft() && GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusChamperTempLeftValve) {
                if (GetMainAppCrystalT2()->GetValveDialogLeft()) {
                    DefaultValue = GetMainAppCrystalT2()->GetValveDialogLeft()->GetValveHeaterTargetTempterature();
                }
                m_StatusChamperTempLeftValve =
                    m_ControlChamberTemperatureLeftValve->SetValueAndAlarmStatus(set, set, DefaultValue, WarnThreshold, AlarmThreshold, MaschineStopThreshold, MaschineStopInSec);
            } else {
                m_StatusChamperTempLeftValve = ALARM_LEVEL_OK;
                ui.doubleSpinBoxHeatersCurrentTemp1->setValue(set);
                if (m_ControlChamberTemperatureLeftValve) {
                    m_ControlChamberTemperatureLeftValve->StopTimerColorStatus();
                    m_ControlChamberTemperatureLeftValve->SetBackgroundColor(HMIColor::ContentBoard);
                }
            }
        } else {
            if (m_ControlChamberTemperatureRightValve && GetMainAppCrystalT2()->GetValveDialogRight() && GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusChamperTempRightValve) {
                if (GetMainAppCrystalT2()->GetValveDialogRight()) {
                    DefaultValue = GetMainAppCrystalT2()->GetValveDialogRight()->GetValveHeaterTargetTempterature();
                }
                m_StatusChamperTempRightValve =
                    m_ControlChamberTemperatureRightValve->SetValueAndAlarmStatus(set, set, DefaultValue, WarnThreshold, AlarmThreshold, MaschineStopThreshold, MaschineStopInSec);
            } else {
                m_StatusChamperTempRightValve = ALARM_LEVEL_OK;
                ui.doubleSpinBoxHeatersCurrentTemp2->setValue(set);
                if (m_ControlChamberTemperatureRightValve) {
                    m_ControlChamberTemperatureRightValve->StopTimerColorStatus();
                    m_ControlChamberTemperatureRightValve->SetBackgroundColor(HMIColor::ContentBoard);
                }
            }
        }
    }
}

void OverviewDialog::SetValvePiezoTemp(double set, int ValveID)
{
    if (GetMainAppCrystalT2()->GetSettingsData()) {
        bool MaschineStopInSec = true;
        double MaschineStopThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_ValvePiezoTemperatureMaschineStopTimeInSec;
        double WarnThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_ValvePiezoTemperatureWarningLevelInDegree;
        double AlarmThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_ValvePiezoTemperatureAlarmLevelInDegree;
        double Default = 0.0;
        if (ValveID == LEFT_VALVE_ID) {
            if (m_ControlPiezoTemperatureLeftValve && GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusPiezoTempLeftValve) {
                m_StatusPiezoTempLeftValve = m_ControlPiezoTemperatureLeftValve->SetValueAndAlarmStatus(set, set, Default, WarnThreshold, AlarmThreshold, MaschineStopThreshold, MaschineStopInSec);
            } else {
                m_StatusPiezoTempLeftValve = ALARM_LEVEL_OK;
                ui.doubleSpinBoxHeatersStackTemp1->setValue(set);
                if (m_ControlPiezoTemperatureLeftValve) {
                    m_ControlPiezoTemperatureLeftValve->StopTimerColorStatus();
                    m_ControlPiezoTemperatureLeftValve->SetBackgroundColor(HMIColor::ContentBoard);
                }
            }
        } else {
            if (m_ControlPiezoTemperatureRightValve && GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusPiezoTempRightValve) {
                m_StatusPiezoTempRightValve = m_ControlPiezoTemperatureRightValve->SetValueAndAlarmStatus(set, set, Default, WarnThreshold, AlarmThreshold, MaschineStopThreshold, MaschineStopInSec);
            } else {
                m_StatusPiezoTempRightValve = ALARM_LEVEL_OK;
                ui.doubleSpinBoxHeatersStackTemp2->setValue(set);
                if (m_ControlPiezoTemperatureRightValve) {
                    m_ControlPiezoTemperatureRightValve->StopTimerColorStatus();
                    m_ControlPiezoTemperatureRightValve->SetBackgroundColor(HMIColor::ContentBoard);
                }
            }
        }
    }
}

void OverviewDialog::SetCounterNumberBottlesRealEjected(double value)
{
    ui.doubleSpinBoxRealEject->setValue(value);
}

void OverviewDialog::SetCounterProductNotFilled(int set)
{
    if (m_ControlCounterProducNotFilled && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData() && GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusCounterProducNotFilled) {
        bool MaschineStopInMin = true;
        double MaschineStopThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_CounterProductNotFilledMaschineStopTimeInSec;
        double WarnThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_CounterProductNotFilledWarningLevel;
        double AlarmThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_CounterProductNotFilledAlarmLevel;
        double Default = ui.doubleSpinBoxRealEject->value();
        // qDebug() << QString("W:%1 A:%2 D:%3").arg(WarnThreshold).arg(AlarmThreshold).arg(Default);
        m_StatusCounterProducNotFilled = m_ControlCounterProducNotFilled->SetValueAndAlarmStatus(set, set, Default, WarnThreshold, AlarmThreshold, MaschineStopThreshold, MaschineStopInMin);
    } else {
        m_StatusCounterProducNotFilled = ALARM_LEVEL_OK;
        ui.doubleSpinBoxCounterProducNotFilled->setValue(set);
        if (m_ControlCounterProducNotFilled) {
            m_ControlCounterProducNotFilled->StopTimerColorStatus();
            m_ControlCounterProducNotFilled->SetBackgroundColor(HMIColor::ContentBoard);
        }
    }
}

void OverviewDialog::SetCounterProductOk(int set)
{
    ui.doubleSpinBoxCounterProductOk->setValue(set);
}

void OverviewDialog::SetDegreeOfPollution(double set)
{
    if (m_ControlDegreeOfPollution && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData() && GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusDegreeOfPolution) {
        bool MaschineStopInMin = true;
        double MaschineStopThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_DegreePollutionMaschineStopTimeInSec;
        double WarnThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_DegreePollutionWarningInPercent;
        double AlarmThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_DegreePollutionAlarmInPercent;
        double Default = 0.0;
        m_StatusDegreeOfPollution = m_ControlDegreeOfPollution->SetValueAndAlarmStatus(set, set, Default, WarnThreshold, AlarmThreshold, MaschineStopThreshold, MaschineStopInMin);
    } else {
        m_StatusDegreeOfPollution = ALARM_LEVEL_OK;
        ui.doubleSpinBoxDegreeOfPollution->setValue(set);
        if (m_ControlDegreeOfPollution) {
            m_ControlDegreeOfPollution->StopTimerColorStatus();
            m_ControlDegreeOfPollution->SetBackgroundColor(HMIColor::ContentBoard);
        }
    }
}

void OverviewDialog::SetLiquidTankPreasure(double set)
{
    if (m_ControlLiquidTankPreasure && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData() && GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusCurrentLiquidTankPreasure &&
        GetMainAppCrystalT2()->GetCurrentProductData()) {
        bool MaschineStopInMin = true;
        double DefaultPreasureInBar = GetMainAppCrystalT2()->GetCurrentProductData()->m_DefaultPreasureInBar;
        if (DefaultPreasureInBar != 0.0) {
            double ValueInPercent = (100.0 / DefaultPreasureInBar) * set;
            double MaschineStopThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_TankPreasureLevelMaschineStopTimeInSec;
            double WarnThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_TankPreasureLevelWarningInPercent;
            double AlarmThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_TankPreasureLevelAlarmInPercent;
            double Default = 100.0;
            int StatusLiquidTankPreasure = m_ControlLiquidTankPreasure->SetValueAndAlarmStatus(set, ValueInPercent, Default, WarnThreshold, AlarmThreshold, MaschineStopThreshold, MaschineStopInMin);
            if (StatusLiquidTankPreasure != ALARM_LEVEL_OK) {
                if (GetMainAppCrystalT2()->GetCurrentMaschineState() == PluginInterface::MachineState::Off) {

                    m_NumberErrorLiquidTankPressure = 0;
                    m_StatusLiquidTankPreasure = ALARM_LEVEL_OK;
                    m_ControlLiquidTankPreasure->SetColorStatus(ALARM_LEVEL_OK);
                } else {
                    m_NumberErrorLiquidTankPressure++;
                    if (m_NumberErrorLiquidTankPressure < GetMainAppCrystalT2()->GetSettingsData()->m_MaximumErrorCount) {
                        m_StatusLiquidTankPreasure = ALARM_LEVEL_OK;
                        m_ControlLiquidTankPreasure->SetColorStatus(ALARM_LEVEL_OK);
                    } else {
                        m_StatusLiquidTankPreasure = StatusLiquidTankPreasure;  // jetzt erst Fehlermeldung
                    }
                }
            } else {
                m_NumberErrorLiquidTankPressure = 0;
                m_StatusLiquidTankPreasure = StatusLiquidTankPreasure;
            }
        } else {
            m_StatusLiquidTankPreasure = ALARM_LEVEL_OK;
            m_ControlLiquidTankPreasure->SetShowValue(DefaultPreasureInBar);
            m_ControlLiquidTankPreasure->StopTimerColorStatus();
            m_ControlLiquidTankPreasure->SetBackgroundColor(HMIColor::ContentBoard);
        }
    } else {
        m_StatusLiquidTankPreasure = ALARM_LEVEL_OK;
        ui.doubleSpinBoxLiquidTankPreasure->setValue(set);
        if (m_ControlLiquidTankPreasure) {
            m_ControlLiquidTankPreasure->StopTimerColorStatus();
            m_ControlLiquidTankPreasure->SetBackgroundColor(HMIColor::ContentBoard);
        }
    }
}

void OverviewDialog::SetLiquidTankFillingLevel(double set)
{
    if (m_ControlLiquidTankFilling && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData() && GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusCurrentLiquidTankFilling) {
        bool MaschineStopInMin = false;
        double MaschineStopThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_TankFillingLevelMaschineStopInLiter;
        double WarnThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_TankFillingLevelWarningInLiter;
        double AlarmThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_TankFillingLevelAlarmInLiter;
        double MaxValue = GetMainAppCrystalT2()->GetSettingsData()->m_TankFillingLevelMaxValue;
        double Default = 0.0;
        if (set <= MaxValue) {
            m_StatusLiquidTankFilling = m_ControlLiquidTankFilling->SetValueAndAlarmStatus(set, set, Default, WarnThreshold, AlarmThreshold, MaschineStopThreshold, MaschineStopInMin);
        } else {
            m_StatusLiquidTankFilling = ALARM_LEVEL_ALARM | ALARM_LEVEL_ALARM_MAX;
            m_ControlLiquidTankFilling->SetColorStatus(m_StatusLiquidTankFilling, true);
            ui.doubleSpinBoxLiquidTankFillingLevel->setValue(set);
        }
    } else {
        m_StatusLiquidTankFilling = ALARM_LEVEL_OK;
        ui.doubleSpinBoxLiquidTankFillingLevel->setValue(set);
        if (m_ControlLiquidTankFilling) {
            m_ControlLiquidTankFilling->StopTimerColorStatus();
            m_ControlLiquidTankFilling->SetBackgroundColor(HMIColor::ContentBoard);
        }
    }
}

void OverviewDialog::SetLiquidTankTemp(double valueInDegree)
{
    if (m_ControlLiquidTankTemperature && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData() && GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusCurrentLiquidTankTemp &&
        GetMainAppCrystalT2()->GetCurrentProductData()) {
        bool MaschineStopInSec = true;
        double DefaultTankTemperature = GetMainAppCrystalT2()->GetCurrentProductData()->m_DefaultPreasureTankTemp;
        if (DefaultTankTemperature != 0.0) {
            double MaschineStopThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_TankTemperatureLevelMaschineStopTimeInSec;
            double WarnThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_TankTemperatureLevelWarningInDegree;
            double AlarmThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_TankTemperatureLevelAlarmInDegree;
            m_StatusLiquidTankTemperature =
                m_ControlLiquidTankTemperature->SetValueAndAlarmStatus(valueInDegree, valueInDegree, DefaultTankTemperature, WarnThreshold, AlarmThreshold, MaschineStopThreshold, MaschineStopInSec);
        } else {
            m_StatusLiquidTankTemperature = ALARM_LEVEL_OK;
            m_ControlLiquidTankTemperature->StopTimerColorStatus();
            m_ControlLiquidTankTemperature->SetBackgroundColor(HMIColor::ContentBoard);
        }

    } else {
        m_StatusLiquidTankTemperature = ALARM_LEVEL_OK;
        ui.doubleSpinBoxLiquidTankTemperature->setValue(valueInDegree);
        if (m_ControlLiquidTankTemperature) {
            m_ControlLiquidTankTemperature->StopTimerColorStatus();
            m_ControlLiquidTankTemperature->SetBackgroundColor(HMIColor::ContentBoard);
        }
    }
}

void OverviewDialog::SetHeatingPipeTemperature(double valueInDegree)
{
    if (m_ControlChamberHeatingPipeTemperatur && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData() && GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusHeatingPipeTemp &&
        GetMainAppCrystalT2()->GetCurrentProductData()) {
        bool MaschineStopInSec = true;
        double DefaultHeatingPipeTemperature = GetMainAppCrystalT2()->GetCurrentProductData()->m_DefaulHeatingPipeTemp;
        if (DefaultHeatingPipeTemperature != 0.0) {
            double MaschineStopThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_HeatingPipeLevelMaschineStopTimeInSec;
            double WarnThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_HeatingPipeTemperatureLevelWarningInPercent;
            double AlarmThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_HeatingPipeTemperatureLevelAlarmInPercent;
            m_StatusHeatingPipeTemperature = m_ControlChamberHeatingPipeTemperatur->SetValueAndAlarmStatus(valueInDegree, valueInDegree, DefaultHeatingPipeTemperature, WarnThreshold, AlarmThreshold,
                                                                                                           MaschineStopThreshold, MaschineStopInSec);
        } else {
            m_StatusHeatingPipeTemperature = ALARM_LEVEL_OK;
            m_ControlChamberHeatingPipeTemperatur->SetBackgroundColor(HMIColor::ContentBoard);
            m_ControlChamberHeatingPipeTemperatur->StopTimerColorStatus();
            m_ControlChamberHeatingPipeTemperatur->SetBackgroundColor(HMIColor::ContentBoard);
        }
    } else {
        m_StatusHeatingPipeTemperature = ALARM_LEVEL_OK;
        ui.doubleSpinBoxHeatingPipeTemperatur->setValue(valueInDegree);
        if (m_ControlChamberHeatingPipeTemperatur) {
            m_ControlChamberHeatingPipeTemperatur->StopTimerColorStatus();
            m_ControlChamberHeatingPipeTemperatur->SetBackgroundColor(HMIColor::ContentBoard);
        }
    }
}

double OverviewDialog::GetCurrentLiquidTankPreasure()
{
    return ui.doubleSpinBoxLiquidTankPreasure->value();
}

double OverviewDialog::GetCurrentPreasureTankTemperature()
{
    return ui.doubleSpinBoxLiquidTankTemperature->value();
}

double OverviewDialog::GetCurrentHeatingPipeTemperature()
{
    return ui.doubleSpinBoxHeatingPipeTemperatur->value();
}

double OverviewDialog::GetCurrentPreasureTankFillingLevel()
{
    return ui.doubleSpinBoxLiquidTankFillingLevel->value();
}
