#include "MaintenanceDialog.h"
#include "GlobalConst.h"
#include "LiveImageView.h"
#include "MainAppCrystalT2.h"
#include "ProductData.h"
#include "Qtgui"
#include "SettingsData.h"
#include "ValveDialog.h"
#include "bmessagebox.h"
#include "colors.h"

#include <audittrail.h>

MaintenanceDialog::MaintenanceDialog(MainAppCrystalT2* pMainAppCrystalT2)
    : QWidget(pMainAppCrystalT2), m_ThreadManualTrigger(NULL), m_ManualTriggerIsActive(false), m_ResetCounter(false), m_WaitTimeEnableAlarmTemperatureTank(8000)
{
    ui.setupUi(this);
    m_MainAppCrystalT2 = pMainAppCrystalT2;
    m_ToggelLeftValveContinuous = true;
    m_ToggelRightValveContinuous = true;

    m_MainErrorTextPurge = tr("Error Purge");
    m_StringCanNotTrigger = tr("Can Not Purge/Trigger Valve");
    m_StringNotEnoughPressure = tr("Not Enough Pressure.");
    m_ErrorTextPurgeLeftValue = m_StringCanNotTrigger + QString(" ") + tr("Left") + QString(" ") + m_StringNotEnoughPressure;
    m_ErrorTextPurgeRightValue = m_StringCanNotTrigger + QString(" ") + tr("Right") + QString(" ") + m_StringNotEnoughPressure;
    m_ErrorTextMaschineIsInOffState = tr("Maschine Is In Off State");

    qApp->installEventFilter(this);
    ui.pushButtonPurgLeftValve->setAttribute(Qt::WA_AcceptTouchEvents);
    ui.pushButtonPurgRightValve->setAttribute(Qt::WA_AcceptTouchEvents);

    ui.pushButtonPurgContinuousLeftValve->setAttribute(Qt::WA_AcceptTouchEvents);
    ui.pushButtonPurgContinuousRightValve->setAttribute(Qt::WA_AcceptTouchEvents);

    connect(ui.pushButtonPurgLeftValve, &QPushButton::pressed, this, &MaintenanceDialog::SlotPressedPurgValveLeft);
    connect(ui.pushButtonPurgLeftValve, &QPushButton::released, this, &MaintenanceDialog::SlotReleasedPurgValveLeft);

    connect(ui.pushButtonPurgRightValve, &QPushButton::pressed, this, &MaintenanceDialog::SlotPressedPurgValveRight);
    connect(ui.pushButtonPurgRightValve, &QPushButton::released, this, &MaintenanceDialog::SlotReleasedPurgValveRight);

    connect(ui.pushButtonPurgContinuousLeftValve, &QPushButton::clicked, this, &MaintenanceDialog::SlotPurgContinuousLeftValve);
    connect(ui.pushButtonPurgContinuousRightValve, &QPushButton::clicked, this, &MaintenanceDialog::SlotPurgContinuousRightValve);

    SetAuditTrailProperties();
    m_ThreadManualTrigger = new ThreadManualTrigger(this);

    connect(ui.doubleSpinBoxNumberOfDropletsPerRun, &QDoubleSpinBox::editingFinished, this, &MaintenanceDialog::SlotCounterManualTriggerChanged);
    connect(ui.doubleSpinBoxNumberRuns, &QDoubleSpinBox::editingFinished, this, &MaintenanceDialog::SlotNumberLoopsChanged);
    connect(ui.doubleSpinBoxTimeInMsBetweenRuns, &QDoubleSpinBox::editingFinished, this, &MaintenanceDialog::SlotTimeBetweenLoopsManualTriggerInMsChanged);
    connect(ui.pushButtonResetCounter, &QPushButton::clicked, this, &MaintenanceDialog::SlotResetCounter);

    connect(ui.pushButtonManualTriggerValveLeft, &QPushButton::clicked, this, &MaintenanceDialog::SlotManualTriggerValveLeft);
    connect(ui.pushButtonManualTriggerValveRight, &QPushButton::clicked, this, &MaintenanceDialog::SlotManualTriggerValveRight);
    connect(ui.pushButtonManualTriggerBoth, &QPushButton::clicked, this, &MaintenanceDialog::SlotManualTriggerBoth);
    connect(this, &MaintenanceDialog::SignalManualTriggerReady, this, &MaintenanceDialog::SlotManualTriggerReady);
}

