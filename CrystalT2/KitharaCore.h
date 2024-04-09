#pragma once
#include <qobject.h>
#include "SharedData.h"

class ImageData;
class MainAppCrystalT2;
class EtherCatSlaveData;
class KitharaCore : public QObject
{
    Q_OBJECT
  public:
    KitharaCore(MainAppCrystalT2* pMainAppCrystalT2);
    ~KitharaCore();
    MainAppCrystalT2* GetMainAppCrystalT2() { return m_MainAppCrystalT2; }

    int OpenKitharaDriver(QString& CustomNumber, QString& NameKernelDLL, QString& StartUpInfo, QString& ErrorMsg);
    int GetKitharaErrorCodeFromRealTimeContext();
    int GetInfoCodeIODevice();
    void SetInfoCodeIODevice(int);
    bool GetInfoCodeIsLeftTriggerSet();
    void SetInfoCodeIsLeftTriggerSet(bool);
    bool GetInfoCodeIsRightTriggerSet();
    void SetInfoCodeIsRightTriggerSet(bool);
    int CloseKitharaDriver();
    int InitRealTimeSystem(int CameraID, int DeviceIndex, int NetworkAdapterID, QString& FileNameCameraSimulation);
    int SearchAndOpenXHCIDeviceForUSBCamera(int DeviceIndex, QString& StatrUpInfo, QString& ErrorMsg);
    int SearchAndOpenNetworkAdapterForNetworkCamera(int DeviceXHCIIndex, QString& StartUpInfo, QString& ErrorMsg);
    int LoadKernelDLL(const char* NameKernelDLL, QString& ErrorMsg);
    int FreeKernelDLL();
    int FreeKithara();
    int WaitForNextImage(int TimeOutInMM, unsigned __int64& counterCameraImages, QString& ErrorMsg);
    int WaitForNextInpuOutput(int TimeOutInMM, QString& ErrorMsg);
    int WaitForCanCopyNewVideo(int TimeOutInms, QString& ErrorMsg);
    int WaitForBottleEjected(int TimeOutInms, QString& ErrorMsg);
    int SearchCameraDevices(int CameraID, int DeviceIndex, QString& CameraHarwareID);
    int InitEtherCatMasterAndSlave(QString& CameraStartUpInfo, int NetworkAdapterID);
    int FreeEtherCatMasterAndSlave();
    int GetKitharaErrorMsg(int ksError, QString& ErrorMsg, QString& AdditionalErrorMsg);
    int StartCameraAcquisition();
    int StopCameraAcquisition();
    int SearchConnectConfigureRealTimeCamera(int CameraNumber, int DeviceIndex);
    bool IsKitharaDriverOpen() { return m_IsKitharaDriverOpen; }
    bool IsCameraResolutionGreaterThanDefaultResolution() { return m_IsCameraResolutionGreaterThanDefaultResolution; }
    void FreeAllKithara();
    unsigned char* GetRawImagedata();
    unsigned char* GetRawVideodata();
    unsigned char* GetRawCleanImagedata();
    unsigned char* GetRawVideodataBottleEjected();
    void SetKernelParameter(ExchangeMemory* pKernelParameter);
    int ConnectAndConfigureCamera(QString& CameraHardwareID);
    int _ConnectAndConfigureCamera(QString& CameraHardWareID);
    int CreateExchangeMemory(QString& ErrorMsg);
    int CreateEvents();
    int RemoveEvents();
    int CreateCallBacks();
    int RemoveCallBacks();
    int CreateRealTimeTasks();
    int StopAndRemoveMeasureTask();
    int StopAndRemoveCameraSimulationTask();
    int CreateSharedMemory();
    int FreeImageSharedMemory();
    void StartRealTimeIOTask();
    void StartRealTimeMeasureTask();
    void StartRealTimeCameraSimulationTask();
    void SetMeasureWindowPosToRealTime(int ID, int xoff, int yoff, int w, int h);
    void SetEdgeThreshold(int set);
    void SetFilterKernelSize(int set);
    void SetReferenceMeasurePositionInPixel(int set);
    void SetReferenceInjectionPositionInPixel(int set);
    void SetDistanceBetweenMeasurePosAndTriggerPosInPixel(double set);
    void SetCurrentPixelSizeInMMPerPixel(double set);
    void SetTriggerOffsetInmm(double set, int ValveID);
    void SetBandDirectional(int set);
    void SetClearVideoBuffer();
    void SetImageBackgroundContrast(int set);
    void SetPollingTimeIOTaskInms(double set);
    void SetTimePeriodTriggerOutputOnInms(double set);
    void SetTimePeriodDigitalOutputOnInms(double set);
    void SetBotteleNeckDiameterToleranceInmm(double set);
    void SetDistancesBetweenValves(double set);
    void SetUsedTriggerOutput(int Value);
    void SetUseSpeedFromISCalcEjectionTime(bool set);
    void SetCurrentMaschineState(int set);
    void SetEnableTrigger(bool set);
    void SetRollingMeanValueLiquid(int set);
    void SetMixerOn(bool on);
    void SetMixerVelocity(int velocity);

