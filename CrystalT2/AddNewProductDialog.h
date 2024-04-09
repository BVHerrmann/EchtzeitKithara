#pragma once

#include <QDialog>
#include "ui_AddNewProductDialog.h"

class MainAppCrystalT2;
class AddNewProductDialog : public QDialog
{
	Q_OBJECT

public:
	AddNewProductDialog(MainAppCrystalT2 *pMainAppCrystalT2, QDialog *Parent);
	~AddNewProductDialog();
	MainAppCrystalT2 *GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
	void showEvent(QShowEvent *);
	void SlotMoveWidget();
	QString GetInputString();
	QString GetSelectedProduct();
	void ClearCombobox();
	void AddItemToCombobox(QString &item);
	void OpenDialogNewInput(const QString &Message);

public slots:
	void SlotOpenCombobox();
	void SlotYes();
	void SlotNo();


private:
	Ui::AddNewProductDialog ui;
	MainAppCrystalT2 *m_MainAppCrystalT2;
};