MaintenanceDialog::~MaintenanceDialog()
{
}

bool MaintenanceDialog::event(QEvent* pEvent)
{
    if (pEvent->type() == QEvent::Show) {
        if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
            ui.doubleSpinBoxNumberOfDropletsPerRun->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_NumberOfDropletsPerRun);
            ui.doubleSpinBoxNumberRuns->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_NumberRunsManualTrigger);
            ui.doubleSpinBoxTimeInMsBetweenRuns->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_TimeBetweenRunsManualTriggerInMs);
        }
        if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetLiveImageView()) {
            AddLiveImageWidget((QWidget*)(GetMainAppCrystalT2()->GetLiveImageView()));
            GetMainAppCrystalT2()->GetLiveImageView()->SetShowPlace(SHOW_PLACE_LIVE_IMAGE_VIEW_DROP_ESTIMATION_DIALOG);
            GetMainAppCrystalT2()->GetLiveImageView()->ClearAllGraphicItem();
            // Verhindert das manuelle verschieben der Messfenster unabhängig welcher Maschinenstatus aktiviert ist
            if (!GetMainAppCrystalT2()->GetLiveImageView()->DisableChangeMeasureWindowPosition()) GetMainAppCrystalT2()->GetLiveImageView()->DrawAllMeasureWindows();
        }
    } else if (pEvent->type() == QEvent::Hide) {
        if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetLiveImageView()) {
            RemoveLiveImageWidget((QWidget*)(GetMainAppCrystalT2()->GetLiveImageView()));
            GetMainAppCrystalT2()->GetLiveImageView()->SetShowPlace(SHOW_PLACE_LIVE_IMAGE_VIEW_EDIT_PRODUCT_DIALOG);  // Ursprungszustand wiederherstellen
            GetMainAppCrystalT2()->GetLiveImageView()->ClearAllGraphicItem();
            GetMainAppCrystalT2()->GetLiveImageView()->SetOnCurrentProductionState();
            GetMainAppCrystalT2()->GetLiveImageView()->DrawAllMeasureWindows();
        }
    }
    return QWidget::event(pEvent);
}

void MaintenanceDialog::AddLiveImageWidget(QWidget* w)
{
    if (ui.ImageFrame->layout()) ui.ImageFrame->layout()->addWidget(w);
}

void MaintenanceDialog::RemoveLiveImageWidget(QWidget* w)
{
    if (ui.ImageFrame->layout()) ui.ImageFrame->layout()->removeWidget(w);
}

void MaintenanceDialog::SetAuditTrailProperties()
{
    ui.pushButtonPurgLeftValve->setProperty(kAuditTrail, ui.pushButtonPurgLeftValve->text());
    ui.pushButtonPurgRightValve->setProperty(kAuditTrail, ui.pushButtonPurgRightValve->text());

    ui.pushButtonManualTriggerValveLeft->setProperty(kAuditTrail, ui.pushButtonManualTriggerValveLeft->text());
    ui.pushButtonManualTriggerValveRight->setProperty(kAuditTrail, ui.pushButtonManualTriggerValveRight->text());
    ui.pushButtonManualTriggerBoth->setProperty(kAuditTrail, ui.pushButtonManualTriggerBoth->text());
    ui.pushButtonResetCounter->setProperty(kAuditTrail, ui.pushButtonResetCounter->text());

    ui.doubleSpinBoxNumberOfDropletsPerRun->setProperty(kAuditTrail, ui.labelDashBoardNumberOfDropletsPerRun->text());
    ui.doubleSpinBoxNumberRuns->setProperty(kAuditTrail, ui.labelDashBoardNumberLoops->text());
    ui.doubleSpinBoxTimeInMsBetweenRuns->setProperty(kAuditTrail, ui.labelDashBoardTimeInMsBetweenRuns->text());
    /*if (m_SwitchValvePowerAndPreasure) {
        m_SwitchValvePowerAndPreasure->setProperty(kAuditTrail, ui.groupBoxSetValvePowerAndPreasure->title());
    }*/
}

