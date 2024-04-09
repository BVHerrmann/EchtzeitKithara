#include "VideoDialog.h"
#include "DisplayVideo.h"
#include "Globalconst.h"
#include "InputMessageDialog.h"
#include "MainAppCrystalT2.h"
#include "ProductData.h"
#include "QMdiSubWindow.h"
#include "SaveVideoDialog.h"
#include "SettingsData.h"
#include "SharedData.h"
#include "VideoHeader.h"
#include "WaitForNewVideo.h"
#include "bmessagebox.h"

#include <audittrail.h>

VideoDialog::VideoDialog(MainAppCrystalT2* pMainAppCrystalT2, int VideoType)
    : QWidget(pMainAppCrystalT2),
      m_WindowsStartUp(false),
      m_PopupSaveVideoDialog(NULL),
      m_TimeToTrackStampInns(0),
      m_MultiTriggerCounter(0),
      m_LastImageTimeStampInns(0),
      m_ShowImagesOnOverview(SHOW_ON_OVERVIEW_LEFT_TRIGGER),
      m_RecordingTriggerPosOn(false)
{
    ui.setupUi(this);
    m_MainAppCrystalT2 = pMainAppCrystalT2;
    m_VideoType = VideoType;

    connect(this, &VideoDialog::SignalShowInfo, this, &VideoDialog::SlotAddNewDebugInfo);
    connect(this, &VideoDialog::SignalDisplayVideos, this, &VideoDialog::SlotDisplayVideos);

    if (m_VideoType == VIDEO_DIALOG_SHOW_TRIGGER_POSITION) {
        connect(GetMainAppCrystalT2()->GetWaitForNewVideo(), &WaitForNewVideo::SignalShowTriggerImage, this, &VideoDialog::SlotShowTriggerImage);
    }
    if (m_VideoType == VIDEO_DIALOG_SHOW_FULL_VIDEO) {
        connect(GetMainAppCrystalT2()->GetWaitForNewVideo(), &WaitForNewVideo::SignalOneVideoReady, this, &VideoDialog::SlotOneVideoReady);
    }
    if (m_VideoType == VIDEO_DIALOG_SHOW_PRODUCT_VIDEO) {
        connect(GetMainAppCrystalT2()->GetWaitForNewVideo(), &WaitForNewVideo::SignalShowProductVideos, this, &VideoDialog::SlotShowProductVideos);
        ui.mdiAreaVideoViewer->setBackground(QBrush(QColor(255, 255, 255)));
        // m_PopupSaveVideoDialog = new PopupSaveVideoDialog(this);
    }

    connect(GetMainAppCrystalT2()->GetWaitForNewVideo(), &WaitForNewVideo::SignalShowInfo, this, &VideoDialog::SlotAddNewDebugInfo);
    connect(ui.horizontalSliderCurrentVideoPositionLeftValve, &QSlider::valueChanged, this, &VideoDialog::SlotSliderVideoPositionLeftValveChanged);
    connect(ui.horizontalSliderCurrentVideoPositionRightValve, &QSlider::valueChanged, this, &VideoDialog::SlotSliderVideoPositionRightValveChanged);
    connect(ui.horizontalSliderTriggerDelayLeftValve, &QSlider::valueChanged, this, &VideoDialog::SlotSliderTriggerDelayLeftValveChanged);

    ui.pushButtonBackwardCurrentVideoPositionLeftValve->hide();
    ui.pushButtonForwardCurrentVideoPositionLeftValve->hide();

    ui.pushButtonBackwardCurrentVideoPositionRightValve->hide();
    ui.pushButtonForwardCurrentVideoPositionRightValve->hide();

    ui.pushButtonBackwardTriggerDelayLeftValve->hide();
    ui.pushButtonForwardTriggerDelayLeftValve->hide();

    ui.pushButtonBackwardTriggerDelayRightValve->hide();
    ui.pushButtonForwardTriggerDelayRightValve->hide();

    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
    if (pSettingsData->m_WorkWithTwoValves && pSettingsData->m_WorkWithSecondTriggerSlider) {
        ui.widgetSliderTriggerDelayRightValve->show();
        connect(ui.horizontalSliderTriggerDelayRightValve, &QSlider::valueChanged, this, &VideoDialog::SlotSliderTriggerDelayRightValveChanged);
    } else
        ui.widgetSliderTriggerDelayRightValve->hide();
    SetAuditTrailProperties();
    SetRequiredAccessLevel();
}

VideoDialog::~VideoDialog()
{
    StopProductVideos();
    RemoveAllSubWindows();
    ClearProductVideos();
}

void VideoDialog::SetRequiredAccessLevel()
{
    ui.horizontalSliderCurrentVideoPositionLeftValve->setProperty(kRequiredAccessLevel, kAccessLevelUser);
    ui.horizontalSliderCurrentVideoPositionRightValve->setProperty(kRequiredAccessLevel, kAccessLevelUser);
    ui.horizontalSliderTriggerDelayLeftValve->setProperty(kRequiredAccessLevel, kAccessLevelUser);
    ui.horizontalSliderTriggerDelayRightValve->setProperty(kRequiredAccessLevel, kAccessLevelUser);
}

