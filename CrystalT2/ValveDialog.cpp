#include "ValveDialog.h"
#include "EditProductDialog.h"
#include "GlobalConst.h"
#include "ImageData.h"
#include "LiveImageView.h"
#include "MainAppCrystalT2.h"
#include "ProductData.h"
#include "SerialInterface.h"
#include "bswitch.h"

#include <audittrail.h>

ValveDialog::ValveDialog(MainAppCrystalT2* pMainAppCrystalT2, int ValveID)
    : QWidget(pMainAppCrystalT2),
      m_IsValveStatusClicked(false),
      m_WindowSetup(false),
      m_ValveID(0),
      m_SerialInterface(NULL),
      m_IsPicCommandDialogVisible(false),
      m_CurrentValveModus(-1),
      m_ParamterChangedByUser(false)
{
    ui.setupUi(this);
    m_ValveID = ValveID;
    m_MainAppCrystalT2 = pMainAppCrystalT2;

    m_SerialInterface = new SerialInterface(this);

    ui.widgetDashBoardTitleCycle->hide();

    ui.comboBoxMode->insertItem(VALVE_MODUS_TIMED, tr("Timed"));
    ui.comboBoxMode->insertItem(VALVE_MODUS_CONTINUOUS, tr("Continuous"));
    // ui.comboBoxMode->insertItem(VALVE_MODUS_PRUGE, tr("Purge"));

    ui.comboBoxHeatersMode->insertItem(VALVE_MODE_HEATER_OFF, tr("Off"));
    ui.comboBoxHeatersMode->insertItem(VALVE_MODE_HEATER_ON, tr("On"));

    SetAuditTrailProperties();

    connect(ui.pushButtonResetError, &QPushButton::clicked, this, &ValveDialog::SlotResetError);
    connect(ui.pushButtonClearList, &QPushButton::clicked, this, &ValveDialog::SlotClearList);
    connect(ui.pushButtonGetStatus, &QPushButton::clicked, this, &ValveDialog::SlotGetValveStatus);
    connect(ui.pushButtonGetLastErrors, &QPushButton::clicked, this, &ValveDialog::SlotGetLastErrors);

    connect(ui.doubleSpinBoxPause, &QDoubleSpinBox::editingFinished, this, &ValveDialog::SlotPauseChanged);
    connect(ui.doubleSpinBoxPulse, &QDoubleSpinBox::editingFinished, this, &ValveDialog::SlotPulseChanged);
    connect(ui.doubleSpinBoxCount, &QDoubleSpinBox::editingFinished, this, &ValveDialog::SlotCountChanged);
    connect(ui.doubleSpinBoxCloseVolts, &QDoubleSpinBox::editingFinished, this, &ValveDialog::SlotCloseVoltageChanged);
    connect(ui.doubleSpinBoxStroke, &QDoubleSpinBox::editingFinished, this, &ValveDialog::SlotStrokeChanged);
    connect(ui.doubleSpinBoxOpen, &QDoubleSpinBox::editingFinished, this, &ValveDialog::SlotOpenChanged);
    connect(ui.doubleSpinBoxClose, &QDoubleSpinBox::editingFinished, this, &ValveDialog::SlotCloseChanged);
    connect(ui.doubleSpinBoxHeatersTemperature, &QDoubleSpinBox::editingFinished, this, &ValveDialog::SlotTemperatureChanged);

    connect(ui.comboBoxMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ValveDialog::SlotModusChanged);
    connect(ui.comboBoxHeatersMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ValveDialog::SlotTemperatureModeChanged);

    connect(GetSerialInterface(), &SerialInterface::SignalCommadSuccessfullySet, this, &ValveDialog::SlotCommadSuccessfullySet);
    connect(GetSerialInterface(), &SerialInterface::SignalRawResponseString, this, &ValveDialog::SlotShowValveResponse);  // debug info
    connect(GetSerialInterface(), &SerialInterface::SignalShowInfo, m_MainAppCrystalT2, &MainAppCrystalT2::SlotAddNewMessage);
    connect(GetSerialInterface(), &SerialInterface::SignalShowValveTemperature, m_MainAppCrystalT2, &MainAppCrystalT2::SlotShowValveTemperature);

    connect(this, &ValveDialog::SignalShowInfo, m_MainAppCrystalT2, &MainAppCrystalT2::SlotAddNewMessage);
    HidePicoCommandsDialog();
    ui.doubleSpinBoxCount->setKeyboardTracking(false);

    //ui.groupBoxPicoCommands->setProperty(kRequiredAccessLevel, kAccessLevelBertram);

    m_WindowSetup = true;
}

ValveDialog::~ValveDialog()
{
}