bool MaintenanceDialog::eventFilter(QObject* obj, QEvent* pEvent)
{
    if (obj == ui.pushButtonPurgLeftValve || obj == ui.pushButtonPurgRightValve) {
        switch (pEvent->type()) {
            case QEvent::TouchBegin:
                if (obj == ui.pushButtonPurgLeftValve) {
                    SlotPressedPurgValveLeft();
                } else {
                    if (obj == ui.pushButtonPurgRightValve) {
                        SlotPressedPurgValveRight();
                    }
                }
                return true;
                break;
            case QEvent::TouchEnd:
                if (obj == ui.pushButtonPurgLeftValve) {
                    SlotReleasedPurgValveLeft();
                } else {
                    if (obj == ui.pushButtonPurgRightValve) {
                        SlotReleasedPurgValveRight();
                    }
                }
                return true;
                break;
            default:
                break;
        }
        return false;
    } else {
        if (obj == ui.pushButtonPurgContinuousLeftValve || obj == ui.pushButtonPurgContinuousRightValve) {
            switch (pEvent->type()) {
                case QEvent::TouchBegin:
                    if (obj == ui.pushButtonPurgContinuousLeftValve) {
                        SlotPurgContinuousLeftValve(m_ToggelLeftValveContinuous);
                        m_ToggelLeftValveContinuous = !m_ToggelLeftValveContinuous;
                    } else {
                        if (obj == ui.pushButtonPurgContinuousRightValve) {
                            SlotPurgContinuousRightValve(m_ToggelRightValveContinuous);
                            m_ToggelRightValveContinuous = !m_ToggelRightValveContinuous;
                        }
                    }
                    return true;
                    break;
               /* case QEvent::TouchEnd:
                    if (obj == ui.pushButtonPurgContinuousLeftValve) {
                        SlotPurgContinuousLeftValve(false);
                    } else {
                        if (obj == ui.pushButtonPurgContinuousRightValve) {
                            SlotPurgContinuousRightValve(false);
                        }
                    }
                    return true;
                    break;*/
                default:
                    break;
            }
            return false;
        }
    }

    return false;
}

void MaintenanceDialog::SlotPurgContinuousLeftValve(bool checked)
{
    if (checked) {
        if (StartPurgValveLeft()) {
            ui.pushButtonPurgContinuousLeftValve->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::green.red()).arg(HMIColor::green.green()).arg(HMIColor::green.blue()));
        } else {
            ui.pushButtonPurgContinuousLeftValve->setChecked(false);  // Spülen konnte nicht gestartet werden Button wieder in den Ausgangszustand
        }
    } else {
        SlotReleasedPurgValveLeft();
        ui.pushButtonPurgContinuousLeftValve->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::Grey.red()).arg(HMIColor::Grey.green()).arg(HMIColor::Grey.blue()));
    }
}

void MaintenanceDialog::SlotPurgContinuousRightValve(bool checked)
{
    if (checked) {
        if (StartPurgValveRight()) {
            ui.pushButtonPurgContinuousRightValve->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::green.red()).arg(HMIColor::green.green()).arg(HMIColor::green.blue()));
        } else {
            ui.pushButtonPurgContinuousRightValve->setChecked(false);  // Spülen konnte nicht gestartet werden Button wieder in den Ausgangszustand
        }
    } else {
        SlotReleasedPurgValveRight();
        ui.pushButtonPurgContinuousRightValve->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::Grey.red()).arg(HMIColor::Grey.green()).arg(HMIColor::Grey.blue()));
    }
}

void MaintenanceDialog::SlotPressedPurgValveLeft()
{
    if (StartPurgValveLeft()) {
        ui.pushButtonPurgLeftValve->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::green.red()).arg(HMIColor::green.green()).arg(HMIColor::green.blue()));
    }
}

void MaintenanceDialog::SlotPressedPurgValveRight()
{
    if (StartPurgValveRight()) {
        ui.pushButtonPurgRightValve->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::green.red()).arg(HMIColor::green.green()).arg(HMIColor::green.blue()));
    }
}

