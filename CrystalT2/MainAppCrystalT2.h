#pragma once
#include <interfaces.h>
#include <qwidget.h>
#include "ImageMetaData.h"
#include "qtimer.h"

#include <boost/accumulators/framework/accumulator_set.hpp>
#include <boost/accumulators/framework/extractor.hpp>
#include <boost/accumulators/statistics/rolling_mean.hpp>
#include <boost/accumulators/statistics/rolling_variance.hpp>
#include <boost/fusion/functional.hpp>

using namespace boost::accumulators;

class MeasuringResultsLiquidWithDateTime
{
  public:
    MeasuringResultsLiquidWithDateTime() { m_BottlesPerMinute = 0.0; };

  public:
    MeasuringResultsLiquid m_MeasuringResultsLiquid;
    QTime m_CurrentTime;
    double m_BottlesPerMinute;
};

class DeactivateAlarmLevelOptionPanel;
class QuickCouplingDialog;
class PopupSaveVideoDialog;
class CoolingDialog;
class PopupDialogFileTransfer;
class TrendGraphOptionPanel;
class TrendGraphWidget;
class SelectTriggerPosOptionPanel;
class SimulateValve;
class IOSettingsDialog;
class AdvancedSettingsDialog;
class SerialPortSettingsDialog;
class ValveDialog;
class GeneralDialog;
class LiveImageView;
class EditProductDialog;
class InputOutputEvents;
class VideoDialog;
class WaitForNewVideo;
class ImageData;
class KitharaCore;
class ProductData;
class PopupDialogProductDialog;
class ValveDialog;
class MainTabWidget;
class SettingsData;
class OverviewDialog;
class CrystalT2Plugin;
class CleanImageDialog;
class AdminSettingsDialog;
class EjectedBottlesDialog;
class MaintenanceDialog;
class MainAppCrystalT2 : public QWidget
{
    Q_OBJECT
  public:
    MainAppCrystalT2(CrystalT2Plugin*);
    virtual ~MainAppCrystalT2();

