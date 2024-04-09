#pragma once

const int us = 10;
const int ms = us * 1000;
const int s = ms * 1000;

const double MAX_EDGE_CONTRAST_IN_GRAY_SCALES = 180.0;
const double THRESHOLD_FACTOR_IN_PERCENT_SOBEL_KERNEL_SIZE_1 = 100.0 / (1.0 * MAX_EDGE_CONTRAST_IN_GRAY_SCALES);
const double THRESHOLD_FACTOR_IN_PERCENT_SOBEL_KERNEL_SIZE_3 = 100.0 / (4.0 * MAX_EDGE_CONTRAST_IN_GRAY_SCALES);
const double THRESHOLD_FACTOR_IN_PERCENT_SOBEL_KERNEL_SIZE_5 = 100.0 / (48.0 * MAX_EDGE_CONTRAST_IN_GRAY_SCALES);
const double THRESHOLD_FACTOR_IN_PERCENT_SOBEL_KERNEL_SIZE_7 = 100.0 / (640.0 * MAX_EDGE_CONTRAST_IN_GRAY_SCALES);

const double MIN_EDGE_CONTRAST_IN_PERCENT = 15.0;

const int NUMBER_MEASUREWINDOWS = 4;
const int ROI_INDEX_MEASURE_SPEED = 0;
const int ROI_INDEX_MEASURE_LIQUID = 1;
const int ROI_INDEX_MEASURE_BOTTLE_UNDER_VALVE = 2;
const int ROI_INDEX_CAMERA = 3;

const int MAX_FIFO_SIZE = 64;
const int MAX_PROFILE_DATA_LENGHT = 640;
// const int MAX_POSSIBLE_FRAMES_PER_SECOND = 1200;

const int GRADIENT_POSITIVE = 0;
const int GRADIENT_NEGATIVE = 1;

/*const int VIDEO_FLAG_SAVE_ALL_IMAGES = 0;
const int VIDEO_FLAG_SAVE_TRIGGER_IS_SET = 1;
const int VIDEO_FLAG_SAVE_PRODUCT_FOUND = 2;
const int VIDEO_FLAG_SAVE_PRODUCT_NOT_FOUND = 3;
const int VIDEO_FLAG_SAVE_IMAGE_PRODUCT_IS_OUT_OF_ROI = 4;
const int VIDEO_FLAG_SAVE_IMAGE_PRODUCT_SIZE_IS_OUT_OF_TOL = 5;
const int VIDEO_FLAG_SAVE_TRIGGER_IS_RESET = 6;
*/
const int EVENT_NEW_IMAGE_AVAILABLE = 1;
const int EVENT_NEW_IMAGE_NOT_AVAILABLE = 0;

const int BAND_DIRECTIONAL_LEFT_TO_RIGHT = 0;
const int BAND_DIRECTIONAL_RIGHT_TO_LEFT = 1;

const int INFO_CODE_INPUT_OUTPUT_DEVICE_NO_INFO = 0;
const int INFO_CODE_INPUT_OUTPUT_DEVICE_ANY_ERROR = 1;
const int INFO_CODE_INPUT_OUTPUT_REAL_TIME_TASKS_RUNS = 2;
const int INFO_CODE_INPUT_OUTPUT_DEVICE_LEFT_TRIGGER_IS_SET = 3;
const int INFO_CODE_INPUT_OUTPUT_DEVICE_TRIGGER_IS_RESET = 4;
const int INFO_CODE_INPUT_DEVICE_KITHARA_ERROR_CODE = 5;
const int INFO_CODE_INPUT_DATA_STATUS = 6;
// const int INFO_CODE_INPUT_CONTROL_VOLTAGE_OK = 7;
const int INFO_CODE_INPUT_CONTROL_VOLTAGE_NOT_OK = 8;
const int INFO_CODE_DEBUG_INFO = 9;
const int INFO_CODE_ETHER_CAT_CYCLUS = 10;
const int INFO_CODE_SPEED_FROM_IS = 11;
const int INFO_CODE_ACTUAL_LIQUID_TANK_PREASURE = 14;
const int INFO_CODE_PREASURE_TANK_FILLING_LEVEL = 16;
const int INFO_CODE_PREASURE_TANK_TEMP = 17;
const int INFO_CODE_INPUT_OUTPUT_DEVICE_RIGHT_TRIGGER_IS_SET = 18;
const int INFO_CODE_INPUT_OUTPUT_DEVICE_BLOW_OUT_EJECTOR_IS_NOT_ACTIVE = 19;
const int INFO_CODE_INPUT_OUTPUT_DEVICE_BLOW_OUT_EJECTOR_IS_ACTIVE = 20;
const int INFO_CODE_ACTUAL_AIR_COOLING_CAMERA = 21;
const int INFO_CODE_ACTUAL_AIR_COOLING_LIGHT = 22;
const int INFO_CODE_ACTUAL_AIR_COOLING_VALVE = 23;
const int INFO_CODE_MANUAL_EJECTION_READY = 24;
const int INFO_CODE_HEATING_PIPE_TEMP = 25;
const int INFO_CODE_NO_PREASURE = 26;
const int INFO_CODE_ACTUAL_WATER_COOLING = 27;
const int INFO_CODE_ACTUAL_FLOW_TRANSMITTER_WATER_COOLING_CIRCUIT = 28;
const int INFO_CODE_ACTUAL_TEMPERATURE_WATER_COOLING_RETURN = 29;
const int INFO_CODE_COUNTER_REAL_EJECTED = 30;
const int INFO_CODE_STATUS_MIXER = 31;