void VideoDialog::SetAuditTrailProperties()
{
}

void VideoDialog::ShowFirstSlider(bool show)
{
    if (show) {
        ui.widgetSliderTriggerDelayLeftValve->show();
        ui.widgetSliderVideoPositionLeftValve->show();
    } else {
        ui.widgetSliderTriggerDelayLeftValve->hide();
        ui.widgetSliderVideoPositionLeftValve->hide();
    }
}

void VideoDialog::ShowSecondSlider(bool show)
{
    if (show) {
        ui.widgetSliderTriggerDelayRightValve->show();
        ui.widgetSliderVideoPositionRightValve->show();
    } else {
        ui.widgetSliderTriggerDelayRightValve->hide();
        ui.widgetSliderVideoPositionRightValve->hide();
    }
}

void VideoDialog::OpenSaveVideoDialog()
{
    if (GetPopupSaveVideoDialog() == NULL) {
        m_PopupSaveVideoDialog = new PopupSaveVideoDialog(m_MainAppCrystalT2,this);
    }
    m_PopupSaveVideoDialog->show();
}

// wird nur auf gerufen wenn m_VideoType == VIDEO_DIALOG_SHOW_FULL_VIDEO
void VideoDialog::SaveVideo(QString& PathAndName)
{
    if (m_ProductVideos.count() > 0) {
        m_ProductVideos.at(0)->SaveVideo(PathAndName);
    }
}

QString VideoDialog::GetTriggerImagesFileLocation()
{
    return GetMainAppCrystalT2()->GetTriggerImagesFileLocation();
}

QString VideoDialog::GetVideoFileLocaton()
{
    return GetMainAppCrystalT2()->GetVideoFileLocation();
}

void VideoDialog::WriteLogFile(const QString& data, const QString& FileName)
{
    if (GetMainAppCrystalT2()) GetMainAppCrystalT2()->WriteLogFile(data, FileName);
}

void VideoDialog::EmitAddNewDebugInfo(const QString& Info, int InfoCode)
{
    emit SignalShowInfo(Info, InfoCode);
}

void VideoDialog::SlotSliderVideoPositionLeftValveChanged(int pos)
{
    double dPos = (double)(pos)*VIDEO_POSITION_SLIDER_FACTOR;
    ui.doubleSpinBoxCurrentVideoPositionLeftValve->setValue(dPos);  // lcdNumberCurrentVideoPosition->display(pos);
    ProductData* pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
    if (pProductData) {
        QString ErrorMsg;
        pProductData->m_VideoPositionLeftValveInmm = dPos;
        pProductData->WriteProductData(ErrorMsg);
    }
    QTimer::singleShot(2500, this, &VideoDialog::SlotForceEditingFinishedVideoPos);
}

void VideoDialog::SlotSliderVideoPositionRightValveChanged(int pos)
{
    double dPos = (double)(pos)*VIDEO_POSITION_SLIDER_FACTOR;
    ui.doubleSpinBoxCurrentVideoPositionRightValve->setValue(dPos);  // lcdNumberCurrentVideoPosition->display(pos);
    ProductData* pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
    if (pProductData) {
        QString ErrorMsg;
        pProductData->m_VideoPositionRightValveInmm = dPos;
        pProductData->WriteProductData(ErrorMsg);
    }
    QTimer::singleShot(2500, this, &VideoDialog::SlotForceEditingFinishedVideoPosSecondValve);
}

void VideoDialog::SlotSliderTriggerDelayLeftValveChanged(int Offset)
{
    double OffsetInmm = (double)(Offset)*TRIGGER_SLIDER_FACTOR;
    if (GetMainAppCrystalT2()) GetMainAppCrystalT2()->SetTriggerOffsetInmm(OffsetInmm, LEFT_VALVE_ID);
    ui.doubleSpinBoxTriggerDelayLeftValve->setValue(OffsetInmm);
    QTimer::singleShot(2500, this, &VideoDialog::SlotForceEditingFinishedFirstTriggerOffset);
}

void VideoDialog::SlotSliderTriggerDelayRightValveChanged(int Offset)
{
    double OffsetInmm = (double)(Offset)*TRIGGER_SLIDER_FACTOR;
    if (GetMainAppCrystalT2()) GetMainAppCrystalT2()->SetTriggerOffsetInmm(OffsetInmm, RIGHT_VALVE_ID);
    ui.doubleSpinBoxTriggerDelayRightValve->setValue(OffsetInmm);
    QTimer::singleShot(2500, this, &VideoDialog::SlotForceEditingFinishedSecondTriggerOffset);
}

void VideoDialog::SlotForceEditingFinishedVideoPosSecondValve()
{
    ui.doubleSpinBoxCurrentVideoPositionRightValve->setFocus();
    ui.doubleSpinBoxCurrentVideoPositionRightValve->clearFocus();  // Audi Trail, force editingFinished
}

void VideoDialog::SlotForceEditingFinishedVideoPos()
{
    ui.doubleSpinBoxCurrentVideoPositionLeftValve->setFocus();
    ui.doubleSpinBoxCurrentVideoPositionLeftValve->clearFocus();  // Audi Trail, force editingFinished
}