void ValveDialog::SetAuditTrailProperties()
{
    ui.comboBoxMode->setProperty(kAuditTrail, ui.labelContentBoardValveMode->text() + tr(" Valve%1").arg(m_ValveID));
    ui.comboBoxHeatersMode->setProperty(kAuditTrail, ui.labelContentBoardHeatersMode->text() + tr(" Valve%1").arg(m_ValveID));
    ui.doubleSpinBoxPause->setProperty(kAuditTrail, ui.labelContentBoardPause->text() + tr(" Valve%1").arg(m_ValveID));
    ui.doubleSpinBoxPulse->setProperty(kAuditTrail, ui.labelContentBoardPulse->text() + tr(" Valve%1").arg(m_ValveID));
    ui.doubleSpinBoxCount->setProperty(kAuditTrail, ui.labelContentBoardCount->text() + tr(" Valve%1").arg(m_ValveID));
    ui.doubleSpinBoxCloseVolts->setProperty(kAuditTrail, ui.labelContentBoardCloseVolts->text() + tr(" Valve%1").arg(m_ValveID));
    ui.doubleSpinBoxStroke->setProperty(kAuditTrail, ui.labelContentBoardStroke->text() + tr(" Valve%1").arg(m_ValveID));
    ui.doubleSpinBoxOpen->setProperty(kAuditTrail, ui.labelContentBoardOpen->text() + tr(" Valve%1").arg(m_ValveID));
    ui.doubleSpinBoxClose->setProperty(kAuditTrail, ui.labelContentBoardClose->text() + tr(" Valve%1").arg(m_ValveID));
    ui.doubleSpinBoxHeatersTemperature->setProperty(kAuditTrail, ui.labelContentBoardHeatersTemperature->text() + tr(" Valve%1").arg(m_ValveID));
}

void ValveDialog::showEvent(QShowEvent*)
{
    if (GetMainAppCrystalT2()->CurrentAccessLevel() == kAccessLevelBertram) {
        ShowPicoCommandsDialog();
        m_IsPicCommandDialogVisible = true;
    } else {
        HidePicoCommandsDialog();
        m_IsPicCommandDialogVisible = false;
    }
}

void ValveDialog::SetGroupName()
{
    QString Title = m_ValveName + QString("(%1)").arg(m_ComPortName);
    ui.groupBoxValveParameter->setTitle(Title);
}

bool ValveDialog::IsSerialPortOpen()
{
    if (GetSerialInterface())
        return GetSerialInterface()->IsSerialPortOpen();
    else
        return false;
}

bool ValveDialog::PortFound()
{
    if (m_ComPortName.isEmpty())
        return false;
    else
        return true;
}

bool ValveDialog::IsValveNotInErrorState()
{
    if (GetSerialInterface())
        return GetSerialInterface()->IsLastStateOk();
    else
        return false;
}

void ValveDialog::StopSerialCommuniction()
{
    if (GetSerialInterface()) GetSerialInterface()->FinishedSerialPort();
}

void ValveDialog::EnableValve()
{
    QString ErrorMsg;
    int rv = ERROR_CODE_NO_ERROR;
    rv = OpenPortAndStartDataExchangeValve(ErrorMsg);
}

void ValveDialog::SetSerialCommand(QString& Command)
{
    if (GetSerialInterface()) GetSerialInterface()->SetSerialCommand(Command);
}

void ValveDialog::SetComPortParameter(int BaudRate, int Parity, int DataBits, int StopBits, QString& ComPortName)
{
    if (GetSerialInterface()) GetSerialInterface()->SetComPortParameter(BaudRate, Parity, DataBits, StopBits, ComPortName);
    m_ComPortName = ComPortName;
}

int ValveDialog::OpenPortAndStartDataExchangeValve(QString& ErrorMsg)
{
    if (GetSerialInterface())
        return GetSerialInterface()->OpenPortAndStartDataExchangeValve(ErrorMsg);
    else
        return ERROR_CODE_ANY_ERROR;
}

void ValveDialog::ShowCycleTime(ValveData& data)
{
    QString CycleText = tr("Cycle: %1 ms").arg(data.m_ValvePulseTimeInms + data.m_ValvePauseTimeInms, 0, 'f', 2);
    ui.labelContentBoardCycle->setText(CycleText);
}

void ValveDialog::SlotClearList()
{
    ui.listWidgetShowResponse->clear();
}

void ValveDialog::HidePicoCommandsDialog()
{
    ui.groupBoxPicoCommands->hide();
}

void ValveDialog::ShowPicoCommandsDialog()
{
    ui.groupBoxPicoCommands->show();
}

void ValveDialog::HideValveParameterDialog()
{
    ui.groupBoxValveParameter->hide();
}

void ValveDialog::ShowValveParameterDialog()
{
    ui.groupBoxValveParameter->show();
}

