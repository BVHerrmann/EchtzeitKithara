#include "ProgressBar.h"

ProgressBar::ProgressBar(QWidget* pParent) : QWidget(pParent)
{
    ui.setupUi(this);
    m_PopupProgressBar = static_cast<PopupProgressBar*>(pParent);
    ui.progressBar->setMinimum(0);
    ui.progressBar->setMaximum(100);
    connect(ui.pushButtonAbort, &QPushButton::clicked, this, &ProgressBar::SlotAbort);
}

ProgressBar::~ProgressBar()
{
}

void ProgressBar::SetValue(int i)
{
    ui.progressBar->setValue(i);
}

void ProgressBar::SetTextInfo(QString& Info)
{
    ui.label->setText(Info);
}

void ProgressBar::SlotAbort()
{
    if (m_PopupProgressBar) m_PopupProgressBar->SetAbort();
}