    CrystalT2Plugin* GetCrystalT2Plugin() { return m_CrystalT2Plugin; }
    QWidget* GetImageWidget() { return m_ImageWidget; }
    OverviewDialog* GetOverviewDialog() { return m_OverviewDialog; }
    SettingsData* GetSettingsData() { return m_SettingsData; }
    MainTabWidget* GetImageThirdLevelNavigationWidget() { return m_ImageThirdLevelNavigationWidget; }
    MainTabWidget* GetSettingsThirdLevelNavigationWidget() { return m_SettingsThirdLevelNavigationWidget; }
    MainTabWidget* GetParameterDialogThirdLevelNavigationWidget() { return m_ParameterDialogThirdLevelNavigationWidget; }
    PopupDialogProductDialog* GetPopupDialogProductDialog() { return m_PopupDialogProductDialog; }
    KitharaCore* GetKitharaCore() { return m_KitharaCore; }
    ImageData* GetImageData() { return m_ImageData; }
    WaitForNewVideo* GetWaitForNewVideo() { return m_WaitForNewVideo; }
    QTimer* GetTimerGetStatusHardwareDevice() { return m_TimerGetStatusHardwareDevice; }
    QTimer* GetTimerCheckCleanImage() { return m_TimerCheckCleanImage; }
    QTimer* GetTimerDrawTrendGraphData() { return m_TimerDrawTrendGraphData; }
    QTimer* GetTimerCalculatBottlesPerMinute() { return m_TimerCalculatBottlesPerMinute; }
    QTimer* GetTimerCheckDiskOverflow() { return m_TimerCheckDiskOverflow; }
    QTimer* GetTimerSaveAuditTrailData() { return m_TimerSaveAuditTrailData; }
    VideoDialog* GetVideoDialogFullVideo() { return m_VideoDialogFullVideo; }
    VideoDialog* GetVideoDialogShowTriggerPos() { return m_VideoDialogShowTriggerPos; }
    VideoDialog* GetVideoDialogShowProductVideos() { return m_VideoDialogShowProductVideos; }
    InputOutputEvents* GetInputOutputEvents() { return m_InputOutputEvents; }
    EditProductDialog* GetEditProductDialog() { return m_EditProductDialog; }
    LiveImageView* GetLiveImageView() { return m_LiveImageView; }
    GeneralDialog* GetGeneralDialog() { return m_GeneralDialog; }
    ValveDialog* GetValveDialogLeft() { return m_ValveDialogLeft; }
    ValveDialog* GetValveDialogRight() { return m_ValveDialogRight; }
    SerialPortSettingsDialog* GetSerialPortSettingsDialog() { return m_SerialPortSettingsDialog; }
    AdvancedSettingsDialog* GetAdvancedSettingsDialog() { return m_AdvancedSettingsDialog; }
    IOSettingsDialog* GetIOSettingsDialog() { return m_IOSettingsDialog; }
    SimulateValve* GetSimulateValve() { return m_SimulateValve; }  // nur für testzwecke in der Simulation der Ventile
    SelectTriggerPosOptionPanel* GetSelectTriggerPosOptionPanel() { return m_SelectTriggerPosOptionPanel; }
    TrendGraphWidget* GetTrendGraphWidget() { return m_TrendGraphWidget; }
    TrendGraphOptionPanel* GetTrendGraphOptionPanel() { return m_TrendGraphOptionPanel; }
    CleanImageDialog* GetCleanImageDialog() { return m_CleanImageDialog; }
    AdminSettingsDialog* GetAdminSettingsDialog() { return m_AdminSettingsDialog; }
    EjectedBottlesDialog* GetEjectedBottlesDialog() { return m_EjectedBottlesDialog; }
    MaintenanceDialog* GetMaintenanceDialog() { return m_MaintenanceDialog; }
    PopupDialogFileTransfer* GetPopupDialogFileTransfer() { return m_PopupDialogFileTransfer; }
    CoolingDialog* GetCoolingDialog() { return m_CoolingDialog; }
    PopupSaveVideoDialog* GetPopupSaveVideoDialog() { return m_PopupSaveVideoDialog; }
    QuickCouplingDialog* GetQuickCouplingDialog() { return m_QuickCouplingDialog; }
    DeactivateAlarmLevelOptionPanel* GetDeactivateAlarmLevelOptionPanel() { return m_DeactivateAlarmLevelOptionPanel; }
    QString GetPathNameProducts() { return m_PathNameProducts; }
    QString GetPathCleanImageLocation() { return m_CleanImageLocation; }
    QString GetXMLCofigurationFileLocation() { return m_XMLCofigurationFileLocation; }
    // QString GetPathNameEtherCatDesignBusTerminals() { return m_PathNameEtherCatDesignBusTerminals; }
    QString GetVideoFileLocation() { return m_VideoFileLocation; }
    QString GetSettingsLocation() { return m_SettingsLocation; }
    QString GetTrendGraphDataLocation() { return m_TrendGraphDataLocation; }
    QString GetErrorImagePoolLocation() { return m_ErrorImagePoolLocation; }
    QString GetTriggerImagesFileLocation() { return m_TriggerImagesFileLocation; }
    QString GetAudiTrailDataLocation() { return m_AudiTrailDataLocation; }
    QString GetAlarmMessageLocationm() { return m_AlarmMessageLocation; }
    QString GetBackupLocationRegistryData() { return m_BackupLocationRegistryData; }
    QString GetScreenshotsLocation() { return m_ScreenshotsLocation; }

    QList<ProductData*> GetListProducts() { return m_ListProducts; }

    bool IsPowerAndPreasureOnValve() { return m_IsPowerAndPreasureOnValve; }

    bool IsBlueLightOn() { return m_BlueLightOn; }
    void SaveSettings(bool SaveIntoReistry = true);
    void LoadSettings(bool LoadFromReistry = true);

