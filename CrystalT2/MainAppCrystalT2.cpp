#include "MainAppCrystalT2.h"
#include <QtSerialPort/QSerialPort>
#include "../logger/loggerplugin.h"
#include "AdminSettingsDialog.h"
#include "AdvancedSettingsDialog.h"
#include "CleanImageDialog.h"
#include "CoolingDialog.h"
#include "CrystalT2Plugin.h"
#include "DeactivateAlarmLevelOptionPanel.h"
#include "EditProductDialog.h"
#include "EjectedBottlesDialog.h"
#include "FileTransferDialog.h"
#include "GeneralDialog.h"
#include "GlobalConst.h"
#include "IOSettingsDialog.h"
#include "ImageData.h"
#include "InputOutputEvents.h"
#include "KitharaCore.h"
#include "LiveImageView.h"
#include "MainTabWidget.h"
#include "MaintenanceDialog.h"
#include "MessageDialog.h"
#include "OverviewDialog.h"
#include "ProductData.h"
#include "ProductDialog.h"  //enthält PopupDialogProductDialog
#include "QtCore"
#include "QtSerialPort/qserialportinfo.h"
#include "QuickCouplingDialog.h"
#include "SaveVideoDialog.h"
#include "SelectTriggerPosOptionPanel.h"
#include "SerialPortSettingsDialog.h"
#include "SettingsData.h"
#include "SharedData.h"
#include "SimulateValve.h"
#include "TrendGraphOptionPanel.h"
#include "TrendGraphWidget.h"
#include "TrendLiquidData.h"
#include "ValveDialog.h"
#include "VideoDialog.h"
#include "WaitForNewVideo.h"

MainAppCrystalT2::MainAppCrystalT2(CrystalT2Plugin* pCrystalT2Plugin)
    : QWidget(),
      m_CrystalT2Plugin(NULL),
      m_OverviewDialog(NULL),
      m_SettingsData(NULL),
      m_ValveDialogLeft(NULL),
      m_ValveDialogRight(NULL),
      m_PopupDialogProductDialog(NULL),
      m_PopupDialogFileTransfer(NULL),
      m_KitharaCore(NULL),
      m_ImageData(NULL),
      m_TimerGetStatusHardwareDevice(NULL),
      m_TimerCheckCleanImage(NULL),
      m_TimerDrawTrendGraphData(NULL),
      m_TimerSaveAuditTrailData(NULL),
      m_WaitForNewVideo(NULL),
      m_InputOutputEvents(NULL),
      m_EditProductDialog(NULL),
      m_VideoDialogFullVideo(NULL),
      m_VideoDialogShowTriggerPos(NULL),
      m_VideoDialogShowProductVideos(NULL),
      m_LiveImageView(NULL),
      m_CleanImageDialog(NULL),
      m_GeneralDialog(NULL),
      m_CoolingDialog(NULL),
      m_ImageThirdLevelNavigationWidget(NULL),
      m_SettingsThirdLevelNavigationWidget(NULL),
      m_ParameterDialogThirdLevelNavigationWidget(NULL),
      m_SerialPortSettingsDialog(NULL),
      m_AdvancedSettingsDialog(NULL),
      m_IOSettingsDialog(NULL),
      m_LastStateErrorLight(false),
      m_SimulateValve(NULL),
      m_NumberSerialPortsFound(0),
      m_TimerCheckDiskOverflow(NULL),
      m_SelectTriggerPosOptionPanel(NULL),
      m_TrendGraphWidget(NULL),
      m_AdminSettingsDialog(NULL),
      m_StackTemparatureLeftValve(0.0),
      m_CurrentTemparatureLeftValve(0.0),
      m_StackTemparatureRightValve(0.0),
      m_CurrentTemparatureRightValve(0.0),
      m_TrendGraphOptionPanel(NULL),
      m_FirstCallSaveTrendDataAfterSoftwareStart(true),
      m_LastCallSaveTrendDataSoftwareFinished(false),
      m_TestCounter(0),
      m_TimerCalculatBottlesPerMinute(NULL),
      m_CurrentCounterProductOk(0),
      m_LastCounterProductOk(0),
      m_ProductsSumTime(NULL),
      m_ProductsSumCount(NULL),
      m_PreasureTooLowCounter(0),
      m_EjectedBottlesDialog(NULL),
      m_DeactivateAlarmLevelOptionPanel(NULL),
      m_SuppressAlarmWarinigPreasureLiquidTank(false),
      m_StatusCounterBottleEjectedOneAfterTheOther(ALARM_LEVEL_OK),
      m_StatusCounterMiddleTooLowOneAfterTheOther(ALARM_LEVEL_OK),
      m_StatusCurrentLiquidTankPreasure(ALARM_LEVEL_OK),
      m_BlueLightOn(true),
      m_MaintenanceDialog(NULL),
      m_PopupSaveVideoDialog(NULL),
      m_QuickCouplingDialog(NULL),
      m_IsPowerAndPreasureOnValve(false),
      m_StatusMixer(0),
      m_ENCStatusMixer(0),
      m_LastStateMixerAndPreasureOn(false),
      m_MixerIsRunning(false),
      m_ActualTemperaturWaterCoolingReturn(INVALID_TEMPERATURE_VALUE),
      m_NumberErrorMixer(0),
      m_LastControlDeviationWaterCooling(0.0),
      m_SumDeviationWaterCooling(0.0),
      m_CurrentWaterCoolingStrokeValue(0.0),
      m_EnablePIDControlWaterCooling(true),
      m_ActualVelocityMixerRPM(0.0)
{
    QString tes = "wete";
    qRegisterMetaType<ImageMetaData>();
    QString AppPath = QCoreApplication::applicationDirPath() + QString("/");
    m_CrystalT2Plugin = pCrystalT2Plugin;                                                // Elternklasse, hier das Plugin
    m_LastTimerMSecsSinceEpoch = QDateTime::currentDateTime().currentMSecsSinceEpoch();  // wird genutzt zur Bestimmung Flaschen/min
    m_SettingsData = new SettingsData(this);                                             // Datenstruktur enhaelt alle relevanten Konfigurationsparameter
    m_SettingsLocation = AppPath + APPLICATION_NAME + ".ini";                            // Allgemeine Programmeinstellungen werden Hier in Ini-Format gespeichert
    QDir dirAppPath(AppPath);
    QStringList IniFilesInRootPath = dirAppPath.entryList(QStringList() << QString("%1*.ini").arg(APPLICATION_NAME), QDir::Files);

    if (IniFilesInRootPath.count() > 0) {
        QString IniFileName = AppPath + QString("/") + IniFilesInRootPath[0];
        QFile IniFile(IniFileName);
        if (IniFile.exists()) {
            // für den Notfall wenn in der Registry keine Einträge vorhanden wird die Ini Datei geladen und danach gelöscht
            // Beim Programmende werden einmal alle Konfigurationseinstellung einmal gesichert
            QSettings settings(IniFileName, QSettings::IniFormat);
            GetSettings(settings, QString(""));
            IniFile.remove();  // danach  Datei löschen da die Einträge in der Registry gespeichert sind
            SaveSettings();    // into registry
        }
    } else {
        LoadSettings();
    }
    m_OverviewDialog = new OverviewDialog(this);  // Statusdialog. Zeigt aktuelle Prozessparameter und das Firmenlogo, ist nach erfolgreichen Start als Erstes sichtbar
    // AppPath + PATH_NMAE_ETHERCAT_XML_CONFIG;  // Speicherort fuer die Konfiguration der einzelnen Busklemmen im XML Format. Dateien sind von Beckhoff
    m_XMLCofigurationFileLocation = m_SettingsData->m_XMLCofigurationFileLocation;
    m_VideoFileLocation = m_SettingsData->m_VideoFilesLocation;           // "d:/VideoFiles"; //Speicherort fuer die Videodateien.
    m_TrendGraphDataLocation = m_SettingsData->m_TrendGraphDataLocation;  // "d:/ValveTemperatureData";//Datei in dem die Temperaturdaten des Ventils kontinuierlich gespeichert werden
    m_ErrorImagePoolLocation = m_SettingsData->m_ErrorImagePoolLocation;
    m_CleanImageLocation = m_SettingsData->m_CleanImageLocation;
    m_TriggerImagesFileLocation = m_SettingsData->m_TriggerImagesFileLocation;
    m_PathNameProducts = m_SettingsData->m_ProductDataLocation;  // Speicherort für die Produkspezifischen Daten
    m_AudiTrailDataLocation = m_SettingsData->m_AudiTrailDataLocation;
    m_AlarmMessageLocation = m_SettingsData->m_AlarmMessageLocation;
    m_BackupLocationRegistryData = m_SettingsData->m_BackupLocationRegistryData;
    m_ScreenshotsLocation = m_SettingsData->m_ScreenShotLocation;
    m_ProductsSumTime = new accumulator_set<qint64, stats<tag::rolling_sum> >(tag::rolling_window::window_size = m_SettingsData->m_TrendGraphRollingMeanSizeBottlesPerMin);
    m_ProductsSumCount = new accumulator_set<qint64, stats<tag::rolling_sum> >(tag::rolling_window::window_size = m_SettingsData->m_TrendGraphRollingMeanSizeBottlesPerMin);
    // verzeichnisse erstellen
    QDir().mkpath(m_ErrorImagePoolLocation);
    QDir().mkpath(m_PathNameProducts);
    QDir().mkdir(m_XMLCofigurationFileLocation);
    QDir().mkpath(m_VideoFileLocation);
    QDir().mkpath(m_CleanImageLocation);
    QDir().mkpath(m_TrendGraphDataLocation);
    QDir().mkpath(m_TriggerImagesFileLocation);
    QDir().mkpath(m_AudiTrailDataLocation);
    QDir().mkpath(m_AlarmMessageLocation);
    QDir().mkpath(m_BackupLocationRegistryData);
    QDir().mkpath(m_ScreenshotsLocation);

    // Enthält drei weitere Registerkarten m_EditProductDialog, m_VideoDialogFullVideo und m_VideoDialogLiveType
    m_ImageThirdLevelNavigationWidget = new MainTabWidget(this, QString("MainTabImageSettings"));
    // Enthält drei weitere Registerkarten m_SerialPortSettingsDialog, m_AdvancedSettingsDialog und m_IOSettingsDialog
    m_SettingsThirdLevelNavigationWidget = new MainTabWidget(this, QString("MainTabSettings"));
    // Enthält zwei weitere Registerkarten m_ValveDialogLeft und m_ValveDialogRight
    m_ParameterDialogThirdLevelNavigationWidget = new MainTabWidget(this, QString("MainTabValveSettings"));

    m_GeneralDialog = new GeneralDialog(this);
    m_PopupDialogProductDialog = new PopupDialogProductDialog(this, m_OverviewDialog);  // Produktdialog
    m_WaitForNewVideo = new WaitForNewVideo(this);
    m_VideoDialogFullVideo = new VideoDialog(this, VIDEO_DIALOG_SHOW_FULL_VIDEO);
    m_VideoDialogShowTriggerPos = new VideoDialog(this, VIDEO_DIALOG_SHOW_TRIGGER_POSITION);
    m_VideoDialogShowProductVideos = new VideoDialog(this, VIDEO_DIALOG_SHOW_PRODUCT_VIDEO);
    m_ValveDialogLeft = new ValveDialog(this, LEFT_VALVE_ID);
    m_ValveDialogRight = new ValveDialog(this, RIGHT_VALVE_ID);
    m_EditProductDialog = new EditProductDialog(this, m_OverviewDialog);
    m_TrendGraphWidget = new TrendGraphWidget(this);
    m_CleanImageDialog = new CleanImageDialog(this);
    m_AdminSettingsDialog = new AdminSettingsDialog(this);
    m_EjectedBottlesDialog = new EjectedBottlesDialog(this);
    m_PopupDialogFileTransfer = new PopupDialogFileTransfer(this, m_EjectedBottlesDialog);
    m_MaintenanceDialog = new MaintenanceDialog(this);
    m_QuickCouplingDialog = new QuickCouplingDialog(this);
    m_CoolingDialog = new CoolingDialog(this);

    m_PopupSaveVideoDialog = new PopupSaveVideoDialog(this, m_OverviewDialog);

    m_SelectTriggerPosOptionPanel = new SelectTriggerPosOptionPanel(this);
    m_TrendGraphOptionPanel = new TrendGraphOptionPanel(this);
    m_DeactivateAlarmLevelOptionPanel = new DeactivateAlarmLevelOptionPanel(this);

    m_KitharaCore = new KitharaCore(this);
    m_ImageData = new ImageData(this);
    m_InputOutputEvents = new InputOutputEvents(this);
    m_LiveImageView = new LiveImageView(this);

    m_TimerGetStatusHardwareDevice = new QTimer(this);
    m_TimerCheckCleanImage = new QTimer(this);
    m_TimerDrawTrendGraphData = new QTimer(this);
    m_TimerCheckDiskOverflow = new QTimer(this);
    m_TimerCalculatBottlesPerMinute = new QTimer(this);
    m_TimerSaveAuditTrailData = new QTimer(this);

    m_SerialPortSettingsDialog = new SerialPortSettingsDialog(this);
    m_AdvancedSettingsDialog = new AdvancedSettingsDialog(this);
    m_IOSettingsDialog = new IOSettingsDialog(this);

    connect(m_TimerDrawTrendGraphData, &QTimer::timeout, this, &MainAppCrystalT2::SlotDrawTrendGraphData);
    connect(m_TimerGetStatusHardwareDevice, &QTimer::timeout, this, &MainAppCrystalT2::SlotStatusHarwareDevice);
    connect(m_TimerCheckCleanImage, &QTimer::timeout, this, &MainAppCrystalT2::SlotCheckCleanImage);
    connect(m_TimerCheckDiskOverflow, &QTimer::timeout, this, &MainAppCrystalT2::SlotCheckDiskOverflow);
    connect(m_TimerCalculatBottlesPerMinute, &QTimer::timeout, this, &MainAppCrystalT2::SlotCalculteBottlesPerMinute);
    connect(m_TimerSaveAuditTrailData, &QTimer::timeout, this, &MainAppCrystalT2::SlotSaveAuditTrailData);

    connect(this, &MainAppCrystalT2::SignalShowInfo, this, &MainAppCrystalT2::SlotAddNewMessage);
    connect(this, &MainAppCrystalT2::SignalInitReady, this, &MainAppCrystalT2::SlotInitReady, Qt::QueuedConnection);

    connect(GetInputOutputEvents(), &InputOutputEvents::SignalShowDebugInfo, this, &MainAppCrystalT2::SlotAddNewDebugInfo);
    connect(GetInputOutputEvents(), &InputOutputEvents::SignalShowInfo, this, &MainAppCrystalT2::SlotIODeviceMessage);

    connect(GetImageData(), &ImageData::SignalShowLiveImage, this, &MainAppCrystalT2::SlotShowLiveImage);
    connect(GetImageData(), &ImageData::SignalShowInfo, this, &MainAppCrystalT2::SlotAddNewMessage);
    connect(GetImageData(), &ImageData::SignalShowCleanImage, this, &MainAppCrystalT2::SlotShowCleanImage);
    connect(GetImageData(), &ImageData::SignalShowDegreeOfPollution, this, &MainAppCrystalT2::SlotShowDegreeOfPollution);
    connect(GetImageData(), &ImageData::SignalSetDateTimeCleanImageIsSaved, this, &MainAppCrystalT2::SlotSetDateTimeCleanImageIsSaved);
    connect(GetImageData(), &ImageData::SignalSetCalibrateStatus, m_EditProductDialog, &EditProductDialog::SlotSetCalibrateStatus);

    connect(this, &MainAppCrystalT2::SignalStartupApplication, this, &MainAppCrystalT2::SlotStartupApplication);

    if (GetOverviewDialog()) {
        GetOverviewDialog()->AddMultiImageWidget(m_VideoDialogShowProductVideos);
    }

    m_ImageThirdLevelNavigationWidget->insertTab(0, m_EditProductDialog, tr("Live"));
    m_ImageThirdLevelNavigationWidget->insertTab(1, m_VideoDialogShowTriggerPos, tr("Trigger"));
    m_ImageThirdLevelNavigationWidget->insertTab(2, m_VideoDialogFullVideo, tr("Video"));

    m_SettingsThirdLevelNavigationWidget->insertTab(0, m_AdminSettingsDialog, tr("Alarm Settings"));
    m_SettingsThirdLevelNavigationWidget->insertTab(1, m_AdvancedSettingsDialog, tr("Image Processing"));
    m_SettingsThirdLevelNavigationWidget->insertTab(2, m_SerialPortSettingsDialog, tr("Valve Configuration"));
    m_SettingsThirdLevelNavigationWidget->insertTab(3, m_IOSettingsDialog, tr("IO Settings"));

    if (GetInputOutputEvents() && !GetInputOutputEvents()->isRunning()) GetInputOutputEvents()->start();

    if (GetEditProductDialog()) {
        GetEditProductDialog()->AddLiveImageWidget(m_LiveImageView);
        if (GetImageData()) {
            GetEditProductDialog()->SetLiveImageSize(GetImageData()->GetImageWidth(), GetImageData()->GetImageHeight());
            GetEditProductDialog()->SlotResetCounters();
        }
    }
    if (GetCleanImageDialog() && GetImageData()) {
        GetCleanImageDialog()->SetCreatedDateAndTimeCleanImage(GetImageData()->GetDateTimeCleanImage());
        GetCleanImageDialog()->SetLiveImageSize(GetImageData()->GetImageWidth(), GetImageData()->GetImageHeight());
    }
    if (GetImageData()) SlotShowCleanImage(GetImageData()->GetCleanImageQt());
    if (m_SettingsData->m_SimulateValve) {
        m_SimulateValve = new SimulateValve(this);
        QString data = "ACT = 030.5C\n STACK = 031.1C<3";
        SlotShowValveTemperature(data, LEFT_VALVE_ID);
        data = "ACT = 028.5C\n STACK = 027.1C<3";
        SlotShowValveTemperature(data, RIGHT_VALVE_ID);
    }
    QSettings settings;
    // Access level
    const QString kEnableAccessLevelService = "Inspector/EnableAccessLevelService";
    settings.setValue(kEnableAccessLevelService, false);

    const QString kEnableAccessLevelSysOp = "Inspector/EnableAccessLevelSysOp";
    settings.setValue(kEnableAccessLevelSysOp, false);

    SetRequiredAccessLevel();
}

MainAppCrystalT2::~MainAppCrystalT2()
{
    FreeAll();
}

void MainAppCrystalT2::OpenSaveTriggerImageManual(QImage& Image, const QPixmap& pixmap)
{
    // if (m_Pixmap && m_VideoDialog && m_VideoDialog->GetMainAppCrystalT2()) {
    //    PopupSaveVideoDialog* pDialog = m_VideoDialog->GetMainAppCrystalT2()->GetPopupSaveVideoDialog();
    //    if (pDialog) {
    //        pDialog->SetDefaultFileName(QString("Image1"));
    //        pDialog->SetDefaultFileExtention(QString(".bmp"));
    //        pDialog->setWindowTitle(tr("Save Image"));
    //        pDialog->SetSaveVideoFile(false);  // Save only a image not a video
    //        pDialog->SetImage(m_Pixmap->pixmap());
    //        pDialog->SetOriginalImage(m_LastImage);
    //        pDialog->show();
    //    }
    if (m_PopupSaveVideoDialog) {
        m_PopupSaveVideoDialog->SetDefaultFileName(QString("Image1"));
        m_PopupSaveVideoDialog->SetDefaultFileExtention(QString(".bmp"));
        m_PopupSaveVideoDialog->setWindowTitle(tr("Save Image"));
        m_PopupSaveVideoDialog->SetSaveVideoFile(false);  // Save only a image not a video
        m_PopupSaveVideoDialog->SetImage(pixmap);
        m_PopupSaveVideoDialog->SetOriginalImage(Image);
        m_PopupSaveVideoDialog->show();
    }
}

void MainAppCrystalT2::SaveFromRegistryToIniFile()
{
    QString CurrentDate = QDateTime::currentDateTime().date().toString();
    QString CurrentTime = QDateTime::currentDateTime().time().toString("hh mm ss zzz");
    QString orgName = QCoreApplication::organizationName();
    QString AppName = QCoreApplication::applicationName();
    QString RegeditLocation = QString("HKEY_CURRENT_USER\\SOFTWARE\\%1\\%2\\%3\\%4").arg(orgName).arg(AppName).arg(PLUGIN_DIR).arg(APPLICATION_NAME);
    QSettings settingReg(RegeditLocation, QSettings::NativeFormat);
    QStringList allgroups = settingReg.childGroups();
    QString IniFileNameWithDate = QString("/%1[%2 %3].ini").arg(APPLICATION_NAME).arg(CurrentDate).arg(CurrentTime);
    QString BackupLocationRegistryData = GetBackupLocationRegistryData() + IniFileNameWithDate;
    QSettings settings(BackupLocationRegistryData, QSettings::IniFormat);
    QDir dirBackUpLocation(GetBackupLocationRegistryData());
    int MaxEntries = 3;

    QStringList NumberFiles = dirBackUpLocation.entryList(QDir::NoDotAndDotDot | QDir::Files, QDir::Time | QDir::Reversed);
    if (NumberFiles.count() > MaxEntries) {
        QFile File(GetBackupLocationRegistryData() + QString("/") + NumberFiles[0]);
        File.remove();
    }
    for (int i = 0; i < allgroups.size(); i++) {
        settingReg.beginGroup(allgroups[i]);
        QStringList keys = settingReg.childKeys();
        for (int j = 0; j < keys.size(); j++) {
            QVariant value = settingReg.value(keys[j]);
            settings.setValue(QString("%1/%2").arg(allgroups[i]).arg(keys[j]), value);
        }
        settingReg.endGroup();
    }
}

void MainAppCrystalT2::SetRequiredAccessLevel()
{
    if (m_IOSettingsDialog) {
        m_IOSettingsDialog->setProperty(kRequiredAccessLevel, kAccessLevelBertram);
    }
    if (m_AdvancedSettingsDialog) {
        m_AdvancedSettingsDialog->setProperty(kRequiredAccessLevel, kAccessLevelBertram);
    }
    if (m_SerialPortSettingsDialog) {
        m_SerialPortSettingsDialog->setProperty(kRequiredAccessLevel, kAccessLevelBertram);
    }
    if (m_SelectTriggerPosOptionPanel) {
        m_SelectTriggerPosOptionPanel->setProperty(kRequiredAccessLevel, kAccessLevelUser);
    }
    if (m_TrendGraphOptionPanel) {
        m_TrendGraphOptionPanel->setProperty(kRequiredAccessLevel, kAccessLevelUser);
    }
    if (m_ValveDialogLeft) {
        m_ValveDialogLeft->setProperty(kRequiredAccessLevel, kAccessLevelAdmin);
    }
    if (m_ValveDialogRight) {
        m_ValveDialogRight->setProperty(kRequiredAccessLevel, kAccessLevelAdmin);
    }
    if (m_MaintenanceDialog) {
        m_MaintenanceDialog->setProperty(kRequiredAccessLevel, kAccessLevelUser);
    }
    if (m_QuickCouplingDialog) {
        m_QuickCouplingDialog->setProperty(kRequiredAccessLevel, kAccessLevelUser);
    }
    if (m_DeactivateAlarmLevelOptionPanel) {
        m_DeactivateAlarmLevelOptionPanel->setProperty(kRequiredAccessLevel, kAccessLevelBertram);
    }
}

void MainAppCrystalT2::SetDoubleSpinBoxBackgroundColor(QDoubleSpinBox* spinBox, const QString& state)
{
    spinBox->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), state);
    spinBox->style()->unpolish(spinBox);
    spinBox->style()->polish(spinBox);
}

double MainAppCrystalT2::GetProductsPerMin()
{
    qint64 products_sum_count = rolling_sum(*m_ProductsSumCount);
    qint64 products_sum_time = rolling_sum(*m_ProductsSumTime);

    if (products_sum_count > 0 && products_sum_time > 0) {
        return products_sum_count * 60.0 / (products_sum_time / 1000.0);
    } else {
        return 0.0;
    }
}

void MainAppCrystalT2::SlotCalculteBottlesPerMinute()
{
    if (m_LastCounterProductOk == 0) {
        m_LastCounterProductOk = m_CurrentCounterProductOk;
        m_LastTimerMSecsSinceEpoch = QDateTime::currentDateTime().currentMSecsSinceEpoch();
    } else {
        if (m_LastCounterProductOk > m_CurrentCounterProductOk) {
            m_LastCounterProductOk = m_CurrentCounterProductOk;
            m_LastTimerMSecsSinceEpoch = QDateTime::currentDateTime().currentMSecsSinceEpoch();
        } else {
            qint64 currentMSecsSinceEpoch = QDateTime::currentDateTime().currentMSecsSinceEpoch();
            qint64 DeltaT = currentMSecsSinceEpoch - m_LastTimerMSecsSinceEpoch;
            qint64 DeltaProductOk = m_CurrentCounterProductOk - m_LastCounterProductOk;

            if (m_ProductsSumTime != NULL && m_ProductsSumCount != NULL) {
                m_ProductsSumTime->operator()(DeltaT);
                m_ProductsSumCount->operator()(DeltaProductOk);
            }
            m_LastCounterProductOk = m_CurrentCounterProductOk;
            m_LastTimerMSecsSinceEpoch = currentMSecsSinceEpoch;
        }
    }
}

bool MainAppCrystalT2::IsDialogShowProductVideosVisible()
{
    if (GetVideoDialogShowProductVideos())
        return GetVideoDialogShowProductVideos()->isVisible();
    else
        return false;
}

int MainAppCrystalT2::CurrentAccessLevel()
{
    if (GetCrystalT2Plugin()) {
        return GetCrystalT2Plugin()->GetCurrentAccessLevel();
    } else
        return 0;
}

void MainAppCrystalT2::ShowOptionPanelImageTab(bool Show)
{
    if (GetSelectTriggerPosOptionPanel()) {
        GetSelectTriggerPosOptionPanel()->show();
        GetSelectTriggerPosOptionPanel()->ShowRow(0, Show);
    }
}

void MainAppCrystalT2::ResetIsClicked()
{
    SettingsData* pSettingsData = GetSettingsData();
    if (pSettingsData) {
        if (pSettingsData->m_WorkWithTwoValves) {
            if (GetValveDialogLeft()) GetValveDialogLeft()->ResetIsClicked();
            if (GetValveDialogRight()) GetValveDialogRight()->ResetIsClicked();
        } else {
            if (GetValveDialogLeft()) GetValveDialogLeft()->ResetIsClicked();
        }
    }
    if (GetImageData()) {
        // in der real time task werden die Zähler für Flaschenauswurf und Füllmenge zu gering zurückgesetzt
        GetImageData()->ResetCountersBottlesEjectionAndLiquidTooLow();
    }
    //
    //  //Wenn Fehler Reset gedrückt wird dann Alle Zähler zurücksetzen
    //  if (GetEditProductDialog()) {
    //    GetEditProductDialog()->ResetIsClicked();
    //}
    SetCurrentMaschineStateToRealTime(GetCurrentMaschineState());
    SetErrorLight(false);
    SetWarningLight(false);
}

void MainAppCrystalT2::SetPiezoCurrentOnOff(int Value)
{
    SettingsData* pSettingsData = GetSettingsData();
    if (pSettingsData) {
        if (pSettingsData->m_WorkWithTwoValves) {
            if (GetValveDialogLeft()) GetValveDialogLeft()->WriteValveParameterPiezoCurrentOnOff(Value);
            if (GetValveDialogRight()) GetValveDialogRight()->WriteValveParameterPiezoCurrentOnOff(Value);
        } else {
            if (GetValveDialogLeft()) GetValveDialogLeft()->WriteValveParameterPiezoCurrentOnOff(Value);
        }
    }
}

bool MainAppCrystalT2::WorkWithTwoValves()
{
    bool rv = true;

    SettingsData* pSettingsData = GetSettingsData();
    if (pSettingsData) {
        if (!pSettingsData->m_WorkWithTwoValves) rv = false;
    }
    if (rv) {
        ProductData* pProductData = GetCurrentProductData();
        if (pProductData) {
            if (pProductData->m_UsedTriggerOutputs == USE_ONLY_LEFT_VALVE || pProductData->m_UsedTriggerOutputs == USE_ONLY_RIGHT_VALVE) rv = false;
        }
    }
    return rv;
}

void MainAppCrystalT2::FreeAll()
{
    if (GetSimulateValve()) {
        delete m_SimulateValve;
        m_SimulateValve = NULL;
    }
    if (GetWaitForNewVideo()) {
        delete m_WaitForNewVideo;
        m_WaitForNewVideo = NULL;
    }
    if (GetInputOutputEvents()) {
        delete m_InputOutputEvents;
        m_InputOutputEvents = NULL;
    }
    if (GetSettingsData()) {
        delete m_SettingsData;
        m_SettingsData = NULL;
    }
    if (GetImageData()) {
        delete m_ImageData;
        m_ImageData = NULL;
    }
    if (GetKitharaCore()) {
        delete m_KitharaCore;
        m_KitharaCore = NULL;
    }
}

void MainAppCrystalT2::SetDigitalOutputValue(EtherCATConfigData::IOChannels ChannelIndex, bool State)
{
    if (GetKitharaCore()) GetKitharaCore()->SetDigitalOutput(ChannelIndex, State);  // value to real time task
}

void MainAppCrystalT2::TriggerGetNewVideoFromRealTimeContext()
{
    if (GetImageData()) GetImageData()->TriggerGetNewVideoFromRealTimeContext();
}

void MainAppCrystalT2::SetTriggerOffsetInmm(double set, int ValveID)
{
    if (GetImageData()) GetImageData()->SetTriggerOffsetInmm(set, ValveID);
}

void MainAppCrystalT2::OpenProductDialog()
{
    if (GetPopupDialogProductDialog()) {
        GetPopupDialogProductDialog()->show();
        GetPopupDialogProductDialog()->setWindowTitle(tr("Open Product"));
    }
}

QString MainAppCrystalT2::GetCurrentProductName()
{
    if (GetSettingsData())
        return GetSettingsData()->m_CurrentProductName;
    else
        return QString("");
}

void MainAppCrystalT2::ShowAndSetCurrentProductName(const QString& Name)
{
    if (GetSettingsData()) GetSettingsData()->m_CurrentProductName = Name;
    if (GetCrystalT2Plugin()) GetCrystalT2Plugin()->SetCurrentProductName(Name);
    SaveSettings();

    GenerateTrendGraphTemperatureFileName();
    GenerateTrendGraphLiquidFileName();
}

bool MainAppCrystalT2::ExistProduct(QString& ProductName)
{
    bool rv = false;
    for (int i = 0; i < m_ListProducts.count(); i++) {
        if (m_ListProducts.at(i)->GetProductName() == ProductName) {
            rv = true;
            break;
        }
    }
    return rv;
}

