#include "AddNewProductDialog.h"
#include "MainAppCrystalT2.h"
#include "ProductDialog.h"
#include "bmessagebox.h"
#include "qtimer.h"

#include <audittrail.h>

AddNewProductDialog::AddNewProductDialog(MainAppCrystalT2* pMainAppCrystalT2, QDialog* parent) : QDialog(parent), m_MainAppCrystalT2(NULL)
{
    ui.setupUi(this);
    m_MainAppCrystalT2 = pMainAppCrystalT2;

    connect(ui.pushButtonOK, &QPushButton::clicked, this, &AddNewProductDialog::SlotYes);
    connect(ui.pushButtonCancel, &QPushButton::clicked, this, &AddNewProductDialog::SlotNo);
    connect(ui.toolButtonSelectInheritProduct, &QToolButton::clicked, this, &AddNewProductDialog::SlotOpenCombobox);
    ui.pushButtonOK->setProperty(kAuditTrail, ui.pushButtonOK->text());
}

AddNewProductDialog::~AddNewProductDialog()
{
}

void AddNewProductDialog::SlotOpenCombobox()
{
    ui.comboBox->showPopup();
}

void AddNewProductDialog::ClearCombobox()
{
    ui.comboBox->clear();
}

QString AddNewProductDialog::GetSelectedProduct()
{
    return ui.comboBox->currentText();
}

void AddNewProductDialog::AddItemToCombobox(QString& item)
{
    ui.comboBox->addItem(item);
}

void AddNewProductDialog::showEvent(QShowEvent*)
{
    ui.lineEditInputData->setFocus();
    activateWindow();
    QTimer::singleShot(10, this, &AddNewProductDialog::SlotMoveWidget);
}

void AddNewProductDialog::SlotMoveWidget()
{
    if (GetMainAppCrystalT2()) move(geometry().x(), 120);
}

QString AddNewProductDialog::GetInputString()
{
    return ui.lineEditInputData->text();
}

void AddNewProductDialog::SlotYes()
{
    if (GetMainAppCrystalT2()) {
        QMessageBox::StandardButton rv;
        QString NewProductName = GetInputString();
        QString CopyFromProductName = GetSelectedProduct();
        if (!NewProductName.isEmpty()) {
            rv = GetMainAppCrystalT2()->GetPopupDialogProductDialog()->GetProductDialog()->InsertNewProduct(NewProductName, CopyFromProductName);
            if (rv == QMessageBox::Yes) accept();
        } else {
            BMessageBox::warning(GetMainAppCrystalT2(), tr("Warning"), tr("Can Not Add New Product. New Product Name Is Empty!"), QMessageBox::Ok);
        }
    }
}

void AddNewProductDialog::SlotNo()
{
    accept();
}

void AddNewProductDialog::OpenDialogNewInput(const QString& Message)
{
    QRegExp rx("^\\w+$");
    QValidator* validator = new QRegExpValidator(rx, this);

    ui.lineEditInputData->setReadOnly(false);
    ui.lineEditInputData->show();
    ui.lineEditInputData->setValidator(validator);
    ui.labelMasseage->setText(Message);
    ui.lineEditInputData->setFocus();
    show();
}
