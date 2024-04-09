#include "SettingsData.h"
#include "GlobalConst.h"
#include "QtSerialPort/qserialport.h"
#include "SharedData.h"

SettingsData::SettingsData(QObject* parent)
    : QObject(parent),
      m_VideoFilesLocation("d:/Videos"),
      m_TriggerImagesFileLocation("d:/TriggerImages"),
      m_TrendGraphDataLocation("d:/TrendGraph"),
      m_ErrorImagePoolLocation("d:/EjectedBottles"),
      m_CleanImageLocation("d:/CleanImageLocation"),
      m_ProductDataLocation("d:/ProductData"),
      m_XMLCofigurationFileLocation("d:/EtherCatXMLFiles"),
      m_AudiTrailDataLocation("d:/AuditTrail"),
      m_AlarmMessageLocation("d:/AlarmMessage"),
      m_BackupLocationRegistryData("d:/BackupLocationRegistryData"),
      m_ScreenShotLocation("d:/Screenshots"),
      m_CycleTimeIOTask(0.1),
      m_TimePeriodTriggerOutputOnInms(10.0),
      m_TimePeriodDigitalOutputOnInms(10.0),
      m_ExposureTime(100.0),
      m_CurrentProductName("DefaultProduct"),
      m_PixelSize(0.24439),
      m_ValveControllerPortName1("COM3"),
      m_ValveControllerPortName2("COM4"),
      m_BaudRate(QSerialPort::Baud115200),
      m_Parity(QSerialPort::NoParity),
      m_DataBits(QSerialPort::Data8),
      m_StopBits(QSerialPort::OneStop),
      m_MaxTriggerImagesOnScreen(16),
      m_KitharaCustomerNumber("026439"),
      m_NameKernelDll("CrystalT2_rt.dll"),
      m_DistanceBottleEjectionInmm(2550.0),
      m_BlowOutEjectorNormallyClosed(true),
      m_WorkWithoutCamera(false),
      m_WorkWithoutEtherCat(false),
      m_WorkWithoutValveController(false),
      m_SizeVideoMemoryInMB(1024),
      m_CameraLightOnOnStartup(true),
      m_PreasureTankHeaterOnOnStartup(false),
      m_PreasureTankValveOnOnStartup(false),
      m_ValveControllerOnOnStartup(false),
      m_DeviceIndexXHCIController(0),
      m_NetworkAdapterID(0),
      m_FactorAnalogOutputDefaultPreasure(5402),
      m_OffsetAnalogOutputDefaultPreasure(290),

      m_FactorAnalogInputCurrentPreasure(0.000184),
      m_OffsetAnalogInputCurrentPreasure(0.0),

      m_FactorAnalogInputTankFillingLevel(0.0004121),
      m_OffsetAnalogInputTankFillingLevel(0.6147),

      m_FactorAnalogInputAirCoolingCamera(1.0),
      m_OffsetAnalogInputAirCoolingCamera(0.0),

      m_FactorAnalogInputAirCoolingCameraLight(1.0),
      m_OffsetAnalogInputAirCoolingCameraLight(0.0),

      m_FactorAnalogInputAirCoolingValves(1.0),
      m_OffsetAnalogInputAirCoolingValves(0.0),

      m_FactorAnalogInputWaterCooling(1.0),
      m_OffsetAnalogInputWaterCooling(0.0),

      m_FactorAnalogInputFlowTransmitterWaterCoolingCircuit(1.0),
      m_OffsetAnalogInputFlowTransmitterWaterCoolingCircuit(0.0),

      m_FactorAnalogInputTemperaturWaterCoolingReturn(1.0),
      m_OffsetAnalogInputTemperaturWaterCoolingReturn(0.0),

      /* m_ValueAirCoolingCameraLitersPerMinute(5.0),
       m_ValueAirCoolingLightLitersPerMinute(5.0),
       m_ValueAirCoolingValveLitersPerMinute(5.0),

       m_ValueWaterCoolingDefaultLitersPerMinute(5.0),
       m_ValueWaterCoolingCircuitDefaultSensor(5.0),*/

      m_WaterCoolingStrokeMinValue(15.0),
      m_WaterCoolingStrokeMaxValue(100.0),
      m_WaterCoolingPFactor(0.9),
      m_WaterCoolingIFactor(0.05),
      m_WaterCoolingDFactor(0.01),

      m_AirCoolingMaschineStopTimeInSec(30.0),
      m_AirCoolingAlarmInPercent(20.0),
      m_AirCoolingWarningInPercent(10.0),

      m_WaterCoolingMaschineStopTimeInSec(30.0),
      m_WaterCoolingAlarm(90.0),
      m_WaterCoolingWarning(80.0),

      m_MixerMovingMaschineStopTimeInSec(30.0),
      m_MixerMovingAlarmInRPM(10),
      m_MixerMovingWarningInRPM(5),

      m_DistanceCameraProduct(585.0),
      m_TankFillingLevelMaschineStopInLiter(0.5),
      m_TankFillingLevelAlarmInLiter(1.0),
      m_TankFillingLevelWarningInLiter(3.0),
      m_TankFillingLevelMaxValue(8.0),
      m_TankTemperatureLevelMaschineStopTimeInSec(30.0),
      m_TankTemperatureLevelAlarmInDegree(15.0),
      m_TankTemperatureLevelWarningInDegree(10.0),
      m_HeatingPipeLevelMaschineStopTimeInSec(30.0),
      m_HeatingPipeTemperatureLevelAlarmInPercent(15.0),
      m_HeatingPipeTemperatureLevelWarningInPercent(10.0),
      m_TankPreasureLevelMaschineStopTimeInSec(30.0),
      m_TankPreasureLevelAlarmInPercent(15.0),
      m_TankPreasureLevelWarningInPercent(10.0),
      m_DegreePollutionMaschineStopTimeInSec(30.0),
      m_DegreePollutionAlarmInPercent(10.0),
      m_DegreePollutionWarningInPercent(5.0),
      m_ValveChamberTemperatureAlarmLevelInDegree(20.0),
      m_ValveChamberTemperatureWarningLevelInDegree(10.0),
      m_ValveChamberTemperatureMaschineStopTimeInSec(30.0),
      m_ValvePiezoTemperatureAlarmLevelInDegree(80.0),
      m_ValvePiezoTemperatureWarningLevelInDegree(60.0),
      m_ValvePiezoTemperatureMaschineStopTimeInSec(30.0),
      m_SpeedDeviationAlarmLevelInMPerMin(5.0),
      m_SpeedDeviationWarningLevelInMPerMin(2.0),
      m_SpeedDeviationMaschineStopTimeInSec(30.0),
      m_CounterProductNotFilledAlarmLevel(20),
      m_CounterProductNotFilledWarningLevel(10),
      m_CounterProductNotFilledMaschineStopTimeInSec(30.0),
      m_MinNumberFoundedInROI(3),
      m_TargetProcessorIOTask(6),
      m_TargetProcessorMeasureTask(7),
      m_VideoFileNameCameraSimulation("Video1.avi"),
      m_MinSpeedInMMPerMs(0.016),
      m_ThresholdBinaryImageLiquid(180),
      m_ThresholdBinaryImageDegreeOfPollution(180),
      m_BackgroundContrast(250),
      m_NumberProductsAverageBottleNeckAndPixelSize(10),
      m_MaxMeasurementsProductIsOutOfTol(500),
      m_TimerIntervalCheckCleanImageInMin(1.0),
      m_MaxStackTemperature(60.0),
      m_IntervalUpdateTrendGraph(0.1),
      m_RightTriggerIsFirst(true),
      m_DistancesBetweenValves(25.0),
      m_WorkWithTwoValves(true),
      m_BandDirectional(BAND_DIRECTIONAL_LEFT_TO_RIGHT),
      m_EdgeAcceptanceThresholdInPercent(30.0),
      m_WorkWithSecondTriggerSlider(true),
      m_MinMeasureWindowHeight(12),
      m_FactorMeasureWindowWidthSearchBottleTopPosiition(0.5),
      m_MinPossibleContrastValueInPercent(15.0),
      m_StartValueMeasureWindowHeightInPixel(25),
      m_NumIterationROIHeight(30),
      m_NumIterationCalculteROIPosition(40),
      m_NumIterationCalculateAccseptanceThreshold(40),
      m_NumIterationCalculatePixelSize(80),
      m_FactorThreshold(0.55),
      m_NumIterationROIYPos(6),
      m_BottleBaseLineOffsetInPix(50),
      m_DiameterTolInPercentAutoCalibrate(25.0),
      m_RollingMeanValueLiquid(50),
      m_TimePeriodNotBlowInms(200),
      m_PollingTimeReadStatusValveInms(500),
      m_TimeOutValueSeriellerPort(2000),
      m_TimeIntervalReadStatusDataInms(1000),
      m_SimulateValve(false),
      m_CameraLiveImageTimeoutInms(2000),
      m_PollingTimeReadNewVideoData(200),
      m_WhiteLightOnOnStartup(true),
      m_MaxNumberFilesTriggerImages(500),
      m_MaxNumberFilesEjectedImages(500),
      m_MaxNumberFilesTrendGraph(7),
      m_BottleOffsetOutOfROIInmm(5.0),
      m_MinWidthInPixelBlueWindow(150),
      m_TrendGraphIimeRangeIndex(3),
      m_TrendGraphRollingMeanSizeBottlesPerMin(360),
      m_TrendGraphFlag(12415),
      m_TrendGraphMaxRangeLiquid(3000),
      m_TrendGraphMinRangeLiquid(0),
      m_TrendGraphAbsolutMaximumLiquid(20000),
      m_TrendGraphAbsolutMinimumLiquid(0),
      m_TrendGraphAbsolutMaximumTemperature(100),
      m_TrendGraphAbsolutMinimumTemperature(0),
      m_TrendGraphAbsolutMaximumBottlesPerMin(400),
      m_TrendGraphAbsolutMinimumBottlesPerMin(0),
      m_UseSpeedFromISCalcEjectionTime(true),
      m_PreasureIncreaseWhenFlushing(6.0),
      m_DefineNoPreasureValue(0.1),
      m_PreasureTolInPercent(10.0),
      m_LiquidFlowSimulationOn(false),
      m_NumberOfDropletsPerRun(1),
      m_NumberRunsManualTrigger(1),
      m_TimeBetweenRunsManualTriggerInMs(5000),
      m_MaxCounterBottleEjectedOneAfterTheOther(500),
      m_MaxCounterMiddleTooLowOneAfterTheOther(500),
      m_PauseTriggerManualInMs(300),
      m_SpeedMixerStepperValue(1800),
      m_EnableStatusDegreeOfPolution(true),
      m_EnableStatusCurrentLiquidTankPreasure(true),
      m_EnableStatusCurrentLiquidTankFilling(true),
      m_EnableStatusCurrentLiquidTankTemp(true),
      m_EnableStatusPiezoTempLeftValve(true),
      m_EnableStatusChamperTempLeftValve(true),
      m_EnableStatusPiezoTempRightValve(true),
      m_EnableStatusChamperTempRightValve(true),
      m_EnableStatusSpeedDeviationBetweenCameraAndIS(true),
      m_EnableStatusHeatingPipeTemp(true),
      m_EnableStatusAirCoolingCameraActualValue(true),
      m_EnableStatusAirCoolingCameraLightActualValue(true),
      m_EnableStatusAirCoolingValvesActualValue(true),
      m_EnableStatusWaterCoolingActualValue(true),
      m_EnableStatusFlowTransmitterWaterCoolingCircuitActualValue(true),
      m_EnableStatusWaterCoolingTemperatureReturnActualValue(true),
      m_EnableStatusCounterProducNotFilled(true),
      m_EnableStatusCounterBottleEjectedOneAfterTheOther(true),
      m_EnableStatusCounterMiddleTooLowOneAfterTheOther(true),
      m_EnableStatusAirCooling(true),
      m_EnableStatusWaterCooling(true),
      m_EnableStatusMixer(true),
      m_CameraMoveOffsetInX(16),
      m_CameraMoveOffsetInY(16),
      m_DeaktivateCheckBottleUnderValve(false),
      m_UseCameraHighRes(false),
      m_MinPixelSizeAutocalibration(0.12),
      m_MaximumErrorCount(8)

{
}

SettingsData::~SettingsData()
{
}
