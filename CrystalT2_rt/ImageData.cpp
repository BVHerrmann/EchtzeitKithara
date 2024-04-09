#include "ImageData.h"

#include <Windows.h>
#include <time.h>

#include "AveragingProductData.h"
#include "KrtsBertram.h"
#include "SharedData.h"
#include "VideoHeader.h"

ImageData::ImageData(ExchangeMemory* pExchangeMemory, unsigned char* pSharedMemoryImageBlock, unsigned char* pSharedMemoryVideoBlock, unsigned char* pSharedMemoryCleanImageBlock,
                     unsigned char* pSharedMemoryVideoBlockFillingProcess)
    : m_SharedMemoryImageBlock(NULL),
      m_SharedMemoryVideoBlock(NULL),
      m_SharedMemoryVideoBlockFillingProcess(NULL),
      m_ExchangeMemory(NULL),
      m_LastTimeStampInNanoSec(0),
      m_LastTimeStampCleanImageInNanoSec(0),
      m_TimeStampProductIsOutOfROIInNanoSec(0),
      m_InjectionTimeFinished(0),
      m_TimeIntervalLiveImageInNanoSec(1000000 * 100),
      m_TimeIntervalCleanImageInNanoSec(1000000 * 10000),
      m_TriggerCounter(0),
      m_MeasureWindowScannSize(0),
      m_MeasureWindowGatePosInPixel(0),
      m_MeasureInjectionPos(false),
      m_FirstOccurrenceInROI(false),
      m_ReadImageFromDisk(false),
      m_LeftTriggerIsSet(false),
      m_RightTriggerIsSet(false),
      m_TriggerDriftInms(0.0),
      m_TriggerDriftInMM(0.0),
      m_ImageWidth(0),
      m_ImageHeight(0),
      m_TestVar(false),
      m_CounterEdgeIsOutOfTol(0),
      m_CounterSizeIsOutOfTol(0),
      m_CounterSimulateSecondTrigger(0),
      m_CounterSimulateFirstTrigger(0),
      m_CounterSimulateDegreeOfPollution(0),
      m_TriggerCounterFirst(0),
      m_TriggerCounterSecond(0),
      m_CounterImagesNoFirstTriggerOccur(0),
      m_LiquidMeasuringReady(false),
      m_CounterSimulateSplashesLeft(0),
      m_CounterSimulateSplashesRight(0),
      m_StartPaintingSimulationLiquidSecondTrigger(false),
      m_StartPaintingSimulationLiquidFirstTrigger(false),
      m_CameraImageID(0),
      m_CurrentCounterEjectBottleByUser(0)
{
    m_SharedMemoryImageBlock = pSharedMemoryImageBlock;
    m_SharedMemoryVideoBlock = pSharedMemoryVideoBlock;
    m_SharedMemoryCleanImageBlock = pSharedMemoryCleanImageBlock;
    m_SharedMemoryVideoBlockFillingProcess = pSharedMemoryVideoBlockFillingProcess;
    m_ExchangeMemory = pExchangeMemory;
    for (int i = 0; i < MAX_NUMBER_VALVES; i++) {  // Bestimmung der Flüssigkeitsmenge für jedes Ventil
        m_AmountLiquidMiddleROI[i] = 0;
        m_AmountSplashRightROI[i] = 0;
        m_AmountSplashLeftROI[i] = 0;
    }
}

ImageData::~ImageData()
{
}

void ImageData::SetCurrentTimeStampInNanoSec(unsigned long long set)
{
    m_CurrentMeasuringResultsSpeed.m_LastTimeStampInns = m_CurrentMeasuringResultsSpeed.m_CurrentTimeStampInns;
    m_CurrentMeasuringResultsSpeed.m_CurrentTimeStampInns = set;
}

void ImageData::SetMeasuringTimeInms(double set)
{
    m_CurrentMeasuringResultsSpeed.m_MeasuringTimeInms = set;
}

bool ImageData::Execute(cv::Mat& CameraImage)
{
    bool ProductFound = false;
    bool ProductOutOfROI = false;
    bool LeftTriggerIsSet = false;
    bool RightTriggerIsSet = false;
    bool FirstOcurrence = false;
    bool ProducktDataBufferFull = false;
    bool EjectBottle = true;
    int MeasureWindowIndex = ROI_INDEX_MEASURE_SPEED;
    int MeasureWindowSpace = 10;
    int x, y, w, h;
    std::string PrintResultsLiquidMeasuring, PrintResultsSpeedMeasuring, DebugMsg;
    std::vector<cv::Rect> AllRoiRectsLiquid;

    m_ExchangeMemory->m_MeasuringParameter.CopyMeasureWindows();
    m_CurrentMeasuringResultsSpeed.ClearResults();                                // Löschen der Messergebniss von der vorherigen Messung
    m_CurrentMeasuringResultsTriggerPos[LEFT_TRIGGER_SIDE_INDEX].ClearResults();  // Messergebnisse der Flaschenposition unter dem Ventil einmal für rechts und einmal für links
    m_CurrentMeasuringResultsTriggerPos[RIGHT_TRIGGER_SIDE_INDEX].ClearResults();
    GetAllLiquidRects(AllRoiRectsLiquid);
    
    if (m_ExchangeMemory->m_LiquidFlowSimulationOn) {
        SimulateLiquidFlow(CameraImage, LEFT_VALVE_INDEX, AllRoiRectsLiquid);
        SimulateLiquidFlow(CameraImage, RIGHT_VALVE_INDEX, AllRoiRectsLiquid);
        // SimulateSplashes(CameraImage, RIGHT_VALVE_INDEX, LEFT_TRIGGER_SIDE_INDEX, AllRoiRectsLiquid);
        // SimulateSplashes(CameraImage, RIGHT_VALVE_INDEX, RIGHT_TRIGGER_SIDE_INDEX, AllRoiRectsLiquid);
        // SimulateSplashes(CameraImage, LEFT_VALVE_INDEX, RIGHT_TRIGGER_SIDE_INDEX, AllRoiRectsLiquid);
        // SimulateSplashes(CameraImage, LEFT_VALVE_INDEX, LEFT_TRIGGER_SIDE_INDEX, AllRoiRectsLiquid);
    }
    if (m_ExchangeMemory->m_SetManualTrigger) {
        double TriggerPointFirstInMs = m_CurrentMeasuringResultsSpeed.m_CurrentTimeStampInns / ((double)(1000000));
        double TriggerPointSecondInMs = TriggerPointFirstInMs;
        if (m_ExchangeMemory->m_UseManualTriggerOutputs == USE_BOTH_VALVES) {
            //beide Trigger können nicht exact gleichzeitig ausgelöst werden es muss immer ein zeitlicher versatz vorhanden sein
            TriggerPointSecondInMs = TriggerPointSecondInMs + m_ExchangeMemory->m_EtherCATConfigData.m_TimePeriodTriggerOutputOnInms * 2;
        }
        int temp = m_ExchangeMemory->m_MeasuringParameter.m_UsedTriggerOutputs;
        m_ExchangeMemory->m_MeasuringParameter.m_UsedTriggerOutputs = m_ExchangeMemory->m_UseManualTriggerOutputs;
        AddTriggerPointsToTriggerList(TriggerPointFirstInMs, TriggerPointSecondInMs);
        m_ExchangeMemory->m_MeasuringParameter.m_UsedTriggerOutputs = temp;  // alten wert wieder auf Ursprung
        m_ExchangeMemory->m_SetManualTrigger = false;
    }
    m_CameraImageID++;
    m_OriginalCameraImage = CameraImage.clone();
    CheckResetCounters();                             // Sollen alle Zähler zurückgesetzt werden
    MeasuringLiqiud(CameraImage, AllRoiRectsLiquid);  // Laufende Berechnung der Flüssigkeitsmenge die in die Flasche gelangt, und die Menge die daneben geht
    if (!m_ExchangeMemory->m_MeasuringParameter.m_DeaktivateCheckBottleUnderValve) {
        MeasureBottlePositionUnderValve(CameraImage);  // Befindet sich die Flasche unter dem Ventil wenn der Trigger ausgelöst wurde ?
    }
    x = m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowOffsetXCopy[ROI_INDEX_MEASURE_SPEED];
    y = m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowOffsetYCopy[ROI_INDEX_MEASURE_SPEED];
    w = m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowWidthCopy[ROI_INDEX_MEASURE_SPEED];
    h = m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowHeightCopy[ROI_INDEX_MEASURE_SPEED];  // Bestimmung der Position und die Geschwindigkeit der Flasche
    ProductFound = StartMeasuringBottlePosition(m_CurrentMeasuringResultsSpeed, CameraImage, PrintResultsSpeedMeasuring, m_MeasureWindowGatePosInPixel, x, y, w, h, ROI_INDEX_MEASURE_SPEED);
    if (ProductFound) {  // vorgabe Durchmesser für Berechnung der PixelSize
        ProducktDataBufferFull = m_AverageProductData.AddNewProductData(m_CurrentMeasuringResultsSpeed, m_ExchangeMemory->m_MeasuringParameter.m_BottleneckDiameter,
                                                                        m_ExchangeMemory->m_MeasuringParameter.m_NumberProductsAverageBottleNeckAndPixelSize);
        m_CurrentMeasuringResultsSpeed.m_CounterBottleInROI++;
        if (ProducktDataBufferFull) {  // Band steht sehr wahrscheinlich, damit beim stillstand immer noch die berechnete Pixelsize und der berechnete Durchmessser angezeigt wird
            AveragedMeasuringResults();
            m_AverageProductData.ClearProductData();
        }
        // Messfensterposition neu berechnen
        if (m_ExchangeMemory->m_MeasuringParameter.m_BandDirectional == BAND_DIRECTIONAL_LEFT_TO_RIGHT)
            m_MeasureWindowGatePosInPixel =
                static_cast<int>(m_CurrentMeasuringResultsSpeed.m_ResultEdgeLeftXPos - m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowOffsetXCopy[MeasureWindowIndex] - MeasureWindowSpace);
        else
            m_MeasureWindowGatePosInPixel =
                (m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowOffsetXCopy[MeasureWindowIndex] + m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowWidthCopy[MeasureWindowIndex]) -
                (static_cast<int>(m_CurrentMeasuringResultsSpeed.m_ResultEdgeRightXPos + MeasureWindowSpace));
        if (m_MeasureWindowGatePosInPixel < 0) m_MeasureWindowGatePosInPixel = 0;
        if (m_MeasureWindowGatePosInPixel >= CameraImage.cols) m_MeasureWindowGatePosInPixel = CameraImage.cols - 1;
        // Berechne Zeitpunkt wenn Produkt zum ersten mal Auserhalb des Messfensters
        if (!m_FirstOccurrenceInROI) {  // new product in measuring window ROI
            m_FirstOccurrenceInROI = FirstOcurrence = true;
            m_TimeStampProductIsOutOfROIInNanoSec =
                m_CurrentMeasuringResultsSpeed.m_CurrentTimeStampInns + CalculateWaitTimeBottleIsOutOfROI(m_CurrentMeasuringResultsSpeed.m_BottleMiddelPositionInmm);
            m_CounterSizeIsOutOfTol = 0;
            m_CounterEdgeIsOutOfTol = 0;
            PrintResultsSpeedMeasuring = PrintResultsSpeedMeasuring + cv::format("Found First Bottle\n");
        }
    } else {                                                                                                   // Flasche ist aus dem Sichtfeld des Messfensters wenn zeit abgelaufen
        if (m_CurrentMeasuringResultsSpeed.m_CurrentTimeStampInns >= m_TimeStampProductIsOutOfROIInNanoSec) {  // Produkt is out of ROI, average result and calculate target time
            m_MeasureWindowGatePosInPixel = 0;                                                                 // open the gate, full ROI is active for the next product
            if (m_FirstOccurrenceInROI) {                                                                      // averarge measure data
                m_CurrentMeasuringResultsSpeed.m_CounterBottleInROI = 0;
                if (m_AverageProductData.GetNumberResultsPerProducts() >= m_ExchangeMemory->m_MeasuringParameter.m_MinNumberFoundedInROI) {
                    double TriggerPointFirst, TriggerPointSecond;
                    AveragedMeasuringResults();  // und Kopie in den Shared Memory
                    CalculateTriggeringPointInTime(m_AveragedMeasuringResults, TriggerPointFirst, TriggerPointSecond);
                    AddTriggerPointsToTriggerList(TriggerPointFirst, TriggerPointSecond);
                    PrintResultsSpeedMeasuring =
                        PrintResultsSpeedMeasuring + cv::format("Trigger Point Ready.\n First:%.2fms Second:%.2fms\n", TriggerPointFirst - m_AveragedMeasuringResults.m_CurrentTimeStampInms,
                                                                TriggerPointSecond - m_AveragedMeasuringResults.m_CurrentTimeStampInms);
                }
                m_AverageProductData.ClearProductData();
                m_FirstOccurrenceInROI = false;
                ProductOutOfROI = true;  // info video data
            }
        }
        if (!ProductOutOfROI) {
            // Keine Flaschen laufen durch das Bild, Messfenster ist nicht richtig positioniert oder Eistellparameter falsch
            // Eine Flasche wird als nicht erkannt klassifiziert, wenn keine Kante gefunden(Kontrast zu gering)
            // oder die gemessene Durchmessergröße ist nicht innerhalb der Toleranz.
            // Um anzuzeigen Flasche nicht gefunden, müssen die Ergebnisse mehrmals hintereinander auftreten
            // Wurden die Kanten nicht gefunden könnte die Position und die Größe des Messfensters nicht stimmen.
            // Passt der gemessene Durchmesser nicht kann es an den eingestellten Toleranzwerten/Durchmesservorgabe liegen
            if (m_CounterEdgeIsOutOfTol > m_ExchangeMemory->m_MeasuringParameter.m_MaxMeasurementsProductIsOutOfTol) {
                m_CurrentMeasuringResultsSpeed.m_CounterEdgeIsLongTimeOutOfTol++;
                m_CounterEdgeIsOutOfTol = 0;
            }

            if (m_CounterSizeIsOutOfTol > m_ExchangeMemory->m_MeasuringParameter.m_MaxMeasurementsProductIsOutOfTol) {
                m_CurrentMeasuringResultsSpeed.m_CounterSizeIsLongTimeOutOfTol++;
                m_CounterSizeIsOutOfTol = 0;
            }
        }
    }
    LeftTriggerIsSet = CheckIsLeftTriggerSet();    // Info kommt von der IO Task
    RightTriggerIsSet = CheckIsRightTriggerSet();  // Info kommt von der IO Task
    // wird auf true gesetzt wenn die Software entscheidet, dass die Flasche ausgeforfen wird oder nicht. Trifft die Software keine Entscheidung wird die Flasche immer ausgeworfen
    m_LiquidMeasuringReady = false;
    EjectBottle = IntegrateLiquidData(m_CurrentMeasuringResultsSpeed.m_CurrentTimeStampInns, PrintResultsLiquidMeasuring, LeftTriggerIsSet, RightTriggerIsSet);
    PrintResultsSpeedMeasuring = MeasuringResultsSpeedToString(m_CurrentMeasuringResultsSpeed, PrintResultsSpeedMeasuring);

    if (m_ExchangeMemory->m_MeasuringParameter.m_InfoLevel == INFO_LEVEL_SHOW_FULL_INFO) {
        if (TheLeftValveIsFilledFirst()) {
            DrawTextOutput(CameraImage, PrintResultsLiquidMeasuring, 330, 10);
            DrawTextOutput(CameraImage, PrintResultsSpeedMeasuring, 0, 10);
        } else {
            DrawTextOutput(CameraImage, PrintResultsLiquidMeasuring, 0, 10);
            DrawTextOutput(CameraImage, PrintResultsSpeedMeasuring, 330, 10);
        }
    } else {
        if (m_ExchangeMemory->m_MeasuringParameter.m_InfoLevel == INFO_LEVEL_SHOW_ONLY_LIQUID_RESULTS) {
            if (TheLeftValveIsFilledFirst()) {
                DrawTextOutput(CameraImage, PrintResultsLiquidMeasuring, 330, 10);
            } else {
                DrawTextOutput(CameraImage, PrintResultsLiquidMeasuring, 0, 10);
            }
        } else {
            if (m_ExchangeMemory->m_MeasuringParameter.m_InfoLevel == INFO_LEVEL_SHOW_ONLY_SPEED_RESULTS) {
                if (TheLeftValveIsFilledFirst()) {
                    DrawTextOutput(CameraImage, PrintResultsSpeedMeasuring, 0, 10);
                } else {
                    DrawTextOutput(CameraImage, PrintResultsSpeedMeasuring, 330, 10);
                }
            }
        }
    }

    SetVideoAndLiveImage(CameraImage, ProductFound, ProductOutOfROI, LeftTriggerIsSet, RightTriggerIsSet, FirstOcurrence, EjectBottle, m_LiquidMeasuringReady);

    return ProductFound;
}

// Prüfung ob Flasche unter dem Ventil gelbes Messfenster
void ImageData::MeasureBottlePositionUnderValve(cv::Mat& CameraImage)
{
    int x = m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowOffsetXCopy[ROI_INDEX_MEASURE_BOTTLE_UNDER_VALVE];
    int y = m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowOffsetYCopy[ROI_INDEX_MEASURE_BOTTLE_UNDER_VALVE];
    int w = m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowWidthCopy[ROI_INDEX_MEASURE_BOTTLE_UNDER_VALVE];
    int h = m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowHeightCopy[ROI_INDEX_MEASURE_BOTTLE_UNDER_VALVE];
    int w2 = static_cast<int>(double(w) / 2.0);
    int ValveDistancehalf = GetDistanceBetweenValvesHalf();
    int NewWidth = w2 + ValveDistancehalf;
    int NewXpos;
    std::string ErrorMsg;

    if (m_ExchangeMemory->m_MeasuringParameter.m_WorkWithTwoValves) {
        switch (m_ExchangeMemory->m_MeasuringParameter.m_UsedTriggerOutputs) {
            case USE_BOTH_VALVES:
                StartMeasuringBottlePosition(m_CurrentMeasuringResultsTriggerPos[LEFT_TRIGGER_SIDE_INDEX], CameraImage, ErrorMsg, 0, x, y, NewWidth, h, ROI_INDEX_MEASURE_BOTTLE_UNDER_VALVE);
                NewXpos = x + w2 - ValveDistancehalf;
                StartMeasuringBottlePosition(m_CurrentMeasuringResultsTriggerPos[RIGHT_TRIGGER_SIDE_INDEX], CameraImage, ErrorMsg, 0, NewXpos, y, NewWidth, h, ROI_INDEX_MEASURE_BOTTLE_UNDER_VALVE);
                break;
            case USE_ONLY_LEFT_VALVE:
                if (TheLeftValveIsFilledFirst()) {
                    StartMeasuringBottlePosition(m_CurrentMeasuringResultsTriggerPos[LEFT_TRIGGER_SIDE_INDEX], CameraImage, ErrorMsg, 0, x, y, NewWidth, h, ROI_INDEX_MEASURE_BOTTLE_UNDER_VALVE);
                } else {
                    NewXpos = x + w2 - ValveDistancehalf;
                    StartMeasuringBottlePosition(m_CurrentMeasuringResultsTriggerPos[RIGHT_TRIGGER_SIDE_INDEX], CameraImage, ErrorMsg, 0, NewXpos, y, NewWidth, h,
                                                 ROI_INDEX_MEASURE_BOTTLE_UNDER_VALVE);
                }
                break;
            case USE_ONLY_RIGHT_VALVE:
                if (TheLeftValveIsFilledFirst()) {
                    NewXpos = x + w2 - ValveDistancehalf;
                    StartMeasuringBottlePosition(m_CurrentMeasuringResultsTriggerPos[RIGHT_TRIGGER_SIDE_INDEX], CameraImage, ErrorMsg, 0, NewXpos, y, NewWidth, h,
                                                 ROI_INDEX_MEASURE_BOTTLE_UNDER_VALVE);
                } else {
                    StartMeasuringBottlePosition(m_CurrentMeasuringResultsTriggerPos[LEFT_TRIGGER_SIDE_INDEX], CameraImage, ErrorMsg, 0, x, y, NewWidth, h, ROI_INDEX_MEASURE_BOTTLE_UNDER_VALVE);
                }
                break;
            default:
                StartMeasuringBottlePosition(m_CurrentMeasuringResultsTriggerPos[LEFT_TRIGGER_SIDE_INDEX], CameraImage, ErrorMsg, 0, x, y, NewWidth, h, ROI_INDEX_MEASURE_BOTTLE_UNDER_VALVE);
                NewXpos = x + w2 - ValveDistancehalf;
                StartMeasuringBottlePosition(m_CurrentMeasuringResultsTriggerPos[RIGHT_TRIGGER_SIDE_INDEX], CameraImage, ErrorMsg, 0, NewXpos, y, NewWidth, h, ROI_INDEX_MEASURE_BOTTLE_UNDER_VALVE);
                break;
        }
    } else {  // Work with one Valve
        StartMeasuringBottlePosition(m_CurrentMeasuringResultsTriggerPos[LEFT_TRIGGER_SIDE_INDEX], CameraImage, ErrorMsg, 0, x, y, w, h, ROI_INDEX_MEASURE_BOTTLE_UNDER_VALVE);
    }
}