void VideoDialog::SlotForceEditingFinishedFirstTriggerOffset()
{
    ui.doubleSpinBoxTriggerDelayLeftValve->setFocus();
    ui.doubleSpinBoxTriggerDelayLeftValve->clearFocus();  // Audi Trail, force editingFinished
}

void VideoDialog::SlotForceEditingFinishedSecondTriggerOffset()
{
    ui.doubleSpinBoxTriggerDelayRightValve->setFocus();    // Audi Trail
    ui.doubleSpinBoxTriggerDelayRightValve->clearFocus();  // Audi Trail, force editingFinished
}

void VideoDialog::SlotAddNewDebugInfo(const QString& ErrorMsg, int InfoCode)
{
    if (GetMainAppCrystalT2()) GetMainAppCrystalT2()->SlotAddNewDebugInfo(ErrorMsg, InfoCode);
}

KitharaCore* VideoDialog::GetKitharaCore()
{
    if (GetMainAppCrystalT2()->GetKitharaCore())
        return GetMainAppCrystalT2()->GetKitharaCore();
    else
        return NULL;
}

void VideoDialog::TriggerGetNewVideoFromRealTimeContext()
{
    if (GetMainAppCrystalT2()) GetMainAppCrystalT2()->TriggerGetNewVideoFromRealTimeContext();
}

void VideoDialog::hideEvent(QHideEvent* event)
{
    if (m_VideoType == VIDEO_DIALOG_SHOW_FULL_VIDEO) {
        StopProductVideos();
    }
}

void VideoDialog::showEvent(QShowEvent*)
{
    m_WindowsStartUp = false;
    ProductData* pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
    if (pProductData) {
        if (m_VideoType == VIDEO_DIALOG_SHOW_FULL_VIDEO) {
            ui.widgetSliderVideoPositionLeftValve->hide();
            ui.widgetSliderVideoPositionRightValve->hide();
            ui.widgetSliderTriggerDelayLeftValve->hide();
            ui.widgetSliderTriggerDelayRightValve->hide();
            GetMainAppCrystalT2()->ShowOptionPanelImageTab(false);           // wird hier nicht benötigt
            GetMainAppCrystalT2()->TriggerGetNewVideoFromRealTimeContext();  // neues video anfordern wenn dialog geöffnet
        } else {
            if (m_VideoType == VIDEO_DIALOG_SHOW_TRIGGER_POSITION) {
                ui.widgetSliderVideoPositionLeftValve->show();
                ui.widgetSliderVideoPositionRightValve->show();
                if (pProductData->m_UsedTriggerOutputs == USE_BOTH_VALVES) {
                    SetShowImagesOnOverview(SHOW_ON_OVERVIEW_LEFT_TRIGGER);
                    ShowFirstSlider(true);
                    ShowSecondSlider(true);
                } else {
                    if (pProductData->m_UsedTriggerOutputs == USE_ONLY_LEFT_VALVE) {
                        SetShowImagesOnOverview(SHOW_ON_OVERVIEW_LEFT_TRIGGER);
                        ShowFirstSlider(true);
                        ShowSecondSlider(false);
                    } else {
                        if (pProductData->m_UsedTriggerOutputs == USE_ONLY_RIGHT_VALVE) {
                            SetShowImagesOnOverview(SHOW_ON_OVERVIEW_RIGHT_TRIGGER);
                            ShowFirstSlider(false);
                            ShowSecondSlider(true);
                        }
                    }
                }
                GetMainAppCrystalT2()->ShowOptionPanelImageTab(false);
            } else {
                if (m_VideoType == VIDEO_DIALOG_SHOW_PRODUCT_VIDEO) {
                    ui.widgetSliderVideoPositionLeftValve->hide();
                    ui.widgetSliderVideoPositionRightValve->hide();
                    ui.widgetSliderTriggerDelayLeftValve->hide();
                    ui.widgetSliderTriggerDelayRightValve->hide();
                    GetMainAppCrystalT2()->ShowOptionPanelImageTab(true);
                }
            }
        }
        ui.doubleSpinBoxTriggerDelayLeftValve->setValue(pProductData->m_TriggerOffsetFirstValveInmm);
        ui.horizontalSliderTriggerDelayLeftValve->setValue(pProductData->m_TriggerOffsetFirstValveInmm / TRIGGER_SLIDER_FACTOR);

        ui.doubleSpinBoxTriggerDelayRightValve->setValue(pProductData->m_TriggerOffsetSecondValveInmm);
        ui.horizontalSliderTriggerDelayRightValve->setValue(pProductData->m_TriggerOffsetSecondValveInmm / TRIGGER_SLIDER_FACTOR);

        ui.doubleSpinBoxCurrentVideoPositionLeftValve->setValue(pProductData->m_VideoPositionLeftValveInmm);
        ui.horizontalSliderCurrentVideoPositionLeftValve->setValue(static_cast<int>(pProductData->m_VideoPositionLeftValveInmm / VIDEO_POSITION_SLIDER_FACTOR));

        ui.doubleSpinBoxCurrentVideoPositionRightValve->setValue(pProductData->m_VideoPositionRightValveInmm);
        ui.horizontalSliderCurrentVideoPositionRightValve->setValue(static_cast<int>(pProductData->m_VideoPositionRightValveInmm / VIDEO_POSITION_SLIDER_FACTOR));
    }
    m_WindowsStartUp = true;
}