bool MaintenanceDialog::StartPurgValveLeft()
{
    bool rv = false;
    if (GetMainAppCrystalT2()) {
        if (GetMainAppCrystalT2()->GetCurrentMaschineState() == PluginInterface::MachineState::Setup && GetMainAppCrystalT2()->GetStatusCurrentLiquidTankPreasure() == ALARM_LEVEL_OK) {
            ValveDialog* pValveDialog = GetMainAppCrystalT2()->GetValveDialogByID(LEFT_VALVE_ID);
            if (pValveDialog) {
                if (pValveDialog->GetCurrentValveModus() != NORDSON_VALVE_MODUS_PURGE) {
                    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
                    if (pSettingsData) {
                        GetMainAppCrystalT2()->SetSuppressAlarmWarinigPreasureLiquidTank(true);
                        GetMainAppCrystalT2()->SetDefaultPreasure(pSettingsData->m_PreasureIncreaseWhenFlushing, false);  // Druck beim Spülen erhöhen
                        rv = true;
                    }
                    // Auf Spülen stellen. In der Klasse ValveDialog wird dann der Trigger gesetzt, wenn der Wert auf Spülen gesetzt ist
                    pValveDialog->WriteValveParameterModus(NORDSON_VALVE_MODUS_PURGE);
                }
            }
        } else {
            QString ErrorText = m_ErrorTextPurgeLeftValue;
            if (GetMainAppCrystalT2()->GetCurrentMaschineState() == PluginInterface::MachineState::Off) {
                ErrorText = ErrorText + QString(" ") + m_ErrorTextMaschineIsInOffState;
            }
            BMessageBox::critical(this, m_MainErrorTextPurge, ErrorText, QMessageBox::Ok);
        }
    }
    return rv;
}

bool MaintenanceDialog::StartPurgValveRight()
{
    bool rv = false;
    if (GetMainAppCrystalT2()) {
        if (GetMainAppCrystalT2()->GetCurrentMaschineState() == PluginInterface::MachineState::Setup && GetMainAppCrystalT2()->GetStatusCurrentLiquidTankPreasure() == ALARM_LEVEL_OK) {
            ValveDialog* pValveDialog = GetMainAppCrystalT2()->GetValveDialogByID(RIGHT_VALVE_ID);
            if (pValveDialog) {
                if (pValveDialog->GetCurrentValveModus() != NORDSON_VALVE_MODUS_PURGE) {
                    // Beim Spülen soll der Druck erhöht werden, wird der Button losgelassen wird der Druck auf den ursprünglichen Druck zurückgesetzt
                    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
                    if (pSettingsData) {
                        GetMainAppCrystalT2()->SetSuppressAlarmWarinigPreasureLiquidTank(true);
                        GetMainAppCrystalT2()->SetDefaultPreasure(pSettingsData->m_PreasureIncreaseWhenFlushing, false);  // Druck beim Spülen erhöhen
                        rv = true;
                    }
                    // Auf Spülen stellen. In der Klasse ValveDialog wird dann der Trigger gesetzt, wenn der Wert auf Spülen gesetzt ist
                    pValveDialog->WriteValveParameterModus(NORDSON_VALVE_MODUS_PURGE);
                }
            }
        } else {
            QString ErrorText = m_ErrorTextPurgeRightValue;
            if (GetMainAppCrystalT2()->GetCurrentMaschineState() == PluginInterface::MachineState::Off) {
                ErrorText = ErrorText + QString(" ") + m_ErrorTextMaschineIsInOffState;
            }
            BMessageBox::critical(this, m_MainErrorTextPurge, ErrorText, QMessageBox::Ok);
        }
    }
    return rv;
}

void MaintenanceDialog::SlotReleasedPurgValveLeft()
{
    if (GetMainAppCrystalT2()->GetStatusCurrentLiquidTankPreasure() == ALARM_LEVEL_OK) {
        ProductData* pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
        if (pProductData) {  // Druck auf ursprünglichen Wert zurücksetzen
            GetMainAppCrystalT2()->SetDefaultPreasure(pProductData->m_DefaultPreasureInBar, false);
        }
        QTimer::singleShot(1000, this, &MaintenanceDialog::SlotSetLastPurgeModusBackValveLeft);
    }
}

