#pragma once

#include <QThread>
#include <Qtcore>
#include "qimage.h"

class PopupProgressBar;
class ProductVideo;
class ProductVideoData : public QThread
{
    Q_OBJECT
  public:
    ProductVideoData(QWidget* parent);
    ~ProductVideoData();
    virtual void run();
    void DisableLiveImageView();
    void FinishedThread();
    ProductVideo* GetProductVideo() { return m_ProductVideo; }
    int GetVideoFramerate() { return m_VideoFramerate; }
    void SetVideoFramerate(int set) { m_VideoFramerate = set; }
    int GetVideoState() { return m_VideoState; }
    void SetVideoState(int set) { m_VideoState = set; }
    void IncrementVideoCounter();
    void DecrementVideoCounter();
    //double GetDisplayZoomFactor() { return m_DisplayZoomFactor; }
    //void SetDisplayZoomFactor(double set) { m_DisplayZoomFactor = set; }
    double GetDisplayZoomFactorWidth() { return m_DisplayZoomFactorWidth;}
    double GetDisplayZoomFactorHeight() { return m_DisplayZoomFactorHeight;}
    void SetDisplayZoomFactorWidth(double set) { m_DisplayZoomFactorWidth=set; }
    void SetDisplayZoomFactorHeight(double set) { m_DisplayZoomFactorHeight=set; }
    void SetEnableLiveImage(bool set) { m_EnableLiveImage = set; }
    int SaveVideo(QString& ErrorMsg);
    void SetVideoFileName(QString& set) { m_VideoFileName = set; };
    void RestartShowVideoThread();
    void DrawIamgeCameraID(QImage& image, unsigned long long ImageCamID);

  public slots:
    void SlotVideoFrameNumberChanged(int);
    void SlotSetProgressBarValue(int);

  signals:
    void SignalShowLiveImage(const QImage& Image);
    void SignalSetCurrentVideoFrameNumber(int);
    void SignalProgressBarValue(int);

  private:
    bool m_ExitThread;
    bool m_EnableLiveImage;
    int m_WaitTimeShowNextVideoFrameInms;
    int m_CurrentVideoFrameNumber;
    int m_VideoState;
    int m_VideoFramerate;
    int m_NumberFramesInVideo;
    QMutex m_WaitLiveImageViewIsDisable;
    QWaitCondition m_WaitConditionLiveViewIsDisable;
    ProductVideo* m_ProductVideo;
    //double m_DisplayZoomFactor;
    double m_DisplayZoomFactorWidth;
    double m_DisplayZoomFactorHeight;
    QImage m_SingleImage;
    QString m_VideoFileName;
    PopupProgressBar* m_PopupProgressBar;
};
