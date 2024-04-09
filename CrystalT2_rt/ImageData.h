#pragma once

#pragma pack(push)
#pragma pack(8)
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#pragma pack(pop)
#include "AveragingProductData.h"
class ExchangeMemory;
class VideoHeader;

class Splash
{
  public:
    Splash() : m_x(0), m_y(0) { init(); }
    ~Splash() = default;
    void update(int interval)
    {
        m_direction += interval * 0.0004;
        double xspeed = m_speed * cos(m_direction);
        double yspeed = m_speed * sin(m_direction);

        m_x += xspeed * interval;
        m_y += yspeed * interval;

        if (m_x < -1 || m_x > 1 || m_y < -1 || m_y > 1) {
            init();
        }

        if (rand() < RAND_MAX / 250) {
            init();
        }
    }

    double m_x;
    double m_y;

  private:
    double m_speed;
    double m_direction;

    void init()
    {
        m_x = 0;
        m_y = 0;
        m_direction = (2 * M_PI * rand()) / RAND_MAX;
        m_speed = (0.015 * rand()) / RAND_MAX;
        m_speed *= m_speed;
    }
};

class SimulatSplashes
{
  public:
    SimulatSplashes() : m_lastTime(0) {}
    ~SimulatSplashes() = default;
    void update(int elapsed)
    {
        for (int i = 0; i < SimulatSplashes::NSplashS; i++) {
            m_pSplashs[i].update(elapsed - m_lastTime);
        }
        m_lastTime = elapsed;
    }
    const static int NSplashS = 1000;

  public:
    Splash m_pSplashs[NSplashS];
    int m_lastTime;
};

class TriggerDataEjection
{
  public:
    TriggerDataEjection()
    {
        m_TimeStamp = 0;
        m_DigitalOutputOnOrOff = true;
    }

  public:
    unsigned long long m_TimeStamp;
    bool m_DigitalOutputOnOrOff;
};

class TriggerDataValve
{
  public:
    TriggerDataValve()
    {
        m_TriggerTimeInms = 0;
        m_DigitalOutputOnOrOff = true;
        m_ChannelNumber = 1;
    }

  public:
    double m_TriggerTimeInms;
    int m_ChannelNumber;
    bool m_DigitalOutputOnOrOff;
};

class ImageHeader;
class ImageData
{
  public:
    ImageData(ExchangeMemory* pExchangeMemory, unsigned char* pSharedMemoryImageBlock, unsigned char* pSharedMemoryVideoBlock, unsigned char* pSharedMemoryCleanImageBlock,
              unsigned char* pSharedMemoryVideoBlockBottlesEjected);
    ~ImageData();

