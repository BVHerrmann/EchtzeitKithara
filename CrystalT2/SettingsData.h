#pragma once

#include <QObject>

class SettingsData : public QObject
{
    Q_OBJECT
  public:
    SettingsData(QObject* parent = NULL);
    ~SettingsData();

  public:
    bool m_UseCameraHighRes;
    bool m_DeaktivateCheckBottleUnderValve;
    // Camera width and height increment
    int m_CameraMoveOffsetInX;
    int m_CameraMoveOffsetInY;
    // settimgs realtime tasks
    int m_TargetProcessorIOTask;
    int m_TargetProcessorMeasureTask;
    double m_DistanceBottleEjectionInmm;

    bool m_BlowOutEjectorNormallyClosed;
    double m_TimePeriodNotBlowInms;
    bool m_UseSpeedFromISCalcEjectionTime;
    // settings IO Ethercat
    double m_CycleTimeIOTask;
    double m_TimePeriodTriggerOutputOnInms;
    double m_TimePeriodDigitalOutputOnInms;
    int m_NetworkAdapterID;
    // settings camera / Measuring
    double m_ExposureTime;
    double m_PixelSize;
    int m_DeviceIndexXHCIController;
    int m_MinNumberFoundedInROI;
    // settings measure speed and liquid
    double m_MinSpeedInMMPerMs;
    int m_ThresholdBinaryImageLiquid;
    int m_ThresholdBinaryImageDegreeOfPollution;
    double m_EdgeAcceptanceThresholdInPercent;
    int m_BackgroundContrast;
    int m_MinMeasureWindowHeight;
    double m_BottleOffsetOutOfROIInmm;
    // double m_FillingPosTol;
    // int    m_MinAmountLiquidBottleUnderValve;

    int m_TimeIntervalReadStatusDataInms;
    int m_CameraLiveImageTimeoutInms;

    QString m_CurrentProductName;
    QString m_KitharaCustomerNumber;
    QString m_NameKernelDll;
    QString m_VideoFilesLocation;
    QString m_TriggerImagesFileLocation;
    QString m_TrendGraphDataLocation;
    QString m_ErrorImagePoolLocation;
    QString m_CleanImageLocation;
    QString m_XMLCofigurationFileLocation;
    QString m_ProductDataLocation;
    QString m_AudiTrailDataLocation;
    QString m_AlarmMessageLocation;
    QString m_BackupLocationRegistryData;
    QString m_ScreenShotLocation;

    QString m_VideoFileNameCameraSimulation;
    // settings serial port
    QString m_ValveControllerPortName1;
    QString m_ValveControllerPortName2;
    int m_Parity;
    int m_BaudRate;
    int m_DataBits;
    int m_StopBits;

    int m_PollingTimeReadStatusValveInms;
    int m_TimeOutValueSeriellerPort;
    // video settings
    int m_MaxTriggerImagesOnScreen;
    // int     m_VideoSaveConditionFlag;
    int m_SizeVideoMemoryInMB;
    int m_PollingTimeReadNewVideoData;

    int m_MaxNumberFilesTrendGraph;
    int m_MaxNumberFilesTriggerImages;
    int m_MaxNumberFilesEjectedImages;

    // for debugging
    bool m_WorkWithoutCamera;
    bool m_WorkWithoutEtherCat;
    bool m_WorkWithoutValveController;
    bool m_SimulateValve;
    bool m_LiquidFlowSimulationOn;
    // digital outputs on/off on startup
    bool m_CameraLightOnOnStartup;
    bool m_PreasureTankHeaterOnOnStartup;
    bool m_PreasureTankValveOnOnStartup;
    bool m_ValveControllerOnOnStartup;
    bool m_WhiteLightOnOnStartup;
    // Valve Order
    // bool m_FirstPortIsFirst;
    bool m_RightTriggerIsFirst;
    bool m_WorkWithTwoValves;
    bool m_WorkWithSecondTriggerSlider;
    int m_BandDirectional;
    // Analog calculation
    // default preasure

    /* double m_ValueAirCoolingCameraLitersPerMinute;
     double m_ValueAirCoolingLightLitersPerMinute;
     double m_ValueAirCoolingValveLitersPerMinute;

     double m_ValueWaterCoolingDefaultLitersPerMinute;
     double m_ValueWaterCoolingCircuitDefaultSensor;*/

