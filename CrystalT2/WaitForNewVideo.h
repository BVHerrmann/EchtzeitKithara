#pragma once

#include <QThread>
#include "ProductVideo.h"
#include "SharedMemoryVideoData.h"

class MainAppCrystalT2;
class VideoHeader;
class MeasuringResultsLiquid;
class WaitForNewVideo : public QThread
{
    Q_OBJECT
  public:
    WaitForNewVideo(MainAppCrystalT2* pMainAppCrystalT2);
    ~WaitForNewVideo();
    virtual void run();
    void StartWaitForNextVideo();
    unsigned char* GetSharedMemoryPointer();
    SharedMemoryVideoData* GetSharedMemoryVideoData() { return m_SharedMemoryVideoData; }
    int AllocateSharedMemoryVideoData(int ImageWidth, int ImageHeight, QString& ErrorMsg);
    MainAppCrystalT2* GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
    bool SpeedCopyVideoFromRealTimeSharedMemoryToApplicationMemory();
    unsigned char* GetImageStartPointer(int ImageIndex);
    unsigned char* GetImageStartPointerByTimeOffset(int StartImageIndex, long long TimeOffset);
    int GetImageBlockSize() { return m_ImageBlockSize; }
    int GetCurrentNumberFrames() { return m_CurrentNumberFrames; }
    int GetImageWidth() { return m_ImageWidth; }
    int GetImageHeight() { return m_ImageHeight; }
    int ReadFramesPerVideo(VideoHeader& Header);
    int GetFramesPerVideo() { return m_FramesPerVideo; }
    int GetVideoStartIndex() { return m_StartVideoIndex; }
    void ShowAllTimeStamps();
    void FinishedThread();
    double GetProductPressentTime() { return m_ProductPressentTime; }
    double GetCurrentSpeedInmmPerms() { return m_CurrentSpeedInmmPerms; }
    int GetVisibileDialogID();
    int GetCurrentVideoTimeOffset(int usedTrigger);
     
 
  signals:
    void SignalShowInfo(const QString& InfoData, int InfoType);
    void SignalShowTriggerImage();
    void SignalOneVideoReady();
    void SignalShowProductVideos();

  public:
    SharedMemoryVideoData* m_SharedMemoryVideoData;
    bool m_ExitThread;
    //bool m_SaveVideoEjectedBottle;
    int m_PollingTimeReadNewVideoData;
    int m_TimeOutValueWaitForNewVideo;
    int m_StartVideoIndex;
    int m_ImageBlockSize;
    int m_BlockSizeTop;
    int m_BlockSizeBottom;
    int m_MaxBlockSize;
    int m_CurrentNumberFrames;
    int m_MaxNumberFrames;
    int m_ImageWidth;
    int m_ImageHeight;
    int m_FramesPerVideo;
    MainAppCrystalT2* m_MainAppCrystalT2;
    QWaitCondition m_WaitForReadyShowImage;
    QMutex m_Mutex;
    double m_ProductPressentTime;
    double m_CurrentSpeedInmmPerms;
   // double m_ImageTimeIntervalInMs;
   // unsigned long long m_LastImageTimeStampInns;
   // unsigned long long m_lastCameraImageID;

    enum ErrorTextAssignment
    {
        NO_SIDE,
        LEFT_TRIGGER_SIDE,
        RIGHT_TRIGGER_SIDE,
        BOTH_TRIGGER_SIDE
    };
};
