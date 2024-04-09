#pragma once

#include "QtCore"
#include "QtGui"

#define APP_VERSION_EPOCH 1
#define APP_VERSION_MAJOR 0
#define APP_VERSION_MINOR 0

const QString APP_VERSION = QString("%1.%2.%3").arg(APP_VERSION_EPOCH).arg(APP_VERSION_MAJOR).arg(APP_VERSION_MINOR);

// const QString ORGANIZATION_NAME = "BertramBildverarbeitung";
const QString APPLICATION_NAME = "ClearSafe";
const QString PLUGIN_DIR = "Plugins";
const QString REGEDIT_DIR = PLUGIN_DIR + "/" + APPLICATION_NAME;
                           

const QString KEY_NAME_SHARED_MEMORY_VIDEO_DATA = "VIDEO_DATA";
//
const int USED_CAMERA_WIDTH = 640;
const int USED_CAMERA_HEIGHT = 480;

const int LANGUAGE_INDEX_UNITED_KINDOM = 0;
const int LANGUAGE_INDEX_GERMANY = 1;
const int LANGUAGE_INDEX_FRANCE = 2;
// error codes
const int ERROR_CODE_NO_ERROR = 0;
const int ERROR_CODE_ANY_ERROR = 1;
const int ERROR_CODE_WARNING1 = 2;
const int ERROR_CODE_WARNING2 = 3;
const int ERROR_CODE_READ_WRITE = 4;
const int ERROR_CODE_NO_PRODUCTS = 5;
const int ERROR_CODE_TIME_OUT = 6;

const double FOUR_AND_1T4_INCH = 25.4 * 4.25;         // 107.95 mm
const double FIVE_AND_1T4_INCH = 25.4 * 5.25;         // 133.35 mm
const double SEVEN_INCH = 25.4 * 7.0;                 // 177.8  mm
const double SEVEN_AND_7I8_INCH = 25.4 * 7.875;       // 200.025 mm
const double EIGHT_AND_1T2_INCH = 25.4 * 8.5;         // 215.9 mm
const double EIGHT_AND_3T4_INCH = 25.4 * 8.75;        // 222.25 mm
const double NINE_AND_5T8_INCH = 25.4 * 9.625;        // 244.475 mm
const double TEN_AND_1T2_INCH = 25.4 * 10.5;          // 266,7 mm
const double ELEVEN_AND_13T16_INCH = 25.4 * 11.8125;  // 300.0375 mm
const double THIRTEEN_AND_1T8_INCH = 25.4 * 13.125;   // 333.375 mm
const double FOURTEENT_INCH = 25.4 * 14.0;            // 355.6 mm

const int VIDEO_DIALOG_SHOW_TRIGGER_POSITION = 1;
const int VIDEO_DIALOG_SHOW_FULL_VIDEO = 2;
const int VIDEO_DIALOG_SHOW_PRODUCT_VIDEO = 3;

const int DRAW_NO_COLOR_FRAME = 0;
const int DRAW_GREEN_COLOR_FRAME = 1;
const int DRAW_RED_COLOR_FRAME = 2;

const int ALARM_LEVEL_OK = 0x0;
const int ALARM_LEVEL_WARNING = 0x4;
const int ALARM_LEVEL_WARNING_MAX = 0x2;
const int ALARM_LEVEL_WARNING_MIN = 0x1;
const int ALARM_LEVEL_ALARM = 0x40;
const int ALARM_LEVEL_ALARM_MAX = 0x20;
const int ALARM_LEVEL_ALARM_MIN = 0x10;
const int ALARM_LEVEL_MASCHINE_STOP = 0x80;

// SiemensHMI Colors
const QColor COLOR_TITLE_BAR = QColor(0, 95, 135);
const QColor COLOR_MAIN_WINDOW = QColor(218, 220, 224);
const QColor COLOR_STATUS_BAR = QColor(39, 51, 56);
const QColor COLOR_PUSHBUTTONS = QColor(64, 77, 83);
const QColor COLOR_OPTION_PANEL = QColor(196, 199, 204);
const QColor COLOR_CONTENT_BOARD = QColor(249, 247, 248);
// SiemensHMI status colors
const QColor COLOR_STATUS_WARNING1 = QColor(234, 206, 33);
const QColor COLOR_STATUS_WARNING2 = QColor(231, 121, 16);
const QColor COLOR_STATUS_ALARM = QColor(202, 51, 51);
const QColor COLOR_STATUS_OK = QColor(133, 167, 7);
const QColor COLOR_STATUS_RELEASE_TRIGGER = QColor(204, 255, 204);

