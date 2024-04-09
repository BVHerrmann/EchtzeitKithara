#include "SerialPortSettingsDialog.h"
#include "Globalconst.h"
#include "MainAppCrystalT2.h"
#include "QtSerialPort/qserialportinfo.h"
#include "SerialInterface.h"
#include "bmessagebox.h"

#include <audittrail.h>

SerialPortSettingsDialog::SerialPortSettingsDialog(MainAppCrystalT2* pMainAppCrystalT2) : QWidget(pMainAppCrystalT2), m_MainAppCrystalT2(NULL), m_WindowSetup(false)
{
    ui.setupUi(this);
    m_MainAppCrystalT2 = pMainAppCrystalT2;
    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();

    if (pSettingsData) {
        ui.comboBoxChooseNumberValves->insertItem(0, tr("One Valve"));
        ui.comboBoxChooseNumberValves->insertItem(1, tr("Two Valves"));
        if (pSettingsData->m_WorkWithTwoValves)
            ui.comboBoxChooseNumberValves->setCurrentIndex(1);
        else
            ui.comboBoxChooseNumberValves->setCurrentIndex(0);
    }

    ui.comboBoxSerialPortBaudrate->addItem(QString("1200"), QSerialPort::Baud1200);
    ui.comboBoxSerialPortBaudrate->addItem(QString("2400"), QSerialPort::Baud2400);
    ui.comboBoxSerialPortBaudrate->addItem(QString("4800"), QSerialPort::Baud4800);
    ui.comboBoxSerialPortBaudrate->addItem(QString("9600"), QSerialPort::Baud9600);
    ui.comboBoxSerialPortBaudrate->addItem(QString("19200"), QSerialPort::Baud19200);
    ui.comboBoxSerialPortBaudrate->addItem(QString("38400"), QSerialPort::Baud38400);
    ui.comboBoxSerialPortBaudrate->addItem(QString("57600"), QSerialPort::Baud57600);
    ui.comboBoxSerialPortBaudrate->addItem(QString("115200"), QSerialPort::Baud115200);

    ui.comboBoxSerialPortStopbits->addItem(QString("1"), QSerialPort::OneStop);
    ui.comboBoxSerialPortStopbits->addItem(QString("1.5"), QSerialPort::OneAndHalfStop);
    ui.comboBoxSerialPortStopbits->addItem(QString("2"), QSerialPort::TwoStop);

    ui.comboBoxSerialPortDatabits->addItem(QString("5"), QSerialPort::Data5);
    ui.comboBoxSerialPortDatabits->addItem(QString("6"), QSerialPort::Data6);
    ui.comboBoxSerialPortDatabits->addItem(QString("7"), QSerialPort::Data7);
    ui.comboBoxSerialPortDatabits->addItem(QString("8"), QSerialPort::Data8);

    ui.comboBoxSerialPortParity->addItem(QString("0"), QSerialPort::NoParity);
    ui.comboBoxSerialPortParity->addItem(QString("2"), QSerialPort::EvenParity);
    ui.comboBoxSerialPortParity->addItem(QString("3"), QSerialPort::OddParity);
    ui.comboBoxSerialPortParity->addItem(QString("4"), QSerialPort::SpaceParity);
    ui.comboBoxSerialPortParity->addItem(QString("5"), QSerialPort::MarkParity);

    SetAuditTrailProperties();

    connect(ui.comboBoxSerialPortParity, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SerialPortSettingsDialog::SlotParityChanged);
    connect(ui.comboBoxSerialPortDatabits, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SerialPortSettingsDialog::SlotDataBitsChanged);
    connect(ui.comboBoxSerialPortStopbits, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SerialPortSettingsDialog::SlotStopBitsChanged);
    connect(ui.comboBoxSerialPortBaudrate, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SerialPortSettingsDialog::SlotBaudrateChanged);

    // connect(ui.checkBoxInterChangeSerialPort, &QCheckBox::stateChanged, this, &SerialPortSettingsDialog::SlotInterchangeSerialPort);
    connect(ui.checkBoxInterChangeTriggerOutput, &QCheckBox::stateChanged, this, &SerialPortSettingsDialog::SlotInterchangeTriggerOutput);
    connect(ui.comboBoxChooseNumberValves, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SerialPortSettingsDialog::SlotNumberValvesChanged);

    //ui.groupBoxSerialSettings->setProperty(kRequiredAccessLevel, kAccessLevelBertram);
    //ui.groupBoxChangeValveOrder->setProperty(kRequiredAccessLevel, kAccessLevelBertram);
    ui.ContentBoardItemInterChangeSerialPort->hide();
    m_WindowSetup = true;
}

SerialPortSettingsDialog::~SerialPortSettingsDialog()
{
}

void SerialPortSettingsDialog::SetAuditTrailProperties()
{
    ui.checkBoxInterChangeSerialPort->setProperty(kAuditTrail, ui.labelInterChangeSerialPort->text());
    ui.checkBoxInterChangeTriggerOutput->setProperty(kAuditTrail, ui.labelInterChangeTriggerOutput->text());
    ui.comboBoxChooseNumberValves->setProperty(kAuditTrail, ui.labelSetNumberValves->text());

    ui.comboBoxSerialPortParity->setProperty(kAuditTrail, ui.labelSerialPortParity->text());
    ui.comboBoxSerialPortDatabits->setProperty(kAuditTrail, ui.labelSerialPortDatabits->text());
    ui.comboBoxSerialPortStopbits->setProperty(kAuditTrail, ui.labelSerialPortStopbits->text());
    ui.comboBoxSerialPortBaudrate->setProperty(kAuditTrail, ui.labelSerialPortBaudrate->text());
}