void MainAppCrystalT2::RemoveProduct(QString& ProductName)
{
    QString ErrorMsg, PathAndFileName = m_PathNameProducts + QString("/") + ProductName + QString(".dat");
    QFile RemovedFile(PathAndFileName);
    int rv = ERROR_CODE_NO_ERROR;

    // remove from disk
    RemovedFile.remove();
    // remove from list
    for (int i = 0; i < m_ListProducts.count(); i++) {
        if (m_ListProducts.at(i)->GetProductName() == ProductName) {
            delete m_ListProducts.at(i);
            m_ListProducts.removeAt(i);
            break;
        }
    }
}

int MainAppCrystalT2::WriteAndInsertNewProduct(const QString& ProductName, const QString& CopyFromProductName, QString& ErrorMsg)
{
    ProductData* pProductData = new ProductData(this);
    ProductData* pCopyProductData = GetProductByProductName(CopyFromProductName);
    int rv;

    if (pCopyProductData) *pProductData = *pCopyProductData;
    pProductData->SetProductName(ProductName);
    rv = pProductData->WriteProductData(ErrorMsg);
    if (rv == ERROR_CODE_NO_ERROR) {
        m_ListProducts.insert(0, pProductData);
    } else {
        delete pProductData;
    }
    return rv;
}

ProductData* MainAppCrystalT2::GetProductByProductName(const QString& Name)
{
    for (int i = 0; i < m_ListProducts.count(); i++) {
        if (m_ListProducts.at(i)->GetProductName() == Name) {
            return m_ListProducts.at(i);
        }
    }
    return NULL;
}

int MainAppCrystalT2::ActivateCurrentProduct(const QString& ProductName, QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    ShowAndSetCurrentProductName(ProductName);     // set product name on Statusbar and Overview Dialog
    SetCurrentProductDataToImageProcessingUnit();  //
    if (GetImageData()) {
        int offsetX = GetImageData()->GetImageOffsetX();
        int offsetY = GetImageData()->GetImageOffsetY();
        // qDebug() << QString("ProducktName:%1 Offsetx:%2  Offsety:%3").arg(ProductName).arg(offsetX).arg(offsetY);
        // set camera ROI offsets
        rv = GetImageData()->SetCameraXOffset(offsetX, ErrorMsg);
        if (rv != ERROR_CODE_NO_ERROR) {
            SlotAddNewMessage(ErrorMsg, QtMsgType::QtFatalMsg);
        }
        rv = GetImageData()->SetCameraYOffset(offsetY, ErrorMsg);
        if (rv != ERROR_CODE_NO_ERROR) {
            SlotAddNewMessage(ErrorMsg, QtMsgType::QtFatalMsg);
        }
    }
    rv = SetCurrentProductDataToValveDevice(ErrorMsg);
    if (GetEditProductDialog() && GetEditProductDialog()->isVisible()) {
        GetEditProductDialog()->SetupWindow();
        if (GetLiveImageView()) {
            GetLiveImageView()->DrawAllMeasureWindows();
        }
    }
    return rv;
}

void MainAppCrystalT2::SetCurrentProductDataToImageProcessingUnit()
{
    if (GetImageData()) GetImageData()->SetCurrentProductDataToImageProcessingUnit();
}

// Wird beim Start der Software oder wenn ein eues Produkt geladen wird, aufgerufen
int MainAppCrystalT2::SetCurrentProductDataToValveDevice(QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    // Alle Produktabhängige Ventilparameter zum Dosierventil
    if (GetValveDialogLeft()) {
        rv = GetValveDialogLeft()->SetAllValveParametersToDevice(ErrorMsg);
    }
    if (GetValveDialogRight()) {
        rv = GetValveDialogRight()->SetAllValveParametersToDevice(ErrorMsg);
    }
    return rv;
}

int MainAppCrystalT2::RenameAndActivateProduct(QString& OldName, QString& NewName, QString& ErrorMsg)
{
    int retVal = ERROR_CODE_NO_ERROR;
    QFileInfoList FileInfolist = GetProductFileInfoList();
    bool NewNameExist = false;

    for (int i = 0; i < FileInfolist.count(); i++) {
        if (FileInfolist.at(i).baseName() == NewName) {
            NewNameExist = true;
            break;
        }
    }
    if (!NewNameExist) {
        QString PathAndFileName = m_PathNameProducts + QString("/") + OldName + QString(".dat");
        QString PathAndFileNewName = m_PathNameProducts + QString("/") + NewName + QString(".dat");
        QFile RenamedFile(PathAndFileName);

        RenamedFile.rename(PathAndFileNewName);
        if (OldName == GetCurrentProductName()) ShowAndSetCurrentProductName(NewName);
        LoadAllProductFiles(ErrorMsg);
        ActivateCurrentProduct(GetCurrentProductName(), ErrorMsg);
        if (GetPopupDialogProductDialog() && GetPopupDialogProductDialog()->GetProductDialog()) GetPopupDialogProductDialog()->GetProductDialog()->UpdateProductList();
    }
    return retVal;
}

int MainAppCrystalT2::LoadAllProductFiles(QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    QString BaseName;
    QFileInfoList list = GetProductFileInfoList();
    bool ListIncludesCurrentProduct = false;

    ClearProductList();
    for (int i = 0; i < list.count(); i++) {
        BaseName = list.at(i).baseName();
        rv = ReadAndAppendProduct(BaseName, ErrorMsg);
    }
    if (list.count() == 0) {
        ErrorMsg = tr("No Products Defined");
        rv = ERROR_CODE_NO_PRODUCTS;
    } else {
        QString CurrentProduct = GetCurrentProductName();
        for (int i = 0; i < list.count(); i++) {
            BaseName = list.at(i).baseName();
            if (CurrentProduct == BaseName) ListIncludesCurrentProduct = true;
        }
        if (!ListIncludesCurrentProduct) {
            CurrentProduct = list.at(0).baseName();
            ShowAndSetCurrentProductName(CurrentProduct);
        }
    }
    return rv;
}

void MainAppCrystalT2::ClearProductList()
{
    for (int i = 0; i < m_ListProducts.count(); i++) delete m_ListProducts.at(i);
    m_ListProducts.clear();
}

int MainAppCrystalT2::ReadAndAppendProduct(QString& ProductName, QString& ErrorMsg)
{
    ProductData* pProductData = new ProductData(this);
    int rv;

    pProductData->SetProductName(ProductName);
    rv = pProductData->ReadProductData(ErrorMsg);
    if (rv == ERROR_CODE_NO_ERROR)
        m_ListProducts.append(pProductData);
    else
        delete pProductData;
    return rv;
}

QFileInfoList MainAppCrystalT2::GetProductFileInfoList()
{
    QDir Path(m_PathNameProducts);
    QStringList filters;

    filters << "*.dat";
    Path.setFilter(QDir::Files);
    Path.setNameFilters(filters);
    Path.setSorting(QDir::Time);
    return Path.entryInfoList();
}

ProductData* MainAppCrystalT2::GetCurrentProductData()
{
    if (GetSettingsData()) {
        for (int i = 0; i < m_ListProducts.count(); i++) {
            if (m_ListProducts.at(i)->GetProductName() == GetSettingsData()->m_CurrentProductName) {
                return m_ListProducts.at(i);
            }
        }
    }
    return NULL;
}

// Anzeigen unterschiedlicher Fehlertexte enum QtMsgType { QtDebugMsg, QtWarningMsg, QtCriticalMsg, QtFatalMsg, QtInfoMsg, QtSystemMsg = QtCriticalMsg };
void MainAppCrystalT2::SlotAddNewMessage(const QString& Msg, QtMsgType MsgType)
{
    if (GetCrystalT2Plugin()) {
        GetCrystalT2Plugin()->SetMessage(Msg, MsgType);  // Info an die übergeordnete Instanz
    }
    if (MsgType == QtMsgType::QtFatalMsg) {
        SetCurrentMaschineStateToRealTime(PluginInterface::MachineState::Error);
        SetErrorLight(true);
    }
}

void MainAppCrystalT2::SlotAddNewDebugInfo(const QString& Msg, int InfoCode)
{
    SlotAddNewMessage(Msg, QtMsgType::QtInfoMsg);
}

// const int OPERATION_STATUS_UNKNOWN = -1;
// const int OPERATION_STATUS_RELEASE_PRODUCTION = 0; //Maschnie ist im Produktionsmodus, Trigger und Flaschenauswurf sind aktiv
// const int OPERATION_STATUS_STARTUP = 1; //Software Startet gerade
// const int OPERATION_STATUS_SETUP = 2; //Trigger löst nicht aus und Flaschenauswurf ist immer an
// const int OPERATION_STATUS_SETUP_ENABLE_TRIGGER = 3; //trigger kann auslösen und Flaschenauswuerf ist immer an
// const int OPERATION_STATUS_ERROR = 4; //Maschine ist im Fehlerzustand, Trigger löst nicht aus und Flaschenauswurf ist immer an
void MainAppCrystalT2::SetCurrentMaschineState(PluginInterface::MachineState set)
{
    int KitharaMaschineState;

    if (GetCrystalT2Plugin()) GetCrystalT2Plugin()->SetCurrentMaschineState(set);
    if (GetLiveImageView()) GetLiveImageView()->SetCurrentMaschineState(set);  // Messfenster-Positionierung sperren oder freigeben
    if (GetEditProductDialog()) GetEditProductDialog()->SetCurrentMaschineState(set);
    // if (GetGeneralDialog()) GetGeneralDialog()->SetCurrentMaschineState(set);

    if (GetKitharaCore()) {
        if (set == PluginInterface::MachineState::Off) {
            ISMaschineEjectAllBottles(true);
            // SetPowerAndPreasureOnTheValves(false);
            SetPreasureOnOff(false);

        } else {
            ISMaschineEjectAllBottles(false);
            if (set == PluginInterface::MachineState::Production || set == PluginInterface::MachineState::Setup) {
                // SetPowerAndPreasureOnTheValves(true);
                SetPreasureOnOff(true);
            }
        }
        if (GetCleanImageDialog() && GetCleanImageDialog()->isVisible()) {
            GetCleanImageDialog()->ChangeButtonColorEjectBottles();
        }
    }
    SetCurrentMaschineStateToRealTime(set);
}

void MainAppCrystalT2::SetPowerAndPreasureOnTheValves(bool on)
{
    if (GetKitharaCore()) {
        if (on) {
            SetPiezoCurrentOnOff(VALVE_PIEZO_CURRENT_ON);
            QTimer::singleShot(1000, this, &MainAppCrystalT2::SlotDelaySetValvePreasureOn);  // Verzögerung damit keine Flüssigkeit aus dem Ventil tritt
        } else {
            // GetKitharaCore()->SetDigitalOutput(EtherCATConfigData::DO_PREASURE_TANK_VALVE, false);  // Druck auf das Ventil ausschalten
            SetPreasureOnOff(false);
            // spielt hier eigentlich keine Rolle, da der Druck nach dem stromlos Schalten noch lange ansteht
            // und die Flüssigket noch aus dem Ventil fließt
            QTimer::singleShot(1000, this, &MainAppCrystalT2::SlotDelaySetPiezoCurrentOff);
        }
        SetAirCoolingValvesAndWaterCooling(on);
    }
}

void MainAppCrystalT2::SetAirCoolingValvesAndWaterCooling(bool on)
{
    if (on) {
        SetAirCoolingValve();
        m_EnablePIDControlWaterCooling = true;
    } else {
        if (GetImageData()) {
            GetImageData()->SetAirCoolingValve(0.0);
        }
        SetWaterCoolingDefault(GetSettingsData()->m_WaterCoolingStrokeMinValue);
        m_EnablePIDControlWaterCooling = false;
    }
}

void MainAppCrystalT2::SlotDelaySetValvePreasureOn()
{
    SetPreasureOnOff(true);
}

void MainAppCrystalT2::SlotDelaySetPiezoCurrentOff()
{
    SetPiezoCurrentOnOff(VALVE_PIEZO_CURRENT_OFF);
}

void MainAppCrystalT2::ISMaschineEjectAllBottles(bool eject)
{
    if (GetKitharaCore()) {
        if (eject) {
            GetKitharaCore()->SetDigitalOutput(EtherCATConfigData::DO_EJECTION_BY_IS_MASCHINE, false);  // is normally closed(IS Maschine soll alles auswerfen)
            GetKitharaCore()->SetDigitalOutput(EtherCATConfigData::DO_BLUE_LIGHT, true);
            m_BlueLightOn = true;
        } else {
            GetKitharaCore()->SetDigitalOutput(EtherCATConfigData::DO_EJECTION_BY_IS_MASCHINE, true);  // is normally closed(IS Maschine soll alles durchlassen)
            GetKitharaCore()->SetDigitalOutput(EtherCATConfigData::DO_BLUE_LIGHT, false);
            m_BlueLightOn = false;
        }
    }
}

void MainAppCrystalT2::SetCurrentMaschineStateToRealTime(PluginInterface::MachineState set)
{
    int KitharaMaschineState;
    switch (set) {
        case PluginInterface::MachineState::Production:  // Wenn Produktion gesetzt dann Anzahl der Flaschen/min neu berechnen
            KitharaMaschineState = OPERATION_STATUS_RELEASE_PRODUCTION;
            // qDebug() << "OPERATION_STATUS_RELEASE_PRODUCTION";
            break;
        case PluginInterface::MachineState::Error:
            KitharaMaschineState = OPERATION_STATUS_ERROR;
            // qDebug() << "OPERATION_STATUS_ERROR";
            break;
        case PluginInterface::MachineState::Off:
            KitharaMaschineState = OPERATION_STATUS_SETUP_DISABLE_TRIGGER;
            // qDebug() << "OPERATION_STATUS_SETUP_DISABLE_TRIGGER";
            break;
        case PluginInterface::MachineState::Setup:
            KitharaMaschineState = OPERATION_STATUS_SETUP_ENABLE_TRIGGER;
            // qDebug() << "OPERATION_STATUS_SETUP_ENABLE_TRIGGER";
            break;
        case PluginInterface::MachineState::Initializing:
            KitharaMaschineState = OPERATION_STATUS_SETUP_DISABLE_TRIGGER;
            // qDebug() << "OPERATION_STATUS_SETUP_DISABLE_TRIGGER";
            break;
        case PluginInterface::MachineState::Stopping:
            KitharaMaschineState = OPERATION_STATUS_SETUP_DISABLE_TRIGGER;
            // qDebug() << "OPERATION_STATUS_SETUP_DISABLE_TRIGGER";
            break;
        case PluginInterface::MachineState::Starting:
            KitharaMaschineState = OPERATION_STATUS_SETUP_DISABLE_TRIGGER;
            // qDebug() << "OPERATION_STATUS_SETUP_DISABLE_TRIGGER";
            break;
        default:
            KitharaMaschineState = OPERATION_STATUS_SETUP_DISABLE_TRIGGER;
            // qDebug() << "DEFAULT OPERATION_STATUS_SETUP_DISABLE_TRIGGER";
            break;
    }
    if (GetKitharaCore()) {
        GetKitharaCore()->SetCurrentMaschineState(KitharaMaschineState);  // info an real time task
    }
}

int MainAppCrystalT2::GetInfoLevel()
{
    if (GetImageData()) {
        return GetImageData()->GetInfoLevel();
    } else {
        return INFO_LEVEL_OFF;
    }
}

PluginInterface::MachineState MainAppCrystalT2::GetCurrentMaschineState()
{
    if (GetCrystalT2Plugin())
        return GetCrystalT2Plugin()->GetCurrentMaschineState();
    else
        return PluginInterface::MachineState::Off;  // tritt nie auf da PlugIn Instanz immer vorhanden
}

void MainAppCrystalT2::SetErrorLight(bool on)
{
    if (GetKitharaCore()) GetKitharaCore()->SetDigitalOutput(EtherCATConfigData::DO_ERROR_LIGHT, on);
}

void MainAppCrystalT2::SetWarningLight(bool on)
{
    if (GetKitharaCore()) GetKitharaCore()->SetDigitalOutput(EtherCATConfigData::DO_ORANGE_LIGHT, on);
}

void MainAppCrystalT2::SetErrorTransfer(bool on)
{
    if (GetKitharaCore()) GetKitharaCore()->SetDigitalOutput(EtherCATConfigData::DO_ERROR_TRANSFER, on);
}

int MainAppCrystalT2::WriteLogFile(const QString& data, const QString& FileName)
{
    int rv = ERROR_CODE_NO_ERROR;
    QString LogFileLocation = "d:/TestLog/";
    QDir().mkdir(LogFileLocation);
    QString PathAndName = LogFileLocation + FileName;
    QString ErrorMsg;
    QFile file(PathAndName);
    QString Time = QDateTime::currentDateTime().time().toString("hh:mm:ss.zzz");
    QString Date = QDateTime::currentDateTime().date().toString("dd.MM.yyyy");
    QFileInfo FileInfo(PathAndName);
    int maxFileSize = 100000;

    if (FileInfo.size() > maxFileSize) file.remove();
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream os(&file);
        os << Date << " " << Time << " " << data << "\r\n";
        file.close();
    } else {
        ErrorMsg = tr("Error! Can Not Open File: %1").arg(PathAndName);
        rv = ERROR_CODE_ANY_ERROR;
    }
    return rv;
}

void MainAppCrystalT2::LoadSettings(bool LoadFromReistry)
{
    QString RegeditDir = REGEDIT_DIR + QString("/");
    if (!LoadFromReistry) {
        // load ini-file from root program path
        RegeditDir = "";
        QString PathNameProducts = m_PathNameProducts + QString("/") + APPLICATION_NAME + ".ini";
        QSettings settings(PathNameProducts, QSettings::IniFormat);
        GetSettings(settings, RegeditDir);
    } else {
        QSettings settings;
        GetSettings(settings, RegeditDir);
    }
}

