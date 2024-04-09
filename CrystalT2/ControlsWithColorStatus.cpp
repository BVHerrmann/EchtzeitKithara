#include "ControlsWithColorStatus.h"
#include "GlobalConst.h"
#include "colors.h"
#include "qspinbox.h"
#include "qstyle.h"

ControlsWithColorStatus::ControlsWithColorStatus(QDoubleSpinBox* pSpinBox, bool SmallerAsThreshold) : QObject(), m_SpinBox(NULL), m_ToggleColor(false), m_ColorStatus(ALARM_LEVEL_OK)
{
    int ColorToggleIntervall = 1000;
    m_SmallerAsThreshold = SmallerAsThreshold;
    m_SpinBox = pSpinBox;
    m_TimerToggleColor = new QTimer(this);
    connect(m_TimerToggleColor, &QTimer::timeout, this, &ControlsWithColorStatus::SlotSetColorStatus);
    m_TimerToggleColor->setInterval(ColorToggleIntervall);
}

ControlsWithColorStatus::~ControlsWithColorStatus()
{
}

void ControlsWithColorStatus::SlotSetColorStatus()
{
    if (m_ToggleColor) {
        SetStatusBackgroundColor();
    } else {
        SetBackgroundColor(HMIColor::ContentBoard);
    }
    m_ToggleColor = !m_ToggleColor;
}

void ControlsWithColorStatus::SetShowValue(double ShowValue)
{
    if (m_SpinBox) {
        m_SpinBox->setValue(ShowValue);
    }
}

int ControlsWithColorStatus::SetValueAndAlarmStatus(double ShowValue, double CurrentValue, double DefaultValue, double WarnThreshold, double AlarmThreshold, double MaschineStopThreshold,
                                                    bool MaschineStopByTime)
{
    SetShowValue(ShowValue);
    double MinValueMaschineStopThreshold = DefaultValue - MaschineStopThreshold;
    double MaxValueMaschineStopThreshold = DefaultValue + MaschineStopThreshold;

    double MinValueAlarmThreshold = DefaultValue - AlarmThreshold;
    double MaxValueAlarmThreshold = DefaultValue + AlarmThreshold;

    double MinValueWarnThreshold = DefaultValue - WarnThreshold;
    double MaxValueWarnThreshold = DefaultValue + WarnThreshold;

    bool WarnLevelOnMaxThresHold = (CurrentValue > MaxValueWarnThreshold);
    bool WarnLevelOnMinThresHold = (CurrentValue < MinValueWarnThreshold);

    bool AlarmLevelOnMaxThresHold = (CurrentValue > MaxValueAlarmThreshold);
    bool AlarmLevelOnMinThresHold = (CurrentValue < MinValueAlarmThreshold);

    bool MaschineStopLevelOn = (CurrentValue > MaxValueMaschineStopThreshold) || (CurrentValue < MinValueMaschineStopThreshold);
    bool AlarmLevelOn = AlarmLevelOnMaxThresHold || AlarmLevelOnMinThresHold;
    bool WarnLevelOn = WarnLevelOnMaxThresHold || WarnLevelOnMinThresHold;

    if (m_SmallerAsThreshold) {
        MaschineStopLevelOn = !MaschineStopLevelOn;
        AlarmLevelOn = !AlarmLevelOn;
        WarnLevelOn = !WarnLevelOn;
    }
    int status = ALARM_LEVEL_OK;
    if (!MaschineStopByTime && MaschineStopLevelOn) {
        status = ALARM_LEVEL_MASCHINE_STOP;
        if (AlarmLevelOnMaxThresHold) {
            status = status | ALARM_LEVEL_ALARM_MAX;  // wert ist überschritten
        }
        if (AlarmLevelOnMinThresHold) {
            status = status | ALARM_LEVEL_ALARM_MIN;  // wert ist unterschritten
        }
        SetColorStatus(status, false);  // not blinking
    } else {
        if (AlarmLevelOn) {
            if (MaschineStopByTime) {
                if (!m_TimerGetAlarmDuration.isValid()) {
                    m_TimerGetAlarmDuration.start();
                }
                double DurationTimeInSec = (m_TimerGetAlarmDuration.elapsed() / 1000.0);
                if (DurationTimeInSec > MaschineStopThreshold) {
                    status = ALARM_LEVEL_MASCHINE_STOP;
                    if (AlarmLevelOnMaxThresHold) {
                        status = status | ALARM_LEVEL_ALARM_MAX;  // wert ist überschritten
                    }
                    if (AlarmLevelOnMinThresHold) {
                        status = status | ALARM_LEVEL_ALARM_MIN;  // wert ist unterschritten
                    }
                    SetColorStatus(status, false);  // not blinking
                } else {
                    status = ALARM_LEVEL_ALARM;
                    if (AlarmLevelOnMaxThresHold) {
                        status = status | ALARM_LEVEL_ALARM_MAX;  // wert ist überschritten
                    }
                    if (AlarmLevelOnMinThresHold) {
                        status = status | ALARM_LEVEL_ALARM_MIN;  // wert ist unterschritten
                    }
                    SetColorStatus(status, true);  // blinking on
                }
            } else {
                status = ALARM_LEVEL_ALARM;
                if (AlarmLevelOnMaxThresHold) {
                    status = status | ALARM_LEVEL_ALARM_MAX;
                }
                if (AlarmLevelOnMinThresHold) {
                    status = status | ALARM_LEVEL_ALARM_MIN;
                }
                SetColorStatus(status, true);  // blinking on
            }
        } else {
            m_TimerGetAlarmDuration.invalidate();
            if (WarnLevelOn) {
                status = ALARM_LEVEL_WARNING;
                if (WarnLevelOnMaxThresHold) {
                    status = status | ALARM_LEVEL_WARNING_MAX;  // wert ist überschritten
                }
                if (WarnLevelOnMinThresHold) {
                    status = status | ALARM_LEVEL_WARNING_MIN;  // wert ist unterschritten
                }
                SetColorStatus(status, false);  // not blinking
            } else {
                SetColorStatus(ALARM_LEVEL_OK, false);  // not blinking
            }
        }
    }
    return m_ColorStatus;
}