//       side==LEFT_TRIGGER_SIDE_INDEX       side==RIGHT_TRIGGER_SIDE_INDEX
//                    v1 <-Valvedistance-> v2
// x,y _____________________________________________________
//     |            |     |             |      |            |
//     |            |     |             |      |            |
//     |  Left ROI  |Mid. |Right  Left  | Mid. | Right ROI  |   h
//     |            | ROI | ROI    ROI  | ROI  |            |
//     |____________|_____|_____________|______|____________|
//
//
//     |________________________|
//                 V
//    Messbereich erster Befüllvorgang Breite w Linke Seite, wenn Laufrichtung von links nach rechts
//
//                              |___________________________|
//                                            V
//                               Messbereich zweiter Befüllvorgang Breite w Rechte Seite, wenn Laufrichtung von links nach rechts
//
//     Zenario 1 Laufrichtung von Rechts nach Links
//
//                                            <-----------------------------  Running direction
//      Left Valve       Right valve
//         ____            ____
//        |    |          |    |
//        |    |          |    |
//        |    |          |    |
//         |  |            |  |
//
//          2.              1.      <------ Befüllreihenfolge( Erst Rechtes Ventil dann Linkes Ventil )
//
//
//     Zenario 2 Laufrichtung von Links nach Rechts
//
//     Running direction           ----------------------------->
//                                                                                     Left Valve       Right valve
//                                                                                       ____            ____
//                                                                                      |    |          |    |
//                                                                                      |    |          |    |
//                                                                                      |    |          |    |
//                                                                                       |  |            |  |
//
//  Befüllreihenfolge( Erst Linkes Ventil dann Rechtes Ventil )------------->              1.              2.
//
//
//     |<-                                w                                   ->|
//
//     |<-                w2            ->|
//
//                      LiquidSectorXpos left          LiquidSectorXpos right
//                      |                              |
//                      v                              v
//
//                        ->        ValveDistance         <-
//      __________________________________|______________________________________   _
//     |                |     |           |            |      |                 |
//     |                |     |           |            |      |                 |
//     |                |     |           |            |      |                 |   h
//     |                |     |           |            |      |                 |
//     |________________|_____|___________|____________|______|_________________|   _
//                                        |
//                           ->width_offset<-
//
//                    ->       <-
//                  LiquidSectorWidth
void ImageData::MeasuringLiqiud(cv::Mat& CameraImage, std::vector<cv::Rect>& liquidRois)
{
    int x, y, w, h;
    int LiquidSectorXpos;
    cv::Rect roi_left_splash = liquidRois[0];
    cv::Rect roi_left_liquid_sector = liquidRois[1];
    cv::Rect roi_middel_splash = liquidRois[2];
    cv::Rect roi_middel_splash_left = liquidRois[3];
    cv::Rect roi_middel_splash_right = liquidRois[4];
    cv::Rect roi_right_liquid_sector = liquidRois[5];
    cv::Rect roi_right_splash = liquidRois[6];

    if (m_ExchangeMemory->m_MeasuringParameter.m_WorkWithTwoValves) {
        switch (m_ExchangeMemory->m_MeasuringParameter.m_UsedTriggerOutputs) {
            case USE_BOTH_VALVES:
                // Messfenster wird in zwei Teilbereiche aufgeteilt: Linke und rechte Seite egal wie die Laufrichtung eingestellt
                x = roi_left_splash.x;
                y = roi_left_splash.y;
                w = roi_left_splash.width + roi_left_liquid_sector.width + roi_middel_splash_left.width;
                h = roi_left_splash.height;
                LiquidSectorXpos = roi_left_splash.width;                                                        // ROI coordinate
                StartMeasuringLiqiudAmount(CameraImage, x, y, w, h, LEFT_TRIGGER_SIDE_INDEX, LiquidSectorXpos);  // Daten Erster Befüllvorgang, wenn Laufrichtung von links nach rechts
                x = x + w;
                w = roi_middel_splash_right.width + roi_right_liquid_sector.width + roi_right_splash.width;
                LiquidSectorXpos = roi_middel_splash_right.width;                                                 // ROI coordinate
                StartMeasuringLiqiudAmount(CameraImage, x, y, w, h, RIGHT_TRIGGER_SIDE_INDEX, LiquidSectorXpos);  // Daten Zweiter Befüllvorgang, wenn Laufrichtung von links nach rechts
                break;
            case USE_ONLY_LEFT_VALVE:
                x = roi_left_splash.x;
                y = roi_left_splash.y;
                w = roi_left_splash.width + roi_left_liquid_sector.width + roi_middel_splash.width;
                h = roi_left_splash.height;
                LiquidSectorXpos = roi_left_splash.width;  // ROI coordinate
                StartMeasuringLiqiudAmount(CameraImage, x, y, w, h, LEFT_TRIGGER_SIDE_INDEX, LiquidSectorXpos);
                break;
            case USE_ONLY_RIGHT_VALVE:
                x = roi_left_splash.x + roi_left_splash.width + roi_left_liquid_sector.width;
                y = roi_left_splash.y;
                w = roi_middel_splash.width + roi_right_liquid_sector.width + roi_right_splash.width;
                h = roi_left_splash.height;
                LiquidSectorXpos = roi_middel_splash.width;  // ROI coordinate
                StartMeasuringLiqiudAmount(CameraImage, x, y, w, h, RIGHT_TRIGGER_SIDE_INDEX, LiquidSectorXpos);
                break;
            default:
                x = roi_left_splash.x;
                y = roi_left_splash.y;
                w = roi_left_splash.width + roi_left_liquid_sector.width + roi_middel_splash_left.width;
                h = roi_left_splash.height;
                LiquidSectorXpos = roi_left_splash.width;                                                        // ROI coordinate
                StartMeasuringLiqiudAmount(CameraImage, x, y, w, h, LEFT_TRIGGER_SIDE_INDEX, LiquidSectorXpos);  // Daten Erster Befüllvorgang, wenn Laufrichtung von links nach rechts
                x = x + w;
                w = roi_middel_splash_right.width + roi_right_liquid_sector.width + roi_right_splash.width;
                LiquidSectorXpos = roi_middel_splash_right.width;                                                 // ROI coordinate
                StartMeasuringLiqiudAmount(CameraImage, x, y, w, h, RIGHT_TRIGGER_SIDE_INDEX, LiquidSectorXpos);  // Daten Zweiter Befüllvorgang, wenn Laufrichtung von links nach rechts
                break;
        }
    } else {  // Work with one Valve
        x = roi_left_splash.x;
        y = roi_left_splash.y;
        w = roi_left_splash.width + roi_left_liquid_sector.width + roi_middel_splash.width + roi_right_liquid_sector.width + roi_right_splash.width;
        h = roi_left_splash.height;
        LiquidSectorXpos = roi_left_splash.width;  // ROI coordinate
        StartMeasuringLiqiudAmount(CameraImage, x, y, w, h, LEFT_TRIGGER_SIDE_INDEX, LiquidSectorXpos);
    }
}

void ImageData::GetAllLiquidRects(std::vector<cv::Rect>& liquidRois)
{
    cv::Rect roi_left_splash;
    cv::Rect roi_left_liquid_sector;
    cv::Rect roi_middel_splash;
    cv::Rect roi_middel_splash_left;
    cv::Rect roi_middel_splash_right;
    cv::Rect roi_right_liquid_sector;
    cv::Rect roi_right_splash;
    int x = m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowOffsetXCopy[ROI_INDEX_MEASURE_LIQUID];  // x pos(top left) blaues Messfenster
    int y = m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowOffsetYCopy[ROI_INDEX_MEASURE_LIQUID];  // y pos(top left) blaues Messfenster
    int w = m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowWidthCopy[ROI_INDEX_MEASURE_LIQUID];    // Gesamtbreite blaues Messfenster
    int h = m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowHeightCopy[ROI_INDEX_MEASURE_LIQUID];   // Höhe blaues Messfenster
    int w2 = static_cast<int>(double(w) / 2.0);                                                           // Halbe Messfensterbreite blaues Messfenster
    int LiquidSectorWidthInPixel = static_cast<int>(m_ExchangeMemory->m_MeasuringParameter.m_InjectonWindowMiddleWidthInMm / m_ExchangeMemory->m_MeasuringParameter.m_PixelSizeInMMPerPixel);
    int ValveDistanceHalf = GetDistanceBetweenValvesHalf();
    int LiquidSectorXpos;
    int splashMiddleWidth2 = ValveDistanceHalf - LiquidSectorWidthInPixel / 2.0;
    //int newWidth;

    if ((ValveDistanceHalf * 2 + LiquidSectorWidthInPixel) > w) {
        int diff = (ValveDistanceHalf * 2 + LiquidSectorWidthInPixel) - w;
        LiquidSectorWidthInPixel = LiquidSectorWidthInPixel - diff;
        if (LiquidSectorWidthInPixel < 2) {
            LiquidSectorWidthInPixel = 2;
            diff = (ValveDistanceHalf * 2 + LiquidSectorWidthInPixel) - w;
            ValveDistanceHalf = ValveDistanceHalf - diff;
            splashMiddleWidth2 = ValveDistanceHalf - LiquidSectorWidthInPixel / 2.0;
        }
    }

    if (splashMiddleWidth2 < 3) {
        splashMiddleWidth2 = 3;
    }
    //newWidth = w2 + width_offset;
    roi_left_splash.x = x;
    roi_left_splash.y = y;
    
    roi_left_splash.width = w2 - ValveDistanceHalf - LiquidSectorWidthInPixel / 2.0;
    if (roi_left_splash.width < 3) {
        roi_left_splash.width = 3;
    }
    roi_left_splash.height = h;
    liquidRois.push_back(roi_left_splash);

    roi_left_liquid_sector.x = roi_left_splash.x + roi_left_splash.width;
    roi_left_liquid_sector.y = roi_left_splash.y;
    roi_left_liquid_sector.width = LiquidSectorWidthInPixel;
    roi_left_liquid_sector.height = h;
    liquidRois.push_back(roi_left_liquid_sector);

    roi_middel_splash.x = roi_left_liquid_sector.x + roi_left_liquid_sector.width;
    roi_middel_splash.y = roi_left_splash.y;
    roi_middel_splash.width = splashMiddleWidth2 * 2;
    roi_middel_splash.height = h;
    liquidRois.push_back(roi_middel_splash);

    roi_middel_splash_left.x = roi_middel_splash.x;
    roi_middel_splash_left.y = roi_middel_splash.y;
    roi_middel_splash_left.width = splashMiddleWidth2;
    roi_middel_splash_left.height = h;
    liquidRois.push_back(roi_middel_splash_left);

    roi_middel_splash_right.x = roi_middel_splash.x + splashMiddleWidth2;
    roi_middel_splash_right.y = roi_middel_splash.y;
    roi_middel_splash_right.width = splashMiddleWidth2;
    roi_middel_splash_right.height = h;
    liquidRois.push_back(roi_middel_splash_right);

    roi_right_liquid_sector.x = roi_middel_splash.x + roi_middel_splash.width;
    roi_right_liquid_sector.y = roi_left_splash.y;
    roi_right_liquid_sector.width = LiquidSectorWidthInPixel;
    roi_right_liquid_sector.height = h;
    liquidRois.push_back(roi_right_liquid_sector);

    roi_right_splash.x = roi_right_liquid_sector.x + roi_right_liquid_sector.width;
    roi_right_splash.y = roi_left_splash.y;
    roi_right_splash.width = roi_left_splash.x + w - roi_right_splash.x;
    roi_right_splash.height = h;
    liquidRois.push_back(roi_right_splash);
}

bool ImageData::IntegrateLiquidData(unsigned long long TimeStampCPU, std::string& StringOutput, bool LeftTriggerIsSet, bool RightTriggerIsSet)
{
    bool BottleEject = true;
    if (m_ExchangeMemory->m_MeasuringParameter.m_WorkWithTwoValves) {
        switch (m_ExchangeMemory->m_MeasuringParameter.m_UsedTriggerOutputs) {
            case USE_BOTH_VALVES:
                BottleEject = IntegrateLiquidDataBothTrigger(TimeStampCPU, StringOutput, LeftTriggerIsSet, RightTriggerIsSet);
                break;
            case USE_ONLY_LEFT_VALVE:
                BottleEject = StartIntegrateLiquidData(TimeStampCPU, StringOutput, LeftTriggerIsSet, LEFT_TRIGGER_SIDE_INDEX);
                break;
            case USE_ONLY_RIGHT_VALVE:
                BottleEject = StartIntegrateLiquidData(TimeStampCPU, StringOutput, RightTriggerIsSet, RIGHT_TRIGGER_SIDE_INDEX);
                break;
            default:
                BottleEject = IntegrateLiquidDataBothTrigger(TimeStampCPU, StringOutput, LeftTriggerIsSet, RightTriggerIsSet);
        }
    } else {  // Work with one Valve
        BottleEject = StartIntegrateLiquidData(TimeStampCPU, StringOutput, LeftTriggerIsSet, LEFT_TRIGGER_SIDE_INDEX);
    }
    return BottleEject;
}

bool ImageData::IntegrateLiquidDataBothTrigger(unsigned long long TimeStampCPU, std::string& StringOutput, bool LeftTriggerIsSet, bool RightTriggerIsSet)
{
    bool BottleEject = true;
    if (TheLeftValveIsFilledFirst()) {
        StartIntegrateLiquidDataFirstTrigger(TimeStampCPU, StringOutput, LeftTriggerIsSet, LEFT_TRIGGER_SIDE_INDEX);
        BottleEject = StartIntegrateLiquidDataSecondTrigger(TimeStampCPU, StringOutput, RightTriggerIsSet, RIGHT_TRIGGER_SIDE_INDEX);
    } else {
        StartIntegrateLiquidDataFirstTrigger(TimeStampCPU, StringOutput, RightTriggerIsSet, RIGHT_TRIGGER_SIDE_INDEX);
        BottleEject = StartIntegrateLiquidDataSecondTrigger(TimeStampCPU, StringOutput, LeftTriggerIsSet, LEFT_TRIGGER_SIDE_INDEX);
    }
    return BottleEject;
}

void ImageData::AveragedMeasuringResults()
{
    double AverageSpeed, AveragePixelSize, AverageNeckDiameterInmm;

    m_AveragedMeasuringResults.ClearResults();

    m_AverageProductData.AverageSpeed(AverageSpeed);
    m_AverageProductData.AveragePixelSize(AveragePixelSize);
    m_AverageProductData.AverageBottleNeckDiameterInmm(AverageNeckDiameterInmm);

    m_AveragedMeasuringResults = m_AverageProductData.AverageProductData();

    m_CurrentMeasuringResultsSpeed.m_CurrentSpeedInmmPerms = m_AveragedMeasuringResults.m_CurrentSpeedInmmPerms = AverageSpeed;
    m_CurrentMeasuringResultsSpeed.m_CalculatePixelSizeInMMPerPixel = m_AveragedMeasuringResults.m_CalculatePixelSizeInMMPerPixel = AveragePixelSize;
    m_AveragedMeasuringResults.m_MeasuredBottleNeckDiameterInmm = AverageNeckDiameterInmm;
    m_AveragedMeasuringResults.m_LastTimeStampInns = m_CurrentMeasuringResultsSpeed.m_LastTimeStampInns;
    m_AveragedMeasuringResults.m_CurrentTimeStampInns = m_CurrentMeasuringResultsSpeed.m_CurrentTimeStampInns;
    // Daten in den Shared memory für die Anzeige
    m_ExchangeMemory->m_MeasuringParameter.m_AveragedMeasuringResults = m_AveragedMeasuringResults;  // copy into shared memory for GUI Application show speed, pixelsize
    m_ExchangeMemory->m_MeasuringParameter.m_AveragedMeasuringResults.m_StatusResults = true;        // indicator for GUI new result set
}

int ImageData::SetVideoAndLiveImage(cv::Mat& CameraImage, bool ProductFound, bool ProductOutofROI, bool LeftTriggerIsSet, bool RightTriggerIsSet, bool FirstOccurrence, bool BottleEjected,
                                    bool LiquiMeasuringReady)
{
    int rv = EVENT_NEW_IMAGE_NOT_AVAILABLE;
    std::string DrawResults, ErrorMsg;
    int MeasureWindowIndex = ROI_INDEX_MEASURE_SPEED;
    unsigned char* pSharedMemoryImageData = NULL;

    if (!m_ExchangeMemory->m_MeasuringParameter.m_ReadVideoFromRealTimeContext) {
        AddNewVideoFrame(CameraImage, FirstOccurrence, ProductFound, LeftTriggerIsSet, RightTriggerIsSet, BottleEjected, LiquiMeasuringReady);
    }
    if (m_ExchangeMemory->m_MeasuringParameter.m_TriggerGetNewVideoFromRealTimeContext) {
        m_ExchangeMemory->m_MeasuringParameter.m_ReadVideoFromRealTimeContext = true;
        m_ExchangeMemory->m_MeasuringParameter.m_TriggerGetNewVideoFromRealTimeContext = false;
        KS_setEvent((KSHandle)(m_ExchangeMemory)->m_HandleEventCanCopyVideoData);
    }
    // here set image to windows application (show the image)
    if (((m_CurrentMeasuringResultsSpeed.m_CurrentTimeStampInns - m_LastTimeStampInNanoSec) > m_TimeIntervalLiveImageInNanoSec) || LiquiMeasuringReady) {
        // liveimage view all 100ms oder wenn Füllmenge berechnet und entscheidung ob Flasche ausgeworfen wird oder nicht
        m_LastTimeStampInNanoSec = m_CurrentMeasuringResultsSpeed.m_CurrentTimeStampInns;
        pSharedMemoryImageData = m_SharedMemoryImageBlock;  // pointer first pixel byte in shared memory
        memcpy(pSharedMemoryImageData, (unsigned char*)(CameraImage.data), CameraImage.rows * CameraImage.cols);
        m_ExchangeMemory->m_MeasuringParameter.m_CurrentMeasuringResults = m_CurrentMeasuringResultsSpeed;  // Aktuelle Messergebnisse in den Shared Memory
        KS_setEvent((KSHandle)(m_ExchangeMemory)->m_HandleImageReceivedEvent);                              // info for GUI Application new image in shared memory
    }
    // Alle 10 Sekunden das aktuelle Bild zur Überprüfing des Verschmutzungsgrades der Hintergrundbeleuchtung
    if ((m_CurrentMeasuringResultsSpeed.m_CurrentTimeStampInns - m_LastTimeStampCleanImageInNanoSec) >
        m_TimeIntervalCleanImageInNanoSec)  // && !m_ExchangeMemory->m_MeasuringParameter.m_TriggerGetNewVideoFromRealTimeContext)//zeige alle 100ms ein bild an
    {                                       // all 10 seconds
        m_LastTimeStampCleanImageInNanoSec = m_CurrentMeasuringResultsSpeed.m_CurrentTimeStampInns;
        CopyCleanImage();
    }
    return rv;
}

