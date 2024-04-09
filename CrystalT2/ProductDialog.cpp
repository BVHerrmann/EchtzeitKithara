#include "ProductDialog.h"
#include "AddNewProductDialog.h"
#include "EditProductDialog.h"
#include "GlobalConst.h"
#include "InputMessageDialog.h"
#include "MainAppCrystalT2.h"
#include "ProductData.h"
#include "SharedData.h"
#include "bmessagebox.h"

#include <audittrail.h>

ProductDialog::ProductDialog(MainAppCrystalT2* pMainAppCrystalT2, QWidget* parent)
    : QDialog(parent), m_MainAppCrystalT2(NULL), m_PopupInputMessageDialog(NULL), m_AddNewProductDialog(NULL), m_PopupDialogProductDialog(NULL)
{
    ui.setupUi(this);
    m_MainAppCrystalT2 = pMainAppCrystalT2;
    m_PopupDialogProductDialog = (PopupDialogProductDialog*)(parent);

    connect(ui.pushButtonClose, &QPushButton::clicked, this, &ProductDialog::SlotCloseProductDialog);
    connect(ui.pushButtonReadAndAppendProduct, &QPushButton::clicked, this, &ProductDialog::SlotReadAndAppendProduct);
    connect(ui.pushButtonLoadProduct, &QPushButton::clicked, this, &ProductDialog::SlotLoadProduct);
    connect(ui.pushButtonRenameProduct, &QPushButton::clicked, this, &ProductDialog::SlotRenameProduct);
    connect(ui.pushButtonLoadDeleteProduct, &QPushButton::clicked, this, &ProductDialog::SlotDeleteProduct);

    ui.pushButtonEditProduct->hide();
    m_PopupInputMessageDialog = new PopupInputMessageDialog(this);
    m_AddNewProductDialog = new AddNewProductDialog(pMainAppCrystalT2, this);
    m_AddNewProductDialog->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);

    connect(m_PopupInputMessageDialog, &PopupInputMessageDialog::SignalYes, this, &ProductDialog::SlotRenameAndActivateProduct);
    SetAuditTrailProperties();
}

ProductDialog::~ProductDialog()
{
}

void ProductDialog::showEvent(QShowEvent*)
{
    UpdateProductList();
    activateWindow();
}

void ProductDialog::SetAuditTrailProperties()
{
    ui.pushButtonLoadProduct->setProperty(kAuditTrail, ui.pushButtonLoadProduct->text());
    ui.pushButtonRenameProduct->setProperty(kAuditTrail, ui.pushButtonRenameProduct->text());
    ui.pushButtonLoadDeleteProduct->setProperty(kAuditTrail, ui.pushButtonLoadDeleteProduct->text());
    ui.pushButtonClose->setProperty(kAuditTrail, ui.pushButtonClose->text());
    ui.pushButtonReadAndAppendProduct->setProperty(kAuditTrail, ui.pushButtonReadAndAppendProduct->text());
}

void ProductDialog::UpdateProductList()
{
    if (GetMainAppCrystalT2()) {
        QString LastProductName = GetMainAppCrystalT2()->GetCurrentProductName();
        QString ProductName;
        QList<ProductData*> pProductList = GetMainAppCrystalT2()->GetListProducts();
        bool LastProductNameIsIn = false;
        int row = 0;

        ui.listProducts->clear();
        for (int i = 0; i < pProductList.count(); i++) {
            ProductName = pProductList.at(i)->GetProductName();
            ui.listProducts->insertItem(row, ProductName);
            if (ProductName == LastProductName) LastProductNameIsIn = true;
            row++;
        }
        if (pProductList.count() > 0) {
            if (!LastProductNameIsIn) {
                LastProductName = pProductList.at(0)->GetProductName();
                GetMainAppCrystalT2()->ShowAndSetCurrentProductName(LastProductName);
            }
            ui.lineEditCurrentProduct->setText(GetMainAppCrystalT2()->GetCurrentProductName());
            ui.listProducts->setCurrentRow(0, QItemSelectionModel::Select);
            ui.listProducts->setFocus();
        } else
            ui.lineEditCurrentProduct->setText("");
    }
}

void ProductDialog::SlotReadAndAppendProduct()
{
    if (GetAddNewProductDialog()) {
        GetAddNewProductDialog()->ClearCombobox();
        for (int i = 0; i < ui.listProducts->count(); i++) {
            QString value = ui.listProducts->item(i)->text();
            GetAddNewProductDialog()->AddItemToCombobox(value);
        }
        GetAddNewProductDialog()->OpenDialogNewInput(tr("Insert New Product Name"));  // show dialog
    }
}

QMessageBox::StandardButton ProductDialog::InsertNewProduct(QString& NewProductName, QString& CopyFromProductName)
{
    QMessageBox::StandardButton rv = QMessageBox::Yes;
    if (GetMainAppCrystalT2()) {
        if (!NewProductName.isEmpty()) {
            if (GetMainAppCrystalT2()->ExistProduct(NewProductName)) {
                BMessageBox* pMessageBox = new BMessageBox(QMessageBox::Information, tr("Override Product"), tr("Product %1 Exist, Override?").arg(NewProductName), this);
                pMessageBox->addButton(QMessageBox::Yes)->setText(tr("Yes"));
                pMessageBox->addButton(QMessageBox::No)->setText(tr("No"));

                if (QDialog::Accepted == pMessageBox->exec()) {
                    GetMainAppCrystalT2()->RemoveProduct(NewProductName);
                    InsertActiveAndLoadProductSettings(NewProductName, CopyFromProductName);
                }
                delete pMessageBox;
            } else
                InsertActiveAndLoadProductSettings(NewProductName, CopyFromProductName);
        }
    }
    return rv;
}