const QString PATH_NMAE_ETHERCAT_XML_CONFIG = "EtherCatXMLFiles";
const QString FILE_NAME_ETHERCAT_BUS_TERMINALS = "EtherCatSlaveData";

const double SUPPORT_RECT_SIZE_IN_PIXEL = 12.0;
// Change MeasureWindowROI by Mouse
const int CURSER_POSITION_NOT_ON_ROI = 0;
const int CURSER_POSITION_MOVE_TOP_LINE = 1;
const int CURSER_POSITION_MOVE_BOTTOM_LINE = 2;
const int CURSER_POSITION_MOVE_LEFT_LINE = 3;
const int CURSER_POSITION_MOVE_RIGHT_LINE = 4;
const int CURSER_POSITION_RESIZE_TOP = 5;
const int CURSER_POSITION_RESIZE_BOTTOM = 6;
const int CURSER_POSITION_RESIZE_LEFT = 7;
const int CURSER_POSITION_RESIZE_RIGHT = 8;
const int CURSER_POSITION_MOVE_IN_ROI = 9;

const int MINIMUM_ROI_SIZE_IN_PIXEL = 12;

const int MAX_IMAGE_DISPLAY_RESOLUTION = 1024;

const int PLAY_VIDEO = 1;
const int STOP_VIDEO = 2;
const int SKIP_BACKWARD = 3;
const int SKIP_FORWARD = 4;
const int SAVE_VIDEO = 5;

const int PROFILE_GRADIENT = 0;
const int PROFILE_GRAY_VALUE = 1;

// valve const
const int VALVE_MODUS_TIMED = 0;
const int VALVE_MODUS_CONTINUOUS = 1;
const int VALVE_MODUS_PRUGE = 2;

const int NORDSON_VALVE_MODUS_TIMED = 1;
const int NORDSON_VALVE_MODUS_CONTINUOUS = 3;
const int NORDSON_VALVE_MODUS_PURGE = 2;

const int VALVE_MODE_HEATER_OFF = 0;
const int VALVE_MODE_HEATER_ON = 1;

const int VALVE_PIEZO_CURRENT_OFF = 0;
const int VALVE_PIEZO_CURRENT_ON = 1;

const QString RESPONSE_END_TAG = "<3";
const QString RESPONSE_ALARM_TAG = "Alarm";
const QString END_TAG_UNKNOWN_COMMAND = "<?";
const QString RESPONSE_INVALID_VALUE = "Invalid";

const QString COMMAND_GET_STATUS_ALARM = "stat";
const QString COMMAND_RESET_ERROR = "arst";
const QString COMMAND_GET_STATUS_HEATERS = "rhtr";
const QString COMMAND_GET_STATUS_VALVE = "rdr1";
const QString COMMAND_GET_LAST_ERRORS = "ralr";

// valve settings
const QString COMMAND_COUNT = "dcn1";
const QString COMMAND_PULSE = "ont1";
const QString COMMAND_PAUSE = "oft1";
const QString COMMAND_MODUS = "drv1";

const QString COMMAND_SET_VALVE_COUNT = "xxxxx" + COMMAND_COUNT;
const QString COMMAND_SET_VALVE_PULSE = "xxxx.xx" + COMMAND_PULSE;
const QString COMMAND_SET_VALVE_PAUSE = "xxxx.xx" + COMMAND_PAUSE;
const QString COMMAND_SET_VALVE_MODUS = "x" + COMMAND_MODUS;

// ramp settings
const QString COMMAND_CLOSE_VOLTAGE = "volp";
const QString COMMAND_STROKE = "strk";
const QString COMMAND_OPEN = "opnt";
const QString COMMAND_CLOSE = "clst";