void ValveDialog::SlotCommadSuccessfullySet(const QString& Command, bool Successfully)
{
    QString CycleText, ErrorMsg, LastCommand = Command;
    QStringList list;
    int IntValue, IntTempValue;
    double DoubleValue, DoubleTempValue;
    ProductData* pProductData = NULL;
    ValveData CurrentValveData;

    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetCurrentProductData()) pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
    if (pProductData && pProductData->ContainsValveKey(m_ValveID)) {
        CurrentValveData = pProductData->GetValveData(m_ValveID);
        QHash<int, ValveData>* pListValveData = pProductData->GetListValveData();
        if (LastCommand.contains(COMMAND_COUNT)) {
            list = LastCommand.split(COMMAND_COUNT);
            IntValue = list.at(0).toInt();
            if (Successfully) {
                if (CurrentValveData.m_ValveCount != IntValue) {
                    IntTempValue = CurrentValveData.m_ValveCount;  // alten wert sichern, wenn evtl. gespeichert nicht funktioniert
                    CurrentValveData.m_ValveCount = IntValue;
                    pListValveData->insert(m_ValveID, CurrentValveData);
                    if (pProductData->WriteProductData(ErrorMsg) != ERROR_CODE_NO_ERROR)  // save parameter
                    {                                                                     // error save value on disk, set old value back
                        CurrentValveData.m_ValveCount = IntTempValue;
                        pListValveData->insert(m_ValveID, CurrentValveData);   // Ursprünglichen wert zurücksetzen da speichern auf platte fehlgeschlagen
                        emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
                    }
                }
            } else {
                ErrorMsg = tr("Can Not Set Command(Count):%1 To Valve:%2").arg(LastCommand).arg(m_ComPortName);
                emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
            }
            disconnect(ui.doubleSpinBoxCount, &QDoubleSpinBox::editingFinished, this, &ValveDialog::SlotCountChanged);
            ui.doubleSpinBoxCount->setValue(CurrentValveData.m_ValveCount);
            connect(ui.doubleSpinBoxCount, &QDoubleSpinBox::editingFinished, this, &ValveDialog::SlotCountChanged);
        } else if (LastCommand.contains(COMMAND_PULSE)) {
            list = LastCommand.split(COMMAND_PULSE);
            DoubleValue = list.at(0).toDouble();
            if (Successfully) {
                if (CurrentValveData.m_ValvePulseTimeInms != DoubleValue) {
                    DoubleTempValue = CurrentValveData.m_ValvePulseTimeInms;
                    CurrentValveData.m_ValvePulseTimeInms = DoubleValue;
                    pListValveData->insert(m_ValveID, CurrentValveData);
                    if (pProductData->WriteProductData(ErrorMsg) != ERROR_CODE_NO_ERROR)  // save parameter
                    {                                                                     // error save value on disk, set old value back
                        CurrentValveData.m_ValvePulseTimeInms = DoubleTempValue;
                        pListValveData->insert(m_ValveID, CurrentValveData);
                        emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
                    }
                }
            } else {
                ErrorMsg = tr("Can Not Set Command(Pulse):%1 To Valve:%2").arg(LastCommand).arg(m_ComPortName);
                emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
            }
            disconnect(ui.doubleSpinBoxPulse, &QDoubleSpinBox::editingFinished, this, &ValveDialog::SlotPulseChanged);
            ui.doubleSpinBoxPulse->setValue(CurrentValveData.m_ValvePulseTimeInms);
            connect(ui.doubleSpinBoxPulse, &QDoubleSpinBox::editingFinished, this, &ValveDialog::SlotPulseChanged);

            if (GetMainAppCrystalT2()->GetEditProductDialog()) {
                GetMainAppCrystalT2()->GetEditProductDialog()->SetResponseValuePulseFromController(m_ValveID, CurrentValveData.m_ValvePulseTimeInms);
            }

            ShowCycleTime(CurrentValveData);
        } else if (LastCommand.contains(COMMAND_PAUSE)) {
            list = LastCommand.split(COMMAND_PAUSE);
            DoubleValue = list.at(0).toDouble();
            if (Successfully) {
                if (CurrentValveData.m_ValvePauseTimeInms != DoubleValue) {
                    DoubleTempValue = CurrentValveData.m_ValvePauseTimeInms;
                    CurrentValveData.m_ValvePauseTimeInms = DoubleValue;
                    pListValveData->insert(m_ValveID, CurrentValveData);
                    if (pProductData->WriteProductData(ErrorMsg) != ERROR_CODE_NO_ERROR)  // save parameter
                    {                                                                     // error save value on disk, set old value back
                        CurrentValveData.m_ValvePauseTimeInms = DoubleTempValue;
                        pListValveData->insert(m_ValveID, CurrentValveData);
                        emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
                    }
                }
            } else {
                ErrorMsg = tr("Can Not Set Command(Pause):%1 To Valve:%2").arg(LastCommand).arg(m_ComPortName);
                emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
            }
            disconnect(ui.doubleSpinBoxPause, &QDoubleSpinBox::editingFinished, this, &ValveDialog::SlotPauseChanged);
            ui.doubleSpinBoxPause->setValue(CurrentValveData.m_ValvePauseTimeInms);
            connect(ui.doubleSpinBoxPause, &QDoubleSpinBox::editingFinished, this, &ValveDialog::SlotPauseChanged);
            ShowCycleTime(CurrentValveData);
        } else if (LastCommand.contains(COMMAND_MODUS)) {
            if (Successfully) {
                list = LastCommand.split(COMMAND_MODUS);
                IntValue = list.at(0).toInt();
                if (IntValue == NORDSON_VALVE_MODUS_PURGE)  // zwei ist spülen
                {
                    SetTrigger(true);
                    m_CurrentValveModus = NORDSON_VALVE_MODUS_PURGE;
                } else {
                    if (CurrentValveData.m_ValveModus != IntValue) {
                        IntTempValue = CurrentValveData.m_ValveModus;
                        CurrentValveData.m_ValveModus = IntValue;
                        pListValveData->insert(m_ValveID, CurrentValveData);
                        if (pProductData->WriteProductData(ErrorMsg) != ERROR_CODE_NO_ERROR)  // save parameter
                        {                                                                     // error save value on disk, set old value back
                            CurrentValveData.m_ValveModus = IntTempValue;
                            pListValveData->insert(m_ValveID, CurrentValveData);
                            emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
                        }
                    }
                    disconnect(ui.comboBoxMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ValveDialog::SlotModusChanged);
                    if (!m_ParamterChangedByUser) {
                        ui.comboBoxMode->setProperty(kAuditTrail, QVariant());
                    }
                    if (CurrentValveData.m_ValveModus == NORDSON_VALVE_MODUS_TIMED)
                        ui.comboBoxMode->setCurrentIndex(VALVE_MODUS_TIMED);  // Zeitgesteuert
                    else {
                        if (CurrentValveData.m_ValveModus == NORDSON_VALVE_MODUS_CONTINUOUS) {
                            ui.comboBoxMode->setCurrentIndex(VALVE_MODUS_CONTINUOUS);  // continuierlich
                        }
                    }
                    if (!m_ParamterChangedByUser) {
                        ui.comboBoxMode->setProperty(kAuditTrail, ui.labelContentBoardValveMode->text() + tr(" Valve%1").arg(m_ValveID));
                    }
                    m_CurrentValveModus = CurrentValveData.m_ValveModus;
                    connect(ui.comboBoxMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ValveDialog::SlotModusChanged);
                }
            } else {
                ErrorMsg = tr("Can Not Set Command(Modus):%1 To Valve:%2").arg(LastCommand).arg(m_ComPortName);
                emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
            }
        } else if (LastCommand.contains(COMMAND_CLOSE_VOLTAGE)) {
            list = LastCommand.split(COMMAND_CLOSE_VOLTAGE);
            DoubleValue = list.at(0).toDouble();
            if (Successfully) {
                if (CurrentValveData.m_ValveCloseVoltage != DoubleValue) {
                    DoubleTempValue = CurrentValveData.m_ValveCloseVoltage;
                    CurrentValveData.m_ValveCloseVoltage = DoubleValue;
                    pListValveData->insert(m_ValveID, CurrentValveData);
                    if (pProductData->WriteProductData(ErrorMsg) != ERROR_CODE_NO_ERROR)  // save parameter
                    {                                                                     // error save value on disk, set old value back
                        CurrentValveData.m_ValveCloseVoltage = DoubleTempValue;
                        pListValveData->insert(m_ValveID, CurrentValveData);
                        emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
                    }
                }
            } else {
                ErrorMsg = tr("Can Not Set Command(Close Voltage):%1 To Valve:%2").arg(LastCommand).arg(m_ComPortName);
                emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
            }
            disconnect(ui.doubleSpinBoxCloseVolts, &QDoubleSpinBox::editingFinished, this, &ValveDialog::SlotCloseVoltageChanged);
            ui.doubleSpinBoxCloseVolts->setValue(CurrentValveData.m_ValveCloseVoltage);
            connect(ui.doubleSpinBoxCloseVolts, &QDoubleSpinBox::editingFinished, this, &ValveDialog::SlotCloseVoltageChanged);
        } else if (LastCommand.contains(COMMAND_STROKE)) {
            list = LastCommand.split(COMMAND_STROKE);
            DoubleValue = list.at(0).toDouble();
            if (Successfully) {
                if (CurrentValveData.m_ValveStrokeInPercent != DoubleValue) {
                    DoubleTempValue = CurrentValveData.m_ValveStrokeInPercent;
                    CurrentValveData.m_ValveStrokeInPercent = DoubleValue;
                    pListValveData->insert(m_ValveID, CurrentValveData);
                    if (pProductData->WriteProductData(ErrorMsg) != ERROR_CODE_NO_ERROR)  // save parameter
                    {                                                                     // error save value on disk, set old value back
                        CurrentValveData.m_ValveStrokeInPercent = DoubleTempValue;
                        pListValveData->insert(m_ValveID, CurrentValveData);
                        emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
                    }
                }
            } else {
                ErrorMsg = tr("Can Not Set Command(Stroke):%1 To Valve:%2").arg(LastCommand).arg(m_ComPortName);
                emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
            }
            disconnect(ui.doubleSpinBoxStroke, &QDoubleSpinBox::editingFinished, this, &ValveDialog::SlotStrokeChanged);
            ui.doubleSpinBoxStroke->setValue(CurrentValveData.m_ValveStrokeInPercent);
            connect(ui.doubleSpinBoxStroke, &QDoubleSpinBox::editingFinished, this, &ValveDialog::SlotStrokeChanged);
        } else if (LastCommand.contains(COMMAND_OPEN)) {
            list = LastCommand.split(COMMAND_OPEN);
            DoubleValue = list.at(0).toDouble() / 1000.0;
            if (Successfully) {
                if (CurrentValveData.m_ValveOpenTimeInms != DoubleValue) {
                    DoubleTempValue = CurrentValveData.m_ValveOpenTimeInms;
                    CurrentValveData.m_ValveOpenTimeInms = DoubleValue;
                    pListValveData->insert(m_ValveID, CurrentValveData);
                    if (pProductData->WriteProductData(ErrorMsg) != ERROR_CODE_NO_ERROR)  // save parameter
                    {                                                                     // error save value on disk, set old value back
                        CurrentValveData.m_ValveOpenTimeInms = DoubleTempValue;
                        pListValveData->insert(m_ValveID, CurrentValveData);
                        emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
                    }
                }
            } else {
                ErrorMsg = tr("Can Not Set Command(Ramp Open):%1 To Valve:%2").arg(LastCommand).arg(m_ComPortName);
                emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
            }
            disconnect(ui.doubleSpinBoxOpen, &QDoubleSpinBox::editingFinished, this, &ValveDialog::SlotOpenChanged);
            ui.doubleSpinBoxOpen->setValue(CurrentValveData.m_ValveOpenTimeInms);
            connect(ui.doubleSpinBoxOpen, &QDoubleSpinBox::editingFinished, this, &ValveDialog::SlotOpenChanged);
        } else if (LastCommand.contains(COMMAND_CLOSE)) {
            list = LastCommand.split(COMMAND_CLOSE);
            DoubleValue = list.at(0).toDouble() / 1000.0;
            if (Successfully) {
                if (CurrentValveData.m_ValveCloseTimeInms != DoubleValue) {
                    DoubleTempValue = CurrentValveData.m_ValveCloseTimeInms;
                    CurrentValveData.m_ValveCloseTimeInms = DoubleValue;
                    pListValveData->insert(m_ValveID, CurrentValveData);
                    if (pProductData->WriteProductData(ErrorMsg) != ERROR_CODE_NO_ERROR)  // save parameter
                    {                                                                     // error save value on disk, set old value back
                        CurrentValveData.m_ValveCloseTimeInms = DoubleTempValue;
                        pListValveData->insert(m_ValveID, CurrentValveData);
                        emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
                    }
                }
            } else {
                ErrorMsg = tr("Can Not Set Command(Ramp Close):%1 To Valve:%2").arg(LastCommand).arg(m_ComPortName);
                emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
            }
            disconnect(ui.doubleSpinBoxClose, &QDoubleSpinBox::editingFinished, this, &ValveDialog::SlotCloseChanged);
            ui.doubleSpinBoxClose->setValue(CurrentValveData.m_ValveCloseTimeInms);
            connect(ui.doubleSpinBoxClose, &QDoubleSpinBox::editingFinished, this, &ValveDialog::SlotCloseChanged);
        } else if (LastCommand.contains(COMMAND_TEMPERATURE)) {
            list = LastCommand.split(COMMAND_TEMPERATURE);
            DoubleValue = list.at(0).toDouble();
            if (Successfully) {
                if (CurrentValveData.m_ValveTemperature != DoubleValue) {
                    DoubleTempValue = CurrentValveData.m_ValveTemperature;
                    CurrentValveData.m_ValveTemperature = DoubleValue;
                    pListValveData->insert(m_ValveID, CurrentValveData);
                    if (pProductData->WriteProductData(ErrorMsg) != ERROR_CODE_NO_ERROR)  // save parameter
                    {                                                                     // error save value on disk, set old value back
                        CurrentValveData.m_ValveTemperature = DoubleTempValue;
                        pListValveData->insert(m_ValveID, CurrentValveData);
                        emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
                    }
                }
            } else {
                ErrorMsg = tr("Can Not Set Command(Heaters Temperature):%1 To Valve:%2").arg(LastCommand).arg(m_ComPortName);
                emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
            }
            disconnect(ui.doubleSpinBoxHeatersTemperature, &QDoubleSpinBox::editingFinished, this, &ValveDialog::SlotTemperatureChanged);
            ui.doubleSpinBoxHeatersTemperature->setValue(CurrentValveData.m_ValveTemperature);
            connect(ui.doubleSpinBoxHeatersTemperature, &QDoubleSpinBox::editingFinished, this, &ValveDialog::SlotTemperatureChanged);
        } else if (LastCommand.contains(COMMAND_TEMPERATURE_MODE)) {
            list = LastCommand.split(COMMAND_TEMPERATURE_MODE);
            IntValue = list.at(0).toInt();
            if (Successfully) {
                if (CurrentValveData.m_ValveModeTemperature != IntValue) {
                    IntTempValue = CurrentValveData.m_ValveModeTemperature;
                    CurrentValveData.m_ValveModeTemperature = IntValue;
                    pListValveData->insert(m_ValveID, CurrentValveData);
                    if (pProductData->WriteProductData(ErrorMsg) != ERROR_CODE_NO_ERROR)  // save parameter
                    {                                                                     // error save value on disk, set old value back
                        CurrentValveData.m_ValveModeTemperature = IntTempValue;
                        pListValveData->insert(m_ValveID, CurrentValveData);
                        emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
                    }
                }
            } else {
                ErrorMsg = tr("Can Not Set Command(Heaters Temperature Mode):%1 To Valve:%2").arg(LastCommand).arg(m_ComPortName);
                emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
            }
            disconnect(ui.comboBoxHeatersMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ValveDialog::SlotTemperatureModeChanged);
            if (!m_ParamterChangedByUser) {
                ui.comboBoxHeatersMode->setProperty(kAuditTrail, QVariant());  // Auditrail ausschalten, nur bei comboboxeen notwendig
            }
            ui.comboBoxHeatersMode->setCurrentIndex(CurrentValveData.m_ValveModeTemperature);
            if (!m_ParamterChangedByUser) {
                ui.comboBoxHeatersMode->setProperty(kAuditTrail, ui.labelContentBoardHeatersMode->text() + tr(" Valve%1").arg(m_ValveID));
            }
            connect(ui.comboBoxHeatersMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ValveDialog::SlotTemperatureModeChanged);
        }
    }
    m_MutexWaitForCommandIsSet.lock();
    m_WaitForCommandIsSet.wakeAll();
    m_MutexWaitForCommandIsSet.unlock();
    m_ParamterChangedByUser = false;
}