    // Watercooling PID Controller Parameter
    double m_WaterCoolingStrokeMinValue;
    double m_WaterCoolingStrokeMaxValue;
    double m_WaterCoolingPFactor;
    double m_WaterCoolingIFactor;
    double m_WaterCoolingDFactor;

    int m_FactorAnalogOutputDefaultPreasure;
    int m_OffsetAnalogOutputDefaultPreasure;

    // current Preasue
    double m_FactorAnalogInputCurrentPreasure;
    double m_OffsetAnalogInputCurrentPreasure;
    // tank fillinglevel
    double m_FactorAnalogInputTankFillingLevel;
    double m_OffsetAnalogInputTankFillingLevel;

    double m_FactorAnalogInputAirCoolingCamera;
    double m_OffsetAnalogInputAirCoolingCamera;

    double m_FactorAnalogInputAirCoolingCameraLight;
    double m_OffsetAnalogInputAirCoolingCameraLight;

    double m_FactorAnalogInputAirCoolingValves;
    double m_OffsetAnalogInputAirCoolingValves;

    double m_FactorAnalogInputWaterCooling;
    double m_OffsetAnalogInputWaterCooling;

    double m_FactorAnalogInputFlowTransmitterWaterCoolingCircuit;
    double m_OffsetAnalogInputFlowTransmitterWaterCoolingCircuit;

    double m_FactorAnalogInputTemperaturWaterCoolingReturn;
    double m_OffsetAnalogInputTemperaturWaterCoolingReturn;

    // Mixer AlarmLevels
    double m_MixerMovingMaschineStopTimeInSec;
    double m_MixerMovingAlarmInRPM;
    double m_MixerMovingWarningInRPM;
    // Air Cooling Alarm Levels
    double m_AirCoolingMaschineStopTimeInSec;
    double m_AirCoolingAlarmInPercent;
    double m_AirCoolingWarningInPercent;

    // Air Cooling Alarm Levels
    double m_WaterCoolingMaschineStopTimeInSec;
    double m_WaterCoolingAlarm;
    double m_WaterCoolingWarning;

    // Camera Distance
    double m_DistanceCameraProduct;
    double m_DistancesBetweenValves;
    //
    double m_DefineNoPreasureValue;
    // Max Preasure for Purge
    double m_PreasureIncreaseWhenFlushing;
    double m_PreasureTolInPercent;
    // Filling tank alarm levels
    double m_TankFillingLevelMaschineStopInLiter;
    double m_TankFillingLevelAlarmInLiter;
    double m_TankFillingLevelWarningInLiter;
    double m_TankFillingLevelMaxValue;
    // heateing Pipe tank alarm levels
    double m_HeatingPipeLevelMaschineStopTimeInSec;
    double m_HeatingPipeTemperatureLevelAlarmInPercent;
    double m_HeatingPipeTemperatureLevelWarningInPercent;
    // heater tank alarm levels
    double m_TankTemperatureLevelMaschineStopTimeInSec;
    double m_TankTemperatureLevelAlarmInDegree;
    double m_TankTemperatureLevelWarningInDegree;
    // Preasure tank alarm levels
    double m_TankPreasureLevelMaschineStopTimeInSec;
    double m_TankPreasureLevelAlarmInPercent;
    double m_TankPreasureLevelWarningInPercent;
    // Degree pollution alarm levels
    double m_DegreePollutionMaschineStopTimeInSec;
    double m_DegreePollutionAlarmInPercent;
    double m_DegreePollutionWarningInPercent;
    // Valve chamber temp. alarm levels
    double m_ValveChamberTemperatureAlarmLevelInDegree;
    double m_ValveChamberTemperatureWarningLevelInDegree;
    double m_ValveChamberTemperatureMaschineStopTimeInSec;
    // Valve piezo temp. alarm levels
    double m_ValvePiezoTemperatureAlarmLevelInDegree;
    double m_ValvePiezoTemperatureWarningLevelInDegree;
    double m_ValvePiezoTemperatureMaschineStopTimeInSec;
    // speeddiff between IS-Maschine and Speed from Camera
    double m_SpeedDeviationAlarmLevelInMPerMin;
    double m_SpeedDeviationWarningLevelInMPerMin;
    double m_SpeedDeviationMaschineStopTimeInSec;

