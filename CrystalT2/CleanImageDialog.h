#pragma once
#include <qwidget.h>

#include <qwidget.h>
#include "ui_CleanImageDialog.h"

class MainAppCrystalT2;
class GraphicsVideoView;
class ControlsWithColorStatus;
class CleanImageDialog : public QWidget
{
    Q_OBJECT
  public:
    CleanImageDialog(MainAppCrystalT2* pMainAppCrystalT2 = Q_NULLPTR);
    ~CleanImageDialog();
    void AddLiveImageWidget(QWidget* w);
    void RemoveLiveImageWidget(QWidget* w);
    bool event(QEvent* pEvent);
    MainAppCrystalT2* GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
    void SetCleanImage(const QImage& Image);
    void SetDegreeOfPollution(double Value);
    void SetCreatedDateAndTimeCleanImage(const QString& DateTime);
    void SetLiveImageSize(int w, int h);
    void SetAuditTrailProperties();
    void SetRequiredAccessLevel();
    void ChangeButtonColorEjectBottles();
    
  public slots:
    void SlotSaveCleanImage();
    void SlotISMaschineEjectAllBottles();
   
  private:
    Ui::CleanImageDialog ui;
    MainAppCrystalT2* m_MainAppCrystalT2;
    GraphicsVideoView* m_GraphicsViewCleanImage;
    ControlsWithColorStatus* m_ControlDegreeOfPollution;
};