void ValveDialog::WaitCommandIsSet(int TimeOut)
{
    m_MutexWaitForCommandIsSet.lock();
    m_WaitForCommandIsSet.wait(&m_MutexWaitForCommandIsSet, TimeOut);
    m_MutexWaitForCommandIsSet.unlock();
}

bool ValveDialog::IsValveHeaterOn()
{
    if (ui.comboBoxHeatersMode->currentIndex() == VALVE_MODE_HEATER_ON)
        return true;
    else
        return false;
}

double ValveDialog::GetValveHeaterTargetTempterature()
{
    return ui.doubleSpinBoxHeatersTemperature->value();
}

void ValveDialog::SetTrigger(bool alwaysOn)
{
    if (m_ValveID == RIGHT_VALVE_ID)
        GetMainAppCrystalT2()->SetManualTriggerOutputValveRight(alwaysOn);
    else
        GetMainAppCrystalT2()->SetManualTriggerOutputValveLeft(alwaysOn);
}

void ValveDialog::SlotTemperatureModeChanged(int value)
{
    if (m_WindowSetup) {
        m_ParamterChangedByUser = true;
        WriteValveParameterTemperatureMode(value);
    }
}

void ValveDialog::SlotTemperatureChanged()
{
    if (m_WindowSetup) WriteValveParameterTemperature(ui.doubleSpinBoxHeatersTemperature->value());
}

