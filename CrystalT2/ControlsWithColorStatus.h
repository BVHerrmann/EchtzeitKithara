#pragma once
#include <qobject.h>
#include "qcolor.h"
#include "qelapsedtimer.h"

class QDoubleSpinBox;
class ControlsWithColorStatus : public QObject
{
    Q_OBJECT
  public:
    ControlsWithColorStatus(QDoubleSpinBox* pSpinBox, bool SmallerAsThreshold);
    ~ControlsWithColorStatus();
    int SetValueAndAlarmStatus(double showValue, double currentValue, double defaultValue, double WarnThreshold, double AlarmThreshold, double MaschineStopThreshold, bool MaschineStopByTime);
    void SetBackgroundColor(const QColor& color);
    void SetColorStatus(int status, bool blinking = false);
    void StartTimerColorStatus();
    void SetStatusBackgroundColor();
    void SetShowValue(double ShowValue);
    void StopTimerColorStatus();

  public slots:
    void SlotSetColorStatus();

  private:
    QDoubleSpinBox* m_SpinBox;
    QString m_LastColorState;
    int m_ColorStatus;
    QTimer* m_TimerToggleColor;
    bool m_SmallerAsThreshold;
    bool m_ToggleColor;
    QElapsedTimer m_TimerGetAlarmDuration;
    QColor m_LastBackGroundColor;
};