void MainAppCrystalT2::GetSettings(QSettings& settings, QString& RegeditDir)
{
    QString SectionName;
    SectionName = RegeditDir + "IODeviceSettings";
    m_SettingsData->m_CycleTimeIOTask = settings.value(QString("%1/CycleTimeIOTask").arg(SectionName), m_SettingsData->m_CycleTimeIOTask).toDouble();
    m_SettingsData->m_TimePeriodTriggerOutputOnInms = settings.value(QString("%1/TimePeriodTriggerOutputOnInms").arg(SectionName), m_SettingsData->m_TimePeriodTriggerOutputOnInms).toDouble();
    m_SettingsData->m_TimePeriodDigitalOutputOnInms = settings.value(QString("%1/TimePeriodDigitalOutputOnInms").arg(SectionName), m_SettingsData->m_TimePeriodDigitalOutputOnInms).toDouble();
    m_SettingsData->m_ExposureTime = settings.value(QString("%1/ExposureTime").arg(SectionName), m_SettingsData->m_ExposureTime).toDouble();
    SectionName = RegeditDir + "SerialPortSettings";
    m_SettingsData->m_ValveControllerPortName1 = settings.value(QString("%1/ValveControllerPortName1").arg(SectionName), m_SettingsData->m_ValveControllerPortName1).toString();
    m_SettingsData->m_ValveControllerPortName2 = settings.value(QString("%1/ValveControllerPortName2").arg(SectionName), m_SettingsData->m_ValveControllerPortName2).toString();
    m_SettingsData->m_BaudRate = settings.value(QString("%1/SerialPortBaudRate").arg(SectionName), m_SettingsData->m_BaudRate).toInt();
    m_SettingsData->m_Parity = settings.value(QString("%1/SerialPortParity").arg(SectionName), m_SettingsData->m_Parity).toInt();
    m_SettingsData->m_DataBits = settings.value(QString("%1/SerialPortDataBits").arg(SectionName), m_SettingsData->m_DataBits).toInt();
    m_SettingsData->m_StopBits = settings.value(QString("%1/SerialPortStopBits").arg(SectionName), m_SettingsData->m_StopBits).toInt();
    m_SettingsData->m_PollingTimeReadStatusValveInms = settings.value(QString("%1/PollingTimeReadStatusValveInms").arg(SectionName), m_SettingsData->m_PollingTimeReadStatusValveInms).toInt();
    m_SettingsData->m_TimeOutValueSeriellerPort = settings.value(QString("%1/TimeOutValueSeriellerPort").arg(SectionName), m_SettingsData->m_TimeOutValueSeriellerPort).toInt();
    SectionName = RegeditDir + "VideoProductSettings";
    m_SettingsData->m_CurrentProductName = settings.value(QString("%1/ProductName").arg(SectionName), m_SettingsData->m_CurrentProductName).toString();
    m_SettingsData->m_MaxTriggerImagesOnScreen = settings.value(QString("%1/MaxTriggerImagesOnScreen").arg(SectionName), m_SettingsData->m_MaxTriggerImagesOnScreen).toInt();
    // m_SettingsData->m_VideoSaveConditionFlag = settings.value(QString("%1/VideoSaveConditionFlag").arg(SectionName), m_SettingsData->m_VideoSaveConditionFlag).toInt();
    m_SettingsData->m_VideoFileNameCameraSimulation = settings.value(QString("%1/VideoFileNameCameraSimulation").arg(SectionName), m_SettingsData->m_VideoFileNameCameraSimulation).toString();
    m_SettingsData->m_VideoFilesLocation = settings.value(QString("%1/VideoFilesLocation").arg(SectionName), m_SettingsData->m_VideoFilesLocation).toString();
    m_SettingsData->m_TrendGraphDataLocation = settings.value(QString("%1/TrendGraphDataLocation").arg(SectionName), m_SettingsData->m_TrendGraphDataLocation).toString();
    m_SettingsData->m_TriggerImagesFileLocation = settings.value(QString("%1/TriggerImagesFileLocation").arg(SectionName), m_SettingsData->m_TriggerImagesFileLocation).toString();
    m_SettingsData->m_ErrorImagePoolLocation = settings.value(QString("%1/ErrorImagePoolLocation").arg(SectionName), m_SettingsData->m_ErrorImagePoolLocation).toString();
    m_SettingsData->m_ProductDataLocation = settings.value(QString("%1/ProductDataLocation").arg(SectionName), m_SettingsData->m_ProductDataLocation).toString();
    m_SettingsData->m_AudiTrailDataLocation = settings.value(QString("%1/AudiTrailDataLocation").arg(SectionName), m_SettingsData->m_AudiTrailDataLocation).toString();
    m_SettingsData->m_AlarmMessageLocation = settings.value(QString("%1/AlarmMessageLocation").arg(SectionName), m_SettingsData->m_AlarmMessageLocation).toString();
    m_SettingsData->m_BackupLocationRegistryData = settings.value(QString("%1/BackupLocationRegistryData").arg(SectionName), m_SettingsData->m_BackupLocationRegistryData).toString();
    m_SettingsData->m_ScreenShotLocation = settings.value(QString("%1/ScreenShotLocation").arg(SectionName), m_SettingsData->m_ScreenShotLocation).toString();
    // m_SettingsData->m_LogFileLocation = settings.value(QString("%1/LogFileLocation").arg(SectionName), m_SettingsData->m_LogFileLocation).toString();
    SectionName = RegeditDir + "KitharaRealtimeDeviceSettings";
    m_SettingsData->m_XMLCofigurationFileLocation = settings.value(QString("%1/XMLCofigurationFileLocation").arg(SectionName), m_SettingsData->m_XMLCofigurationFileLocation).toString();
    m_SettingsData->m_KitharaCustomerNumber = settings.value(QString("%1/KitharaCustomerNumber").arg(SectionName), m_SettingsData->m_KitharaCustomerNumber).toString();
    m_SettingsData->m_NameKernelDll = settings.value(QString("%1/NameKernelDLL").arg(SectionName), m_SettingsData->m_NameKernelDll).toString();
    m_SettingsData->m_DeviceIndexXHCIController = settings.value(QString("%1/DeviceIndexXHCIController").arg(SectionName), m_SettingsData->m_DeviceIndexXHCIController).toInt();
    m_SettingsData->m_NetworkAdapterID = settings.value(QString("%1/NetworkAdapterID").arg(SectionName), QVariant(m_SettingsData->m_NetworkAdapterID)).toInt();
    m_SettingsData->m_TargetProcessorIOTask = settings.value(QString("%1/TargetProcessorIOTask").arg(SectionName), m_SettingsData->m_TargetProcessorIOTask).toInt();
    m_SettingsData->m_TargetProcessorMeasureTask = settings.value(QString("%1/TargetProcessorMeasureTask").arg(SectionName), m_SettingsData->m_TargetProcessorMeasureTask).toInt();
    SectionName = RegeditDir + "DistanceParameterSettings";
    m_SettingsData->m_DistanceBottleEjectionInmm = settings.value(QString("%1/DistanceBottleEjectionInmm").arg(SectionName), m_SettingsData->m_DistanceBottleEjectionInmm).toDouble();
    m_SettingsData->m_DistanceCameraProduct = settings.value(QString("%1/DistanceCameraProduct").arg(SectionName), m_SettingsData->m_DistanceCameraProduct).toDouble();
    m_SettingsData->m_DistancesBetweenValves = settings.value(QString("%1/DistancesBetweenValves").arg(SectionName), m_SettingsData->m_DistancesBetweenValves).toDouble();
    m_SettingsData->m_BlowOutEjectorNormallyClosed = settings.value(QString("%1/BlowOutEjectorNormallyClosed").arg(SectionName), m_SettingsData->m_BlowOutEjectorNormallyClosed).toBool();
    m_SettingsData->m_PixelSize = settings.value(QString("%1/PixelSize").arg(SectionName), m_SettingsData->m_PixelSize).toDouble();
    m_SettingsData->m_SizeVideoMemoryInMB = settings.value(QString("%1/SizeVideoMemoryInMB").arg(SectionName), m_SettingsData->m_SizeVideoMemoryInMB).toInt();
    m_SettingsData->m_BottleOffsetOutOfROIInmm = settings.value(QString("%1/BottleOffsetOutOfROIInmm").arg(SectionName), m_SettingsData->m_BottleOffsetOutOfROIInmm).toDouble();
    m_SettingsData->m_UseSpeedFromISCalcEjectionTime = settings.value(QString("%1/UseSpeedFromISCalcEjectionTime").arg(SectionName), m_SettingsData->m_UseSpeedFromISCalcEjectionTime).toBool();
    SectionName = RegeditDir + "SwitchOffDifferentDevice";
    m_SettingsData->m_WorkWithoutCamera = settings.value(QString("%1/WorkWithoutCamera").arg(SectionName), m_SettingsData->m_WorkWithoutCamera).toBool();
    m_SettingsData->m_WorkWithoutEtherCat = settings.value(QString("%1/WorkWithoutEtherCat").arg(SectionName), m_SettingsData->m_WorkWithoutEtherCat).toBool();
    m_SettingsData->m_WorkWithoutValveController = settings.value(QString("%1/WorkWithoutValveController").arg(SectionName), m_SettingsData->m_WorkWithoutValveController).toBool();
    m_SettingsData->m_WorkWithTwoValves = settings.value(QString("%1/WorkWithTwoValves").arg(SectionName), m_SettingsData->m_WorkWithTwoValves).toBool();
    m_SettingsData->m_WorkWithSecondTriggerSlider = settings.value(QString("%1/WorkWithSecondTriggerSlider").arg(SectionName), m_SettingsData->m_WorkWithSecondTriggerSlider).toBool();
    m_SettingsData->m_SimulateValve = settings.value(QString("%1/SimulateValve").arg(SectionName), m_SettingsData->m_SimulateValve).toBool();
    m_SettingsData->m_LiquidFlowSimulationOn = settings.value(QString("%1/LiquidFlowSimulationOn").arg(SectionName), m_SettingsData->m_LiquidFlowSimulationOn).toBool();
    SectionName = RegeditDir + "SetDigitalOutputOnStartup";
    m_SettingsData->m_CameraLightOnOnStartup = settings.value(QString("%1/CameraLightOnOnStartup").arg(SectionName), m_SettingsData->m_CameraLightOnOnStartup).toBool();
    m_SettingsData->m_PreasureTankHeaterOnOnStartup = settings.value(QString("%1/PreasureTankHeaterOnOnStartup").arg(SectionName), m_SettingsData->m_PreasureTankHeaterOnOnStartup).toBool();
    m_SettingsData->m_PreasureTankValveOnOnStartup = settings.value(QString("%1/PreasureTankValveOnOnStartup").arg(SectionName), m_SettingsData->m_PreasureTankValveOnOnStartup).toBool();
    m_SettingsData->m_ValveControllerOnOnStartup = settings.value(QString("%1/ValveControllerOnOnStartup").arg(SectionName), m_SettingsData->m_ValveControllerOnOnStartup).toBool();
    m_SettingsData->m_WhiteLightOnOnStartup = settings.value(QString("%1/WhiteLightOnOnStartup").arg(SectionName), m_SettingsData->m_WhiteLightOnOnStartup).toBool();
    SectionName = RegeditDir + "FactorAndOffsetAnalogDevices";
    m_SettingsData->m_FactorAnalogOutputDefaultPreasure = settings.value(QString("%1/FactorAnalogOutputDefaultPreasure").arg(SectionName), m_SettingsData->m_FactorAnalogOutputDefaultPreasure).toInt();
    m_SettingsData->m_OffsetAnalogOutputDefaultPreasure = settings.value(QString("%1/OffsetAnalogOutputDefaultPreasure").arg(SectionName), m_SettingsData->m_OffsetAnalogOutputDefaultPreasure).toInt();
    m_SettingsData->m_FactorAnalogInputCurrentPreasure = settings.value(QString("%1/FactorAnalogInputCurrentPreasure").arg(SectionName), m_SettingsData->m_FactorAnalogInputCurrentPreasure).toDouble();
    m_SettingsData->m_OffsetAnalogInputCurrentPreasure = settings.value(QString("%1/OffsetAnalogInputCurrentPreasure").arg(SectionName), m_SettingsData->m_OffsetAnalogInputCurrentPreasure).toDouble();
    m_SettingsData->m_FactorAnalogInputTankFillingLevel =
        settings.value(QString("%1/FactorAnalogInputTankFillingLevel").arg(SectionName), m_SettingsData->m_FactorAnalogInputTankFillingLevel).toDouble();
    m_SettingsData->m_OffsetAnalogInputTankFillingLevel =
        settings.value(QString("%1/OffsetAnalogInputTankFillingLevel").arg(SectionName), m_SettingsData->m_OffsetAnalogInputTankFillingLevel).toDouble();

    m_SettingsData->m_FactorAnalogInputAirCoolingCamera =
        settings.value(QString("%1/FactorAnalogInputAirCoolingCamera").arg(SectionName), m_SettingsData->m_FactorAnalogInputAirCoolingCamera).toDouble();
    m_SettingsData->m_OffsetAnalogInputAirCoolingCamera =
        settings.value(QString("%1/OffsetAnalogInputAirCoolingCamera").arg(SectionName), m_SettingsData->m_OffsetAnalogInputAirCoolingCamera).toDouble();

    m_SettingsData->m_FactorAnalogInputAirCoolingCameraLight =
        settings.value(QString("%1/FactorAnalogInputAirCoolingCameraLight").arg(SectionName), m_SettingsData->m_FactorAnalogInputAirCoolingCameraLight).toDouble();
    m_SettingsData->m_OffsetAnalogInputAirCoolingCameraLight =
        settings.value(QString("%1/OffsetAnalogInputAirCoolingCameraLight").arg(SectionName), m_SettingsData->m_OffsetAnalogInputAirCoolingCameraLight).toDouble();

    m_SettingsData->m_FactorAnalogInputAirCoolingValves =
        settings.value(QString("%1/FactorAnalogInputAirCoolingValves").arg(SectionName), m_SettingsData->m_FactorAnalogInputAirCoolingValves).toDouble();
    m_SettingsData->m_OffsetAnalogInputAirCoolingValves =
        settings.value(QString("%1/OffsetAnalogInputAirCoolingValves").arg(SectionName), m_SettingsData->m_OffsetAnalogInputAirCoolingValves).toDouble();

    m_SettingsData->m_FactorAnalogInputWaterCooling = settings.value(QString("%1/FactorAnalogInputWaterCooling").arg(SectionName), m_SettingsData->m_FactorAnalogInputWaterCooling).toDouble();
    m_SettingsData->m_OffsetAnalogInputWaterCooling = settings.value(QString("%1/OffsetAnalogInputWaterCooling").arg(SectionName), m_SettingsData->m_OffsetAnalogInputWaterCooling).toDouble();

    m_SettingsData->m_FactorAnalogInputFlowTransmitterWaterCoolingCircuit =
        settings.value(QString("%1/FactorAnalogInputFlowTransmitterWaterCoolingCircuit").arg(SectionName), m_SettingsData->m_FactorAnalogInputFlowTransmitterWaterCoolingCircuit).toDouble();
    m_SettingsData->m_OffsetAnalogInputFlowTransmitterWaterCoolingCircuit =
        settings.value(QString("%1/OffsetAnalogInputFlowTransmitterWaterCoolingCircuit").arg(SectionName), m_SettingsData->m_OffsetAnalogInputFlowTransmitterWaterCoolingCircuit).toDouble();

    m_SettingsData->m_FactorAnalogInputTemperaturWaterCoolingReturn =
        settings.value(QString("%1/FactorAnalogInputTemperaturWaterCoolingReturn").arg(SectionName), m_SettingsData->m_FactorAnalogInputTemperaturWaterCoolingReturn).toDouble();
    m_SettingsData->m_OffsetAnalogInputTemperaturWaterCoolingReturn =
        settings.value(QString("%1/OffsetAnalogInputTemperaturWaterCoolingReturn").arg(SectionName), m_SettingsData->m_OffsetAnalogInputTemperaturWaterCoolingReturn).toDouble();

    /* m_SettingsData->m_ValueAirCoolingCameraLitersPerMinute =
         settings.value(QString("%1/ValueAirCoolingCameraLitersPerMinute").arg(SectionName), m_SettingsData->m_ValueAirCoolingCameraLitersPerMinute).toDouble();

     m_SettingsData->m_ValueAirCoolingLightLitersPerMinute =
         settings.value(QString("%1/ValueAirCoolingLightLitersPerMinute").arg(SectionName), m_SettingsData->m_ValueAirCoolingLightLitersPerMinute).toDouble();

     m_SettingsData->m_ValueAirCoolingValveLitersPerMinute =
         settings.value(QString("%1/ValueAirCoolingValveLitersPerMinute").arg(SectionName), m_SettingsData->m_ValueAirCoolingValveLitersPerMinute).toDouble();

     m_SettingsData->m_ValueWaterCoolingDefaultLitersPerMinute =
         settings.value(QString("%1/ValueWaterCoolingDefaultLitersPerMinute").arg(SectionName), m_SettingsData->m_ValueWaterCoolingDefaultLitersPerMinute).toDouble();
     m_SettingsData->m_ValueWaterCoolingCircuitDefaultSensor =
         settings.value(QString("%1/ValueWaterCoolingCircuitDefaultSensor").arg(SectionName), m_SettingsData->m_ValueWaterCoolingCircuitDefaultSensor).toDouble();*/

    SectionName = RegeditDir + "ThresholdsAndLimits";
    // m_SettingsData->m_DeviationThresholdSpeed = settings.value(QString("%1/DeviationThresholdSpeed").arg(SectionName), m_SettingsData->m_DeviationThresholdSpeed).toDouble();
    m_SettingsData->m_MinSpeedInMMPerMs = settings.value(QString("%1/MinSpeedInMMPerMs").arg(SectionName), m_SettingsData->m_MinSpeedInMMPerMs).toDouble();
    m_SettingsData->m_ThresholdBinaryImageLiquid = settings.value(QString("%1/ThresholdBinaryImageLiquid").arg(SectionName), m_SettingsData->m_ThresholdBinaryImageLiquid).toInt();
    m_SettingsData->m_MinNumberFoundedInROI = settings.value(QString("%1/MinNumberFoundedInROI").arg(SectionName), m_SettingsData->m_MinNumberFoundedInROI).toInt();
    m_SettingsData->m_NumberProductsAverageBottleNeckAndPixelSize =
        settings.value(QString("%1/NumberProductsAverageSpeedAndPixelSize").arg(SectionName), m_SettingsData->m_NumberProductsAverageBottleNeckAndPixelSize).toInt();
    m_SettingsData->m_MaxMeasurementsProductIsOutOfTol = settings.value(QString("%1/MaxMeasurementsProductIsOutOfTol").arg(SectionName), m_SettingsData->m_MaxMeasurementsProductIsOutOfTol).toInt();
    m_SettingsData->m_ThresholdBinaryImageDegreeOfPollution =
        settings.value(QString("%1/ThresholdBinaryImageDegreeOfPollution").arg(SectionName), m_SettingsData->m_ThresholdBinaryImageDegreeOfPollution).toInt();
    m_SettingsData->m_TimerIntervalCheckCleanImageInMin =
        settings.value(QString("%1/TimerIntervalCheckCleanImageInMin").arg(SectionName), m_SettingsData->m_TimerIntervalCheckCleanImageInMin).toDouble();
    m_SettingsData->m_MaxStackTemperature = settings.value(QString("%1/MaxStackTemperature").arg(SectionName), m_SettingsData->m_MaxStackTemperature).toDouble();
    m_SettingsData->m_IntervalUpdateTrendGraph = settings.value(QString("%1/IntervalUpdateTrendGraph").arg(SectionName), m_SettingsData->m_IntervalUpdateTrendGraph).toDouble();
    m_SettingsData->m_MaxNumberFilesTrendGraph = settings.value(QString("%1/MaxNumberFilesTrendGraph").arg(SectionName), m_SettingsData->m_MaxNumberFilesTrendGraph).toInt();
    m_SettingsData->m_RollingMeanValueLiquid = settings.value(QString("%1/RollingMeanValueLiquid").arg(SectionName), m_SettingsData->m_RollingMeanValueLiquid).toInt();
    m_SettingsData->m_EdgeAcceptanceThresholdInPercent = settings.value(QString("%1/EdgeAcceptanceThresholdInPercent").arg(SectionName), m_SettingsData->m_EdgeAcceptanceThresholdInPercent).toDouble();
    m_SettingsData->m_BackgroundContrast = settings.value(QString("%1/BackgroundContrast").arg(SectionName), m_SettingsData->m_BackgroundContrast).toInt();
    m_SettingsData->m_MinMeasureWindowHeight = settings.value(QString("%1/MinMeasureWindowHeight").arg(SectionName), m_SettingsData->m_MinMeasureWindowHeight).toInt();
    // m_SettingsData->m_FillingPosTol = settings.value(QString("%1/FillingPosTol").arg(SectionName), m_SettingsData->m_FillingPosTol).toDouble();
    m_SettingsData->m_MinWidthInPixelBlueWindow = settings.value(QString("%1/MinWidthInPixelBlueWindow").arg(SectionName), m_SettingsData->m_MinWidthInPixelBlueWindow).toInt();
    // m_SettingsData->m_MinAmountLiquidBottleUnderValve = settings.value(QString("%1/MinAmountLiquidBottleUnderValve").arg(SectionName), m_SettingsData->m_MinAmountLiquidBottleUnderValve).toInt();
    m_SettingsData->m_PreasureIncreaseWhenFlushing = settings.value(QString("%1/PreasureIncreaseWhenFlushing").arg(SectionName), m_SettingsData->m_PreasureIncreaseWhenFlushing).toDouble();
    m_SettingsData->m_PreasureTolInPercent = settings.value(QString("%1/PreasureTolInPercent").arg(SectionName), m_SettingsData->m_PreasureTolInPercent).toDouble();
    m_SettingsData->m_MaxNumberFilesTriggerImages = settings.value(QString("%1/MaxNumberFilesTriggerImages").arg(SectionName), m_SettingsData->m_MaxNumberFilesTriggerImages).toInt();
    m_SettingsData->m_MaxNumberFilesEjectedImages = settings.value(QString("%1/MaxNumberFilesEjectedImages").arg(SectionName), m_SettingsData->m_MaxNumberFilesEjectedImages).toInt();
    m_SettingsData->m_DefineNoPreasureValue = settings.value(QString("%1/DefineNoPreasureValue").arg(SectionName), m_SettingsData->m_DefineNoPreasureValue).toDouble();
    m_SettingsData->m_TankFillingLevelMaschineStopInLiter =
        settings.value(QString("%1/TankFillingLevelMaschineStopInLiter").arg(SectionName), m_SettingsData->m_TankFillingLevelMaschineStopInLiter).toDouble();
    m_SettingsData->m_TankFillingLevelAlarmInLiter = settings.value(QString("%1/TankFillingLevelAlarmInLiter").arg(SectionName), m_SettingsData->m_TankFillingLevelAlarmInLiter).toDouble();
    m_SettingsData->m_TankFillingLevelWarningInLiter = settings.value(QString("%1/TankFillingLevelWarningInLiter").arg(SectionName), m_SettingsData->m_TankFillingLevelWarningInLiter).toDouble();
    m_SettingsData->m_TankFillingLevelMaxValue = settings.value(QString("%1/TankFillingLevelMaxValue").arg(SectionName), m_SettingsData->m_TankFillingLevelMaxValue).toDouble();

    m_SettingsData->m_TankTemperatureLevelMaschineStopTimeInSec =
        settings.value(QString("%1/TankTemperatureLevelMaschineStopTimeInSec").arg(SectionName), m_SettingsData->m_TankTemperatureLevelMaschineStopTimeInSec).toDouble();
    m_SettingsData->m_TankTemperatureLevelAlarmInDegree =
        settings.value(QString("%1/TankTemperatureLevelAlarmInDegree").arg(SectionName), m_SettingsData->m_TankTemperatureLevelAlarmInDegree).toDouble();
    m_SettingsData->m_TankTemperatureLevelWarningInDegree =
        settings.value(QString("%1/TankTemperatureLevelWarningInDegree").arg(SectionName), m_SettingsData->m_TankTemperatureLevelWarningInDegree).toDouble();

    m_SettingsData->m_TankPreasureLevelMaschineStopTimeInSec =
        settings.value(QString("%1/TankPreasureLevelMaschineStopTimeInSec").arg(SectionName), m_SettingsData->m_TankPreasureLevelMaschineStopTimeInSec).toDouble();
    m_SettingsData->m_TankPreasureLevelAlarmInPercent = settings.value(QString("%1/TankPreasureLevelAlarmInPercent").arg(SectionName), m_SettingsData->m_TankPreasureLevelAlarmInPercent).toDouble();
    m_SettingsData->m_TankPreasureLevelWarningInPercent =
        settings.value(QString("%1/TankPreasureLevelWarningInPercent").arg(SectionName), m_SettingsData->m_TankPreasureLevelWarningInPercent).toDouble();

    m_SettingsData->m_DegreePollutionMaschineStopTimeInSec =
        settings.value(QString("%1/DegreePollutionMaschineStopTimeInSec").arg(SectionName), m_SettingsData->m_DegreePollutionMaschineStopTimeInSec).toDouble();
    m_SettingsData->m_DegreePollutionAlarmInPercent = settings.value(QString("%1/DegreePollutionAlarmInPercent").arg(SectionName), m_SettingsData->m_DegreePollutionAlarmInPercent).toDouble();
    m_SettingsData->m_DegreePollutionWarningInPercent = settings.value(QString("%1/DegreePollutionWarningInPercent").arg(SectionName), m_SettingsData->m_DegreePollutionWarningInPercent).toDouble();

    m_SettingsData->m_ValveChamberTemperatureWarningLevelInDegree =
        settings.value(QString("%1/ValveChamberTemperatureWarningLevelInDegree").arg(SectionName), m_SettingsData->m_ValveChamberTemperatureWarningLevelInDegree).toDouble();
    m_SettingsData->m_ValveChamberTemperatureAlarmLevelInDegree =
        settings.value(QString("%1/ValveChamberTemperatureAlarmLevelInDegree").arg(SectionName), m_SettingsData->m_ValveChamberTemperatureAlarmLevelInDegree).toDouble();
    m_SettingsData->m_ValveChamberTemperatureMaschineStopTimeInSec =
        settings.value(QString("%1/ValveChamberTemperatureMaschineStopTimeInSec").arg(SectionName), m_SettingsData->m_ValveChamberTemperatureMaschineStopTimeInSec).toDouble();

    m_SettingsData->m_ValvePiezoTemperatureWarningLevelInDegree =
        settings.value(QString("%1/ValvePiezoTemperatureWarningLevelInDegree").arg(SectionName), m_SettingsData->m_ValvePiezoTemperatureWarningLevelInDegree).toDouble();
    m_SettingsData->m_ValvePiezoTemperatureAlarmLevelInDegree =
        settings.value(QString("%1/ValvePiezoTemperatureAlarmLevelInDegree").arg(SectionName), m_SettingsData->m_ValvePiezoTemperatureAlarmLevelInDegree).toDouble();
    m_SettingsData->m_ValvePiezoTemperatureMaschineStopTimeInSec =
        settings.value(QString("%1/ValvePiezoTemperatureMaschineStopTimeInSec").arg(SectionName), m_SettingsData->m_ValvePiezoTemperatureMaschineStopTimeInSec).toDouble();

    m_SettingsData->m_SpeedDeviationAlarmLevelInMPerMin =
        settings.value(QString("%1/SpeedDeviationAlarmLevelInMPerMin").arg(SectionName), m_SettingsData->m_SpeedDeviationAlarmLevelInMPerMin).toDouble();
    m_SettingsData->m_SpeedDeviationWarningLevelInMPerMin =
        settings.value(QString("%1/SpeedDeviationWarningLevelInMPerMin").arg(SectionName), m_SettingsData->m_SpeedDeviationWarningLevelInMPerMin).toDouble();
    m_SettingsData->m_SpeedDeviationMaschineStopTimeInSec =
        settings.value(QString("%1/SpeedDeviationMaschineStopTimeInSec").arg(SectionName), m_SettingsData->m_SpeedDeviationMaschineStopTimeInSec).toDouble();

    m_SettingsData->m_HeatingPipeLevelMaschineStopTimeInSec =
        settings.value(QString("%1/HeatingPipeLevelMaschineStopTimeInSec").arg(SectionName), m_SettingsData->m_HeatingPipeLevelMaschineStopTimeInSec).toDouble();
    m_SettingsData->m_HeatingPipeTemperatureLevelAlarmInPercent =
        settings.value(QString("%1/HeatingPipeTemperatureLevelAlarmInPercent").arg(SectionName), m_SettingsData->m_HeatingPipeTemperatureLevelAlarmInPercent).toDouble();
    m_SettingsData->m_HeatingPipeTemperatureLevelWarningInPercent =
        settings.value(QString("%1/HeatingPipeTemperatureLevelWarningInPercent").arg(SectionName), m_SettingsData->m_HeatingPipeTemperatureLevelWarningInPercent).toDouble();

    m_SettingsData->m_AirCoolingMaschineStopTimeInSec = settings.value(QString("%1/AirCoolingMaschineStopTimeInSec").arg(SectionName), m_SettingsData->m_AirCoolingMaschineStopTimeInSec).toDouble();
    m_SettingsData->m_AirCoolingAlarmInPercent = settings.value(QString("%1/AirCoolingAlarmInPercent").arg(SectionName), m_SettingsData->m_AirCoolingAlarmInPercent).toDouble();
    m_SettingsData->m_AirCoolingWarningInPercent = settings.value(QString("%1/AirCoolingWarningInPercent").arg(SectionName), m_SettingsData->m_AirCoolingWarningInPercent).toDouble();

    m_SettingsData->m_WaterCoolingMaschineStopTimeInSec =
        settings.value(QString("%1/WaterCoolingMaschineStopTimeInSec").arg(SectionName), m_SettingsData->m_WaterCoolingMaschineStopTimeInSec).toDouble();
    m_SettingsData->m_WaterCoolingAlarm = settings.value(QString("%1/WaterCoolingAlarm").arg(SectionName), m_SettingsData->m_WaterCoolingAlarm).toDouble();
    m_SettingsData->m_WaterCoolingWarning = settings.value(QString("%1/WaterCoolingWarning").arg(SectionName), m_SettingsData->m_WaterCoolingWarning).toDouble();

    m_SettingsData->m_CounterProductNotFilledAlarmLevel =
        settings.value(QString("%1/CounterProductNotFilledAlarmLevel").arg(SectionName), m_SettingsData->m_CounterProductNotFilledAlarmLevel).toDouble();
    m_SettingsData->m_CounterProductNotFilledWarningLevel =
        settings.value(QString("%1/CounterProductNotFilledWarningLevel").arg(SectionName), m_SettingsData->m_CounterProductNotFilledWarningLevel).toDouble();
    m_SettingsData->m_CounterProductNotFilledMaschineStopTimeInSec =
        settings.value(QString("%1/CounterProductNotFilledMaschineStopTimeInSec").arg(SectionName), m_SettingsData->m_CounterProductNotFilledMaschineStopTimeInSec).toDouble();

    m_SettingsData->m_MixerMovingMaschineStopTimeInSec = settings.value(QString("%1/MixerMovingMaschineStopTimeInSec").arg(SectionName), m_SettingsData->m_MixerMovingMaschineStopTimeInSec).toDouble();
    m_SettingsData->m_MixerMovingAlarmInRPM = settings.value(QString("%1/MixerMovingAlarmInRPM").arg(SectionName), m_SettingsData->m_MixerMovingAlarmInRPM).toDouble();
    m_SettingsData->m_MixerMovingWarningInRPM = settings.value(QString("%1/MixerMovingWarningInRPM").arg(SectionName), m_SettingsData->m_MixerMovingWarningInRPM).toDouble();

    m_SettingsData->m_NumberOfDropletsPerRun = settings.value(QString("%1/NumberOfDropletsPerRun").arg(SectionName), m_SettingsData->m_NumberOfDropletsPerRun).toInt();
    m_SettingsData->m_NumberRunsManualTrigger = settings.value(QString("%1/NumberRunsManualTrigger").arg(SectionName), m_SettingsData->m_NumberRunsManualTrigger).toInt();
    m_SettingsData->m_TimeBetweenRunsManualTriggerInMs = settings.value(QString("%1/TimeBetweenRunsManualTriggerInMs").arg(SectionName), m_SettingsData->m_TimeBetweenRunsManualTriggerInMs).toInt();

    m_SettingsData->m_MaxCounterBottleEjectedOneAfterTheOther =
        settings.value(QString("%1/MaxCounterBottleEjectedOneAfterTheOther").arg(SectionName), m_SettingsData->m_MaxCounterBottleEjectedOneAfterTheOther).toInt();
    m_SettingsData->m_MaxCounterMiddleTooLowOneAfterTheOther =
        settings.value(QString("%1/MaxCounterMiddleTooLowOneAfterTheOther").arg(SectionName), m_SettingsData->m_MaxCounterMiddleTooLowOneAfterTheOther).toInt();
    m_SettingsData->m_PauseTriggerManualInMs = settings.value(QString("%1/PauseTriggerManualInMs").arg(SectionName), m_SettingsData->m_PauseTriggerManualInMs).toInt();
    m_SettingsData->m_SpeedMixerStepperValue = settings.value(QString("%1/SpeedMixerStepperValue").arg(SectionName), m_SettingsData->m_SpeedMixerStepperValue).toInt();
    m_SettingsData->m_MaximumErrorCount = settings.value(QString("%1/MaximumErrorCount").arg(SectionName), m_SettingsData->m_MaximumErrorCount).toInt();

    SectionName = RegeditDir + "GeneralSettings";
    // m_SettingsData->m_FirstPortIsFirst = settings.value(QString("%1/FirstPortIsFirst").arg(SectionName), m_SettingsData->m_FirstPortIsFirst).toBool();
    m_SettingsData->m_RightTriggerIsFirst = settings.value(QString("%1/FirstTriggerOutIsFirst").arg(SectionName), m_SettingsData->m_RightTriggerIsFirst).toBool();
    m_SettingsData->m_BandDirectional = settings.value(QString("%1/BandDirectional").arg(SectionName), m_SettingsData->m_BandDirectional).toInt();
    m_SettingsData->m_TimeIntervalReadStatusDataInms = settings.value(QString("%1/TimeIntervalReadStatusDataInms").arg(SectionName), m_SettingsData->m_TimeIntervalReadStatusDataInms).toInt();
    m_SettingsData->m_CameraLiveImageTimeoutInms = settings.value(QString("%1/CameraLiveImageTimeoutInms").arg(SectionName), m_SettingsData->m_CameraLiveImageTimeoutInms).toInt();
    m_SettingsData->m_PollingTimeReadNewVideoData = settings.value(QString("%1/PollingTimeReadNewVideoData").arg(SectionName), m_SettingsData->m_PollingTimeReadNewVideoData).toInt();
    m_SettingsData->m_CameraMoveOffsetInX = settings.value(QString("%1/CameraMoveOffsetInX").arg(SectionName), m_SettingsData->m_CameraMoveOffsetInX).toInt();
    m_SettingsData->m_CameraMoveOffsetInY = settings.value(QString("%1/CameraMoveOffsetInY").arg(SectionName), m_SettingsData->m_CameraMoveOffsetInY).toInt();
    m_SettingsData->m_DeaktivateCheckBottleUnderValve = settings.value(QString("%1/DeaktivateCheckBottleUnderValve").arg(SectionName), m_SettingsData->m_DeaktivateCheckBottleUnderValve).toBool();
    m_SettingsData->m_UseCameraHighRes = settings.value(QString("%1/UseCameraHighRes").arg(SectionName), m_SettingsData->m_UseCameraHighRes).toBool();
    SectionName = RegeditDir + "AutoCalibrate";
    m_SettingsData->m_FactorMeasureWindowWidthSearchBottleTopPosiition =
        settings.value(QString("%1/FactorMeasureWindowWidthSearchBottleTopPosiition").arg(SectionName), m_SettingsData->m_FactorMeasureWindowWidthSearchBottleTopPosiition).toDouble();
    m_SettingsData->m_MinPossibleContrastValueInPercent =
        settings.value(QString("%1/MinPossibleContrastValueInPercent").arg(SectionName), m_SettingsData->m_MinPossibleContrastValueInPercent).toDouble();
    m_SettingsData->m_StartValueMeasureWindowHeightInPixel =
        settings.value(QString("%1/StartValueMeasureWindowHeightInPixel").arg(SectionName), m_SettingsData->m_StartValueMeasureWindowHeightInPixel).toInt();
    m_SettingsData->m_NumIterationROIHeight = settings.value(QString("%1/NumIterationROIHeight").arg(SectionName), m_SettingsData->m_NumIterationROIHeight).toInt();
    m_SettingsData->m_NumIterationCalculteROIPosition = settings.value(QString("%1/NumIterationCalculteROIPosition").arg(SectionName), m_SettingsData->m_NumIterationCalculteROIPosition).toInt();
    m_SettingsData->m_NumIterationCalculateAccseptanceThreshold =
        settings.value(QString("%1/NumIterationCalculateAccseptanceThreshold").arg(SectionName), m_SettingsData->m_NumIterationCalculateAccseptanceThreshold).toInt();
    m_SettingsData->m_NumIterationCalculatePixelSize = settings.value(QString("%1/NumIterationCalculatePixelSize").arg(SectionName), m_SettingsData->m_NumIterationCalculatePixelSize).toInt();
    m_SettingsData->m_FactorThreshold = settings.value(QString("%1/FactorThreshold").arg(SectionName), m_SettingsData->m_FactorThreshold).toDouble();
    // m_SettingsData->m_FactorGreenROIWidthByDiameter = settings.value(QString("%1/FactorGreenROIWidthByDiameter").arg(SectionName), m_SettingsData->m_FactorGreenROIWidthByDiameter).toDouble();
    // m_SettingsData->m_FactorYellowROIWidthByDiameter = settings.value(QString("%1/FactorYellowROIWidthByDiameter").arg(SectionName), m_SettingsData->m_FactorYellowROIWidthByDiameter).toDouble();
    m_SettingsData->m_DiameterTolInPercentAutoCalibrate = settings.value(QString("%1/DiameterTolInMMInAutoCalibrate").arg(SectionName), m_SettingsData->m_DiameterTolInPercentAutoCalibrate).toDouble();
    m_SettingsData->m_BottleBaseLineOffsetInPix = settings.value(QString("%1/BottleBaseLineOffsetInPix").arg(SectionName), m_SettingsData->m_BottleBaseLineOffsetInPix).toInt();
    m_SettingsData->m_NumIterationROIYPos = settings.value(QString("%1/NumIterationROIYPos").arg(SectionName), m_SettingsData->m_NumIterationROIYPos).toInt();
    m_SettingsData->m_MinPixelSizeAutocalibration = settings.value(QString("%1/MinPixelSizeAutocalibration").arg(SectionName), m_SettingsData->m_MinPixelSizeAutocalibration).toDouble();

    SectionName = RegeditDir + "TrendGraph";
    m_SettingsData->m_TrendGraphIimeRangeIndex = settings.value(QString("%1/TrendGraphIimeRangeIndex").arg(SectionName), m_SettingsData->m_TrendGraphIimeRangeIndex).toInt();
    m_SettingsData->m_TrendGraphRollingMeanSizeBottlesPerMin =
        settings.value(QString("%1/TrendGraphRollingMeanSizeBottlesPerMin").arg(SectionName), m_SettingsData->m_TrendGraphRollingMeanSizeBottlesPerMin).toInt();
    m_SettingsData->m_TrendGraphFlag = settings.value(QString("%1/TrendGraphFlag").arg(SectionName), m_SettingsData->m_TrendGraphFlag).toInt();
    m_SettingsData->m_TrendGraphMaxRangeLiquid = settings.value(QString("%1/TrendGraphMaxRangeLiquid").arg(SectionName), m_SettingsData->m_TrendGraphMaxRangeLiquid).toInt();
    m_SettingsData->m_TrendGraphMinRangeLiquid = settings.value(QString("%1/TrendGraphMinRangeLiquid").arg(SectionName), m_SettingsData->m_TrendGraphMinRangeLiquid).toInt();
    m_SettingsData->m_TrendGraphAbsolutMaximumLiquid = settings.value(QString("%1/TrendGraphAbsolutMaximumLiquid").arg(SectionName), m_SettingsData->m_TrendGraphAbsolutMaximumLiquid).toInt();
    m_SettingsData->m_TrendGraphAbsolutMinimumLiquid = settings.value(QString("%1/TrendGraphAbsolutMinimumLiquid").arg(SectionName), m_SettingsData->m_TrendGraphAbsolutMinimumLiquid).toInt();
    m_SettingsData->m_TrendGraphAbsolutMaximumTemperature =
        settings.value(QString("%1/TrendGraphAbsolutMaximumTemperature").arg(SectionName), m_SettingsData->m_TrendGraphAbsolutMaximumTemperature).toInt();
    m_SettingsData->m_TrendGraphAbsolutMinimumTemperature =
        settings.value(QString("%1/TrendGraphAbsolutMinimumTemperature").arg(SectionName), m_SettingsData->m_TrendGraphAbsolutMinimumTemperature).toInt();
    m_SettingsData->m_TrendGraphAbsolutMaximumBottlesPerMin =
        settings.value(QString("%1/TrendGraphAbsolutMaximumBottlesPerMin").arg(SectionName), m_SettingsData->m_TrendGraphAbsolutMaximumBottlesPerMin).toInt();
    m_SettingsData->m_TrendGraphAbsolutMinimumBottlesPerMin =
        settings.value(QString("%1/TrendGraphAbsolutMinimumBottlesPerMin").arg(SectionName), m_SettingsData->m_TrendGraphAbsolutMinimumBottlesPerMin).toInt();

    SectionName = RegeditDir + "DeactivateAlarmLevels";
    m_SettingsData->m_EnableStatusCurrentLiquidTankPreasure =
        settings.value(QString("%1/EnableStatusCurrentLiquidTankPreasure").arg(SectionName), m_SettingsData->m_EnableStatusCurrentLiquidTankPreasure).toBool();
    m_SettingsData->m_EnableStatusCurrentLiquidTankFilling =
        settings.value(QString("%1/EnableStatusCurrentLiquidTankFilling").arg(SectionName), m_SettingsData->m_EnableStatusCurrentLiquidTankFilling).toBool();
    m_SettingsData->m_EnableStatusCurrentLiquidTankTemp =
        settings.value(QString("%1/EnableStatusCurrentLiquidTankTemp").arg(SectionName), m_SettingsData->m_EnableStatusCurrentLiquidTankTemp).toBool();
    m_SettingsData->m_EnableStatusPiezoTempLeftValve = settings.value(QString("%1/EnableStatusPiezoTempLeftValve").arg(SectionName), m_SettingsData->m_EnableStatusPiezoTempLeftValve).toBool();
    m_SettingsData->m_EnableStatusChamperTempLeftValve = settings.value(QString("%1/EnableStatusChamperTempLeftValve").arg(SectionName), m_SettingsData->m_EnableStatusChamperTempLeftValve).toBool();
    m_SettingsData->m_EnableStatusPiezoTempRightValve = settings.value(QString("%1/EnableStatusPiezoTempRightValve").arg(SectionName), m_SettingsData->m_EnableStatusPiezoTempRightValve).toBool();
    m_SettingsData->m_EnableStatusChamperTempRightValve =
        settings.value(QString("%1/EnableStatusChamperTempRightValve").arg(SectionName), m_SettingsData->m_EnableStatusChamperTempRightValve).toBool();
    m_SettingsData->m_EnableStatusSpeedDeviationBetweenCameraAndIS =
        settings.value(QString("%1/EnableStatusSpeedDeviationBetweenCameraAndIS").arg(SectionName), m_SettingsData->m_EnableStatusSpeedDeviationBetweenCameraAndIS).toBool();
    m_SettingsData->m_EnableStatusHeatingPipeTemp = settings.value(QString("%1/EnableStatusHeatingPipeTemp").arg(SectionName), m_SettingsData->m_EnableStatusHeatingPipeTemp).toBool();
    m_SettingsData->m_EnableStatusAirCoolingCameraActualValue =
        settings.value(QString("%1/EnableStatusAirCoolingCameraActualValue").arg(SectionName), m_SettingsData->m_EnableStatusAirCoolingCameraActualValue).toBool();
    m_SettingsData->m_EnableStatusAirCoolingCameraLightActualValue =
        settings.value(QString("%1/EnableStatusAirCoolingCameraLightActualValue").arg(SectionName), m_SettingsData->m_EnableStatusAirCoolingCameraLightActualValue).toBool();
    m_SettingsData->m_EnableStatusAirCoolingValvesActualValue =
        settings.value(QString("%1/EnableStatusAirCoolingValvesActualValue").arg(SectionName), m_SettingsData->m_EnableStatusAirCoolingValvesActualValue).toBool();
    m_SettingsData->m_EnableStatusWaterCoolingActualValue =
        settings.value(QString("%1/EnableStatusWaterCoolingActualValue").arg(SectionName), m_SettingsData->m_EnableStatusWaterCoolingActualValue).toBool();
    m_SettingsData->m_EnableStatusFlowTransmitterWaterCoolingCircuitActualValue =
        settings.value(QString("%1/EnableStatusFlowTransmitterWaterCoolingCircuitActualValue").arg(SectionName), m_SettingsData->m_EnableStatusFlowTransmitterWaterCoolingCircuitActualValue).toBool();
    m_SettingsData->m_EnableStatusWaterCoolingTemperatureReturnActualValue =
        settings.value(QString("%1/EnableStatusWaterCoolingTemperatureReturnActualValue").arg(SectionName), m_SettingsData->m_EnableStatusWaterCoolingTemperatureReturnActualValue).toBool();
    m_SettingsData->m_EnableStatusCounterProducNotFilled =
        settings.value(QString("%1/EnableStatusCounterProducNotFilled").arg(SectionName), m_SettingsData->m_EnableStatusCounterProducNotFilled).toBool();
    m_SettingsData->m_EnableStatusCounterBottleEjectedOneAfterTheOther =
        settings.value(QString("%1/EnableStatusCounterBottleEjectedOneAfterTheOther").arg(SectionName), m_SettingsData->m_EnableStatusCounterBottleEjectedOneAfterTheOther).toBool();
    m_SettingsData->m_EnableStatusCounterMiddleTooLowOneAfterTheOther =
        settings.value(QString("%1/EnableStatusCounterMiddleTooLowOneAfterTheOther").arg(SectionName), m_SettingsData->m_EnableStatusCounterMiddleTooLowOneAfterTheOther).toBool();
    m_SettingsData->m_EnableStatusDegreeOfPolution = settings.value(QString("%1/EnableStatusDegreeOfPolution").arg(SectionName), m_SettingsData->m_EnableStatusDegreeOfPolution).toBool();
    m_SettingsData->m_EnableStatusMixer = settings.value(QString("%1/EnableStatusMixer").arg(SectionName), m_SettingsData->m_EnableStatusMixer).toBool();

    SectionName = RegeditDir + "WaterCoolingPIDParameter";
    m_SettingsData->m_WaterCoolingStrokeMinValue = settings.value(QString("%1/WaterCoolingStrokeMinValue").arg(SectionName), m_SettingsData->m_WaterCoolingStrokeMinValue).toDouble();
    m_SettingsData->m_WaterCoolingStrokeMaxValue = settings.value(QString("%1/WaterCoolingStrokeMaxValue").arg(SectionName), m_SettingsData->m_WaterCoolingStrokeMaxValue).toDouble();
    m_SettingsData->m_WaterCoolingPFactor = settings.value(QString("%1/WaterCoolingPFactor").arg(SectionName), m_SettingsData->m_WaterCoolingPFactor).toDouble();
    m_SettingsData->m_WaterCoolingIFactor = settings.value(QString("%1/WaterCoolingIFactor").arg(SectionName), m_SettingsData->m_WaterCoolingIFactor).toDouble();
    m_SettingsData->m_WaterCoolingDFactor = settings.value(QString("%1/WaterCoolingDFactor").arg(SectionName), m_SettingsData->m_WaterCoolingDFactor).toDouble();
}