void ImageData::CheckResetCounters()
{
    // wird gesetzt wenn der Button Reset All Counters gedrückt wird
    if (m_ExchangeMemory->m_MeasuringParameter.m_ResetAllCounters) {
        m_CurrentMeasuringResultsLiqiud.m_CounterBottleFilled = 0;
        m_CurrentMeasuringResultsLiqiud.m_CounterMiddleTooLittle = 0;
        m_CurrentMeasuringResultsLiqiud.m_CounterMiddleTooMuch = 0;
        m_CurrentMeasuringResultsLiqiud.m_CounterMiddleNotOk = 0;
        m_CurrentMeasuringResultsLiqiud.m_CounterLeftTooBig = 0;
        m_CurrentMeasuringResultsLiqiud.m_CounterRightTooBig = 0;
        m_CurrentMeasuringResultsLiqiud.m_CounterBottleNotUnderValve = 0;
        m_CurrentMeasuringResultsLiqiud.m_CounterBottleNotFilled = 0;

        m_CurrentMeasuringResultsSpeed.m_CounterContrastOutOfTol = 0;
        m_CurrentMeasuringResultsSpeed.m_CounterEdgeIsLongTimeOutOfTol = 0;
        m_CurrentMeasuringResultsSpeed.m_CounterSizeIsLongTimeOutOfTol = 0;

        m_ExchangeMemory->m_MeasuringParameter.m_CurrentMeasuringResultsLiqiud.m_CounterBottleFilled = 0;
        m_ExchangeMemory->m_MeasuringParameter.m_CurrentMeasuringResultsLiqiud.m_CounterMiddleTooLittle = 0;
        m_ExchangeMemory->m_MeasuringParameter.m_CurrentMeasuringResultsLiqiud.m_CounterMiddleTooMuch = 0;
        m_ExchangeMemory->m_MeasuringParameter.m_CurrentMeasuringResultsLiqiud.m_CounterMiddleNotOk = 0;
        m_ExchangeMemory->m_MeasuringParameter.m_CurrentMeasuringResultsLiqiud.m_CounterLeftTooBig = 0;
        m_ExchangeMemory->m_MeasuringParameter.m_CurrentMeasuringResultsLiqiud.m_CounterRightTooBig = 0;
        m_ExchangeMemory->m_MeasuringParameter.m_CurrentMeasuringResultsLiqiud.m_CounterBottleNotUnderValve = 0;
        m_ExchangeMemory->m_MeasuringParameter.m_CurrentMeasuringResultsLiqiud.m_CounterBottleNotFilled = 0;
        m_ExchangeMemory->m_MeasuringParameter.m_ResetAllCounters = false;
        m_ExchangeMemory->m_CounterNumberBottlesRealEjected = 0;
    }
    // wird gesetzt wenn der Fehler zurückgesetzt wird
    if (m_ExchangeMemory->m_MeasuringParameter.m_ResetCountersBottlesEjectionAndLiquidTooLow) {
        m_ExchangeMemory->m_MeasuringParameter.m_CurrentMeasuringResultsLiqiud.m_CounterBottleNotFilledOneAfterTheOther = 0;
        m_ExchangeMemory->m_MeasuringParameter.m_CurrentMeasuringResultsLiqiud.m_CounterMiddleNotOkOneAfterTheOther = 0;
        m_CurrentMeasuringResultsLiqiud.m_CounterBottleNotFilledOneAfterTheOther = 0;
        m_CurrentMeasuringResultsLiqiud.m_CounterMiddleNotOkOneAfterTheOther = 0;
        m_ExchangeMemory->m_MeasuringParameter.m_ResetCountersBottlesEjectionAndLiquidTooLow = false;
    }
}

bool ImageData::CheckIsLeftTriggerSet()
{
    if (m_LeftTriggerIsSet) {
        m_LeftTriggerIsSet = false;
        return true;
    } else
        return false;
}

bool ImageData::CheckIsRightTriggerSet()
{
    if (m_RightTriggerIsSet) {
        m_RightTriggerIsSet = false;
        return true;
    } else
        return false;
}
//       side==LEFT_TRIGGER_SIDE_INDEX       side==RIGHT_TRIGGER_SIDE_INDEX
//                    v1 <-Valvedistance-> v2
// x,y _____________________________________________________
//     |            |     |Right  Left  |      |            |
//     |            |     |ROI    ROI   |      |            |
//     |  Left ROI  |Mid. |Left   Right |Mid.  | Right ROI  |   h
//     |  Left Side |Left |Side   Side  |Right | Right Side |
//     |____________|_____|_____________|______|____________|
//
//
//     |________________________|
//         Linke Seite/Ventil
//
//                              |___________________________|
//                                     Rechte Seite/Ventil
void ImageData::SetMeasureResultsLiquid(MeasuringResultsLiquid& CurrentMeasuringResultsLiqiud, int side, std::string& Output)
{
    if (m_AmountSplashLeftROI[side] > CurrentMeasuringResultsLiqiud.m_MaxAmountSplashLeftROI) {
        CurrentMeasuringResultsLiqiud.m_MaxAmountSplashLeftROI = m_AmountSplashLeftROI[side];
    }
    if (m_AmountSplashRightROI[side] > CurrentMeasuringResultsLiqiud.m_MaxAmountSplashRightROI) {
        CurrentMeasuringResultsLiqiud.m_MaxAmountSplashRightROI = m_AmountSplashRightROI[side];
    }
    CurrentMeasuringResultsLiqiud.m_SumAmountMiddle = CurrentMeasuringResultsLiqiud.m_SumAmountMiddle + m_AmountLiquidMiddleROI[side];

    if (m_AmountSplashLeftROI[side] > CurrentMeasuringResultsLiqiud.m_MaxAmountSplashLeftROIPerSide[side]) {
        // Umfang der Spritzer auf der linken Hälfte für jede Seite
        CurrentMeasuringResultsLiqiud.m_MaxAmountSplashLeftROIPerSide[side] = m_AmountSplashLeftROI[side];
    }
    if (m_AmountSplashRightROI[side] > CurrentMeasuringResultsLiqiud.m_MaxAmountSplashRightROIPerSide[side]) {
        // Umfang der Spritzer auf der rechte Hälfte für jede Seite
        CurrentMeasuringResultsLiqiud.m_MaxAmountSplashRightROIPerSide[side] = m_AmountSplashRightROI[side];
    }
    if (!CurrentMeasuringResultsLiqiud.m_BottleIsUnderValve) {  // Flasche ist noch nicht mittig unter dem Ventil gewesen
        std::string sideString;

        if (side == LEFT_TRIGGER_SIDE_INDEX) {
            sideString = "Left Side";
        } else {
            sideString = "Right Side";
        }
        if (!m_ExchangeMemory->m_MeasuringParameter.m_DeaktivateCheckBottleUnderValve) {
            if (m_CurrentMeasuringResultsTriggerPos[side].ProductFound()) {
                CurrentMeasuringResultsLiqiud.m_BottleIsUnderValve = true;
                Output = Output + cv::format("Bottle Under Valve, %s. Score:%.2f%%\n", sideString, m_CurrentMeasuringResultsTriggerPos[side].m_BottleMatchScoreInPercent);
            }
        } else {
            CurrentMeasuringResultsLiqiud.m_BottleIsUnderValve = true;
        }
    }
}

void ImageData::SetInjectionTimes(unsigned long long TimeStampInns, MeasuringResultsLiquid& CurrentMeasuringResultsLiqiud)
{
    double InjectionLenghtInMM = GetInjectionLenghtInMM();  // Flaschendurchmessergeteilt durch Wurzel von 2
    CurrentMeasuringResultsLiqiud.ClearResults();
    CurrentMeasuringResultsLiqiud.m_InjectionTime = static_cast<unsigned long long>((InjectionLenghtInMM / GetAverageSpeedInMMPerMs()) + 0.5) * 1000000;
    CurrentMeasuringResultsLiqiud.m_InjectionTimeHalf = static_cast<unsigned long long>((InjectionLenghtInMM / (GetAverageSpeedInMMPerMs() * 2.0)) + 0.5) * 1000000;
    CurrentMeasuringResultsLiqiud.m_InjectionTimeFinished = TimeStampInns + CurrentMeasuringResultsLiqiud.m_InjectionTime;
}

// wird nur aufgerufen wenn beide Trigger aktiv sind
void ImageData::StartIntegrateLiquidDataFirstTrigger(unsigned long long TimeStampInns, std::string& Output, bool IsTriggerSet, int side)
{
    std::string& SideName = cv::format("Right");
    if (side == LEFT_TRIGGER_SIDE_INDEX) {
        SideName = cv::format("Left");
    }
    if (IsTriggerSet) {
        if (GetAverageSpeedInMMPerMs() > m_ExchangeMemory->m_MeasuringParameter.m_MinSpeedInMMPerMs) {
            if (m_CurrentMeasuringResultsLiqiudFirstTrigger.m_InjectionTimeFinished > 0) {  // Ein neuer Trigger wurde ausgelöst obwohl der vorherige Befüllvorgang noch nicht beendet, tritt immer dann
                                                                                            // auf wenn die Flaschen so dicht zusammen stehen das sie sich sich berühren.
                m_StackFirstTriggerLiquidData.push_back(m_CurrentMeasuringResultsLiqiudFirstTrigger);  // Hier die Annahme Flasche ist schon befüllt
                m_TriggerCounterFirst++;
                Output = Output + cv::format("%s Filling Ready\n", SideName.c_str());
            }
            SetInjectionTimes(TimeStampInns, m_CurrentMeasuringResultsLiqiudFirstTrigger);
            CalculateEjectionPointInTime(TimeStampInns);
            Output = Output + cv::format("%s Trigger Is Set! Injection Time:%0.3fms\n", SideName.c_str(), m_CurrentMeasuringResultsLiqiudFirstTrigger.m_InjectionTime / 1000000.0);
        }
        m_CounterImagesNoFirstTriggerOccur = 0;
    }
    if (m_CurrentMeasuringResultsLiqiudFirstTrigger.m_InjectionTimeFinished >= TimeStampInns) {
        // die nächsten bilder die ab jetzt kommen dienen zur Berechnung der Injektionsmenge, der Spritzer im linken Messfenster und Pruefung ob Flasche unter dem Ventil
        SetMeasureResultsLiquid(m_CurrentMeasuringResultsLiqiudFirstTrigger, side, Output);
        Output = Output + cv::format("%s Liquid SplashL:%d Middle:%d SplashR:%d\n", SideName.c_str(), m_CurrentMeasuringResultsLiqiudFirstTrigger.m_MaxAmountSplashLeftROI,
                                     m_CurrentMeasuringResultsLiqiudFirstTrigger.m_SumAmountMiddle, m_CurrentMeasuringResultsLiqiudFirstTrigger.m_MaxAmountSplashRightROI);
    } else {
        if (m_CurrentMeasuringResultsLiqiudFirstTrigger.m_InjectionTimeFinished > 0)  // InjectionTimeFinished ist nur dann größer null, wenn vorher ein Trigger ausgelöst wurde
        {                                                                             // Flasche ist am ersten Ventil vorbei, erster Befüllvorgang ist beendet, Sichern der Ergebnisse
            if (m_StackFirstTriggerLiquidData.size() > 2) {
                // Tritt eigentlich nie auf nur dann wenn der zweite Trigger nicht auslöst, hier nur eine Sicherheitsmaßnahme um zu verhindern das der Stack überläuft
                // der zweite Trigger löst nicht aus. Dann Stack löschen
                while (!m_StackFirstTriggerLiquidData.empty())  // Sicherheitsmaßnahme, Überlauf muss verhindert werden
                    m_StackFirstTriggerLiquidData.pop_front();
            }
            m_StackFirstTriggerLiquidData.push_back(m_CurrentMeasuringResultsLiqiudFirstTrigger);
            Output = Output + cv::format("%s Filling Ready\n", SideName.c_str());
            m_CurrentMeasuringResultsLiqiudFirstTrigger.m_InjectionTimeFinished = 0;
            m_TriggerCounterFirst++;
        } else {
            m_CounterImagesNoFirstTriggerOccur++;           // der Zähler zählt seit wann kein Trigger mehr ausgelöst hat
            if (m_CounterImagesNoFirstTriggerOccur > 1000)  // ist ca. 1 sec, da alle 1.12ms ein neuse Bild kommt
            {  // der Stack müsste eigentlich in diesem Zustand leer sein, wenn seit 1 sec kein neuer Trigger gekommen ist, Sicherheitshalber nochmal prüfen
                while (!m_StackFirstTriggerLiquidData.empty())
                    m_StackFirstTriggerLiquidData.pop_front();  // Dieser Zustand wird wahrscheinlich niemals erreicht, nur dann wenn der zweite Trigger nicht auslöst
                m_CounterImagesNoFirstTriggerOccur = 0;
            }
        }
    }
}

// wird nur aufgerufen wenn beide Trigger aktiv sind
bool ImageData::StartIntegrateLiquidDataSecondTrigger(unsigned long long TimeStampInns, std::string& Output, bool IsTriggerSet, int side)
{
    bool EjectBottle = true;
    std::string& SideName = cv::format("Right");  // Auf welcher Seite steht der zweite Trigger
    if (side == LEFT_TRIGGER_SIDE_INDEX) {        // Auf welcher Seite steht der zweite Trigger
        SideName = cv::format("Left");
    }
    if (IsTriggerSet) {
        if (GetAverageSpeedInMMPerMs() > m_ExchangeMemory->m_MeasuringParameter.m_MinSpeedInMMPerMs) {
            if (m_CurrentMeasuringResultsLiqiudSecondTrigger.m_InjectionTimeFinished > 0) {  // Ein neuer Trigger wurde ausgelöst obwohl der vorherige Befüllvorgang noch nicht beendet, tritt immer
                                                                                             // dann auf wenn die Flaschen so dicht zusammen stehen das sie sich sich berühren.
                EjectBottle = SumUpFirstAndSecondLiquidData(Output);
                m_TriggerCounterSecond++;
            }
            SetInjectionTimes(TimeStampInns, m_CurrentMeasuringResultsLiqiudSecondTrigger);
            Output = Output + cv::format("%s Trigger Is Set! Injection Time:%0.3fms\n", SideName.c_str(), m_CurrentMeasuringResultsLiqiudSecondTrigger.m_InjectionTime / 1000000.0);
        }
    }
    if (m_CurrentMeasuringResultsLiqiudSecondTrigger.m_InjectionTimeFinished >= TimeStampInns) {
        // die nächsten Bilder die ab jetzt kommen dienen zur Berechnung der Injektionsmenge, der Spritzer im linken Messfenster und Pruefung ob Flasche unter dem Ventil
        SetMeasureResultsLiquid(m_CurrentMeasuringResultsLiqiudSecondTrigger, side, Output);
        if (!IsTriggerSet)
            Output = Output + cv::format("%s Liqiud SplashL:%d Middle:%d SplashR:%d\n", SideName.c_str(), m_CurrentMeasuringResultsLiqiudSecondTrigger.m_MaxAmountSplashLeftROI,
                                         m_CurrentMeasuringResultsLiqiudSecondTrigger.m_SumAmountMiddle, m_CurrentMeasuringResultsLiqiudSecondTrigger.m_MaxAmountSplashRightROI);
    } else {  // Flasche ist am zweiten Ventilen vorbei, Befüllvorgang ist beendet, sichern der Ergebnisse und Überprüfung Freigabe Flasche
        if (m_CurrentMeasuringResultsLiqiudSecondTrigger.m_InjectionTimeFinished > 0) {
            // zu diesem Zeitpunkt ist der erste Trigger schon durchgelaufen und die Ergebnisse aus dem ersten Durchlauf sind schon unter m_StackFirstTriggerLiquidData gespeichert
            EjectBottle = SumUpFirstAndSecondLiquidData(Output);  // Zusammenfassen der Ergebinissen aus dem ersten Befüllvorgang und dem zweiten Befüllvorgang
            m_CurrentMeasuringResultsLiqiudSecondTrigger.m_InjectionTimeFinished = 0;
            m_TriggerCounterSecond++;
        }
    }
    if (m_ExchangeMemory->m_MeasuringParameter.m_InfoLevel == 8)
        Output = Output + cv::format("TriggerCountLeft:%d  TriggerCountRight:%d\n", m_TriggerCounterFirst, m_TriggerCounterSecond);  // Dise Werte müssen immer gleich groß sein!!!!!!!!
    return EjectBottle;
}

bool ImageData::TheLeftValveIsFilledFirst()
{
    if (m_ExchangeMemory->m_MeasuringParameter.m_BandDirectional == BAND_DIRECTIONAL_LEFT_TO_RIGHT)
        return true;
    else
        return false;
}