    void SetCurrentTimeStampInNanoSec(unsigned long long set);
    void SetMeasuringTimeInms(double set);  // { m_MeasuringTimeInms = set; }
    bool Execute(cv::Mat& CameraImage);
    void MeasureBottlePositionUnderValve(cv::Mat& CameraImage);
    bool StartMeasuringBottlePosition(MeasuringResults& measuringResults, cv::Mat& CameraImage, std::string& ErrorMsg, int XMeasureWindowOffset, int x, int y, int w, int h, int ROIIndex);
    void CopyROIImage(cv::Rect& ROIRect, cv::Mat& Source, cv::Mat& ROIImage);
    int EdgeDetection(MeasuringResults& CurrentMeasuringResults, cv::Mat& ROIImage, double& minVal, double& maxVal, cv::Point2d& MinLocation, cv::Point2d& MaxLocation, int MeasureWindowIndex);
    double FindFirstEdge(cv::Mat& MeanValue, double& Contrast, int dir);
    int MinMaxLocSubPix(cv::Point2d* SubPixLoc, cv::Mat& Image, cv::Point2d* LocIn);
    int SubPixFitParabola(cv::Point2d* Result, cv::Point2d& P1, cv::Point2d& P2, cv::Point2d& P3);
    unsigned long long CalculateWaitTimeBottleIsOutOfROI(double CenterXPosBottleInMM);
    double GetAverageSpeedInMMPerMs();
    double GetProductPresentTimeInms();
    unsigned long long CalculatProductPressentTime();
    void CalculateTriggeringPointInTime(MeasuringResults& measuringResults, double& TriggerPoint1, double& TriggerPoint2);
    void AddTriggerPointsToTriggerList(double& TriggerPoint1, double& TriggerPoint2);
    std::string MeasuringResultsSpeedToString(MeasuringResults& measuringResults, std::string& ErrorMsg);
    void DrawTextOutput(cv::Mat& Image, std::string& output, int xpos, int ypos);
    int SetVideoAndLiveImage(cv::Mat& CameraImge, bool ProductFound, bool ProductOutofROI, bool FirstTriggerIsSet, bool SecondTriggerIsSet, bool FirstOccurrence, bool BottleEjected,
                             bool LiquiMeasuringReady);
    void AddNewVideoFrame(cv::Mat& CameraImage, bool FirstOccurrence, bool ProductFound, bool FirstTriggerIsSet, bool SecondTriggerIsSet, bool BottleEjected, bool LiquiMeasuringReady);
    void AddNewVideoFrameFillingProcess(cv::Mat& CameraImage, ImageHeader& imageHeader);
    void ClearVideoBuffer();
    bool StartIntegrateLiquidData(unsigned long long TimeStampCPU, std::string& Output, bool set, int side);
    void StartIntegrateLiquidDataFirstTrigger(unsigned long long TimeStampCPU, std::string& Output, bool IsTriggerSet, int side);
    bool StartIntegrateLiquidDataSecondTrigger(unsigned long long TimeStampCPU, std::string& Output, bool IsTriggerSet, int side);
    double GetFrontTriggerPointInTimeInms();
    int GetFrontTriggerChannelNumber();
    unsigned long long GetFrontEjectionPointInTimeInns();
    bool GetFrontEjectionSetOrReset();
    void DeleteFrontTriggerPointInTime();
    void DeleteFrontEjectionPointInTime();
    void SetLeftTriggerIsSet(bool set) { m_LeftTriggerIsSet = set; }
    void SetRightTriggerIsSet(bool set) { m_RightTriggerIsSet = set; }
    double GetTriggerDriftInms() { return m_TriggerDriftInms; }
    double GetTriggerDriftInMM() { return m_TriggerDriftInMM; }
    void SetTriggerDriftInms(double set) { m_TriggerDriftInms = set; }
    void SetTriggerDriftInMM(double set) { m_TriggerDriftInMM = set; }
    bool TriggerSetOrReset();
    void SetImageWidth(unsigned int set) { m_ImageWidth = set; }
    void SetImageHeight(unsigned int set) { m_ImageHeight = set; }
    void DebugFormat(const char* format, ...);
    void SetInputOutputInfoData(std::string& InfoString, int InfoCode);
    void SetExchangeMemory(ExchangeMemory* pExchangeMemory) { m_ExchangeMemory = pExchangeMemory; }
    void GetCurrentTimeStampIn100nsUnits(ExchangeMemory* SharedData, unsigned long long& TimeStamp);
    void CalculateEjectionPointInTime(unsigned long long CurrentTimeStampInns);
    void ApplyEjectionData();
    void AveragedMeasuringResults();
    void StartMeasuringLiqiudAmount(cv::Mat& CameraImage, int x, int y, int w, int h, int side, int ValveDistanceHalf);
    ExchangeMemory* GetExchangeMemory() { return m_ExchangeMemory; }
    void CalculateMeanAndStandardDevationLiquid(std::list<MeasuringResultsLiquid>& m_ListAverageAmountOfLiquid);
    void CheckResetCounters();
    void SimulateDegreeOfPollution(cv::Mat& CameraImage);
    void SimulateLiquidFlow(cv::Mat& CameraImage, int ValveIndex, std::vector<cv::Rect>& liquidRois);
    void SimulateSplashes(cv::Mat& CameraImage, int ValveIndex, int side, std::vector<cv::Rect>& liquidRois);
    int IntegerRand(int iMin, int iMax);
    void WriteImge(cv::Mat& image, int flag);
    bool CheckIsLeftTriggerSet();
    bool CheckIsRightTriggerSet();
    bool CheckResultsLiquidMeasuringAndApplyEjection(std::string& Output);
    bool IntegrateLiquidData(unsigned long long TimeStampCPU, std::string& Output, bool FirstTriggerSet, bool SecondTriggerSet);
    static bool SortByTriggerTime(TriggerDataValve& T1, TriggerDataValve& T2);
    void SetMeasureResultsLiquid(MeasuringResultsLiquid& CurrentMeasuringResultsLiqiud, int side, std::string& Output);
    void SetInjectionTimes(unsigned long long TimeStampInns, MeasuringResultsLiquid& CurrentMeasuringResultsLiqiud);
    double GetInjectionLenghtInMM();
    bool SumUpFirstAndSecondLiquidData(std::string& Output);
    int GetDistanceBetweenValvesHalf();
    void CopyCleanImage();
    double GetSpeedFromISMaschineInMMPerMs();
    bool TheLeftValveIsFilledFirst();
    void DrawDroplet(cv::Mat& SubImage, int ValveIndex);
    bool IntegrateLiquidDataBothTrigger(unsigned long long TimeStampCPU, std::string& StringOutput, bool LeftTriggerIsSet, bool RightTriggerIsSet);
    void GetAllLiquidRects(std::vector<cv::Rect>& liquidRois);
    void MeasuringLiqiud(cv::Mat& CameraImage, std::vector<cv::Rect>& liquidRois);
    bool CheckBottleEjectionConditions();