void MainAppCrystalT2::SetSettings(QSettings& settings, QString& RegeditDir)
{
    QString SectionName;
    SectionName = RegeditDir + "IODeviceSettings";
    settings.setValue(QString("%1/CycleTimeIOTask").arg(SectionName), m_SettingsData->m_CycleTimeIOTask);
    settings.setValue(QString("%1/TimePeriodTriggerOutputOnInms").arg(SectionName), m_SettingsData->m_TimePeriodTriggerOutputOnInms);
    settings.setValue(QString("%1/TimePeriodDigitalOutputOnInms").arg(SectionName), m_SettingsData->m_TimePeriodDigitalOutputOnInms);
    settings.setValue(QString("%1/ExposureTime").arg(SectionName), m_SettingsData->m_ExposureTime);
    SectionName = RegeditDir + "SerialPortSettings";
    settings.setValue(QString("%1/ValveControllerPortName1").arg(SectionName), m_SettingsData->m_ValveControllerPortName1);
    settings.setValue(QString("%1/ValveControllerPortName2").arg(SectionName), m_SettingsData->m_ValveControllerPortName2);
    settings.setValue(QString("%1/SerialPortBaudRate").arg(SectionName), m_SettingsData->m_BaudRate);
    settings.setValue(QString("%1/SerialPortParity").arg(SectionName), m_SettingsData->m_Parity);
    settings.setValue(QString("%1/SerialPortDataBits").arg(SectionName), m_SettingsData->m_DataBits);
    settings.setValue(QString("%1/SerialPortStopBits").arg(SectionName), m_SettingsData->m_StopBits);
    settings.setValue(QString("%1/PollingTimeReadStatusValveInms").arg(SectionName), m_SettingsData->m_PollingTimeReadStatusValveInms);
    settings.setValue(QString("%1/TimeOutValueSeriellerPort").arg(SectionName), m_SettingsData->m_TimeOutValueSeriellerPort);
    SectionName = RegeditDir + "VideoProductSettings";
    settings.setValue(QString("%1/ProductName").arg(SectionName), m_SettingsData->m_CurrentProductName);
    settings.setValue(QString("%1/MaxTriggerImagesOnScreen").arg(SectionName), m_SettingsData->m_MaxTriggerImagesOnScreen);
    // settings.setValue(QString("%1/VideoSaveConditionFlag").arg(SectionName), m_SettingsData->m_VideoSaveConditionFlag);
    settings.setValue(QString("%1/VideoFileNameCameraSimulation").arg(SectionName), m_SettingsData->m_VideoFileNameCameraSimulation);
    settings.setValue(QString("%1/VideoFilesLocation").arg(SectionName), m_SettingsData->m_VideoFilesLocation);
    settings.setValue(QString("%1/TrendGraphDataLocation").arg(SectionName), m_SettingsData->m_TrendGraphDataLocation);
    settings.setValue(QString("%1/TriggerImagesFileLocation").arg(SectionName), m_SettingsData->m_TriggerImagesFileLocation);
    settings.setValue(QString("%1/ErrorImagePoolLocation").arg(SectionName), m_SettingsData->m_ErrorImagePoolLocation);
    settings.setValue(QString("%1/ProductDataLocation").arg(SectionName), m_SettingsData->m_ProductDataLocation);
    settings.setValue(QString("%1/AudiTrailDataLocation").arg(SectionName), m_SettingsData->m_AudiTrailDataLocation);
    settings.setValue(QString("%1/AlarmMessageLocation").arg(SectionName), m_SettingsData->m_AlarmMessageLocation);
    settings.setValue(QString("%1/BackupLocationRegistryData").arg(SectionName), m_SettingsData->m_BackupLocationRegistryData);
    settings.setValue(QString("%1/ScreenShotLocation").arg(SectionName), m_SettingsData->m_ScreenShotLocation);

    // settings.setValue(QString("%1/LogFileLocation").arg(SectionName), m_SettingsData->m_LogFileLocation);
    SectionName = RegeditDir + "KitharaRealtimeDeviceSettings";
    settings.setValue(QString("%1/XMLCofigurationFileLocation").arg(SectionName), m_SettingsData->m_XMLCofigurationFileLocation);
    settings.setValue(QString("%1/KitharaCustomerNumber").arg(SectionName), m_SettingsData->m_KitharaCustomerNumber);
    settings.setValue(QString("%1/NameKernelDLL").arg(SectionName), m_SettingsData->m_NameKernelDll);
    settings.setValue(QString("%1/DeviceIndexXHCIController").arg(SectionName), m_SettingsData->m_DeviceIndexXHCIController);
    settings.setValue(QString("%1/NetworkAdapterID").arg(SectionName), QVariant(m_SettingsData->m_NetworkAdapterID));
    settings.setValue(QString("%1/TargetProcessorIOTask").arg(SectionName), m_SettingsData->m_TargetProcessorIOTask);
    settings.setValue(QString("%1/TargetProcessorMeasureTask").arg(SectionName), m_SettingsData->m_TargetProcessorMeasureTask);
    SectionName = RegeditDir + "DistanceParameterSettings";
    settings.setValue(QString("%1/DistanceBottleEjectionInmm").arg(SectionName), m_SettingsData->m_DistanceBottleEjectionInmm);
    settings.setValue(QString("%1/DistanceCameraProduct").arg(SectionName), m_SettingsData->m_DistanceCameraProduct);
    settings.setValue(QString("%1/DistancesBetweenValves").arg(SectionName), m_SettingsData->m_DistancesBetweenValves);
    settings.setValue(QString("%1/BlowOutEjectorNormallyClosed").arg(SectionName), m_SettingsData->m_BlowOutEjectorNormallyClosed);
    settings.setValue(QString("%1/PixelSize").arg(SectionName), m_SettingsData->m_PixelSize);
    settings.setValue(QString("%1/SizeVideoMemoryInMB").arg(SectionName), m_SettingsData->m_SizeVideoMemoryInMB);
    settings.setValue(QString("%1/BottleOffsetOutOfROIInmm").arg(SectionName), m_SettingsData->m_BottleOffsetOutOfROIInmm);
    settings.setValue(QString("%1/UseSpeedFromISCalcEjectionTime").arg(SectionName), m_SettingsData->m_UseSpeedFromISCalcEjectionTime);
    SectionName = RegeditDir + "SwitchOffDifferentDevice";
    settings.setValue(QString("%1/WorkWithoutCamera").arg(SectionName), m_SettingsData->m_WorkWithoutCamera);
    settings.setValue(QString("%1/WorkWithoutEtherCat").arg(SectionName), m_SettingsData->m_WorkWithoutEtherCat);
    settings.setValue(QString("%1/WorkWithoutValveController").arg(SectionName), m_SettingsData->m_WorkWithoutValveController);
    settings.setValue(QString("%1/WorkWithTwoValves").arg(SectionName), m_SettingsData->m_WorkWithTwoValves);
    settings.setValue(QString("%1/WorkWithSecondTriggerSlider").arg(SectionName), m_SettingsData->m_WorkWithSecondTriggerSlider);
    settings.setValue(QString("%1/SimulateValve").arg(SectionName), m_SettingsData->m_SimulateValve);
    settings.setValue(QString("%1/LiquidFlowSimulationOn").arg(SectionName), m_SettingsData->m_LiquidFlowSimulationOn);
    SectionName = RegeditDir + "SetDigitalOutputOnStartup";
    settings.setValue(QString("%1/CameraLightOnOnStartup").arg(SectionName), m_SettingsData->m_CameraLightOnOnStartup);
    settings.setValue(QString("%1/PreasureTankHeaterOnOnStartup").arg(SectionName), m_SettingsData->m_PreasureTankHeaterOnOnStartup);
    settings.setValue(QString("%1/PreasureTankValveOnOnStartup").arg(SectionName), m_SettingsData->m_PreasureTankValveOnOnStartup);
    settings.setValue(QString("%1/ValveControllerOnOnStartup").arg(SectionName), m_SettingsData->m_ValveControllerOnOnStartup);
    settings.setValue(QString("%1/WhiteLightOnOnStartup").arg(SectionName), m_SettingsData->m_WhiteLightOnOnStartup);
    SectionName = RegeditDir + "FactorAndOffsetAnalogDevices";
    settings.setValue(QString("%1/FactorAnalogOutputDefaultPreasure").arg(SectionName), m_SettingsData->m_FactorAnalogOutputDefaultPreasure);
    settings.setValue(QString("%1/OffsetAnalogOutputDefaultPreasure").arg(SectionName), m_SettingsData->m_OffsetAnalogOutputDefaultPreasure);
    settings.setValue(QString("%1/FactorAnalogInputCurrentPreasure").arg(SectionName), m_SettingsData->m_FactorAnalogInputCurrentPreasure);
    settings.setValue(QString("%1/OffsetAnalogInputCurrentPreasure").arg(SectionName), m_SettingsData->m_OffsetAnalogInputCurrentPreasure);
    settings.setValue(QString("%1/FactorAnalogInputTankFillingLevel").arg(SectionName), m_SettingsData->m_FactorAnalogInputTankFillingLevel);
    settings.setValue(QString("%1/OffsetAnalogInputTankFillingLevel").arg(SectionName), m_SettingsData->m_OffsetAnalogInputTankFillingLevel);
    settings.setValue(QString("%1/FactorAnalogInputAirCoolingCamera").arg(SectionName), m_SettingsData->m_FactorAnalogInputAirCoolingCamera);
    settings.setValue(QString("%1/OffsetAnalogInputAirCoolingCamera").arg(SectionName), m_SettingsData->m_OffsetAnalogInputAirCoolingCamera);
    settings.setValue(QString("%1/FactorAnalogInputAirCoolingCameraLight").arg(SectionName), m_SettingsData->m_FactorAnalogInputAirCoolingCameraLight);
    settings.setValue(QString("%1/OffsetAnalogInputAirCoolingCameraLight").arg(SectionName), m_SettingsData->m_OffsetAnalogInputAirCoolingCameraLight);
    settings.setValue(QString("%1/FactorAnalogInputAirCoolingValves").arg(SectionName), m_SettingsData->m_FactorAnalogInputAirCoolingValves);
    settings.setValue(QString("%1/OffsetAnalogInputAirCoolingValves").arg(SectionName), m_SettingsData->m_OffsetAnalogInputAirCoolingValves);
    settings.setValue(QString("%1/FactorAnalogInputWaterCooling").arg(SectionName), m_SettingsData->m_FactorAnalogInputWaterCooling);
    settings.setValue(QString("%1/OffsetAnalogInputWaterCooling").arg(SectionName), m_SettingsData->m_OffsetAnalogInputWaterCooling);
    settings.setValue(QString("%1/FactorAnalogInputFlowTransmitterWaterCoolingCircuit").arg(SectionName), m_SettingsData->m_FactorAnalogInputFlowTransmitterWaterCoolingCircuit);
    settings.setValue(QString("%1/OffsetAnalogInputFlowTransmitterWaterCoolingCircuit").arg(SectionName), m_SettingsData->m_OffsetAnalogInputFlowTransmitterWaterCoolingCircuit);
    settings.setValue(QString("%1/FactorAnalogInputTemperaturWaterCoolingReturn").arg(SectionName), m_SettingsData->m_FactorAnalogInputTemperaturWaterCoolingReturn);
    settings.setValue(QString("%1/OffsetAnalogInputTemperaturWaterCoolingReturn").arg(SectionName), m_SettingsData->m_OffsetAnalogInputTemperaturWaterCoolingReturn);

    /* settings.setValue(QString("%1/ValueAirCoolingCameraLitersPerMinute").arg(SectionName), m_SettingsData->m_ValueAirCoolingCameraLitersPerMinute);
     settings.setValue(QString("%1/ValueAirCoolingLightLitersPerMinute").arg(SectionName), m_SettingsData->m_ValueAirCoolingLightLitersPerMinute);
     settings.setValue(QString("%1/ValueAirCoolingValveLitersPerMinute").arg(SectionName), m_SettingsData->m_ValueAirCoolingValveLitersPerMinute);

     settings.setValue(QString("%1/ValueWaterCoolingDefaultLitersPerMinute").arg(SectionName), m_SettingsData->m_ValueWaterCoolingDefaultLitersPerMinute);
     settings.setValue(QString("%1/ValueWaterCoolingCircuitDefaultSensor").arg(SectionName), m_SettingsData->m_ValueWaterCoolingCircuitDefaultSensor);*/

    SectionName = RegeditDir + "ThresholdsAndLimits";
    // settings.setValue(QString("%1/DeviationThresholdSpeed").arg(SectionName), m_SettingsData->m_DeviationThresholdSpeed);
    settings.setValue(QString("%1/MinSpeedInMMPerMs").arg(SectionName), m_SettingsData->m_MinSpeedInMMPerMs);
    settings.setValue(QString("%1/ThresholdBinaryImageLiquid").arg(SectionName), m_SettingsData->m_ThresholdBinaryImageLiquid);
    settings.setValue(QString("%1/MinNumberFoundedInROI").arg(SectionName), m_SettingsData->m_MinNumberFoundedInROI);
    settings.setValue(QString("%1/NumberProductsAverageSpeedAndPixelSize").arg(SectionName), m_SettingsData->m_NumberProductsAverageBottleNeckAndPixelSize);
    settings.setValue(QString("%1/MaxMeasurementsProductIsOutOfTol").arg(SectionName), m_SettingsData->m_MaxMeasurementsProductIsOutOfTol);
    settings.setValue(QString("%1/ThresholdBinaryImageDegreeOfPollution").arg(SectionName), m_SettingsData->m_ThresholdBinaryImageDegreeOfPollution);
    settings.setValue(QString("%1/TimerIntervalCheckCleanImageInMin").arg(SectionName), m_SettingsData->m_TimerIntervalCheckCleanImageInMin);
    settings.setValue(QString("%1/MaxStackTemperature").arg(SectionName), m_SettingsData->m_MaxStackTemperature);
    settings.setValue(QString("%1/IntervalUpdateTrendGraph").arg(SectionName), m_SettingsData->m_IntervalUpdateTrendGraph);
    settings.setValue(QString("%1/MaxNumberFilesTrendGraph").arg(SectionName), m_SettingsData->m_MaxNumberFilesTrendGraph);
    settings.setValue(QString("%1/RollingMeanValueLiquid").arg(SectionName), m_SettingsData->m_RollingMeanValueLiquid);
    settings.setValue(QString("%1/EdgeAcceptanceThresholdInPercent").arg(SectionName), m_SettingsData->m_EdgeAcceptanceThresholdInPercent);
    settings.setValue(QString("%1/BackgroundContrast").arg(SectionName), m_SettingsData->m_BackgroundContrast);
    settings.setValue(QString("%1/MinMeasureWindowHeight").arg(SectionName), m_SettingsData->m_MinMeasureWindowHeight);
    // settings.setValue(QString("%1/FillingPosTol").arg(SectionName), m_SettingsData->m_FillingPosTol);
    settings.setValue(QString("%1/MinWidthInPixelBlueWindow").arg(SectionName), m_SettingsData->m_MinWidthInPixelBlueWindow);
    // settings.setValue(QString("%1/MinAmountLiquidBottleUnderValve").arg(SectionName), m_SettingsData->m_MinAmountLiquidBottleUnderValve);
    settings.setValue(QString("%1/PreasureIncreaseWhenFlushing").arg(SectionName), m_SettingsData->m_PreasureIncreaseWhenFlushing);
    settings.setValue(QString("%1/DefineNoPreasureValue").arg(SectionName), m_SettingsData->m_DefineNoPreasureValue);
    settings.setValue(QString("%1/PreasureTolInPercent").arg(SectionName), m_SettingsData->m_PreasureTolInPercent);
    settings.setValue(QString("%1/MaxNumberFilesTriggerImages").arg(SectionName), m_SettingsData->m_MaxNumberFilesTriggerImages);
    settings.setValue(QString("%1/MaxNumberFilesEjectedImages").arg(SectionName), m_SettingsData->m_MaxNumberFilesEjectedImages);
    settings.setValue(QString("%1/TankFillingLevelMaschineStopInLiter").arg(SectionName), m_SettingsData->m_TankFillingLevelMaschineStopInLiter);
    settings.setValue(QString("%1/TankFillingLevelAlarmInLiter").arg(SectionName), m_SettingsData->m_TankFillingLevelAlarmInLiter);
    settings.setValue(QString("%1/TankFillingLevelWarningInLiter").arg(SectionName), m_SettingsData->m_TankFillingLevelWarningInLiter);
    settings.setValue(QString("%1/TankFillingLevelMaxValue").arg(SectionName), m_SettingsData->m_TankFillingLevelMaxValue);
    settings.setValue(QString("%1/TankTemperatureLevelMaschineStopTimeInSec").arg(SectionName), m_SettingsData->m_TankTemperatureLevelMaschineStopTimeInSec);
    settings.setValue(QString("%1/TankTemperatureLevelAlarmInDegree").arg(SectionName), m_SettingsData->m_TankTemperatureLevelAlarmInDegree);
    settings.setValue(QString("%1/TankTemperatureLevelWarningInDegree").arg(SectionName), m_SettingsData->m_TankTemperatureLevelWarningInDegree);
    settings.setValue(QString("%1/TankPreasureLevelMaschineStopTimeInSec").arg(SectionName), m_SettingsData->m_TankPreasureLevelMaschineStopTimeInSec);
    settings.setValue(QString("%1/TankPreasureLevelAlarmInPercent").arg(SectionName), m_SettingsData->m_TankPreasureLevelAlarmInPercent);
    settings.setValue(QString("%1/TankPreasureLevelWarningInPercent").arg(SectionName), m_SettingsData->m_TankPreasureLevelWarningInPercent);
    settings.setValue(QString("%1/DegreePollutionMaschineStopTimeInSec").arg(SectionName), m_SettingsData->m_DegreePollutionMaschineStopTimeInSec);
    settings.setValue(QString("%1/DegreePollutionAlarmInPercent").arg(SectionName), m_SettingsData->m_DegreePollutionAlarmInPercent);
    settings.setValue(QString("%1/DegreePollutionWarningInPercent").arg(SectionName), m_SettingsData->m_DegreePollutionWarningInPercent);
    settings.setValue(QString("%1/ValveChamberTemperatureWarningLevelInDegree").arg(SectionName), m_SettingsData->m_ValveChamberTemperatureWarningLevelInDegree);
    settings.setValue(QString("%1/ValveChamberTemperatureAlarmLevelInDegree").arg(SectionName), m_SettingsData->m_ValveChamberTemperatureAlarmLevelInDegree);
    settings.setValue(QString("%1/ValveChamberTemperatureMaschineStopTimeInSec").arg(SectionName), m_SettingsData->m_ValveChamberTemperatureMaschineStopTimeInSec);
    settings.setValue(QString("%1/ValvePiezoTemperatureWarningLevelInDegree").arg(SectionName), m_SettingsData->m_ValvePiezoTemperatureWarningLevelInDegree);
    settings.setValue(QString("%1/ValvePiezoTemperatureAlarmLevelInDegree").arg(SectionName), m_SettingsData->m_ValvePiezoTemperatureAlarmLevelInDegree);
    settings.setValue(QString("%1/ValvePiezoTemperatureMaschineStopTimeInSec").arg(SectionName), m_SettingsData->m_ValvePiezoTemperatureMaschineStopTimeInSec);
    settings.setValue(QString("%1/SpeedDeviationAlarmLevelInMPerMin").arg(SectionName), m_SettingsData->m_SpeedDeviationAlarmLevelInMPerMin);
    settings.setValue(QString("%1/SpeedDeviationWarningLevelInMPerMin").arg(SectionName), m_SettingsData->m_SpeedDeviationWarningLevelInMPerMin);
    settings.setValue(QString("%1/SpeedDeviationMaschineStopTimeInSec").arg(SectionName), m_SettingsData->m_SpeedDeviationMaschineStopTimeInSec);
    settings.setValue(QString("%1/HeatingPipeLevelMaschineStopTimeInSec").arg(SectionName), m_SettingsData->m_HeatingPipeLevelMaschineStopTimeInSec);
    settings.setValue(QString("%1/HeatingPipeTemperatureLevelAlarmInPercent").arg(SectionName), m_SettingsData->m_HeatingPipeTemperatureLevelAlarmInPercent);
    settings.setValue(QString("%1/HeatingPipeTemperatureLevelWarningInPercent").arg(SectionName), m_SettingsData->m_HeatingPipeTemperatureLevelWarningInPercent);
    settings.setValue(QString("%1/AirCoolingMaschineStopTimeInSec").arg(SectionName), m_SettingsData->m_AirCoolingMaschineStopTimeInSec);
    settings.setValue(QString("%1/AirCoolingAlarmInPercent").arg(SectionName), m_SettingsData->m_AirCoolingAlarmInPercent);
    settings.setValue(QString("%1/AirCoolingWarningInPercent").arg(SectionName), m_SettingsData->m_AirCoolingWarningInPercent);
    settings.setValue(QString("%1/WaterCoolingMaschineStopTimeInSec").arg(SectionName), m_SettingsData->m_WaterCoolingMaschineStopTimeInSec);
    settings.setValue(QString("%1/WaterCoolingAlarm").arg(SectionName), m_SettingsData->m_WaterCoolingAlarm);
    settings.setValue(QString("%1/WaterCoolingWarning").arg(SectionName), m_SettingsData->m_WaterCoolingWarning);
    settings.setValue(QString("%1/CounterProductNotFilledAlarmLevel").arg(SectionName), m_SettingsData->m_CounterProductNotFilledAlarmLevel);
    settings.setValue(QString("%1/CounterProductNotFilledWarningLevel").arg(SectionName), m_SettingsData->m_CounterProductNotFilledWarningLevel);
    settings.setValue(QString("%1/CounterProductNotFilledMaschineStopTimeInSec").arg(SectionName), m_SettingsData->m_CounterProductNotFilledMaschineStopTimeInSec);
    settings.setValue(QString("%1/NumberOfDropletsPerRun").arg(SectionName), m_SettingsData->m_NumberOfDropletsPerRun);
    settings.setValue(QString("%1/NumberRunsManualTrigger").arg(SectionName), m_SettingsData->m_NumberRunsManualTrigger);
    settings.setValue(QString("%1/TimeBetweenRunsManualTriggerInMs").arg(SectionName), m_SettingsData->m_TimeBetweenRunsManualTriggerInMs);
    settings.setValue(QString("%1/MaxCounterBottleEjectedOneAfterTheOther").arg(SectionName), m_SettingsData->m_MaxCounterBottleEjectedOneAfterTheOther);
    settings.setValue(QString("%1/MaxCounterMiddleTooLowOneAfterTheOther").arg(SectionName), m_SettingsData->m_MaxCounterMiddleTooLowOneAfterTheOther);
    settings.setValue(QString("%1/PauseTriggerManualInMs").arg(SectionName), m_SettingsData->m_PauseTriggerManualInMs);
    settings.setValue(QString("%1/SpeedMixerStepperValue").arg(SectionName), m_SettingsData->m_SpeedMixerStepperValue);
    settings.setValue(QString("%1/MixerMovingMaschineStopTimeInSec").arg(SectionName), m_SettingsData->m_MixerMovingMaschineStopTimeInSec);
    settings.setValue(QString("%1/MixerMovingAlarmInRPM").arg(SectionName), m_SettingsData->m_MixerMovingAlarmInRPM);
    settings.setValue(QString("%1/MixerMovingWarningInRPM").arg(SectionName), m_SettingsData->m_MixerMovingWarningInRPM);
    settings.setValue(QString("%1/MaximumErrorCount").arg(SectionName), m_SettingsData->m_MaximumErrorCount);

    SectionName = RegeditDir + "GeneralSettings";
    // settings.setValue(QString("%1/FirstPortIsFirst").arg(SectionName), m_SettingsData->m_FirstPortIsFirst);
    settings.setValue(QString("%1/FirstTriggerOutIsFirst").arg(SectionName), m_SettingsData->m_RightTriggerIsFirst);
    settings.setValue(QString("%1/BandDirectional").arg(SectionName), m_SettingsData->m_BandDirectional);
    settings.setValue(QString("%1/TimeIntervalReadStatusDataInms").arg(SectionName), m_SettingsData->m_TimeIntervalReadStatusDataInms);
    settings.setValue(QString("%1/CameraLiveImageTimeoutInms").arg(SectionName), m_SettingsData->m_CameraLiveImageTimeoutInms);
    settings.setValue(QString("%1/PollingTimeReadNewVideoData").arg(SectionName), m_SettingsData->m_PollingTimeReadNewVideoData);
    settings.setValue(QString("%1/CameraMoveOffsetInX").arg(SectionName), m_SettingsData->m_CameraMoveOffsetInX);
    settings.setValue(QString("%1/CameraMoveOffsetInY").arg(SectionName), m_SettingsData->m_CameraMoveOffsetInY);
    settings.setValue(QString("%1/DeaktivateCheckBottleUnderValve").arg(SectionName), m_SettingsData->m_DeaktivateCheckBottleUnderValve);
    settings.setValue(QString("%1/UseCameraHighRes").arg(SectionName), m_SettingsData->m_UseCameraHighRes);

    SectionName = RegeditDir + "AutoCalibrate";
    settings.setValue(QString("%1/FactorMeasureWindowWidthSearchBottleTopPosiition").arg(SectionName), m_SettingsData->m_FactorMeasureWindowWidthSearchBottleTopPosiition);
    settings.setValue(QString("%1/MinPossibleContrastValueInPercent").arg(SectionName), m_SettingsData->m_MinPossibleContrastValueInPercent);
    settings.setValue(QString("%1/StartValueMeasureWindowHeightInPixel").arg(SectionName), m_SettingsData->m_StartValueMeasureWindowHeightInPixel);
    settings.setValue(QString("%1/NumIterationROIHeight").arg(SectionName), m_SettingsData->m_NumIterationROIHeight);
    settings.setValue(QString("%1/NumIterationCalculteROIPosition").arg(SectionName), m_SettingsData->m_NumIterationCalculteROIPosition);
    settings.setValue(QString("%1/NumIterationCalculateAccseptanceThreshold").arg(SectionName), m_SettingsData->m_NumIterationCalculateAccseptanceThreshold);
    settings.setValue(QString("%1/NumIterationCalculatePixelSize").arg(SectionName), m_SettingsData->m_NumIterationCalculatePixelSize);
    settings.setValue(QString("%1/FactorThreshold").arg(SectionName), m_SettingsData->m_FactorThreshold);
    settings.setValue(QString("%1/DiameterTolInMMInAutoCalibrate").arg(SectionName), m_SettingsData->m_DiameterTolInPercentAutoCalibrate);
    settings.setValue(QString("%1/BottleBaseLineOffsetInPix").arg(SectionName), m_SettingsData->m_BottleBaseLineOffsetInPix);
    settings.setValue(QString("%1/NumIterationROIYPos").arg(SectionName), m_SettingsData->m_NumIterationROIYPos);
    settings.setValue(QString("%1/MinPixelSizeAutocalibration").arg(SectionName), m_SettingsData->m_MinPixelSizeAutocalibration);

    SectionName = RegeditDir + "TrendGraph";
    settings.setValue(QString("%1/TrendGraphIimeRangeIndex").arg(SectionName), m_SettingsData->m_TrendGraphIimeRangeIndex);
    settings.setValue(QString("%1/TrendGraphRollingMeanSizeBottlesPerMin").arg(SectionName), m_SettingsData->m_TrendGraphRollingMeanSizeBottlesPerMin);
    settings.setValue(QString("%1/TrendGraphFlag").arg(SectionName), m_SettingsData->m_TrendGraphFlag);
    settings.setValue(QString("%1/TrendGraphMaxRangeLiquid").arg(SectionName), m_SettingsData->m_TrendGraphMaxRangeLiquid);
    settings.setValue(QString("%1/TrendGraphMinRangeLiquid").arg(SectionName), m_SettingsData->m_TrendGraphMinRangeLiquid);
    settings.setValue(QString("%1/TrendGraphAbsolutMaximumLiquid").arg(SectionName), m_SettingsData->m_TrendGraphAbsolutMaximumLiquid);
    settings.setValue(QString("%1/TrendGraphAbsolutMinimumLiquid").arg(SectionName), m_SettingsData->m_TrendGraphAbsolutMinimumLiquid);
    settings.setValue(QString("%1/TrendGraphAbsolutMaximumTemperature").arg(SectionName), m_SettingsData->m_TrendGraphAbsolutMaximumTemperature);
    settings.setValue(QString("%1/TrendGraphAbsolutMinimumTemperature").arg(SectionName), m_SettingsData->m_TrendGraphAbsolutMinimumTemperature);
    settings.setValue(QString("%1/TrendGraphAbsolutMaximumBottlesPerMin").arg(SectionName), m_SettingsData->m_TrendGraphAbsolutMaximumBottlesPerMin);
    settings.setValue(QString("%1/TrendGraphAbsolutMinimumBottlesPerMin").arg(SectionName), m_SettingsData->m_TrendGraphAbsolutMinimumBottlesPerMin);

    SectionName = RegeditDir + "WaterCoolingPIDParameter";
    settings.setValue(QString("%1/WaterCoolingStrokeMinValue").arg(SectionName), m_SettingsData->m_WaterCoolingStrokeMinValue);
    settings.setValue(QString("%1/WaterCoolingStrokeMaxValue").arg(SectionName), m_SettingsData->m_WaterCoolingStrokeMaxValue);
    settings.setValue(QString("%1/WaterCoolingPFactor").arg(SectionName), m_SettingsData->m_WaterCoolingPFactor);
    settings.setValue(QString("%1/WaterCoolingIFactor").arg(SectionName), m_SettingsData->m_WaterCoolingIFactor);
    settings.setValue(QString("%1/WaterCoolingDFactor").arg(SectionName), m_SettingsData->m_WaterCoolingDFactor);
}