void ControlsWithColorStatus::SetColorStatus(int status, bool blinking)
{
    m_ColorStatus = status;
    if (blinking) {
        StartTimerColorStatus();
    } else {
        m_TimerToggleColor->stop();
        SetStatusBackgroundColor();
    }
}

void ControlsWithColorStatus::SetStatusBackgroundColor()
{
    if (m_ColorStatus & ALARM_LEVEL_WARNING) {
        SetBackgroundColor(HMIColor::WarningLow);
    } else {
        if (m_ColorStatus & ALARM_LEVEL_ALARM) {
            SetBackgroundColor(HMIColor::WarningHigh);
        } else {
            if (m_ColorStatus & ALARM_LEVEL_MASCHINE_STOP) {
                SetBackgroundColor(HMIColor::Alarm);
            } else {
                SetBackgroundColor(HMIColor::ContentBoard);
            }
        }
    }
}

void ControlsWithColorStatus::StartTimerColorStatus()
{
    if (!m_TimerToggleColor->isActive()) {
        m_TimerToggleColor->start();
    }
}

void ControlsWithColorStatus::StopTimerColorStatus()
{
    if (m_TimerToggleColor->isActive()) {
        m_TimerToggleColor->stop();
    }
}

void ControlsWithColorStatus::SetBackgroundColor(const QColor& color)
{
    if (m_SpinBox && m_LastBackGroundColor != color) {
        m_SpinBox->setStyleSheet(QString("background-color:rgb(%1, %2, %3); border: 0px").arg(color.red()).arg(color.green()).arg(color.blue()));
        m_LastBackGroundColor = color;
    }
}
