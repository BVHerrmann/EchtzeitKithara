#include "SaveVideoDialog.h"
#include "MainAppCrystalT2.h"
#include "VideoDialog.h"
#include "bmessagebox.h"
#include "interfaces.h"
#include "qfileinfo.h"

#include <audittrail.h>

SaveVideoDialog::SaveVideoDialog(MainAppCrystalT2* pMainAppCrystalT2, QWidget* parent)
    : QDialog((QWidget*)pMainAppCrystalT2), m_DefaultFileName("Video1"), m_DefaultFileExtention(".avi"), m_SaveVideoFile(true)
{
    ui.setupUi(this);
    m_MainAppCrystalT2 = pMainAppCrystalT2;
    m_PopupSaveVideoDialog = (PopupSaveVideoDialog*)parent;
    ui.lineEditInputDataVideoLocation->setReadOnly(true);
    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &SaveVideoDialog::SlotOkPressed);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &SaveVideoDialog::SlotCancelPressed);

    if (ui.buttonBox->button(QDialogButtonBox::Yes)) ui.buttonBox->button(QDialogButtonBox::Yes)->setText(tr("Yes"));
    if (ui.buttonBox->button(QDialogButtonBox::No)) ui.buttonBox->button(QDialogButtonBox::No)->setText(tr("No"));
    SetRequiredAccessLevel();
}

SaveVideoDialog::~SaveVideoDialog()
{
}

void SaveVideoDialog::SetRequiredAccessLevel()
{
    ui.buttonBox->button(QDialogButtonBox::Yes)->setProperty(kRequiredAccessLevel, kAccessLevelUser);
    ui.buttonBox->button(QDialogButtonBox::No)->setProperty(kRequiredAccessLevel, kAccessLevelUser);
}

void SaveVideoDialog::SetAuditTrailProperties()
{
    ui.buttonBox->button(QDialogButtonBox::Yes)->setProperty(kAuditTrail, tr("Save Video"));
}

void SaveVideoDialog::showEvent(QShowEvent*)
{
    if (m_SaveVideoFile) {
        ui.labelImageView->hide();
    }
    ui.lineEditInputDataVideoLocation->hide();
    ui.lineEditInputDataNewVideoFile->hide();
    ui.labelMasseageNewVideoFile->hide();
    ui.labelMasseageVideoLocation->hide();
    ui.widgetDashBoardInsertNewVideoFile->hide();
    ui.widgetDashBoardVideoLocation->hide();
}

void SaveVideoDialog::SetImage(const QPixmap& Image)
{
    ui.labelImageView->setPixmap(Image);
}

void SaveVideoDialog::SetOriginalImage(const QImage& Image)
{
    m_OriginalImage = Image;
}

QString SaveVideoDialog::GetFileName()
{
    return ui.lineEditInputDataNewVideoFile->text();
}

QString SaveVideoDialog::GetLocation()
{
    return ui.lineEditInputDataVideoLocation->text();
}

void SaveVideoDialog::SlotOkPressed()
{
    QString CurrentDate = QDateTime::currentDateTime().date().toString();
    QString CurrentTime = QDateTime::currentDateTime().time().toString("hh mm ss zzz");
   
   
    m_PopupSaveVideoDialog->accept();
    if (GetMainAppCrystalT2()) {
        if (m_SaveVideoFile) {
            QString Location = GetMainAppCrystalT2()->GetVideoFileLocation();
            QString FileName = QString("VideoFile[%1 %2]").arg(CurrentDate).arg(CurrentTime);
            QString PathAndName = Location + QString("/") + FileName + m_DefaultFileExtention;
            if (GetMainAppCrystalT2()->GetVideoDialogFullVideo()) {
                GetMainAppCrystalT2()->GetVideoDialogFullVideo()->SaveVideo(PathAndName);
            }
            // m_VideoDialog->SaveVideo(PathAndName);
        } else {
            QString Location = GetMainAppCrystalT2()->GetTriggerImagesFileLocation();
            QString FileName = QString("ImageSavedManually[%1 %2]").arg(CurrentDate).arg(CurrentTime);
            QString PathAndName = Location + QString("/") + FileName + m_DefaultFileExtention;
            m_OriginalImage.save(PathAndName);
        }
    }
}

void SaveVideoDialog::SlotCancelPressed()
{
    m_PopupSaveVideoDialog->reject();
}