void ValveDialog::SlotCloseVoltageChanged()
{
    if (m_WindowSetup) {
        WriteValveParameterCloseVoltage(ui.doubleSpinBoxCloseVolts->value());
    }
}

void ValveDialog::SlotStrokeChanged()
{
    if (m_WindowSetup) WriteValveParameterStroke(ui.doubleSpinBoxStroke->value());
}

void ValveDialog::SlotCloseChanged()
{
    if (m_WindowSetup) WriteValveParameterClose(ui.doubleSpinBoxClose->value());
}

void ValveDialog::SlotOpenChanged()
{
    if (m_WindowSetup) WriteValveParameterOpen(ui.doubleSpinBoxOpen->value());
}

void ValveDialog::SlotModusChanged(int value)
{
    if (m_WindowSetup) {
        m_ParamterChangedByUser = true;
        if (value == VALVE_MODUS_TIMED) {
            WriteValveParameterModus(NORDSON_VALVE_MODUS_TIMED);
        } else {
            if (value == VALVE_MODUS_CONTINUOUS) {
                WriteValveParameterModus(NORDSON_VALVE_MODUS_CONTINUOUS);
            } else {
                WriteValveParameterModus(NORDSON_VALVE_MODUS_PURGE);
            }
        }
    }
}

void ValveDialog::SlotPauseChanged()
{
    if (m_WindowSetup) WriteValveParameterPause(ui.doubleSpinBoxPause->value());
}