void ProductDialog::InsertActiveAndLoadProductSettings(QString& NewProductName, QString& CopyFromProductName)
{
    QString ErrorMsg;
    int retVal = GetMainAppCrystalT2()->WriteAndInsertNewProduct(NewProductName, CopyFromProductName, ErrorMsg);
    if (retVal == ERROR_CODE_NO_ERROR) {
        retVal = GetMainAppCrystalT2()->ActivateCurrentProduct(NewProductName, ErrorMsg);
        if (retVal != ERROR_CODE_NO_ERROR) {
            GetMainAppCrystalT2()->SlotAddNewMessage(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
            GetMainAppCrystalT2()->SetCurrentMaschineState(PluginInterface::MachineState::Error);
        }
    } else {
        GetMainAppCrystalT2()->SlotAddNewMessage(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
        GetMainAppCrystalT2()->SetCurrentMaschineState(PluginInterface::MachineState::Error);
    }
    UpdateProductList();
}

void ProductDialog::SlotLoadProduct()
{
    if (GetMainAppCrystalT2()) {
        QList<QListWidgetItem*> m_List = ui.listProducts->selectedItems();
        QString ErrorMsg, NewProductName;

        if (m_List.count() > 0) {
            NewProductName = m_List.at(0)->text();
            ui.lineEditCurrentProduct->setText(NewProductName);
        }
        GetMainAppCrystalT2()->ActivateCurrentProduct(NewProductName, ErrorMsg);
    }
}

void ProductDialog::SlotRenameProduct()
{
    QList<QListWidgetItem*> m_List = ui.listProducts->selectedItems();

    if (m_List.count() > 0) {
        m_CurrentProductName = m_List.at(0)->text();

        if (GetPopupInputMessageDialog()) {
            GetPopupInputMessageDialog()->setWindowTitle(tr("Rename Product "
                                                            "%1"
                                                            "")
                                                             .arg(m_CurrentProductName));
            GetPopupInputMessageDialog()->GetInputMessageDialog()->OpenYesNoDialog(tr("NewName"), false);
        }
    }
}

void ProductDialog::SlotRenameAndActivateProduct()
{
    if (GetPopupInputMessageDialog() && GetPopupInputMessageDialog()->GetInputMessageDialog()) {
        QString ErrorMsg, NewName = GetPopupInputMessageDialog()->GetInputMessageDialog()->GetInputString();
        if (!NewName.isEmpty() && GetMainAppCrystalT2()) GetMainAppCrystalT2()->RenameAndActivateProduct(m_CurrentProductName, NewName, ErrorMsg);
    }
}

void ProductDialog::SlotDeleteProduct()
{
    QList<QListWidgetItem*> ListSelected = ui.listProducts->selectedItems();

    if (ListSelected.count() > 0) {
        if (ui.listProducts->count() > 1) {  // es müssen mindesten zwei Produkte verfügbar sein damit eins gelöscht werden kann
            QString ProductName = ListSelected.at(0)->text();
            if (GetMainAppCrystalT2()) {
                BMessageBox* pMessageBox = new BMessageBox(QMessageBox::Information, tr("Delete Product"),
                                                           tr("Delete Product "
                                                              "%1"
                                                              " From System?")
                                                               .arg(ProductName),
                                                           this);
                pMessageBox->addButton(QMessageBox::Yes)->setText(tr("Yes"));
                pMessageBox->addButton(QMessageBox::No)->setText(tr("No"));

                if (QDialog::Accepted == pMessageBox->exec()) {
                    QString ErrorMsg, PathAndFileName = GetMainAppCrystalT2()->GetPathNameProducts() + QString("/") + ProductName + QString(".dat");
                    QFile RemovedFile(PathAndFileName);
                    int rv = ERROR_CODE_NO_ERROR;

                    RemovedFile.remove();
                    rv = GetMainAppCrystalT2()->LoadAllProductFiles(ErrorMsg);
                    if (rv != ERROR_CODE_NO_ERROR) {
                        if (rv == ERROR_CODE_NO_PRODUCTS) {                                             // kein produkt vorhanden
                            GetMainAppCrystalT2()->SlotAddNewMessage(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
                            GetMainAppCrystalT2()->SetCurrentMaschineState(PluginInterface::MachineState::Error);
                            GetMainAppCrystalT2()->ShowAndSetCurrentProductName(tr("Undefined"));
                            GetMainAppCrystalT2()->SaveSettings();
                        } else {
                            if (rv == ERROR_CODE_READ_WRITE) {
                                GetMainAppCrystalT2()->SlotAddNewMessage(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
                                GetMainAppCrystalT2()->SetCurrentMaschineState(PluginInterface::MachineState::Error);
                            }
                        }
                    }
                    UpdateProductList();
                    rv = GetMainAppCrystalT2()->ActivateCurrentProduct(GetMainAppCrystalT2()->GetCurrentProductName(), ErrorMsg);
                    if (rv == ERROR_CODE_READ_WRITE) {
                        GetMainAppCrystalT2()->SlotAddNewMessage(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
                        GetMainAppCrystalT2()->SetCurrentMaschineState(PluginInterface::MachineState::Error);
                    }
                }
                delete pMessageBox;
            }
        } else {
            BMessageBox::warning(this, tr("Can Not Delete Product"), tr("System Cannot Run Without A Single Product!"), QMessageBox::Ok);
        }
    }
}

void ProductDialog::SlotCloseProductDialog()
{
    if (GetPopupDialogProductDialog()) GetPopupDialogProductDialog()->accept();
}
