#pragma once

#include <QThread>
#include <Qtcore>
#include "ImageMetaData.h"
#include "SharedData.h"
#include "SharedMemoryVideoData.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

class MainAppCrystalT2;
class KitharaCore;
class ImageData : public QThread
{
	Q_OBJECT
public:
	ImageData(MainAppCrystalT2 *pMainAppCrystalT2);
	~ImageData();
	virtual void   run();
	MainAppCrystalT2  *GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
	KitharaCore *GetKitharaCore();
	void   SetCurrentProductDataToImageProcessingUnit();
	void   SaveProductData();
	int    InitRealTimeSystem(QString &ErrorMsg);
	int    StartCameraAcquisition();
	int    StopCameraAcquisition();
	bool   IsKitharaDriverOpen();
	void   SetShouldTerminate(bool set) { m_TerminateLiveImageView = set; }
	void   SetRealTimeCameraInitialised(bool set) { m_RealTimeCameraInitialised = set; }
	bool   IsRealTimeCameraInitialised() { return m_RealTimeCameraInitialised; }
	ExchangeMemory *GetKernelParameter() { return &m_KernelParameter; }
	void   DisableLiveImageView();
	void   FinishedThread();
	void   FinishedThreadAndFreeAllAllocations();
	void   WriteQImage(QImage &image);
	int    ReadQImage(QImage &image, QString &ErrorMsg);
	double GetDisplayZoomFactor() { return m_DisplayZoomFactor; }
	void   CalculateDisplayZoomFactor(int &DisplayWidth, int &DisplayHeight);
	int    GetImageDataID()    { return m_ImageDataID; }
	int    GetImageWidth()     { return m_ImageWidth; }
	int    GetImagePitch()     { return m_ImagePitch; }
	int    GetImageHeight()    { return m_ImageHeight; }
	int    GetImageOffsetX()   { return m_ImageOffsetX; }
	int    GetImageOffsetY()   { return m_ImageOffsetY; }
    int    GetMaxImageWidth();
    int    GetMaxImageHeight();
       
	
	double TransformImageXCoorToCartXCoor(double pos);
	double TransformImageYCoorToCartYCoor(double pos);
	double TransformCartXCoorToImageXCoor(double xpos);
	double TransformCartYCoorToImageYCoor(double ypos);

