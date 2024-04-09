#pragma once

#include <QDialog>
#include <QWidget>
#include "bmessagebox.h"
#include "popupdialog.h"
#include "ui_ProductDialog.h"

class MainAppCrystalT2;
class PopupInputMessageDialog;
class AddNewProductDialog;
class PopupDialogProductDialog;
class ProductDialog : public QDialog
{
    Q_OBJECT
  public:
    ProductDialog(MainAppCrystalT2* pMainAppCrystalT2, QWidget* parent);
    ~ProductDialog();
    MainAppCrystalT2* GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
    PopupInputMessageDialog* GetPopupInputMessageDialog() { return m_PopupInputMessageDialog; }
    AddNewProductDialog* GetAddNewProductDialog() { return m_AddNewProductDialog; }
    PopupDialogProductDialog* GetPopupDialogProductDialog() { return m_PopupDialogProductDialog; }
    void showEvent(QShowEvent* ev);
    void UpdateProductList();
    QMessageBox::StandardButton InsertNewProduct(QString& NewProductName, QString& CopyFromProduct);
    void InsertActiveAndLoadProductSettings(QString& NewProductName, QString& CopyFromProductName);
    void SetAuditTrailProperties();

  public slots:
    void SlotCloseProductDialog();
    void SlotReadAndAppendProduct();
    void SlotLoadProduct();
    void SlotRenameProduct();
    void SlotDeleteProduct();
    void SlotRenameAndActivateProduct();

  private:
    Ui::ProductDialog ui;
    MainAppCrystalT2* m_MainAppCrystalT2;
    PopupInputMessageDialog* m_PopupInputMessageDialog;
    AddNewProductDialog* m_AddNewProductDialog;
    QString m_CurrentProductName;
    PopupDialogProductDialog* m_PopupDialogProductDialog;
};

class PopupDialogProductDialog : public PopupDialog
{
  public:
    PopupDialogProductDialog(MainAppCrystalT2* pMainAppCrystalT2, QWidget* parent) : PopupDialog(parent)
    {
        m_ProductDialog = new ProductDialog(pMainAppCrystalT2, this);
        QBoxLayout* box = new QVBoxLayout();
        centralWidget()->setLayout(box);
        setWindowTitle(tr("Open Product"));
        box->addWidget(m_ProductDialog);
    }
    ProductDialog* GetProductDialog() { return m_ProductDialog; }

  private:
    ProductDialog* m_ProductDialog;
};