bool ImageData::SumUpFirstAndSecondLiquidData(std::string& Output)
{
    bool EjectBottle = true;

    if (m_StackFirstTriggerLiquidData.size() > 0) {  // Zusammenfassen der Ergebnisse aus dem ersten und zweiten Befüllvorgang
        MeasuringResultsLiquid CurrentMeasuringResultsLiqiudFirstTrigger = m_StackFirstTriggerLiquidData.front();
        m_StackFirstTriggerLiquidData.pop_front();
        m_CurrentMeasuringResultsLiqiud.m_SumAmountLiquidFirstTrigger = CurrentMeasuringResultsLiqiudFirstTrigger.m_SumAmountMiddle;
        m_CurrentMeasuringResultsLiqiud.m_SumAmountLiquidSecondTrigger = m_CurrentMeasuringResultsLiqiudSecondTrigger.m_SumAmountMiddle;
        m_CurrentMeasuringResultsLiqiud.m_SumAmountMiddle = CurrentMeasuringResultsLiqiudFirstTrigger.m_SumAmountMiddle + m_CurrentMeasuringResultsLiqiudSecondTrigger.m_SumAmountMiddle;

        // Wird die Triggerposition so ungünstig gesetzt und das blaus Messfenster leigt am Rand des Bildes kann es vorkommen dass die
        // Position der Flasche unter dem zweiten Ventil(wenn die Flasche einen großen Durchmesser hat < 35mm) nicht erkannt wird, daher sollte es ausreichend sein, wenn die Flasche einmal im gelben
        // Messfenster gefunden wurde
        if (CurrentMeasuringResultsLiqiudFirstTrigger.m_BottleIsUnderValve || m_CurrentMeasuringResultsLiqiudSecondTrigger.m_BottleIsUnderValve)
            // if (CurrentMeasuringResultsLiqiudFirstTrigger.m_BottleIsUnderValve && m_CurrentMeasuringResultsLiqiudSecondTrigger.m_BottleIsUnderValve)
            m_CurrentMeasuringResultsLiqiud.m_BottleIsUnderValve = true;
        else
            m_CurrentMeasuringResultsLiqiud.m_BottleIsUnderValve = false;

        if (m_CurrentMeasuringResultsLiqiudSecondTrigger.m_MaxAmountSplashRightROI > CurrentMeasuringResultsLiqiudFirstTrigger.m_MaxAmountSplashRightROI)
            m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashRightROI = m_CurrentMeasuringResultsLiqiudSecondTrigger.m_MaxAmountSplashRightROI;
        else
            m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashRightROI = CurrentMeasuringResultsLiqiudFirstTrigger.m_MaxAmountSplashRightROI;

        if (m_CurrentMeasuringResultsLiqiudSecondTrigger.m_MaxAmountSplashLeftROI > CurrentMeasuringResultsLiqiudFirstTrigger.m_MaxAmountSplashLeftROI)
            m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashLeftROI = m_CurrentMeasuringResultsLiqiudSecondTrigger.m_MaxAmountSplashLeftROI;
        else
            m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashLeftROI = CurrentMeasuringResultsLiqiudFirstTrigger.m_MaxAmountSplashLeftROI;

        if (TheLeftValveIsFilledFirst()) {
            m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashLeftROIPerSide[RIGHT_TRIGGER_SIDE_INDEX] =
                m_CurrentMeasuringResultsLiqiudSecondTrigger.m_MaxAmountSplashLeftROIPerSide[RIGHT_TRIGGER_SIDE_INDEX];
            m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashRightROIPerSide[RIGHT_TRIGGER_SIDE_INDEX] =
                m_CurrentMeasuringResultsLiqiudSecondTrigger.m_MaxAmountSplashRightROIPerSide[RIGHT_TRIGGER_SIDE_INDEX];
            m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashLeftROIPerSide[LEFT_TRIGGER_SIDE_INDEX] =
                CurrentMeasuringResultsLiqiudFirstTrigger.m_MaxAmountSplashLeftROIPerSide[LEFT_TRIGGER_SIDE_INDEX];
            m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashRightROIPerSide[LEFT_TRIGGER_SIDE_INDEX] =
                CurrentMeasuringResultsLiqiudFirstTrigger.m_MaxAmountSplashRightROIPerSide[LEFT_TRIGGER_SIDE_INDEX];
        } else {
            // First Trigger Is On Right Side
            m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashLeftROIPerSide[RIGHT_TRIGGER_SIDE_INDEX] =
                CurrentMeasuringResultsLiqiudFirstTrigger.m_MaxAmountSplashLeftROIPerSide[RIGHT_TRIGGER_SIDE_INDEX];
            m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashRightROIPerSide[RIGHT_TRIGGER_SIDE_INDEX] =
                CurrentMeasuringResultsLiqiudFirstTrigger.m_MaxAmountSplashRightROIPerSide[RIGHT_TRIGGER_SIDE_INDEX];
            m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashLeftROIPerSide[LEFT_TRIGGER_SIDE_INDEX] =
                m_CurrentMeasuringResultsLiqiudSecondTrigger.m_MaxAmountSplashLeftROIPerSide[LEFT_TRIGGER_SIDE_INDEX];
            m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashRightROIPerSide[LEFT_TRIGGER_SIDE_INDEX] =
                m_CurrentMeasuringResultsLiqiudSecondTrigger.m_MaxAmountSplashRightROIPerSide[LEFT_TRIGGER_SIDE_INDEX];
        }

    } else {
        m_CurrentMeasuringResultsLiqiud.m_SumAmountMiddle = m_CurrentMeasuringResultsLiqiudSecondTrigger.m_SumAmountMiddle;
        m_CurrentMeasuringResultsLiqiud.m_BottleIsUnderValve = m_CurrentMeasuringResultsLiqiudSecondTrigger.m_BottleIsUnderValve;
        m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashRightROI = m_CurrentMeasuringResultsLiqiudSecondTrigger.m_MaxAmountSplashRightROI;
        m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashLeftROI = m_CurrentMeasuringResultsLiqiudSecondTrigger.m_MaxAmountSplashLeftROI;
        Output = Output + cv::format("No First Trigger\n");
    }
    Output = Output + cv::format("Left And Right Filling Ready. Stack Size First:%d\n", m_StackFirstTriggerLiquidData.size());
    EjectBottle = CheckResultsLiquidMeasuringAndApplyEjection(Output);
    return EjectBottle;
}

// Wird nur aufgerufen wenn nur ein Triggerventil angeschlossen oder nur ein Ventil aktiv
bool ImageData::StartIntegrateLiquidData(unsigned long long TimeStampInns, std::string& Output, bool IsTriggerSet, int side)
{
    bool EjectBottle = true;
    if (IsTriggerSet) {
        if (GetAverageSpeedInMMPerMs() > m_ExchangeMemory->m_MeasuringParameter.m_MinSpeedInMMPerMs) {
            SetInjectionTimes(TimeStampInns, m_CurrentMeasuringResultsLiqiud);
            CalculateEjectionPointInTime(TimeStampInns);
            Output = Output + cv::format("Trigger Is Set! Injection Time:%0.3fms\n", m_CurrentMeasuringResultsLiqiud.m_InjectionTime / 1000000.0);
        }
    }
    if (m_CurrentMeasuringResultsLiqiud.m_InjectionTimeFinished >= TimeStampInns) {
        // die nächsten bilder die ab jetzt kommen dienen zur Berechnung der Injektionsmenge und pruefung ob Flasche unter dem Ventil
        SetMeasureResultsLiquid(m_CurrentMeasuringResultsLiqiud, side, Output);
        if (!IsTriggerSet)
            Output = Output + cv::format("Liqiud L:%d M:%d R:%d\n", m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashLeftROI, m_CurrentMeasuringResultsLiqiud.m_SumAmountMiddle,
                                         m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashRightROI);
    } else {  // Flasche ist am Ventilen vorbei, Auswertung der Messergebnissen
        if (m_CurrentMeasuringResultsLiqiud.m_InjectionTimeFinished > 0) {
            EjectBottle = CheckResultsLiquidMeasuringAndApplyEjection(Output);
        }
    }
    return EjectBottle;
}

bool ImageData::CheckResultsLiquidMeasuringAndApplyEjection(std::string& Output)
{
    m_CurrentMeasuringResultsLiqiud.m_AmountSplashLeftOk = true;
    m_CurrentMeasuringResultsLiqiud.m_AmountSplashRightOk = true;
    m_CurrentMeasuringResultsLiqiud.m_AmountLiquidOk = true;
    m_CurrentMeasuringResultsLiqiud.m_SumAmountLiquidFirstTriggerOk = true;
    m_CurrentMeasuringResultsLiqiud.m_SumAmountLiquidSecondTriggerOk = true;
    m_CurrentMeasuringResultsLiqiud.m_SumAmountLiquidFirstTriggerMinThresholdOk = true;
    m_CurrentMeasuringResultsLiqiud.m_SumAmountMiddleFirstTriggerMaxThresholdOk = true;
    m_CurrentMeasuringResultsLiqiud.m_SumAmountLiquidSecondTriggerMinThresholdOk = true;
    m_CurrentMeasuringResultsLiqiud.m_SumAmountMiddleSecondTriggerMaxThresholdOk = true;
    std::string first_trigger_valve_name = "Right Valve";
    std::string second_trigger_valve_name = "Left Valve";

    m_CurrentMeasuringResultsLiqiud.m_TheLeftValveIsFilledFirst = TheLeftValveIsFilledFirst();  // is used for trendgraph data
    if (m_CurrentMeasuringResultsLiqiud.m_TheLeftValveIsFilledFirst) {
        first_trigger_valve_name = "Left Valve";
        second_trigger_valve_name = "Right Valve";
    }
    if (m_ExchangeMemory->m_MeasuringParameter.m_UsedTriggerOutputs == USE_BOTH_VALVES) {
        // Prüfung Flüssigkeitsmenge erster Trigger ok oder nicht ok
        if (m_CurrentMeasuringResultsLiqiud.m_SumAmountLiquidFirstTrigger < m_ExchangeMemory->m_MeasuringParameter.m_MinAcceptanceThresholdLiquidMiddleROI) {
            m_CurrentMeasuringResultsLiqiud.m_SumAmountLiquidFirstTriggerMinThresholdOk = false;
        }
        if (m_CurrentMeasuringResultsLiqiud.m_SumAmountLiquidFirstTrigger > m_ExchangeMemory->m_MeasuringParameter.m_MaxAcceptanceThresholdLiquidMiddleROI) {
            m_CurrentMeasuringResultsLiqiud.m_SumAmountMiddleFirstTriggerMaxThresholdOk = false;
        }
        if (!m_CurrentMeasuringResultsLiqiud.m_SumAmountLiquidFirstTriggerMinThresholdOk || !m_CurrentMeasuringResultsLiqiud.m_SumAmountMiddleFirstTriggerMaxThresholdOk) {
            m_CurrentMeasuringResultsLiqiud.m_SumAmountLiquidFirstTriggerOk = false;
        }
        // Prüfung Flüssigkeitsmenge zweiter Trigger ok oder nicht ok
        if (m_CurrentMeasuringResultsLiqiud.m_SumAmountLiquidSecondTrigger < m_ExchangeMemory->m_MeasuringParameter.m_MinAcceptanceThresholdLiquidMiddleROI) {
            m_CurrentMeasuringResultsLiqiud.m_SumAmountLiquidSecondTriggerMinThresholdOk = false;
        }
        if (m_CurrentMeasuringResultsLiqiud.m_SumAmountLiquidSecondTrigger > m_ExchangeMemory->m_MeasuringParameter.m_MaxAcceptanceThresholdLiquidMiddleROI) {
            m_CurrentMeasuringResultsLiqiud.m_SumAmountMiddleSecondTriggerMaxThresholdOk = false;
        }
        if (!m_CurrentMeasuringResultsLiqiud.m_SumAmountLiquidSecondTriggerMinThresholdOk || !m_CurrentMeasuringResultsLiqiud.m_SumAmountMiddleSecondTriggerMaxThresholdOk) {
            m_CurrentMeasuringResultsLiqiud.m_SumAmountLiquidSecondTriggerOk = false;
        }

        if (!m_CurrentMeasuringResultsLiqiud.m_SumAmountLiquidFirstTriggerOk || !m_CurrentMeasuringResultsLiqiud.m_SumAmountLiquidSecondTriggerOk) {
            m_CurrentMeasuringResultsLiqiud.m_AmountLiquidOk = false;
        }
    } else {
        if (m_CurrentMeasuringResultsLiqiud.m_SumAmountMiddle < m_ExchangeMemory->m_MeasuringParameter.m_MinAcceptanceThresholdLiquidMiddleROI ||
            m_CurrentMeasuringResultsLiqiud.m_SumAmountMiddle > m_ExchangeMemory->m_MeasuringParameter.m_MaxAcceptanceThresholdLiquidMiddleROI) {
            m_CurrentMeasuringResultsLiqiud.m_AmountLiquidOk = false;
        }
    }
    // Prüfung Spritzer links oder rechts ok oder nicht ok
    if (m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashLeftROI > m_ExchangeMemory->m_MeasuringParameter.m_AcceptanceThresholdLiquidLeftAndRightROI) {
        m_CurrentMeasuringResultsLiqiud.m_AmountSplashLeftOk = false;
    }
    if (m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashRightROI > m_ExchangeMemory->m_MeasuringParameter.m_AcceptanceThresholdLiquidLeftAndRightROI) {
        m_CurrentMeasuringResultsLiqiud.m_AmountSplashRightOk = false;
    }
    bool EjectBottle = true;
    Output = Output + cv::format("Bottling Done! M:%d L:%d R:%d\n", m_CurrentMeasuringResultsLiqiud.m_SumAmountMiddle, m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashLeftROI,
                                 m_CurrentMeasuringResultsLiqiud.m_MaxAmountSplashRightROI);
    if (m_CurrentMeasuringResultsLiqiud.m_BottleIsUnderValve) {
        if (m_CurrentMeasuringResultsLiqiud.m_AmountLiquidOk && m_CurrentMeasuringResultsLiqiud.m_AmountSplashLeftOk && m_CurrentMeasuringResultsLiqiud.m_AmountSplashRightOk) {
            // if (m_ExchangeMemory->m_MaschineState == OPERATION_STATUS_RELEASE_PRODUCTION) {
            //    ApplyEjectionData();  // Freigabe, ausblasen aus, Flasche kann passieren
            //    EjectBottle = false;
            //}
            EjectBottle = CheckBottleEjectionConditions();
            m_CurrentMeasuringResultsLiqiud.m_CounterBottleNotFilledOneAfterTheOther = 0;
            m_CurrentMeasuringResultsLiqiud.m_CounterMiddleNotOkOneAfterTheOther = 0;
            m_CurrentMeasuringResultsLiqiud.m_CounterBottleFilled++;
            Output = Output + cv::format("Bottle Is Filled\n");
        } else {
            if (!m_CurrentMeasuringResultsLiqiud.m_AmountLiquidOk) {
                if (m_ExchangeMemory->m_MeasuringParameter.m_UsedTriggerOutputs == USE_BOTH_VALVES) {
                    if (!m_CurrentMeasuringResultsLiqiud.m_SumAmountLiquidFirstTriggerOk) {
                        if (!m_CurrentMeasuringResultsLiqiud.m_SumAmountLiquidFirstTriggerMinThresholdOk) {
                            Output = Output + cv::format("Filling Failed. Liquid %s Too Little!\n", first_trigger_valve_name);
                            // m_CurrentMeasuringResultsLiqiud.m_CounterMiddleTooLittle++;
                        } else {
                            Output = Output + cv::format("Filling Failed. Liquid %s Too Much!\n", first_trigger_valve_name);
                            // m_CurrentMeasuringResultsLiqiud.m_CounterMiddleTooMuch++;
                        }
                    }
                    if (!m_CurrentMeasuringResultsLiqiud.m_SumAmountLiquidSecondTriggerOk) {
                        if (!m_CurrentMeasuringResultsLiqiud.m_SumAmountLiquidSecondTriggerMinThresholdOk) {
                            Output = Output + cv::format("Filling Failed. Liquid %s Too Little!\n", second_trigger_valve_name);
                            // m_CurrentMeasuringResultsLiqiud.m_CounterMiddleTooLittle++;
                        } else {
                            Output = Output + cv::format("Filling Failed. Liquid %s Too Much!\n", second_trigger_valve_name);
                            // m_CurrentMeasuringResultsLiqiud.m_CounterMiddleTooMuch++;
                        }
                    }
                } else {
                    if (m_ExchangeMemory->m_MeasuringParameter.m_UsedTriggerOutputs == USE_ONLY_LEFT_VALVE) {
                        if (m_CurrentMeasuringResultsLiqiud.m_SumAmountMiddle < m_ExchangeMemory->m_MeasuringParameter.m_MinAcceptanceThresholdLiquidMiddleROI) {
                            Output = Output + cv::format("Filling Failed. Liquid Left Valve Too Little!\n");
                            // m_CurrentMeasuringResultsLiqiud.m_CounterMiddleTooLittle++;
                        } else {
                            Output = Output + cv::format("Filling Failed. Liquid Left Valve Too Much!\n");
                            // m_CurrentMeasuringResultsLiqiud.m_CounterMiddleTooMuch++;
                        }
                    } else {
                        if (m_CurrentMeasuringResultsLiqiud.m_SumAmountMiddle < m_ExchangeMemory->m_MeasuringParameter.m_MinAcceptanceThresholdLiquidMiddleROI) {
                            Output = Output + cv::format("Filling Failed. Liquid Right Valve Too Little!\n");
                            // m_CurrentMeasuringResultsLiqiud.m_CounterMiddleTooLittle++;
                        } else {
                            Output = Output + cv::format("Filling Failed. Liquid Right Valve Too Much!\n");
                            // m_CurrentMeasuringResultsLiqiud.m_CounterMiddleTooMuch++;
                        }
                    }
                }
                m_CurrentMeasuringResultsLiqiud.m_CounterMiddleNotOk++;
                if (m_ExchangeMemory->m_MaschineState == OPERATION_STATUS_RELEASE_PRODUCTION) {
                    m_CurrentMeasuringResultsLiqiud.m_CounterMiddleNotOkOneAfterTheOther++;
                }
            }
            if (!m_CurrentMeasuringResultsLiqiud.m_AmountSplashLeftOk) {
                Output = Output + cv::format("Filling Failed. Splash Left Too Much!\n");
                m_CurrentMeasuringResultsLiqiud.m_CounterLeftTooBig++;
            }
            if (!m_CurrentMeasuringResultsLiqiud.m_AmountSplashRightOk) {
                Output = Output + cv::format("Filling Failed. Splash Right Tool Much!\n");
                m_CurrentMeasuringResultsLiqiud.m_CounterRightTooBig++;
            }
        }
    } else {  // Die Flasche steht nicht mittig unter dem Ventil
        m_CurrentMeasuringResultsLiqiud.m_CounterBottleNotUnderValve++;
        Output = Output + cv::format("Filling Failed. Bottle Not Under Valve!\n");
    }
    if (!m_CurrentMeasuringResultsLiqiud.m_BottleIsUnderValve || !m_CurrentMeasuringResultsLiqiud.m_AmountLiquidOk || !m_CurrentMeasuringResultsLiqiud.m_AmountSplashLeftOk ||
        !m_CurrentMeasuringResultsLiqiud.m_AmountSplashRightOk) {
        m_CurrentMeasuringResultsLiqiud.m_CounterBottleNotFilled++;
        if (m_ExchangeMemory->m_MaschineState == OPERATION_STATUS_RELEASE_PRODUCTION) {
            m_CurrentMeasuringResultsLiqiud.m_CounterBottleNotFilledOneAfterTheOther++;
        }
    }
    // Information geht in den ImageHeader für jedes Bild im Sharedmemory um später in auf der Windowsseite das Bild zu finden bei dem die Software die Flasche ausgeworfen hat
    m_LiquidMeasuringReady = true;
    m_CurrentMeasuringResultsLiqiud.m_CurrentMaschineState = m_ExchangeMemory->m_MaschineState;
    m_CurrentMeasuringResultsLiqiud.m_EjectBottle = EjectBottle;
    m_CurrentMeasuringResultsLiqiud.m_LiquidMeasuringReady = m_LiquidMeasuringReady;
    m_CurrentMeasuringResultsLiqiud.m_InjectionTimeFinished = 0;

    if (m_ListAverageAmountOfLiquid.size() >= m_ExchangeMemory->m_MeasuringParameter.m_RollingMeanValueLiquid) {
        m_ListAverageAmountOfLiquid.pop_front();
    }
    m_ListAverageAmountOfLiquid.push_back(m_CurrentMeasuringResultsLiqiud);
    CalculateMeanAndStandardDevationLiquid(m_ListAverageAmountOfLiquid);

    m_CurrentMeasuringResultsLiqiud.m_StatusResults = true;                                                    // indicator for GUI new result set
    m_ExchangeMemory->m_MeasuringParameter.m_CurrentMeasuringResultsLiqiud = m_CurrentMeasuringResultsLiqiud;  // Speichern der daten für die anzeige
    m_CurrentMeasuringResultsLiqiud.ClearResults();                                                            // Zähler werden nicht gelöscht

    return EjectBottle;
}