const int MAX_NUMBER_SLAVES = 32;

const int MAX_IMAGES_IN_VIDEO_BUFFER = 32768;

// status startup operation status
const int OPERATION_STATUS_UNKNOWN = -1;
const int OPERATION_STATUS_RELEASE_PRODUCTION = 0;     // Maschnie ist im Produktionsmodus, Trigger und Flaschenauswurf sind aktiv
const int OPERATION_STATUS_STARTUP = 1;                // Software Startet gerade
const int OPERATION_STATUS_SETUP_DISABLE_TRIGGER = 2;  // Trigger löst nicht aus und Flaschenauswurf ist immer an
const int OPERATION_STATUS_SETUP_ENABLE_TRIGGER = 3;   // trigger kann auslösen und Flaschenauswuerf ist immer an
const int OPERATION_STATUS_ERROR = 4;                  // Maschine ist im Fehlerzustand, Trigger löst nicht aus und Flaschenauswurf ist immer an

const int INFO_LEVEL_OFF = 0;
const int INFO_LEVEL_SHOW_LIQUID_WINDOW = 1;
const int INFO_LEVEL_SHOW_BIN_IMAGES = 2;
const int INFO_LEVEL_SHOW_BOTTLE_POSITION = 3;
const int INFO_LEVEL_SHOW_ONLY_LIQUID_RESULTS = 4;
const int INFO_LEVEL_SHOW_ONLY_SPEED_RESULTS = 5;
const int INFO_LEVEL_SHOW_FULL_INFO = 6;
const int INFO_LEVEL_SHOW_BIN_CLEAN_IMAGE = 7;
const int INFO_LEVEL_SHOW_INFO_DATA_EJECTED_BOTTLE = 8;

const int USE_BOTH_VALVES = 0;
const int USE_ONLY_LEFT_VALVE = 1;
const int USE_ONLY_RIGHT_VALVE = 2;

const int SIZE_ETHER_CAT_IN_BYTES = 1024;

const unsigned char IMAGE_CAPTURE_TYPE_NORMAL_IMAGE = 0x00;
const unsigned char IMAGE_CAPTURE_TYPE_PRODUCT_FOUND = 0x01;
const unsigned char IMAGE_CAPTURE_TYPE_FIRST_OCCURRENCE = 0x02;
const unsigned char IMAGE_CAPTURE_TYPE_LEFT_TRIGGER_IS_SET = 0x04;
const unsigned char IMAGE_CAPTURE_TYPE_TRIGGER_IS_RESET = 0x08;
const unsigned char IMAGE_CAPTURE_TYPE_RIGHT_TRIGGER_IS_SET = 0x10;
const unsigned char IMAGE_CAPTURE_TYPE_BOTTLE_EJECTED = 0x20;
const unsigned char IMAGE_CAPTURE_TYPE_LIQUID_MEASURING_READY = 0x40;
const unsigned char IMAGE_CAPTURE_TYPE_BOTTLE_EJECTED_DATA_ON_DISK = 0x80;

const int MAX_NUMBER_VALVES = 2;
const int LEFT_VALVE_INDEX = 0;
const int RIGHT_VALVE_INDEX = 1;

const int LEFT_TRIGGER_SIDE_INDEX = 0;
const int RIGHT_TRIGGER_SIDE_INDEX = 1;
const int NO_SIDE_INDEX = 2;

const int FIRST_TRIGGER_ID = 1;
const int SECOND_TRIGGER_ID = 2;
const int SINGLE_TRIGGER_ID = 3;

const int PLAY_VIDEO_CAMERA_SIMULATION = 0;
const int STOP_VIDEO_CAMERA_SIMULATION = 1;
const int STEP_VIDEO_CAMERA_SIMULATION = 2;
const int PLAY_ONE_VIDEO_CAMERA_SIMULATION = 3;

// 0x1600
class MixerENCControlCompact
{
  public:
    unsigned char m_ENCControlCompact[4];
};
// 0x1601
//class MixerENCControl
//{
//  public:
//    unsigned char m_ENCControl[6];
//};
// 0x1602 STM Control
class MixerSTMControl
{
  public:
    short stmControl;
};
// 0x1604 STM Velocity
class MixerSTMVelocity
{
  public:
    short Velocity;
};
// 0x1606 POS Control
//class MixerPOSControl
//{
//  public:
//    unsigned char m_POSControl[14];
//};

// 0x1A00 ENC Status compact
class MixerENCStatusCompact
{
  public:
    unsigned char m_ENCStatusCompact[6];
};
// 0x1A01 ENC Status
//class MixerENCStatus
//{
//  public:
//    unsigned char m_ENCStatus[10];
//};
// 0x1A03 STM Status
class MixerSTMStatus
{
  public:
    short stmStatus;
};
// 0x1A04 STM Synchron info data
//class MixerInfoData
//{
//  public:
//    unsigned char m_infoData[4];
//};
// 0x1A06 POS Status
//class MixerPOSStatus
//{
//  public:
//    unsigned char POSStatus[12];
//};

class PositionDataMixer
{
  public:
    unsigned char m_InternalPosition[4];
    unsigned char m_ExternalPosition[4];
    unsigned long long m_CurrentTimeStamp;
};

