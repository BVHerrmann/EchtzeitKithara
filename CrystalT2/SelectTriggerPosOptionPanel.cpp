#include "SelectTriggerPosOptionPanel.h"
#include "MainAppCrystalT2.h"
#include "VideoDialog.h"

#include <audittrail.h>

SelectTriggerPosOptionPanel::SelectTriggerPosOptionPanel(MainAppCrystalT2* pMainAppCrystalT2)
    : OptionPanel(pMainAppCrystalT2), m_CheckBoxRecordImages(NULL), m_TextRecordImages(NULL), m_FormLayout(NULL)
{
    m_MainAppCrystalT2 = pMainAppCrystalT2;
    m_FormLayout = new QFormLayout();
    setLayout(m_FormLayout);

    m_CheckBoxRecordImages = new QCheckBox();
    m_TextRecordImages = new QLabel(tr("Record Trigger Images"));
    connect(m_CheckBoxRecordImages, &QCheckBox::stateChanged, this, &SelectTriggerPosOptionPanel::SlotSetCheckedRecordImages);
    m_FormLayout->insertRow(0, m_TextRecordImages, m_CheckBoxRecordImages);
    SetAuditTrailProperties();
    SetRequiredAccessLevel();
}

void SelectTriggerPosOptionPanel::SetRequiredAccessLevel()
{
    /*if (m_CheckBoxRecordImages) {
        m_CheckBoxRecordImages->setProperty(kRequiredAccessLevel, kAccessLevelUser);
    }*/
}

void SelectTriggerPosOptionPanel::SetAuditTrailProperties()
{
    if (m_CheckBoxRecordImages) {
        m_CheckBoxRecordImages->setProperty(kAuditTrail, m_TextRecordImages->text());
    }
}

void SelectTriggerPosOptionPanel::ShowRow(int row, bool sh)
{
    if (sh) {
        m_FormLayout->insertRow(0, m_TextRecordImages, m_CheckBoxRecordImages);
        m_CheckBoxRecordImages->show();
        m_TextRecordImages->show();
    } else {
        m_CheckBoxRecordImages->hide();
        m_TextRecordImages->hide();
        m_FormLayout->removeWidget(m_CheckBoxRecordImages);
        m_FormLayout->removeWidget(m_TextRecordImages);
    }
}

void SelectTriggerPosOptionPanel::SlotSetCheckedRecordImages(int set)
{
    if (GetMainAppCrystalT2()) {
        if (set == Qt::Checked)
            GetMainAppCrystalT2()->GetVideoDialogShowProductVideos()->SetRecordingTriggerPosOn(true);
        else
            GetMainAppCrystalT2()->GetVideoDialogShowProductVideos()->SetRecordingTriggerPosOn(false);
    }
}