	void   SetReferenceMeasurePositionInPixel(int set);
	void   SetReferenceInjectionPositionInPixel(int set);
    void SetDistanceBetweenMeasurePosAndTriggerPosInPixel(double set);
    QRect  GetMeasureWindowRect(int Key);// { return m_MeasureWindowRects.value(Key); }
	void   SetMeasureWindowRect(int Key, QRect &NewRect);
	QHash<int, QRect>  *GetMeasureWindowRects();
    double MetricToPixel(double MM);
	double PixelToMetric(double Pix);
    void   SetEdgeThreshold(int set);
	int    GetEdgeThreshold();
	void   SetFilterKernelSize(int set);
	int    GetFilterKernelSize();
	void   SetExposureTime(double set);
	void   SetImageBackgroundContrast(int set);
	int    GetImageBackgroundContrast();
	void   SetPollingTimeIOTaskInms(double set);
	double GetPollingTimeIOTaskInms();
	void   SetTimePeriodTriggerOutputOnInms(double set);
	double GetTimePeriodTriggerOutputInms();
	void   SetTimePeriodDigitalOutputOnInms(double set);
	double GetTimePeriodDigitalOutputInms();
	void   SetBotteleNeckDiameterToleranceInmm(double set);
	double GetBotteleNeckDiameterToleranceInmm();
	void   SetCurrentPixelSizeInMMPerPixel(double set);
	double GetCurrentPixelSizeInMMPerPixel();
	void   SetTriggerOffsetInmm(double set,int ValveID);
	void   MirrorMeasureWindows();
	int    GetBandDirectional();
	void   SetBandDirectional(int);
	void   SetTargetBottleneckDiameter(double set);
	double GetTargetBottleneckDiameter();
	void   SetProductWidth(double set);
	double GetProductWidth();
	void TriggerGetNewVideoFromRealTimeContext();
	double GetProductPresentTime(double SpeedInMMPerSecond);
	bool ReadImageFromFile(QString &ErrorMsg);
	void SetClearVideoBuffer();
	void DrawTextOnQImage(QImage &img);
	void SetDefaultPreasure(short set);
    void SetAirCoolingCamera(short set);
    void SetAirCoolingLight(short set);
    void SetAirCoolingValve(short set);
    void SetWaterCoolingDefault(short value);
    void SetWaterCoolingSensor(short value);
	void SetDistanceBottleEjection(double set);
	void SetDistancesBetweenValves(double set);
	double GetMeasuredSpeedInMMPerms() {return m_MeasuredSpeedInMMPerms;}
	bool   GetDigitalInput(EtherCATConfigData::IOChannels Channel);
	void SetAnalogOutputValue(const QString &Name, short Value);
	void SetAnalogInputValue(const QString &ChannelName, short Value);//Nur für tests
	void SetAnalogInputValue2Byte(const QString &ChannelName, short &Value);//Nur für Tests
	void SetVideoStateCameraSimulation(int set);
	void SetInfoLevel(int set);
	int GetInfoLevel();
	int GetDebugCounter();
	void SetAcceptanceThresholdLiquidLeftAndRightROI(int set);
	void SetMinAcceptanceThresholdLiquidMiddleROI(int set);
    void SetMaxAcceptanceThresholdLiquidMiddleROI(int set);
	void ResetAllCounters();
    void ResetCountersBottlesEjectionAndLiquidTooLow();
	void SetInjectonWindowMiddleWidthInMm(double set);
	void SetMinSpeedInMMPerMs(double set);
	void SetThresholdBinaryImageLiquid(int set);
	void SetNumberProductsAverageBottleNeckAndPixelSize(int set);
	void SetMinNumberFoundedInROI(int set);
	void SetRollingMeanValueLiquid(int set);
	void SetMaxMeasurementsProductIsOutOfTol(int set);
	void  SetSaveCleanImage(bool set) { m_SaveCleanImage = set; }
	void  SetCheckCleanImage(bool set) { m_CheckCleanImage = set; }
	QImage GetCleanImageQt() { return m_CleanImageQt; }
	QString GetDateTimeCleanImage() {return m_DateTimeCleanImage;}
    void SetChangeTriggerOutputOrder(bool set);
	void SetUsedTriggerOutput(int Value);
    bool FindBottleTopLine(ImageMetaData &LiveImage);
	void CopyROIImage(cv::Rect &ROIRect, cv::Mat &Source, cv::Mat &ROIImage);
	double FindFirstEdge(cv::Mat &MeanValue, double &Contrast, int dir);
	bool   FindOptimalROIPosition(ImageMetaData &LiveImage, bool BestPosition=true);
	bool   FindOptimalEdgeThreshold(ImageMetaData &LiveImage);
    bool FindOptimalROIPositionEx(ImageMetaData& LiveImage);
	bool IsAutoCalibrateIsOn() { return m_AutoCalibrateIsOn; }
	void SetAutoCalibrateIsOn(bool set, bool AbortByUser=false);
    void GetContrastAndPosLeftRightEdge(cv::Mat &ROIImage, double &ContrastLeftEdge, double &ContrastRightEdge, double &LeftPos, double &RightPos);
	double GetAverageCalibrationData();
	int MinMaxLocSubPix(cv::Point2d* SubPixLoc, cv::Mat &Image, cv::Point2d* LocIn);
	int SubPixFitParabola(cv::Point2d* Result, cv::Point2d& P1, cv::Point2d& P2, cv::Point2d& P3);
	void AutoCalibrateSystem(ImageMetaData &LiveImage);
    void CheckIsMeasureWindowInRange(QRect& NewRect, int key);
    void SetDoNotChangeBlueWindow(bool set) { m_DoNotChangeBlueWindow = set; };
	QRect GetMeasureWindowCheckCleanImage();
    void SetFormatFromISInmm(double set);
    void SetUseSpeedFromISCalcEjectionTime(bool set);
    void SetButtonIsClickedEjectTheNextnBottles();
    void SetNumberEjectedBottlesByUser(int set);
    int SetCameraViewPort(int offsetX, int offsetY, int width, int height, QString& ErrorMsg);
    int SetCameraXOffset(int newOffsetX, QString& ErrorMsg);
    int SetCameraYOffset(int newOffsetY, QString& ErrorMsg);
    bool IsCameraResolutionGreaterThanDefaultResolution();
    int GetCounterNumberBottlesRealEjected();
    void SetMixerOn(bool on);
    void SetMixerVelocity(int velocity);
    void SetActualMixerVelocity(ushort set);
    void SetENCStatusMixerSimulation(ushort set);
    void SetStatusMixerSimulation(ushort set);

signals:
	void SignalShowLiveImage(const ImageMetaData &Image);
	void SignalShowInfo(const QString &InfoData, QtMsgType MsgType);
	void SignalGenerateNewVideoFile();
	void SignalShowCleanImage(const QImage &Image);
	void SignalShowDegreeOfPollution(const double Value);
	void SignalSetDateTimeCleanImageIsSaved(const QString &DateTime);
	void SignalSetCalibratetMeasureWindowSpeed(const QRect &NewWindow);
	void SignalSetCalibrateStatus(const QString &Info, int value);
	void SignalShowMessageBoxResultsCalibration();
    void SignalDrawAutocalibrateROI(const QRect &rect);

public slots:
    void SlotSetCalibratetMeasureWindowSpeed(const QRect &NewWindow);
	void SlotShowMessageBoxResultsCalibration();
    void SlotDrawAutocalibrateROI(const QRect &rect);

private:
	MainAppCrystalT2       *m_MainAppCrystalT2;                      //Parent/GUI class
	ExchangeMemory          m_KernelParameter;                 //Measuringparamter for real time task
	bool                    m_RealTimeCameraInitialised;                 //Enable/Disable Live Image View
	bool                    m_TerminateLiveImageView;          //finished the run function/live image view
	bool                    m_SaveCleanImage;
	bool                    m_EnableAddNewImageIntoSharedMemory;
	bool                    m_CheckCleanImage;
	bool                    m_AutoCalibrateIsOn;
	bool                    m_AutoCalibrateBottleTopLineFound;
	bool                    m_DoNotChangeBlueWindow;
	QMutex                  m_WaitLiveImageViewIsDisable;     
	QWaitCondition          m_WaitConditionLiveViewIsDisable;  //Wait condition live thread is finished
	QImage                  m_ImageFromFile;
	double                  m_DisplayZoomFactor;
	double                  m_MaxEdgeContrastAutoCalibrate;
	double                  m_BottelNeckDiameterTolBeforeCalibrating;
	int                     m_NumberFramesPerInterval;
	int                     m_ImageDataID;
	int                     m_ImageWidth;           //camera ROI image width
	int                     m_ImagePitch;           //camera ROI image pitch
	int                     m_ImageHeight;          //camera ROI image height 
	int                     m_ImageOffsetX;         //camera ROI offset x
	int                     m_ImageOffsetY;         //camera ROI offset y
	unsigned __int64        m_LastCameraImageCounter;
	int                     m_CurrentStepAutoCalibrate;
	int                     m_CounterAutoCalibrate;
	int                     m_InfoLevel;
	double                  m_MeasuredSpeedInMMPerms;
	double                  m_CalculatetPixelSize, m_CalculatetEdgeThreshold,m_LastEdgeThreshold;
	unsigned long long      m_ImageCounter;
	int                     m_ImageFileCounter;
	QString                 m_LastErrorMsg;
	QString                 m_DateTimeCleanImage;
	cv::Mat                 m_CleanImageOpenCV;
	QImage                  m_CleanImageQt;
	QList<double>           m_ListCalibrateData1;
    QList<double>           m_ListCalibrateData2;
	QRect                   m_MeasureWindowSpeedBeforeCalibrating;
};