class EtherCatDataSet
{
  public:
    EtherCatDataSet()
    {
        memset(EtherCatData, 0, sizeof(unsigned char) * SIZE_ETHER_CAT_IN_BYTES);
        m_MixerSTMStatus.stmStatus = 0;
        memset(m_MixerENCControlCompact.m_ENCControlCompact, 0, sizeof(MixerENCControlCompact));
        //memset(m_MixerENCControl.m_ENCControl, 0, sizeof(MixerENCControl));
        m_MixerSTMControl.stmControl = 0;
        m_MixerSTMVelocity.Velocity = 0;
        //memset(m_MixerPOSControl.m_POSControl, 0, sizeof(MixerPOSControl));
        memset(m_MixerENCStatusCompact.m_ENCStatusCompact, 0, sizeof(MixerENCStatusCompact));
        //memset(m_MixerENCStatus.m_ENCStatus, 0, sizeof(MixerENCStatus));
        m_MixerSTMStatus.stmStatus = 0;
        //memset(m_MixerInfoData.m_infoData, 0, sizeof(MixerInfoData));
        //memset(m_MixerPOSStatus.POSStatus, 0, sizeof(MixerPOSStatus));
        memset(m_MixerPositionData.m_ExternalPosition, 0, 4);
        memset(m_MixerPositionData.m_InternalPosition, 0, 4);
        m_MixerPositionData.m_CurrentTimeStamp = 0;
    };

  public:
    unsigned char EtherCatData[SIZE_ETHER_CAT_IN_BYTES];
    MixerENCControlCompact m_MixerENCControlCompact;  // 0x1600
    //MixerENCControl m_MixerENCControl;                // 0x1601
    MixerSTMControl m_MixerSTMControl;                // 0x1602
    MixerSTMVelocity m_MixerSTMVelocity;              // 0x1604
    //MixerPOSControl m_MixerPOSControl;                // 0x1606
    MixerENCStatusCompact m_MixerENCStatusCompact;    // 0x1A00
    //MixerENCStatus m_MixerENCStatus;                  // 0x1A01
    MixerSTMStatus m_MixerSTMStatus;                  // 0x1A03
    //MixerInfoData m_MixerInfoData;                    // 0x1A04
    //MixerPOSStatus m_MixerPOSStatus;                  // 0x1A06
    PositionDataMixer m_MixerPositionData;
};

class IOTerminal
{
  public:
    IOTerminal()
    {
        m_FirstByte = 0;
        m_LenghtInBytes = 0;
        m_IsInput = true;
        m_IsMotor = false;
    }

  public:
    int m_FirstByte;
    int m_LenghtInBytes;
    bool m_IsInput;
    bool m_IsMotor;
};

class IOChannelData
{
  public:
    IOChannelData()
    {
        m_FirstByte = 0;
        m_LengthInByte = 0;
        m_ChannelNumber = 0;
        m_IsInput = true;
    }

  public:
    int m_FirstByte;
    int m_LengthInByte;
    int m_ChannelNumber;
    bool m_IsInput;
};

class EtherCATConfigData
{
  public:
    EtherCATConfigData()
    {
        m_HandleNetworkAdapter = 0;
        m_HandleEtherCatMaster = 0;
        for (int i = 0; i < MAX_NUMBER_SLAVES; i++) m_HandleEtherCatSlave[i] = 0;
        m_HandleEtherCatDataSet = 0;
        m_HandleReadWriteMutex = 0;
        m_HandleReadEtherCatDataTask = 0;
        m_HandleReadEtherCatDataCallBack = 0;
        m_HandleIOTask = 0;
        m_HandleIOTaskCallBack = 0;
        m_NumberSlaves = 0;
        m_CurrentTimeStampIn100nsUnits = 0;
        m_LastTimeStampIn100nsUnits = 0;
        m_CyclusTimeIOTaskInms = 0.0;
        m_TimePeriodTriggerOutputOnInms = 0.0;
        m_TimePeriodDigitalOutputOnInms = 0.0;
        // memset(&m_EtherCatDataSet, 0, sizeof(EtherCatDataSet));
        m_ExecuteMixer = false;
    }
    enum IOChannels {
        DI_CONTROL_VOLTAGE = 0,
        DI_CLOCK_SIGNAL_FROM_IS,
        DI_EJECTION_CONTROL,
        AI_TANK_TEMPERATURE,
        AI_HEATING_PIPE_TEMPERATURE,
        AI_ACTUAL_AIR_COOLING_CAMERA,
        AI_ACTUAL_AIR_COOLING_CAMERA_LIGHT,
        AI_ACTUAL_AIR_COOLING_VALVE,
        AI_ACTUAL_WATER_COOLING,
        AI_ACTUAL_FLOW_TRANSMITTER_WATER_COOLING_CIRCUIT,
        AI_ACTUAL_TEMPERATURE_WATER_COOLING_RETURN,
        AI_ACTUAL_PREASURE_VALUE,
        AI_PREASURE_TANK_FILLING_LEVEL_VALUE,
        DO_ERROR_LIGHT,
        DO_ERROR_TRANSFER,
        DO_COUNTER_EJECTION_TRANSFER,
        DO_PREASURE_TANK_HEATER,
        DO_HEATING_PIPE,
        DO_CAMERA_LIGHT,
        DO_VALVE_CONTROLLER,
        DO_BOTTLE_EJECTION,
        DO_TRIGGER1_VALVE,
        DO_TRIGGER2_VALVE,
        DO_WHITE_LIGHT,
        DO_BLUE_LIGHT,
        DO_ORANGE_LIGHT,
        DO_PREASURE_TANK_VALVE,
        DO_EJECTION_BY_IS_MASCHINE,
        AO_SET_POINT_PREASURE_VALUE,
        AO_SET_POINT_WATER_COOLING_SENSOR,
        AO_SET_POINT_AIR_COOLING_CAMERA,
        AO_SET_POINT_AIR_COOLING_LIGHT,
        AO_SET_POINT_AIR_COOLING_VALVES,
        AO_SET_POINT_WATER_COOLING,
        END_VALUE
    };
    // enum IOChannels {
    //    CONTROL_VOLTAGE = 0,
    //    CLOCK_SIGNALE_FROM_IS,
    //    EJECTION_CONTROL,
    //    ACTUAL_PREASUERE_VALUE,
    //    PREASURE_TANK_FILLING_LEVEL,
    //    PREASURE_TANK_TEMPERATURE,
    //    REFERENCE_PREASURE_VALUE,
    //    ERROR_LIGHT,
    //    ERROR_TRANSFER,
    //    COUNTER_EJECTION_TRANSFER,
    //    PREASURE_TANK_HEATER,
    //    PREASURE_TANK_VALUE,
    //    CAMERA_LIGHT,
    //    VALVE_CONTROLLER,
    //    BOTTLE_EJECTION,
    //    TRIGGER1_VALUE,
    //    TRIGGER2_VALUE,
    //    WHITE_LIGHT,
    //    BLUE_LIGHT,
    //    EJECTION_IS_MASCHINE,
    //    PREASURE_TANK_VALUE_2,
    //    HEATING_PIPE,
    //    // ACTUAL_FLOW_RATE_CAMERA,
    //    // ACTUAL_FLOW_RATE_LIGHT,
    //    // ACTUAL_FLOW_RATE_VALVE,
    //    // REFERENCE_FLOW_RATE_CAMERA,
    //    // REFERENCE_FLOW_RATE_LIGHT,
    //    // REFERENCE_FLOW_RATE_VALVE,
    //    END_VALUE
    //};

