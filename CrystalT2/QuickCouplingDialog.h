#pragma once

#include <QWidget>
#include "ui_QuickCouplingDialog.h"

class BSwitch;
class MainAppCrystalT2;
class QuickCouplingDialog : public QWidget
{
    Q_OBJECT

  public:
    QuickCouplingDialog(MainAppCrystalT2* pMainAppCrystalT2);
    ~QuickCouplingDialog();
    MainAppCrystalT2* GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
    bool event(QEvent* pEvent);
    void SetAuditTrailProperties();

  public slots:
    void SlotSwitchPowerAndPreasure(bool on);
    void SlotSetErrorMsgNoValvePowerAndPreasure();

  private:
    Ui::QuickCouplingDialogClass ui;
    MainAppCrystalT2* m_MainAppCrystalT2;
    BSwitch* m_SwitchValvePowerAndPreasure;
    QTimer* m_TimerSetErrorMsgNoValvePowerAndPreasure;
};