void SerialPortSettingsDialog::SlotNumberValvesChanged(int index)
{
    if (GetMainAppCrystalT2()) {
        QString Message;
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();

        if (index == 0)
            Message = tr("Work with two valves");
        else
            Message = tr("Work with one valves");
        BMessageBox::information(this, Message, tr("Software must be restarted for the changes to take effect!"), QMessageBox::Ok);
        if (pSettingsData) {
            if (index == 1)
                pSettingsData->m_WorkWithTwoValves = true;
            else
                pSettingsData->m_WorkWithTwoValves = false;
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void SerialPortSettingsDialog::SlotInterchangeSerialPort(int State)
{
    /*if (m_WindowSetup && GetMainAppCrystalT2())
    {
            SettingsData *pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
            if (pSettingsData)
            {
                    if (State == Qt::Checked)
                            pSettingsData->m_FirstPortIsFirst = false;
                    else
                            pSettingsData->m_FirstPortIsFirst = true;
                    GetMainAppCrystalT2()->SaveSettings();
                    GetMainAppCrystalT2()->ChangeValveOrder();
            }
    }*/
}

void SerialPortSettingsDialog::SlotInterchangeTriggerOutput(int State)
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            if (State == Qt::Checked)
                pSettingsData->m_RightTriggerIsFirst = false;
            else
                pSettingsData->m_RightTriggerIsFirst = true;
            GetMainAppCrystalT2()->SaveSettings();
            GetMainAppCrystalT2()->ChangeTriggerOutputOrder();  // set to real time task
        }
    }
}

void SerialPortSettingsDialog::showEvent(QShowEvent*)
{
    m_WindowSetup = false;
    if (GetMainAppCrystalT2()) {
        int index;
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        const QList<QSerialPortInfo> PortInfo = QSerialPortInfo::availablePorts();

        if (pSettingsData) {
            //Audittrail für comboboxen kurzzeitig ausschalten
            ui.comboBoxChooseNumberValves->setProperty(kAuditTrail, QVariant());
            ui.comboBoxSerialPortParity->setProperty(kAuditTrail, QVariant());
            ui.comboBoxSerialPortDatabits->setProperty(kAuditTrail, QVariant());
            ui.comboBoxSerialPortStopbits->setProperty(kAuditTrail, QVariant());
            ui.comboBoxSerialPortBaudrate->setProperty(kAuditTrail, QVariant());

            index = ui.comboBoxSerialPortBaudrate->findData(pSettingsData->m_BaudRate);
            ui.comboBoxSerialPortBaudrate->setCurrentIndex(index);

            index = ui.comboBoxSerialPortStopbits->findData(pSettingsData->m_StopBits);
            ui.comboBoxSerialPortStopbits->setCurrentIndex(index);

            index = ui.comboBoxSerialPortDatabits->findData(pSettingsData->m_DataBits);
            ui.comboBoxSerialPortDatabits->setCurrentIndex(index);

            index = ui.comboBoxSerialPortParity->findData(pSettingsData->m_Parity);
            ui.comboBoxSerialPortParity->setCurrentIndex(index);

            /*if (!pSettingsData->m_FirstPortIsFirst)
                    ui.checkBoxInterChangeSerialPort->setCheckState(Qt::Checked);
            else
                    ui.checkBoxInterChangeSerialPort->setCheckState(Qt::Unchecked);*/
            if (!pSettingsData->m_RightTriggerIsFirst)
                ui.checkBoxInterChangeTriggerOutput->setCheckState(Qt::Checked);
            else
                ui.checkBoxInterChangeTriggerOutput->setCheckState(Qt::Unchecked);

            ui.comboBoxChooseNumberValves->setProperty(kAuditTrail, ui.labelSetNumberValves->text());
            ui.comboBoxSerialPortParity->setProperty(kAuditTrail, ui.labelSerialPortParity->text());
            ui.comboBoxSerialPortDatabits->setProperty(kAuditTrail, ui.labelSerialPortDatabits->text());
            ui.comboBoxSerialPortStopbits->setProperty(kAuditTrail, ui.labelSerialPortStopbits->text());
            ui.comboBoxSerialPortBaudrate->setProperty(kAuditTrail, ui.labelSerialPortBaudrate->text());

        }

    }
    m_WindowSetup = true;
}

void SerialPortSettingsDialog::SlotParityChanged(int index)
{
    if (GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (m_WindowSetup && pSettingsData) {
            pSettingsData->m_Parity = ui.comboBoxSerialPortParity->itemData(index).toInt();
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void SerialPortSettingsDialog::SlotDataBitsChanged(int index)
{
    if (GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (m_WindowSetup && pSettingsData) {
            pSettingsData->m_DataBits = ui.comboBoxSerialPortDatabits->itemData(index).toInt();
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void SerialPortSettingsDialog::SlotStopBitsChanged(int index)
{
    if (GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (m_WindowSetup && pSettingsData) {
            pSettingsData->m_StopBits = ui.comboBoxSerialPortStopbits->itemData(index).toInt();
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void SerialPortSettingsDialog::SlotBaudrateChanged(int index)
{
    if (GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (m_WindowSetup && pSettingsData) {
            pSettingsData->m_BaudRate = ui.comboBoxSerialPortBaudrate->itemData(index).toInt();
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}