void VideoDialog::SlotTriggerRealTimeTaskGetNewVideo()
{
    TriggerGetNewVideoFromRealTimeContext();  // beim Sichtbarwerden des Dialogs neues Video anfordern
}

void VideoDialog::ClearProductVideos()
{
    for (int i = 0; i < m_ProductVideos.count(); i++) {
        delete m_ProductVideos.at(i);
    }
    m_ProductVideos.clear();
}

void VideoDialog::SlotDisplayVideos()
{
    for (int i = (m_ProductVideos.count() - 1); i >= 0; i--) {
        DisplayVideo* pDisplayVideo = m_ProductVideos.at(i)->GetDisplayVideo();
        QMdiSubWindow* sub = ui.mdiAreaVideoViewer->addSubWindow((QWidget*)(pDisplayVideo));
        sub->setWindowFlags(Qt::FramelessWindowHint);
        sub->show();
    }
    TileSubWindows();
}

void VideoDialog::CalculateDisplayZoomFactor(int CurrentImageWidth, int CurrentImageHeight)
{
    double ZoomFactor = 1.0;
    int widthArea = ui.mdiAreaVideoViewer->width();
    int heighArea = ui.mdiAreaVideoViewer->height();
    int NewImageWidth, NewImageHeight;
    double ZoomFactorWidth, ZoomFactorHeight;
    double Factor;

    if (m_ProductVideos.count() > 0 && CurrentImageWidth > 0 && CurrentImageHeight > 0) {
        NewImageWidth = CurrentImageWidth;
        NewImageHeight = CurrentImageHeight;
        Factor = (double)(CurrentImageWidth) / CurrentImageHeight;
        switch (m_ProductVideos.count()) {
            case 1:
                break;
            case 2:
                break;
            case 4:
                NewImageWidth = (double)(widthArea) / 2;
                NewImageHeight = (double)(heighArea) / 2;
                break;
            case 6:
                NewImageWidth = (double)(widthArea) / 3;
                NewImageHeight = (NewImageWidth / Factor);
                break;
            case 8:
                NewImageWidth = (double)(widthArea) / 4;
                NewImageHeight = (NewImageWidth / Factor);
                break;
            case 12:
                NewImageWidth = (double)(widthArea) / 4;
                NewImageHeight = (NewImageWidth / Factor);
                break;
            case 16:
                NewImageWidth = (double)(widthArea) / 4.0;
                NewImageHeight = (NewImageWidth / Factor);
                if ((NewImageHeight * 4) > heighArea) {
                    double diff = (NewImageHeight * 4.0) - heighArea;
                    NewImageHeight = NewImageHeight - diff / 4.0;
                    NewImageWidth = NewImageHeight * Factor;
                }
                break;
            case 20:
                NewImageWidth = (double)(widthArea) / 5;
                NewImageHeight = (NewImageWidth / Factor);
                break;
            case 25:
                NewImageWidth = (double)(widthArea) / 5;
                NewImageHeight = (NewImageWidth / Factor);
                if ((NewImageHeight * 5.0) > heighArea) {
                    double diff = (NewImageHeight * 5.0) - heighArea;
                    NewImageHeight = NewImageHeight - diff / 5.0;
                    NewImageWidth = NewImageHeight * Factor;
                }
                break;
            case 36:
                NewImageWidth = (double)(widthArea) / 6;
                NewImageHeight = (NewImageWidth / Factor);
            case 42:
                NewImageWidth = (double)(widthArea) / 7;
                NewImageHeight = (NewImageWidth / Factor);
                break;
            default:
                break;
        }
        ZoomFactorWidth = (double)(NewImageWidth) / CurrentImageWidth;
        ZoomFactorHeight = (double)(NewImageHeight) / CurrentImageHeight;
        for (int i = 0; i < m_ProductVideos.count(); i++) {
            m_ProductVideos.at(i)->SetDisplayZoomFactorWidth(ZoomFactorWidth);
            m_ProductVideos.at(i)->SetDisplayZoomFactorHeight(ZoomFactorHeight);
        }
    }
}

void VideoDialog::ShowSubVideos()
{
    for (int i = 0; i < m_ProductVideos.count(); i++) {
        DisplayVideo* pDisplayVideo = m_ProductVideos.at(i)->GetDisplayVideo();
        QMdiSubWindow* sub = ui.mdiAreaVideoViewer->addSubWindow((QWidget*)(pDisplayVideo));
        if (!sub->isVisible()) {
            sub->setWindowFlags(Qt::FramelessWindowHint);
            sub->show();
        }
    }
}

void VideoDialog::StartProductVideos()
{
    for (int i = 0; i < m_ProductVideos.count(); i++) m_ProductVideos.at(i)->StartImagAcquisition();  // starten bildanzeige
}