  public:
    double m_CyclusTimeIOTaskInms;
    double m_TimePeriodTriggerOutputOnInms;
    double m_TimePeriodDigitalOutputOnInms;
    unsigned long long m_CurrentTimeStampIn100nsUnits;
    unsigned long long m_LastTimeStampIn100nsUnits;
    unsigned long long m_DeltaTFromIS;
    //
    int m_HandleNetworkAdapter;
    int m_HandleEtherCatMaster;
    int m_HandleEtherCatSlave[MAX_NUMBER_SLAVES];
    int m_HandleEtherCatDataSet;
    int m_HandleReadWriteMutex;
    int m_HandleReadEtherCatDataTask;
    int m_HandleReadEtherCatDataCallBack;
    int m_HandleIOTask;
    int m_HandleIOTaskCallBack;
    int m_NumberSlaves;
    bool m_ExecuteMixer;
    // error codes mixer
    int m_ErrorCodesetMotorVoltage;
    int m_ErrorCodesetMotorCurrent;
    int m_ErrorCodesetStartType;
    int m_ErrorCodesetOperationMode;
    int m_ErrorCodesetAdressSTMControl;
    int m_ErrorCodesetAdressSTMVelocity;
    int m_ErrorCodesetAdressSTMSTMStatus;
    // int m_ErrorCodesetAdressPOSStatus;
    //int m_ErrorCodesetInfoDataCurrentSpeed;
    //int m_ErrorCodeSetAdressInfoData;
    int m_ErrorCodesetAdressENCStatusCompact;
    int m_ErrorCodeRestoreMotorSettings;
    int m_ErrorCodeSetMotorWatchDog;
    int m_ErrorCodeSetVelocityMin;
    // end error code mixer
    IOTerminal m_IOTerminals[MAX_NUMBER_SLAVES];
    IOChannelData m_IOChannels[END_VALUE];

    EtherCatDataSet m_EtherCatDataSet;
};

class MeasuringResultsLiquid
{
  public:
    MeasuringResultsLiquid()
    {
        m_CounterBottleFilled = 0;
        m_CounterBottleNotFilled = 0;
        m_CounterMiddleNotOk = 0;
        m_CounterMiddleTooLittle = 0;
        m_CounterMiddleTooMuch = 0;
        m_CounterRightTooBig = 0;
        m_CounterLeftTooBig = 0;
        m_CounterBottleNotUnderValve = 0;
        m_CounterBottleNotFilledOneAfterTheOther = 0;
        m_CounterMiddleNotOkOneAfterTheOther = 0;
        m_TheLeftValveIsFilledFirst = true;
        ClearResults();
    }
    void ClearResults()
    {
        m_MaxAmountSplashLeftROI = 0;
        m_MaxAmountSplashRightROI = 0;
        m_BottleIsUnderValve = false;
        m_AmountLiquidOk = false;
        m_AmountSplashLeftOk = false;
        m_AmountSplashRightOk = false;
        m_SumAmountLiquidFirstTriggerOk = false;
        m_SumAmountLiquidSecondTriggerOk = false;
        m_SumAmountLiquidFirstTriggerMinThresholdOk = false;
        m_SumAmountMiddleFirstTriggerMaxThresholdOk = false;
        m_SumAmountLiquidSecondTriggerMinThresholdOk = false;
        m_SumAmountMiddleSecondTriggerMaxThresholdOk = false;
        m_SumAmountMiddle = 0;
        m_SumAmountLiquidFirstTrigger = 0;
        m_SumAmountLiquidSecondTrigger = 0;
        m_StatusResults = false;
        m_InjectionTimeFinished = 0;
        m_InjectionTime = 0;
        m_MeanValueAmountMiddle = 0.0;
        m_StandardDeviationMiddle = 0.0;
        m_MeanValueAmountMiddleFirstTrigger = 0.0;
        m_StandardDeviationMiddleFirstTrigger = 0.0;
        m_MeanValueAmountMiddleSecondTrigger = 0.0;
        m_StandardDeviationMiddleSecondTrigger = 0.0;
        m_CurrentMaschineState = 0;
        m_EjectBottle = true;
        m_LiquidMeasuringReady = false;
        for (int i = 0; i < MAX_NUMBER_VALVES; i++) {
            m_MaxAmountSplashLeftROIPerSide[i] = 0;
            m_MaxAmountSplashRightROIPerSide[i] = 0;
        }
    }