void ValveDialog::SlotPulseChanged()
{
    if (m_WindowSetup) WriteValveParameterPulse(ui.doubleSpinBoxPulse->value());
}

void ValveDialog::SlotCountChanged()
{
    if (m_WindowSetup) WriteValveParameterCount(ui.doubleSpinBoxCount->value());
}

void ValveDialog::SlotGetLastErrors()
{
    QString Command = COMMAND_GET_LAST_ERRORS;
    SetSerialCommand(Command);
}

void ValveDialog::SlotResetError()
{
    QString Command = COMMAND_RESET_ERROR;
    SetSerialCommand(Command);
}

void ValveDialog::SlotGetValveStatus()
{
    m_IsValveStatusClicked = true;
    QString Command = COMMAND_GET_STATUS_VALVE;
    SetSerialCommand(Command);
}

void ValveDialog::SlotGetHeatersStatus()
{
    m_IsValveStatusClicked = true;
    QString Command = COMMAND_GET_STATUS_HEATERS;
    SetSerialCommand(Command);
}

bool ValveDialog::IsValveStatusClicked()
{
    bool rv;
    if (m_IsValveStatusClicked) {
        m_IsValveStatusClicked = false;
        rv = true;
    } else
        rv = false;
    return rv;
}

void ValveDialog::SetPICOComand(const QString& command, double Value)
{
    QString Command = command;
    QString Temp = QString::number(static_cast<int>(Value));
    int NumberFill = Command.count(QChar('x')) - Temp.length();

    for (int i = 0; i < NumberFill; i++) Temp.prepend("0");
    Command = Temp + Command.remove(QChar('x'));
    SetSerialCommand(Command);
}

