#include "ProductVideoData.h"
#include "GlobalConst.h"
#include "ProductVideo.h"
#include "VideoHeader.h"

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"

#include "ProgressBar.h"

ProductVideoData::ProductVideoData(QWidget* parent)
    : QThread(parent),
      m_ProductVideo(NULL),
      m_ExitThread(false),
      m_EnableLiveImage(true),
      m_WaitTimeShowNextVideoFrameInms(100),
      m_VideoState(PLAY_VIDEO),
      m_CurrentVideoFrameNumber(0),
      m_VideoFramerate(10),
      m_NumberFramesInVideo(0),
      m_PopupProgressBar(NULL),
      m_DisplayZoomFactorWidth(1.0),
      m_DisplayZoomFactorHeight(1.0)
{
    m_ProductVideo = static_cast<ProductVideo*>(parent);
    m_PopupProgressBar = new PopupProgressBar(parent);
    m_PopupProgressBar->SetMessageBoxSecondTextInfo(tr("Save Video Ready"));

    connect(this, &ProductVideoData::SignalProgressBarValue, m_PopupProgressBar, &PopupProgressBar::SlotSetValue);
}

ProductVideoData::~ProductVideoData()
{
    FinishedThread();
}

void ProductVideoData::RestartShowVideoThread()
{
    if (!isRunning()) {
        m_ExitThread = false;
        m_EnableLiveImage = true;
        start(QThread::IdlePriority);
    }
}

void ProductVideoData::run()
{
    unsigned char* pSharedMemoryImageData = NULL;
    QImage Image;
    QString Msg;

    m_CurrentVideoFrameNumber = 0;
    while (true) {
        if (m_ExitThread) break;
        if (m_EnableLiveImage) {
            m_NumberFramesInVideo = GetProductVideo()->GetNumberFrames();
            if (m_CurrentVideoFrameNumber >= m_NumberFramesInVideo)
                m_CurrentVideoFrameNumber = 0;
            else {
                if (m_CurrentVideoFrameNumber < 0) m_CurrentVideoFrameNumber = m_NumberFramesInVideo - 1;
            }
            pSharedMemoryImageData = GetProductVideo()->GetImageStartPointer(m_CurrentVideoFrameNumber);
            switch (m_VideoState) {
                case PLAY_VIDEO:
                    m_CurrentVideoFrameNumber++;
                    m_WaitTimeShowNextVideoFrameInms = static_cast<int>(1000.0 / m_VideoFramerate);
                    break;
                case STOP_VIDEO:
                    m_WaitTimeShowNextVideoFrameInms = 500;
                    break;
                case SKIP_BACKWARD:
                    m_WaitTimeShowNextVideoFrameInms = 500;
                    break;
                case SKIP_FORWARD:
                    m_WaitTimeShowNextVideoFrameInms = 500;
                    break;
                case SAVE_VIDEO:
                    SaveVideo(Msg);
                    break;
                default:
                    m_CurrentVideoFrameNumber++;
                    m_WaitTimeShowNextVideoFrameInms = static_cast<int>(1000.0 / m_VideoFramerate);
                    break;
            }
            if (pSharedMemoryImageData) {
                ImageHeader imageHeader;
                unsigned long long ImageCameraId = 0;
                unsigned char* CurrentAddress = pSharedMemoryImageData - sizeof(ImageHeader);  // Image address triggerpoint
                if (CurrentAddress) {
                    memcpy(&imageHeader, CurrentAddress, sizeof(ImageHeader));
                }
                emit SignalSetCurrentVideoFrameNumber(m_CurrentVideoFrameNumber);
                Image = QImage(pSharedMemoryImageData, GetProductVideo()->GetImageWidth(), GetProductVideo()->GetImageHeight(), QImage::Format_Grayscale8);  // .scaled(ScaledWith, ScaledHeight);
                emit SignalShowLiveImage(Image);
            } else {
                if (GetProductVideo()) {
                    QString Info = QString("Run Video Zeropointer Num Fr.:%1 Current:%2").arg(m_NumberFramesInVideo).arg(m_CurrentVideoFrameNumber);
                    m_CurrentVideoFrameNumber = 0;
                    GetProductVideo()->SlotAddNewDebugInfo(Info, 0);
                }
            }
        } else {
            m_WaitLiveImageViewIsDisable.lock();
            m_WaitConditionLiveViewIsDisable.wakeAll();  // signal an gui dass livebild anzeige gestoppt ist
            m_WaitLiveImageViewIsDisable.unlock();
        }
        msleep(m_WaitTimeShowNextVideoFrameInms);
    }
}