bool ImageData::CheckBottleEjectionConditions()
{
    bool EjectBottle = true;
    if (m_ExchangeMemory->m_MaschineState != OPERATION_STATUS_RELEASE_PRODUCTION) {
        if (m_ExchangeMemory->m_ButtonIsClickedEjectTheNextnBottles) {
            std::string InfoManualEjectionReady = "Manual Ejection Ready";
            SetInputOutputInfoData(InfoManualEjectionReady, INFO_CODE_MANUAL_EJECTION_READY);
        }
        m_ExchangeMemory->m_ButtonIsClickedEjectTheNextnBottles = false;
        m_CurrentCounterEjectBottleByUser = 0;
    }
    if (m_CurrentCounterEjectBottleByUser == 0 && m_ExchangeMemory->m_MaschineState == OPERATION_STATUS_RELEASE_PRODUCTION) {
        ApplyEjectionData();  //Übergabe der Werte, Wann das Ausblasen ausgeschaltet wird
        EjectBottle = false;  // Diese Flasche nicht auswerfen
    }
    if (m_ExchangeMemory->m_ButtonIsClickedEjectTheNextnBottles) {
        m_CurrentCounterEjectBottleByUser++;
        if (m_CurrentCounterEjectBottleByUser > m_ExchangeMemory->m_NumberEjectedBottlesByUser) {
            m_ExchangeMemory->m_ButtonIsClickedEjectTheNextnBottles = false;
            m_CurrentCounterEjectBottleByUser = 0;
            std::string InfoManualEjectionReady = "Manual Ejection Ready";
            SetInputOutputInfoData(InfoManualEjectionReady, INFO_CODE_MANUAL_EJECTION_READY);
        }
    }
    return EjectBottle;
}

void ImageData::CopyCleanImage()
{                                                                           // Bild speichern um die Verschmutzung zu bestimmen
    unsigned char* pSharedMemoryImageData = m_SharedMemoryCleanImageBlock;  // hier sauberes bild unter dem ventil da Flasche gerade befüllt ist
    memcpy(pSharedMemoryImageData + sizeof(ImageHeader), (unsigned char*)(m_OriginalCameraImage.data), m_OriginalCameraImage.rows * m_OriginalCameraImage.cols);
}

void ImageData::CalculateMeanAndStandardDevationLiquid(std::list<MeasuringResultsLiquid>& ListAverageAmountOfLiquid)
{
    double MeanAmountMiddle = 0.0;
    double MeanAmountMiddleFirstTrigger = 0.0;
    double MeanAmountMiddleSecondTrigger = 0.0;
    for (std::list<MeasuringResultsLiquid>::iterator i = ListAverageAmountOfLiquid.begin(); i != ListAverageAmountOfLiquid.end(); i++) {
        MeanAmountMiddleFirstTrigger = MeanAmountMiddleFirstTrigger + (*i).m_SumAmountLiquidFirstTrigger;
        MeanAmountMiddleSecondTrigger = MeanAmountMiddleSecondTrigger + (*i).m_SumAmountLiquidSecondTrigger;
        MeanAmountMiddle = MeanAmountMiddle + (*i).m_SumAmountMiddle;
    }

    if (ListAverageAmountOfLiquid.size() > 3) {
        MeanAmountMiddleFirstTrigger = MeanAmountMiddleFirstTrigger / ListAverageAmountOfLiquid.size();
        MeanAmountMiddleSecondTrigger = MeanAmountMiddleSecondTrigger / ListAverageAmountOfLiquid.size();
        MeanAmountMiddle = MeanAmountMiddle / ListAverageAmountOfLiquid.size();
        double AccumAmountMiddle = 0.0;
        double AccumAmountFirstTrigger = 0.0;
        double AccumAmountSecondTrigger = 0.0;

        for (std::list<MeasuringResultsLiquid>::iterator i = ListAverageAmountOfLiquid.begin(); i != ListAverageAmountOfLiquid.end(); i++) {
            AccumAmountMiddle += (MeanAmountMiddle - (*i).m_SumAmountMiddle) * (MeanAmountMiddle - (*i).m_SumAmountMiddle);
            AccumAmountFirstTrigger += (MeanAmountMiddleFirstTrigger - (*i).m_SumAmountLiquidFirstTrigger) * (MeanAmountMiddleFirstTrigger - (*i).m_SumAmountLiquidFirstTrigger);
            AccumAmountSecondTrigger += (MeanAmountMiddleSecondTrigger - (*i).m_SumAmountLiquidSecondTrigger) * (MeanAmountMiddleSecondTrigger - (*i).m_SumAmountLiquidSecondTrigger);
        }
        m_CurrentMeasuringResultsLiqiud.m_StandardDeviationMiddle = sqrt(AccumAmountMiddle / ListAverageAmountOfLiquid.size());
        m_CurrentMeasuringResultsLiqiud.m_MeanValueAmountMiddle = MeanAmountMiddle;
        m_CurrentMeasuringResultsLiqiud.m_StandardDeviationMiddleFirstTrigger = sqrt(AccumAmountFirstTrigger / ListAverageAmountOfLiquid.size());
        m_CurrentMeasuringResultsLiqiud.m_MeanValueAmountMiddleFirstTrigger = MeanAmountMiddleFirstTrigger;
        m_CurrentMeasuringResultsLiqiud.m_StandardDeviationMiddleSecondTrigger = sqrt(AccumAmountSecondTrigger / ListAverageAmountOfLiquid.size());
        m_CurrentMeasuringResultsLiqiud.m_MeanValueAmountMiddleSecondTrigger = MeanAmountMiddleSecondTrigger;
    } else {
        if (ListAverageAmountOfLiquid.size() > 0) {
            MeanAmountMiddleFirstTrigger = MeanAmountMiddleFirstTrigger / ListAverageAmountOfLiquid.size();
            MeanAmountMiddleSecondTrigger = MeanAmountMiddleSecondTrigger / ListAverageAmountOfLiquid.size();
            MeanAmountMiddle = MeanAmountMiddle / ListAverageAmountOfLiquid.size();

            m_CurrentMeasuringResultsLiqiud.m_StandardDeviationMiddle = 0.0;
            m_CurrentMeasuringResultsLiqiud.m_MeanValueAmountMiddle = MeanAmountMiddle;
            m_CurrentMeasuringResultsLiqiud.m_StandardDeviationMiddleFirstTrigger = 0.0;
            m_CurrentMeasuringResultsLiqiud.m_MeanValueAmountMiddleFirstTrigger = MeanAmountMiddleFirstTrigger;
            m_CurrentMeasuringResultsLiqiud.m_StandardDeviationMiddleSecondTrigger = 0.0;
            m_CurrentMeasuringResultsLiqiud.m_MeanValueAmountMiddleSecondTrigger = MeanAmountMiddleSecondTrigger;
        }
    }
}

bool ImageData::GetFrontEjectionSetOrReset()
{
    if (m_ListEjectionPointInTime.size() > 0)
        return m_ListEjectionPointInTime.front().m_DigitalOutputOnOrOff;
    else
        return false;
}

unsigned long long ImageData::GetFrontEjectionPointInTimeInns()
{
    if (m_ListEjectionPointInTime.size() > 0)
        return m_ListEjectionPointInTime.front().m_TimeStamp;  // first element
    else
        return 0;  // no data in FIFO stack
}

bool ImageData::TriggerSetOrReset()
{
    if (m_ListTriggerPointInTime.size() > 0)
        return m_ListTriggerPointInTime.front().m_DigitalOutputOnOrOff;
    else
        return false;
}

double ImageData::GetFrontTriggerPointInTimeInms()
{
    if (m_ListTriggerPointInTime.size() > 0)
        return m_ListTriggerPointInTime.front().m_TriggerTimeInms;  // m_TriggerTime1Inms;//first element
    else
        return -1.0;  // no data in FIFO stack
}

int ImageData::GetFrontTriggerChannelNumber()
{
    if (m_ListTriggerPointInTime.size() > 0)
        return m_ListTriggerPointInTime.front().m_ChannelNumber;  // m_TriggerTime1Inms;//first element
    else
        return -1;  // no data in FIFO stack
}

void ImageData::DeleteFrontTriggerPointInTime()
{
    if (m_ListTriggerPointInTime.size() > 0) m_ListTriggerPointInTime.pop_front();  // delete first element
}

void ImageData::DeleteFrontEjectionPointInTime()
{
    if (m_ListEjectionPointInTime.size() > 0) m_ListEjectionPointInTime.pop_front();  // delete first element
}

void ImageData::ClearVideoBuffer()
{
    VideoHeader videoHeader;

    memcpy(&videoHeader, m_SharedMemoryVideoBlock, sizeof(VideoHeader));  // read video header
    videoHeader.m_FrameIndex = 0;
    videoHeader.m_CurrentNumberFrames = 0;
    memcpy(m_SharedMemoryVideoBlock, &videoHeader, sizeof(VideoHeader));  // write video header
    m_TriggerCounter = 0;
}

void ImageData::AddNewVideoFrameFillingProcess(cv::Mat& CameraImage, ImageHeader& ImageHeader)
{
    if (m_SharedMemoryVideoBlockFillingProcess) {
        VideoHeader videoHeader;
        unsigned char* pVideoData = m_SharedMemoryVideoBlockFillingProcess;
        bool BottleEject = false;

        memcpy(&videoHeader, pVideoData, sizeof(VideoHeader));  // read video header
                                                                //      if (videoHeader.m_EnableWriteVideoData)
        {
            int ImageSize = (CameraImage.cols * CameraImage.rows);
            pVideoData = pVideoData + sizeof(VideoHeader) + (videoHeader.m_FrameIndex * (ImageSize + sizeof(ImageHeader)));  // pointer position for new image
            memcpy(pVideoData, &ImageHeader, sizeof(ImageHeader));                                                           // Speichern Bildheader
            pVideoData = pVideoData + sizeof(ImageHeader);
            memcpy(pVideoData, (unsigned char*)(CameraImage.data), ImageSize);  // Speichern Bild in Shared memory

            if (videoHeader.m_MaxNumberFrames > videoHeader.m_CurrentNumberFrames) {
                videoHeader.m_CurrentNumberFrames = videoHeader.m_CurrentNumberFrames + 1;  // Aktuelle Anzahl von Bildern im Sharedmemory
            }

            if (videoHeader.m_FrameIndex < videoHeader.m_MaxNumberFrames) videoHeader.m_FrameIndex = videoHeader.m_FrameIndex + 1;  // Indexnummer des zuletzt gespeicherten Bildes
            if (videoHeader.m_FrameIndex >= videoHeader.m_MaxNumberFrames) {  // new frame at the beginning shared memory ist voll, beginne wieder von Vorne
                videoHeader.m_FrameIndex = 0;
            }
            if (/*(videoHeader.m_MaxNumberFrames == videoHeader.m_CurrentNumberFrames) && */ (ImageHeader.m_ImageState & IMAGE_CAPTURE_TYPE_BOTTLE_EJECTED) &&
                (ImageHeader.m_ImageState & IMAGE_CAPTURE_TYPE_LIQUID_MEASURING_READY)) {
                videoHeader.m_EnableWriteVideoData = false;  // sperren erst auf der Windows seite auslesen und dann wieder freigeben
                BottleEject = true;
            }
            memcpy(m_SharedMemoryVideoBlockFillingProcess, &videoHeader, sizeof(VideoHeader));  // write video header

            if (BottleEject) {
                KS_setEvent((KSHandle)(m_ExchangeMemory)->m_HandleEventBottleEjected);  // Signal an die GUI Flasche wird ausgeworfen
            }
        }
    }
}

void ImageData::AddNewVideoFrame(cv::Mat& CameraImage, bool FirstOccurrence, bool ProductFound, bool LeftTriggerIsSet, bool RightTriggerIsSet, bool BottleEjected, bool LiquidMeasuringReady)
{
    VideoHeader videoHeader;
    ImageHeader imageHeader;
    unsigned char* pVideoData = NULL;

    // Fill Imageheader
    if (ProductFound) {
        imageHeader.m_ImageState = imageHeader.m_ImageState | IMAGE_CAPTURE_TYPE_PRODUCT_FOUND;  // 0x01;
    }
    if (FirstOccurrence) {
        imageHeader.m_ImageState = imageHeader.m_ImageState | IMAGE_CAPTURE_TYPE_FIRST_OCCURRENCE;  // 0x02;
    }
    if (LeftTriggerIsSet) {
        imageHeader.m_ImageState = imageHeader.m_ImageState | IMAGE_CAPTURE_TYPE_LEFT_TRIGGER_IS_SET;  // 0x04;
    }
    if (RightTriggerIsSet) {
        imageHeader.m_ImageState = imageHeader.m_ImageState | IMAGE_CAPTURE_TYPE_RIGHT_TRIGGER_IS_SET;  // 0x10
    }
    if (BottleEjected) {
        imageHeader.m_ImageState = imageHeader.m_ImageState | IMAGE_CAPTURE_TYPE_BOTTLE_EJECTED;  // 0x20;
    }
    if (LiquidMeasuringReady) {
        imageHeader.m_ImageState = imageHeader.m_ImageState | IMAGE_CAPTURE_TYPE_LIQUID_MEASURING_READY;  // 0x40;
    }
    if (m_ExchangeMemory->m_MaschineState == OPERATION_STATUS_RELEASE_PRODUCTION) {
        imageHeader.m_ProductionOn = true;
    }
    imageHeader.m_CurrentMeasuringResultsLiqiud = m_ExchangeMemory->m_MeasuringParameter.m_CurrentMeasuringResultsLiqiud;
    imageHeader.m_TimeStampInns = m_CurrentMeasuringResultsSpeed.m_CurrentTimeStampInns;
    imageHeader.m_CameraImageID = m_CameraImageID;
    // End Fill Imageheader
    pVideoData = m_SharedMemoryVideoBlock;
    memcpy(&videoHeader, pVideoData, sizeof(VideoHeader));  // read video header
    if (m_CurrentMeasuringResultsSpeed.m_CurrentTimeStampInns > m_CurrentMeasuringResultsSpeed.m_LastTimeStampInns) {
        int ImageSize = (CameraImage.cols * CameraImage.rows);
        videoHeader.m_CurrentTimeStampInns = m_CurrentMeasuringResultsSpeed.m_CurrentTimeStampInns;
        videoHeader.m_LastTimeStampInns = m_CurrentMeasuringResultsSpeed.m_LastTimeStampInns;
        videoHeader.m_ProductPressentTime = GetProductPresentTimeInms();
        videoHeader.m_CurrentSpeedInmmPerms = GetAverageSpeedInMMPerMs();
        if (videoHeader.m_FrameIndex < MAX_IMAGES_IN_VIDEO_BUFFER)
            videoHeader.m_ImageIndexIsCopied[videoHeader.m_FrameIndex] = 0;  // als noch nicht kopiert kennzeichnen, wird auf der Windowsseite genutzt, wenn das Bild in Sharedmemory auf der
                                                                             // Windowsseite kopiert wird, wird dort dann eine  1 eingertagen

        pVideoData = pVideoData + sizeof(VideoHeader) + (videoHeader.m_FrameIndex * (ImageSize + sizeof(ImageHeader)));  // pointer position for new image
        memcpy(pVideoData, &imageHeader, sizeof(ImageHeader));                                                           // Speichern Bildheader
        pVideoData = pVideoData + sizeof(ImageHeader);
        memcpy(pVideoData, (unsigned char*)(CameraImage.data), ImageSize);  // Speichern Bild in Shared memory

        if (videoHeader.m_MaxNumberFrames > videoHeader.m_CurrentNumberFrames) videoHeader.m_CurrentNumberFrames = videoHeader.m_CurrentNumberFrames + 1;  // Aktuelle Anzahl von Bilder im Sharedmemory

        if (videoHeader.m_FrameIndex < videoHeader.m_MaxNumberFrames) videoHeader.m_FrameIndex = videoHeader.m_FrameIndex + 1;  // Indexnummer des zuletzt gespeicherten Bildes
        if (videoHeader.m_FrameIndex >= videoHeader.m_MaxNumberFrames) {  // new frame at the beginning shared memory ist voll, beginne wieder von Vorne
            videoHeader.m_FrameIndex = 0;
        }
        memcpy(m_SharedMemoryVideoBlock, &videoHeader, sizeof(VideoHeader));  // write video header
    }
    AddNewVideoFrameFillingProcess(CameraImage, imageHeader);
    return;
}

void ImageData::DrawTextOutput(cv::Mat& Image, std::string& output, int xpos, int ypos)
{
    std::istringstream ss(output);
    std::string token;
    int fontface = cv::FONT_HERSHEY_SIMPLEX;
    double scale = 0.4;
    int thickness = 1;
    int baseline = 0;
    unsigned long long TimeStamp1, TimeStamp2;

    while (std::getline(ss, token, '\n')) {
        cv::Size text = cv::getTextSize(token, fontface, scale, thickness, &baseline);
        cv::rectangle(Image, cv::Point(xpos, ypos) + cv::Point(0, baseline), cv::Point(xpos, ypos) + cv::Point(text.width, -text.height), CV_RGB(255, 255, 255), cv::FILLED);
        cv::putText(Image, token, cv::Point(xpos, ypos), fontface, scale, cv::Scalar(0), thickness);
        ypos = ypos + 15;
    }
}

std::string ImageData::MeasuringResultsSpeedToString(MeasuringResults& measuringResults, std::string& AddMsg)
{
    std::string Output;

    if (measuringResults.ProductFound()) {  // Measure data calculate speed
        Output = Output + AddMsg;
        Output = Output + cv::format("C:%.1f%% EdgeL:%.1f%%  EdgeR:%.1f%% Count:%d\n", measuringResults.m_BottleContrastInPercent, measuringResults.m_EdgeLeftContrastInPercent,
                                     measuringResults.m_EdgeRightContrastInPercent, measuringResults.m_CounterBottleInROI);
        Output = Output + cv::format("D:%.1fmm PS:%.3fmm/pix Speed:%.3fmm/ms\n", measuringResults.m_MeasuredBottleNeckDiameterInmm, measuringResults.m_CalculatePixelSizeInMMPerPixel,
                                     measuringResults.m_CurrentSpeedInmmPerms);
    } else
        Output = AddMsg;
    return Output;
}

