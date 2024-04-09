#include "ProductVideo.h"
#include "DisplayVideo.h"
#include "GlobalConst.h"
#include "GraphicsVideoView.h"
#include "ProductVideoData.h"
#include "VideoDialog.h"
#include "VideoHeader.h"

ProductVideo::ProductVideo(int VideoNumber, VideoDialog* parent)
    : QWidget(parent),
      m_GraphicsVideoView(NULL),
      m_ProductVideoData(NULL),
      m_DisplayVideo(NULL),
      m_ImageWidth(0),
      m_ImageHeight(0),
      m_BitsPerPixel(8),
      m_NumberFrames(0),
      m_VideoDialog(NULL),
      m_ActionForAll(true),
      m_StartFrameIndex(-1),
      m_ImageCounter(0),
      m_TimeStampTrigger(0)
{
    m_GraphicsVideoView = new GraphicsVideoView(this, parent);
    m_ProductVideoData = new ProductVideoData(this);
    m_DisplayVideo = new DisplayVideo(this);
    m_DisplayVideo->AddWidget(m_GraphicsVideoView);

    m_VideoDialog = parent;
    connect(m_ProductVideoData, SIGNAL(SignalShowLiveImage(const QImage&)), this, SLOT(SlotShowLiveImage(const QImage&)));
    connect(m_ProductVideoData, SIGNAL(SignalSetCurrentVideoFrameNumber(int)), this, SLOT(SlotSetCurrentVideoFrameNumber(int)));
    connect(m_DisplayVideo, SIGNAL(SignalSliderVideoFrameNumberChanged(int)), m_ProductVideoData, SLOT(SlotVideoFrameNumberChanged(int)));
    connect(m_DisplayVideo, SIGNAL(SignalSliderVideoFramerateChanged(int)), this, SLOT(SlotSliderVideoFramerateChanged(int)));
    connect(m_DisplayVideo, SIGNAL(SignalPlayVideo()), this, SLOT(SlotPlayVideo()));
    connect(m_DisplayVideo, SIGNAL(SignalStopVideo()), this, SLOT(SlotStopVideo()));
    connect(m_DisplayVideo, SIGNAL(SignalSkipBackward()), this, SLOT(SlotSkipBackward()));
    connect(m_DisplayVideo, SIGNAL(SignalSkipForward()), this, SLOT(SlotSkipForward()));
    connect(m_DisplayVideo, SIGNAL(SignalSaveVideo(QString&)), this, SLOT(SlotSaveVideo(QString&)));
}

void ProductVideo::SetEnableMouseEvents(bool set)
{
    if (GetGraphicsVideoView()) GetGraphicsVideoView()->SetEnableMouseEvents(set);
}

void ProductVideo::SetProductVideoSettings(int ImageWidth, int ImageHeight, int BitsPerPixel, int StartFrameIndex, int NumberFrames, unsigned long long TimeStampInnns)
{
    m_ImageWidth = ImageWidth;
    m_ImageHeight = ImageHeight;
    m_BitsPerPixel = BitsPerPixel;
    m_NumberFrames = NumberFrames;
    m_StartFrameIndex = StartFrameIndex;
    m_TimeStampTrigger = TimeStampInnns;
    m_DisplayVideo->SetCurrentFrameLCDNumber(0);
    m_DisplayVideo->SetLCDFramesPerSecond(GetProductVideoData()->GetVideoFramerate());
    m_DisplayVideo->SetFramesPerSecond(GetProductVideoData()->GetVideoFramerate());
}

unsigned char* ProductVideo::GetImageStartPointer(int OffsetIndex)
{
    if (m_StartFrameIndex >= 0)
        return GetVideoDialog()->GetImageStartPointer(m_StartFrameIndex + OffsetIndex);
    else {
        return NULL;
    }
}

unsigned char* ProductVideo::GetImageStartPointerByTimeOffset(long long TimeOffset)
{
    if (GetVideoDialog())
        return GetVideoDialog()->GetImageStartPointerByTimeOffset(m_StartFrameIndex, TimeOffset);
    else
        return NULL;
}

void ProductVideo::ResetStartIndex()
{
    m_StartFrameIndex = -1;
}

void ProductVideo::SlotSaveVideo(QString& PathAndName)
{
    SaveVideo(PathAndName);
}

void ProductVideo::SlotAddNewDebugInfo(const QString& ErrorMsg, int InfoCode)
{
    if (GetVideoDialog()) GetVideoDialog()->EmitAddNewDebugInfo(ErrorMsg, InfoCode);
}

void ProductVideo::SlotSetCurrentVideoFrameNumber(int CurrentVidoFrame)
{
    m_DisplayVideo->SetMaximumVideoSlider(m_NumberFrames);
    m_DisplayVideo->SetCurrentFrameNumber(CurrentVidoFrame);     // set the slider
    m_DisplayVideo->SetCurrentFrameLCDNumber(CurrentVidoFrame);  // set LCD number
}