const QString COMMAND_SET_VALVE_CLOSE_VOLTAGE = "xxx" + COMMAND_CLOSE_VOLTAGE;
const QString COMMAND_SET_VALVE_STROKE = "xxx" + COMMAND_STROKE;
const QString COMMAND_SET_VALVE_OPEN = "xxxx" + COMMAND_OPEN;
const QString COMMAND_SET_VALVE_CLOSE = "xxxx" + COMMAND_CLOSE;

// heaters settings
const QString COMMAND_TEMPERATURE = "stmp";
const QString COMMAND_TEMPERATURE_MODE = "chtr";

const QString COMMAND_SET_TEMPERATURE = "xxx.x" + COMMAND_TEMPERATURE;
const QString COMMAND_SET_TEMPERATURE_MODE = "x" + COMMAND_TEMPERATURE_MODE;

const QString COMMAND_PIEZO_CURRENT = "dpwr";
const QString COMMAND_SET_PIEZO_CURRENT = "x" + COMMAND_PIEZO_CURRENT;

const int LEFT_VALVE_ID = 1;
const int RIGHT_VALVE_ID = 2;

const int LEFT_VIDEO_POS = 1;
const int RIGHT_VIDEO_POS = 2;

// login levels
const int LOGIN_LEVEL_OPERATOR = 1;  // logout level
const int LOGIN_LEVEL_SUPER_OPERATOR = 2;
const int LOGIN_LEVEL_ADMINISTRATOR = 3;
const int LOGIN_LEVEL_BERTRAM = 10;

const int SHOW_ON_OVERVIEW_LEFT_TRIGGER = 1;
const int SHOW_ON_OVERVIEW_RIGHT_TRIGGER = 2;
const int SHOW_ON_OVERVIEW_BOTH_TRIGGER = 3;

//
enum MEASUR_WINDOW_IDS { ROI_ID_CAMERA, ROI_ID_MEASURE_SPEED, ROI_ID_MEASURE_LIQUID, ROI_ID_MEASURE_BOTTLE_UNDER_VALVE, ROI_ID_CHECK_CLEAN_IMAGE };
// Names EtherCat Slaves Channels

// Namenskonventionen der einzelnen I/O Kanaele die Namen müssen mit dem Inhalt der Konfigurationsdatei übereinstimmen

// Klemme [K10] Position 1 EL1008 Digital Input
// Ist die Steuerspannung eingeschaltet
const QString DI_NAME_CONTROL_VOLTAGE = "ControlVoltage";
// Taktsignal von der IS-Maschine, wird benötigt um die Bandgeschwindikeit über die Vorgabe des Formates(in Zoll) der IS Maschine zu bestimmen
const QString DI_NAME_CLOCK_SIGNAL_FROM_IS = "ClockSignalFromIS";
// Info das eine Flasche ausgeworfen wurde, Signal wird weitergeleitet zum Digitalen Ausgang K40 Kanal 8
const QString DI_NAME_EJECTION_CONTROL = "EjectionControl";
// Ende [K10]

// Klemme [K31] Position 2 EL3202 Analog Input
// Analoger Temperaturwert des Flüssigkeitsbehälters
const QString AI_NAME_TANK_TEMPERATURE = "TankTemperature";
// Analoger Temperaturwert des Heizschlauches
const QString AI_NAME_HEATING_PIPE_TEMPERATURE = "HeatingPipe";
// Ende Klemme [K31]

// Klemme [K32] Position 3 EL3202 Analog Input
// Luftkuehlung Kamera Istwert
const QString AI_NAME_ACTUAL_AIR_COOLING_CAMERA_AND_BACK_LIGHT = "ActualAirCoolingCameraAndBackLight";
// Luftkuehlung Hintergrundbeleuchtung Istwert
const QString AI_NAME_ACTUAL_AIR_COOLING_GLASS = "ActualAirCoolingGlass";
// Luftkuehlung Dosierventile Istwert
const QString AI_NAME_ACTUAL_AIR_COOLING_VALVE = "ActualAirCoolingValves";
// Volumenstromregler Wasserkuehlkreislauf Istwert
const QString AI_NAME_ACTUAL_WATER_COOLING = "ActualWaterCooling";
// Ende Klemme [K32]