void MainAppCrystalT2::SaveSettings(bool SaveIntoReistry)
{
    QString RegeditDir = REGEDIT_DIR + QString("/");
    if (!SaveIntoReistry) {
        // save into ini-file make a backup from registry and save it to the product path
        RegeditDir = "";
        QString PathNameProducts = m_PathNameProducts + QString("/") + APPLICATION_NAME + ".ini";
        QSettings settings(PathNameProducts, QSettings::IniFormat);
        SetSettings(settings, RegeditDir);
    } else {
        QSettings settings;
        SetSettings(settings, RegeditDir);
    }
}

void MainAppCrystalT2::Initialize()
{
    SlotStartupApplication();
}

void MainAppCrystalT2::Uninitialize()
{
    FinishedRealTimeSystem();
}

QString MainAppCrystalT2::GetComplationDate(QString& FileName)
{
    QString CompilationTime;
    QString AppPath = QCoreApplication::applicationDirPath() + QString("/") + FileName;
    QFileInfo fileInfo(AppPath);
    QString time_format = "MMMM d yyyy  HH:mm:ss";
    QLocale locale(QLocale("en_US"));

    QDateTime dateTime = fileInfo.lastModified();
    return locale.toString(dateTime, time_format);
}

void MainAppCrystalT2::SlotStartupApplication()
{
    int rv = ERROR_CODE_NO_ERROR;
    QString Temp, StartUpInfo, ErrorMsg;
    QString ProductName = "DefaultProduct";
    QString CustomerNumber, NameKernelDLL;
    QString ImageFormatString;

    SetCurrentMaschineState(PluginInterface::MachineState::Initializing);  // pluginOPERATION_STATUS_STARTUP);
    if (GetSettingsData()) {
        ProductName = GetSettingsData()->m_CurrentProductName;        // Zuletzt benutztes Produkt
        CustomerNumber = GetSettingsData()->m_KitharaCustomerNumber;  // Kitharakundennummer
        NameKernelDLL = GetSettingsData()->m_NameKernelDll;           // Name der Dll in dem die Echtzeitfunktionalität implementiert ist
    }
    QString AppName = QCoreApplication::applicationName() + QString(".dll");
    QString compilationTime = GetComplationDate(AppName);
    SlotAddNewMessage(tr("Start Software Load Last Product."), QtMsgType::QtInfoMsg);
    SlotAddNewMessage(tr("Software Windows Part Build Date:%1.").arg(compilationTime), QtMsgType::QtInfoMsg);
    rv = LoadAllProductFiles(ErrorMsg);
    if (rv != ERROR_CODE_NO_ERROR) {  // no Products defined, create default product, wenn kein Produkt vorhanden dann wird automatisch eins angelegt
        ProductName = "DefaultProduct";
        rv = WriteAndInsertNewProduct(ProductName, QString(""), ErrorMsg);
        if (rv != ERROR_CODE_NO_ERROR) {                         // Produkt konnte nicht gespeichert werden
            SlotAddNewMessage(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
        }
        ShowAndSetCurrentProductName(ProductName);  // Anzeige des Produktnamens in der Statusbar
    } else
        ShowAndSetCurrentProductName(GetCurrentProductName());  // Anzeige des Produktnamens in der Statusbar
    if (rv == ERROR_CODE_NO_ERROR) {
        SetCurrentProductDataToImageProcessingUnit();  // Produkt und Produktunabhängige Daten in den Klassenvariablen setzen, für die Übergabe in den Echtzeitbereich vorbereiten
        if (GetKitharaCore() && GetImageData()) {
            SlotAddNewMessage(tr("Software Real Time Part Build Date:%1.").arg(GetComplationDate(NameKernelDLL)), QtMsgType::QtInfoMsg);
            if (GetKitharaCore()->OpenKitharaDriver(CustomerNumber, NameKernelDLL, StartUpInfo, ErrorMsg) == ERROR_CODE_ANY_ERROR)
            //Öffnet den Treiber, erzeugt den Sharedmemory für den Datenaustausch zwischen der Applikation und dem Ectzeitteil, Lädt die Dll in dem die Echtzeitfuntionalität
            // implementiert ist und zeigt die verwendeten CPU's die für die Echtzeit genutzt werden
            {  // Hier entweder kein Treiber installiert Lizenz nicht vorhanden, Kundennumer stimmt nicht oder Sharedmemory konnte nicht angelegt werden
                SlotAddNewMessage(ErrorMsg, QtMsgType::QtFatalMsg);
                SearchSerialPorts(ErrorMsg);
                StartInitRealTimeSystem();
            } else {  // alles ok Treiber, Lizenz, Sharedmemory
                SlotAddNewMessage(StartUpInfo, QtMsgType::QtInfoMsg);
                rv = SearchSerialPorts(ErrorMsg);  // Suche verfügbare Serielle Ports für die Ventile
                if (rv == ERROR_CODE_ANY_ERROR)
                    SlotAddNewMessage(ErrorMsg, QtMsgType::QtFatalMsg);  // Keine Port gefunden bzw. kein Ventil angeschlossen
                else {
                    if (rv == ERROR_CODE_WARNING1) {
                        // Fall wenn für zwei Ventile Konfiguriert aber nur ein ist angeschlossen. Warnig deshalb, da Anlage auch mit nur einem Ventil arbeiten kann
                        SlotAddNewMessage(ErrorMsg, QtMsgType::QtWarningMsg);
                    }
                }
                StartInitRealTimeSystem();  // Init Realtime System
            }
        } else {  // Fehler tritt nie auf da GetKitharaCore()!=0 immer erfüllt ist
            ErrorMsg = tr("Error! RealTime Instance Is Not Allocatete!");
            SlotAddNewMessage(ErrorMsg, QtMsgType::QtFatalMsg);
            StartInitRealTimeSystem();
        }
    } else {  // Fehler tritt nur dann auf wenn Speichern der Datei nicht möglich
        SlotAddNewMessage(ErrorMsg, QtMsgType::QtFatalMsg);
    }
}

void MainAppCrystalT2::SlotInitReady()
{
    int index = -1;

    // GetKitharaCore()->SetDigitalOutput(EtherCATConfigData::DO_CAMERA_LIGHT, true);
    m_ParameterDialogThirdLevelNavigationWidget->insertTab(index, GetGeneralDialog(), tr("General Settings"));
    index = 0;

    // m_ParameterDialogThirdLevelNavigationWidget->insertTab(1, GetGeneralDialog(), tr("Quick Coupling"));
    SettingsData* pSettingsData = GetSettingsData();
    if (pSettingsData) {
        if (m_NumberSerialPortsFound > 0) {
            if (pSettingsData->m_WorkWithTwoValves) {
                GetValveDialogLeft()->SetGroupName();
                GetValveDialogRight()->SetGroupName();
                m_ParameterDialogThirdLevelNavigationWidget->insertTab(++index, GetValveDialogLeft(), GetValveDialogLeft()->GetValveName());
                m_ParameterDialogThirdLevelNavigationWidget->insertTab(++index, GetValveDialogRight(), GetValveDialogRight()->GetValveName());
            } else {  // One Port in use -> Only one Valve in use
                GetValveDialogLeft()->SetGroupName();
                m_ParameterDialogThirdLevelNavigationWidget->insertTab(++index, GetValveDialogLeft(), GetValveDialogLeft()->GetValveName());
            }
        } else {
            GetValveDialogLeft()->SetValveName(QString(""));
            GetValveDialogLeft()->SetValveID(LEFT_VALVE_ID);
        }

        m_ParameterDialogThirdLevelNavigationWidget->insertTab(++index, GetMaintenanceDialog(), tr("Drop Volume"));
        m_ParameterDialogThirdLevelNavigationWidget->insertTab(++index, GetQuickCouplingDialog(), tr("Quick Coupling"));
        m_ParameterDialogThirdLevelNavigationWidget->insertTab(++index, GetCoolingDialog(), tr("Cooling"));
        StartTimerReadStatusData(pSettingsData->m_TimeIntervalReadStatusDataInms);
        StartTimerIntervalCleanImage(pSettingsData->m_TimerIntervalCheckCleanImageInMin);
        StartTimerUpdateTrendGraph(pSettingsData->m_IntervalUpdateTrendGraph);
        StartTimerCalculatBottlesPerMinute();
        StartTimerCheckDiskOverflow(1.0);  // min
        StartTimerSaveAuditTrailData();
    }

    int UsedTriggerOutputs = GetUsedTriggerOutput();
    if (GetOverviewDialog()) {
        if (UsedTriggerOutputs == USE_BOTH_VALVES)  // use both
        {
            GetOverviewDialog()->ShowCheckBoxFirstTrigger(true);
            GetOverviewDialog()->ShowCheckBoxSecondTrigger(true);
        } else {
            if (UsedTriggerOutputs == USE_ONLY_LEFT_VALVE)  // use first
            {
                GetOverviewDialog()->ShowCheckBoxFirstTrigger(true);
                GetOverviewDialog()->ShowCheckBoxSecondTrigger(false);
                GetOverviewDialog()->CheckCheckBoxFirstTrigger(true);
            } else {
                if (UsedTriggerOutputs == USE_ONLY_RIGHT_VALVE)  // use second
                {
                    GetOverviewDialog()->ShowCheckBoxFirstTrigger(false);
                    GetOverviewDialog()->ShowCheckBoxSecondTrigger(true);
                    GetOverviewDialog()->CheckCheckBoxSecondTrigger(true);
                }
            }
        }
    }

    SetCurrentMaschineState(PluginInterface::MachineState::Off);                             // Hier Ende der Initialisierung
    GetCrystalT2Plugin()->SetStartupInitReady(true);                                         // Registerkarten sichtbar machen
    QTimer::singleShot(2000, this, &MainAppCrystalT2::SlotDelayStartDataExchangeVideoData);  // verzögerte Start, damit MDIWidget/Multriggeranzeige die richtige Höhe und Breite besitzt
}

void MainAppCrystalT2::SlotDelayStartDataExchangeVideoData()
{
    if (GetWaitForNewVideo()) {
        GetWaitForNewVideo()->StartWaitForNextVideo();
    }
    if (GetSettingsData()->m_WorkWithoutCamera) {
        if (GetEditProductDialog()) {
            GetEditProductDialog()->SlotRunCameraSimulation();
        }
    }
}

bool MainAppCrystalT2::IsStartupInitReady()
{
    if (GetCrystalT2Plugin())
        return GetCrystalT2Plugin()->IsStartupInitReady();
    else
        return false;
}

int MainAppCrystalT2::GetUsedTriggerOutput()
{
    ProductData* pProductData = GetCurrentProductData();
    if (pProductData) {
        return pProductData->m_UsedTriggerOutputs;
    } else {
        return USE_BOTH_VALVES;
    }
}

bool MainAppCrystalT2::IsFirstTriggerOnLeftSide()
{
    if (GetSettingsData()->m_BandDirectional == BAND_DIRECTIONAL_LEFT_TO_RIGHT)
        return true;
    else
        return false;
}

void MainAppCrystalT2::SlotShowLiveImage(const ImageMetaData& Image)
{
    if (GetImageData()) {
        // double TimeInms = (Image.m_CurrentMeasuringResult.m_CurrentTimeStampInns - Image.m_CurrentMeasuringResult.m_LastTimeStampInns) / ((double)(1000000.0));
        // double  FPS = 0.0;
        double Factor = 1.4142 / 2.0;  // Wurzel 2 halbe
        double realTime;

        m_CurrentCounterProductOk = Image.m_CurrentMeasuringResultsLiqiud.m_CounterBottleFilled;
        if (GetOverviewDialog()) {
            GetOverviewDialog()->SetCounterProductNotFilled(Image.m_CurrentMeasuringResultsLiqiud.m_CounterBottleNotFilled);
            GetOverviewDialog()->SetCounterProductOk(Image.m_CurrentMeasuringResultsLiqiud.m_CounterBottleFilled);
        }
        if (GetAdvancedSettingsDialog()) {
            if (GetAdvancedSettingsDialog()->isVisible()) {
                GetAdvancedSettingsDialog()->SetInspectionTime(Image.m_CurrentMeasuringResult.m_MeasuringTimeInms);
                GetAdvancedSettingsDialog()->SetProductPresentTime(GetImageData()->GetProductPresentTime(Image.m_CurrentMeasuringResult.m_CurrentSpeedInmmPerms * 1000.0));
                realTime = (Image.m_CurrentMeasuringResult.m_CurrentTimeStampInns - Image.m_CurrentMeasuringResult.m_LastTimeStampInns) / ((double)(1000000));
                GetAdvancedSettingsDialog()->SetRealTimeInterval(realTime);
            }
            if (Image.m_CurrentMeasuringResult.m_CurrentSpeedInmmPerms <= 0.0)
                GetAdvancedSettingsDialog()->SetSpeed(-1.0);  // noch nicht berechnet
            else
                GetAdvancedSettingsDialog()->SetSpeed(Image.m_CurrentMeasuringResult.m_CurrentSpeedInmmPerms * 60.0);  // in m/min
        }
        if (GetSettingsData() && GetEditProductDialog()) {
            if (GetEditProductDialog()->isVisible()) {
                if (Image.m_CurrentMeasuringResultsLiqiud.m_StatusResults) {
                    bool UseLeftTrigger = true;
                    bool UseRightTrigger = true;
                    ProductData* pProductData = GetCurrentProductData();
                    if (pProductData) {
                        if (pProductData->m_UsedTriggerOutputs == USE_ONLY_LEFT_VALVE) {
                            UseRightTrigger = false;
                        }
                        if (pProductData->m_UsedTriggerOutputs == USE_ONLY_RIGHT_VALVE) {
                            UseLeftTrigger = false;
                        }
                    }
                    if (UseLeftTrigger && UseRightTrigger) {
                        if (IsFirstTriggerOnLeftSide()) {
                            GetEditProductDialog()->SetAmountOfLiquidMiddleLeftTriggerSide(Image.m_CurrentMeasuringResultsLiqiud.m_MeanValueAmountMiddleFirstTrigger,
                                                                                           Image.m_CurrentMeasuringResultsLiqiud.m_StandardDeviationMiddleFirstTrigger,
                                                                                           Image.m_CurrentMeasuringResultsLiqiud.m_MeanValueAmountMiddle);
                            GetEditProductDialog()->SetAmountOfLiquidMiddleRightTriggerSide(Image.m_CurrentMeasuringResultsLiqiud.m_MeanValueAmountMiddleSecondTrigger,
                                                                                            Image.m_CurrentMeasuringResultsLiqiud.m_StandardDeviationMiddleSecondTrigger,
                                                                                            Image.m_CurrentMeasuringResultsLiqiud.m_MeanValueAmountMiddle);
                        } else {
                            GetEditProductDialog()->SetAmountOfLiquidMiddleLeftTriggerSide(Image.m_CurrentMeasuringResultsLiqiud.m_MeanValueAmountMiddleSecondTrigger,
                                                                                           Image.m_CurrentMeasuringResultsLiqiud.m_StandardDeviationMiddleSecondTrigger,
                                                                                           Image.m_CurrentMeasuringResultsLiqiud.m_MeanValueAmountMiddle);
                            GetEditProductDialog()->SetAmountOfLiquidMiddleRightTriggerSide(Image.m_CurrentMeasuringResultsLiqiud.m_MeanValueAmountMiddleFirstTrigger,
                                                                                            Image.m_CurrentMeasuringResultsLiqiud.m_StandardDeviationMiddleFirstTrigger,
                                                                                            Image.m_CurrentMeasuringResultsLiqiud.m_MeanValueAmountMiddle);
                        }
                        GetEditProductDialog()->SetAmountOfSplashesLeftOnLeftTriggerSide(Image.m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashLeftROIPerSide[LEFT_TRIGGER_SIDE_INDEX]);
                        GetEditProductDialog()->SetAmountOfSplashesRightOnLeftTriggerSide(Image.m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashRightROIPerSide[LEFT_TRIGGER_SIDE_INDEX]);

                        GetEditProductDialog()->SetAmountOfSplashesLeftOnRightTriggerSide(Image.m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashLeftROIPerSide[RIGHT_TRIGGER_SIDE_INDEX]);
                        GetEditProductDialog()->SetAmountOfSplashesRightOnRightTriggerSide(Image.m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashRightROIPerSide[RIGHT_TRIGGER_SIDE_INDEX]);
                    } else {
                        if (UseLeftTrigger) {
                            GetEditProductDialog()->SetAmountOfLiquidMiddleLeftTriggerSide(Image.m_CurrentMeasuringResultsLiqiud.m_MeanValueAmountMiddle,
                                                                                           Image.m_CurrentMeasuringResultsLiqiud.m_StandardDeviationMiddle,
                                                                                           Image.m_CurrentMeasuringResultsLiqiud.m_MeanValueAmountMiddle);
                            GetEditProductDialog()->SetAmountOfSplashesLeftOnLeftTriggerSide(Image.m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashLeftROI);
                            GetEditProductDialog()->SetAmountOfSplashesRightOnLeftTriggerSide(Image.m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashRightROI);
                        } else {
                            GetEditProductDialog()->SetAmountOfLiquidMiddleRightTriggerSide(Image.m_CurrentMeasuringResultsLiqiud.m_MeanValueAmountMiddle,
                                                                                            Image.m_CurrentMeasuringResultsLiqiud.m_StandardDeviationMiddle,
                                                                                            Image.m_CurrentMeasuringResultsLiqiud.m_MeanValueAmountMiddle);
                            GetEditProductDialog()->SetAmountOfSplashesLeftOnRightTriggerSide(Image.m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashLeftROI);
                            GetEditProductDialog()->SetAmountOfSplashesRightOnRightTriggerSide(Image.m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashRightROI);
                        }
                    }
                }
            }  // if (GetEditProductDialog()->isVisible())
            GetEditProductDialog()->SetCounterBottleFilled(Image.m_CurrentMeasuringResultsLiqiud.m_CounterBottleFilled);
            GetEditProductDialog()->SetCounterBottleNotFilled(Image.m_CurrentMeasuringResultsLiqiud.m_CounterBottleNotFilled);
            GetEditProductDialog()->SetCounterMiddleTooLow(Image.m_CurrentMeasuringResultsLiqiud.m_CounterMiddleNotOk);
            GetEditProductDialog()->SetCounterLeftTooBig(Image.m_CurrentMeasuringResultsLiqiud.m_CounterLeftTooBig);
            GetEditProductDialog()->SetCounterRightTooBig(Image.m_CurrentMeasuringResultsLiqiud.m_CounterRightTooBig);
            GetEditProductDialog()->SetCounterBottleNotInPos(Image.m_CurrentMeasuringResultsLiqiud.m_CounterBottleNotUnderValve);

            GetEditProductDialog()->SetCounterContrastIsOutOfTol(Image.m_CurrentMeasuringResult.m_CounterContrastOutOfTol);
            GetEditProductDialog()->SetCounterSizeIsOutOfTol(Image.m_CurrentMeasuringResult.m_CounterSizeIsLongTimeOutOfTol);
            GetEditProductDialog()->SetCounterEdgeIsOutOfTol(Image.m_CurrentMeasuringResult.m_CounterEdgeIsLongTimeOutOfTol);

            GetEditProductDialog()->SetMeasuredNeckDiameter(Image.m_AveragedMeasuringResults.m_MeasuredBottleNeckDiameterInmm, Image.m_AveragedMeasuringResults.m_StatusResults);

            CheckBottleNotFiledAndDropletVolumeOneAfterTheOther(Image.m_CurrentMeasuringResultsLiqiud.m_CounterBottleNotFilledOneAfterTheOther,
                                                                Image.m_CurrentMeasuringResultsLiqiud.m_CounterMiddleNotOkOneAfterTheOther);
        }

        if (GetAdvancedSettingsDialog() && GetAdvancedSettingsDialog()->isVisible()) {
            GetAdvancedSettingsDialog()->SetMeasuredPixelSize(Image.m_AveragedMeasuringResults.m_CalculatePixelSizeInMMPerPixel, Image.m_AveragedMeasuringResults.m_StatusResults);
            GetAdvancedSettingsDialog()->SetMeasuredNeckDiameter(Image.m_AveragedMeasuringResults.m_MeasuredBottleNeckDiameterInmm, Image.m_AveragedMeasuringResults.m_StatusResults);
        }

        if (GetLiveImageView()) {
            GetLiveImageView()->setImage(Image);
        }
        if (Image.m_CurrentMeasuringResultsLiqiud.m_StatusResults) {
            if (CanUpdateTrendGraph()) {
                MeasuringResultsLiquidWithDateTime LiquidData;
                // if (m_ListMeasuringResultsLiquid.size() > 100) m_ListMeasuringResultsLiquid.pop_front();
                LiquidData.m_MeasuringResultsLiquid = Image.m_CurrentMeasuringResultsLiqiud;
                LiquidData.m_CurrentTime = QTime::currentTime();
                LiquidData.m_BottlesPerMinute = GetProductsPerMin();
                // if (GetAdminSettingsDialog() && GetAdminSettingsDialog()->isVisible()) {
                //     GetAdminSettingsDialog()->SetBottlesPerMiniute(LiquidData.m_BottlesPerMinute);
                // }
                // if (GetOverviewDialog()) GetOverviewDialog()->SetBottlesPerMiniute(LiquidData.m_BottlesPerMinute);
                // m_ListMeasuringResultsLiquid.push_back(LiquidData);
                SaveTrendGraphData(LiquidData);
            }
        }
        CheckProductCenterTolerance(Image.m_CurrentMeasuringResult.m_CurrentSpeedInmmPerms);
    }
}

bool MainAppCrystalT2::CanUpdateTrendGraph()
{
    if (GetCurrentMaschineState() == PluginInterface::MachineState::Production || GetCurrentMaschineState() == PluginInterface::MachineState::Setup) {
        return true;
    } else {
        return false;
    }
}

void MainAppCrystalT2::CheckBottleNotFiledAndDropletVolumeOneAfterTheOther(int countBottleNotFilledOneAfterTheOther, int countMiddleNotOkOneAfterTheOther)
{
    if (GetSettingsData()) {
        if (countBottleNotFilledOneAfterTheOther >= GetSettingsData()->m_MaxCounterBottleEjectedOneAfterTheOther) {
            m_StatusCounterBottleEjectedOneAfterTheOther = ALARM_LEVEL_MASCHINE_STOP;
        } else {
            m_StatusCounterBottleEjectedOneAfterTheOther = ALARM_LEVEL_OK;
        }
        if (countMiddleNotOkOneAfterTheOther >= GetSettingsData()->m_MaxCounterMiddleTooLowOneAfterTheOther) {
            m_StatusCounterMiddleTooLowOneAfterTheOther = ALARM_LEVEL_MASCHINE_STOP;
        } else {
            m_StatusCounterMiddleTooLowOneAfterTheOther = ALARM_LEVEL_OK;
        }
    }
}

void MainAppCrystalT2::CheckProductCenterTolerance(double CurrentSpeed)
{
    if (GetCurrentProductData()) {
        double Factor = 1.4142 / 2.0;  // Wurzel 2 halbe
        double InnerSquare = GetCurrentProductData()->m_BottleNeckInnerDiameter * Factor;
        ValveData valveDataLeftValve = GetCurrentProductData()->GetValveData(LEFT_VALVE_ID);
        double LeftValvePulseDistanceInMM = CurrentSpeed * valveDataLeftValve.m_ValvePulseTimeInms * Factor;

        if (LeftValvePulseDistanceInMM > InnerSquare) {
            QString Warning = GetWarningMessagePulseTooBig(tr("Left Valve"), valveDataLeftValve.m_ValvePulseTimeInms, InnerSquare, CurrentSpeed);
            SlotAddNewMessage(Warning, QtMsgType::QtWarningMsg);
        }
        if (GetSettingsData() && GetSettingsData()->m_WorkWithTwoValves) {
            ValveData valveDataRightValve = GetCurrentProductData()->GetValveData(RIGHT_VALVE_ID);
            double RightValvePulseDistanceInMM = CurrentSpeed * valveDataRightValve.m_ValvePulseTimeInms * Factor;
            if (RightValvePulseDistanceInMM > InnerSquare) {
                QString Warning = GetWarningMessagePulseTooBig(tr("Right Valve"), valveDataRightValve.m_ValvePulseTimeInms, InnerSquare, CurrentSpeed);
                SlotAddNewMessage(Warning, QtMsgType::QtWarningMsg);
            }
        }
    }
}

QString MainAppCrystalT2::GetWarningMessagePulseTooBig(const QString& Valve, double Pulse, double InnerSquare, double Speed)
{
    return tr("Injection Time %1 Too Big! (%2ms) > (Inner Square(%3mm)/Speed(%4mm/ms))").arg(Valve).arg(Pulse, 0, 'f', 1).arg(InnerSquare).arg(Speed, 0, 'f', 2);
}

void MainAppCrystalT2::SlotIODeviceMessage(const QString& Info, int InfoType)
{
    QStringList listInfo;
    switch (InfoType) {
        case INFO_CODE_INPUT_OUTPUT_DEVICE_BLOW_OUT_EJECTOR_IS_ACTIVE:
            break;
        case INFO_CODE_INPUT_OUTPUT_DEVICE_LEFT_TRIGGER_IS_SET:
            if (GetSimulateValve()) {
                GetSimulateValve()->LeftTriggerOn();  // info to virtual valve device
                GetSimulateValve()->StartResetTriggerLeft();
            }
            if (GetMaintenanceDialog() && /*GetMaintenanceDialog()->isVisible() &&*/ GetMaintenanceDialog()->IsManualTriggerActive()) {
                GetMaintenanceDialog()->IncrementTriggerCounterReadingLeftValve();
            }
            break;
        case INFO_CODE_INPUT_OUTPUT_DEVICE_RIGHT_TRIGGER_IS_SET:
            if (GetSimulateValve()) {
                GetSimulateValve()->RightTriggerOn();  // info to virtual valve device
                GetSimulateValve()->StartResetTriggerRight();
            }
            if (GetMaintenanceDialog() && /*GetMaintenanceDialog()->isVisible() &&*/ GetMaintenanceDialog()->IsManualTriggerActive()) {
                GetMaintenanceDialog()->IncrementTriggerCounterReadingRightValve();
            }
            break;
        case INFO_CODE_INPUT_OUTPUT_REAL_TIME_TASKS_RUNS:
            emit SignalShowInfo(Info, QtMsgType::QtInfoMsg);
            break;
        case INFO_CODE_INPUT_OUTPUT_DEVICE_ANY_ERROR:
            emit SignalShowInfo(Info, QtMsgType::QtFatalMsg);
            break;
        case INFO_CODE_INPUT_DEVICE_KITHARA_ERROR_CODE:
            emit SignalShowInfo(Info, QtMsgType::QtFatalMsg);
            break;
        case INFO_CODE_ETHER_CAT_CYCLUS:
            if (GetIOSettingsDialog() && GetIOSettingsDialog()->isVisible()) GetIOSettingsDialog()->SetEtherCatCyclus(Info);
            break;
        case INFO_CODE_INPUT_CONTROL_VOLTAGE_NOT_OK:
            emit SignalShowInfo(Info, QtMsgType::QtFatalMsg);
            break;
        case INFO_CODE_SPEED_FROM_IS:
            if (GetAdvancedSettingsDialog() && GetAdvancedSettingsDialog()->isVisible()) GetAdvancedSettingsDialog()->SetSpeedFromIS(Info.toDouble());
            break;
        case INFO_CODE_PREASURE_TANK_TEMP:
            if (GetOverviewDialog()) {
                GetOverviewDialog()->SetLiquidTankTemp(Info.toDouble());
            }
            break;
        case INFO_CODE_HEATING_PIPE_TEMP:
            if (GetOverviewDialog()) {
                GetOverviewDialog()->SetHeatingPipeTemperature(Info.toDouble());
            }
            break;
        case INFO_CODE_ACTUAL_LIQUID_TANK_PREASURE:
            if (GetOverviewDialog()) {
                GetOverviewDialog()->SetLiquidTankPreasure(Info.toDouble());
            }
            if (GetIOSettingsDialog() && GetIOSettingsDialog()->isVisible()) {
                GetIOSettingsDialog()->SetActualPreasureValue(Info.toDouble());
            }
            break;
        case INFO_CODE_PREASURE_TANK_FILLING_LEVEL:
            if (GetOverviewDialog()) {
                GetOverviewDialog()->SetLiquidTankFillingLevel(Info.toDouble());
            }
            if (GetIOSettingsDialog() && GetIOSettingsDialog()->isVisible()) {
                GetIOSettingsDialog()->SetActualTankFillingLevel(Info.toDouble());
            }
            break;
        case INFO_CODE_ACTUAL_AIR_COOLING_CAMERA:
            if (GetIOSettingsDialog() && GetIOSettingsDialog()->isVisible()) {
                GetIOSettingsDialog()->SetActualAirCoolingCamera(Info.toDouble());
            }
            if (GetCoolingDialog()) {
                GetCoolingDialog()->SetActualAirCoolingCamera(Info.toDouble());
            }
            break;
        case INFO_CODE_ACTUAL_AIR_COOLING_LIGHT:
            if (GetIOSettingsDialog() && GetIOSettingsDialog()->isVisible()) {
                GetIOSettingsDialog()->SetActualAirCoolingCameraLight(Info.toDouble());
            }
            if (GetCoolingDialog()) {
                GetCoolingDialog()->SetActualAirCoolingCameraLight(Info.toDouble());
            }
            break;
        case INFO_CODE_ACTUAL_AIR_COOLING_VALVE:
            if (GetIOSettingsDialog() && GetIOSettingsDialog()->isVisible()) {
                GetIOSettingsDialog()->SetActualAirCoolingValves(Info.toDouble());
            }
            if (GetCoolingDialog()) {
                GetCoolingDialog()->SetActualAirCoolingValves(Info.toDouble());
            }
            break;
        case INFO_CODE_ACTUAL_WATER_COOLING:
            if (GetIOSettingsDialog() && GetIOSettingsDialog()->isVisible()) {
                GetIOSettingsDialog()->SetActualWaterCooling(Info.toDouble());
            }
            if (GetCoolingDialog()) {
                GetCoolingDialog()->SetActualWaterCooling(Info.toDouble());
            }
            break;
        case INFO_CODE_ACTUAL_FLOW_TRANSMITTER_WATER_COOLING_CIRCUIT:
            if (GetIOSettingsDialog() && GetIOSettingsDialog()->isVisible()) {
                GetIOSettingsDialog()->SetActualFlowTransmitterWaterCoolingCircuit(Info.toDouble());
            }
            if (GetCoolingDialog()) {
                GetCoolingDialog()->SetActualFlowTransmitterWaterCoolingCircuit(Info.toDouble());
            }
            break;
        case INFO_CODE_ACTUAL_TEMPERATURE_WATER_COOLING_RETURN:
            m_ActualTemperaturWaterCoolingReturn = Info.toDouble();
            if (GetIOSettingsDialog() && GetIOSettingsDialog()->isVisible()) {
                GetIOSettingsDialog()->SetActualTemperaturWaterCoolingReturn(m_ActualTemperaturWaterCoolingReturn);
            }
            if (GetCoolingDialog()) {
                GetCoolingDialog()->SetActualTemperaturWaterCoolingReturn(m_ActualTemperaturWaterCoolingReturn);
            }
            break;
        case INFO_CODE_MANUAL_EJECTION_READY:
            if (GetGeneralDialog()) {
                GetGeneralDialog()->SetManualEjectionReady();
            }
            break;
        case INFO_CODE_COUNTER_REAL_EJECTED:
            if (GetOverviewDialog()) {
                GetOverviewDialog()->SetCounterNumberBottlesRealEjected(Info.toDouble());
            }
            break;
        case INFO_CODE_STATUS_MIXER:
            listInfo = Info.split("|");
            if (listInfo.count() == 4) {
                m_StatusMixer = static_cast<ushort>(listInfo[0].toInt());
                m_ENCStatusMixer = static_cast<ushort>(listInfo[1].toInt());
                if (GetSettingsData()->m_WorkWithoutEtherCat) {  // nur für testzwecke!!!!
                    if (GetIOSettingsDialog()) {
                        m_ActualVelocityMixerRPM = GetIOSettingsDialog()->GetMixerSpeedSimulationInRPM();
                    }
                } else {
                    m_ActualVelocityMixerRPM = GetMixerTerminalValueToRotationPerMinute(listInfo[2].toDouble());
                }
                if (GetIOSettingsDialog() && GetIOSettingsDialog()->isVisible()) {
                    GetIOSettingsDialog()->SetENCStatusMixer(m_ENCStatusMixer);
                    GetIOSettingsDialog()->SetStatusMixer(m_StatusMixer);
                }
                if (GetKitharaCore() && GetKitharaCore()->IsMixerOn()) {
                    // ready to enable               //ready                    //error                 //moving positiv
                    if (!(m_StatusMixer & 0x1) || !(m_StatusMixer & 0x2) || (m_StatusMixer & 0x8) || !(m_StatusMixer & 0x10)) {
                        m_NumberErrorMixer++;
                        if (m_NumberErrorMixer > 10) {  
                            m_MixerIsRunning = false;
                        } else {
                            m_MixerIsRunning = true;
                        }
                    } else {
                        // Status input a       Status inpiut b     Status input c
                        if (!(m_ENCStatusMixer & 0x100) && !(m_ENCStatusMixer & 0x200) && !(m_ENCStatusMixer & 0x400)) {
                            m_NumberErrorMixer++;
                            if (m_NumberErrorMixer > 15) {  // seit 7.5 sekunden keine Spannung am Motor
                                m_MixerIsRunning = false;
                            } else {
                                m_MixerIsRunning = true;
                            }
                        } else {
                            m_MixerIsRunning = true;
                            m_NumberErrorMixer = 0;
                        }
                    }
                }
                if (GetIOSettingsDialog()) {
                    GetIOSettingsDialog()->SetActualVelocityMixerRPM(m_ActualVelocityMixerRPM);
                }
            }
            break;
        default:
            break;
    }
}

void MainAppCrystalT2::SlotSetTriggerStateOff(int ValveID)
{
    if (GetAdvancedSettingsDialog()) {
        GetAdvancedSettingsDialog()->SetTriggerState(TRIGGER_OFF, ValveID);
    }
}

// MODE = OFF
// SET = 055.3C
// ACT = 031.5C
// STACK = 031.1C

void MainAppCrystalT2::SlotShowValveTemperature(const QString& StatusHeaters, int ValveID)
{
    QString Line, Status = StatusHeaters;
    QStringList list;
    QString TagACT = "ACT";
    QString TagSTACK = "STACK";
    QRegExp space("\\s");
    double CurrentTemp = 0.0;
    double StackTemp = 0.0;
    bool TagACTFound = false;
    bool TagSTACKFound = false;

    Status = Status.remove(Status.length() - RESPONSE_END_TAG.length(), RESPONSE_END_TAG.length());  // remove end tag(<3)
    list = Status.split('\n');
    for (int i = 0; i < list.count(); i++) {
        Line = list.at(i);
        if (Line.contains(TagACT)) {
            Line = Line.remove(space);                   // remove space
            Line = Line.remove(Line.length() - 1, 1);    // remove last char 'C'
            Line = Line.remove(0, TagACT.length() + 1);  // remove text and '='
            CurrentTemp = Line.toDouble();
            TagACTFound = true;
        } else {
            if (Line.contains(TagSTACK)) {
                Line = Line.remove(space);
                Line = Line.remove(Line.length() - 1, 1);  // remove last
                Line = Line.remove(0, TagSTACK.length() + 1);
                StackTemp = Line.toDouble();
                TagSTACKFound = true;
            }
        }
    }
    if (GetOverviewDialog() && TagSTACKFound && TagACTFound) {
        Line = "";
        if (GetSettingsData()) {
            GetOverviewDialog()->SetValveChamberTemp(CurrentTemp, ValveID);
            GetOverviewDialog()->SetValvePiezoTemp(StackTemp, ValveID);
        }
        if (ValveID == LEFT_VALVE_ID) {
            m_StackTemparatureLeftValve = StackTemp;      // um die Temparaturdaten zu speichern und im Trendgraph anzuzeigen
            m_CurrentTemparatureLeftValve = CurrentTemp;  // um die Temparaturdaten zu speichern und im Trendgraph anzuzeigen

        } else {
            m_StackTemparatureRightValve = StackTemp;      // um die Temparaturdaten zu speichern und im Trendgraph anzuzeigen
            m_CurrentTemparatureRightValve = CurrentTemp;  // um die Temparaturdaten zu speichern und im Trendgraph anzuzeigen
        }
    }
}

void MainAppCrystalT2::StartTimerReadStatusData(double setInms)
{
    if (GetTimerGetStatusHardwareDevice()) {
        GetTimerGetStatusHardwareDevice()->setInterval(static_cast<int>(setInms));
        GetTimerGetStatusHardwareDevice()->start();
    }
}

void MainAppCrystalT2::StartTimerIntervalCleanImage(double setInmin)
{
    if (GetTimerCheckCleanImage()) {
        GetTimerCheckCleanImage()->setInterval(static_cast<int>(setInmin * 60 * 1000));
        GetTimerCheckCleanImage()->start();
    }
}

void MainAppCrystalT2::StartTimerUpdateTrendGraph(double setInmin)
{
    if (GetTimerDrawTrendGraphData()) {
        GetTimerDrawTrendGraphData()->setInterval(static_cast<int>(setInmin * 60 * 1000));
        GetTimerDrawTrendGraphData()->start();
    }
}

void MainAppCrystalT2::StartTimerCheckDiskOverflow(double setInmin)
{
    if (GetTimerCheckDiskOverflow()) {
        GetTimerCheckDiskOverflow()->setInterval(static_cast<int>(setInmin * 60 * 1000));
        GetTimerCheckDiskOverflow()->start();
    }
}

void MainAppCrystalT2::StartTimerCalculatBottlesPerMinute()
{
    if (GetTimerCalculatBottlesPerMinute()) {
        GetTimerCalculatBottlesPerMinute()->setInterval(static_cast<int>(1000));
        GetTimerCalculatBottlesPerMinute()->start();
    }
}

void MainAppCrystalT2::StartTimerSaveAuditTrailData()
{
    double timeInMin = 5.0;
    if (GetTimerSaveAuditTrailData()) {
        GetTimerSaveAuditTrailData()->setInterval(static_cast<int>(timeInMin * 60 * 1000));
        GetTimerSaveAuditTrailData()->start();
    }
}

// wird beim Starten der Software im Thread aufgerufen, wenn der Kitharatreiber, die Dll mit der Echtzeitfunktionalität erfolgreich geladen wurde und der Sharedmemory für die Kommunikation
// zwischen Application und Echtzeitteil vorhanden ist
void MainAppCrystalT2::StartInitRealTimeSystem()
{
    if (GetImageData()) {
        QString QInfo, ErrorMsg;
        int rv = ERROR_CODE_NO_ERROR;

        if (GetImageData()->IsKitharaDriverOpen()) {
            QInfo = tr("Start Initialisation Real Time Device, Please Wait ....");
            emit SignalShowInfo(QInfo, QtMsgType::QtInfoMsg);
            GetImageData()->InitRealTimeSystem(ErrorMsg);
            if (GetImageData()->IsRealTimeCameraInitialised()) {
                if (GetWaitForNewVideo()) {  // allocate windows shared memory for video
                    rv = GetWaitForNewVideo()->AllocateSharedMemoryVideoData(GetImageData()->GetImageWidth(), GetImageData()->GetImageHeight(), ErrorMsg);
                    if (rv != ERROR_CODE_NO_ERROR) {
                        emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);
                    }
                }
                if (GetEjectedBottlesDialog()) {
                    GetEjectedBottlesDialog()->StartWaitForBottlesEjected();
                }
                // start camera
                SlotStartImageAcquisition();
                // start realtime tasks
                GetImageData()->GetKitharaCore()->StartRealTimeTasks();
                // start live image thread
                if (GetImageData() && !GetImageData()->isRunning()) {
                    GetImageData()->start();
                }

                if (GetSettingsData()) {
                    if (GetSettingsData()->m_CameraLightOnOnStartup) {
                        QInfo = tr("Set Camera Light On");
                        emit SignalShowInfo(QInfo, QtMsgType::QtInfoMsg);
                        GetKitharaCore()->SetDigitalOutput(EtherCATConfigData::DO_CAMERA_LIGHT, true);
                    }
                    /*if (GetSettingsData()->m_PreasureTankValveOnOnStartup) {
                        QInfo = tr("Set Temperature Tank Preasure On");
                        emit SignalShowInfo(QInfo, QtMsgType::QtInfoMsg);
                        GetKitharaCore()->SetDigitalOutput(EtherCATConfigData::DO_PREASURE_TANK_VALVE, true);
                    }*/
                    if (GetSettingsData()->m_ValveControllerOnOnStartup) {
                        QInfo = tr("Set Valve Controller On");
                        emit SignalShowInfo(QInfo, QtMsgType::QtInfoMsg);
                        GetKitharaCore()->SetDigitalOutput(EtherCATConfigData::DO_VALVE_CONTROLLER, true);
                    }
                    if (GetSettingsData()->m_WhiteLightOnOnStartup) {
                        QInfo = tr("Set White Light On");
                        emit SignalShowInfo(QInfo, QtMsgType::QtInfoMsg);
                        GetKitharaCore()->SetDigitalOutput(EtherCATConfigData::DO_WHITE_LIGHT, true);
                    }

                    if (GetSettingsData()->m_WorkWithoutEtherCat) {  // nur für testzwecke!!!!
                        if (GetIOSettingsDialog()) GetIOSettingsDialog()->SetTestDefaultParameter();
                    }

                    ProductData* pProductData = GetCurrentProductData();
                    if (pProductData) {
                        SetDefaultPreasure(pProductData->m_DefaultPreasureInBar, false);
                    }
                    m_CurrentWaterCoolingStrokeValue = GetSettingsData()->m_WaterCoolingStrokeMinValue;
                    SetWaterCoolingDefault(GetSettingsData()->m_WaterCoolingStrokeMinValue);
                    SetAirCoolingCamera();
                    SetAirCoolingLight();
                    SetAirCoolingValve();
                    /*if (GetSettingsData()->m_WorkWithoutCamera) {
                        if (GetEditProductDialog()){
                            GetEditProductDialog()->SlotRunCameraSimulation();
                        }
                    }*/
                }
            } else {
                QInfo = tr("Error! Can Not Initialised Real Time System");
                emit SignalShowInfo(QInfo, QtMsgType::QtInfoMsg);
                if (!GetImageData()->isRunning()) GetImageData()->start();
                GetImageData()->SetRealTimeCameraInitialised(false);
            }
        } else {
            QInfo = tr("Error! Can Not Open Real Time Driver. No License");
            emit SignalShowInfo(QInfo, QtMsgType::QtInfoMsg);
            if (!GetImageData()->isRunning()) GetImageData()->start();
            GetImageData()->SetRealTimeCameraInitialised(false);
        }
        InitSerialPortsAndSetParameter();
        emit SignalInitReady();  // Starte Timer und setze Maschinestate auf off
    }
}