void ValveDialog::SetPICOComandDecimal(const QString& command, double Value, int decimal)
{
    QString Command = command;
    QString Temp = QString::number(Value, 'f', decimal);
    int NumberFill = Command.count(QChar('x')) + Command.count(QChar('.')) - Temp.length();

    for (int i = 0; i < NumberFill; i++) Temp.prepend("0");
    Command = Command.remove(QChar('.'));
    Command = Temp + Command.remove(QChar('x'));
    SetSerialCommand(Command);
}

void ValveDialog::WriteValveParameterPiezoCurrentOnOff(int Value)
{
    SetPICOComand(COMMAND_SET_PIEZO_CURRENT, Value);
}

void ValveDialog::WriteValveParameterTemperatureMode(int Value)
{
    SetPICOComand(COMMAND_SET_TEMPERATURE_MODE, Value);
}

void ValveDialog::WriteValveParameterTemperature(double Value)
{
    SetPICOComandDecimal(COMMAND_SET_TEMPERATURE, Value, 1);
}

void ValveDialog::WriteValveParameterCloseVoltage(double Value)
{
    SetPICOComand(COMMAND_SET_VALVE_CLOSE_VOLTAGE, Value);
}

void ValveDialog::WriteValveParameterStroke(double Value)
{
    SetPICOComand(COMMAND_SET_VALVE_STROKE, Value);
}

void ValveDialog::WriteValveParameterClose(double Value)
{
    SetPICOComand(COMMAND_SET_VALVE_CLOSE, Value * 1000.0);
}

void ValveDialog::WriteValveParameterOpen(double Value)
{
    SetPICOComand(COMMAND_SET_VALVE_OPEN, Value * 1000.0);
}

void ValveDialog::WriteValveParameterModus(int Value)
{
    SetPICOComand(COMMAND_SET_VALVE_MODUS, Value);
}

void ValveDialog::WriteValveParameterCount(double Value)
{
    SetPICOComand(COMMAND_SET_VALVE_COUNT, Value);
}

void ValveDialog::WriteValveParameterPulse(double Value)
{
    SetPICOComandDecimal(COMMAND_SET_VALVE_PULSE, Value);
}

void ValveDialog::WriteValveParameterPause(double Value)
{
    SetPICOComandDecimal(COMMAND_SET_VALVE_PAUSE, Value);
}