void MaintenanceDialog::SlotReleasedPurgValveRight()
{
    if (GetMainAppCrystalT2()->GetStatusCurrentLiquidTankPreasure() == ALARM_LEVEL_OK) {
        ProductData* pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
        if (pProductData) {  // Druck auf ursprünglichen Wert zurücksetzen
            GetMainAppCrystalT2()->SetDefaultPreasure(pProductData->m_DefaultPreasureInBar, false);
        }
        QTimer::singleShot(1000, this, &MaintenanceDialog::SlotSetLastPurgeModusBackValveRight);
    }
}

void MaintenanceDialog::SlotSetLastPurgeModusBackValveLeft()
{
    if (GetMainAppCrystalT2()) {
        ValveDialog* pValveDialog = GetMainAppCrystalT2()->GetValveDialogByID(LEFT_VALVE_ID);
        if (pValveDialog) {
            pValveDialog->SetTrigger(false);
            ui.pushButtonPurgLeftValve->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::Grey.red()).arg(HMIColor::Grey.green()).arg(HMIColor::Grey.blue()));
            pValveDialog->WriteValveParameterModus(NORDSON_VALVE_MODUS_TIMED);
        }
    }
    // Fehlerstatus wieder freigeben
    QTimer::singleShot(m_WaitTimeEnableAlarmTemperatureTank, this, &MaintenanceDialog::SlotEnableSuppressAlarmWarinigPreasureLiquidTank);
}

void MaintenanceDialog::SlotSetLastPurgeModusBackValveRight()
{
    if (GetMainAppCrystalT2()) {
        ValveDialog* pValveDialog = GetMainAppCrystalT2()->GetValveDialogByID(RIGHT_VALVE_ID);
        if (pValveDialog) {
            pValveDialog->SetTrigger(false);
            ui.pushButtonPurgRightValve->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::Grey.red()).arg(HMIColor::Grey.green()).arg(HMIColor::Grey.blue()));
            pValveDialog->WriteValveParameterModus(NORDSON_VALVE_MODUS_TIMED);  // m_CurrentModusValve2);//Auf alten Wert zurückstellen
        }
    }
    // Fehlerstatus wieder freigeben
    QTimer::singleShot(m_WaitTimeEnableAlarmTemperatureTank, this, &MaintenanceDialog::SlotEnableSuppressAlarmWarinigPreasureLiquidTank);
}

void MaintenanceDialog::SlotEnableSuppressAlarmWarinigPreasureLiquidTank()
{
    if (GetMainAppCrystalT2()) {
        GetMainAppCrystalT2()->SetSuppressAlarmWarinigPreasureLiquidTank(false);
    }
}

void MaintenanceDialog::StartManualTriggerValve(int triggerCount, int usedTriggerOutput)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        m_ManualTriggerIsActive = true;
        int numberRuns = GetMainAppCrystalT2()->GetSettingsData()->m_NumberRunsManualTrigger;
        int timeBetweenRunsManualTriggerInMs = GetMainAppCrystalT2()->GetSettingsData()->m_TimeBetweenRunsManualTriggerInMs;

        for (int j = 0; j < numberRuns; j++) {
            for (int i = 0; i < triggerCount; i++) {
                switch (usedTriggerOutput) {
                    case USE_ONLY_LEFT_VALVE:
                        GetMainAppCrystalT2()->SetManualTriggerOutputSetAndResetValveLeft();
                        break;
                    case USE_ONLY_RIGHT_VALVE:
                        GetMainAppCrystalT2()->SetManualTriggerOutputSetAndResetValveRight();
                        break;
                    case USE_BOTH_VALVES:
                        GetMainAppCrystalT2()->SetManualTriggerOutputValveLeftAndRight();
                        break;
                    default:
                        GetMainAppCrystalT2()->SetManualTriggerOutputValveLeftAndRight();
                        break;
                }
                QThread::currentThread()->msleep(GetMainAppCrystalT2()->GetSettingsData()->m_PauseTriggerManualInMs);
                if (m_ResetCounter) {
                    numberRuns = triggerCount = 0;  // break
                }
            }
            if (j < (numberRuns - 1)) {
                QThread::currentThread()->msleep(timeBetweenRunsManualTriggerInMs);
                if (m_ResetCounter) {
                    numberRuns = triggerCount = 0;  // break
                }
            }
        }
        emit SignalManualTriggerReady();
    }
}