void VideoDialog::StopProductVideos()
{
    for (int i = 0; i < m_ProductVideos.count(); i++) m_ProductVideos.at(i)->StopImagAcquisition();  // stop bildanzeige
}

unsigned char* VideoDialog::GetImageStartPointer(int ImageIndex)
{
    if (GetMainAppCrystalT2()->GetWaitForNewVideo())
        return GetMainAppCrystalT2()->GetWaitForNewVideo()->GetImageStartPointer(ImageIndex);
    else
        return NULL;
}

unsigned char* VideoDialog::GetImageStartPointerByTimeOffset(int StartImageIndexTrigger, long long TimeOffset)
{
    if (GetMainAppCrystalT2()->GetWaitForNewVideo())
        return GetMainAppCrystalT2()->GetWaitForNewVideo()->GetImageStartPointerByTimeOffset(StartImageIndexTrigger, TimeOffset);
    else
        return NULL;
}

void VideoDialog::PrepareShowProductVideos()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetWaitForNewVideo()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        int MaxTriggerImagesOnScreen = 16;
        ProductVideo* pProductVideo = NULL;

        if (pSettingsData) MaxTriggerImagesOnScreen = pSettingsData->m_MaxTriggerImagesOnScreen;
        if (MaxTriggerImagesOnScreen != m_ProductVideos.count()) {
            RemoveAllSubWindows();
            ClearProductVideos();
            for (int i = 0; i < MaxTriggerImagesOnScreen; i++) {
                pProductVideo = new ProductVideo(i + 1, this);
                pProductVideo->SetEnableMouseEvents(true);
                m_ProductVideos.append(pProductVideo);
                pProductVideo->HideControls();
            }
            m_MultiTriggerCounter = 0;
            m_LastImageTimeStampInns = 0;
            CalculateDisplayZoomFactor(GetMainAppCrystalT2()->GetWaitForNewVideo()->GetImageWidth(), GetMainAppCrystalT2()->GetWaitForNewVideo()->GetImageHeight());
            emit SignalDisplayVideos();
        }
    }
}

int VideoDialog::ShowProductVideos(QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    int NumberImages = 1;
    int CurrentNumberFrames = GetMainAppCrystalT2()->GetWaitForNewVideo()->GetCurrentNumberFrames();
    unsigned char* pImageStartPointer = NULL;
    ImageHeader imageHeader;
    unsigned char TriggerFlag;
    int TimeOffsetInms = 0;
    int VideoPosType = LEFT_VIDEO_POS;

    PrepareShowProductVideos();
    if (GetShowImagesOnOverview() == SHOW_ON_OVERVIEW_RIGHT_TRIGGER) VideoPosType = RIGHT_VIDEO_POS;

    TimeOffsetInms = GetCurrentVideoTimeOffset(VideoPosType);
    for (int i = 0; i < m_ProductVideos.count(); i++) m_ProductVideos.at(i)->ResetStartIndex();  // Startindex intern auf -1 setzen
    for (int StartIndex = 0; StartIndex < GetMainAppCrystalT2()->GetWaitForNewVideo()->GetCurrentNumberFrames(); StartIndex++) {
        pImageStartPointer =
            GetMainAppCrystalT2()->GetWaitForNewVideo()->GetImageStartPointer(StartIndex) - sizeof(ImageHeader);  // pSharedMemoryImageData + sizeof(VideoHeader) + (ImageBlockSize * i);
        if (pImageStartPointer) {
            memcpy(&imageHeader, pImageStartPointer, sizeof(ImageHeader));
            if (GetShowImagesOnOverview() == SHOW_ON_OVERVIEW_LEFT_TRIGGER)
                TriggerFlag = IMAGE_CAPTURE_TYPE_LEFT_TRIGGER_IS_SET;
            else
                TriggerFlag = IMAGE_CAPTURE_TYPE_RIGHT_TRIGGER_IS_SET;
            if (imageHeader.m_ImageState & TriggerFlag)  // trigger pos
            {
                if (imageHeader.m_TimeStampInns > m_LastImageTimeStampInns) {
                    if (m_ProductVideos.count() > m_MultiTriggerCounter) {
                        m_ProductVideos.at(m_MultiTriggerCounter)
                            ->SetProductVideoSettings(GetMainAppCrystalT2()->GetWaitForNewVideo()->GetImageWidth(), GetMainAppCrystalT2()->GetWaitForNewVideo()->GetImageHeight(), 8, StartIndex,
                                                      NumberImages, imageHeader.m_TimeStampInns);
                        rv = m_ProductVideos.at(m_MultiTriggerCounter)->CopyAndShowSingleImageByTimeOffset(TimeOffsetInms, ErrorMsg, VideoPosType);  // Show single image
                        if (rv == ERROR_CODE_NO_ERROR) {                                                                                             // Kopieren war erfolgreich
                            m_ProductVideos.at(m_MultiTriggerCounter)->DrawGreenColorFrame();
                            if (m_MultiTriggerCounter > 0)
                                m_ProductVideos.at(m_MultiTriggerCounter - 1)->ClearGreenFrame();  // Lösche letzten Rahmen
                            else
                                m_ProductVideos.at(m_ProductVideos.count() - 1)->ClearGreenFrame();
                            m_LastImageTimeStampInns = imageHeader.m_TimeStampInns;
                            m_MultiTriggerCounter++;
                        }
                    }
                    if (m_MultiTriggerCounter >= m_ProductVideos.count()) m_MultiTriggerCounter = 0;
                }
            }
        }
    }
    return rv;
}

