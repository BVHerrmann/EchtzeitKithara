#pragma once

#include <QDialog>
#include "ui_SaveVideoDialog.h"
#include "popupdialog.h"

class MainAppCrystalT2;
class PopupSaveVideoDialog;
class SaveVideoDialog : public QDialog
{
	Q_OBJECT
public:
     SaveVideoDialog(MainAppCrystalT2* pMainAppCrystalT2, QWidget* parent);
	~SaveVideoDialog();
	void showEvent(QShowEvent *);
	QString GetFileName();
	QString GetLocation();
	//VideoDialog  *GetVideoDialog() { return m_VideoDialog; }
    MainAppCrystalT2* GetMainAppCrystalT2() {return m_MainAppCrystalT2;}
	void SetDefaultFileName(const QString &set) { m_DefaultFileName = set; }
	void SetDefaultFileExtention(const QString &set) { m_DefaultFileExtention = set; }
	void SetSaveVideoFile(bool set) { m_SaveVideoFile = set; }
	void SetImage(const QPixmap &Image);
	void SetOriginalImage(const QImage &Image);
    void SetAuditTrailProperties();
    void SetRequiredAccessLevel();

public slots:
	void SlotOkPressed();
	void SlotCancelPressed();

private:
	Ui::SaveVideoDialog      ui;
	PopupSaveVideoDialog    *m_PopupSaveVideoDialog;
    MainAppCrystalT2   *m_MainAppCrystalT2;
	QString m_DefaultFileName;
	QString m_DefaultFileExtention;
	QImage m_OriginalImage;
	bool m_SaveVideoFile;
};


class PopupSaveVideoDialog : public PopupDialog
{
public:
    PopupSaveVideoDialog(MainAppCrystalT2* pMainAppCrystalT2, QWidget* parent) : PopupDialog(parent)
	{
        m_SaveVideoDialog = new SaveVideoDialog(pMainAppCrystalT2, this);
		QBoxLayout *box = new QVBoxLayout();
		centralWidget()->setLayout(box);
		setWindowTitle(tr("Save Video"));
		box->addWidget(m_SaveVideoDialog);
    }
	SaveVideoDialog *GetSaveVideoDialog()      {return m_SaveVideoDialog; }
	void SetDefaultFileName(const QString &set)      { if(m_SaveVideoDialog) m_SaveVideoDialog->SetDefaultFileName(set);}
	void SetDefaultFileExtention(const QString &set) { if (m_SaveVideoDialog)m_SaveVideoDialog->SetDefaultFileExtention(set);}
	void SetSaveVideoFile(bool set)                  { if (m_SaveVideoDialog)m_SaveVideoDialog->SetSaveVideoFile(set); }
	void SetImage(const QPixmap &set)                { if (m_SaveVideoDialog)m_SaveVideoDialog->SetImage(set); }
	void SetOriginalImage(const QImage &set)         { if (m_SaveVideoDialog)m_SaveVideoDialog->SetOriginalImage(set); }
    //void SetRequiredAccessLevel()                    { if (m_SaveVideoDialog) m_SaveVideoDialog->SetRequiredAccessLevel();}


private:
	SaveVideoDialog *m_SaveVideoDialog;
	QImage m_OriginalImage;
	
};