  public:
    unsigned long long m_InjectionTimeFinished;
    unsigned long long m_InjectionTime;
    unsigned long long m_InjectionTimeHalf;
    int m_MaxAmountSplashLeftROIPerSide[MAX_NUMBER_VALVES];   // Umfang der Spritzer auf der linken Hälfte für jede Seite
    int m_MaxAmountSplashRightROIPerSide[MAX_NUMBER_VALVES];  // Umfang der Spritzer auf der rechten Hälfte für jede Seite
    int m_MaxAmountSplashLeftROI;                             // Gesamter Umfang der Spritzer für die linke Seite des ersten Ventils und die linke Seite des zweiten Ventils
    int m_MaxAmountSplashRightROI;                            // Gesamter Umfang der Spritzer für die rechte Seite des ersten Ventils und die rechte Seite des zweiten Ventils
    int m_SumAmountMiddle;                                    // Gesamte Flüssigkeitsmenge für beide Ventile zusammen
    int m_SumAmountLiquidFirstTrigger;                        // Gesamte Flüssigkeitsmenge für das erste Ventil
    int m_SumAmountLiquidSecondTrigger;                       // Gesamte Flüssigkeitsmenge für das zweite Ventil
    int m_CounterBottleFilled;
    int m_CounterBottleNotFilled;
    int m_CounterMiddleNotOk;
    int m_CounterMiddleTooLittle;
    int m_CounterMiddleTooMuch;
    int m_CounterRightTooBig;
    int m_CounterLeftTooBig;
    int m_CounterBottleNotUnderValve;
    int m_CurrentMaschineState;
    int m_CounterBottleNotFilledOneAfterTheOther;  // Anzahl Flaschen die hintereinader ausgeworfen wurden
    int m_CounterMiddleNotOkOneAfterTheOther;      // Anzahl Volumenmege zu gering hintereinader
    bool m_StatusResults;
    bool m_BottleIsUnderValve;
    bool m_AmountLiquidOk;
    bool m_AmountSplashLeftOk;
    bool m_AmountSplashRightOk;
    bool m_SumAmountLiquidFirstTriggerOk;
    bool m_SumAmountLiquidSecondTriggerOk;
    bool m_SumAmountLiquidFirstTriggerMinThresholdOk;
    bool m_SumAmountMiddleFirstTriggerMaxThresholdOk;
    bool m_SumAmountLiquidSecondTriggerMinThresholdOk;
    bool m_SumAmountMiddleSecondTriggerMaxThresholdOk;
    bool m_EjectBottle;
    bool m_LiquidMeasuringReady;
    bool m_TheLeftValveIsFilledFirst;
    double m_MeanValueAmountMiddle;
    double m_StandardDeviationMiddle;
    double m_MeanValueAmountMiddleFirstTrigger;
    double m_StandardDeviationMiddleFirstTrigger;
    double m_MeanValueAmountMiddleSecondTrigger;
    double m_StandardDeviationMiddleSecondTrigger;