// Klemme [K33] Position 4 EL3202 Analog Input
const QString AI_NAME_ACTUAL_FLOW_TRANSMITTER_WATER_COOLING_CIRCUIT = "ActualFlowTransmitterWaterCoolingCircuit";
const QString AI_NAME_ACTUAL_TEMPERATURE_WATER_COOLING_RETURN = "ActualTemperaturWaterCoolingReturn";
const QString AI_NAME_ACTUAL_PREASURE = "ActualPreasureValue";
const QString AI_NAME_ACTUAL_TANK_FILLING_LEVEL = "ActualTankFillingLevel";
// Ende Klemme [K33]

// Klemme [K40] Position 5 EL2008 Digital Output
const QString DO_NAME_ERROR_LIGHT = "ErrorLight";
const QString DO_NAME_ERROR_TRANSFER = "ErrorTransfer";
const QString DO_NAME_COUNTER_EJECTION_TRANSFER = "CounterEjectionTransfer";
const QString DO_NAME_PREASURE_TANK_HEATER = "PressureTankHeater";
const QString DO_NAME_HEATING_PIPE = "HeatingPipe";
const QString DO_NAME_CAMERA_LIGHT = "CameraLight";
const QString DO_NAME_VALVE_CONTROLLER = "ValveController";
const QString DO_NAME_BOTTLE_EJECTION = "BottleEjection";
// Ende Klemme [K40]

// Klemme [K41] Position 6 EL2252 Digital Output
const QString DO_NAME_TRIGGER1_VALVE = "Trigger1Valve";
const QString DO_NAME_TRIGGER2_VALVE = "Trigger2Valve";
// Ende Klemme [K40]

// Klemme [K42] Position 7 EL2008 Digital Output
const QString DO_NAME_WHITE_LIGHT = "WhiteLightBottleEjection";
const QString DO_NAME_BLUE_LIGHT = "BlueLightEjectionIS";
const QString DO_NAME_PREASURE_TANK_VALVE = "PressureTankValve";
const QString DO_NAME_EJECTION_BY_IS_MASCHINE = "EjectionIS";
const QString DO_NAME_ORANGE_LIGHT = "OrangeLightWarning";
// Ende Klemme [K42]

// Klemme [K70] Position 8 EL4122 Digital Output
const QString AO_NAME_SET_POINT_PREASURE_VALUE = "SetPointPreasureValue";
const QString AO_NAME_SET_POINT_WATER_COOLING_SENSOR = "SetPointWaterCoolingSensor";
// Ende Klemme [K70]

// Klemme [K71] Position 8 EL4124 Digital Output
const QString AO_NAME_SET_POINT_AIR_COOLING_CAMERA_AND_BACK_LIGHT = "SetPointAirCoolingCameraAndBackLight";
const QString AO_NAME_SET_POINT_AIR_COOLING_GLASS = "SetPointAirCoolingGlass";
const QString AO_NAME_SET_POINT_AIR_COOLING_VALVES = "SetPointAirCoolingValves";
const QString AO_NAME_SET_POINT_WATER_COOLING      = "SetPointWaterCooling";
// Ende Klemme [K71]

//const QString DO_NAME_PREASURE_TANK_VALVE_2 = "PressureTanksValve2";


const double INVALID_TEMPERATURE_VALUE = 0.0;

const int OVERVIEW_DIALOG_INDEX = 0;
const int IMAGEVIEW_DIALOG_INDEX = 1;
//const int GENERAL_DIALOG_INDEX = 2;
const int PARAMETER_DIALOG_INDEX = 2;
const int SETTINGS_DIALOG_INDEX = 3;
const int TREND_GRAPH_DIALOG_INDEX = 4;
const int CLEAN_IMAGE_DIALOG_INDEX = 5;
const int EJECTED_BOTTLES_DIALOG_INDEX = 6;
//const int MAINTENANCE_DIALOG_INDEX = 8;
//const int COOLING_DIALOG_INDEX = 9;

const QString PROPERTY_NAME_CHANGE_BACKGROUND_COLOR = "STATE";

const int TRIGGER_ON = 1;
const int TRIGGER_OFF = 2;

const int BLOW_ON = 1;
const int BLOW_OFF = 2;

