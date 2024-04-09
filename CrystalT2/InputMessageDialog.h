#pragma once

#include <QDialog>
#include "ui_InputMessageDialog.h"
#include "popupdialog.h"
#include "qinputdialog.h"
#include "qlineedit.h"

class PopupInputMessageDialog;
class MainAppCrystalT2;
class InputMessageDialog : public QDialog
{
	Q_OBJECT
public:
	InputMessageDialog(QWidget *parent);
	~InputMessageDialog();
	int OpenYesNoDialog(const QString &LineEditMessage, bool Modal=false);
	//int OpenOKDialog(bool  Modal = false);
	int OpenYesNoDialog(bool  Modal = false);
	QString GetInputString();
	void ClearInputString();
	

public slots:
	void SlotYes();
	void SlotNo();
	void SlotCloseDialog();

private:
	Ui::InputMessageDialog   ui;
	
	PopupInputMessageDialog *m_PopupInputMessageDialog;
};


class PopupInputMessageDialog : public PopupDialog
{
	Q_OBJECT
public:
	PopupInputMessageDialog(QWidget *parent) : PopupDialog(parent)
	{
		m_InputMessageDialog = new InputMessageDialog(this);
		QBoxLayout *box      = new QVBoxLayout();
		centralWidget()->setLayout(box);
		box->addWidget(m_InputMessageDialog);
		setWindowTitle(tr("Message"));
		setMaximumHeight(160);
	}
	InputMessageDialog *GetInputMessageDialog() { return m_InputMessageDialog; }
	
	void SendSignalYes() { emit SignalYes(); }
	void SendSignalNO() { emit SignalNo(); }
signals:
	void SignalYes();
	void SignalNo();

private:
	InputMessageDialog *m_InputMessageDialog;
};