void MainAppCrystalT2::SetPreasureOnOff(bool set)
{
    if (set) {
        ProductData* pProductData = GetCurrentProductData();
        if (pProductData) {
            SetDefaultPreasure(pProductData->m_DefaultPreasureInBar, false);
        }
        if (GetKitharaCore()) {
            GetKitharaCore()->SetDigitalOutput(EtherCATConfigData::DO_PREASURE_TANK_VALVE, true);
        }
        m_IsPowerAndPreasureOnValve = true;
    } else {
        SetDefaultPreasure(0.0, false);
        if (GetKitharaCore()) {
            GetKitharaCore()->SetDigitalOutput(EtherCATConfigData::DO_PREASURE_TANK_VALVE, false);
        }
        m_IsPowerAndPreasureOnValve = false;
    }
}

void MainAppCrystalT2::SaveProductData()
{
    if (GetImageData()) {
        GetImageData()->SaveProductData();
    }
}

void MainAppCrystalT2::SetWaterCoolingDefault(double ValueInpercent)
{
    short ivalue = PhysicalSizeToTerminalValue(MainAppCrystalT2::AnalogTerminals::WATER_COOLING, ValueInpercent);
    if (GetImageData()) {
        GetImageData()->SetWaterCoolingDefault(ivalue);
    }
}

void MainAppCrystalT2::SetAirCoolingCamera()
{
    if (GetCurrentProductData()) {
        double dvalue = GetCurrentProductData()->m_ValueAirCoolingCameraLitersPerMinute;
        short ivalue = PhysicalSizeToTerminalValue(MainAppCrystalT2::AnalogTerminals::AIR_COOLING_CAMERA, dvalue);
        if (GetImageData()) {
            GetImageData()->SetAirCoolingCamera(ivalue);
        }
    }
}

void MainAppCrystalT2::SetAirCoolingLight()
{
    if (GetSettingsData()) {
        double dvalue = GetCurrentProductData()->m_ValueAirCoolingLightLitersPerMinute;
        short ivalue = PhysicalSizeToTerminalValue(MainAppCrystalT2::AnalogTerminals::AIR_COOLING_CAMERA_LIGHT, dvalue);
        if (GetImageData()) {
            GetImageData()->SetAirCoolingLight(ivalue);
        }
    }
}

void MainAppCrystalT2::SetAirCoolingValve()
{
    if (GetCurrentProductData()) {
        double dvalue = GetCurrentProductData()->m_ValueAirCoolingValveLitersPerMinute;
        short ivalue = PhysicalSizeToTerminalValue(MainAppCrystalT2::AnalogTerminals::AIR_COOLING_VALVES, dvalue);
        if (GetImageData()) {
            GetImageData()->SetAirCoolingValve(ivalue);
        }
    }
}

void MainAppCrystalT2::InitSerialPortsAndSetParameter()
{
    if (GetSettingsData() && !GetSettingsData()->m_WorkWithoutValveController) {  // Initialisierung der Seriellen Schnittstelle für die Kommunikation mit dem Nordson ventil
        // SetDigitalOutputValue(EtherCATConfigData::SET_POWER_SUPPLY_VALVE, true);  // Schalte Versorgungsspannung ein
        QString ErrorMsg, QInfo = tr("Set Power On And Start Initialisation Valve Device/Serial Port, Please Wait ....");
        emit SignalShowInfo(QInfo, QtMsgType::QtInfoMsg);
        thread()->msleep(10000);  // warte bis Gerät vollsändig Hochgefahren
        int rv = OpenComPorts(ErrorMsg);
        thread()->msleep(1000);
        if (rv == ERROR_CODE_NO_ERROR) {
            rv = SetCurrentProductDataToValveDevice(ErrorMsg);  // Parameter zum Nordson Ventil
            if (rv != ERROR_CODE_NO_ERROR) emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);
        }
        thread()->msleep(5000);  // Warte bis alle Ventilparameter gesetzt und in der Anzeige erscheinen sind
    }
}

int MainAppCrystalT2::SearchSerialPorts(QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    SettingsData* pSettingsData = GetSettingsData();

    if (pSettingsData && !pSettingsData->m_WorkWithoutValveController) {
        const QList<QSerialPortInfo> PortInfo = QSerialPortInfo::availablePorts();
        QString CurrentComPort;
        QList<QString> PortsFound;
        QString PortName;

        for (int i = 0; i < PortInfo.count(); i++) {
            PortName = PortInfo.at(i).portName();
            if (PortName == pSettingsData->m_ValveControllerPortName1 || PortName == pSettingsData->m_ValveControllerPortName2) {
                PortsFound.append(PortName);
            }
        }
        qSort(PortsFound.begin(), PortsFound.end());  // sortierun damit Portnummer mit dem niedrigsten Index immer an erster Stelle, ist dann das linke Ventil
        m_NumberSerialPortsFound = PortsFound.size();
        for (int i = 0; i < PortsFound.size(); i++) {
            if (i == 0) {
                PortName = tr("Founded Ports: %1").arg(PortsFound.at(i));
                if (GetValveDialogLeft()) {
                    CurrentComPort = PortsFound.at(i);
                    GetValveDialogLeft()->SetComPortParameter(pSettingsData->m_BaudRate, pSettingsData->m_Parity, pSettingsData->m_DataBits, pSettingsData->m_StopBits, CurrentComPort);
                    if (pSettingsData->m_WorkWithTwoValves) {
                        GetValveDialogLeft()->SetValveName(tr("Left Valve"));
                        GetValveDialogLeft()->SetValveID(LEFT_VALVE_ID);
                    } else {
                        GetValveDialogLeft()->SetValveName(QString(""));
                        GetValveDialogLeft()->SetValveID(LEFT_VALVE_ID);
                    }
                }
            }
            if (i == 1 && pSettingsData->m_WorkWithTwoValves) {
                PortName = PortName + QString(" %1").arg(PortsFound.at(i));
                if (GetValveDialogRight()) {
                    CurrentComPort = PortsFound.at(i);
                    GetValveDialogRight()->SetComPortParameter(pSettingsData->m_BaudRate, pSettingsData->m_Parity, pSettingsData->m_DataBits, pSettingsData->m_StopBits, CurrentComPort);
                    GetValveDialogRight()->SetValveName(tr("Right Valve"));
                    GetValveDialogRight()->SetValveID(RIGHT_VALVE_ID);
                }
            }
        }
        if (m_NumberSerialPortsFound > 0) {
            emit SignalShowInfo(PortName, QtMsgType::QtInfoMsg);
            if (m_NumberSerialPortsFound == 1) {  // One Port in use -> Only one Valve in use
                GetValveDialogLeft()->SetValveName(QString(""));
                GetValveDialogLeft()->SetValveID(LEFT_VALVE_ID);
                if (pSettingsData->m_WorkWithTwoValves) {
                    // System ist für zwei Ventilte configuriert aber nur eins ist angeschlossen. Anlage arbeitet dann nur mit einem Ventil. Warnhinweis an den Bediener
                    ErrorMsg = tr("Warning! System Configured For Two Valves. One Is Only Connected");
                    rv = ERROR_CODE_WARNING1;
                }
            }
        } else {
            GetValveDialogLeft()->SetValveName(QString(""));
            GetValveDialogLeft()->SetValveID(LEFT_VALVE_ID);
            ErrorMsg = tr("Error! Can Not Connect To Nordson Valve. No Serial Ports On Operating System Available Or Valves has No Cable Connection");
            rv = ERROR_CODE_ANY_ERROR;
        }
    }
    return rv;
}

ValveDialog* MainAppCrystalT2::GetValveDialogByID(int ID)
{
    ValveDialog* pValveDialog = NULL;
    SettingsData* pSettingsData = GetSettingsData();

    if (pSettingsData && GetValveDialogLeft()) {
        if (!pSettingsData->m_WorkWithTwoValves)
            pValveDialog = GetValveDialogLeft();
        else {
            if (GetValveDialogLeft()->GetValveID() == ID) {
                pValveDialog = GetValveDialogLeft();
            } else {
                if (GetValveDialogRight()) {
                    if (GetValveDialogRight()->GetValveID() == ID) {
                        pValveDialog = GetValveDialogRight();
                    }
                }
            }
        }
    }
    return pValveDialog;
}

void MainAppCrystalT2::ChangeValveAndTriggerOrder()
{
    SettingsData* pSettingsData = GetSettingsData();
    if (pSettingsData && pSettingsData->m_WorkWithTwoValves) {  // sind zwei Ventile im Einsatz
        pSettingsData->m_RightTriggerIsFirst = !pSettingsData->m_RightTriggerIsFirst;
        SaveSettings();
        if (GetImageData()) {
            GetImageData()->SetChangeTriggerOutputOrder(pSettingsData->m_RightTriggerIsFirst);  // set to real time task
        }
    }
}

void MainAppCrystalT2::SetManualTriggerOutputSetAndResetValveLeft()
{
    if (m_StatusCurrentLiquidTankPreasure == ALARM_LEVEL_OK) {
        if (GetImageData() && GetImageData()->GetKitharaCore()) {
            GetImageData()->GetKitharaCore()->SetManualTrigger(USE_ONLY_LEFT_VALVE);
        }
    }
}

void MainAppCrystalT2::SetManualTriggerOutputSetAndResetValveRight()
{
    if (m_StatusCurrentLiquidTankPreasure == ALARM_LEVEL_OK) {
        if (GetImageData() && GetImageData()->GetKitharaCore()) {
            GetImageData()->GetKitharaCore()->SetManualTrigger(USE_ONLY_RIGHT_VALVE);
        }
    }
}

void MainAppCrystalT2::SetManualTriggerOutputValveLeftAndRight()
{
    if (m_StatusCurrentLiquidTankPreasure == ALARM_LEVEL_OK) {
        if (GetImageData() && GetImageData()->GetKitharaCore()) {
            GetImageData()->GetKitharaCore()->SetManualTrigger(USE_BOTH_VALVES);
        }
    }
}

void MainAppCrystalT2::SetManualTriggerOutputValveLeft(bool set)
{
    if (m_StatusCurrentLiquidTankPreasure == ALARM_LEVEL_OK) {
        SetDigitalOutputValue(EtherCATConfigData::DO_TRIGGER1_VALVE, set);  // Triggerausgang direkt setzen
        if (GetSimulateValve()) {
            if (set) {
                GetSimulateValve()->LeftTriggerOn();
            } else {
                GetSimulateValve()->SlotLeftTriggerOff();
            }
        }
    }
}

void MainAppCrystalT2::SetManualTriggerOutputValveRight(bool set)
{
    if (m_StatusCurrentLiquidTankPreasure == ALARM_LEVEL_OK) {
        SetDigitalOutputValue(EtherCATConfigData::DO_TRIGGER2_VALVE, set);  // Triggerausgang direkt setzen
        if (GetSimulateValve()) {
            if (set) {
                GetSimulateValve()->RightTriggerOn();  // info to virtual valve device
            } else {
                GetSimulateValve()->SlotRightTriggerOff();
            }
        }
    }
}

void MainAppCrystalT2::ChangeTriggerOutputOrder()
{
    SettingsData* pSettingsData = GetSettingsData();
    if (pSettingsData && pSettingsData->m_WorkWithTwoValves) {
        if (GetImageData()) {
            GetImageData()->SetChangeTriggerOutputOrder(pSettingsData->m_RightTriggerIsFirst);  // set to real time task
        }
    }
}

int MainAppCrystalT2::OpenComPorts(QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    if (GetValveDialogLeft() && GetValveDialogLeft()->PortFound()) {
        rv = GetValveDialogLeft()->OpenPortAndStartDataExchangeValve(ErrorMsg);
        if (rv != ERROR_CODE_NO_ERROR) emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);
    }
    if (GetValveDialogRight() && GetValveDialogRight()->PortFound()) {
        rv = GetValveDialogRight()->OpenPortAndStartDataExchangeValve(ErrorMsg);
        if (rv != ERROR_CODE_NO_ERROR) emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);
    }
    return rv;
}

void MainAppCrystalT2::FinishedRealTimeSystem()
{
    if (GetImageData()) {
        GetImageData()->SetMixerOn(false);
    }
    if (GetEjectedBottlesDialog()) {
        GetEjectedBottlesDialog()->StopRunnigThreads();
    }
    if (GetValveDialogLeft()) {
        GetValveDialogLeft()->StopSerialCommuniction();
    }
    if (GetValveDialogRight()) {
        GetValveDialogRight()->StopSerialCommuniction();
    }
    if (GetTimerGetStatusHardwareDevice()) {
        GetTimerGetStatusHardwareDevice()->stop();
    }
    if (GetTimerCheckCleanImage()) {
        GetTimerCheckCleanImage()->stop();
    }
    if (GetTimerCheckDiskOverflow()) {
        GetTimerCheckDiskOverflow()->stop();
    }
    if (GetTimerDrawTrendGraphData()) {
        GetTimerDrawTrendGraphData()->stop();
    }
    if (GetTimerCalculatBottlesPerMinute()) {
        GetTimerCalculatBottlesPerMinute()->stop();
    }
    if (GetTimerSaveAuditTrailData()) {
        GetTimerSaveAuditTrailData()->stop();
    }
    if (GetVideoDialogFullVideo()) {
        GetVideoDialogFullVideo()->StopProductVideos();  // FinishedAllVideoThreads();//wenn die Videofunktion noch läuft, muß sie als erstets beendet werden
    }
    if (GetVideoDialogShowProductVideos()) {
        GetVideoDialogShowProductVideos()->StopProductVideos();
    }
    if (GetVideoDialogShowTriggerPos()) {
        GetVideoDialogShowTriggerPos()->StopProductVideos();
    }
    if (GetWaitForNewVideo()) {
        GetWaitForNewVideo()->FinishedThread();
    }
    // beende lese status vom Realtimekontext
    if (GetInputOutputEvents()) {
        delete m_InputOutputEvents;
        m_InputOutputEvents = NULL;
    }
    // beende komplette Realtime application
    if (GetImageData()) {
        GetImageData()->FinishedThreadAndFreeAllAllocations();
        delete m_ImageData;
        m_ImageData = NULL;
    }
    FinishedSoftwareWriteInfoIntoTrendGraphFile();
    SaveFromRegistryToIniFile();  // make a copy from registry save all settings into ini file
}

void MainAppCrystalT2::SetClearVideoBuffer()
{
    if (GetImageData()) GetImageData()->SetClearVideoBuffer();
}

void MainAppCrystalT2::SlotStartImageAcquisition()
{
    if (GetImageData()) {
        if (GetImageData()->StartCameraAcquisition() == ERROR_CODE_NO_ERROR) {
            QString QInfo = tr("Start The Image Acquisition: OK");
            emit SignalShowInfo(QInfo, QtMsgType::QtInfoMsg);
        }
    }
}

void MainAppCrystalT2::SlotStopImageAcquisition()
{
    if (GetImageData()) {
        if (GetImageData()->StopCameraAcquisition() == ERROR_CODE_NO_ERROR) {
            QString QInfo = tr("Stop The Image Acquisition: OK");
            emit SignalShowInfo(QInfo, QtMsgType::QtInfoMsg);
        }
    }
}