    double m_CounterProductNotFilledAlarmLevel;
    double m_CounterProductNotFilledWarningLevel;
    double m_CounterProductNotFilledMaschineStopTimeInSec;

    int m_MaxCounterBottleEjectedOneAfterTheOther;
    int m_MaxCounterMiddleTooLowOneAfterTheOther;  // droplet volume too low

    // Threshold max PICO Stack Temperature
    double m_MaxStackTemperature;
    double m_IntervalUpdateTrendGraph;

    int m_MaximumErrorCount;

    int m_SpeedMixerStepperValue;

    int m_RollingMeanValueLiquid;
    int m_NumberProductsAverageBottleNeckAndPixelSize;
    int m_MaxMeasurementsProductIsOutOfTol;
    double m_TimerIntervalCheckCleanImageInMin;
    // bool m_EnableCheckDegreeOfPollution;
    int m_MinWidthInPixelBlueWindow;
    int m_NumberOfDropletsPerRun;
    int m_NumberRunsManualTrigger;
    int m_TimeBetweenRunsManualTriggerInMs;
    int m_PauseTriggerManualInMs;

    // AutocalibrationSettings
    double m_FactorMeasureWindowWidthSearchBottleTopPosiition;
    double m_MinPossibleContrastValueInPercent;
    int m_StartValueMeasureWindowHeightInPixel;
    int m_NumIterationROIHeight;
    int m_NumIterationCalculteROIPosition;
    int m_NumIterationCalculateAccseptanceThreshold;
    int m_NumIterationCalculatePixelSize;
    int m_BottleBaseLineOffsetInPix;
    int m_NumIterationROIYPos;
    double m_FactorThreshold;
    double m_DiameterTolInPercentAutoCalibrate;
    double m_MinPixelSizeAutocalibration;

    // TrendGraphSettings
    int m_TrendGraphIimeRangeIndex;
    int m_TrendGraphRollingMeanSizeBottlesPerMin;
    int m_TrendGraphFlag;
    int m_TrendGraphMaxRangeLiquid;
    int m_TrendGraphMinRangeLiquid;
    int m_TrendGraphAbsolutMaximumLiquid;
    int m_TrendGraphAbsolutMinimumLiquid;
    int m_TrendGraphAbsolutMaximumTemperature;
    int m_TrendGraphAbsolutMinimumTemperature;
    int m_TrendGraphAbsolutMaximumBottlesPerMin;
    int m_TrendGraphAbsolutMinimumBottlesPerMin;
    // enable/disable Alarms
    bool m_EnableStatusDegreeOfPolution;
    bool m_EnableStatusCurrentLiquidTankPreasure;
    bool m_EnableStatusCurrentLiquidTankFilling;
    bool m_EnableStatusCurrentLiquidTankTemp;
    bool m_EnableStatusPiezoTempLeftValve;
    bool m_EnableStatusChamperTempLeftValve;
    bool m_EnableStatusPiezoTempRightValve;
    bool m_EnableStatusChamperTempRightValve;
    bool m_EnableStatusSpeedDeviationBetweenCameraAndIS;
    bool m_EnableStatusHeatingPipeTemp;

    bool m_EnableStatusAirCoolingCameraActualValue;
    bool m_EnableStatusAirCoolingCameraLightActualValue;
    bool m_EnableStatusAirCoolingValvesActualValue;
    bool m_EnableStatusWaterCoolingActualValue;
    bool m_EnableStatusFlowTransmitterWaterCoolingCircuitActualValue;
    bool m_EnableStatusWaterCoolingTemperatureReturnActualValue;

    bool m_EnableStatusCounterProducNotFilled;
    bool m_EnableStatusCounterBottleEjectedOneAfterTheOther;
    bool m_EnableStatusCounterMiddleTooLowOneAfterTheOther;

    bool m_EnableStatusAirCooling;
    bool m_EnableStatusWaterCooling;

    bool m_EnableStatusMixer;
};
