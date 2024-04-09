#include "DisplayVideo.h"
#include "ProductVideo.h"
#include "QtWidgets"
#include "SaveVideoDialog.h"
#include "VideoDialog.h"
#include <interfaces.h>
#include <audittrail.h>

DisplayVideo::DisplayVideo(ProductVideo* pProductVideo) : QWidget((QWidget*)(pProductVideo)), m_EnableSliderSetFrameNumber(false)
{
    ui.setupUi(this);

    m_ProductVideo = pProductVideo;
    ui.pushButtonStart->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui.pushButtonStop->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    ui.pushButtonBackward->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    ui.pushButtonForward->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    ui.horizontalSliderVideoFrameRate->setMaximum(50);

    SetAuditTrailProperties();
    SetRequiredAccessLevel();
    connect(ui.horizontalSliderVideoFrameRate, SIGNAL(sliderMoved(int)), this, SLOT(SlotSliderVideoFramerateChanged(int)));
    connect(ui.horizontalSliderCurrentVideoFrame, SIGNAL(sliderMoved(int)), this, SLOT(SlotSliderVideoFrameNumberChanged(int)));
    connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(SlotPlayVideo()));
    connect(ui.pushButtonStop, SIGNAL(clicked()), this, SLOT(SlotStopVideo()));
    connect(ui.pushButtonBackward, SIGNAL(clicked()), this, SLOT(SlotSkipBackward()));
    connect(ui.pushButtonForward, SIGNAL(clicked()), this, SLOT(SlotSkipForward()));
    connect(ui.pushButtonSaveVideo, SIGNAL(clicked()), this, SLOT(SlotSaveVideo()));
    connect(ui.pushButtonGetNewVideo, SIGNAL(clicked()), this, SLOT(SlotGetNewVideo()));
}

DisplayVideo::~DisplayVideo()
{
}

void DisplayVideo::SetRequiredAccessLevel()
{
    ui.pushButtonSaveVideo->setProperty(kRequiredAccessLevel, kAccessLevelUser);
}

void DisplayVideo::SetAuditTrailProperties()
{
    ui.horizontalSliderVideoFrameRate->setProperty(kAuditTrail, tr("Video Frame Rate"));
    ui.horizontalSliderCurrentVideoFrame->setProperty(kAuditTrail, tr("Current Video Frame"));

    ui.pushButtonStart->setProperty(kAuditTrail, tr("Video Start"));
    ui.pushButtonStop->setProperty(kAuditTrail, tr("Video Stop"));
    ui.pushButtonBackward->setProperty(kAuditTrail, tr("Video Skip Backward"));
    ui.pushButtonForward->setProperty(kAuditTrail, tr("Video Skip Forward"));
    ui.pushButtonSaveVideo->setProperty(kAuditTrail, ui.pushButtonSaveVideo->text());
    ui.pushButtonGetNewVideo->setProperty(kAuditTrail, ui.pushButtonGetNewVideo->text());
}

void DisplayVideo::AddWidget(QWidget* ImageView)
{
    if (ui.frameliveImage->layout()) ui.frameliveImage->layout()->addWidget(ImageView);
}

void DisplayVideo::ShowControls()
{
    ui.framePlayerControls->show();
}

void DisplayVideo::HideControls()
{
    ui.framePlayerControls->hide();
}

void DisplayVideo::SetMaximumVideoSlider(int MaxValue)
{
    ui.horizontalSliderCurrentVideoFrame->setMaximum(MaxValue);
}

void DisplayVideo::SetCurrentFrameNumber(int Number)
{
    ui.horizontalSliderCurrentVideoFrame->setValue(Number);
}

void DisplayVideo::SetCurrentFrameLCDNumber(int Number)
{
    ui.lcdNumberCurrentVideoFrameNumber->display(Number + 1);
}

void DisplayVideo::SetLCDFramesPerSecond(int Number)
{
    ui.lcdNumberSetVideoFramesPerSecond->display(Number);
}

void DisplayVideo::SetFramesPerSecond(int Number)
{
    ui.horizontalSliderVideoFrameRate->setValue(Number);
}

void DisplayVideo::SlotSaveVideo()
{
    if (GetProductVideo() && GetProductVideo()->GetVideoDialog()) GetProductVideo()->GetVideoDialog()->OpenSaveVideoDialog();
}

void DisplayVideo::SlotGetNewVideo()
{
    if (GetProductVideo() && GetProductVideo()->GetVideoDialog()) GetProductVideo()->GetVideoDialog()->TriggerGetNewVideoFromRealTimeContext();
}

void DisplayVideo::SlotSliderVideoFrameNumberChanged(int pos)
{
    if (m_EnableSliderSetFrameNumber) {
        emit SignalSliderVideoFrameNumberChanged(pos);
    }
}

void DisplayVideo::SlotSliderVideoFramerateChanged(int pos)
{
    SetLCDFramesPerSecond(pos);
    emit SignalSliderVideoFramerateChanged(pos);
}

void DisplayVideo::SlotPlayVideo()
{
    m_EnableSliderSetFrameNumber = false;
    emit SignalPlayVideo();
}

void DisplayVideo::SlotStopVideo()
{
    m_EnableSliderSetFrameNumber = true;
    emit SignalStopVideo();
}

void DisplayVideo::SlotSkipBackward()
{
    emit SignalSkipBackward();
}

void DisplayVideo::SlotSkipForward()
{
    emit SignalSkipForward();
}