    void SetSettings(QSettings& settings, QString& RegeditDir);
    void GetSettings(QSettings& settings, QString& RegeditDir);

    void OpenProductDialog();
    QString GetCurrentProductName();
    void ShowAndSetCurrentProductName(const QString& Name);
    bool ExistProduct(QString& ProductName);
    void RemoveProduct(QString& ProductName);
    int WriteAndInsertNewProduct(const QString& ProductName, const QString& CopyFromProductName, QString& ErrorMsg);
    int ActivateCurrentProduct(const QString& ProductName, QString& ErrorMsg);
    void SetCurrentProductDataToImageProcessingUnit();
    int SetCurrentProductDataToValveDevice(QString& ErrorMsg);
    ProductData* GetProductByProductName(const QString& Name);
    ProductData* GetCurrentProductData();
    int RenameAndActivateProduct(QString& OldName, QString& NewName, QString& ErrorMsg);
    QFileInfoList GetProductFileInfoList();
    int LoadAllProductFiles(QString& ErrorMsg);
    void ClearProductList();
    int ReadAndAppendProduct(QString& ProductName, QString& ErrorMsg);
    void SetCurrentMaschineState(PluginInterface::MachineState set);
    PluginInterface::MachineState GetCurrentMaschineState();
    int WriteLogFile(const QString& data, const QString& FileName);
    void SaveTrendLiquidData(MeasuringResultsLiquidWithDateTime& LiquidData);
    void SetCurrentMaschineStateToRealTime(PluginInterface::MachineState set);

    int GetInfoLevel();

    void SetErrorLight(bool on);
    void SetWarningLight(bool on);
    void SetErrorTransfer(bool on);

    void StartInitRealTimeSystem();
    void FinishedRealTimeSystem();

    void TriggerGetNewVideoFromRealTimeContext();
    void SetTriggerOffsetInmm(double set, int ValveID);

    void Initialize();
    void Uninitialize();

    void StartTimerIntervalCleanImage(double set);
    void StartTimerUpdateTrendGraph(double set);
    void StartTimerReadStatusData(double set);
    void StartTimerCalculatBottlesPerMinute();
    void StartTimerCheckDiskOverflow(double set);
    void StartTimerSaveAuditTrailData();

    void FreeAll();
    QRect GetMeasureWindowRect(int MeasureWindowID);
    void SetProfileLineData(const double* ArrayGradient, const double* ArrayGrayValue, int size, double EdgeLeftPos, double EdgeRightPos);

    double MetricToPixel(double MM);
    double PixelToMetric(double Pix);
    double GetDisplayZoomFactor();
    double DistanceInmmToms(double DistanceInMM);
    void MeasureWindowChangedByMouse(int MeasureWindowID);
    void SetBandDirectional(int set);
    void MirrorMeasureWindows();
    void SetDigitalOutputValue(EtherCATConfigData::IOChannels ChannelIndex, bool State);
    void CheckFilesTrendGraph(QString& Dir, QStringList& FilterNames);
    void CheckFilesRecordingTriggerImages(QString& Dir);
    void CheckFilesEjectedBottles(QString& Dir);
    int WriteCSVFile(QString& FileName, QString& data);
    void SaveTrendTemperatureData(double PreasureTankTemp, double HeatingPipeTemp, double WaterCoolingTemp);
    void SetClearVideoBuffer();
    void SetPreasureTankHeater(bool on);
    void SetHeatingPipe(bool on);
    int OpenComPorts(QString& ErrorMsg);
    int SearchSerialPorts(QString& ErrorMsg);
    // void ChangeValveOrder();
    void ChangeTriggerOutputOrder();
    void ChangeValveAndTriggerOrder();

    void CheckProductCenterTolerance(double CurrentSpeed);

