#pragma once

#include <QWidget>
#include "GlobalConst.h"
#include "qthread"
#include "ui_MaintenanceDialog.h"

class MainAppCrystalT2;
class ThreadManualTrigger;
class MaintenanceDialog : public QWidget
{
    Q_OBJECT

  public:
    MaintenanceDialog(MainAppCrystalT2* pMainAppCrystalT2 = nullptr);
    ~MaintenanceDialog();
    MainAppCrystalT2* GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
    bool event(QEvent* pEvent);
    bool eventFilter(QObject*, QEvent* pEvent);
    void SetAuditTrailProperties();
    void StartManualTriggerValve(int triggerCount, int usedTriggerOutput);
    void IncrementTriggerCounterReadingLeftValve();
    void IncrementTriggerCounterReadingRightValve();
    bool IsManualTriggerActive() { return m_ManualTriggerIsActive; }
    void AddLiveImageWidget(QWidget* w);
    void RemoveLiveImageWidget(QWidget* w);
    bool StartPurgValveLeft();
    bool StartPurgValveRight();

  signals:
    void SignalManualTriggerReady();

  public slots:
    void SlotPressedPurgValveLeft();
    void SlotReleasedPurgValveLeft();
    void SlotPressedPurgValveRight();
    void SlotReleasedPurgValveRight();
    void SlotSetLastPurgeModusBackValveLeft();
    void SlotSetLastPurgeModusBackValveRight();
    void SlotEnableSuppressAlarmWarinigPreasureLiquidTank();
    void SlotManualTriggerReady();
    void SlotManualTriggerValveLeft();
    void SlotManualTriggerValveRight();
    void SlotManualTriggerBoth();
    void SlotCounterManualTriggerChanged();
    void SlotNumberLoopsChanged();
    void SlotTimeBetweenLoopsManualTriggerInMsChanged();
    void SlotResetCounter();
    void SlotPurgContinuousLeftValve(bool clicked);
    void SlotPurgContinuousRightValve(bool clicked);

  private:
    Ui::MaintenanceDialogClass ui;
    MainAppCrystalT2* m_MainAppCrystalT2;
    ThreadManualTrigger* m_ThreadManualTrigger;
    bool m_ManualTriggerIsActive;
    bool m_ResetCounter;
    bool m_ToggelLeftValveContinuous;
    bool m_ToggelRightValveContinuous;
    int m_WaitTimeEnableAlarmTemperatureTank;
    QString m_MainErrorTextPurge;
    QString m_ErrorTextPurgeLeftValue, m_ErrorTextPurgeRightValue, m_ErrorTextMaschineIsInOffState;
    QString m_StringCanNotTrigger;
    QString m_StringNotEnoughPressure;
};

class ThreadManualTrigger : public QThread
{
  public:
    ThreadManualTrigger(MaintenanceDialog* pMaintenanceDialog) : QThread()
    {
        m_MaintenanceDialog = pMaintenanceDialog;
        m_UsedTriggerOutputs = 0;  // default use both
        m_TriggerCount = 1;
    }
    void SetTriggerCount(int set) { m_TriggerCount = set; }
    void SetUsedTriggerOutput(int set) { m_UsedTriggerOutputs = set; }
    void run()
    {
        if (m_MaintenanceDialog) {
            m_MaintenanceDialog->StartManualTriggerValve(m_TriggerCount, m_UsedTriggerOutputs);
        }
    }

  private:
    MaintenanceDialog* m_MaintenanceDialog;
    int m_UsedTriggerOutputs;
    int m_TriggerCount;
};