int VideoDialog::ShowTriggerImage(QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    int k = 0;
    int MaxProductsOnScreen = 1;
    int NumberImages = 1;
    int TimeOffsetLeftVideoPosInms = 0;
    int TimeOffsetRightVideoPosInms = 0;
    int UsedTrigger = GetMainAppCrystalT2()->GetUsedTriggerOutput();  // GetUsedTrigger();
    int NumberFrames = GetMainAppCrystalT2()->GetWaitForNewVideo()->GetCurrentNumberFrames();
    ImageHeader imageHeader;
    ProductVideo* pProductVideo;
    unsigned char* pImageStartPointer;
    bool firstTriggerFound = false;
    bool secondTriggerFound = false;
    unsigned char TriggerFlag;
    QString Info;

    if (UsedTrigger == USE_BOTH_VALVES) MaxProductsOnScreen = 2;

    if (NumberFrames > 0) {
        TimeOffsetLeftVideoPosInms = GetCurrentVideoTimeOffset(LEFT_VIDEO_POS);
        TimeOffsetRightVideoPosInms = GetCurrentVideoTimeOffset(RIGHT_VIDEO_POS);
        if (MaxProductsOnScreen != m_ProductVideos.count()) {
            RemoveAllSubWindows();
            ClearProductVideos();
            for (int i = 0; i < MaxProductsOnScreen; i++) {
                pProductVideo = new ProductVideo(i + 1, this);
                m_ProductVideos.append(pProductVideo);
                pProductVideo->HideControls();
            }
            m_LastImageTimeStampInns = 0;
            emit SignalDisplayVideos();
        }

        for (int i = 0; i < m_ProductVideos.count(); i++) m_ProductVideos.at(i)->ResetStartIndex();  // Startindex intern auf -1 setzen
        for (int StartIndex = 0; StartIndex < GetMainAppCrystalT2()->GetWaitForNewVideo()->GetCurrentNumberFrames(); StartIndex++) {
            pImageStartPointer = GetMainAppCrystalT2()->GetWaitForNewVideo()->GetImageStartPointer(StartIndex) - sizeof(ImageHeader);
            if (pImageStartPointer) {
                memcpy(&imageHeader, pImageStartPointer, sizeof(ImageHeader));
                if (UsedTrigger == USE_BOTH_VALVES || UsedTrigger == USE_ONLY_LEFT_VALVE) {
                    if (imageHeader.m_ImageState & IMAGE_CAPTURE_TYPE_LEFT_TRIGGER_IS_SET && !firstTriggerFound)  // trigger pos
                    {
                        firstTriggerFound = true;
                        if (imageHeader.m_TimeStampInns > m_LastImageTimeStampInns) {  // Nur aktuelle Triggerposition anzeigen
                            if (m_ProductVideos.count() > k) {
                                m_ProductVideos.at(k)->SetProductVideoSettings(GetMainAppCrystalT2()->GetWaitForNewVideo()->GetImageWidth(),
                                                                               GetMainAppCrystalT2()->GetWaitForNewVideo()->GetImageHeight(), 8, StartIndex, NumberImages, imageHeader.m_TimeStampInns);
                                rv = m_ProductVideos.at(k)->CopyAndShowSingleImageByTimeOffset(TimeOffsetLeftVideoPosInms, ErrorMsg, LEFT_VIDEO_POS);  // Show single image
                                if (rv == ERROR_CODE_NO_ERROR) {                                                                                       // Kopieren war erfolgreich
                                    k++;
                                    m_LastImageTimeStampInns = imageHeader.m_TimeStampInns;
                                }
                            }
                            if (k >= m_ProductVideos.count()) break;
                        }
                    }
                }
                if ((UsedTrigger == USE_BOTH_VALVES) && k > 0) {                                                    // Suche nach zweiter Position wenn Erste gefunden
                    if (imageHeader.m_ImageState & IMAGE_CAPTURE_TYPE_RIGHT_TRIGGER_IS_SET && !secondTriggerFound)  // trigger pos
                    {
                        secondTriggerFound = true;
                        if (imageHeader.m_TimeStampInns > m_LastImageTimeStampInns) {  // Nur aktuelle Triggerposition anzeigen
                            if (m_ProductVideos.count() > k) {
                                m_ProductVideos.at(k)->SetProductVideoSettings(GetMainAppCrystalT2()->GetWaitForNewVideo()->GetImageWidth(),
                                                                               GetMainAppCrystalT2()->GetWaitForNewVideo()->GetImageHeight(), 8, StartIndex, NumberImages, imageHeader.m_TimeStampInns);
                                rv = m_ProductVideos.at(k)->CopyAndShowSingleImageByTimeOffset(TimeOffsetRightVideoPosInms, ErrorMsg, RIGHT_VIDEO_POS);  // Show single image
                                if (rv == ERROR_CODE_NO_ERROR) {                                                                                         // Kopieren war erfolgreich
                                    k++;
                                    m_LastImageTimeStampInns = imageHeader.m_TimeStampInns;
                                }
                            }
                            if (k >= m_ProductVideos.count()) break;
                        }
                    }
                } else {
                    if (UsedTrigger == USE_ONLY_RIGHT_VALVE) {
                        if (imageHeader.m_ImageState & IMAGE_CAPTURE_TYPE_RIGHT_TRIGGER_IS_SET && !secondTriggerFound)  // trigger pos
                        {
                            secondTriggerFound = true;
                            if (imageHeader.m_TimeStampInns > m_LastImageTimeStampInns) {  // Nur aktuelle Triggerposition anzeigen
                                if (m_ProductVideos.count() > k) {
                                    m_ProductVideos.at(k)->SetProductVideoSettings(GetMainAppCrystalT2()->GetWaitForNewVideo()->GetImageWidth(),
                                                                                   GetMainAppCrystalT2()->GetWaitForNewVideo()->GetImageHeight(), 8, StartIndex, NumberImages,
                                                                                   imageHeader.m_TimeStampInns);
                                    rv = m_ProductVideos.at(k)->CopyAndShowSingleImageByTimeOffset(TimeOffsetRightVideoPosInms, ErrorMsg, RIGHT_VIDEO_POS);  // Show single image
                                    if (rv == ERROR_CODE_NO_ERROR) {                                                                                         // Kopieren war erfolgreich
                                        k++;
                                        m_LastImageTimeStampInns = imageHeader.m_TimeStampInns;
                                    }
                                }
                                if (k >= m_ProductVideos.count()) break;
                            }
                        }
                    }
                }
            }
        }
        if (k == 0) {
            ErrorMsg = tr("No Products In Video Buffer!");
            rv = ERROR_CODE_ANY_ERROR;
        }
    }
    return rv;
}

