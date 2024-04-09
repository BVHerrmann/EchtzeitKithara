#pragma once
#include <qwidget.h>
#include "QtConcurrent/qtconcurrentrun.h"
#include "ui_EjectedBottlesDialog.h"

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"

class MainAppCrystalT2;
class GraphicsVideoView;
class MeasuringResultsLiquid;
class ImageHeader;

class EjectedBottlesDialog : public QWidget
{
    Q_OBJECT
  public:
    EjectedBottlesDialog(MainAppCrystalT2* pMainAppCrystalT2 = Q_NULLPTR);
    ~EjectedBottlesDialog();
    MainAppCrystalT2* GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
    void SetImageLeftTriggerOn(const QImage& Image);
    void SetImageRightTriggerOn(const QImage& Image);
    void LoadEjectedBottlesList();
    void showEvent(QShowEvent* ev);
    int ReadVideo(const QString& VideoFile);
    void StopRunnigThreads();
    void StartWaitForBottlesEjected();
    void CopyVideoDataBottleEject();
    unsigned char* GetImageStartPointer(int ImageIndex);
    unsigned char* GetSharedMemoryPointer();
    unsigned char* GetImageStartPointerByTimeOffset(int StartImageIndexTrigger, long long TimeOffsetInms);
    QList<QPair<QString, int>> GetResultsLiquidMeasuringAsString(MeasuringResultsLiquid& results, int& ErrorType);
    QString GetErrorStringLeftTriggerOn(QList<QPair<QString, int>>& listErrorText);
    QString GetErrorStringRightTriggerOn(QList<QPair<QString, int>>& listErrorText);
    void DrawLiqudWidowIntoErrorImage(const cv::Mat& MatImage, int index, ImageHeader& imageHeader);
    void SaveTriggerdImages();
    void WaitForBottlesEjectedAndCopyData();
    void SetTriggerImagesIntoList(int StartIndex, QList<QPair<QString, int>>& listErrorStrings, const QString& DirName, bool& foundFirst, bool& foundSecond);
    int GetCurrentVideoTimeOffset(int TriggerPos);
    int GetCurrentNumberFrames() { return m_CurrentNumberFrames; }
    void SetAuditTrailProperties();
    void SetRequiredAccessLevel();
    void DeleteDir();
   
  signals:
    void SignalSetVideoImage(const QImage& Image);
    void SignalStartVideoReady();
    void SignalWaitForBottlesEjectedFinished();
    void SignalSaveNewSet();

  public slots:
    void SlotShowSelectedDataSet(QListWidgetItem* item);
    void SlotErrorTypeChanged(int);
    void SlotSetVideoImage(const QImage& Image);
    void SlotStartVideo();
    void SlotSelectShowTriggerImages(int);
    void SlotSelectShowVideo(int);
    void SlotDeleteDir();
    void SlotOpenFileTransferDialog();

  private:
    Ui::EjectedBottlesDialog ui;
    MainAppCrystalT2* m_MainAppCrystalT2;
    GraphicsVideoView* m_GraphicsViewLeftTrigger;
    GraphicsVideoView* m_GraphicsViewRightTrigger;
    QString m_SelectedSubDirName;
    bool m_BreakVideo;
    bool m_BreakStopWaitForBottlesEjected;
    bool m_BreakSaveTriggerImages;
    bool m_ShowVideo;
    bool m_SaveVideoEjectedBottle;
    int m_ImageBlockSize;
    int m_CurrentNumberFrames;
    int m_MaxNumberFrames;
    int m_MaxBlockSize;
    int m_BlockSizeBottom;
    int m_BlockSizeTop;
    int m_StartVideoIndex;
    int m_CurrentNumberFramesInFillingProcess;
    int m_CacheMaximumNumberEjectedBottles;
    double m_CurrentSpeedInmmPerms;
    double m_ImageTimeIntervalInMs;
    QFuture<void> m_FutureRunVideoFillingProcess;
    QFuture<void> m_WaitForBottleEjected;
    QFuture<void> m_WriteTriggerImages;
    QString m_VideoFilename;
    QString m_ImageFileExtension;
    QThread* m_StartVideoThread;
    QList<QPair<cv::Mat, QString>> m_ListSavedTriggerImages;
    QList<QPair<QString, QString>> m_ListSavedErrorText;
    QList<QPair<QVector<cv::Mat>, QString>> m_ListSavedFillingProcessVideos;
    unsigned long long m_LastImageTimeStampInNs;
    enum ErrorTextAssignment { NO_SIDE, LEFT_TRIGGER_SIDE, RIGHT_TRIGGER_SIDE, BOTH_TRIGGER_SIDE };
};