    MeasuringResultsLiquid& operator=(const MeasuringResultsLiquid& other)
    {
        if (this != &other) {
            m_MaxAmountSplashLeftROI = other.m_MaxAmountSplashLeftROI;
            m_MaxAmountSplashRightROI = other.m_MaxAmountSplashRightROI;
            m_BottleIsUnderValve = other.m_BottleIsUnderValve;
            m_CounterBottleFilled = other.m_CounterBottleFilled;
            m_CounterBottleNotFilled = other.m_CounterBottleNotFilled;
            m_CounterMiddleNotOk = other.m_CounterMiddleNotOk;
            m_CounterMiddleTooLittle = other.m_CounterMiddleTooLittle;
            m_CounterMiddleTooMuch = other.m_CounterMiddleTooMuch;
            m_CounterRightTooBig = other.m_CounterRightTooBig;
            m_CounterLeftTooBig = other.m_CounterLeftTooBig;
            m_CounterBottleNotUnderValve = other.m_CounterBottleNotUnderValve;
            m_CounterBottleNotFilledOneAfterTheOther = other.m_CounterBottleNotFilledOneAfterTheOther;
            m_CounterMiddleNotOkOneAfterTheOther = other.m_CounterMiddleNotOkOneAfterTheOther;
            m_SumAmountMiddle = other.m_SumAmountMiddle;
            m_StatusResults = other.m_StatusResults;
            m_MeanValueAmountMiddle = other.m_MeanValueAmountMiddle;
            m_StandardDeviationMiddle = other.m_StandardDeviationMiddle;
            m_InjectionTimeFinished = other.m_InjectionTimeFinished;
            m_InjectionTime = other.m_InjectionTime;
            m_SumAmountLiquidFirstTrigger = other.m_SumAmountLiquidFirstTrigger;
            m_SumAmountLiquidSecondTrigger = other.m_SumAmountLiquidSecondTrigger;
            m_MeanValueAmountMiddleFirstTrigger = other.m_MeanValueAmountMiddleFirstTrigger;
            m_StandardDeviationMiddleFirstTrigger = other.m_StandardDeviationMiddleFirstTrigger;
            m_MeanValueAmountMiddleSecondTrigger = other.m_MeanValueAmountMiddleSecondTrigger;
            m_StandardDeviationMiddleSecondTrigger = other.m_StandardDeviationMiddleSecondTrigger;
            m_CurrentMaschineState = other.m_CurrentMaschineState;
            m_EjectBottle = other.m_EjectBottle;
            m_LiquidMeasuringReady = other.m_LiquidMeasuringReady;
            m_AmountLiquidOk = other.m_AmountLiquidOk;
            m_AmountSplashLeftOk = other.m_AmountSplashLeftOk;
            m_AmountSplashRightOk = other.m_AmountSplashRightOk;
            m_TheLeftValveIsFilledFirst = other.m_TheLeftValveIsFilledFirst;

            m_SumAmountLiquidFirstTriggerOk = other.m_SumAmountLiquidFirstTriggerOk;
            m_SumAmountLiquidSecondTriggerOk = other.m_SumAmountLiquidSecondTriggerOk;
            m_SumAmountLiquidFirstTriggerMinThresholdOk = other.m_SumAmountLiquidFirstTriggerMinThresholdOk;
            m_SumAmountMiddleFirstTriggerMaxThresholdOk = other.m_SumAmountMiddleFirstTriggerMaxThresholdOk;
            m_SumAmountLiquidSecondTriggerMinThresholdOk = other.m_SumAmountLiquidSecondTriggerMinThresholdOk;
            m_SumAmountMiddleSecondTriggerMaxThresholdOk = other.m_SumAmountMiddleSecondTriggerMaxThresholdOk;

            for (int i = 0; i < MAX_NUMBER_VALVES; i++) {
                m_MaxAmountSplashLeftROIPerSide[i] = other.m_MaxAmountSplashLeftROIPerSide[i];
                m_MaxAmountSplashRightROIPerSide[i] = other.m_MaxAmountSplashRightROIPerSide[i];
            }
        }
        return *this;
    }
};

class MeasuringResults
{
  public:
    MeasuringResults()
    {
        m_CurrentTimeStampInns = 0;
        m_LastTimeStampInns = 0;
        m_SizeProfileData = 0;
        m_CurrentSpeedInmmPerms = -1.0;  // geschwindikeit ist noch nicht berechnet
        m_CounterBottleInROI = 0;
        m_CalculatePixelSizeInMMPerPixel = 0.0;
        m_CorrectedTimeOffset = 0.0;
        m_MeasuringTimeInms = 0.0;
        m_StatusResults = false;  // if true result are new
        m_CounterContrastOutOfTol = 0;
        m_CounterSizeIsLongTimeOutOfTol = 0;
        m_CounterEdgeIsLongTimeOutOfTol = 0;
        ClearResults();
    }
    void ClearResults()
    {
        m_EdgeLeftFound = false;
        m_EdgeRightFound = false;
        m_ProductSizeInTolerance = false;
        m_ProductInMinTolerance = false;
        m_ProductContrastOk = false;
        m_ResultEdgeLeftXPos = 0.0;
        m_EdgeLeftContrastInPercent = 0.0;
        m_ResultEdgeRightXPos = 0.0;
        m_EdgeRightContrastInPercent = 0.0;
        m_BottleMiddelPositionInmm = 0.0;
        m_SizeProfileData = 0;
        m_MeasuredBottleNeckDiameterInmm = 0.0;
        m_BottleContrastInPercent = 0.0;
        m_BottleMatchScoreInPercent = 0.0;
        m_CurrentTimeStampInms = 0.0;
        m_DriftInmmBottlePosToMeasureWindowMiddlePos = 0.0;
        for (int i = 0; i < MAX_PROFILE_DATA_LENGHT; i++) {
            m_GradientProfileData[i] = 0.0;
            m_RawProfileData[i] = 0.0;
        }
    }
    bool EdgesFound() { return m_EdgeLeftFound && m_EdgeRightFound ? true : false; }
    bool ProductFound()
    {
        if (EdgesFound() && m_ProductSizeInTolerance && m_ProductContrastOk)
            return true;
        else
            return false;
    }

