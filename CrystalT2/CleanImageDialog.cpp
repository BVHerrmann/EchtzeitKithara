#include "CleanImageDialog.h"
#include "ControlsWithColorStatus.h"
#include "GlobalConst.h"
#include "GraphicsVideoView.h"
#include "ImageData.h"
#include "LiveImageView.h"
#include "MainAppCrystalT2.h"
#include "OverviewDialog.h"
#include "ProductData.h"
#include "bmessagebox.h"
#include "settingsdata.h"

#include <audittrail.h>
#include "colors.h"

CleanImageDialog::CleanImageDialog(MainAppCrystalT2* pMainAppCrystalT2) : QWidget(pMainAppCrystalT2), m_MainAppCrystalT2(NULL), m_GraphicsViewCleanImage(NULL), m_ControlDegreeOfPollution(NULL)
{
    ui.setupUi(this);
    int TimerIntervallFlashInMs = 1000;
    m_MainAppCrystalT2 = pMainAppCrystalT2;
    m_GraphicsViewCleanImage = new GraphicsVideoView(this);

    connect(ui.pushButtonSaveCleanImage, &QPushButton::clicked, this, &CleanImageDialog::SlotSaveCleanImage);
    connect(ui.pushButtonEjectISmaschine, &QPushButton::clicked, this, &CleanImageDialog::SlotISMaschineEjectAllBottles);
    if (ui.frameCleanImage->layout()) {
        ui.frameCleanImage->layout()->addWidget(m_GraphicsViewCleanImage);
    }
    bool SmallerAsThreshold = false;
    m_ControlDegreeOfPollution = new ControlsWithColorStatus(ui.doubleSpinBoxDegreeOfPollution, SmallerAsThreshold);
    ui.doubleSpinBoxDegreeOfPollution->setValue(0.0);
    SetAuditTrailProperties();
    SetRequiredAccessLevel();
}

CleanImageDialog::~CleanImageDialog()
{
}

void CleanImageDialog::SetRequiredAccessLevel()
{
    ui.groupBoxSettingsCleanImage->setProperty(kRequiredAccessLevel, kAccessLevelUser);
    // ui.pushButtonEjectISmaschine->setProperty(kRequiredAccessLevel, kAccessLevelUser);
    ui.groupBoxBottleEjectionViaIS->setProperty(kRequiredAccessLevel, kAccessLevelUser);
}

void CleanImageDialog::SetAuditTrailProperties()
{
    ui.pushButtonSaveCleanImage->setProperty(kAuditTrail, ui.pushButtonSaveCleanImage->text());
    ui.pushButtonEjectISmaschine->setProperty(kAuditTrail, ui.pushButtonEjectISmaschine->text());
}

bool CleanImageDialog::event(QEvent* pEvent)
{
    if (pEvent->type() == QEvent::Show) {
        if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetLiveImageView()) {
            AddLiveImageWidget((QWidget*)(GetMainAppCrystalT2()->GetLiveImageView()));
            GetMainAppCrystalT2()->GetLiveImageView()->SetShowPlace(SHOW_PLACE_LIVE_IMAGE_VIEW_CLEAN_IMAGE_DIALOG);
            GetMainAppCrystalT2()->GetLiveImageView()->ClearAllGraphicItem();
            // Verhindert das manuelle verschieben der Messfenster unabhängig welcher Maschinenstatus aktiviert ist
            if (!GetMainAppCrystalT2()->GetLiveImageView()->DisableChangeMeasureWindowPosition()) {
                GetMainAppCrystalT2()->GetLiveImageView()->DrawAllMeasureWindows();
            }
            ChangeButtonColorEjectBottles();
        }
    } else if (pEvent->type() == QEvent::Hide) {
        if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetLiveImageView()) {
            RemoveLiveImageWidget((QWidget*)(GetMainAppCrystalT2()->GetLiveImageView()));
            GetMainAppCrystalT2()->GetLiveImageView()->SetShowPlace(SHOW_PLACE_LIVE_IMAGE_VIEW_EDIT_PRODUCT_DIALOG);  // Ursprungszustand wiederherstellen
            GetMainAppCrystalT2()->GetLiveImageView()->ClearAllGraphicItem();
            GetMainAppCrystalT2()->GetLiveImageView()->SetOnCurrentProductionState();
            GetMainAppCrystalT2()->GetLiveImageView()->DrawAllMeasureWindows();
        }
    }
    return QWidget::event(pEvent);
}

