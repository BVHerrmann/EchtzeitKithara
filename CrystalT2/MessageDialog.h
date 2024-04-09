#pragma once

#include <QDialog>
#include "ui_MessageDialog.h"
#include "popupdialog.h"

class PopupMessageDialog;
class MainAppCrystalT2;
//Class not in use
class MessageDialog : public QDialog
{
	Q_OBJECT
public:
	MessageDialog(MainAppCrystalT2 *pMainAppCrystalT2, QWidget *parent);
	~MessageDialog();
	void AddNewMessage(const QString &message, QtMsgType MsgType);
	MainAppCrystalT2 *GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
	void ClearAllMessages();
	void ShowClearButton(bool);
	void ShowCloseButton(bool);
	void ShowResetButton(bool);
	void EnableCloseButton(bool sen);
	bool IsAnyErrorIn();
	void RemoveAllErrorMessages();
	bool ResetErrors();
	void SetAllIn(bool set) {m_AllIn = set;}
	void SetMaxEntries(int set) { m_MaxEntries = set; }
	PopupMessageDialog    *GetPopupMessageDialog() { return m_PopupMessageDialog; }
	
public slots:
	void SlotCloseMessageDialog();
	void SlotClearAllMessages();
	void SlotCloseButtonClicked();
	void SlotResetErrors();

private:
	Ui::MessageDialog      ui;
	MainAppCrystalT2      *m_MainAppCrystalT2;
	PopupMessageDialog    *m_PopupMessageDialog;
	bool m_AllIn;
	int m_MaxEntries;
};


class PopupMessageDialog : public PopupDialog
{
public:
	PopupMessageDialog(MainAppCrystalT2 *pMainAppCrystalT2, QWidget *parent) : PopupDialog((QWidget *)(parent))
	{
		m_MessageDialog = new MessageDialog(pMainAppCrystalT2,this);
		QBoxLayout *box = new QVBoxLayout();
		centralWidget()->setLayout(box);
                setWindowTitle(QString("Initsystem"));
		box->addWidget(m_MessageDialog);
    }
	MessageDialog *GetMessageDialog() { return m_MessageDialog; }
private:
	MessageDialog *m_MessageDialog;
};