void ProductVideoData::DrawIamgeCameraID(QImage& image, unsigned long long ImageCamID)
{
    QString StringImageCamID = QString("%1").arg(ImageCamID);
    QPainter Painter(&image);
    QFont font = Painter.font();
    font.setPixelSize(15);
    Painter.setFont(font);
    Painter.setPen(Qt::darkGray);
    Painter.drawText(10, 10, 100, 25, Qt::AlignLeft, StringImageCamID);
    Painter.end();
}

void ProductVideoData::SlotSetProgressBarValue(int Value)
{
    if (m_PopupProgressBar) {
        m_PopupProgressBar->GetProgressBar()->SetValue(Value);
    }
}

int ProductVideoData::SaveVideo(QString& ErrorMsg)
{
    unsigned char* pRawData = NULL;  
    int rv = ERROR_CODE_NO_ERROR;
    bool Abort = false;

    if (!m_VideoFileName.isEmpty()) {
        int NumberFramesInVideo = GetProductVideo()->GetNumberFrames();
        cv::Mat CurrentMatImage;
        cv::VideoWriter video(m_VideoFileName.toLatin1().data(), cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, cv::Size(GetProductVideo()->GetImageWidth(), GetProductVideo()->GetImageHeight()));

        if (m_PopupProgressBar) m_PopupProgressBar->SetLocation(m_VideoFileName);
        emit SignalProgressBarValue(0);
        if (video.isOpened()) {
            for (int i = 0; i < NumberFramesInVideo; i++) {
                pRawData = GetProductVideo()->GetImageStartPointer(i);
                if (pRawData) {
                    CurrentMatImage = cv::Mat(GetProductVideo()->GetImageHeight(), GetProductVideo()->GetImageWidth(), CV_8U, (void*)pRawData, GetProductVideo()->GetImageWidth());
                    cv::Mat colorFrame;
                    cvtColor(CurrentMatImage, colorFrame, cv::COLOR_GRAY2BGR);
                    video.write(colorFrame);
                }
                if (m_PopupProgressBar) {
                    if (m_PopupProgressBar->IsAbort()) {
                        video.release();
                        emit SignalProgressBarValue(100);
                        QFile RemoveFile(m_VideoFileName);
                        RemoveFile.remove();
                        Abort = true;
                    } else {
                        int Value = static_cast<int>((double(i) / NumberFramesInVideo) * 100.0);
                        emit SignalProgressBarValue(Value);
                    }
                }
            }
            if (!Abort) {
                video.release();
                emit SignalProgressBarValue(100);  // bei 100 wird der Dialog geschlossen
            }
        } else {
            ErrorMsg = tr("Can Not Open Video File: %1").arg(m_VideoFileName);
            rv = ERROR_CODE_ANY_ERROR;
        }
    }
    m_VideoState = PLAY_VIDEO;
    return rv;
}

void ProductVideoData::SlotVideoFrameNumberChanged(int pos)
{
    m_CurrentVideoFrameNumber = pos;
    if (m_CurrentVideoFrameNumber >= m_NumberFramesInVideo)
        m_CurrentVideoFrameNumber = 0;
    else {
        if (m_CurrentVideoFrameNumber < 0) m_CurrentVideoFrameNumber = m_NumberFramesInVideo - 1;
    }
}

void ProductVideoData::IncrementVideoCounter()
{
    if (m_CurrentVideoFrameNumber < (m_NumberFramesInVideo - 1))
        m_CurrentVideoFrameNumber++;
    else
        m_CurrentVideoFrameNumber = 0;
}

void ProductVideoData::DecrementVideoCounter()
{
    if (m_CurrentVideoFrameNumber > 0)
        m_CurrentVideoFrameNumber--;
    else
        m_CurrentVideoFrameNumber = m_NumberFramesInVideo - 1;
}

void ProductVideoData::DisableLiveImageView()
{
    if (isRunning()) {                  // thread läuft noch
        if (m_EnableLiveImage) {        // livebildanzeige ist aktiv
            m_EnableLiveImage = false;  // livebildanzeige deaktivieren
            m_WaitLiveImageViewIsDisable.lock();
            m_WaitConditionLiveViewIsDisable.wait(&m_WaitLiveImageViewIsDisable, m_WaitTimeShowNextVideoFrameInms * 2);  // warte bis livebildanzeige beendet
            m_WaitLiveImageViewIsDisable.unlock();
        }
    }
}

void ProductVideoData::FinishedThread()
{
    if (isRunning()) {  // thread läuft noch
        DisableLiveImageView();
        m_ExitThread = true;
        wait(m_WaitTimeShowNextVideoFrameInms * 2);  // warte bis thread beendet
        if (isRunning()) {
            QString Info = QString("Error Thread Is Running");
            GetProductVideo()->SlotAddNewDebugInfo(Info, 0);
        }
    }
}
