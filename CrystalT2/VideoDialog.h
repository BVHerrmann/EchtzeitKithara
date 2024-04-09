#pragma once

#include <QWidget>
#include "ui_VideoDialog.h"

class PopupSaveVideoDialog;
class MainAppCrystalT2;
class ProductVideo;
class SettingsData;
class KitharaCore;
class VideoDialog : public QWidget
{
    Q_OBJECT
  public:
    VideoDialog(MainAppCrystalT2* pMainAppCrystalT2, int VideoType);
    ~VideoDialog();
    MainAppCrystalT2* GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
    PopupSaveVideoDialog* GetPopupSaveVideoDialog() { return m_PopupSaveVideoDialog; }
    int AddVideoOneProductCyclus(QString& ErrorMsg);
    int ShowTriggerImage(QString& ErrorMsg);
    int ShowFullVideo(QString& ErrorMsg);
    int ShowProductVideos(QString& ErrorMsg);
    void showEvent(QShowEvent*);
    void hideEvent(QHideEvent* event);
    void ClearProductVideos();
    void TileSubWindows();
    void CascadeSubWindows();
    void RemoveAllSubWindows();
    void PlayAllVideos();
    void StopAllVideos();
    void SkipForwardAllVideos();
    void SkipBackwardAllVideos();
    void SliderVideoFramerateChangedAllVideos(int set);
    void StartProductVideos();
    void StopProductVideos();
    KitharaCore* GetKitharaCore();
    unsigned char* GetImageStartPointer(int ImageIndex);
    unsigned char* GetImageStartPointerByTimeOffset(int StartImageIndexTrigger, long long TimeOffset);
    void TriggerGetNewVideoFromRealTimeContext();
    void EmitAddNewDebugInfo(const QString& Info, int InfoCode);
    void ShowSubVideos();
    void CalculateDisplayZoomFactor(int CurrentImageWidth, int CurrentImageHeight);
    void WriteLogFile(const QString& data, const QString& FileName);
    void OpenSaveVideoDialog();
    QString GetVideoFileLocaton();
    QString GetTriggerImagesFileLocation();
    void SaveVideo(QString& PathAndName);
    int GetShowImagesOnOverview() { return m_ShowImagesOnOverview; }
    void SetShowImagesOnOverview(int set) { m_ShowImagesOnOverview = set; }
    int GetCurrentVideoTimeOffset(int TriggerNumber);
    void RecordingTriggerImages(QImage& Image);
    bool IsRecordingTriggerPosOn() { return m_RecordingTriggerPosOn; }
    void SetRecordingTriggerPosOn(bool set) { m_RecordingTriggerPosOn = set; }
    void ShowFirstSlider(bool show);
    void ShowSecondSlider(bool show);
    void SetAuditTrailProperties();
    void SetRequiredAccessLevel();
    void PrepareShowProductVideos();

  public slots:
    void SlotShowTriggerImage();
    void SlotOneVideoReady();
    void SlotShowProductVideos();
    void SlotAddNewDebugInfo(const QString& ErrorMsg, int InfoCode);
    void SlotDisplayVideos();
    void SlotSliderVideoPositionLeftValveChanged(int);
    void SlotSliderVideoPositionRightValveChanged(int);
    void SlotSliderTriggerDelayLeftValveChanged(int);
    void SlotSliderTriggerDelayRightValveChanged(int);
    void SlotTriggerRealTimeTaskGetNewVideo();
    void SlotForceEditingFinishedVideoPos();
    void SlotForceEditingFinishedVideoPosSecondValve();
    void SlotForceEditingFinishedFirstTriggerOffset();
    void SlotForceEditingFinishedSecondTriggerOffset();

  signals:
    void SignalShowInfo(const QString& InfoData, int InfoType);
    void SignalDisplayVideos();

  private:
    Ui::VideoDialog ui;
    MainAppCrystalT2* m_MainAppCrystalT2;
    QList<ProductVideo*> m_ProductVideos;
    PopupSaveVideoDialog* m_PopupSaveVideoDialog;
    bool m_WindowsStartUp;
    bool m_RecordingTriggerPosOn;
    int m_ShowImagesOnOverview;
    int m_VideoType;
    int m_MultiTriggerCounter;
    unsigned long long m_TimeToTrackStampInns;
    unsigned long long m_LastImageTimeStampInns;
   
};