    QString GetWarningMessagePulseTooBig(const QString& Valve, double Pulse, double InnerSquare, double Speed);
    bool IsStartupInitReady();
    int CurrentAccessLevel();
    void ResetIsClicked();
    void SetPiezoCurrentOnOff(int Value);
    void ShowOptionPanelImageTab(bool Show);
    bool IsDialogShowProductVideosVisible();
    bool WorkWithTwoValves();
    void LoadTrendGraphDataFromFile(QString& FileBaseName);
    void UnLoadTrendGraphData();
    void FinishedSoftwareWriteInfoIntoTrendGraphFile();
    double DoubleRand(double fMin, double fMax);
    double GetProductsPerMin();
    void SetHeaderCSVLiquidData(QString& ResultLine);
    void SetHeaderCSVTemperatureData(QString& ResultLine);
    void SetManualTriggerOutputValveLeft(bool set);
    void SetManualTriggerOutputValveRight(bool set);
    void SetManualTriggerOutputSetAndResetValveLeft();
    void SetManualTriggerOutputSetAndResetValveRight();
    void SetManualTriggerOutputValveLeftAndRight();
    ValveDialog* GetValveDialogByID(int ID);
    void InitSerialPortsAndSetParameter();
    
    void SetDefaultPreasure(double value, bool SaveToProductData = true);
    void SetPreasureOnOff(bool set);
    /* void SetAirCoolingCamera(double set);
     void SetAirCoolingLight(double set);
     void SetAirCoolingValve(double set);*/
    void SaveTrendGraphData(MeasuringResultsLiquidWithDateTime& LiquidData);
    void SetDoubleSpinBoxBackgroundColor(QDoubleSpinBox* spinBox, const QString& state);
    int GetUsedTriggerOutput();
    bool IsFirstTriggerOnLeftSide();
    QPair<QRect, QRect> GetLeftRighFlowChannelLiquid(const QRect& LiquidRect);
    void SetMessageAndSetMascineState(int Status, const QString& SubErrorText);
    void SetSuppressAlarmWarinigPreasureLiquidTank(bool set) { m_SuppressAlarmWarinigPreasureLiquidTank = set; }
    void CheckBottleNotFiledAndDropletVolumeOneAfterTheOther(int countBottleNotFilled, int countMiddleTooLow);

    int GetStatusCounterBottleEjectedOneAfterTheOther() { return m_StatusCounterBottleEjectedOneAfterTheOther; }
    int GetStatusCounterMiddleTooLowOneAfterTheOther() { return m_StatusCounterMiddleTooLowOneAfterTheOther; }

    void ISMaschineEjectAllBottles(bool eject);
    bool CanUpdateTrendGraph();
    void SetPowerAndPreasureOnTheValves(bool on);
    int GetImageWidth();
    int GetImageHeight();

    void SetRequiredAccessLevel();

    double TerminalValueToPhysicalSize(int type, short sValue);
    short PhysicalSizeToTerminalValue(int type, double dValue);

    void SetWaterCoolingDefault(double ValueInPercent);
    // void SetWaterCoolingSensor();
    void SetAirCoolingCamera();
    void SetAirCoolingLight();
    void SetAirCoolingValve();

    std::pair<double, double> GetFactorAndOffset(int type);

    void SaveFromRegistryToIniFile();
    void OpenSaveTriggerImageManual(QImage& Image, const QPixmap& set);

    void SaveProductData();

    QString GetTrendGraphTemperatureFileName();
    QString GetTrendGraphLiquidFileName();

    void GenerateTrendGraphTemperatureFileName();
    void GenerateTrendGraphLiquidFileName();

    int GetStatusCurrentLiquidTankPreasure() { return m_StatusCurrentLiquidTankPreasure; }

    double PIDControllerWaterCooling(double setPoint, double pv);
    double GetActualTemperaturWaterCoolingReturn() { return m_ActualTemperaturWaterCoolingReturn; }
    void SetActualTemperaturWaterCoolingReturn(double set) { m_ActualTemperaturWaterCoolingReturn = set; }