int VideoDialog::GetCurrentVideoTimeOffset(int TriggerPos)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetWaitForNewVideo()) {
        return GetMainAppCrystalT2()->GetWaitForNewVideo()->GetCurrentVideoTimeOffset(TriggerPos);
    } else {
        return 0;
    }
}

void VideoDialog::RecordingTriggerImages(QImage& Image)
{
    QString Location = GetMainAppCrystalT2()->GetTriggerImagesFileLocation();
    QString CurrentDate = QDateTime::currentDateTime().date().toString();
    QString CurrentTime = QDateTime::currentDateTime().time().toString("hh mm ss zzz");
    QString FileName;
    QString PathAndName;

    FileName = QString("Image[%1 %2]").arg(CurrentDate).arg(CurrentTime);
    PathAndName = Location + QString("/") + FileName + ".bmp";
    Image.save(PathAndName);
}

int VideoDialog::ShowFullVideo(QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    unsigned char* pVideoStartPointer;
    ProductVideo* pProductVideo;
    int StartIndex = 0;
    int MaxProductsOnScreen = 1;

    if (MaxProductsOnScreen != m_ProductVideos.count()) {
        RemoveAllSubWindows();
        ClearProductVideos();
        for (int i = 0; i < MaxProductsOnScreen; i++) {
            pProductVideo = new ProductVideo(i + 1, this);
            m_ProductVideos.append(pProductVideo);
        }
        emit SignalDisplayVideos();
    }
    if (GetMainAppCrystalT2()->GetWaitForNewVideo()->GetCurrentNumberFrames() > 0) {
        pVideoStartPointer = GetMainAppCrystalT2()->GetWaitForNewVideo()->GetImageStartPointer(StartIndex);  // Zeiger auf das erste bild
        if (pVideoStartPointer && m_ProductVideos.count() > 0) {
            m_ProductVideos.at(0)->SetProductVideoSettings(GetMainAppCrystalT2()->GetWaitForNewVideo()->GetImageWidth(), GetMainAppCrystalT2()->GetWaitForNewVideo()->GetImageHeight(), 8, StartIndex,
                                                           GetMainAppCrystalT2()->GetWaitForNewVideo()->GetCurrentNumberFrames(), 0);
            m_ProductVideos.at(0)->StartImagAcquisition();
        }
    } else {
        ErrorMsg = tr("No Images In Video Buffer!");
        rv = ERROR_CODE_ANY_ERROR;
    }
    return rv;
}