  private:
    ExchangeMemory* m_ExchangeMemory;
    AveragingProductData m_AverageProductData;                                // class averages product data
    MeasuringResults m_CurrentMeasuringResultsSpeed;                          // current measure value
    MeasuringResults m_CurrentMeasuringResultsTriggerPos[MAX_NUMBER_VALVES];  // current measure value
    MeasuringResults m_AveragedMeasuringResults;                              // results all measuredata per product
    MeasuringResultsLiquid m_CurrentMeasuringResultsLiqiud;                   // Results form first and second trigger
    MeasuringResultsLiquid m_CurrentMeasuringResultsLiqiudFirstTrigger;
    MeasuringResultsLiquid m_CurrentMeasuringResultsLiqiudSecondTrigger;
    std::list<TriggerDataValve> m_ListTriggerPointInTime;  // FIFO Products triggering point in time
    std::list<TriggerDataEjection> m_ListEjectionPointInTime;
    std::list<MeasuringResultsLiquid> m_StackFirstTriggerLiquidData;
    unsigned char* m_SharedMemoryImageBlock;                   // Shared Memory live imge view
    unsigned char* m_SharedMemoryVideoBlock;                   // Shared Memory product video
    unsigned char* m_SharedMemoryCleanImageBlock;              // Shared Memory Clean Image
    unsigned char* m_SharedMemoryVideoBlockFillingProcess;     // Shared Memory Bottle Ejected
    unsigned long long m_LastTimeStampInNanoSec;               // last timestamp from camera
    unsigned long long m_LastTimeStampCleanImageInNanoSec;     // last timestamp from camera
    unsigned long long m_TimeStampProductIsOutOfROIInNanoSec;  // time value product leaves the measure window ROI
    unsigned long long m_InjectionTimeFinished;                // time value product leaves the camera ROI
    unsigned long long m_TimeIntervalLiveImageInNanoSec;       // time value draw frames per scond
    unsigned long long m_TimeIntervalCleanImageInNanoSec;      // time value draw frames per scond
    unsigned long long m_TriggerCounter;                       // counts the trigger events
    unsigned long long m_CounterEdgeIsOutOfTol;
    unsigned long long m_CounterSizeIsOutOfTol;
    unsigned long long m_CounterSimulateSecondTrigger;
    unsigned long long m_CounterSimulateFirstTrigger;
    unsigned long long m_CounterSimulateDegreeOfPollution;
    unsigned long long m_CounterSimulateSplashesLeft;
    unsigned long long m_CounterSimulateSplashesRight;
    unsigned long long m_TriggerCounterFirst, m_TriggerCounterSecond;
    unsigned long long m_CounterImagesNoFirstTriggerOccur;
    unsigned long long m_CameraImageID;
    unsigned long long m_CurrentCounterEjectBottleByUser;
    unsigned int m_MeasureWindowScannSize;
    unsigned int m_ImageWidth;
    unsigned int m_ImageHeight;
    int m_MeasureWindowGatePosInPixel;
    int m_AmountSplashLeftROI[MAX_NUMBER_VALVES];       // Umfang der Spritzer auf der linken Hälfte für jeweils ein Ventil
    int m_AmountSplashRightROI[MAX_NUMBER_VALVES];      // Umfang der Spritzer auf der rechten Hälfte für jeweils ein Ventil
    int m_AmountLiquidMiddleROI[MAX_NUMBER_VALVES];     // Flüssigkeitsmenge die durch das mittlere Fenster hindurch fließt
    int m_InjectionMiddleXPosition[MAX_NUMBER_VALVES];  // XPosition des Mittleren Messfensters
    bool m_MeasureInjectionPos;
    bool m_FirstOccurrenceInROI;  // Zustand der Letzen Messung true = Flasche im Messfenster false Flasch nicht im Messfenster
    bool m_ReadImageFromDisk;
    bool m_LeftTriggerIsSet;
    bool m_RightTriggerIsSet;
    bool m_LiquidMeasuringReady;
    bool m_StartPaintingSimulationLiquidSecondTrigger;
    bool m_StartPaintingSimulationLiquidFirstTrigger;
    double m_TriggerDriftInms;
    double m_TriggerDriftInMM;
    TriggerDataEjection m_EjectionSet, m_EjectionReset;
    bool m_TestVar;
    cv::Mat m_DirtyImage;
    cv::Mat m_OriginalCameraImage;
    std::list<MeasuringResultsLiquid> m_ListAverageAmountOfLiquid;
    SimulatSplashes m_SimulateSplashes;
};