    double GetCurrentWaterCoolingStrokeValue() { return m_CurrentWaterCoolingStrokeValue; }

    void SetAirCoolingValvesAndWaterCooling(bool on);
    QString GetComplationDate(QString& FileName);
    int GetMinBlueWindowWidthInPixel();
    double GetMixerTerminalValueToRotationPerMinute(double TerminalValue);
    double GetMixerRotationPerMinuteToTerminalValue(double RotationPerMinute);

    enum AnalogTerminals {
        PREASURE_VALUE,
        AIR_COOLING_CAMERA,
        AIR_COOLING_CAMERA_LIGHT,
        AIR_COOLING_VALVES,
        FILLING_LEVEL,
        WATER_COOLING_DEFAULT,
        WATER_COOLING,
        FLOW_TRANSMITTER_WATER_COOLING_CIRCUIT,
        TEMPERATURE_WATER_COOLING_RETURN,
        PREASURE_TANK_TEMPERATURE,
        HEATING_PIPE_TEMPERATURE,
        END_VALUE
    };

  signals:
    void SignalShowInfo(const QString& InfoData, QtMsgType MsgType);
    void SignalInitReady();
    void SignalStartupApplication();

  public slots:
    void SlotStartupApplication();
    void SlotAddNewMessage(const QString& ErrorMsg, QtMsgType MsgType);
    void SlotAddNewDebugInfo(const QString& ErrorMsg, int InfoCode);
    void SlotStatusHarwareDevice();
    void SlotCheckCleanImage();
    void SlotShowCleanImage(const QImage& Image);
    void SlotShowDegreeOfPollution(const double Value);
    void SlotSetDateTimeCleanImageIsSaved(const QString& DateTime);
    void SlotStartImageAcquisition();
    void SlotStopImageAcquisition();
    void SlotShowLiveImage(const ImageMetaData& Image);
    void SlotIODeviceMessage(const QString& Info, int InfoType);
    void SlotShowValveTemperature(const QString& StatusHeaters, int ValveID);
    void SlotSetTriggerStateOff(int);
    void SlotInitReady();
    void SlotCheckDiskOverflow();
    void SlotCalculteBottlesPerMinute();
    void SlotDrawTrendGraphData();
    void SlotDelaySetPiezoCurrentOff();
    void SlotDelaySetValvePreasureOn();
    void SlotDelayStartDataExchangeVideoData();
    void SlotSaveAuditTrailData();