void MaintenanceDialog::SlotManualTriggerReady()
{
    ui.pushButtonManualTriggerValveLeft->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::Grey.red()).arg(HMIColor::Grey.green()).arg(HMIColor::Grey.blue()));
    ui.pushButtonPurgLeftValve->setEnabled(true);
    ui.pushButtonManualTriggerValveRight->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::Grey.red()).arg(HMIColor::Grey.green()).arg(HMIColor::Grey.blue()));
    ui.pushButtonPurgRightValve->setEnabled(true);
    ui.pushButtonManualTriggerBoth->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::Grey.red()).arg(HMIColor::Grey.green()).arg(HMIColor::Grey.blue()));

    if (m_ResetCounter) {
        ui.doubleSpinBoxCurrentTriggerCounterReadingLeftValve->setValue(0.0);
        ui.doubleSpinBoxCurrentTriggerCounterReadingRightValve->setValue(0.0);
        m_ResetCounter = false;
    }
    m_ManualTriggerIsActive = false;
}

void MaintenanceDialog::SlotManualTriggerValveLeft()
{
    if (GetMainAppCrystalT2() && m_ThreadManualTrigger && !m_ThreadManualTrigger->isRunning()) {
        if (GetMainAppCrystalT2()->GetCurrentMaschineState() == PluginInterface::MachineState::Setup && GetMainAppCrystalT2()->GetStatusCurrentLiquidTankPreasure() == ALARM_LEVEL_OK) {
            ui.pushButtonPurgLeftValve->setEnabled(false);
            ui.pushButtonManualTriggerValveLeft->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::green.red()).arg(HMIColor::green.green()).arg(HMIColor::green.blue()));
            m_ThreadManualTrigger->SetTriggerCount(ui.doubleSpinBoxNumberOfDropletsPerRun->value());
            ui.doubleSpinBoxCurrentTriggerCounterReadingLeftValve->setValue(0.0);
            m_ThreadManualTrigger->SetUsedTriggerOutput(USE_ONLY_LEFT_VALVE);
            m_ThreadManualTrigger->start();
        } else {
            QString ErrorText = m_ErrorTextPurgeLeftValue;
            if (GetMainAppCrystalT2()->GetCurrentMaschineState() == PluginInterface::MachineState::Off) {
                ErrorText = ErrorText + QString(" ") + m_ErrorTextMaschineIsInOffState;
            }
            BMessageBox::critical(this, m_MainErrorTextPurge, ErrorText, QMessageBox::Ok);
        }
    }
}

void MaintenanceDialog::SlotManualTriggerValveRight()
{
    if (GetMainAppCrystalT2() && m_ThreadManualTrigger && !m_ThreadManualTrigger->isRunning()) {
        if (GetMainAppCrystalT2()->GetCurrentMaschineState() == PluginInterface::MachineState::Setup && GetMainAppCrystalT2()->GetStatusCurrentLiquidTankPreasure() == ALARM_LEVEL_OK) {
            ui.pushButtonPurgRightValve->setEnabled(false);
            ui.pushButtonManualTriggerValveRight->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::green.red()).arg(HMIColor::green.green()).arg(HMIColor::green.blue()));
            m_ThreadManualTrigger->SetTriggerCount(ui.doubleSpinBoxNumberOfDropletsPerRun->value());
            ui.doubleSpinBoxCurrentTriggerCounterReadingRightValve->setValue(0.0);
            m_ThreadManualTrigger->SetUsedTriggerOutput(USE_ONLY_RIGHT_VALVE);
            m_ThreadManualTrigger->start();
        } else {
            QString ErrorText = m_ErrorTextPurgeRightValue;
            if (GetMainAppCrystalT2()->GetCurrentMaschineState() == PluginInterface::MachineState::Off) {
                ErrorText = ErrorText + QString(" ") + m_ErrorTextMaschineIsInOffState;
            }
            BMessageBox::critical(this, m_MainErrorTextPurge, ErrorText, QMessageBox::Ok);
        }
    }
}