void ImageData::AddTriggerPointsToTriggerList(double& TriggerPointFirst, double& TriggerPointSecond)
{
    if (m_ExchangeMemory->m_EnableTrigger)  // Wird nur auf false gesetzt wenn der Druck absinkt oder nicht eingeschaltet ist
    {
        TriggerDataValve FirstTriggerSet, FirstTriggerReset;
        TriggerDataValve SecondTriggerSet, SecondTriggerReset;

        if (m_ListTriggerPointInTime.size() >= MAX_FIFO_SIZE) {  //überlauf verhindern
            std::string Info = "Trigger Buffer Overflow";
            m_ListTriggerPointInTime.pop_front();  // delete first element set
            m_ListTriggerPointInTime.pop_front();  // delete second element reset
            SetInputOutputInfoData(Info, INFO_CODE_INPUT_OUTPUT_DEVICE_ANY_ERROR);
        }
        FirstTriggerSet.m_TriggerTimeInms = TriggerPointFirst;
        FirstTriggerSet.m_DigitalOutputOnOrOff = true;
        FirstTriggerReset.m_TriggerTimeInms = TriggerPointFirst + m_ExchangeMemory->m_EtherCATConfigData.m_TimePeriodTriggerOutputOnInms;
        FirstTriggerReset.m_DigitalOutputOnOrOff = false;
        SecondTriggerSet.m_TriggerTimeInms = TriggerPointSecond;
        SecondTriggerSet.m_DigitalOutputOnOrOff = true;
        SecondTriggerReset.m_TriggerTimeInms = TriggerPointSecond + m_ExchangeMemory->m_EtherCATConfigData.m_TimePeriodTriggerOutputOnInms;
        SecondTriggerReset.m_DigitalOutputOnOrOff = false;

        if (TheLeftValveIsFilledFirst()) {
            // first is left -> m_ChannelNumber = 1
            FirstTriggerSet.m_ChannelNumber = FirstTriggerReset.m_ChannelNumber = 1;
            // second  is right -> m_ChannelNumber = 2
            SecondTriggerSet.m_ChannelNumber = SecondTriggerReset.m_ChannelNumber = 2;
        } else {
            // first is right -> m_ChannelNumber = 2
            FirstTriggerSet.m_ChannelNumber = FirstTriggerReset.m_ChannelNumber = 2;
            // second is left -> m_ChannelNumber = 1
            SecondTriggerSet.m_ChannelNumber = SecondTriggerReset.m_ChannelNumber = 1;
        }
        if (m_ExchangeMemory->m_MeasuringParameter.m_WorkWithTwoValves) {
            switch (m_ExchangeMemory->m_MeasuringParameter.m_UsedTriggerOutputs) {
                case USE_BOTH_VALVES:
                    m_ListTriggerPointInTime.push_back(FirstTriggerSet);
                    m_ListTriggerPointInTime.push_back(FirstTriggerReset);
                    m_ListTriggerPointInTime.push_back(SecondTriggerSet);
                    m_ListTriggerPointInTime.push_back(SecondTriggerReset);
                    if (TriggerPointFirst != TriggerPointSecond) {
                        m_ListTriggerPointInTime.sort(ImageData::SortByTriggerTime);  // Sicherstellen das der mit der kleinsten Zeit an vorderster Position steht
                    }
                    break;
                case USE_ONLY_LEFT_VALVE:
                    if (TheLeftValveIsFilledFirst()) {
                        m_ListTriggerPointInTime.push_back(FirstTriggerSet);
                        m_ListTriggerPointInTime.push_back(FirstTriggerReset);
                    } else {
                        m_ListTriggerPointInTime.push_back(SecondTriggerSet);
                        m_ListTriggerPointInTime.push_back(SecondTriggerReset);
                    }
                    break;
                case USE_ONLY_RIGHT_VALVE:
                    if (TheLeftValveIsFilledFirst()) {
                        m_ListTriggerPointInTime.push_back(SecondTriggerSet);
                        m_ListTriggerPointInTime.push_back(SecondTriggerReset);
                    } else {
                        m_ListTriggerPointInTime.push_back(FirstTriggerSet);
                        m_ListTriggerPointInTime.push_back(FirstTriggerReset);
                    }
                    break;
                default:
                    m_ListTriggerPointInTime.push_back(FirstTriggerSet);
                    m_ListTriggerPointInTime.push_back(FirstTriggerReset);
                    m_ListTriggerPointInTime.push_back(SecondTriggerSet);
                    m_ListTriggerPointInTime.push_back(SecondTriggerReset);
                    if (TriggerPointFirst != TriggerPointSecond) {
                        m_ListTriggerPointInTime.sort(ImageData::SortByTriggerTime);  // Sicherstellen das der mit der kleinsten Zeit an vorderster Position steht
                    }
                    break;
            }
        } else {
            m_ListTriggerPointInTime.push_back(FirstTriggerSet);
            m_ListTriggerPointInTime.push_back(FirstTriggerReset);
        }
    }
}

bool ImageData::SortByTriggerTime(TriggerDataValve& T1, TriggerDataValve& T2)
{
    return T1.m_TriggerTimeInms < T2.m_TriggerTimeInms;
}

/*
                                                         Abstand zwischen den Ventilen(V1 und V2) 27 mm
                                                        TriggerpointFirst = V1         TrigerpointSecond = V2
                                                                V1     V2
        ReferenceMeasurePosInMM                                 |      |
                   |                                    _______________________
                   v                                   |                       |
   _________________________________                   |                       | Blaues Messfenster
  |_________________________________|Gruenes Messf.    |_______________________|
           












                   |<- DistanceBetweenMeasurePosAndTriggerPosInMM ->|    <- Bei diesem Wert ist der halbe Flaschendurchmesser und der halbe Abstand der Ventile schon abgezogen
                     _________                        _________
                    |_________|                      |_________|
                         ^                                ^
                         |                                |
                 Berechnete Fl. Pos                   Wenn Flasche an dieser Stelle, dann muss V1 auslösen
                         ^                                ^
                         |________________________________|
                                        V
                          Distanz/Speed = TriggerPointFirst

BottleMiddelPositionInmm: Ist der berechnete Mittelwert aus allen gemessenen Positionen im grünen Messfenster, er kann vor oder hinter der ReferenceMeasurPosInMM liegen
*/
void ImageData::CalculateTriggeringPointInTime(MeasuringResults& measuringResults, double& TriggerPointFirst, double& TriggerPointSecond)
{
    if (measuringResults.m_CurrentSpeedInmmPerms > m_ExchangeMemory->m_MeasuringParameter.m_MinSpeedInMMPerMs) {
        // Abstand Mittenposition gruenes Messfenster zu Mittenposition blaues Messfenster der halbe Flaschendurchmesser ist hier schon abgezogen
        double DistanceBetweenMeasurePosAndTriggerPosInMM =
            m_ExchangeMemory->m_MeasuringParameter.m_DistanceBetweenMeasureMiddlePosAndTriggerPosInPixel * m_ExchangeMemory->m_MeasuringParameter.m_PixelSizeInMMPerPixel;
        // Mittenposition des gruenen Messfensters
        double ReferenceMeasurePositionInMM = m_ExchangeMemory->m_MeasuringParameter.m_ReferenceMeasurePositionInPixel * m_ExchangeMemory->m_MeasuringParameter.m_PixelSizeInMMPerPixel;
        double TimeBetweenInjectionAndMeasurePosInms = DistanceBetweenMeasurePosAndTriggerPosInMM / measuringResults.m_CurrentSpeedInmmPerms;
        double TimeOffsetMeasurePositionInms = (measuringResults.m_BottleMiddelPositionInmm - ReferenceMeasurePositionInMM) / measuringResults.m_CurrentSpeedInmmPerms;  // in mmSec;
        double TimeOffsetInjectionPosFirstValveInms = m_ExchangeMemory->m_MeasuringParameter.m_TriggerOffsetFirstValveInmm / measuringResults.m_CurrentSpeedInmmPerms;
        double TimeOffsetInjectionPosSecondValveInms = m_ExchangeMemory->m_MeasuringParameter.m_TriggerOffsetSecondValveInmm / measuringResults.m_CurrentSpeedInmmPerms;
        double TimeOffsetDistanceBetweenFirstAndSecond = m_ExchangeMemory->m_MeasuringParameter.m_DistancesBetweenValves / measuringResults.m_CurrentSpeedInmmPerms;

        if (m_ExchangeMemory->m_MeasuringParameter.m_BandDirectional == BAND_DIRECTIONAL_LEFT_TO_RIGHT) {
            TimeOffsetMeasurePositionInms = TimeOffsetMeasurePositionInms * (-1.0);
        } else {
            TimeOffsetInjectionPosFirstValveInms = TimeOffsetInjectionPosFirstValveInms * (-1.0);
            TimeOffsetInjectionPosSecondValveInms = TimeOffsetInjectionPosSecondValveInms * (-1.0);
        }
        TriggerPointFirst = measuringResults.m_CurrentTimeStampInms + TimeOffsetMeasurePositionInms + TimeBetweenInjectionAndMeasurePosInms + TimeOffsetInjectionPosFirstValveInms;
        TriggerPointSecond = measuringResults.m_CurrentTimeStampInms + TimeOffsetMeasurePositionInms + TimeBetweenInjectionAndMeasurePosInms + TimeOffsetInjectionPosSecondValveInms +
                             TimeOffsetDistanceBetweenFirstAndSecond;
    } else {
        TriggerPointFirst = 0;
        TriggerPointSecond = 0;
    }
}

// hier nur die Berechnung des Möglichen Auswurfes, später wird entschieden ob Auswurf aktiviert/deaktiviert wird, nämlich dann wenn Flasche erfolgreich befüllt
void ImageData::CalculateEjectionPointInTime(unsigned long long CurrentTimeStampInns)
{
    double CurrentSpeedInMMPerms;

    if (m_ExchangeMemory->m_MeasuringParameter.m_UseSpeedFromISCalcEjectionTime)
        CurrentSpeedInMMPerms = GetSpeedFromISMaschineInMMPerMs();
    else
        CurrentSpeedInMMPerms = GetAverageSpeedInMMPerMs();
    m_EjectionSet.m_TimeStamp = 0;
    m_EjectionReset.m_TimeStamp = 0;
    if (CurrentSpeedInMMPerms > m_ExchangeMemory->m_MeasuringParameter.m_MinSpeedInMMPerMs) {
        double DeltaTInms = m_ExchangeMemory->m_MeasuringParameter.m_DistanceBottleEjectionInmm / CurrentSpeedInMMPerms;   // zeit bis zur auswurfposition
        double TimePeriodNotBlowInms = m_ExchangeMemory->m_MeasuringParameter.m_ProductWidthInmm / CurrentSpeedInMMPerms;  // zeitdauer nicht blasen

        m_EjectionSet.m_TimeStamp = CurrentTimeStampInns + static_cast<unsigned long long>(DeltaTInms) * 1000000;
        m_EjectionSet.m_DigitalOutputOnOrOff = false;  // Flasche ist Ok nicht auswerfen, Blasen aus

        m_EjectionReset.m_TimeStamp = m_EjectionSet.m_TimeStamp + static_cast<unsigned long long>(TimePeriodNotBlowInms) * 1000000;
        m_EjectionReset.m_DigitalOutputOnOrOff = true;  // wenn Flasche am Auswerfer vorbei dann wieder Blasen an

        m_ExchangeMemory->m_MeasuringParameter.m_TimePeriodNotBlowInms = TimePeriodNotBlowInms;
        if (m_ExchangeMemory->m_MeasuringParameter.m_BlowOutEjectorNormallyClosed) {  // Wenn Auswerfer normalerweise geschlossen dann Setzen und Rücksetzen invertieren
            m_EjectionSet.m_DigitalOutputOnOrOff = !m_EjectionSet.m_DigitalOutputOnOrOff;
            m_EjectionReset.m_DigitalOutputOnOrOff = !m_EjectionReset.m_DigitalOutputOnOrOff;
        }
    }
}

void ImageData::ApplyEjectionData()
{
    if (m_ListEjectionPointInTime.size() >= MAX_FIFO_SIZE) {
        m_ListEjectionPointInTime.pop_front();  // delete first element set
        m_ListEjectionPointInTime.pop_front();  // delete second element reset
    }
    if (m_EjectionSet.m_TimeStamp > 0 && m_EjectionReset.m_TimeStamp > 0) {
        m_ListEjectionPointInTime.push_back(m_EjectionSet);    // add at the end
        m_ListEjectionPointInTime.push_back(m_EjectionReset);  // add at the end
    }
}

void ImageData::GetCurrentTimeStampIn100nsUnits(ExchangeMemory* SharedData, unsigned long long& TimeStamp)
{
    int64 CurrentTimeStampIn100nsUnits = 0;
    KS_getClock(&CurrentTimeStampIn100nsUnits, KS_CLOCK_MEASURE_HIGHEST);
    KS_convertClock(&CurrentTimeStampIn100nsUnits, KS_CLOCK_MEASURE_HIGHEST, KS_CLOCK_MACHINE_TIME, 0);
    TimeStamp = static_cast<unsigned long long>(CurrentTimeStampIn100nsUnits);  // +SharedData->m_MeasuringParameter.m_TimeStampOffsetIn100nsUnits;
}

unsigned long long ImageData::CalculateWaitTimeBottleIsOutOfROI(double CenterXPosBottleInMM)
{
    if (GetAverageSpeedInMMPerMs() > m_ExchangeMemory->m_MeasuringParameter.m_MinSpeedInMMPerMs) {
        int MeasureWindowIndex = ROI_INDEX_MEASURE_SPEED;
        int x = m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowOffsetXCopy[MeasureWindowIndex];
        int w = m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowWidthCopy[MeasureWindowIndex];
        double PositionInMMBottelIsOutOfROI;
        double Distance;
        double WaitTimeInMsBottleIsOutOfROI;
        double OffsetInMM = m_ExchangeMemory->m_MeasuringParameter.m_BottleOffsetOutOfROIInmm;
        double HalfBottelneckInMM = m_ExchangeMemory->m_MeasuringParameter.m_BottleneckDiameter / 2.0;

        if (m_ExchangeMemory->m_MeasuringParameter.m_BandDirectional == BAND_DIRECTIONAL_LEFT_TO_RIGHT) {  // messfenster sitzt links
            PositionInMMBottelIsOutOfROI = fabs((static_cast<double>(x) + static_cast<double>(w)) * m_ExchangeMemory->m_MeasuringParameter.m_PixelSizeInMMPerPixel - HalfBottelneckInMM + OffsetInMM);
            Distance = PositionInMMBottelIsOutOfROI - CenterXPosBottleInMM;
        } else {  // messfenster sitzt rechts
            PositionInMMBottelIsOutOfROI = fabs(static_cast<double>(x) * m_ExchangeMemory->m_MeasuringParameter.m_PixelSizeInMMPerPixel + HalfBottelneckInMM - OffsetInMM);
            Distance = CenterXPosBottleInMM - PositionInMMBottelIsOutOfROI;
        }
        WaitTimeInMsBottleIsOutOfROI = (Distance / GetAverageSpeedInMMPerMs());
        return static_cast<unsigned long long>(WaitTimeInMsBottleIsOutOfROI * 1000000);
    } else
        return 0;
}

double ImageData::GetSpeedFromISMaschineInMMPerMs()
{
    double Speed = 0.0;
    double DeltaTInms = m_ExchangeMemory->m_EtherCATConfigData.m_DeltaTFromIS / ((double)(10000.0));  // m_DeltaTFromIS wird in der IO-Task berechnet

    DeltaTInms = DeltaTInms / 2.0;  // die zeit muss noch halbiert werden, halbe Periodendauer
    if (DeltaTInms > 0.0) {
        Speed = m_ExchangeMemory->m_MeasuringParameter.m_FormatFromISInmm / DeltaTInms;
    }
    return Speed;
}

double ImageData::GetAverageSpeedInMMPerMs()
{
    return m_AveragedMeasuringResults.m_CurrentSpeedInmmPerms;
}

double ImageData::GetProductPresentTimeInms()
{
    return CalculatProductPressentTime() / 1000000.0;
}

unsigned long long ImageData::CalculatProductPressentTime()
{
    if (GetAverageSpeedInMMPerMs() > 0.0) {
        double DistanceInMM = m_ExchangeMemory->m_MeasuringParameter.m_PixelSizeInMMPerPixel * m_ImageWidth;
        double TimeInMs = DistanceInMM / GetAverageSpeedInMMPerMs();
        return static_cast<unsigned long long>(TimeInMs * 1000000);  // in ns
    } else
        return 0;
}

void ImageData::SimulateLiquidFlow(cv::Mat& CameraImage, int ValveIndex, std::vector<cv::Rect>& liquidRois)
{
    cv::Rect roi_liquid_sector;
    cv::Mat ROIImage;
    if (ValveIndex == LEFT_VALVE_INDEX) {
        roi_liquid_sector = liquidRois[1];
    } else {
        roi_liquid_sector = liquidRois[5];
    }
    cv::Point pt1(roi_liquid_sector.x, roi_liquid_sector.y - 10);
    cv::Point pt2(roi_liquid_sector.x + roi_liquid_sector.width, roi_liquid_sector.height - 10);
    cv::rectangle(CameraImage, pt1, pt2, cv::Scalar(60, 60, 60), -1);  // draw valve into image
    int NumberPrintedDots = 35;

    if (ValveIndex == RIGHT_VALVE_INDEX) {
        if (m_ExchangeMemory->m_MeasuringParameter.m_UsedTriggerOutputs == USE_ONLY_LEFT_VALVE) {
            return;
        }
        if (m_RightTriggerIsSet) {
            m_CounterSimulateSecondTrigger = 0;
            m_StartPaintingSimulationLiquidSecondTrigger = true;
        }
        if (m_StartPaintingSimulationLiquidSecondTrigger) {
            m_CounterSimulateSecondTrigger++;
            if (m_CounterSimulateSecondTrigger < NumberPrintedDots) {
                CopyROIImage(roi_liquid_sector, CameraImage, ROIImage);
                DrawDroplet(ROIImage, ValveIndex);
                ROIImage.copyTo(CameraImage(roi_liquid_sector));
            } else {
                m_StartPaintingSimulationLiquidSecondTrigger = false;
            }
        }
    } else {
        if (m_ExchangeMemory->m_MeasuringParameter.m_UsedTriggerOutputs == USE_ONLY_RIGHT_VALVE) {
            return;
        }
        if (m_LeftTriggerIsSet) {
            m_CounterSimulateFirstTrigger = 0;
            m_StartPaintingSimulationLiquidFirstTrigger = true;
        }
        if (m_StartPaintingSimulationLiquidFirstTrigger) {
            m_CounterSimulateFirstTrigger++;
            if (m_CounterSimulateFirstTrigger < NumberPrintedDots) {
                CopyROIImage(roi_liquid_sector, CameraImage, ROIImage);
                DrawDroplet(ROIImage, ValveIndex);
                ROIImage.copyTo(CameraImage(roi_liquid_sector));
            } else {
                m_StartPaintingSimulationLiquidFirstTrigger = false;
            }
        }
    }
}

void ImageData::DrawDroplet(cv::Mat& SubImage, int ValveIndex)
{
    cv::Mat LiquidImage = cv::Mat::zeros(SubImage.rows, SubImage.cols, CV_8U);
    int xpos = SubImage.cols / 2;  // IntegerRand(1, SubImage.cols - 2);
    int ypos = 0;                  // IntegerRand(1, SubImage.rows - 2);
    int startRadius = 3;           // Großer radius Simulation große Menge Flüssigkeit , kleiner Radius geringe Flüssigkeitsmenge
    int pos = 0;
    int startRadiusLeftTrigger = IntegerRand(3, 5);
    int startRadiusRightTrigger = IntegerRand(5, 7);

    if (ValveIndex == RIGHT_VALVE_INDEX) {
        startRadius = startRadiusRightTrigger;
        ypos = m_CounterSimulateSecondTrigger * 2;
    } else {
        startRadius = startRadiusLeftTrigger;
        ypos = m_CounterSimulateFirstTrigger * 2;
    }
    for (int i = 0; i < (startRadius - 1); i++) {
        pos = pos + 2;
        int radius = startRadius;
        if (radius > 0) cv::circle(LiquidImage, cv::Point(xpos, ypos - pos), radius--, cv::Scalar(128, 128, 128), -1, 8, 0);
        if (radius > 0) cv::circle(LiquidImage, cv::Point(xpos, ypos - pos - 2), radius--, cv::Scalar(128, 128, 128), -1, 8, 0);
        if (radius > 0) cv::circle(LiquidImage, cv::Point(xpos, ypos - pos - 4), radius--, cv::Scalar(128, 128, 128), -1, 8, 0);
        if (radius > 0) cv::circle(LiquidImage, cv::Point(xpos, ypos - pos - 6), radius--, cv::Scalar(128, 128, 128), -1, 8, 0);
        if (radius > 0) cv::circle(LiquidImage, cv::Point(xpos, ypos - pos - 8), radius--, cv::Scalar(128, 128, 128), -1, 8, 0);
        startRadius--;
    }
    SubImage = SubImage - LiquidImage;
}