  private:
    CrystalT2Plugin* m_CrystalT2Plugin;  // Parent Class
    QWidget* m_ImageWidget;
    OverviewDialog* m_OverviewDialog;
    MainTabWidget* m_ImageThirdLevelNavigationWidget;
    MainTabWidget* m_SettingsThirdLevelNavigationWidget;
    MainTabWidget* m_ParameterDialogThirdLevelNavigationWidget;
    SettingsData* m_SettingsData;
    PopupDialogProductDialog* m_PopupDialogProductDialog;
    VideoDialog* m_VideoDialogFullVideo;
    VideoDialog* m_VideoDialogShowTriggerPos;
    VideoDialog* m_VideoDialogShowProductVideos;
    InputOutputEvents* m_InputOutputEvents;
    EditProductDialog* m_EditProductDialog;
    LiveImageView* m_LiveImageView;
    GeneralDialog* m_GeneralDialog;
    TrendGraphWidget* m_TrendGraphWidget;
    ValveDialog* m_ValveDialogLeft;
    ValveDialog* m_ValveDialogRight;
    SerialPortSettingsDialog* m_SerialPortSettingsDialog;
    AdvancedSettingsDialog* m_AdvancedSettingsDialog;
    IOSettingsDialog* m_IOSettingsDialog;
    SelectTriggerPosOptionPanel* m_SelectTriggerPosOptionPanel;
    TrendGraphOptionPanel* m_TrendGraphOptionPanel;
    CleanImageDialog* m_CleanImageDialog;
    AdminSettingsDialog* m_AdminSettingsDialog;
    EjectedBottlesDialog* m_EjectedBottlesDialog;
    MaintenanceDialog* m_MaintenanceDialog;
    PopupDialogFileTransfer* m_PopupDialogFileTransfer;
    PopupSaveVideoDialog* m_PopupSaveVideoDialog;
    CoolingDialog* m_CoolingDialog;
    QuickCouplingDialog* m_QuickCouplingDialog;
    DeactivateAlarmLevelOptionPanel* m_DeactivateAlarmLevelOptionPanel;
    QList<ProductData*> m_ListProducts;
    QString m_PathNameProducts;
    QString m_XMLCofigurationFileLocation;
    QString m_SettingsLocation;
    QString m_VideoFileLocation;
    QString m_TrendGraphDataLocation;
    QString m_CleanImageLocation;
    QString m_ErrorImagePoolLocation;
    QString m_LastErrorLogMsg;
    QString m_TriggerImagesFileLocation;
    QString m_AudiTrailDataLocation;
    QString m_AlarmMessageLocation;
    QString m_BackupLocationRegistryData;
    QString m_ScreenshotsLocation;
    QString m_TrendGraphTemperatureFileName;
    QString m_TrendGraphLiquidFileName;
    int m_NumberSerialPortsFound;
    bool m_LastStateErrorLight;
    bool m_BlueLightOn;
    bool m_IsPowerAndPreasureOnValve;
    bool m_EnablePIDControlWaterCooling;
    WaitForNewVideo* m_WaitForNewVideo;
    KitharaCore* m_KitharaCore;
    ImageData* m_ImageData;
    QTimer* m_TimerGetStatusHardwareDevice;
    QTimer* m_TimerCheckCleanImage;
    QTimer* m_TimerDrawTrendGraphData;
    QTimer* m_TimerCheckDiskOverflow;
    QTimer* m_TimerCalculatBottlesPerMinute;
    QTimer* m_TimerSaveAuditTrailData;
    SimulateValve* m_SimulateValve;  // nur für testzwecke in der Simulation der Ventile

    int m_TestCounter;
    int m_PreasureTooLowCounter;

    int m_StatusCounterBottleEjectedOneAfterTheOther;
    int m_StatusCounterMiddleTooLowOneAfterTheOther;
    int m_StatusCurrentLiquidTankPreasure;

    double m_StackTemparatureLeftValve;
    double m_CurrentTemparatureLeftValve;

    double m_StackTemparatureRightValve;
    double m_CurrentTemparatureRightValve;

    bool m_FirstCallSaveTrendDataAfterSoftwareStart;
    bool m_LastCallSaveTrendDataSoftwareFinished;

    bool m_SuppressAlarmWarinigPreasureLiquidTank;  // beim spülen wird der Druck erhöht, dann wird der Wert auf false gesetzt um die Alarm/Warning zu unterdrücken

    qint64 m_LastTimerMSecsSinceEpoch;
    qint64 m_CurrentCounterProductOk;
    qint64 m_LastCounterProductOk;
    accumulator_set<qint64, stats<tag::rolling_sum> >* m_ProductsSumCount;
    accumulator_set<qint64, stats<tag::rolling_sum> >* m_ProductsSumTime;

    ushort m_StatusMixer;
    ushort m_ENCStatusMixer;
    double m_ActualVelocityMixerRPM;
    //int m_LastCounterMixer;
    unsigned long long m_NumberErrorMixer;
    bool m_LastStateMixerAndPreasureOn;
    bool m_MixerIsRunning;
    double m_ActualTemperaturWaterCoolingReturn;
    double m_LastControlDeviationWaterCooling;
    double m_SumDeviationWaterCooling;
    double m_CurrentWaterCoolingStrokeValue;
};
