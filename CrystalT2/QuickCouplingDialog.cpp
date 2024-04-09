#include "QuickCouplingDialog.h"
#include "MainAppCrystalT2.h"
#include "bswitch.h"

#include <audittrail.h>

QuickCouplingDialog::QuickCouplingDialog(MainAppCrystalT2* pMainAppCrystalT2) : QWidget(pMainAppCrystalT2), m_SwitchValvePowerAndPreasure(NULL), m_TimerSetErrorMsgNoValvePowerAndPreasure(NULL)
{
    ui.setupUi(this);
    m_MainAppCrystalT2 = pMainAppCrystalT2;
    m_SwitchValvePowerAndPreasure = new BSwitch(ui.widgetSetCurrentAndPreasureOnOff);
    connect(m_SwitchValvePowerAndPreasure, &BSwitch::clicked, this, &QuickCouplingDialog::SlotSwitchPowerAndPreasure);

    m_TimerSetErrorMsgNoValvePowerAndPreasure = new QTimer(this);
    connect(m_TimerSetErrorMsgNoValvePowerAndPreasure, &QTimer::timeout, this, &QuickCouplingDialog::SlotSetErrorMsgNoValvePowerAndPreasure);
    m_TimerSetErrorMsgNoValvePowerAndPreasure->setInterval(5000);
    SetAuditTrailProperties();
}

QuickCouplingDialog::~QuickCouplingDialog()
{
}

void QuickCouplingDialog::SetAuditTrailProperties()
{
    if (m_SwitchValvePowerAndPreasure) {
        m_SwitchValvePowerAndPreasure->setProperty(kAuditTrail, ui.groupBoxSetValvePowerAndPreasure->title());
    }
}

void QuickCouplingDialog::SlotSwitchPowerAndPreasure(bool on)
{
    if (GetMainAppCrystalT2()) {
        GetMainAppCrystalT2()->SetPowerAndPreasureOnTheValves(on);

        if (on) {
            ui.labelSetCurrentAndPreasureOnOff->setText(tr("On"));
            m_TimerSetErrorMsgNoValvePowerAndPreasure->stop();
        } else {
            ui.labelSetCurrentAndPreasureOnOff->setText(tr("Off"));
            m_TimerSetErrorMsgNoValvePowerAndPreasure->start();
        }
    }
}

void QuickCouplingDialog::SlotSetErrorMsgNoValvePowerAndPreasure()
{
    if (GetMainAppCrystalT2()) {
        if (!GetMainAppCrystalT2()->IsPowerAndPreasureOnValve()) {
            GetMainAppCrystalT2()->SlotAddNewMessage(tr("Valve Power And Preasure Is Off"), QtMsgType::QtFatalMsg);
        } else {
            m_TimerSetErrorMsgNoValvePowerAndPreasure->stop();
        }
    }
}

bool QuickCouplingDialog::event(QEvent* pEvent)
{
    if (pEvent->type() == QEvent::Show) {
        if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
            disconnect(m_SwitchValvePowerAndPreasure, &BSwitch::clicked, this, &QuickCouplingDialog::SlotSwitchPowerAndPreasure);
            if (GetMainAppCrystalT2()->IsPowerAndPreasureOnValve()) {
                m_SwitchValvePowerAndPreasure->setChecked(true);
                ui.labelSetCurrentAndPreasureOnOff->setText(tr("On"));
            } else {
                m_SwitchValvePowerAndPreasure->setChecked(false);
                ui.labelSetCurrentAndPreasureOnOff->setText(tr("Off"));
            }
            connect(m_SwitchValvePowerAndPreasure, &BSwitch::clicked, this, &QuickCouplingDialog::SlotSwitchPowerAndPreasure);
        }
    }
    return QWidget::event(pEvent);
}