int ValveDialog::SetAllValveParametersToDevice(QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    if (IsSerialPortOpen()) {
        if (GetMainAppCrystalT2() && IsValveNotInErrorState()) {
            ProductData* pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
            if (pProductData && pProductData->ContainsValveKey(m_ValveID)) {
                ValveData CurrentValveData = pProductData->GetValveData(m_ValveID);
                m_ParamterChangedByUser = false;
                // m_Mutex.lock();
                // set valve parameter to device
                WriteValveParameterCount(CurrentValveData.m_ValveCount);
                // m_WaitCondition.wait(&m_Mutex, WaitTimeSetValueToDeviceInms);//wait umtil value is set to device
                WriteValveParameterPulse(CurrentValveData.m_ValvePulseTimeInms);
                // m_WaitCondition.wait(&m_Mutex, WaitTimeSetValueToDeviceInms);//wait umtil value is set to device
                WriteValveParameterPause(CurrentValveData.m_ValvePauseTimeInms);
                // m_WaitCondition.wait(&m_Mutex, WaitTimeSetValueToDeviceInms);//wait umtil value is set to device
                WriteValveParameterModus(CurrentValveData.m_ValveModus);
                // m_WaitCondition.wait(&m_Mutex, WaitTimeSetValueToDeviceInms);//wait umtil value is set to device
                WriteValveParameterCloseVoltage(CurrentValveData.m_ValveCloseVoltage);
                // m_WaitCondition.wait(&m_Mutex, WaitTimeSetValueToDeviceInms);//wait umtil value is set to device
                WriteValveParameterStroke(CurrentValveData.m_ValveStrokeInPercent);
                // m_WaitCondition.wait(&m_Mutex, WaitTimeSetValueToDeviceInms);//wait umtil value is set to device
                WriteValveParameterClose(CurrentValveData.m_ValveCloseTimeInms);
                // m_WaitCondition.wait(&m_Mutex, WaitTimeSetValueToDeviceInms);//wait umtil value is set to device
                WriteValveParameterOpen(CurrentValveData.m_ValveOpenTimeInms);
                // m_WaitCondition.wait(&m_Mutex, WaitTimeSetValueToDeviceInms);//wait umtil value is set to device
                WriteValveParameterTemperature(CurrentValveData.m_ValveTemperature);
                // m_WaitCondition.wait(&m_Mutex, WaitTimeSetValueToDeviceInms);//wait umtil value is set to device
                WriteValveParameterTemperatureMode(CurrentValveData.m_ValveModeTemperature);
                // m_WaitCondition.wait(&m_Mutex, WaitTimeSetValueToDeviceInms);//wait umtil value is set to device
                // m_Mutex.unlock();
            }
        } else {
            rv = ERROR_CODE_ANY_ERROR;
            ErrorMsg = tr("Error Set Valve Parameter. Device Not Connected Or In Error State! Port:%1").arg(m_ComPortName);
        }
    }

    return rv;
}

void ValveDialog::ResetIsClicked()
{
    if (!IsValveNotInErrorState()) {
        SlotResetError();
    }
}

void ValveDialog::SlotShowValveResponse(const QString& Msg)
{
    QString Response = Msg;
    QStringList list;

    list = Response.split('\n');
    for (int i = 0; i < list.count(); i++) {
        ui.listWidgetShowResponse->addItem(list.at(i));
    }
    // if (InfoCode == ERROR_CODE_ANY_ERROR)
    //	NewStyle.replace(QString("rgb(%1,%2,%3)").arg(COLOR_TITLE_BAR.red()).arg(COLOR_TITLE_BAR.green()).arg(COLOR_TITLE_BAR.blue()),
    // QString("rgb(%1,%2,%3)").arg(COLOR_STATUS_ALARM.red()).arg(COLOR_STATUS_ALARM.green()).arg(COLOR_STATUS_ALARM.blue())); else
    //	NewStyle.replace(QString("rgb(%1,%2,%3)").arg(COLOR_STATUS_ALARM.red()).arg(COLOR_STATUS_ALARM.green()).arg(COLOR_STATUS_ALARM.blue()),
    // QString("rgb(%1,%2,%3)").arg(COLOR_TITLE_BAR.red()).arg(COLOR_TITLE_BAR.green()).arg(COLOR_TITLE_BAR.blue())); ui.pushButtonResetError->setStyleSheet(NewStyle);

    if (ui.listWidgetShowResponse->count() > 100) {
        ui.listWidgetShowResponse->clear();
    }
}

SettingsData* ValveDialog::GetSettingsData()
{
    if (GetMainAppCrystalT2())
        return GetMainAppCrystalT2()->GetSettingsData();
    else
        return NULL;
}

int ValveDialog::WriteLogFile(const QString& data, const QString& FileName)
{
    if (GetMainAppCrystalT2())
        return GetMainAppCrystalT2()->WriteLogFile(data, FileName);
    else
        return ERROR_CODE_NO_ERROR;
}