void ProductVideo::StartImagAcquisition()
{
    if (m_ProductVideoData && !m_ProductVideoData->isRunning()) {
        m_ProductVideoData->RestartShowVideoThread();
    }
}

void ProductVideo::StopImagAcquisition()
{
    if (m_ProductVideoData && m_ProductVideoData->isRunning()) m_ProductVideoData->FinishedThread();
}

int ProductVideo::CopyAndShowSingleImageByTimeOffset(int TimeOffset, QString& ErrorMsg, int TriggerNumber)
{
    QImage Image;
    int rv = ERROR_CODE_NO_ERROR;
    unsigned char* pSharedMemoryImageData = GetImageStartPointerByTimeOffset(TimeOffset);
    if (pSharedMemoryImageData) {
        Image = QImage(pSharedMemoryImageData, GetImageWidth(), GetImageHeight(), QImage::Format_Grayscale8);
        if (GetVideoDialog()) {
            if (GetVideoDialog()->IsRecordingTriggerPosOn()) GetVideoDialog()->RecordingTriggerImages(Image);
        }
        SlotShowLiveImage(Image);
    } else {
        ErrorMsg = tr("Can Not Show Image! TimeOffset:%1ms").arg(TimeOffset);
        rv = ERROR_CODE_ANY_ERROR;
    }
    return rv;
}

void ProductVideo::SlotShowLiveImage(const QImage& Image)
{
    //double ZoomFactor = m_ProductVideoData->GetDisplayZoomFactor();
    double ZoomFactorWidth = m_ProductVideoData->GetDisplayZoomFactorWidth();
    double ZoomFactorHeight = m_ProductVideoData->GetDisplayZoomFactorHeight();
    
    m_GraphicsVideoView->setImage(Image, ZoomFactorWidth, ZoomFactorHeight);
}

void ProductVideo::DrawGreenColorFrame()
{
    if (m_ProductVideoData && m_GraphicsVideoView) {
        m_GraphicsVideoView->DrawGreenColorFrame();
    }
}

void ProductVideo::ClearGreenFrame()
{
    if (m_GraphicsVideoView) m_GraphicsVideoView->ClearGreenFrame();
}

void ProductVideo::HideControls()
{
    m_DisplayVideo->HideControls();
}

void ProductVideo::SlotSliderVideoFramerateChanged(int set)
{
    if (m_ActionForAll)
        GetVideoDialog()->SliderVideoFramerateChangedAllVideos(set);
    else
        SliderVideoFramerateChanged(set);
}

void ProductVideo::SliderVideoFramerateChanged(int set)
{
    if (m_ProductVideoData) m_ProductVideoData->SetVideoFramerate(set);
}

void ProductVideo::SlotPlayVideo()
{
    if (m_ActionForAll)
        GetVideoDialog()->PlayAllVideos();
    else
        PlayVideo();
}

void ProductVideo::PlayVideo()
{
    if (m_ProductVideoData) m_ProductVideoData->SetVideoState(PLAY_VIDEO);
}

void ProductVideo::SaveVideo(QString& FileName)
{
    if (m_ProductVideoData) {
        m_ProductVideoData->SetVideoFileName(FileName);
        m_ProductVideoData->SetVideoState(SAVE_VIDEO);
    }
}

void ProductVideo::SlotStopVideo()
{
    if (m_ActionForAll)
        GetVideoDialog()->StopAllVideos();
    else
        StopVideo();
}

void ProductVideo::StopVideo()
{
    if (m_ProductVideoData) m_ProductVideoData->SetVideoState(STOP_VIDEO);
}

void ProductVideo::SlotSkipForward()
{
    if (m_ActionForAll)
        GetVideoDialog()->SkipForwardAllVideos();
    else
        SkipForward();
}

void ProductVideo::SkipForward()
{
    if (m_ProductVideoData) {
        m_ProductVideoData->SetVideoState(SKIP_FORWARD);
        m_ProductVideoData->IncrementVideoCounter();
    }
}

void ProductVideo::SlotSkipBackward()
{
    if (m_ActionForAll)
        GetVideoDialog()->SkipBackwardAllVideos();
    else
        SkipBackward();
}

void ProductVideo::SkipBackward()
{
    if (m_ProductVideoData) {
        m_ProductVideoData->SetVideoState(SKIP_BACKWARD);
        m_ProductVideoData->DecrementVideoCounter();
    }
}

void ProductVideo::SetDisplayZoomFactorWidth(double set)
{
    if (m_ProductVideoData) {
        m_ProductVideoData->SetDisplayZoomFactorWidth(set);
    }
}

void ProductVideo::SetDisplayZoomFactorHeight(double set)
{
    if (m_ProductVideoData) {
        m_ProductVideoData->SetDisplayZoomFactorHeight(set);
    }
}

ProductVideo::~ProductVideo()
{
    if (m_ProductVideoData) {
        delete m_ProductVideoData;
        m_ProductVideoData = NULL;
    }
    if (m_DisplayVideo) {
        delete m_DisplayVideo;
        m_DisplayVideo = NULL;
    }
}