int VideoDialog::AddVideoOneProductCyclus(QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    VideoHeader videoHeader;
    ImageHeader imageHeader;
    ProductVideo* pProductVideo;
    unsigned char* pSharedMemoryImageData = GetMainAppCrystalT2()->GetWaitForNewVideo()->GetSharedMemoryPointer();
    int k = 0;
    int MaxProducts = 1;
    QString DebugInfo;

    if (pSharedMemoryImageData) {
        memcpy(&videoHeader, pSharedMemoryImageData, sizeof(VideoHeader));  // read video header
        double TimeInms = (videoHeader.m_CurrentTimeStampInns - videoHeader.m_LastTimeStampInns) / ((double)(1000000.0));
        double FPS = 1000.0 / TimeInms;
        double ProductPresentTimeInSec = (videoHeader.m_ProductPressentTime / 1000.0);
        int FramesPerVideo = static_cast<int>(FPS * ProductPresentTimeInSec);

        for (int StartIndex = 0; StartIndex < GetMainAppCrystalT2()->GetWaitForNewVideo()->GetCurrentNumberFrames(); StartIndex++) {
            unsigned char* pImageStartPointer =
                GetMainAppCrystalT2()->GetWaitForNewVideo()->GetImageStartPointer(StartIndex) - sizeof(ImageHeader);  // pSharedMemoryImageData + sizeof(VideoHeader) + (ImageBlockSize * i);
            if (pImageStartPointer) {
                memcpy(&imageHeader, pImageStartPointer, sizeof(ImageHeader));
                if (imageHeader.m_ImageState & 0x02)  // trigger pos
                {
                    if ((videoHeader.m_CurrentNumberFrames - StartIndex) >= FramesPerVideo) {  // Video ist vollständig enthalten
                        pProductVideo = new ProductVideo(++k, this);
                        pProductVideo->SetProductVideoSettings(videoHeader.m_ImageWidth, videoHeader.m_ImageHeight, videoHeader.m_BitsPerPixel, StartIndex, FramesPerVideo, 0);
                        m_ProductVideos.append(pProductVideo);
                        if (m_ProductVideos.count() >= MaxProducts) return rv;
                    }
                }
            } else {
                QString Info = QString("ImageStartPointer Is Zero. StartIndex:%1 Number Frames:%2").arg(StartIndex).arg(GetMainAppCrystalT2()->GetWaitForNewVideo()->GetCurrentNumberFrames());
                SignalShowInfo(Info, 0);
            }
        }
        if (m_ProductVideos.count() == 0) {
            ErrorMsg = tr("No Images In Video Buffer!");
            rv = ERROR_CODE_ANY_ERROR;
        }
    }
    return rv;
}

void VideoDialog::PlayAllVideos()
{
    for (int i = 0; i < m_ProductVideos.count(); i++) {
        m_ProductVideos.at(i)->PlayVideo();
    }
}

void VideoDialog::StopAllVideos()
{
    for (int i = 0; i < m_ProductVideos.count(); i++) {
        m_ProductVideos.at(i)->StopVideo();
    }
}

void VideoDialog::SkipForwardAllVideos()
{
    for (int i = 0; i < m_ProductVideos.count(); i++) {
        m_ProductVideos.at(i)->SkipForward();
    }
}

void VideoDialog::SkipBackwardAllVideos()
{
    for (int i = 0; i < m_ProductVideos.count(); i++) {
        m_ProductVideos.at(i)->SkipBackward();
    }
}

void VideoDialog::SliderVideoFramerateChangedAllVideos(int set)
{
    for (int i = 0; i < m_ProductVideos.count(); i++) {
        m_ProductVideos.at(i)->SliderVideoFramerateChanged(set);
    }
}

void VideoDialog::RemoveAllSubWindows()
{
    QList<QMdiSubWindow*> ListMDIWindow = ui.mdiAreaVideoViewer->subWindowList();  // WindowOrder order = CreationOrder) const

    while (ListMDIWindow.count() > 0) {
        ui.mdiAreaVideoViewer->removeSubWindow((QWidget*)(ListMDIWindow.at(0)));
        ListMDIWindow = ui.mdiAreaVideoViewer->subWindowList();
    }
}

void VideoDialog::TileSubWindows()
{
    ui.mdiAreaVideoViewer->setViewMode(QMdiArea::SubWindowView);
    ui.mdiAreaVideoViewer->tileSubWindows();
}

void VideoDialog::CascadeSubWindows()
{
    ui.mdiAreaVideoViewer->setViewMode(QMdiArea::TabbedView);
    ui.mdiAreaVideoViewer->cascadeSubWindows();

    QList<QMdiSubWindow*> ListMDIWindow = ui.mdiAreaVideoViewer->subWindowList();
    for (int i = 0; i < ListMDIWindow.count(); i++) {
        ListMDIWindow.at(i)->widget()->showMaximized();
    }
    QTabBar* Bar = ui.mdiAreaVideoViewer->findChild<QTabBar*>();
    if (Bar) {
        Bar->setExpanding(false);
    }
}

void VideoDialog::SlotShowProductVideos()
{
    QString ErrorMsg;
    int rv;

    if (GetShowImagesOnOverview() == SHOW_ON_OVERVIEW_BOTH_TRIGGER)
        rv = ShowTriggerImage(ErrorMsg);
    else
        rv = ShowProductVideos(ErrorMsg);
    if (rv != ERROR_CODE_NO_ERROR) SignalShowInfo(ErrorMsg, 0);
}

void VideoDialog::SlotShowTriggerImage()
{
    QString ErrorMsg;

    int rv = ShowTriggerImage(ErrorMsg);
    if (rv != ERROR_CODE_NO_ERROR) SignalShowInfo(ErrorMsg, 0);
}

void VideoDialog::SlotOneVideoReady()
{
    QString ErrorMsg;
    ShowFullVideo(ErrorMsg);
    StartProductVideos();
}