void MainAppCrystalT2::SlotStatusHarwareDevice()
{
    QString SubErrorText, Text, Command = COMMAND_GET_STATUS_HEATERS;
    ProductData* pProductData = GetCurrentProductData();
    double CurrentPreasureTankTemperature = INVALID_TEMPERATURE_VALUE;
    double CurrentHeatingPipeTemperature = INVALID_TEMPERATURE_VALUE;
    double CurrentLiquidTankPreasure = -1.0;

    int StatusDegreeOfPolution = ALARM_LEVEL_OK;
    // int StatusCurrentLiquidTankPreasure = ALARM_LEVEL_OK;
    int StatusCurrentLiquidTankFilling = ALARM_LEVEL_OK;
    int StatusCurrentLiquidTankTemp = ALARM_LEVEL_OK;
    int StatusPiezoTempLeftValve = ALARM_LEVEL_OK;
    int StatusChamperTempLeftValve = ALARM_LEVEL_OK;
    int StatusPiezoTempRightValve = ALARM_LEVEL_OK;
    int StatusChamperTempRightValve = ALARM_LEVEL_OK;
    int StatusSpeedDeviationBetweenCameraAndIS = ALARM_LEVEL_OK;
    int StatusHeatingPipeTemp = ALARM_LEVEL_OK;
    int StatusAirCoolingCameraAndLightActualValue = ALARM_LEVEL_OK;
    int StatusAirCoolingGlassActualValue = ALARM_LEVEL_OK;
    int StatusAirCoolingValvesActualValue = ALARM_LEVEL_OK;
    int StatusWaterCoolingActualValue = ALARM_LEVEL_OK;
    int StatusFlowTransmitterWaterCoolingCircuitActualValue = ALARM_LEVEL_OK;
    int StatusWaterCoolingTemperatureReturnActualValue = ALARM_LEVEL_OK;
    int StatusCounterProducNotFilled = ALARM_LEVEL_OK;
    int StatusMixerRunning = ALARM_LEVEL_OK;

    if (GetOverviewDialog()) {
        CurrentPreasureTankTemperature = GetOverviewDialog()->GetCurrentPreasureTankTemperature();
        CurrentLiquidTankPreasure = GetOverviewDialog()->GetCurrentLiquidTankPreasure();
        CurrentHeatingPipeTemperature = GetOverviewDialog()->GetCurrentHeatingPipeTemperature();

        StatusDegreeOfPolution = GetOverviewDialog()->GetStatusDegreeOfPollution();
        StatusCurrentLiquidTankFilling = GetOverviewDialog()->GetStatusLiquidTankFilling();
        StatusCurrentLiquidTankTemp = GetOverviewDialog()->GetStatusLiquidTankTemperature();
        StatusHeatingPipeTemp = GetOverviewDialog()->GetStatusHeatingPipeTemperature();
        StatusCounterProducNotFilled = GetOverviewDialog()->GetStatusCounterProducNotFilled();
        if (!m_SuppressAlarmWarinigPreasureLiquidTank) {
            // der Druck wird beim Spülen der Ventile kurzfristig erhöht, um eine Fehlermeldung zu verhindern, wird der Wert während des Spülens auf true gesetzt
            // Ist das Spülen beendet, wird der Wert dann verzögert wieder auf false gesetzt
            m_StatusCurrentLiquidTankPreasure = GetOverviewDialog()->GetStatusLiquidTankPreasure();
        } else {
            m_StatusCurrentLiquidTankPreasure = ALARM_LEVEL_OK;
        }
        StatusPiezoTempLeftValve = GetOverviewDialog()->GetStatusPiezoTempLeftValve();
        StatusChamperTempLeftValve = GetOverviewDialog()->GetStatusChamperTempLeftValve();
        StatusPiezoTempRightValve = GetOverviewDialog()->GetStatusPiezoTempRightValve();
        StatusChamperTempRightValve = GetOverviewDialog()->GetStatusChamperTempRightValve();
    }
    if (GetCoolingDialog()) {
        StatusAirCoolingCameraAndLightActualValue = GetCoolingDialog()->GetStatusAirCoolingCameraAndLightActualValue();
        StatusAirCoolingGlassActualValue = GetCoolingDialog()->GetStatusAirCoolingGlassActualValue();
        StatusAirCoolingValvesActualValue = GetCoolingDialog()->GetStatusAirCoolingValvesActualValue();
        /*StatusWaterCoolingActualValue = GetCoolingDialog()->GetStatusWaterCoolingActualValue();
        StatusFlowTransmitterWaterCoolingCircuitActualValue = GetCoolingDialog()->GetStatusFlowTransmitterWaterCoolingCircuitActualValue();*/
        StatusWaterCoolingTemperatureReturnActualValue = GetCoolingDialog()->GetStatusWaterCoolingTemperatureReturnActualValue();
    }
    if (GetAdvancedSettingsDialog()) {
        StatusSpeedDeviationBetweenCameraAndIS = GetAdvancedSettingsDialog()->GetStatusSpeedDeviationBetweenCameraAndIS();
    }

    if (GetSettingsData()) {
        if (pProductData) {
            // control pressure tank temperarture
            if ((pProductData->m_DefaultPreasureTankTemp > CurrentPreasureTankTemperature) && (CurrentPreasureTankTemperature != INVALID_TEMPERATURE_VALUE))
                SetPreasureTankHeater(true);
            else
                SetPreasureTankHeater(false);
            // control heating pipe temperarture
            if ((pProductData->m_DefaulHeatingPipeTemp > CurrentHeatingPipeTemperature) && (CurrentHeatingPipeTemperature != INVALID_TEMPERATURE_VALUE))
                SetHeatingPipe(true);
            else
                SetHeatingPipe(false);
            // control water cooling
            if (m_ActualTemperaturWaterCoolingReturn != INVALID_TEMPERATURE_VALUE && m_EnablePIDControlWaterCooling) {
                m_CurrentWaterCoolingStrokeValue = PIDControllerWaterCooling(pProductData->m_ValveWaterCoolingDefaultTemperature, m_ActualTemperaturWaterCoolingReturn);
                SetWaterCoolingDefault(m_CurrentWaterCoolingStrokeValue);
            }
        } else {
            SetPreasureTankHeater(false);
            SetHeatingPipe(false);
        }
        // Ist der Druck fast Null bzw. unterschreitet einen definierten Wert, dann Ventile nicht mehr ansteuern und den Mixer ausschalten
        if (GetKitharaCore()) {
            if (CurrentLiquidTankPreasure < GetSettingsData()->m_DefineNoPreasureValue) {
                GetKitharaCore()->SetMixerOn(false);
                m_LastStateMixerAndPreasureOn = false;
            } else {
                GetKitharaCore()->SetMixerOn(true);
                if (m_LastStateMixerAndPreasureOn) {
                    if (!m_MixerIsRunning) {
                        // Mixer is not running no power
                        StatusMixerRunning = ALARM_LEVEL_MASCHINE_STOP;
                    } else {
                        // Mixer läuft prüfe ob Geschwindigkeit ok
                        if (GetIOSettingsDialog()) {
                            StatusMixerRunning = GetIOSettingsDialog()->GetStatusMixerVelocity();
                            // StatusMixerRunning = ALARM_LEVEL_OK;
                        }
                    }
                }
                m_LastStateMixerAndPreasureOn = true;
            }
        }

        if (!GetSettingsData()->m_EnableStatusPiezoTempLeftValve) {
            StatusPiezoTempLeftValve = ALARM_LEVEL_OK;
        }
        if (!GetSettingsData()->m_EnableStatusChamperTempLeftValve) {
            StatusChamperTempLeftValve = ALARM_LEVEL_OK;
        }
        if (!GetSettingsData()->m_EnableStatusPiezoTempRightValve) {
            StatusPiezoTempRightValve = ALARM_LEVEL_OK;
        }
        if (!GetSettingsData()->m_EnableStatusChamperTempRightValve) {
            StatusChamperTempRightValve = ALARM_LEVEL_OK;
        }
        if (!GetSettingsData()->m_EnableStatusMixer) {
            StatusMixerRunning = ALARM_LEVEL_OK;
        }
        if (!GetSettingsData()->m_EnableStatusAirCooling) {
            StatusWaterCoolingTemperatureReturnActualValue = ALARM_LEVEL_OK;
        }
        if (!GetSettingsData()->m_EnableStatusAirCooling) {
            StatusAirCoolingCameraAndLightActualValue = ALARM_LEVEL_OK;
            StatusAirCoolingGlassActualValue = ALARM_LEVEL_OK;
            StatusAirCoolingValvesActualValue = ALARM_LEVEL_OK;
        }
        if (!GetSettingsData()->m_EnableStatusCounterProducNotFilled) {
            StatusCounterProducNotFilled = ALARM_LEVEL_OK;
        }
        if (!GetSettingsData()->m_EnableStatusCurrentLiquidTankPreasure) {
            m_StatusCurrentLiquidTankPreasure = ALARM_LEVEL_OK;
        }
        if (!GetSettingsData()->m_EnableStatusAirCoolingCameraActualValue) {
            StatusAirCoolingCameraAndLightActualValue = ALARM_LEVEL_OK;
        }
        if (!GetSettingsData()->m_EnableStatusAirCoolingCameraLightActualValue) {
            StatusAirCoolingGlassActualValue = ALARM_LEVEL_OK;
        }
        if (!GetSettingsData()->m_EnableStatusAirCoolingValvesActualValue) {
            StatusAirCoolingValvesActualValue = ALARM_LEVEL_OK;
        }
        if (!GetSettingsData()->m_EnableStatusCounterBottleEjectedOneAfterTheOther) {
            m_StatusCounterBottleEjectedOneAfterTheOther = ALARM_LEVEL_OK;
        }
        if (!GetSettingsData()->m_EnableStatusCounterMiddleTooLowOneAfterTheOther) {
            m_StatusCounterMiddleTooLowOneAfterTheOther = ALARM_LEVEL_OK;
        }
        // set commant Nordson valves to get the valve status data
        if (GetValveDialogLeft()) {
            GetValveDialogLeft()->SetSerialCommand(Command);
        }
        if (GetSettingsData()->m_WorkWithTwoValves) {
            if (GetValveDialogRight()) GetValveDialogRight()->SetSerialCommand(Command);
        }
        // Set red light on  -> maschine stop
        if (StatusDegreeOfPolution & ALARM_LEVEL_MASCHINE_STOP || StatusCurrentLiquidTankFilling & ALARM_LEVEL_MASCHINE_STOP || StatusCurrentLiquidTankTemp & ALARM_LEVEL_MASCHINE_STOP ||
            m_StatusCurrentLiquidTankPreasure & ALARM_LEVEL_MASCHINE_STOP || StatusPiezoTempLeftValve & ALARM_LEVEL_MASCHINE_STOP || StatusChamperTempLeftValve & ALARM_LEVEL_MASCHINE_STOP ||
            StatusPiezoTempRightValve & ALARM_LEVEL_MASCHINE_STOP || StatusChamperTempRightValve & ALARM_LEVEL_MASCHINE_STOP || StatusSpeedDeviationBetweenCameraAndIS & ALARM_LEVEL_MASCHINE_STOP ||
            m_StatusCounterBottleEjectedOneAfterTheOther & ALARM_LEVEL_MASCHINE_STOP || m_StatusCounterMiddleTooLowOneAfterTheOther & ALARM_LEVEL_MASCHINE_STOP ||
            StatusHeatingPipeTemp & ALARM_LEVEL_MASCHINE_STOP || StatusAirCoolingCameraAndLightActualValue & ALARM_LEVEL_MASCHINE_STOP ||
            StatusAirCoolingGlassActualValue & ALARM_LEVEL_MASCHINE_STOP || StatusAirCoolingValvesActualValue & ALARM_LEVEL_MASCHINE_STOP ||
            StatusWaterCoolingActualValue & ALARM_LEVEL_MASCHINE_STOP || StatusFlowTransmitterWaterCoolingCircuitActualValue & ALARM_LEVEL_MASCHINE_STOP ||
            StatusWaterCoolingTemperatureReturnActualValue & ALARM_LEVEL_MASCHINE_STOP || StatusCounterProducNotFilled & ALARM_LEVEL_MASCHINE_STOP || StatusMixerRunning & ALARM_LEVEL_MASCHINE_STOP) {
            SetErrorLight(true);
            SetWarningLight(false);  // Orange off
            GetKitharaCore()->SetEnableTrigger(false);
        } else {
            GetKitharaCore()->SetEnableTrigger(true);
            // set alarm level blinking red
            if (StatusDegreeOfPolution & ALARM_LEVEL_ALARM || StatusCurrentLiquidTankFilling & ALARM_LEVEL_ALARM || StatusCurrentLiquidTankTemp & ALARM_LEVEL_ALARM ||
                m_StatusCurrentLiquidTankPreasure & ALARM_LEVEL_ALARM || StatusPiezoTempLeftValve & ALARM_LEVEL_ALARM || StatusChamperTempLeftValve & ALARM_LEVEL_ALARM ||
                StatusPiezoTempRightValve & ALARM_LEVEL_ALARM || StatusChamperTempRightValve & ALARM_LEVEL_ALARM || StatusSpeedDeviationBetweenCameraAndIS & ALARM_LEVEL_ALARM ||
                m_StatusCounterBottleEjectedOneAfterTheOther & ALARM_LEVEL_ALARM || m_StatusCounterMiddleTooLowOneAfterTheOther & ALARM_LEVEL_ALARM || StatusHeatingPipeTemp & ALARM_LEVEL_ALARM ||
                StatusAirCoolingCameraAndLightActualValue & ALARM_LEVEL_ALARM || StatusAirCoolingGlassActualValue & ALARM_LEVEL_ALARM || StatusAirCoolingValvesActualValue & ALARM_LEVEL_ALARM ||
                StatusWaterCoolingActualValue & ALARM_LEVEL_ALARM || StatusFlowTransmitterWaterCoolingCircuitActualValue & ALARM_LEVEL_ALARM ||
                StatusWaterCoolingTemperatureReturnActualValue & ALARM_LEVEL_ALARM || StatusCounterProducNotFilled & ALARM_LEVEL_ALARM || StatusMixerRunning & ALARM_LEVEL_ALARM) {
                SetErrorLight(m_LastStateErrorLight);  // red blinking
                m_LastStateErrorLight = !m_LastStateErrorLight;
                SetWarningLight(false);  // Orange off
            } else {
                // set warning level
                if (StatusDegreeOfPolution & ALARM_LEVEL_WARNING || StatusCurrentLiquidTankFilling & ALARM_LEVEL_WARNING || StatusCurrentLiquidTankTemp & ALARM_LEVEL_WARNING ||
                    m_StatusCurrentLiquidTankPreasure & ALARM_LEVEL_WARNING || StatusPiezoTempLeftValve & ALARM_LEVEL_WARNING || StatusChamperTempLeftValve & ALARM_LEVEL_WARNING ||
                    StatusPiezoTempRightValve & ALARM_LEVEL_WARNING || StatusChamperTempRightValve & ALARM_LEVEL_WARNING || StatusSpeedDeviationBetweenCameraAndIS & ALARM_LEVEL_WARNING ||
                    m_StatusCounterBottleEjectedOneAfterTheOther & ALARM_LEVEL_WARNING || m_StatusCounterMiddleTooLowOneAfterTheOther & ALARM_LEVEL_WARNING ||
                    StatusHeatingPipeTemp & ALARM_LEVEL_WARNING || StatusAirCoolingCameraAndLightActualValue & ALARM_LEVEL_WARNING || StatusAirCoolingGlassActualValue & ALARM_LEVEL_WARNING ||
                    StatusAirCoolingValvesActualValue & ALARM_LEVEL_WARNING || StatusWaterCoolingActualValue & ALARM_LEVEL_WARNING ||
                    StatusFlowTransmitterWaterCoolingCircuitActualValue & ALARM_LEVEL_WARNING || StatusWaterCoolingTemperatureReturnActualValue & ALARM_LEVEL_WARNING ||
                    StatusCounterProducNotFilled & ALARM_LEVEL_WARNING || StatusMixerRunning & ALARM_LEVEL_WARNING) {
                    SetWarningLight(true);  // Orange on
                }
            }
        }
        // set error text
        if (StatusMixerRunning != ALARM_LEVEL_OK) {
            SubErrorText = "";
            if (StatusMixerRunning & ALARM_LEVEL_WARNING_MAX || StatusMixerRunning & ALARM_LEVEL_ALARM_MAX) {
                double defaultSpeed = GetMixerTerminalValueToRotationPerMinute(GetSettingsData()->m_SpeedMixerStepperValue);
                SubErrorText = tr("Mixer Speed Too High. Actual %1 rpm Default %2 rpm").arg(m_ActualVelocityMixerRPM, 0, 'f', 1).arg(defaultSpeed, 0, 'f', 1);  // Mixer läuft, aber zu schnell
            }
            if (StatusMixerRunning & ALARM_LEVEL_WARNING_MIN || StatusMixerRunning & ALARM_LEVEL_ALARM_MIN) {
                double defaultSpeed = GetMixerTerminalValueToRotationPerMinute(GetSettingsData()->m_SpeedMixerStepperValue);
                SubErrorText = tr("Mixer Speed Too Low. Actual %1 rpm Default %2 rpm").arg(m_ActualVelocityMixerRPM, 0, 'f', 1).arg(defaultSpeed, 0, 'f', 1);  // Mixer läuft, aber zu langsam
            }
            if ((StatusMixerRunning & ALARM_LEVEL_MASCHINE_STOP) && SubErrorText.isEmpty()) {
                SubErrorText = tr("Mixer Is Not Running");  // hier Spannungsversorgung unterbrochen
            }
            SetMessageAndSetMascineState(StatusMixerRunning, SubErrorText);
        }
        if (StatusAirCoolingCameraAndLightActualValue != ALARM_LEVEL_OK) {
            SubErrorText = tr("Air Cooling Camera And Light Is Out Of Range.");
            SetMessageAndSetMascineState(StatusAirCoolingCameraAndLightActualValue, SubErrorText);
        }
        if (StatusAirCoolingGlassActualValue != ALARM_LEVEL_OK) {
            SubErrorText = tr("Air Cooling Glass Is Out Of Range.");
            SetMessageAndSetMascineState(StatusAirCoolingGlassActualValue, SubErrorText);
        }
        if (StatusAirCoolingValvesActualValue != ALARM_LEVEL_OK) {
            SubErrorText = tr("Air Cooling Valves Is Out Of Range.");
            SetMessageAndSetMascineState(StatusAirCoolingValvesActualValue, SubErrorText);
        }
        if (StatusWaterCoolingActualValue != ALARM_LEVEL_OK) {
            SubErrorText = tr("Water Cooling Is Out Of Range");
            SetMessageAndSetMascineState(StatusWaterCoolingActualValue, SubErrorText);
        }
        if (StatusFlowTransmitterWaterCoolingCircuitActualValue != ALARM_LEVEL_OK) {
            SubErrorText = tr("Water Cooling Flow Tranmitter Is Out Of Range");
            SetMessageAndSetMascineState(StatusFlowTransmitterWaterCoolingCircuitActualValue, SubErrorText);
        }
        if (StatusWaterCoolingTemperatureReturnActualValue != ALARM_LEVEL_OK) {
            SubErrorText = tr("Water Cooling Temperatur Is Out Of Range.");
            SetMessageAndSetMascineState(StatusWaterCoolingTemperatureReturnActualValue, SubErrorText);
        }
        if (StatusDegreeOfPolution != ALARM_LEVEL_OK) {
            SubErrorText = tr("Degree Of Pollution Too Much.");
            SetMessageAndSetMascineState(StatusDegreeOfPolution, SubErrorText);
        }
        if (StatusCurrentLiquidTankFilling != ALARM_LEVEL_OK) {
            if (StatusCurrentLiquidTankFilling & ALARM_LEVEL_ALARM_MAX) {
                SubErrorText = tr("Tank Filling Level Too High.");
                SetMessageAndSetMascineState(StatusCurrentLiquidTankFilling, SubErrorText);
            } else {
                SubErrorText = tr("Tank Filling Level Too Low.");
                SetMessageAndSetMascineState(StatusCurrentLiquidTankFilling, SubErrorText);
            }
        }
        if (StatusHeatingPipeTemp != ALARM_LEVEL_OK) {
            if (StatusHeatingPipeTemp & ALARM_LEVEL_WARNING_MAX || StatusHeatingPipeTemp & ALARM_LEVEL_ALARM_MAX) {
                SubErrorText = tr("Heating Pipe Temperature Too High.");
            }
            if (StatusHeatingPipeTemp & ALARM_LEVEL_WARNING_MIN || StatusHeatingPipeTemp & ALARM_LEVEL_ALARM_MIN) {
                SubErrorText = tr("Heating Pipe Temperature Too Low.");
            }
            SetMessageAndSetMascineState(StatusHeatingPipeTemp, SubErrorText);
        }
        if (StatusCurrentLiquidTankTemp != ALARM_LEVEL_OK) {
            if (StatusCurrentLiquidTankTemp & ALARM_LEVEL_WARNING_MAX || StatusCurrentLiquidTankTemp & ALARM_LEVEL_ALARM_MAX) {
                SubErrorText = tr("Temperature Tank Too High.");
            }
            if (StatusCurrentLiquidTankTemp & ALARM_LEVEL_WARNING_MIN || StatusCurrentLiquidTankTemp & ALARM_LEVEL_ALARM_MIN) {
                SubErrorText = tr("Temperature Tank Too Low.");
            }
            SetMessageAndSetMascineState(StatusCurrentLiquidTankTemp, SubErrorText);
        }
        if (m_StatusCurrentLiquidTankPreasure != ALARM_LEVEL_OK) {
            SubErrorText = tr("Tank Pressure(%1 bar) Out Of Range.").arg(CurrentLiquidTankPreasure);
            SetMessageAndSetMascineState(m_StatusCurrentLiquidTankPreasure, SubErrorText);
        }
        if (StatusPiezoTempLeftValve != ALARM_LEVEL_OK) {
            if (StatusPiezoTempLeftValve & ALARM_LEVEL_WARNING_MAX || StatusPiezoTempLeftValve & ALARM_LEVEL_ALARM_MAX) {
                SubErrorText = tr("Left Valve Piezo Temperature Too High");
            }
            if (StatusPiezoTempLeftValve & ALARM_LEVEL_WARNING_MIN || StatusPiezoTempLeftValve & ALARM_LEVEL_ALARM_MIN) {
                SubErrorText = tr("Left Valve Piezo Temperature Too Low");
            }
            SetMessageAndSetMascineState(StatusPiezoTempLeftValve, SubErrorText);
        }
        if (StatusPiezoTempRightValve != ALARM_LEVEL_OK) {
            if (StatusPiezoTempRightValve & ALARM_LEVEL_WARNING_MAX || StatusPiezoTempRightValve & ALARM_LEVEL_ALARM_MAX) {
                SubErrorText = tr("Right Valve Piezo Temperature Too High");
            }
            if (StatusPiezoTempRightValve & ALARM_LEVEL_WARNING_MIN || StatusPiezoTempRightValve & ALARM_LEVEL_ALARM_MIN) {
                SubErrorText = tr("Right Valve Piezo Temperature Too Low");
            }
            SetMessageAndSetMascineState(StatusPiezoTempRightValve, SubErrorText);
        }
        if (StatusChamperTempLeftValve != ALARM_LEVEL_OK) {
            if (StatusChamperTempLeftValve & ALARM_LEVEL_WARNING_MAX || StatusChamperTempLeftValve & ALARM_LEVEL_ALARM_MAX) {
                SubErrorText = tr("Left Valve Chamber Temperature Too High.");
            }
            if (StatusChamperTempLeftValve & ALARM_LEVEL_WARNING_MIN || StatusChamperTempLeftValve & ALARM_LEVEL_ALARM_MIN) {
                SubErrorText = tr("Left Valve Chamber Temperature Too Low.");
            }
            SetMessageAndSetMascineState(StatusChamperTempLeftValve, SubErrorText);
        }
        if (StatusChamperTempRightValve != ALARM_LEVEL_OK) {
            if (StatusChamperTempRightValve & ALARM_LEVEL_WARNING_MAX || StatusChamperTempRightValve & ALARM_LEVEL_ALARM_MAX) {
                SubErrorText = tr("Right Valve Chamber Temperature Too High.");
            }
            if (StatusChamperTempRightValve & ALARM_LEVEL_WARNING_MIN || StatusChamperTempRightValve & ALARM_LEVEL_ALARM_MIN) {
                SubErrorText = tr("Right Valve Chamber Temperature Too Low.");
            }
            SetMessageAndSetMascineState(StatusChamperTempRightValve, SubErrorText);
        }
        if (StatusSpeedDeviationBetweenCameraAndIS != ALARM_LEVEL_OK) {
            SubErrorText = tr("Speed Between Camera And IS Maschien Out Of Range");
            SetMessageAndSetMascineState(StatusSpeedDeviationBetweenCameraAndIS, SubErrorText);
        }
        if (m_StatusCounterBottleEjectedOneAfterTheOther != ALARM_LEVEL_OK) {
            SubErrorText = tr("Bottles Ejected In A Row Exceed The Max. Value(Max:%1)").arg(GetSettingsData()->m_MaxCounterBottleEjectedOneAfterTheOther);
            SetMessageAndSetMascineState(m_StatusCounterBottleEjectedOneAfterTheOther, SubErrorText);
        }
        if (m_StatusCounterMiddleTooLowOneAfterTheOther != ALARM_LEVEL_OK) {
            SubErrorText = tr("A Row Of Unfilled Bottles(Droplet Volume) Exceeds The Max. Value(Max:%1)").arg(GetSettingsData()->m_MaxCounterMiddleTooLowOneAfterTheOther);
            SetMessageAndSetMascineState(m_StatusCounterMiddleTooLowOneAfterTheOther, SubErrorText);
        }
        if (StatusCounterProducNotFilled != ALARM_LEVEL_OK) {
            SubErrorText = tr("Number Of Ejected Bottles > Recorded By The Ejector");
            SetMessageAndSetMascineState(StatusCounterProducNotFilled, SubErrorText);
        }
    }
    if (GetOverviewDialog()) GetOverviewDialog()->SetBottlesPerMiniute(GetProductsPerMin());
}

double MainAppCrystalT2::GetMixerTerminalValueToRotationPerMinute(double TerminalValue)
{
    return (TerminalValue / 3600.0) * 60.0;  // siehe beckhoff EL7041
}

double MainAppCrystalT2::GetMixerRotationPerMinuteToTerminalValue(double RotationPerMinute)
{
    return (RotationPerMinute / 60.0) * 3600.0;  // siehe beckhoff EL7041
}

double MainAppCrystalT2::PIDControllerWaterCooling(double setpoint, double currentValue)
{
    double DeltaMax = 90.0;
    double dt = GetSettingsData()->m_TimeIntervalReadStatusDataInms / 1000.0;

    if (dt <= 0.0) {
        dt = 1.0;
    }
    // Calculate error
    double error = ((currentValue - setpoint) / DeltaMax) * 100.0;

    // Proportional term
    double Pout = GetSettingsData()->m_WaterCoolingPFactor * error;

    // Integral term
    if (m_CurrentWaterCoolingStrokeValue < GetSettingsData()->m_WaterCoolingStrokeMaxValue && m_CurrentWaterCoolingStrokeValue > GetSettingsData()->m_WaterCoolingStrokeMinValue) {
        m_SumDeviationWaterCooling += error * dt;
    }
    double Iout = GetSettingsData()->m_WaterCoolingIFactor * m_SumDeviationWaterCooling;

    // Derivative term
    double derivative = (error - m_LastControlDeviationWaterCooling) / dt;
    double Dout = GetSettingsData()->m_WaterCoolingDFactor * derivative;

    // Calculate total output
    m_CurrentWaterCoolingStrokeValue = GetSettingsData()->m_WaterCoolingStrokeMinValue + Pout + Iout + Dout;

    // Restrict to max/min
    if (m_CurrentWaterCoolingStrokeValue > GetSettingsData()->m_WaterCoolingStrokeMaxValue)
        m_CurrentWaterCoolingStrokeValue = GetSettingsData()->m_WaterCoolingStrokeMaxValue;
    else if (m_CurrentWaterCoolingStrokeValue < GetSettingsData()->m_WaterCoolingStrokeMinValue)
        m_CurrentWaterCoolingStrokeValue = GetSettingsData()->m_WaterCoolingStrokeMinValue;

    // Save error to previous error
    m_LastControlDeviationWaterCooling = error;

    // qDebug() << QString("P:%1 I:%2 D:%3").arg(Pout).arg(Iout).arg(Dout);

    return m_CurrentWaterCoolingStrokeValue;
}

void MainAppCrystalT2::SetMessageAndSetMascineState(int Status, const QString& SubErrorText)
{
    QString Text;
    if (Status & ALARM_LEVEL_MASCHINE_STOP) {
        Text = tr("Error! %1").arg(SubErrorText);
        SlotAddNewMessage(Text, QtMsgType::QtFatalMsg);  // hier wird die Maschine in den Stoppzustand gesetzt
    } else {
        if (Status & ALARM_LEVEL_ALARM) {
            Text = tr("Alarm! %1").arg(SubErrorText);
            SlotAddNewMessage(Text, QtMsgType::QtCriticalMsg);
        } else {
            if (Status & ALARM_LEVEL_WARNING) {
                Text = tr("Warning! %1").arg(SubErrorText);
                SlotAddNewMessage(Text, QtMsgType::QtWarningMsg);
            }
        }
    }
}

void MainAppCrystalT2::SetHeatingPipe(bool on)
{
    if (GetKitharaCore()) {
        if (on)
            GetKitharaCore()->SetDigitalOutput(EtherCATConfigData::DO_HEATING_PIPE, true);
        else
            GetKitharaCore()->SetDigitalOutput(EtherCATConfigData::DO_HEATING_PIPE, false);
    }
}

void MainAppCrystalT2::SetPreasureTankHeater(bool on)
{
    if (GetKitharaCore()) {
        if (on)
            GetKitharaCore()->SetDigitalOutput(EtherCATConfigData::DO_PREASURE_TANK_HEATER, true);
        else
            GetKitharaCore()->SetDigitalOutput(EtherCATConfigData::DO_PREASURE_TANK_HEATER, false);
    }
}

void MainAppCrystalT2::SlotCheckCleanImage()
{
    if (GetImageData()) GetImageData()->SetCheckCleanImage(true);
}

void MainAppCrystalT2::SlotShowCleanImage(const QImage& Image)
{
    if (GetCleanImageDialog()) GetCleanImageDialog()->SetCleanImage(Image);
}

void MainAppCrystalT2::SlotShowDegreeOfPollution(const double Value)
{
    if (GetCleanImageDialog()) {
        GetCleanImageDialog()->SetDegreeOfPollution(Value);
    }
}

void MainAppCrystalT2::SlotSetDateTimeCleanImageIsSaved(const QString& DateTime)
{
    if (GetCleanImageDialog()) GetCleanImageDialog()->SetCreatedDateAndTimeCleanImage(DateTime);
}

QRect MainAppCrystalT2::GetMeasureWindowRect(int MeasureWindowID)
{
    if (GetImageData())
        return GetImageData()->GetMeasureWindowRect(MeasureWindowID);
    else
        return QRect();
}

void MainAppCrystalT2::SetProfileLineData(const double* ArrayGradient, const double* ArrayGrayValue, int size, double EdgeLeftPos, double EdgeRightPos)
{
    if (GetEditProductDialog() && GetEditProductDialog()->isVisible() && GetEditProductDialog()->IsPlotLineProfilVisible())
        GetEditProductDialog()->SetLineDataGradient(ArrayGradient, size, EdgeLeftPos, EdgeRightPos);
}