  public:
    bool m_EdgeLeftFound;
    bool m_EdgeRightFound;
    bool m_ProductSizeInTolerance;
    bool m_ProductContrastOk;
    bool m_ProductInMinTolerance;
    bool m_StatusResults;
    double m_ResultEdgeLeftXPos;
    double m_EdgeLeftContrastInPercent;
    double m_ResultEdgeRightXPos;
    double m_EdgeRightContrastInPercent;
    double m_CurrentTimeStampInms;
    double m_BottleMiddelPositionInmm;
    double m_DriftInmmBottlePosToMeasureWindowMiddlePos;
    double m_MeasuringTimeInms;
    double m_CurrentSpeedInmmPerms;
    double m_MeasuredBottleNeckDiameterInmm;
    double m_BottleContrastInPercent;
    double m_BottleMatchScoreInPercent;
    double m_CalculatePixelSizeInMMPerPixel;
    double m_CorrectedTimeOffset;
    double m_GradientProfileData[MAX_PROFILE_DATA_LENGHT];  // Daten für den Kurvenverlauf (Plotwindow)
    double m_RawProfileData[MAX_PROFILE_DATA_LENGHT];       // Daten für den Kurvenverlauf (Plotwindow)
    double m_LeftEdgeLocation;                              // Daten für den Kurvenverlauf (Plotwindow)
    double m_RightEdgeLocation;                             // Daten für den Kurvenverlauf (Plotwindow)
    int m_SizeProfileData;
    int m_CounterBottleInROI;
    unsigned long long m_CurrentTimeStampInns;
    unsigned long long m_LastTimeStampInns;
    unsigned long long m_CounterContrastOutOfTol;
    unsigned long long m_CounterSizeIsLongTimeOutOfTol;
    unsigned long long m_CounterEdgeIsLongTimeOutOfTol;

    // MeasuringResultsLiquid m_CurrentMeasuringResultsLiqiud;//Enthält die Messergebnisse der Berechnung der Flüssigkeitsmenge die in dei Flasch Iniziert worden ist

    MeasuringResults& operator=(const MeasuringResults& other)
    {
        if (this != &other) {
            m_EdgeLeftFound = other.m_EdgeLeftFound;
            m_EdgeRightFound = other.m_EdgeRightFound;
            m_ProductSizeInTolerance = other.m_ProductSizeInTolerance;
            m_ProductContrastOk = other.m_ProductContrastOk;
            m_ProductInMinTolerance = other.m_ProductInMinTolerance;
            m_ResultEdgeLeftXPos = other.m_ResultEdgeLeftXPos;
            m_EdgeLeftContrastInPercent = other.m_EdgeLeftContrastInPercent;
            m_ResultEdgeRightXPos = other.m_ResultEdgeRightXPos;
            m_EdgeRightContrastInPercent = other.m_EdgeRightContrastInPercent;
            m_CurrentTimeStampInms = other.m_CurrentTimeStampInms;
            m_BottleMiddelPositionInmm = other.m_BottleMiddelPositionInmm;
            m_DriftInmmBottlePosToMeasureWindowMiddlePos = other.m_DriftInmmBottlePosToMeasureWindowMiddlePos;
            m_MeasuringTimeInms = other.m_MeasuringTimeInms;
            m_CurrentSpeedInmmPerms = other.m_CurrentSpeedInmmPerms;
            m_MeasuredBottleNeckDiameterInmm = other.m_MeasuredBottleNeckDiameterInmm;
            m_BottleContrastInPercent = other.m_BottleContrastInPercent;
            m_BottleMatchScoreInPercent = other.m_BottleMatchScoreInPercent;
            m_CalculatePixelSizeInMMPerPixel = other.m_CalculatePixelSizeInMMPerPixel;
            m_CorrectedTimeOffset = other.m_CorrectedTimeOffset;
            m_SizeProfileData = other.m_SizeProfileData;
            m_StatusResults = other.m_StatusResults;
            m_CounterContrastOutOfTol = other.m_CounterContrastOutOfTol;
            m_CounterSizeIsLongTimeOutOfTol = other.m_CounterSizeIsLongTimeOutOfTol;
            m_CounterEdgeIsLongTimeOutOfTol = other.m_CounterEdgeIsLongTimeOutOfTol;
            m_LeftEdgeLocation = other.m_LeftEdgeLocation;
            m_RightEdgeLocation = other.m_RightEdgeLocation;
            for (int i = 0; i < m_SizeProfileData; i++) {
                m_GradientProfileData[i] = other.m_GradientProfileData[i];
                m_RawProfileData[i] = other.m_RawProfileData[i];
            }
            m_CounterBottleInROI = other.m_CounterBottleInROI;
            m_CurrentTimeStampInns = other.m_CurrentTimeStampInns;
            m_LastTimeStampInns = other.m_LastTimeStampInns;
            // m_CurrentMeasuringResultsLiqiud              = other.m_CurrentMeasuringResultsLiqiud;
        }
        return *this;
    }
};

class MeasuringParameter
{
  public:
    bool m_AbortMeasuringTask;
    bool m_AbortInputOutputTask;
    bool m_ReadyMeasuringTask;
    bool m_TriggerGetNewVideoFromRealTimeContext;
    bool m_ReadVideoFromRealTimeContext;
    bool m_ClearVideoBuffer;
    bool m_BlowOutEjectorNormallyClosed;
    bool m_ResetAllCounters;  // m_ResetAllLiquidCouners;
    bool m_ResetCountersBottlesEjectionAndLiquidTooLow;
    bool m_RightTriggerIsFirst;
    bool m_WorkWithTwoValves;
    bool m_CalibrateModus;
    bool m_UseSpeedFromISCalcEjectionTime;
    bool m_DeaktivateCheckBottleUnderValve;
    int m_InfoLevel;
    // int m_VideoFlag;
    int m_MeasureWindowWidth[NUMBER_MEASUREWINDOWS];
    int m_MeasureWindowHeight[NUMBER_MEASUREWINDOWS];
    int m_MeasureWindowOffsetX[NUMBER_MEASUREWINDOWS];
    int m_MeasureWindowOffsetY[NUMBER_MEASUREWINDOWS];