    bool IsMixerOn();

    MeasuringResults* GetCurrentMeasuringResults();
    MeasuringResults* GetCurrentAveragedMeasuringResults();
    MeasuringResultsLiquid* GetCurrentMeasuringResultsLiqiud();
    int SetExposureTime(double set);
    int GetCurrentFifoSize();
    int GetFloatNodeCameraValue(const QString& NodeName, double& Value);
    int SetFloatNodeCameraValue(const QString& NodeName, double Value);
    int GetBoolNodeCameraValue(const QString& NodeName, bool& Value);
    int SetBoolNodeCameraValue(const QString& NodeName, bool Value);
    int SetStringNodeCameraValue(const QString& NodeName, QString& Value);
    int GetInt64NodeCameraValue(const QString& NodeName, long long& Value);
    int SetInt64NodeCameraValue(const QString& NodeName, int Value);
    int SetEnumarateCameraValue(const QString& NodeName, int Value);
    bool IsReadVideoFromRealTimeContextOk();
    void SetReadVideoFromRealTimeContextOk(bool set);
    void TriggerGetNewVideoFromRealTimeContext();
    double GetProductPresentTime(double SpeedInMMPerSecond);
    int InitCameraHandler();
    int FreeCameraHandler();
    void ForceSetEventGetNewImage();
    void ForceSetEventInputOutput();
    void ForceSetEventCanCopyVideoData();
    void ForceSetEventBottleEjected();
    int OpenRealTimeCamera(QString& CameraHardWareID);
    int CloseRealTimeCamera();
    int CloseXHCIController();
    int CloseNetworkAdapter();
    void SetTargetBottleneckDiameter(double set);
    void SetProductWidth(double set);
    int GetNumberDedicatedCPUs(QString& CameraStartUpInfo, QString& ErrorMsg);
    int CalculateTimeStampOffsetBetweenCameraAndRealTimeClock();
    int StopAndRemoveIOTask();
    int StopAndRemoveReadEtherCatDataTask();
    void GetInfoMsgIODevice(QString& ErrorMsg);
    ImageData* GetImageData();
    static void DebugFormat(const char* format, ...);
    void SendMessageToGUI(QString& mMsg, QtMsgType MsgType);  // int MsgType);
    unsigned char GetDigitalInputs(int FirstByte);
    bool GetDigitalInput(EtherCATConfigData::IOChannels Channel);
    double GetEtherCatCycelTimeInms();
    double GetDeltaTFromISInms();
    int MakeCameraDeviceReset();
    QString GetKitharaVersion();
    QString GetOpenCvVersion() { return m_OpenCvVersion; }
    QString GetCameraVendor() { return m_CameraVendor; }
    QString GetCameraSerialNumber() { return m_CameraSerialNumber; }
    QString GetCameraName() { return m_CameraName; }
    bool GetWorkWithoutCamera() { return m_WorkWithoutCamera; }
    void SetDigitalOutput(EtherCATConfigData::IOChannels Channel, bool Value);
    bool SetAnalogOutputValue(const QString& Name, short Value);
    bool SetAnalogInputValue(const QString& Name, short Value);  // Nur für tests
    bool GetAnalogInputValue2Byte(const QString& Name, short& Value);
    bool SetAnalogInputValue2Byte(const QString& ChannelName, short& Value);  // Nur für tests
    bool GetAnalogInputValue4Byte(QString& Name, int& Value);
    bool SetAnalogOutputValue2Byte(const QString& ChannelName, short& Value);
    void ReadDesignBusTerminals();
    int GetDigitalIOIndex(const QString& TerminalTypeName, const QString& ChannelName, int& ChannelIndex);
    int GetAnalogIOIndex(const QString& TerminalTypeName, const QString& ChannelName, int& ChannelIndex);
    static bool PositionLessThan(EtherCatSlaveData& SlaveData1, EtherCatSlaveData& SlaveData2);
    void ConfigureDigitalOutputs();
    void ConfigureDigitalInputs();
    void ConfigureAnalogOutputs();
    void ConfigureAnalogInputs();
    void SetDistanceBottleEjection(double set);
    void SetManualTrigger(int UsedTriggerOutput);
    void SetInfoLevel(int set);
    int GetInfoLevel();
    int GetDebugCounter();
    double GetCurrentSpeedInMMPerms();
    void SetCalibrateModus(bool set);
    bool GetCalibrateModus();
    void StartRealTimeTasks();
    bool IsMeasuringTaskStarted();
    bool IsIOTaskStarted();
    void SetMeasuringTaskStarted(bool set);
    void SetIOTaskStarted(bool set);
    int FreeExchangeSharedMemory();
    int CreateSharedMemoryForCameraSimulation(QString& FileNameVideoFileCameraSimulation);
    int FreeImageSharedMemoryForCameraSimulation();
    void SetVideoStateCameraSimulation(int set);
    void SetAcceptanceThresholdLiquidLeftAndRightROI(int set);
    void SetMinAcceptanceThresholdLiquidMiddleROI(int set);
    void SetMaxAcceptanceThresholdLiquidMiddleROI(int set);
    unsigned long long GetEtherCATTimeStampIn100nsUnits();
    bool IsEtherCATMasterConnected();
    void ResetAllCounters();
    void ResetCountersBottlesEjectionAndLiquidTooLow();
    void SetInjectonWindowMiddleWidthInMm(double set);
    void SetMinSpeedInMMPerMs(double set);
    void SetThresholdBinaryImageLiquid(int set);
    void SetNumberProductsAverageBottleNeckAndPixelSize(int set);
    void SetMinNumberFoundedInROI(int set);
    void SetChangeTriggerOutputOrder(bool set);
    void SetMaxMeasurementsProductIsOutOfTol(int set);
    bool ConnectToEcatMaster();
    void SetFormatFromISInmm(double set);
    void SetButtonIsClickedEjectTheNextnBottles();
    void SetNumberEjectedBottlesByUser(int set);
    // void EnableTriggerOutputs(bool on);
    int SetCameraViewPort(int offsetX, int offsetY, int width, int height, QString& ErrorMsg);
    bool CheckCameraROIPosition(int& offsetX, int& offsety, int& width, int& height, int widthMax, int heightMax);
    int SetCameraXOffset(int offsetX, QString& ErrorMsg);
    int SetCameraYOffset(int offsetY, QString& ErrorMsg);
    int GetCounterNumberBottlesRealEjected();
    unsigned char* GetENCStatusCompactMixer();
    // unsigned char* GetENCStatusMixer();
    ushort GetStatusMixer();
    // ushort GetCounterMixer();
    /*ushort GetActualVelocityMixer();
    ushort GetPOSStatusMixer();*/
    // unsigned char* GetMixerInfoData();
    // unsigned char* GetMixerPOSStatus();
    unsigned char* GetMixerInternalPosition();
    unsigned char* GetMixerExternalPosition();
    unsigned long long GetMixerInternalPositionTimeStamp();
    /*short GetMixerInfoData1();
    short GetMixerInfoData2();*/
    // unsigned char *GetPOSStatusCompact();

