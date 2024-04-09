#include "GeneralDialog.h"
#include "CrystalT2Plugin.h"
#include "GlobalConst.h"
#include "ImageData.h"
#include "LiveImageView.h"
#include "MainAppCrystalT2.h"
#include "ProductData.h"
#include "SettingsData.h"
#include "SimulateValve.h"
#include "ValveDialog.h"
#include "VideoDialog.h"
#include "bmessagebox.h"
#include "colors.h"
#include "contentboardvalueitem.h"

#include <audittrail.h>


GeneralDialog::GeneralDialog(MainAppCrystalT2* pMainAppCrystalT2) : QWidget(pMainAppCrystalT2), m_WindowsStartUp(false), m_TimerNoPowerSupplyValve(NULL)
{
    ui.setupUi(this);
   
    m_MainAppCrystalT2 = pMainAppCrystalT2;

    ui.comboBoxFormatIS->insertItem(0, tr("4(1/4)\""));
    ui.comboBoxFormatIS->insertItem(1, tr("5(1/4)\""));
    ui.comboBoxFormatIS->insertItem(2, tr("7\""));
    ui.comboBoxFormatIS->insertItem(3, tr("7(7/8)\""));
    ui.comboBoxFormatIS->insertItem(4, tr("8(1/2)\""));
    ui.comboBoxFormatIS->insertItem(5, tr("8(3/4)\""));
    ui.comboBoxFormatIS->insertItem(6, tr("9(5/8)\""));
    ui.comboBoxFormatIS->insertItem(7, tr("10(1/2)\""));
    ui.comboBoxFormatIS->insertItem(8, tr("11(13/16)\""));
    ui.comboBoxFormatIS->insertItem(9, tr("13(1/8)\""));
    ui.comboBoxFormatIS->insertItem(10, tr("14\""));

    ui.comboBoxBandDirectional->insertItem(0, tr("Left To Right"));
    ui.comboBoxBandDirectional->insertItem(1, tr("Right To Left"));

    connect(ui.doubleSpinBoxDefaultPreasure, &QDoubleSpinBox::editingFinished, this, &GeneralDialog::SlotDefaultPreasureChanged);
    connect(ui.doubleSpinBoxDistanceValveEjection, &QDoubleSpinBox::editingFinished, this, &GeneralDialog::SlotDistanceBottleEjectionChanged);
    connect(ui.comboBoxFormatIS, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GeneralDialog::SlotFormatISChanged);
    connect(ui.doubleSpinBoxPreasureTankDefaultTemp, &QDoubleSpinBox::editingFinished, this, &GeneralDialog::SlotPreasureTankDefaultTempChanged);
    connect(ui.doubleSpinBoxBlowOutLenght, &QDoubleSpinBox::editingFinished, this, &GeneralDialog::SlotBlowOutLenghtChanged);
    connect(ui.comboBoxBandDirectional, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GeneralDialog::SlotBandDirectionalChanged);
    connect(ui.pushButtonManualEject, &QPushButton::clicked, this, &GeneralDialog::SlotManualEject);
    connect(ui.doubleSpinBoxNumberEjectedBottlesByUser, &QDoubleSpinBox::editingFinished, this, &GeneralDialog::SlotNumberEjectedBottlesByUser);
    connect(ui.doubleSpinBoxDefaultHeatingPipe, &QDoubleSpinBox::editingFinished, this, &GeneralDialog::SlotDefaultHeatingPipeChanged);
    
    m_TimerNoPowerSupplyValve = new QTimer(this);
    connect(m_TimerNoPowerSupplyValve, &QTimer::timeout, this, &GeneralDialog::SlotAddMessageNoPowerSupplyValve);

    SetAuditTrailProperties();
    SetRequiredAccessLevel();
}

GeneralDialog::~GeneralDialog()
{
}

void GeneralDialog::SetRequiredAccessLevel()
{
    ui.groupBoxGeneraProductSettings->setProperty(kRequiredAccessLevel, kAccessLevelUser);
    ui.groupBoxEjectBottleSettings->setProperty(kRequiredAccessLevel, kAccessLevelUser);
    ui.groupBoxBottleEjectionTest->setProperty(kRequiredAccessLevel, kAccessLevelUser);
    ui.widgetDashBoardButtonStartManualEject->setProperty(kRequiredAccessLevel, kAccessLevelUser);
}