double MainAppCrystalT2::MetricToPixel(double MM)
{
    if (GetImageData())
        return GetImageData()->MetricToPixel(MM);
    else
        return 1.0;
}

double MainAppCrystalT2::PixelToMetric(double Pix)
{
    if (GetImageData())
        return GetImageData()->PixelToMetric(Pix);
    else
        return 1.0;
}

double MainAppCrystalT2::GetDisplayZoomFactor()
{
    if (GetImageData())
        return GetImageData()->GetDisplayZoomFactor();
    else
        return 1.0;
}

double MainAppCrystalT2::DistanceInmmToms(double DistanceInMM)
{
    if (GetKitharaCore() && GetKitharaCore()->GetCurrentSpeedInMMPerms() > 0.0)
        return DistanceInMM / GetKitharaCore()->GetCurrentSpeedInMMPerms();
    else
        return 0.0;
}

void MainAppCrystalT2::MeasureWindowChangedByMouse(int MeasureWindowID)
{
    if (GetEditProductDialog() && GetEditProductDialog()->isVisible()) GetEditProductDialog()->UpdateMesureWindow(MeasureWindowID);
}

void MainAppCrystalT2::SetBandDirectional(int set)
{
    if (GetImageData()) GetImageData()->SetBandDirectional(set);
}

void MainAppCrystalT2::MirrorMeasureWindows()
{
    if (GetImageData()) GetImageData()->MirrorMeasureWindows();
}

void MainAppCrystalT2::SaveTrendGraphData(MeasuringResultsLiquidWithDateTime& LiquidData)
{
    double PreasureTankTemp = 0.0;
    double HeatingPipeTemp = 0.0;
    double WaterCoolingTemp = 0.0;

    if (GetOverviewDialog()) {
        PreasureTankTemp = GetOverviewDialog()->GetCurrentPreasureTankTemperature();
        HeatingPipeTemp = GetOverviewDialog()->GetCurrentHeatingPipeTemperature();
    }
    if (GetCoolingDialog()) {
        WaterCoolingTemp = GetCoolingDialog()->GetActualTemperaturWaterCoolingReturn();
    }

    SaveTrendLiquidData(LiquidData);
    SaveTrendTemperatureData(PreasureTankTemp, HeatingPipeTemp, WaterCoolingTemp);
    if (m_FirstCallSaveTrendDataAfterSoftwareStart) {
        m_FirstCallSaveTrendDataAfterSoftwareStart = false;
    }
}

void MainAppCrystalT2::SlotDrawTrendGraphData()
{
    if (CanUpdateTrendGraph() && GetTrendGraphWidget() && GetTrendGraphWidget()->isVisible()) {
        GetTrendGraphWidget()->DrawTrendDataTemperature();
        GetTrendGraphWidget()->DrawTrendDataLiquid();
    }
}

void MainAppCrystalT2::GenerateTrendGraphTemperatureFileName()
{
    QString CurrentProductName = GetCurrentProductName();
    m_TrendGraphTemperatureFileName =
        m_TrendGraphDataLocation + QString("/%1_%2[%3].csv").arg(CurrentProductName).arg(TREND_GRAPH_BASE_FILE_NAME_TEMPERATURE).arg(QDate::currentDate().toString(CSV_FILE_DATE_FORMAT));
}

void MainAppCrystalT2::GenerateTrendGraphLiquidFileName()
{
    QString CurrentProductName = GetCurrentProductName();
    m_TrendGraphLiquidFileName =
        m_TrendGraphDataLocation + QString("/%1_%2[%3].csv").arg(CurrentProductName).arg(TREND_GRAPH_BASE_FILE_NAME_LIQUID).arg(QDate::currentDate().toString(CSV_FILE_DATE_FORMAT));
}

QString MainAppCrystalT2::GetTrendGraphTemperatureFileName()
{
    return m_TrendGraphTemperatureFileName;
}

QString MainAppCrystalT2::GetTrendGraphLiquidFileName()
{
    return m_TrendGraphLiquidFileName;
}

void MainAppCrystalT2::SaveTrendTemperatureData(double PreasureTankTemp, double HeatingPipeTemp, double WaterCoolingTemp)
{
    TrendTemperatureData CurrentTemperatureData;
    CurrentTemperatureData.m_Time = QTime::currentTime();
    // QString CurrentProductName = GetCurrentProductName();
    QString FileName = GetTrendGraphTemperatureFileName();
    // m_TrendGraphDataLocation + QString("/Product_%1_%2[%3].csv").arg(CurrentProductName).arg(TREND_GRAPH_BASE_FILE_NAME_TEMPERATURE).arg(QDate::currentDate().toString(CSV_FILE_DATE_FORMAT));
    QString ResultLine;
    QFile CurrentFile(FileName);
    QChar degree = (0x00b0);  // degree symbol

    CurrentTemperatureData.m_CurrentTemperatureLeftValve = m_CurrentTemparatureLeftValve;
    CurrentTemperatureData.m_StackTemperatureLeftValve = m_StackTemparatureLeftValve;
    CurrentTemperatureData.m_CurrentTemperatureRightValve = m_CurrentTemparatureRightValve;
    CurrentTemperatureData.m_StackTemperatureRightValve = m_StackTemparatureRightValve;
    CurrentTemperatureData.m_CurrentPreasureTankTemperature = PreasureTankTemp;
    CurrentTemperatureData.m_CurrentHeatingPipeTemperature = HeatingPipeTemp;
    CurrentTemperatureData.m_CurrentWaterCoolingTemperature = WaterCoolingTemp;

    if (CurrentFile.exists()) {
        if (m_FirstCallSaveTrendDataAfterSoftwareStart) {
            ResultLine += CurrentTemperatureData.m_Time.toString(CSV_FILE_TIME_FORMAT) + CSV_FILE_SEPERATOR;
            ResultLine += QString(" %1").arg(TREND_GRAPH_FLAG_START_SOFTWARE);
            ResultLine += "\n";
        }
        ResultLine += CurrentTemperatureData.m_Time.toString(CSV_FILE_TIME_FORMAT) + CSV_FILE_SEPERATOR;
        ResultLine += QString(" %1").arg(CurrentTemperatureData.m_StackTemperatureLeftValve, 0, 'f', 1) + CSV_FILE_SEPERATOR;
        ResultLine += QString(" %1").arg(CurrentTemperatureData.m_CurrentTemperatureLeftValve, 0, 'f', 1) + CSV_FILE_SEPERATOR;
        ResultLine += QString(" %1").arg(CurrentTemperatureData.m_StackTemperatureRightValve, 0, 'f', 1) + CSV_FILE_SEPERATOR;
        ResultLine += QString(" %1").arg(CurrentTemperatureData.m_CurrentTemperatureRightValve, 0, 'f', 1) + CSV_FILE_SEPERATOR;
        ResultLine += QString(" %1").arg(CurrentTemperatureData.m_CurrentPreasureTankTemperature, 0, 'f', 1) + CSV_FILE_SEPERATOR;
        ResultLine += QString(" %1").arg(CurrentTemperatureData.m_CurrentHeatingPipeTemperature, 0, 'f', 1) + CSV_FILE_SEPERATOR;
        ResultLine += QString(" %1").arg(CurrentTemperatureData.m_CurrentWaterCoolingTemperature, 0, 'f', 1) + CSV_FILE_SEPERATOR;
        ResultLine += QString("%1").arg(GetCurrentMaschineState()) + CSV_FILE_SEPERATOR;
        ResultLine = ResultLine.replace(QChar('.'), QChar(','));  // wg. Excel
    } else {
        if (GetTrendGraphWidget()) GetTrendGraphWidget()->ClearTrendGraphTemperatureData();
        SetHeaderCSVTemperatureData(ResultLine);
        if (m_FirstCallSaveTrendDataAfterSoftwareStart) {
            ResultLine += "\n";
            ResultLine += CurrentTemperatureData.m_Time.toString(CSV_FILE_TIME_FORMAT) + CSV_FILE_SEPERATOR;
            ResultLine += QString("%1").arg(TREND_GRAPH_FLAG_START_SOFTWARE);
        }
    }
    if (GetTrendGraphWidget()) {
        GetTrendGraphWidget()->AddNewTemperatureData(CurrentTemperatureData);
    }
    ResultLine += "\n";
    WriteCSVFile(FileName, ResultLine);
}

void MainAppCrystalT2::SetHeaderCSVLiquidData(QString& ResultLine)
{
    ResultLine += tr("Time") + CSV_FILE_SEPERATOR;
    ResultLine += tr("Mean Left") + CSV_FILE_SEPERATOR;
    ResultLine += tr("Std  Left") + CSV_FILE_SEPERATOR;
    ResultLine += tr("Mean Right") + CSV_FILE_SEPERATOR;
    ResultLine += tr("Std  Right") + CSV_FILE_SEPERATOR;
    ResultLine += tr("Mean Sum") + CSV_FILE_SEPERATOR;
    ResultLine += tr("Std  Sum") + CSV_FILE_SEPERATOR;
    ResultLine += tr("EjectBottle") + CSV_FILE_SEPERATOR;
    ResultLine += tr("Bottles/min") + CSV_FILE_SEPERATOR;
    ResultLine += tr("Maschine State(8=Production 7=Setup 5=Off)") + CSV_FILE_SEPERATOR;
}

void MainAppCrystalT2::SetHeaderCSVTemperatureData(QString& ResultLine)
{
    ResultLine += tr("Time") + CSV_FILE_SEPERATOR;
    ResultLine += tr("Piezo Left") + CSV_FILE_SEPERATOR;
    ResultLine += tr("Chamber Left") + CSV_FILE_SEPERATOR;
    ResultLine += tr("Piezo Right") + CSV_FILE_SEPERATOR;
    ResultLine += tr("Chamber Right") + CSV_FILE_SEPERATOR;
    ResultLine += tr("Temperature Tank") + CSV_FILE_SEPERATOR;
    ResultLine += tr("Heating Pipe") + CSV_FILE_SEPERATOR;
    ResultLine += tr("Water Cooling") + CSV_FILE_SEPERATOR;
    ResultLine += tr("Maschine State(8=Production 7=Setup 5=Off)") + CSV_FILE_SEPERATOR;
}

void MainAppCrystalT2::SaveTrendLiquidData(MeasuringResultsLiquidWithDateTime& CurrentMeasuringResultsLiqiud)
{
    TrendLiquidData trendLiquidData;
    QString FileName = GetTrendGraphLiquidFileName();
    QString ResultLine;
    QFile CurrentFile(FileName);

    if (!CurrentFile.exists()) {  // Save file with header line
        if (GetTrendGraphWidget()) GetTrendGraphWidget()->ClearTrendGraphLiquidData();
        SetHeaderCSVLiquidData(ResultLine);
        ResultLine += "\n";
        WriteCSVFile(FileName, ResultLine);
        ResultLine = "";
    }
    ResultLine = "";
    trendLiquidData.m_Time = CurrentMeasuringResultsLiqiud.m_CurrentTime;
    trendLiquidData.m_BottlesPerMinute = CurrentMeasuringResultsLiqiud.m_BottlesPerMinute;
    if (CurrentMeasuringResultsLiqiud.m_MeasuringResultsLiquid.m_TheLeftValveIsFilledFirst) {
        trendLiquidData.m_AmountLiquidLeftValve = CurrentMeasuringResultsLiqiud.m_MeasuringResultsLiquid.m_MeanValueAmountMiddleFirstTrigger;
        trendLiquidData.m_StandardDeviationAmountLiquidLeftValve = CurrentMeasuringResultsLiqiud.m_MeasuringResultsLiquid.m_StandardDeviationMiddleFirstTrigger;
        trendLiquidData.m_AmountLiquidRightValve = CurrentMeasuringResultsLiqiud.m_MeasuringResultsLiquid.m_MeanValueAmountMiddleSecondTrigger;
        trendLiquidData.m_StandardDeviationAmountLiquidRightValve = CurrentMeasuringResultsLiqiud.m_MeasuringResultsLiquid.m_StandardDeviationMiddleSecondTrigger;
    } else {
        trendLiquidData.m_AmountLiquidRightValve = CurrentMeasuringResultsLiqiud.m_MeasuringResultsLiquid.m_MeanValueAmountMiddleFirstTrigger;
        trendLiquidData.m_StandardDeviationAmountLiquidRightValve = CurrentMeasuringResultsLiqiud.m_MeasuringResultsLiquid.m_StandardDeviationMiddleFirstTrigger;
        trendLiquidData.m_AmountLiquidLeftValve = CurrentMeasuringResultsLiqiud.m_MeasuringResultsLiquid.m_MeanValueAmountMiddleSecondTrigger;
        trendLiquidData.m_StandardDeviationAmountLiquidLeftValve = CurrentMeasuringResultsLiqiud.m_MeasuringResultsLiquid.m_StandardDeviationMiddleSecondTrigger;
    }
    trendLiquidData.m_SumAmountLiquid = CurrentMeasuringResultsLiqiud.m_MeasuringResultsLiquid.m_MeanValueAmountMiddle;
    trendLiquidData.m_StandardDeviationSumAmountLiquid = CurrentMeasuringResultsLiqiud.m_MeasuringResultsLiquid.m_StandardDeviationMiddle;

    trendLiquidData.m_MaschineState = GetCurrentMaschineState();
    trendLiquidData.m_EjectBottle = CurrentMeasuringResultsLiqiud.m_MeasuringResultsLiquid.m_EjectBottle;
    trendLiquidData.m_EjectBottleValid = CurrentMeasuringResultsLiqiud.m_MeasuringResultsLiquid.m_LiquidMeasuringReady;
    if (m_FirstCallSaveTrendDataAfterSoftwareStart) {
        ResultLine += trendLiquidData.m_Time.toString(CSV_FILE_TIME_FORMAT) + CSV_FILE_SEPERATOR;
        ResultLine += QString(" %1").arg(TREND_GRAPH_FLAG_START_SOFTWARE);
        ResultLine += "\n";
    }
    ResultLine += trendLiquidData.m_Time.toString(CSV_FILE_TIME_FORMAT) + CSV_FILE_SEPERATOR;
    ResultLine += QString(" %1").arg(trendLiquidData.m_AmountLiquidLeftValve, 0, 'f', 1) + CSV_FILE_SEPERATOR;
    ResultLine += QString(" %1").arg(trendLiquidData.m_StandardDeviationAmountLiquidLeftValve, 0, 'f', 1) + CSV_FILE_SEPERATOR;
    ResultLine += QString(" %1").arg(trendLiquidData.m_AmountLiquidRightValve, 0, 'f', 1) + CSV_FILE_SEPERATOR;
    ResultLine += QString(" %1").arg(trendLiquidData.m_StandardDeviationAmountLiquidRightValve, 0, 'f', 1) + CSV_FILE_SEPERATOR;
    ResultLine += QString(" %1").arg(trendLiquidData.m_SumAmountLiquid, 0, 'f', 1) + CSV_FILE_SEPERATOR;
    ResultLine += QString(" %1").arg(trendLiquidData.m_StandardDeviationSumAmountLiquid, 0, 'f', 1) + CSV_FILE_SEPERATOR;
    if (trendLiquidData.m_EjectBottle && trendLiquidData.m_EjectBottleValid && trendLiquidData.m_MaschineState == PluginInterface::MachineState::Production)
        ResultLine += QString(" %1").arg(1) + CSV_FILE_SEPERATOR;
    else
        ResultLine += QString(" %1").arg(0) + CSV_FILE_SEPERATOR;
    ResultLine += QString(" %1").arg(trendLiquidData.m_BottlesPerMinute, 0, 'f', 1) + CSV_FILE_SEPERATOR;
    ResultLine += QString(" %1").arg(trendLiquidData.m_MaschineState) + CSV_FILE_SEPERATOR;
    ResultLine = ResultLine.replace(QChar('.'), QChar(','));  // wg. Excel
    if (GetTrendGraphWidget()) {
        GetTrendGraphWidget()->AddNewLiquidData(trendLiquidData);
    }
    ResultLine += "\n";
    WriteCSVFile(FileName, ResultLine);
}

void MainAppCrystalT2::FinishedSoftwareWriteInfoIntoTrendGraphFile()
{
    QString FileNameLiquid = GetTrendGraphLiquidFileName();
    QString FileNameTemperature = GetTrendGraphTemperatureFileName();
    QString ResultLine;
    QFile CurrentFileLiquid(FileNameLiquid);
    QFile CurrentFileTemperature(FileNameTemperature);
    QString QCurrentTime = QTime::currentTime().toString(CSV_FILE_TIME_FORMAT);

    if (!CurrentFileLiquid.exists()) {
        SetHeaderCSVLiquidData(ResultLine);
        ResultLine += "\n";
        ResultLine += QCurrentTime + CSV_FILE_SEPERATOR;
        ResultLine += QString("%1").arg(TREND_GRAPH_FLAG_FINISHED_SOFTWARE);
        ResultLine += "\n";
        WriteCSVFile(FileNameLiquid, ResultLine);

    } else {
        ResultLine += QCurrentTime + CSV_FILE_SEPERATOR;
        ResultLine += QString("%1").arg(TREND_GRAPH_FLAG_FINISHED_SOFTWARE);
        ResultLine += "\n";
        WriteCSVFile(FileNameLiquid, ResultLine);
    }
    ResultLine = "";

    if (!CurrentFileTemperature.exists()) {
        SetHeaderCSVTemperatureData(ResultLine);
        ResultLine += "\n";
        ResultLine += QCurrentTime + CSV_FILE_SEPERATOR;
        ResultLine += QString("%1").arg(TREND_GRAPH_FLAG_FINISHED_SOFTWARE);
        ResultLine += "\n";
        WriteCSVFile(FileNameTemperature, ResultLine);
    } else {
        ResultLine += QCurrentTime + CSV_FILE_SEPERATOR;
        ResultLine += QString("%1").arg(TREND_GRAPH_FLAG_FINISHED_SOFTWARE);
        ResultLine += "\n";
        WriteCSVFile(FileNameTemperature, ResultLine);
    }
}

void MainAppCrystalT2::SlotSaveAuditTrailData()
{
    QString FileNameAudittrail = m_AudiTrailDataLocation + QString("/AuditTrail[%1].csv").arg(QDate::currentDate().toString(CSV_FILE_DATE_FORMAT));
    QString FileNameAlarmMessage = m_AlarmMessageLocation + QString("/Alarmmessage[%1].txt").arg(QDate::currentDate().toString(CSV_FILE_DATE_FORMAT));
    if (GetCrystalT2Plugin()) {
        GetCrystalT2Plugin()->SaveAuditTrail(QVariant(FileNameAudittrail));
        GetCrystalT2Plugin()->SaveAlarmMessage(QVariant(FileNameAlarmMessage));
    }
}

void MainAppCrystalT2::SlotCheckDiskOverflow()
{
    QStringList FilterNames;
    QString RecordingLocation = GetTriggerImagesFileLocation() + "/";
    QString EjectedBottlesLocation = GetErrorImagePoolLocation() + "/";

    FilterNames << QString("%1*.csv").arg(TREND_GRAPH_BASE_FILE_NAME_LIQUID);
    CheckFilesTrendGraph(m_TrendGraphDataLocation, FilterNames);
    FilterNames << QString("%1*.csv").arg(TREND_GRAPH_BASE_FILE_NAME_TEMPERATURE);
    CheckFilesTrendGraph(m_TrendGraphDataLocation, FilterNames);

    CheckFilesRecordingTriggerImages(RecordingLocation);
    CheckFilesEjectedBottles(EjectedBottlesLocation);
}

void MainAppCrystalT2::CheckFilesTrendGraph(QString& Dir, QStringList& FilterNames)
{
    QDir dir(Dir);
    QStringList totalFiles;
    int MaxEntries = 7;  // days
    SettingsData* pSettingsData = GetSettingsData();

    if (pSettingsData) MaxEntries = pSettingsData->m_MaxNumberFilesTrendGraph;

    totalFiles = dir.entryList(FilterNames, QDir::NoDotAndDotDot | QDir::Files, QDir::Time | QDir::Reversed);
    int diff = totalFiles.count() - MaxEntries;

    if (diff > 0) {
        for (int i = 0; i < diff; i++) {
            QFile File(Dir + QString("/") + totalFiles[i]);
            File.remove();
        }
    }
}

void MainAppCrystalT2::CheckFilesRecordingTriggerImages(QString& Dir)
{
    QDir dir(Dir);
    QStringList totalDirs;
    int MaxEntries = 500;
    SettingsData* pSettingsData = GetSettingsData();

    if (pSettingsData) MaxEntries = pSettingsData->m_MaxNumberFilesTriggerImages;

    totalDirs = dir.entryList(QDir::NoDotAndDotDot | QDir::Files, QDir::Time | QDir::Reversed);
    int diff = totalDirs.count() - MaxEntries;

    if (diff > 0) {
        for (int i = 0; i < diff; i++) {
            QFile File(Dir + QString("/") + totalDirs[i]);
            File.remove();
        }
    }
}

void MainAppCrystalT2::CheckFilesEjectedBottles(QString& rootDirName)
{
    for (int i = 0; i < 3; i++) {
        QString SubDirName = ERROR_TYPS_SUB_DIR_NAMES[i];
        QString DirAndSubPath = rootDirName + SubDirName;
        QDir dirEjectedBottles(DirAndSubPath);
        QFileInfoList FileInfoList = dirEjectedBottles.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time | QDir::Reversed);
        int MaxEntries = 500;
        SettingsData* pSettingsData = GetSettingsData();

        if (pSettingsData) MaxEntries = pSettingsData->m_MaxNumberFilesEjectedImages;

        int diff = FileInfoList.count() - MaxEntries;
        if (diff > 0) {
            for (int i = 0; i < diff; i++) {
                QDir dir(FileInfoList.at(i).filePath());
                dir.removeRecursively();
            }
        }
    }
}

int MainAppCrystalT2::WriteCSVFile(QString& FileName, QString& data)
{
    int retVal = ERROR_CODE_NO_ERROR;
    QFile CurrentFile;

    CurrentFile.setFileName(FileName);
    if (CurrentFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {  // write data
        QTextStream os(&CurrentFile);
        os << data;
        CurrentFile.close();
    } else
        retVal = ERROR_CODE_ANY_ERROR;
    return retVal;
}

// Nur zum Testen
double MainAppCrystalT2::DoubleRand(double fMin, double fMax)
{
    double f = (double)qrand() / (RAND_MAX + 1.0);
    return fMin + f * (fMax - fMin);
}

void MainAppCrystalT2::LoadTrendGraphDataFromFile(QString& FileBaseName)
{
    if (GetTrendGraphWidget()) {
        QDateTime FileDate;
        QString PathAndFileName = m_TrendGraphDataLocation + QString("/%1.csv").arg(FileBaseName);
        if (FileBaseName.contains(TREND_GRAPH_BASE_FILE_NAME_LIQUID)) {
            GetTrendGraphWidget()->LoadDataFromCSVFile(PathAndFileName, TREND_GRAPH_PLOT_ID_LIQUID, FileDate);
            GetTrendGraphWidget()->SetEnableDrawCurrentTrendDataLiquid(false);  // verhindern der Anzeige der Laufenden daten
            GetTrendGraphWidget()->DrawTrendDataLiquidFromFile(FileDate);
            GetTrendGraphWidget()->ShowTimeRangeControlsLiquid();
            GetTrendGraphWidget()->ShowControlsTimeIntervall(false);
        } else {
            if (FileBaseName.contains(TREND_GRAPH_BASE_FILE_NAME_TEMPERATURE)) {
                GetTrendGraphWidget()->LoadDataFromCSVFile(PathAndFileName, TREND_GRAPH_PLOT_ID_TEMPERATURE, FileDate);
                GetTrendGraphWidget()->SetEnableDrawCurrentTrendDataTemperature(false);
                GetTrendGraphWidget()->DrawTrendDataTemperatureFromFile(FileDate);
                GetTrendGraphWidget()->ShowTimeRangeControlsTemperature();
                GetTrendGraphWidget()->ShowControlsTimeIntervall(false);
            }
        }
    }
}

void MainAppCrystalT2::UnLoadTrendGraphData()
{
    if (GetTrendGraphWidget()) {
        GetTrendGraphWidget()->ClearTrendGraphLiquidDataFromFile();
        GetTrendGraphWidget()->ClearTrendGraphTemperatureDataFromFile();
        GetTrendGraphWidget()->SetEnableDrawCurrentTrendDataLiquid(true);
        GetTrendGraphWidget()->SetEnableDrawCurrentTrendDataTemperature(true);
        GetTrendGraphWidget()->HideTimeRangeControlsLiquid();
        GetTrendGraphWidget()->HideTimeRangeControlsTemperature();
        GetTrendGraphWidget()->ShowControlsTimeIntervall(true);
    }
}

int MainAppCrystalT2::GetMinBlueWindowWidthInPixel()
{
    SettingsData* pSettingsData = GetSettingsData();
    ProductData* pProductData = GetCurrentProductData();
    int MinBlueWindowWidth = 200;

    if (pProductData && pSettingsData && pSettingsData->m_PixelSize > 0.0) {
        int InjectionMiddleWindowWidthInPixel = static_cast<int>(pProductData->m_InjectionMiddleWindowWidthInMm / pSettingsData->m_PixelSize);
        int ValveDistanceHalf = static_cast<int>((pSettingsData->m_DistancesBetweenValves / pSettingsData->m_PixelSize) / 2.0);
        MinBlueWindowWidth = ValveDistanceHalf * 2 + InjectionMiddleWindowWidthInPixel + 6;
    }
    return MinBlueWindowWidth;
}

void MainAppCrystalT2::SetDefaultPreasure(double valueInBar, bool SaveToProductData)
{
    SettingsData* pSettingsData = GetSettingsData();
    if (pSettingsData) {
        if (GetImageData()) {
            short svalue = static_cast<short>(pSettingsData->m_FactorAnalogOutputDefaultPreasure * valueInBar + pSettingsData->m_OffsetAnalogOutputDefaultPreasure);
            if (SaveToProductData) {
                QString ErrorMsg;
                ProductData* pProductData = GetCurrentProductData();
                if (pProductData) {
                    pProductData->m_DefaultPreasureInBar = valueInBar;
                    pProductData->WriteProductData(ErrorMsg);  // save to productdata
                }
            }
            GetImageData()->SetDefaultPreasure(svalue);  // set to real time task / Analog output
        }
    }
}

std::pair<double, double> MainAppCrystalT2::GetFactorAndOffset(int type)
{
    double factor = 1.0;
    double offset = 0.0;

    if (GetSettingsData()) {
        switch (type) {
            case AnalogTerminals::PREASURE_VALUE:
                factor = GetSettingsData()->m_FactorAnalogInputCurrentPreasure;
                offset = GetSettingsData()->m_OffsetAnalogInputCurrentPreasure;
                break;
            case AnalogTerminals::AIR_COOLING_CAMERA:
                factor = GetSettingsData()->m_FactorAnalogInputAirCoolingCamera;
                offset = GetSettingsData()->m_OffsetAnalogInputAirCoolingCamera;
                break;
            case AnalogTerminals::AIR_COOLING_CAMERA_LIGHT:
                factor = GetSettingsData()->m_FactorAnalogInputAirCoolingCameraLight;
                offset = GetSettingsData()->m_OffsetAnalogInputAirCoolingCameraLight;
                break;
            case AnalogTerminals::AIR_COOLING_VALVES:
                factor = GetSettingsData()->m_FactorAnalogInputAirCoolingValves;
                offset = GetSettingsData()->m_OffsetAnalogInputAirCoolingValves;
                break;
            case AnalogTerminals::FILLING_LEVEL:
                factor = GetSettingsData()->m_FactorAnalogInputTankFillingLevel;
                offset = GetSettingsData()->m_OffsetAnalogInputTankFillingLevel;
                break;
            case AnalogTerminals::WATER_COOLING:
                factor = GetSettingsData()->m_FactorAnalogInputWaterCooling;
                offset = GetSettingsData()->m_OffsetAnalogInputWaterCooling;
                break;
            case AnalogTerminals::FLOW_TRANSMITTER_WATER_COOLING_CIRCUIT:
                factor = GetSettingsData()->m_FactorAnalogInputFlowTransmitterWaterCoolingCircuit;
                offset = GetSettingsData()->m_OffsetAnalogInputFlowTransmitterWaterCoolingCircuit;
                break;
            case AnalogTerminals::TEMPERATURE_WATER_COOLING_RETURN:
                factor = GetSettingsData()->m_FactorAnalogInputTemperaturWaterCoolingReturn;
                offset = GetSettingsData()->m_OffsetAnalogInputTemperaturWaterCoolingReturn;
                break;
            case AnalogTerminals::PREASURE_TANK_TEMPERATURE:
                offset = 0.0;
                factor = 0.1;  // siehe beckhoff
                break;
            case AnalogTerminals::HEATING_PIPE_TEMPERATURE:
                offset = 0.0;
                factor = 0.1;  // siehe beckhoff
                break;
            default:
                factor = 1.0;
                offset = 0.0;
                break;
        }
    }

    return std::make_pair(factor, offset);
}

double MainAppCrystalT2::TerminalValueToPhysicalSize(int type, short sValue)
{
    std::pair<double, double> factorOffset = GetFactorAndOffset(type);
    double factor = factorOffset.first;
    double offset = factorOffset.second;
    return factor * sValue + offset;
}

short MainAppCrystalT2::PhysicalSizeToTerminalValue(int type, double dValue)
{
    short sValue = 0;
    std::pair<double, double> factorOffset = GetFactorAndOffset(type);
    double factor = factorOffset.first;
    double offset = factorOffset.second;

    if (factor != 0.0) {
        sValue = (dValue - offset) / factor;
    }
    return sValue;
}

QPair<QRect, QRect> MainAppCrystalT2::GetLeftRighFlowChannelLiquid(const QRect& LiquidRect)
{
    QPair<QRect, QRect> FlowChannelRects;
    ProductData* pProductData = GetCurrentProductData();

    if (pProductData && GetSettingsData()) {
        int MiddleWidtInPixel = static_cast<int>(pProductData->m_InjectionMiddleWindowWidthInMm / GetSettingsData()->m_PixelSize);
        int ValveDistanceHalf = static_cast<int>((GetSettingsData()->m_DistancesBetweenValves / GetSettingsData()->m_PixelSize) / 2.0);

        FlowChannelRects.first =
            QRect(LiquidRect.x() + static_cast<int>(LiquidRect.width() / 2.0) - ValveDistanceHalf - static_cast<int>(MiddleWidtInPixel / 2.0), LiquidRect.y(), MiddleWidtInPixel, LiquidRect.height());
        FlowChannelRects.second =
            QRect(LiquidRect.x() + static_cast<int>(LiquidRect.width() / 2.0) + ValveDistanceHalf - static_cast<int>(MiddleWidtInPixel / 2.0), LiquidRect.y(), MiddleWidtInPixel, LiquidRect.height());
    }
    return FlowChannelRects;
}