const int STEP_ONE_AUTO_CALIBRATE_SEARCH_BOTTLE_TOP_LINE = 0;
const int STEP_TWO_AUTO_CALIBRATE_SEARCH_OPTIMAL_ROI_POS = 1;
const int STEP_THRE_AUTO_CALIBRATE_SEARCH_OPTIMAL_ROI_HEIGHT = 2;
const int STEP_FOUR_AUTO_CALIBRATE_SEARCH_OPTIMAL_EDGE_THRESHOLD = 3;
const int STEP_FIVE_AUTO_CALIBRATE_SEARCH_OPTIMAL_PIXEL_SIZE = 4;

const double TRIGGER_SLIDER_FACTOR = 1.0 / 20;
const double VIDEO_POSITION_SLIDER_FACTOR = 1.0 / 20;

const int GRAPH_SHOW_TEMP_PIEZO_LEFT = 0x01;
const int GRAPH_SHOW_TEMP_CHAMBER_LEFT = 0x02;
const int GRAPH_SHOW_TEMP_PIEZO_RIGHT = 0x04;
const int GRAPH_SHOW_TEMP_CHAMBER_RIGHT = 0x08;
const int GRAPH_SHOW_TEMP_PREASURE_TANK = 0x10;
const int GRAPH_SHOW_DROPLET_LEFT_VALVE_MEAN = 0x20;
const int GRAPH_SHOW_DROPLET_RIGHT_VALVE_MEAN = 0x40;
const int GRAPH_SHOW_DROPLET_SUM_MEAN = 0x80;
const int GRAPH_SHOW_DROPLET_LEFT_VALVE_RAW_POINTS = 0x100;
const int GRAPH_SHOW_DROPLET_LEFT_VALVE_STD = 0x200;
const int GRAPH_SHOW_DROPLET_RIGHT_VALVE_RAW_POINTS = 0x400;
const int GRAPH_SHOW_DROPLET_RIGHT_VALVE_STD = 0x800;
const int GRAPH_SHOW_BOTTLES_PER_MIN = 0x1000;
const int GRAPH_SHOW_EJECTED_BOTTLES = 0x2000;
const int GRAPH_SHOW_HEATING_PIPE = 0x4000;
const int GRAPH_SHOW_WATER_COOLING = 0x8000;

const QString CSV_FILE_DATE_FORMAT = "dd_MM_yyyy";
const QString CSV_FILE_TIME_FORMAT = "hh:mm:ss:zzz";
const QString CSV_FILE_SEPERATOR = ";";

const QString TREND_GRAPH_BASE_FILE_NAME_TEMPERATURE = "Temperature";
const QString TREND_GRAPH_BASE_FILE_NAME_LIQUID = "Liquid";

const int TREND_GRAPH_PLOT_ID_TEMPERATURE = 1;
const int TREND_GRAPH_PLOT_ID_LIQUID = 2;

const QString TREND_GRAPH_FLAG_FINISHED_SOFTWARE = "Finished Software";
const QString TREND_GRAPH_FLAG_START_SOFTWARE = "Start Software";

const int TREND_GRAPH_TIME_WINDOW_INDEX_THREE_MIN = 0;
const int TREND_GRAPH_TIME_WINDOW_INDEX_FIVE_MIN = 1;
const int TREND_GRAPH_TIME_WINDOW_INDEX_FIFTEEN_MIN = 2;
const int TREND_GRAPH_TIME_WINDOW_INDEX_ONE_HOUR = 3;
const int TREND_GRAPH_TIME_WINDOW_INDEX_THREE_HOUR = 4;

const int SHOW_PLACE_LIVE_IMAGE_VIEW_EDIT_PRODUCT_DIALOG = 1;
const int SHOW_PLACE_LIVE_IMAGE_VIEW_CLEAN_IMAGE_DIALOG = 2;
const int SHOW_PLACE_LIVE_IMAGE_VIEW_DROP_ESTIMATION_DIALOG = 3;

const int ERROR_TYPE_BOTTLE_NOT_UNDER_VALVE = 2;
const int ERROR_TYPE_FILLING_QUANTITY_NOT_OK = 1;
const int ERROR_TYPE_SPLASHES_TOO_MUCH = 0;

const QString ERROR_TYPS_SUB_DIR_NAMES[3] = {"/SplashesTooMuch", "/FillingQuantityNotOk", "/BottleNotUnderValve"};