    void SetActualVelocityMixer(ushort set);
    void SetENCStatusMixerSimulation(ushort set);
    void SetStatusMixerSimulation(ushort set);
    int WorkaroundSetCameraWidht(int width, QString& ErrorMsg);
    // void SetComplationDateRealTimeDLL(QString& NameKernelDLL);
    QString GetCompilationTimeRealTimeDLL() { return m_CompilationTimeRealTimeDLL; }
    int GetMaxImageWidth() { return m_MaxImageWidth; }
    int GetMaxImageHeight() { return m_MaxImageHeight; }
    void SetKernelParameterToRealTime();
    int StepperMotorSetPDOAssignVelocity(QString& ErrorMsg);

  private:
    bool m_IsKitharaDriverOpen;
    bool m_WorkWithoutCamera;
    bool m_WorkWithoutEtherCat;
    bool m_LiquidFlowSimulationOn;
    bool m_IsInitWent;
    bool m_IsCameraResolutionGreaterThanDefaultResolution;
    bool m_UseUSBCameraInterface;  // nur füe Testzwecke
    ExchangeMemory* m_ExchangeMemory;
    ExchangeMemory* m_KernelParameter;
    unsigned char* m_PointerImageBlock;
    unsigned char* m_PointerVideoBlock;
    unsigned char* m_PointerCleanImageBlock;
    unsigned char* m_PointerVideoBlockFillingProcess;
    unsigned char* m_PointerVideoBlockCameraSimulation;
    // int m_ImageWidth;
    // int m_ImagePitch;   // Pitch in bytes of the bitmap
    // int m_ImageHeight;  // Height in pixels of the bitmap
    // int m_ImageOffsetX;
    // int m_ImageOffsetY;
    int m_MaxImageWidth;
    int m_MaxImageHeight;
    int m_HandleKernel;
    MainAppCrystalT2* m_MainAppCrystalT2;
    QString m_OpenCvVersion;
    QString m_CameraVendor;
    QString m_CameraName;
    QString m_CameraSerialNumber;
    QString m_NameKernelDLL;
    QString m_CompilationTimeRealTimeDLL;
    QList<EtherCatSlaveData> m_ListEtherCatSlaveData;
    QMutex m_MutexSetGetAnalogInput;
};
