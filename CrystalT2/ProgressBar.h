#pragma once
#include <qwidget.h>
#include "bmessagebox.h"
#include "popupdialog.h"
#include "ui_ProgressBar.h"

class PopupProgressBar;
class ProgressBar : public QWidget
{
    Q_OBJECT
  public:
    ProgressBar(QWidget* pParent);
    ~ProgressBar();
    void SetValue(int i);
    void SetTextInfo(QString& Info);

  public slots:
    void SlotAbort();

  private:
    Ui::ProgressBar ui;
    PopupProgressBar* m_PopupProgressBar;
};

class PopupProgressBar : public PopupDialog
{
    Q_OBJECT
  public:
    PopupProgressBar(QWidget* parent) : PopupDialog(parent)
    {
        m_Abort = false;
        m_ProgressBar = new ProgressBar(this);
        QBoxLayout* box = new QVBoxLayout();
        centralWidget()->setLayout(box);
        setWindowTitle(tr("Save Video"));
        box->addWidget(m_ProgressBar);
        m_InfoTextPleaseWait = tr("Save Video Please Wait....");
        m_MessageBoxTextInfo = tr("Save Video Info");
    }
    bool IsAbort() { return m_Abort; }
    void SetAbort()
    {
        hide();
        emit SignalAbortIsClicked();
    }
    void SetLocation(QString& Location) { m_VideoFileName = Location; }
    void SetInfoTextPleaseWait(const QString& set)
    {
        m_InfoTextPleaseWait = set;
        if (m_ProgressBar) m_ProgressBar->SetTextInfo(m_InfoTextPleaseWait);
    }
    void SetMessageBoxTextInfo(const QString& set) { m_MessageBoxTextInfo = set; }
    void SetMessageBoxSecondTextInfo(const QString& set) { m_MessageBoxSecondTextInfo = set; }
    void SetTitle(const QString& set) { setWindowTitle(set); }
  signals:
    void SignalAbortIsClicked();
  public slots:
    void SlotSetValue(int Value)
    {
        if (Value == 0) {
            m_Abort = false;
            m_ProgressBar->SetTextInfo(m_InfoTextPleaseWait);
            PopupDialog::show();
        }
        m_ProgressBar->SetValue(Value);
        if (Value >= 100) {
            hide();
            if (!m_Abort) BMessageBox::information(this, m_MessageBoxTextInfo, m_MessageBoxSecondTextInfo, QMessageBox::Ok);
        }
        if (Value == -1) {
            if (isVisible()) {
                hide();
            }
        }
        
    }
    void mouseReleaseEvent(QMouseEvent* event)  
    {
        QDialog::mouseReleaseEvent(event);
    }

    ProgressBar* GetProgressBar() { return m_ProgressBar; }

  private:
    ProgressBar* m_ProgressBar;
    bool m_Abort;
    QString m_VideoFileName;
    QString m_InfoTextPleaseWait;
    QString m_MessageBoxTextInfo;
    QString m_MessageBoxSecondTextInfo;
};