void ImageData::SimulateSplashes(cv::Mat& CameraImage, int ValveIndex, int side, std::vector<cv::Rect>& liquidRois)
{
    int CounterSimulateSplashes = m_CounterSimulateSplashesLeft;
    if (side == RIGHT_TRIGGER_SIDE_INDEX) {
        CounterSimulateSplashes = m_CounterSimulateSplashesRight;
    }
    if (CounterSimulateSplashes > 0) {
        CounterSimulateSplashes++;
    }

    if (ValveIndex == LEFT_VALVE_INDEX) {
        if (m_LeftTriggerIsSet) {
            CounterSimulateSplashes = 1;
        }
    } else {
        if (ValveIndex == RIGHT_VALVE_INDEX) {
            if (m_RightTriggerIsSet) {
                CounterSimulateSplashes = 1;
            }
        }
    }

    if (CounterSimulateSplashes > 30) {
        if (CounterSimulateSplashes > 50) {
            CounterSimulateSplashes = 0;
        }
        cv::Rect ROIRect;
        if (side == LEFT_TRIGGER_SIDE_INDEX) {
            if (ValveIndex == LEFT_VALVE_INDEX) {
                ROIRect = liquidRois[0];  // roi_left_splash;
            } else {
                ROIRect = liquidRois[4];  // roi_middel_splash_right;
            }
        } else {
            if (ValveIndex == LEFT_VALVE_INDEX) {
                ROIRect = liquidRois[3];  // roi_middel_splash_left;
            } else {
                ROIRect = liquidRois[6];  // roi_right_splash;
            }
        }
        cv::Mat ROIImage;
        cv::Mat SplashImage = cv::Mat::zeros(ROIRect.height, ROIRect.width, CV_8U);
        int xposAdd, yposAdd, xposMinus, yposMinus;

        m_SimulateSplashes.update(m_CameraImageID * 3000);
        for (int i = 0; i < SimulatSplashes::NSplashS; i++) {
            int xpos = (m_SimulateSplashes.m_pSplashs[i].m_x + 1) * ROIRect.width / 2;
            int ypos = m_SimulateSplashes.m_pSplashs[i].m_y * ROIRect.width / 2 + ROIRect.height;
            if (xpos >= ROIRect.width) xpos = ROIRect.width - 1;
            if (ypos >= ROIRect.height) ypos = ROIRect.height - 1;
            if (xpos < 0) xpos = 0;
            if (ypos < 0) ypos = 0;
            SplashImage.at<uchar>(ypos, xpos) = 90;
        }
        CopyROIImage(ROIRect, CameraImage, ROIImage);
        ROIImage = ROIImage - SplashImage;
        ROIImage.copyTo(CameraImage(ROIRect));
    }

    if (side == RIGHT_TRIGGER_SIDE_INDEX) {
        m_CounterSimulateSplashesRight = CounterSimulateSplashes;
    } else {
        m_CounterSimulateSplashesLeft = CounterSimulateSplashes;
    }
}

void ImageData::SimulateDegreeOfPollution(cv::Mat& CameraImage)
{
    cv::Rect ROIRect;  // = GetMeasureWindowCheckCleanImage();
    ROIRect.x = 0;
    ROIRect.y = 0;
    ROIRect.width = CameraImage.cols;
    ROIRect.height = CameraImage.rows;
    int w = ROIRect.width;
    int h = ROIRect.height;
    cv::Mat ROIImage;  // , DirtyImage;
    int xpos, ypos, Numbers;
    int xposAdd, yposAdd, xposMinus, yposMinus;

    if (m_DirtyImage.empty())
        m_DirtyImage = cv::Mat::zeros(h, w, CV_8U);  // cv::Mat.zeros(n_rows, n_cols, CvType.CV_8UC1);
    else {
        if (h != m_DirtyImage.rows || w != m_DirtyImage.cols) m_DirtyImage = cv::Mat::zeros(h, w, CV_8U);
    }

    // m_DirtyImage = cv::Mat::zeros(h, w, CV_8U);
    srand(time(0));
    m_CounterSimulateDegreeOfPollution++;
    Numbers = static_cast<int>((double)(m_CounterSimulateDegreeOfPollution) / 200.0);
    for (int i = 0; i < Numbers; i++) {
        xpos = IntegerRand(0, w - 1);
        ypos = IntegerRand(0, h - 1);
        xposAdd = xpos + 1;
        yposAdd = ypos + 1;
        xposMinus = xpos - 1;
        yposMinus = ypos - 1;

        if (xposAdd >= w) xposAdd = w - 1;
        if (yposAdd >= h) yposAdd = h - 1;

        if (xposMinus < 0) xposMinus = 0;
        if (yposMinus < 0) yposMinus = 0;

        if (m_DirtyImage.at<uchar>(ypos, xpos) == 0) {
            m_DirtyImage.at<uchar>(ypos, xpos) = 90;
            m_DirtyImage.at<uchar>(ypos, xposMinus) = 120;
            m_DirtyImage.at<uchar>(ypos, xposAdd) = 120;
            m_DirtyImage.at<uchar>(yposMinus, xpos) = 120;
            m_DirtyImage.at<uchar>(yposAdd, xpos) = 120;
        }
        // m_CounterSimulateDegreeOfPollution = 0;
    }

    CopyROIImage(ROIRect, CameraImage, ROIImage);
    ROIImage = ROIImage - m_DirtyImage;

    ROIImage.copyTo(CameraImage(ROIRect));
}

int ImageData::IntegerRand(int iMin, int iMax)
{
    double r = iMax - iMin;
    return iMin + static_cast<int>(r * rand() / (RAND_MAX + 1.0));
}

//       side==LEFT_TRIGGER_SIDE_INDEX       side==RIGHT_TRIGGER_SIDE_INDEX
//                    v1 <-Valvedistance-> v2
// x,y _____________________________________________________
//     |            |     |             |      |            |
//     |            |     |             |      |            |
//     |  Left ROI  |Mid. |Right  Left  | Mid. | Right ROI  |   h
//     |            | ROI | ROI    ROI  | ROI  |            |
//     |____________|_____|_____________|______|____________|
//
//
//     |          w             |
//
//    Messbereich erster Befüllvorgang Breite w Linke Seite
//
//                              |             w             |
//
//                    Messbereich zweiter Befüllvorgang Breite w Rechte Seite
//
//
//
//     |<-        w          ->|
//                   ->    ValveDistance    <-
//      _____________________________________________________   _
//     |            |     |             |      |            |
//     |            |     |             |      |            |
//     |            |     |             |      |            |   h
//     |            |     |             |      |            |
//     |____________|_____|_____________|______|____________|   _
//
//                ->       <-
//              LiquidSectorWidth
void ImageData::StartMeasuringLiqiudAmount(cv::Mat& CameraImage, int x, int y, int w, int h, int side, int LiquidSectorXpos)
{
    double thresh = m_ExchangeMemory->m_MeasuringParameter.m_ThresholdBinaryImageLiquid;
    double maxValue = 255;
    cv::Rect ROIRect(x, y, w, h);
    cv::Rect LeftROIRect, MiddleROIRect, RightROIRect;
    cv::Mat ROIImage, LeftImage, RightImage, MiddleImage;
    cv::Moments momentsLeft, momentsMiddle, momentsRight;
    int LiquidSectorWidthInPixel = static_cast<int>(m_ExchangeMemory->m_MeasuringParameter.m_InjectonWindowMiddleWidthInMm / m_ExchangeMemory->m_MeasuringParameter.m_PixelSizeInMMPerPixel);
    int LiquidSectorWidtHalf = static_cast<int>(LiquidSectorWidthInPixel / 2.0);

    LeftROIRect.y = MiddleROIRect.y = RightROIRect.y = 0;
    LeftROIRect.height = MiddleROIRect.height = RightROIRect.height = h;
    if (side == LEFT_TRIGGER_SIDE_INDEX) {
        MiddleROIRect.x = LiquidSectorXpos;
        MiddleROIRect.width = LiquidSectorWidthInPixel;

        LeftROIRect.x = 0;
        LeftROIRect.width = MiddleROIRect.x;

        RightROIRect.x = MiddleROIRect.x + LiquidSectorWidthInPixel;
        RightROIRect.width = w - RightROIRect.x;
        m_InjectionMiddleXPosition[side] = x + MiddleROIRect.x + LiquidSectorWidtHalf;
    } else {
        if (side == RIGHT_TRIGGER_SIDE_INDEX) {
            MiddleROIRect.x = LiquidSectorXpos;
            MiddleROIRect.width = LiquidSectorWidthInPixel;

            LeftROIRect.x = 0;
            LeftROIRect.width = MiddleROIRect.x;

            RightROIRect.x = MiddleROIRect.x + LiquidSectorWidthInPixel;
            RightROIRect.width = w - RightROIRect.x;
            m_InjectionMiddleXPosition[side] = x + MiddleROIRect.x + LiquidSectorWidtHalf;
        } else {  // Hier nur ein Ventil
            MiddleROIRect.x = static_cast<int>(w / 2.0 - LiquidSectorWidtHalf);
            MiddleROIRect.width = LiquidSectorWidthInPixel;

            LeftROIRect.x = 0;
            LeftROIRect.width = MiddleROIRect.x - x;

            RightROIRect.x = w - (MiddleROIRect.x + LiquidSectorWidthInPixel);
            RightROIRect.width = w - RightROIRect.x;

            m_InjectionMiddleXPosition[side] = x + static_cast<int>((double)(w) / 2);
        }
    }

    // ROI aus Kamerbild kopieren
    CopyROIImage(ROIRect, CameraImage, ROIImage);  // Blaues Messfenster aus dem Kamerbild ausschneiden
    // binaerbild berechnen
    cv::threshold(ROIImage, ROIImage, thresh, maxValue, cv::THRESH_BINARY_INV);  // Binärbild erzeugen (Hintergrund schwarz Vordergrund weiss)
    // ROI in drei bereiche aufteilen
    if (LiquidSectorWidthInPixel < w) {
        CopyROIImage(LeftROIRect, ROIImage, LeftImage);  // Linkes Fenster ausschneiden
        momentsLeft = cv::moments(LeftImage, true);      // momente berechnen bzw. die flaeche fuer jeden Bereich
        m_AmountSplashLeftROI[side] = static_cast<int>(momentsLeft.m00);
    }
    CopyROIImage(MiddleROIRect, ROIImage, MiddleImage);
    momentsMiddle = cv::moments(MiddleImage, true);  // momente berechnen bzw. die flaeche fuer jeden Bereich
    m_AmountLiquidMiddleROI[side] = static_cast<int>(momentsMiddle.m00);
    if (LiquidSectorWidthInPixel < w) {
        CopyROIImage(RightROIRect, ROIImage, RightImage);
        momentsRight = cv::moments(RightImage, true);  // momente berechnen bzw. die flaeche fuer jeden Bereich
        m_AmountSplashRightROI[side] = static_cast<int>(momentsRight.m00);
    }

    if (m_ExchangeMemory->m_MeasuringParameter.m_InfoLevel == INFO_LEVEL_SHOW_FULL_INFO || m_ExchangeMemory->m_MeasuringParameter.m_InfoLevel == INFO_LEVEL_SHOW_ONLY_LIQUID_RESULTS) {
        ROIImage.copyTo(CameraImage(ROIRect));
    }
    if (m_ExchangeMemory->m_MeasuringParameter.m_InfoLevel == INFO_LEVEL_SHOW_FULL_INFO || m_ExchangeMemory->m_MeasuringParameter.m_InfoLevel == INFO_LEVEL_SHOW_ONLY_LIQUID_RESULTS) {
        // draw line show middle window
        cv::line(CameraImage, cv::Point(x + MiddleROIRect.x, y), cv::Point(x + MiddleROIRect.x, y + h), cv::Scalar(128));
        cv::line(CameraImage, cv::Point(x + MiddleROIRect.x + LiquidSectorWidthInPixel, y), cv::Point(x + MiddleROIRect.x + LiquidSectorWidthInPixel, y + h), cv::Scalar(128));
    }
}

int ImageData::GetDistanceBetweenValvesHalf()
{
    if (m_ExchangeMemory->m_MeasuringParameter.m_PixelSizeInMMPerPixel > 0)
        return static_cast<int>((m_ExchangeMemory->m_MeasuringParameter.m_DistancesBetweenValves / m_ExchangeMemory->m_MeasuringParameter.m_PixelSizeInMMPerPixel) / 2.0);
    else
        return 50;
}

double ImageData::GetInjectionLenghtInMM()
{
    return m_ExchangeMemory->m_MeasuringParameter.m_BottleneckDiameter / 1.41;  // Wurzel 2
}

bool ImageData::StartMeasuringBottlePosition(MeasuringResults& CurrentMeasuringResults, cv::Mat& CameraImage, std::string& ErrorMsg, int GateXOffset, int x, int y, int w, int h, int ROIIndex)
{
    int EdgeLeftContrast, EdgeRightContrast;
    int YStartDrawPosition;
    int BottelXPos, NewY, NewH;
    double MaxBottleneckDiameterInMM = m_ExchangeMemory->m_MeasuringParameter.m_BottleneckDiameter + m_ExchangeMemory->m_MeasuringParameter.m_BotteleNeckDiameterToleranceInmm;
    double MinBottleneckDiameterInMM = m_ExchangeMemory->m_MeasuringParameter.m_BottleneckDiameter - m_ExchangeMemory->m_MeasuringParameter.m_BotteleNeckDiameterToleranceInmm;
    double ImageBackgroundContrast = m_ExchangeMemory->m_MeasuringParameter.m_ImageBackgroundContrast;
    double MaxContrastValue = 220;
    // double MaxValueBottleneckDiameterInMM = m_ExchangeMemory->m_MeasuringParameter.m_BottleneckDiameter * 1.5;  // Toleranzbereich int geprüft wird ob gemessener Durchmesser ausserhalb dr
    // Toleranz double MinValueBottleneckDiameterInMM = m_ExchangeMemory->m_MeasuringParameter.m_BottleneckDiameter * 0.5;
    double ForegroundGrayValue, ContrastLeftEdge, ContrastRightEdge;
    cv::Point2d LeftEdgeLocation, RightEdgeLocation;
    cv::Mat ROIImage, BottleneckImage;
    std::string Output;

    EdgeLeftContrast = EdgeRightContrast = 0;
    if (m_ExchangeMemory->m_MeasuringParameter.m_BandDirectional == BAND_DIRECTIONAL_LEFT_TO_RIGHT) x = x + GateXOffset;
    w = w - GateXOffset;
    if (w > 10) {
        // m_ExchangeMemory->m_DebugCounter = 924;
        cv::Rect ROIRect(x, y, w, h);
        CopyROIImage(ROIRect, CameraImage, ROIImage);
        EdgeDetection(CurrentMeasuringResults, ROIImage, ContrastLeftEdge, ContrastRightEdge, LeftEdgeLocation, RightEdgeLocation, ROIIndex);
        // m_ExchangeMemory->m_DebugCounter = 928;
        EdgeLeftContrast = static_cast<int>(fabs(static_cast<double>(ContrastLeftEdge)) * THRESHOLD_FACTOR_IN_PERCENT_SOBEL_KERNEL_SIZE_3 + 0.5);
        EdgeRightContrast = static_cast<int>(fabs(static_cast<double>(ContrastRightEdge)) * THRESHOLD_FACTOR_IN_PERCENT_SOBEL_KERNEL_SIZE_3 + 0.5);
        CurrentMeasuringResults.m_EdgeLeftFound = EdgeLeftContrast > m_ExchangeMemory->m_MeasuringParameter.m_EdgeThreshold ? true : false;
        CurrentMeasuringResults.m_EdgeRightFound = EdgeRightContrast > m_ExchangeMemory->m_MeasuringParameter.m_EdgeThreshold ? true : false;

        if (CurrentMeasuringResults.m_EdgeLeftFound) {
            CurrentMeasuringResults.m_ResultEdgeLeftXPos = static_cast<double>(ROIRect.x) + LeftEdgeLocation.x;
            CurrentMeasuringResults.m_EdgeLeftContrastInPercent = EdgeLeftContrast;
        }
        if (CurrentMeasuringResults.m_EdgeRightFound) {
            CurrentMeasuringResults.m_ResultEdgeRightXPos = static_cast<double>(ROIRect.x) + RightEdgeLocation.x;
            CurrentMeasuringResults.m_EdgeRightContrastInPercent = EdgeRightContrast;
        }

        YStartDrawPosition = m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowOffsetYCopy[ROIIndex] + m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowHeightCopy[ROIIndex];
        if (CurrentMeasuringResults.EdgesFound()) {
            CurrentMeasuringResults.m_MeasuredBottleNeckDiameterInmm =
                m_ExchangeMemory->m_MeasuringParameter.m_PixelSizeInMMPerPixel * fabs(CurrentMeasuringResults.m_ResultEdgeRightXPos - CurrentMeasuringResults.m_ResultEdgeLeftXPos);
            if (CurrentMeasuringResults.m_MeasuredBottleNeckDiameterInmm > MaxBottleneckDiameterInMM || CurrentMeasuringResults.m_MeasuredBottleNeckDiameterInmm < MinBottleneckDiameterInMM)
                CurrentMeasuringResults.m_ProductSizeInTolerance = false;
            else
                CurrentMeasuringResults.m_ProductSizeInTolerance = true;
            if (CurrentMeasuringResults.m_ProductSizeInTolerance) {
                int xpos, width = abs(static_cast<int>(RightEdgeLocation.x) - static_cast<int>(LeftEdgeLocation.x));
                // m_ExchangeMemory->m_DebugCounter = 958;
                if (RightEdgeLocation.x < LeftEdgeLocation.x)
                    xpos = static_cast<int>(x + RightEdgeLocation.x + 0.5);
                else
                    xpos = static_cast<int>(x + LeftEdgeLocation.x + 0.5);

                cv::Rect BottleRect(xpos, y, width, ROIImage.rows);
                BottleneckImage = CameraImage(BottleRect);
                ForegroundGrayValue = cv::mean(BottleneckImage).val[0];

                // m_ExchangeMemory->m_DebugCounter = 968;
                CurrentMeasuringResults.m_BottleContrastInPercent = (fabs(ImageBackgroundContrast - ForegroundGrayValue)) * 100.0 / MaxContrastValue;
                if (CurrentMeasuringResults.m_BottleContrastInPercent > 100.0)
                    CurrentMeasuringResults.m_BottleContrastInPercent = 100.0;
                else if (CurrentMeasuringResults.m_BottleContrastInPercent < 0.0)
                    CurrentMeasuringResults.m_BottleContrastInPercent = 0.0;
                if (CurrentMeasuringResults.m_BottleContrastInPercent > m_ExchangeMemory->m_MeasuringParameter.m_EdgeThreshold)
                    CurrentMeasuringResults.m_ProductContrastOk = true;
                else
                    CurrentMeasuringResults.m_ProductContrastOk = false;
                if (CurrentMeasuringResults.m_ProductContrastOk) {
                    CurrentMeasuringResults.m_BottleMatchScoreInPercent =
                        (CurrentMeasuringResults.m_BottleContrastInPercent + CurrentMeasuringResults.m_EdgeLeftContrastInPercent + CurrentMeasuringResults.m_EdgeRightContrastInPercent) / 3.0;
                    CurrentMeasuringResults.m_BottleMiddelPositionInmm =
                        ((CurrentMeasuringResults.m_ResultEdgeLeftXPos + CurrentMeasuringResults.m_ResultEdgeRightXPos) * m_ExchangeMemory->m_MeasuringParameter.m_PixelSizeInMMPerPixel) / 2.0;
                    CurrentMeasuringResults.m_CurrentTimeStampInms = CurrentMeasuringResults.m_CurrentTimeStampInns / ((double)(1000000));
                    CurrentMeasuringResults.m_DriftInmmBottlePosToMeasureWindowMiddlePos =
                        CurrentMeasuringResults.m_BottleMiddelPositionInmm -
                        (m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowOffsetXCopy[ROIIndex] + m_ExchangeMemory->m_MeasuringParameter.m_MeasureWindowWidthCopy[ROIIndex] / 2.0) *
                            m_ExchangeMemory->m_MeasuringParameter.m_PixelSizeInMMPerPixel;
                } else {  // tritt sehr selten auf unter umständen gar nicht
                    ErrorMsg += cv::format("Contrast(%.1f%%) Out Of Tol. Threshold:%d%%\n", CurrentMeasuringResults.m_BottleContrastInPercent, m_ExchangeMemory->m_MeasuringParameter.m_EdgeThreshold);
                    if (ROIIndex == ROI_INDEX_MEASURE_SPEED) CurrentMeasuringResults.m_CounterContrastOutOfTol++;  // nur für GUI Anzeige
                }
                // if (ROIIndex == ROI_INDEX_MEASURE_SPEED) {
                //     m_CounterSizeIsOutOfTol = 0;
                // }
            } else {
                if ((m_ExchangeMemory->m_MeasuringParameter.m_InfoLevel == INFO_LEVEL_SHOW_FULL_INFO || m_ExchangeMemory->m_MeasuringParameter.m_InfoLevel == INFO_LEVEL_SHOW_ONLY_SPEED_RESULTS))
                    ErrorMsg +=
                        cv::format("Size(%.1fmm) Out Of Range(%.1fmm-%.1fmm)\n", CurrentMeasuringResults.m_MeasuredBottleNeckDiameterInmm, MinBottleneckDiameterInMM, MaxBottleneckDiameterInMM);
                if (ROIIndex == ROI_INDEX_MEASURE_SPEED) {  // && CurrentMeasuringResults.m_MeasuredBottleNeckDiameterInmm < MaxValueBottleneckDiameterInMM &&
                                                            // CurrentMeasuringResults.m_MeasuredBottleNeckDiameterInmm > MinValueBottleneckDiameterInMM) {
                    m_CounterSizeIsOutOfTol++;              // erst bei n mal hintereinander annahme productgrösse ist außerhalb der toleranz
                }
            }
        } else {
            if ((m_ExchangeMemory->m_MeasuringParameter.m_InfoLevel == INFO_LEVEL_SHOW_FULL_INFO || m_ExchangeMemory->m_MeasuringParameter.m_InfoLevel == INFO_LEVEL_SHOW_ONLY_SPEED_RESULTS))
                ErrorMsg += cv::format("No Product! EdgeL:%d%% EdgeR:%d%% Thres.:%d%%\n", EdgeLeftContrast, EdgeRightContrast, m_ExchangeMemory->m_MeasuringParameter.m_EdgeThreshold);
            if (ROIIndex == ROI_INDEX_MEASURE_SPEED) {
                if (!CurrentMeasuringResults.m_EdgeRightFound && !CurrentMeasuringResults.m_EdgeLeftFound) {
                    m_CounterEdgeIsOutOfTol++;
                }
            }
        }
        if ((m_ExchangeMemory->m_MeasuringParameter.m_InfoLevel == INFO_LEVEL_SHOW_FULL_INFO || m_ExchangeMemory->m_MeasuringParameter.m_InfoLevel == INFO_LEVEL_SHOW_ONLY_SPEED_RESULTS)) {
            if (CurrentMeasuringResults.ProductFound()) {
                BottelXPos = static_cast<int>((CurrentMeasuringResults.m_ResultEdgeRightXPos + CurrentMeasuringResults.m_ResultEdgeLeftXPos) / 2.0 + 0.5);
                cv::line(CameraImage, cv::Point(BottelXPos, YStartDrawPosition), cv::Point(BottelXPos, YStartDrawPosition + 40), cv::Scalar(255));     // draw arrow
                cv::line(CameraImage, cv::Point(BottelXPos, YStartDrawPosition), cv::Point(BottelXPos - 4, YStartDrawPosition + 4), cv::Scalar(255));  // draw arrow
                cv::line(CameraImage, cv::Point(BottelXPos, YStartDrawPosition), cv::Point(BottelXPos + 4, YStartDrawPosition + 4), cv::Scalar(255));  // draw arrow

                cv::line(CameraImage, cv::Point(static_cast<int>(CurrentMeasuringResults.m_ResultEdgeLeftXPos + 0.5), YStartDrawPosition),
                         cv::Point(static_cast<int>(CurrentMeasuringResults.m_ResultEdgeLeftXPos + 0.5), YStartDrawPosition + 40), cv::Scalar(255));  // draw arrow
                cv::line(CameraImage, cv::Point(static_cast<int>(CurrentMeasuringResults.m_ResultEdgeRightXPos + 0.5), YStartDrawPosition),
                         cv::Point(static_cast<int>(CurrentMeasuringResults.m_ResultEdgeRightXPos + 0.5), YStartDrawPosition + 40), cv::Scalar(255));  // draw arrow
            } else {
                cv::line(CameraImage, cv::Point(static_cast<int>(CurrentMeasuringResults.m_ResultEdgeLeftXPos + 0.5), YStartDrawPosition),
                         cv::Point(static_cast<int>(CurrentMeasuringResults.m_ResultEdgeLeftXPos + 0.5), YStartDrawPosition + 40), cv::Scalar(255));  // draw arrow
                cv::line(CameraImage, cv::Point(static_cast<int>(CurrentMeasuringResults.m_ResultEdgeRightXPos + 0.5), YStartDrawPosition),
                         cv::Point(static_cast<int>(CurrentMeasuringResults.m_ResultEdgeRightXPos + 0.5), YStartDrawPosition + 40), cv::Scalar(180));  // draw arrow
            }
        }
        if (m_ExchangeMemory->m_MeasuringParameter.m_InfoLevel == INFO_LEVEL_SHOW_FULL_INFO || m_ExchangeMemory->m_MeasuringParameter.m_InfoLevel == INFO_LEVEL_SHOW_ONLY_SPEED_RESULTS) {
            if (ROIIndex == ROI_INDEX_MEASURE_SPEED) ROIImage.copyTo(CameraImage(ROIRect));  // copy ROI Image back
        }
    }
    return CurrentMeasuringResults.ProductFound();
}