void MaintenanceDialog::SlotManualTriggerBoth()
{
    if (m_ThreadManualTrigger && !m_ThreadManualTrigger->isRunning()) {
        if (GetMainAppCrystalT2()->GetCurrentMaschineState() == PluginInterface::MachineState::Setup && GetMainAppCrystalT2()->GetStatusCurrentLiquidTankPreasure() == ALARM_LEVEL_OK) {
            ui.pushButtonManualTriggerBoth->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::green.red()).arg(HMIColor::green.green()).arg(HMIColor::green.blue()));
            ui.pushButtonPurgLeftValve->setEnabled(false);
            ui.pushButtonManualTriggerValveLeft->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::green.red()).arg(HMIColor::green.green()).arg(HMIColor::green.blue()));
            ui.pushButtonPurgRightValve->setEnabled(false);
            ui.pushButtonManualTriggerValveRight->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::green.red()).arg(HMIColor::green.green()).arg(HMIColor::green.blue()));
            m_ThreadManualTrigger->SetTriggerCount(ui.doubleSpinBoxNumberOfDropletsPerRun->value());
            ui.doubleSpinBoxCurrentTriggerCounterReadingLeftValve->setValue(0.0);
            ui.doubleSpinBoxCurrentTriggerCounterReadingRightValve->setValue(0.0);
            m_ThreadManualTrigger->SetUsedTriggerOutput(USE_BOTH_VALVES);
            m_ThreadManualTrigger->start();
        } else {
            QString ErrorText = m_StringCanNotTrigger + QString(" ") + tr("Left And Right") + QString(" ") + m_StringNotEnoughPressure;
            if (GetMainAppCrystalT2()->GetCurrentMaschineState() == PluginInterface::MachineState::Off) {
                ErrorText = ErrorText + QString(" ") + m_ErrorTextMaschineIsInOffState;
            }
            BMessageBox::critical(this, m_MainErrorTextPurge, ErrorText, QMessageBox::Ok);
        }
    }
}

void MaintenanceDialog::SlotCounterManualTriggerChanged()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        GetMainAppCrystalT2()->GetSettingsData()->m_NumberOfDropletsPerRun = ui.doubleSpinBoxNumberOfDropletsPerRun->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void MaintenanceDialog::SlotNumberLoopsChanged()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        GetMainAppCrystalT2()->GetSettingsData()->m_NumberRunsManualTrigger = ui.doubleSpinBoxNumberRuns->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void MaintenanceDialog::SlotTimeBetweenLoopsManualTriggerInMsChanged()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        GetMainAppCrystalT2()->GetSettingsData()->m_TimeBetweenRunsManualTriggerInMs = ui.doubleSpinBoxTimeInMsBetweenRuns->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void MaintenanceDialog::SlotResetCounter()
{
    if (m_ManualTriggerIsActive) {
        m_ResetCounter = true;
    }
    ui.doubleSpinBoxCurrentTriggerCounterReadingLeftValve->setValue(0.0);
    ui.doubleSpinBoxCurrentTriggerCounterReadingRightValve->setValue(0.0);
    //Test touch with mouse
    //QCoreApplication::sendEvent(ui.pushButtonPurgContinuousRightValve, new QTouchEvent(QEvent::TouchBegin));
    
}

void MaintenanceDialog::IncrementTriggerCounterReadingLeftValve()
{
    double count = ui.doubleSpinBoxCurrentTriggerCounterReadingLeftValve->value();
    count++;
    ui.doubleSpinBoxCurrentTriggerCounterReadingLeftValve->setValue(count);
}

void MaintenanceDialog::IncrementTriggerCounterReadingRightValve()
{
    double count = ui.doubleSpinBoxCurrentTriggerCounterReadingRightValve->value();
    count++;
    ui.doubleSpinBoxCurrentTriggerCounterReadingRightValve->setValue(count);
}