    int m_MeasureWindowWidthCopy[NUMBER_MEASUREWINDOWS];
    int m_MeasureWindowHeightCopy[NUMBER_MEASUREWINDOWS];
    int m_MeasureWindowOffsetXCopy[NUMBER_MEASUREWINDOWS];
    int m_MeasureWindowOffsetYCopy[NUMBER_MEASUREWINDOWS];

    int m_EdgeThreshold;
    int m_ReferenceMeasurePositionInPixel;
    int m_ReferenceInjectionPositionInPixel;
    int m_FilterKernelSize;
    int m_CurrentFifoSize;
    int m_BandDirectional;
    int m_ImageBackgroundContrast;
    int m_MinNumberFoundedInROI;
    int m_TargetProcessorIOTask;
    int m_TargetProcessorMeasureTask;
    int m_ThresholdBinaryImageLiquid;
    int m_NumberProductsAverageBottleNeckAndPixelSize;
    int m_MaxMeasurementsProductIsOutOfTol;
    int m_UsedTriggerOutputs;
    int m_RollingMeanValueLiquid;
    // int m_MinAmountLiquidBottleUnderValve;
    double m_DistanceBetweenMeasureMiddlePosAndTriggerPosInPixel;
    double m_BotteleNeckDiameterToleranceInmm;
    double m_PixelSizeInMMPerPixel;
    double m_TriggerOffsetFirstValveInmm;
    double m_TriggerOffsetSecondValveInmm;
    double m_BottleneckDiameter;
    double m_ProductWidthInmm;
    double m_DistanceBottleEjectionInmm;
    double m_MinSpeedInMMPerMs;
    double m_DistancesBetweenValves;
    double m_MinAcceptanceThresholdLiquidMiddleROI;
    double m_MaxAcceptanceThresholdLiquidMiddleROI;
    double m_AcceptanceThresholdLiquidLeftAndRightROI;
    double m_InjectonWindowMiddleWidthInMm;
    double m_TimePeriodNotBlowInms;
    double m_BottleOffsetOutOfROIInmm;
    // double m_FillingPosTol;
    double m_FormatFromISInmm;  // Um die Geschwindigkeit von der IS Maschine zu berechnen. Diese Geschw. wird benutzt um den Auswurfzeitpunkt zu berechnen
    long long m_TimeStampOffsetIn100nsUnits;
    MeasuringResults m_AveragedMeasuringResults;
    MeasuringResults m_CurrentMeasuringResults;
    MeasuringResultsLiquid m_CurrentMeasuringResultsLiqiud;

    void CopyMeasureWindows()
    {
        for (int i = 0; i < NUMBER_MEASUREWINDOWS; i++) {
            m_MeasureWindowWidthCopy[i] = m_MeasureWindowWidth[i];
            m_MeasureWindowHeightCopy[i] = m_MeasureWindowHeight[i];
            m_MeasureWindowOffsetXCopy[i] = m_MeasureWindowOffsetX[i];
            m_MeasureWindowOffsetYCopy[i] = m_MeasureWindowOffsetY[i];
        }
    }
};

// Aufbau des Shared Memory für den Datenaustausch zwischen der Application(Windows) und dem Echtzeitteil
class ExchangeMemory
{
  public:
    ExchangeMemory* pAppPtr;
    ExchangeMemory* pSysPtr;
    int m_HandleKernel;
    int m_HandleImageReceivedEvent;
    int m_HandleEventCanCopyVideoData;
    int m_HandleEventMeasuringTaskFinished;
    int m_HandleEventInputOutputTaskFinished;
    int m_HandleImageReceivedCallBack;
    int m_HandleEventBottleEjected;
    int m_HandleController;
    int m_HandleImageBlock;
    int m_HandleVideoBlock;
    int m_HandleVideoBlockCameraSimulation;
    int m_HandleCleanImageBlock;
    int m_HandleVideoBlockFillingProcess;
    int m_HandleCamera;
    int m_HandleStream;
    int m_HandleImageReceivedEventMeasuring;
    int m_HandleMeasureTaskCallBack;
    int m_HandleMeasuringTask;
    int m_HandleCameraSimulationTask;
    int m_HandleEventInputOutput;
    int m_InfoCodeInputOutputDevice;
    int m_KS_ErrorCode;
    int m_MaschineState;
    int m_VideoStateCameraSimulation;
    int m_UseManualTriggerOutputs;
    int m_NumberEjectedBottlesByUser;
    int m_CounterNumberBottlesRealEjected;
    char m_BufferInfoText[512];
    bool m_RealTimeModusOnRing0;
    bool m_MeasuringTaskIsStarted;
    bool m_IOTaskIsStatred;
    bool m_CameraSimulationOn;
    bool m_AbortCameraSimulation;
    bool m_StopSimulation;
    bool m_EtherCatSimulation;
    bool m_LiquidFlowSimulationOn;
    bool m_EnableTrigger;
    bool m_SetManualTrigger;
    bool m_ButtonIsClickedEjectTheNextnBottles;
    bool m_InfoLeftTriggerIsSet;
    bool m_InfoRightTriggerIsSet;
    // bool m_EnableTriggerOutputs;
    unsigned __int64 m_CounterCameraFrames;
    unsigned __int64 m_DebugCounter;
    MeasuringParameter m_MeasuringParameter;  // Enthält die Messparameter und die Messergebnisse
    EtherCATConfigData m_EtherCATConfigData;  // Enthält alle Parameter für die Kommunikation mit den Beckhoffklemmen und das Speicherabbild
};