void CleanImageDialog::SetLiveImageSize(int w, int h)
{
    w = w + 8;
    h = h + 8;
    ui.ImageFrame->setMaximumWidth(w);
    ui.ImageFrame->setMaximumHeight(h);
    ui.ImageFrame->setMinimumWidth(w);
    ui.ImageFrame->setMinimumHeight(h);

    ui.frameCleanImage->setMaximumWidth(w);
    ui.frameCleanImage->setMaximumHeight(h);
    ui.frameCleanImage->setMinimumWidth(w);
    ui.frameCleanImage->setMinimumHeight(h);
}

void CleanImageDialog::AddLiveImageWidget(QWidget* w)
{
    if (ui.ImageFrame->layout()) ui.ImageFrame->layout()->addWidget(w);
}

void CleanImageDialog::RemoveLiveImageWidget(QWidget* w)
{
    if (ui.ImageFrame->layout()) ui.ImageFrame->layout()->removeWidget(w);
}

void CleanImageDialog::SetCleanImage(const QImage& Image)
{
    if (m_GraphicsViewCleanImage) m_GraphicsViewCleanImage->setImage(Image, 1.0, 1.0);
}

void CleanImageDialog::SlotSaveCleanImage()
{
    BMessageBox* pMessageBox = new BMessageBox(QMessageBox::Information, tr("Save Reference Image"), tr("Save New Reference Image ?"), this);
    pMessageBox->addButton(QMessageBox::Yes)->setText(tr("Yes"));
    pMessageBox->addButton(QMessageBox::No)->setText(tr("No"));

    if (pMessageBox->exec() != -1) {
        if (pMessageBox->standardButton(pMessageBox->clickedButton()) == QMessageBox::Yes) {
            if (GetMainAppCrystalT2()->GetImageData()) GetMainAppCrystalT2()->GetImageData()->SetSaveCleanImage(true);
        }
    }
    delete pMessageBox;
}

void CleanImageDialog::SlotISMaschineEjectAllBottles()
{
    if (GetMainAppCrystalT2()->IsBlueLightOn()) {
        GetMainAppCrystalT2()->ISMaschineEjectAllBottles(false);
    } else {
        GetMainAppCrystalT2()->ISMaschineEjectAllBottles(true);
    }
    ChangeButtonColorEjectBottles();
}

void CleanImageDialog::ChangeButtonColorEjectBottles()
{
    if (!GetMainAppCrystalT2()->IsBlueLightOn()) {
        ui.pushButtonEjectISmaschine->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::Grey.red()).arg(HMIColor::Grey.green()).arg(HMIColor::Grey.blue()));
        ui.pushButtonEjectISmaschine->setText(tr("Start Ejection"));
    } else {
        ui.pushButtonEjectISmaschine->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(0).arg(0).arg(255));
        ui.pushButtonEjectISmaschine->setText(tr("Stop Ejection"));
    }
}

void CleanImageDialog::SetDegreeOfPollution(double Value)
{
    if (m_ControlDegreeOfPollution && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData() && GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusDegreeOfPolution) {
      
            bool MaschineStopInMin = true;
            double MaschineStopThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_DegreePollutionMaschineStopTimeInSec;
            double WarnThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_DegreePollutionWarningInPercent;
            double AlarmThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_DegreePollutionAlarmInPercent;
            double Default = 0.0;
            m_ControlDegreeOfPollution->SetValueAndAlarmStatus(Value, Value, Default, WarnThreshold, AlarmThreshold, MaschineStopThreshold, MaschineStopInMin);
      
    } else {
        ui.doubleSpinBoxDegreeOfPollution->setValue(Value);
        if (m_ControlDegreeOfPollution) {
            m_ControlDegreeOfPollution->StopTimerColorStatus();
            m_ControlDegreeOfPollution->SetBackgroundColor(HMIColor::ContentBoard);
        }
    }
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetOverviewDialog()) {
        GetMainAppCrystalT2()->GetOverviewDialog()->SetDegreeOfPollution(Value);
    }
}

void CleanImageDialog::SetCreatedDateAndTimeCleanImage(const QString& DateTime)
{
    ui.groupBoxReferenceImage->setTitle(tr("Reference image[Creation date:%1]").arg(DateTime));
}