void GeneralDialog::SetAuditTrailProperties()
{
    ui.doubleSpinBoxDefaultPreasure->setProperty(kAuditTrail, ui.labelContentBoardDefaultPreasure->text());
    ui.doubleSpinBoxDistanceValveEjection->setProperty(kAuditTrail, ui.labelContentBoardDistanceValveEjection->text());
    ui.doubleSpinBoxPreasureTankDefaultTemp->setProperty(kAuditTrail, ui.labelContentBoardPreasureTankDefaultTemp->text());
    ui.doubleSpinBoxBlowOutLenght->setProperty(kAuditTrail, ui.labelContentBoardBlowOutLenght->text());
    ui.comboBoxBandDirectional->setProperty(kAuditTrail, ui.labelContentBoardBandDirectional->text());
    ui.pushButtonManualEject->setProperty(kAuditTrail, ui.pushButtonManualEject->text());
    ui.doubleSpinBoxNumberEjectedBottlesByUser->setProperty(kAuditTrail, ui.labelContentBoardNumberEjectedBottlesByUser->text());
    ui.comboBoxFormatIS->setProperty(kAuditTrail, ui.labelContentBoardFormatFromIS->text());
    ui.doubleSpinBoxDefaultHeatingPipe->setProperty(kAuditTrail, ui.labelContentBoardDefaultHeatingPipe->text());
}

void GeneralDialog::SlotManualEject()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) {
        ui.pushButtonManualEject->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::green.red()).arg(HMIColor::green.green()).arg(HMIColor::green.blue()));
        SlotNumberEjectedBottlesByUser();
        GetMainAppCrystalT2()->GetImageData()->SetButtonIsClickedEjectTheNextnBottles();
    }
}

void GeneralDialog::SlotNumberEjectedBottlesByUser()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) {
        int value = static_cast<int>(ui.doubleSpinBoxNumberEjectedBottlesByUser->value());
        GetMainAppCrystalT2()->GetImageData()->SetNumberEjectedBottlesByUser(value);
    }
}

void GeneralDialog::SetManualEjectionReady()
{
    ui.pushButtonManualEject->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::Grey.red()).arg(HMIColor::Grey.green()).arg(HMIColor::Grey.blue()));
}

void GeneralDialog::SlotAddMessageNoPowerSupplyValve()
{
    QString Text = tr("Error! No Valve Power Supply.");
    GetMainAppCrystalT2()->SlotAddNewMessage(Text, QtMsgType::QtFatalMsg);
}

//void GeneralDialog::SetPowerSupplyValveOff()
//{
//    if (GetMainAppCrystalT2()) {
//        if (GetMainAppCrystalT2()->GetValveDialogLeft()) GetMainAppCrystalT2()->GetValveDialogLeft()->StopSerialCommuniction();
//        if (GetMainAppCrystalT2()->GetValveDialogRight()) GetMainAppCrystalT2()->GetValveDialogRight()->StopSerialCommuniction();
//        GetMainAppCrystalT2()->SetDigitalOutputValue(EtherCATConfigData::SET_POWER_SUPPLY_VALVE, false);
//        if (GetMainAppCrystalT2()->GetSimulateValve()) GetMainAppCrystalT2()->GetSimulateValve()->KillProcess();
//    }
//}

void GeneralDialog::SetPowerSupplyValveOn()
{
    if (GetMainAppCrystalT2()) {
        GetMainAppCrystalT2()->GetCrystalT2Plugin()->reset();
        if (GetMainAppCrystalT2()->GetSimulateValve()) {
            GetMainAppCrystalT2()->GetSimulateValve()->StartProcess();
            GetMainAppCrystalT2()->GetSimulateValve()->WaitForStarted();
        }
        GetMainAppCrystalT2()->SetCurrentMaschineState(PluginInterface::MachineState::Initializing);
        GetMainAppCrystalT2()->SlotAddNewMessage(tr("Switch Valve Power Supply On And Set Valve Paramter. Please Wait...!"), QtMsgType::QtInfoMsg);
        GetMainAppCrystalT2()->InitSerialPortsAndSetParameter();
        GetMainAppCrystalT2()->SetCurrentMaschineState(PluginInterface::MachineState::Off);
    }
}

void GeneralDialog::SlotBlowOutLenghtChanged()
{
    if (m_WindowsStartUp && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) GetMainAppCrystalT2()->GetImageData()->SetProductWidth(ui.doubleSpinBoxBlowOutLenght->value());
}