int ImageData::EdgeDetection(MeasuringResults& CurrentMeasuringResults, cv::Mat& ROIImage, double& ContrastLeftEdge, double& ContrastRightEdge, cv::Point2d& LeftEdgeLocation,
                             cv::Point2d& RightEdgeLocation, int MeasureWindowIndex)
{
    int rv = 0;
    double NewPos;
    double MaxContrastLeftEdge = 0.0;
    double MaxContrastRightEdge = 0.0;
    cv::Mat TempMeanValue(1, ROIImage.cols, CV_32F);
    cv::Mat MeanValue(1, ROIImage.cols, CV_32F);        // buffer Mittelwert normales Bild
    cv::Mat MeanValueGray(1, ROIImage.cols, CV_32F);    // buffer Mittelwert normales Bild
    cv::Mat MeanValue180(1, ROIImage.cols, CV_32F);     // buffer Mittelwert um 180 Grad gedrehtes Bild
    cv::Mat GradientXOrginal(ROIImage.size(), CV_32F);  // das Steigungsbild
    cv::Mat temp;
    cv::Point2d LeftEdgeLocationSubPixel;
    cv::Point2d RightEdgeLocationSubPixel;

    ContrastLeftEdge = ContrastRightEdge = 0.0;

    cv::Sobel(ROIImage, GradientXOrginal, CV_32F, 1, 0, m_ExchangeMemory->m_MeasuringParameter.m_FilterKernelSize);
    cv::reduce(GradientXOrginal, MeanValue, 0, cv::REDUCE_AVG);
    cv::minMaxLoc(MeanValue, &ContrastLeftEdge, &ContrastRightEdge);
    convertScaleAbs(GradientXOrginal, ROIImage);

    NewPos = FindFirstEdge(MeanValue, ContrastLeftEdge, GRADIENT_NEGATIVE);
    LeftEdgeLocation = cv::Point2d(NewPos, 0.0);
    cv::flip(MeanValue, MeanValue180, 1);
    NewPos = FindFirstEdge(MeanValue180, ContrastRightEdge, GRADIENT_POSITIVE);
    RightEdgeLocation = cv::Point2d(static_cast<double>((ROIImage.cols - 1)) - NewPos, 0.0);

    MeanValue = cv::abs(MeanValue);  // nur für die Anzeige damit nur positive werte angezeigt
    rv = MinMaxLocSubPix(&LeftEdgeLocationSubPixel, MeanValue, &LeftEdgeLocation);
    if (rv == 0) LeftEdgeLocation = LeftEdgeLocationSubPixel;
    rv = MinMaxLocSubPix(&RightEdgeLocationSubPixel, MeanValue, &RightEdgeLocation);
    if (rv == 0) RightEdgeLocation = RightEdgeLocationSubPixel;

    if (MeasureWindowIndex == ROI_INDEX_MEASURE_SPEED) {  // hier nur für die Anzege der daten
        // nur für die Profildarstellung des Kantenverlaufs
        ROIImage.convertTo(temp, CV_32F);
        cv::reduce(temp, MeanValueGray, 0, cv::REDUCE_AVG);
        if (MeanValue.cols > MAX_PROFILE_DATA_LENGHT)
            CurrentMeasuringResults.m_SizeProfileData = MAX_PROFILE_DATA_LENGHT;
        else
            CurrentMeasuringResults.m_SizeProfileData = MeanValue.cols;

        CurrentMeasuringResults.m_LeftEdgeLocation = LeftEdgeLocation.x;
        CurrentMeasuringResults.m_RightEdgeLocation = RightEdgeLocation.x;
        for (int c = 0; c < CurrentMeasuringResults.m_SizeProfileData; c++) {
            CurrentMeasuringResults.m_GradientProfileData[c] = static_cast<double>(MeanValue.at<float>(0, c));
            CurrentMeasuringResults.m_RawProfileData[c] = static_cast<double>(MeanValueGray.at<float>(0, c));
        }
    }
    return rv;
}

double ImageData::FindFirstEdge(cv::Mat& MeanValue, double& Contrast, int dir)
{
    double Pos = 0.0;
    float Threshold = 0.5;

    for (int i = 0; i < MeanValue.cols - 1; i++) {
        float value = MeanValue.at<float>(0, i);
        float next = MeanValue.at<float>(0, i + 1);

        if (dir == GRADIENT_NEGATIVE) {
            if (value < Contrast * Threshold && value < next) {
                Pos = static_cast<double>(i);
                Contrast = static_cast<double>(value);
                break;
            }
        } else {
            if (value > Contrast * Threshold && value > next) {
                Pos = static_cast<double>(i);
                Contrast = static_cast<double>(value);
                break;
            }
        }
    }
    return Pos;
}

int ImageData::MinMaxLocSubPix(cv::Point2d* SubPixLoc, cv::Mat& Image, cv::Point2d* LocIn)
{
    int rv = 0;
    cv::Size MaxScan;
    cv::Point ScanRectMin;  // I used two Points instead of a Rect to prevent having Rect compute right/left values in each loop below
    cv::Point ScanRectMax;
    float FloatValueChange = 0.1f;  // FLT_EPSILON * 200.0f; // smallest change that we can do math on with some meaningful result.
    float SrcStartingPoint;
    // results
    cv::Point ScanRight;
    cv::Point ScanLeft;
    cv::Point ScanUp;
    cv::Point ScanDown;
    cv::Point Center = *LocIn;
    cv::Point2d A, B, C;
    cv::Point2d ResultX, ResultY;

    // set default result in case we fail
    SubPixLoc->x = LocIn->x;
    SubPixLoc->y = LocIn->y;
    MaxScan.width = Image.cols >> 4;
    MaxScan.height = Image.rows >> 4;

    ScanRectMin.x = static_cast<int>(LocIn->x) - MaxScan.width;
    if (ScanRectMin.x < 0) ScanRectMin.x = 0;

    ScanRectMin.y = static_cast<int>(LocIn->y) - MaxScan.height;
    if (ScanRectMin.y < 0) ScanRectMin.y = 0;

    ScanRectMax.x = static_cast<int>(LocIn->x) + MaxScan.width;
    if (ScanRectMax.x >= Image.cols) ScanRectMax.x = Image.cols - 1;

    ScanRectMax.y = static_cast<int>(LocIn->y) + MaxScan.height;
    if (ScanRectMax.y >= Image.rows) ScanRectMax.y = Image.rows - 1;

    // scan to find area to use. this can get complicated since we may be given a point near any of the edges of the blob we want to use.
    SrcStartingPoint = Image.at<float>(static_cast<int>(LocIn->y), static_cast<int>(LocIn->x));

    if (Image.cols >= 20) {
        ScanRight = Center;
        while (true) {
            ++ScanRight.x;                               // no point checking the passed location. so inc first
            if (ScanRight.x > ScanRectMax.x) return -1;  // ran out of room to scan
            if (abs(Image.at<float>(ScanRight.y, ScanRight.x) - SrcStartingPoint) > FloatValueChange) break;
        }
        ScanLeft = Center;
        while (true) {
            --ScanLeft.x;                               // no point checking the passed location. so inc first
            if (ScanLeft.x < ScanRectMin.x) return -1;  // ran out of room to scan
            if (abs(Image.at<float>(ScanLeft.y, ScanLeft.x) - SrcStartingPoint) > FloatValueChange) break;
        }
    }
    if (Image.rows >= 20) {
        ScanUp = Center;
        while (true) {
            ++ScanUp.y;                               // assume G cords. The actual direction of Up in the image is not important since the math is symmetrical
            if (ScanUp.y > ScanRectMax.y) return -1;  // ran out of room to scan
            if (abs(Image.at<float>(ScanUp.y, ScanUp.x) - SrcStartingPoint) > FloatValueChange) break;
        }
        ScanDown = Center;
        while (true) {
            --ScanDown.y;                               // assume G cords. The actual direction of Up in the image is not important since the math is symmetrical
            if (ScanDown.y < ScanRectMin.y) return -1;  // ran out of room to scan
            if (abs(Image.at<float>(ScanDown.y, ScanDown.x) - SrcStartingPoint) > FloatValueChange) break;
        }
    }
    if (Image.cols >= 20) {
        // B is highest, A and C are on the sides
        // X axis
        A.x = ScanLeft.x;                               // The pixel cords
        A.y = Image.at<float>(ScanLeft.y, ScanLeft.x);  // the Heat map value

        B.x = Center.x;                             // The pixel cords
        B.y = Image.at<float>(Center.y, Center.x);  // the Heat map value

        C.x = ScanRight.x;                                // The pixel cords
        C.y = Image.at<float>(ScanRight.y, ScanRight.x);  // the Heat map value
        rv = SubPixFitParabola(&ResultX, A, B, C);
        if (rv == 0) {
            // we throw away the y and use the x
            // clip and set error
            if (ResultX.x < ScanLeft.x) {
                ResultX.x = ScanLeft.x;
                // ErrorVal = 1;
            }
            if (ResultX.x > ScanRight.x) {
                ResultX.x = ScanRight.x;
                // ErrorVal = 1;
            }
            SubPixLoc->x = ResultX.x;
        }
    }
    if (Image.rows >= 20) {
        // Y axis
        // this time we swap x and y since the parabola is always found in the x
        A.x = ScanDown.y;                               // The pixel cords
        A.y = Image.at<float>(ScanDown.y, ScanDown.x);  // the Heat map value

        B.x = Center.y;                             // The pixel cords
        B.y = Image.at<float>(Center.y, Center.x);  // the Heat map value

        C.x = ScanUp.y;                             // The pixel cords
        C.y = Image.at<float>(ScanUp.y, ScanUp.x);  // the Heat map value
        rv = SubPixFitParabola(&ResultY, A, B, C);
        if (rv == 0) {
            // we throw away the y and use the x
            ResultY.y = ResultY.x;
            // clip and set error
            if (ResultY.y < ScanDown.y) {
                ResultY.y = ScanDown.y;
                // ErrorVal = 1;
            }
            if (ResultY.y > ScanUp.y) {
                ResultY.y = ScanUp.y;
                // ErrorVal = 1;
            }
            SubPixLoc->y = ResultY.y;
        }
    }
    return rv;
}

int ImageData::SubPixFitParabola(cv::Point2d* Result, cv::Point2d& P1, cv::Point2d& P2, cv::Point2d& P3)
{
    int rv = 0;
    // A x1^2 + B x1 + C = y1
    // A x2^2 + B x2 + C = y2
    // A x3^2 + B x3 + C = y3
    double denom = (P1.x - P2.x) * (P1.x - P3.x) * (P2.x - P3.x);
    if (denom != 0.0) {
        double A = (P3.x * (P2.y - P1.y) + P2.x * (P1.y - P3.y) + P1.x * (P3.y - P2.y)) / denom;
        double B = ((P3.x * P3.x) * (P1.y - P2.y) + (P2.x * P2.x) * (P3.y - P1.y) + (P1.x * P1.x) * (P2.y - P3.y)) / denom;
        double C = (P2.x * P3.x * (P2.x - P3.x) * P1.y + P3.x * P1.x * (P3.x - P1.x) * P2.y + P1.x * P2.x * (P1.x - P2.x) * P3.y) / denom;
        // y = A * x^2 + B * x + C
        // finde maximum
        if (A != 0.0) {
            Result->x = -B / (2 * A);           // erste ableitung null setzen maximum suchen
            Result->y = C - (B * B) / (4 * A);  // dann x in gleichung einsetzen um y zu berechnen
        } else
            rv = -1;
    } else
        rv = -1;
    return rv;
}

void ImageData::CopyROIImage(cv::Rect& ROIRect, cv::Mat& Source, cv::Mat& ROIImage)
{
    cv::Mat ROIImageRef(Source, ROIRect);

    ROIImage = cv::Mat::zeros(ROIRect.height, ROIRect.width, CV_8UC1);
    ROIImageRef.copyTo(ROIImage(cv::Rect(0, 0, ROIRect.width, ROIRect.height)));
}

// does not work
void ImageData::DebugFormat(const char* format, ...)
{
    static char s_printf_buf[512];
    va_list args;
    va_start(args, format);
    _vsnprintf_s(s_printf_buf, sizeof(s_printf_buf), format, args);
    va_end(args);
    OutputDebugStringA(s_printf_buf);
}

// is used for debugging
void ImageData::SetInputOutputInfoData(std::string& InfoString, int InfoCode)
{
    m_ExchangeMemory->m_InfoCodeInputOutputDevice = InfoCode;
    strncpy_s(m_ExchangeMemory->m_BufferInfoText, InfoString.c_str(), sizeof(m_ExchangeMemory->m_BufferInfoText));
    m_ExchangeMemory->m_BufferInfoText[sizeof(m_ExchangeMemory->m_BufferInfoText) - 1] = 0;

    KS_setEvent((KSHandle)(m_ExchangeMemory)->m_HandleEventInputOutput);  // event for GUI App
}

// used for tests
void ImageData::WriteImge(cv::Mat& image, int flag)
{
    std::string PathAndFileName = "d:\\temp\\RealTimeImage1.bmp";
    if (flag == 1) PathAndFileName = "d:\\temp\\RealTimeImage2.bmp";
    cv::imwrite(PathAndFileName, image);
}
