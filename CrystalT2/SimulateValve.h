#pragma once
#include "QtCore"
#include "qlocalsocket.h"

class MainAppCrystalT2;
class SimulateValve : public QObject
{
    Q_OBJECT
  public:
    SimulateValve(MainAppCrystalT2* pMainAppCrystalT2);
    ~SimulateValve();
    void LeftTriggerOn();
    void RightTriggerOn();
    void SetValveRightName();
    void SetValveLeftName();
    void KillProcess();
    void StartProcess();
    bool WaitForStarted();
    void Disconnect();
    void StartResetTriggerLeft();
    void StartResetTriggerRight();
    MainAppCrystalT2* GetMainAppCrystalT2(){return m_MainAppCrystalT2;}

  public slots:
    void SlotConnectedValveLeft();
    void SlotConnectedValveRight();
    void SlotProcessValveLeftIsRunning();
    void SlotProcessValveRightIsRunning();
    void SlotTryConnectToServerLeft();
    void SlotTryConnectToServerRight();
    void SlotLeftTriggerOff();
    void SlotRightTriggerOff();

  private:
    MainAppCrystalT2* m_MainAppCrystalT2;
    QLocalSocket* m_ClientValveLeft;
    QLocalSocket* m_ClientValveRight;
    QProcess *m_ValveLeftProcess, *m_ValveRightProcess;
    QTimer* m_TimerConnectToServerLeft;
    QTimer* m_TimerConnectToServerRight;
    bool m_ValveLeftConnected;
    bool m_ValveRightConnected;
    int m_counterLeft, m_counterRight;
    QString m_ValveLeftName, m_ValveRightName;
};