void GeneralDialog::showEvent(QShowEvent* ev)
{
    m_WindowsStartUp = false;
    if (GetMainAppCrystalT2()) {
        if (GetMainAppCrystalT2()->GetCurrentProductData()) {
            ui.doubleSpinBoxDefaultPreasure->setValue(GetMainAppCrystalT2()->GetCurrentProductData()->m_DefaultPreasureInBar);
            ui.doubleSpinBoxPreasureTankDefaultTemp->setValue(GetMainAppCrystalT2()->GetCurrentProductData()->m_DefaultPreasureTankTemp);
            ui.doubleSpinBoxBlowOutLenght->setValue(GetMainAppCrystalT2()->GetCurrentProductData()->GetProductWidth());
            ui.doubleSpinBoxDefaultHeatingPipe->setValue(GetMainAppCrystalT2()->GetCurrentProductData()->m_DefaulHeatingPipeTemp);
        }
        ui.comboBoxFormatIS->setProperty(kAuditTrail, QVariant());
        if (GetMainAppCrystalT2()->GetCurrentProductData()->m_FormatFromISInmm == FOUR_AND_1T4_INCH)
            ui.comboBoxFormatIS->setCurrentIndex(0);
        else {
            if (GetMainAppCrystalT2()->GetCurrentProductData()->m_FormatFromISInmm == FIVE_AND_1T4_INCH)
                ui.comboBoxFormatIS->setCurrentIndex(1);
            else {
                if (GetMainAppCrystalT2()->GetCurrentProductData()->m_FormatFromISInmm == SEVEN_INCH)
                    ui.comboBoxFormatIS->setCurrentIndex(2);
                else {
                    if (GetMainAppCrystalT2()->GetCurrentProductData()->m_FormatFromISInmm == SEVEN_AND_7I8_INCH)
                        ui.comboBoxFormatIS->setCurrentIndex(3);
                    else {
                        if (GetMainAppCrystalT2()->GetCurrentProductData()->m_FormatFromISInmm == EIGHT_AND_1T2_INCH)
                            ui.comboBoxFormatIS->setCurrentIndex(4);
                        else {
                            if (GetMainAppCrystalT2()->GetCurrentProductData()->m_FormatFromISInmm == EIGHT_AND_3T4_INCH)
                                ui.comboBoxFormatIS->setCurrentIndex(5);
                            else {
                                if (GetMainAppCrystalT2()->GetCurrentProductData()->m_FormatFromISInmm == NINE_AND_5T8_INCH)
                                    ui.comboBoxFormatIS->setCurrentIndex(6);
                                else {
                                    if (GetMainAppCrystalT2()->GetCurrentProductData()->m_FormatFromISInmm == TEN_AND_1T2_INCH)
                                        ui.comboBoxFormatIS->setCurrentIndex(7);
                                    else {
                                        if (GetMainAppCrystalT2()->GetCurrentProductData()->m_FormatFromISInmm == ELEVEN_AND_13T16_INCH)
                                            ui.comboBoxFormatIS->setCurrentIndex(8);
                                        else {
                                            if (GetMainAppCrystalT2()->GetCurrentProductData()->m_FormatFromISInmm == THIRTEEN_AND_1T8_INCH)
                                                ui.comboBoxFormatIS->setCurrentIndex(9);
                                            else {
                                                if (GetMainAppCrystalT2()->GetCurrentProductData()->m_FormatFromISInmm == FOURTEENT_INCH) ui.comboBoxFormatIS->setCurrentIndex(10);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        ui.comboBoxFormatIS->setProperty(kAuditTrail, ui.labelContentBoardFormatFromIS->text());
        if (GetMainAppCrystalT2()->GetSettingsData()) {
            ui.doubleSpinBoxDistanceValveEjection->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_DistanceBottleEjectionInmm);
            ui.comboBoxBandDirectional->setProperty(kAuditTrail, QVariant());
            if (GetMainAppCrystalT2()->IsFirstTriggerOnLeftSide())
                ui.comboBoxBandDirectional->setCurrentIndex(0);
            else
                ui.comboBoxBandDirectional->setCurrentIndex(1);
            ui.comboBoxBandDirectional->setProperty(kAuditTrail, ui.labelContentBoardBandDirectional->text());
        }
    }
    m_WindowsStartUp = true;
}

void GeneralDialog::SlotDefaultHeatingPipeChanged()
{
    if (m_WindowsStartUp && GetMainAppCrystalT2()) {
        QString ErrorMsg;
        ProductData* pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
        if (pProductData) {
            pProductData->m_DefaulHeatingPipeTemp = ui.doubleSpinBoxDefaultHeatingPipe->value();
            pProductData->WriteProductData(ErrorMsg);  // save to productdata
        }
    }
}

void GeneralDialog::SlotBandDirectionalChanged(int index)
{
    if (m_WindowsStartUp && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        if (index == 0) {
            GetMainAppCrystalT2()->SetBandDirectional(BAND_DIRECTIONAL_LEFT_TO_RIGHT);  // Info to realtime task
            GetMainAppCrystalT2()->GetSettingsData()->m_BandDirectional = BAND_DIRECTIONAL_LEFT_TO_RIGHT;
        } else {
            GetMainAppCrystalT2()->SetBandDirectional(BAND_DIRECTIONAL_RIGHT_TO_LEFT);  // Info to realtime task
            GetMainAppCrystalT2()->GetSettingsData()->m_BandDirectional = BAND_DIRECTIONAL_RIGHT_TO_LEFT;
        }
        GetMainAppCrystalT2()->SaveSettings();
        GetMainAppCrystalT2()->MirrorMeasureWindows();
        GetMainAppCrystalT2()->GetLiveImageView()->DrawAllMeasureWindows();
        GetMainAppCrystalT2()->ChangeValveAndTriggerOrder();
    }
}

void GeneralDialog::SlotPreasureTankDefaultTempChanged()
{
    if (m_WindowsStartUp && GetMainAppCrystalT2()) {
        QString ErrorMsg;
        ProductData* pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
        if (pProductData) {
            pProductData->m_DefaultPreasureTankTemp = ui.doubleSpinBoxPreasureTankDefaultTemp->value();
            pProductData->WriteProductData(ErrorMsg);  // save to productdata
        }
    }
}

void GeneralDialog::SlotDefaultPreasureChanged()
{
    if (m_WindowsStartUp && GetMainAppCrystalT2()) {
        GetMainAppCrystalT2()->SetDefaultPreasure(ui.doubleSpinBoxDefaultPreasure->value(), true);  // Save to productdata and set to realtime task
    }
}

void GeneralDialog::SlotDistanceBottleEjectionChanged()
{
    if (m_WindowsStartUp && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_DistanceBottleEjectionInmm = ui.doubleSpinBoxDistanceValveEjection->value();
        GetMainAppCrystalT2()->SaveSettings();
        GetMainAppCrystalT2()->GetImageData()->SetDistanceBottleEjection(ui.doubleSpinBoxDistanceValveEjection->value());  // set to real time task
    }
}

void GeneralDialog::SlotFormatISChanged(int index)
{
    if (m_WindowsStartUp && GetMainAppCrystalT2()) {
        double FormatFromISInmm;
        switch (index) {
            case 0:
                FormatFromISInmm = FOUR_AND_1T4_INCH;
                break;
            case 1:
                FormatFromISInmm = FIVE_AND_1T4_INCH;
                break;
            case 2:
                FormatFromISInmm = SEVEN_INCH;
                break;
            case 3:
                FormatFromISInmm = SEVEN_AND_7I8_INCH;
                break;
            case 4:
                FormatFromISInmm = EIGHT_AND_1T2_INCH;
                break;
            case 5:
                FormatFromISInmm = EIGHT_AND_3T4_INCH;
                break;
            case 6:
                FormatFromISInmm = NINE_AND_5T8_INCH;
                break;
            case 7:
                FormatFromISInmm = TEN_AND_1T2_INCH;
                break;
            case 8:
                FormatFromISInmm = ELEVEN_AND_13T16_INCH;
                break;
            case 9:
                FormatFromISInmm = THIRTEEN_AND_1T8_INCH;
                break;
            case 10:
                FormatFromISInmm = FOURTEENT_INCH;
                break;
            default:
                FormatFromISInmm = FIVE_AND_1T4_INCH;
                break;
        }
        if (GetMainAppCrystalT2()->GetImageData()) GetMainAppCrystalT2()->GetImageData()->SetFormatFromISInmm(FormatFromISInmm);
    }
}
