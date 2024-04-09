#include "MessageDialog.h"
#include <QStandardItemModel>
#include <QtCore>
#include "GlobalConst.h"
#include "MainAppCrystalT2.h"
#include "SharedData.h"
#include "bmessagebox.h"
#include "qmessagebox.h"

//Class not in use
MessageDialog::MessageDialog(MainAppCrystalT2* pMainAppCrystalT2, QWidget* parent) : QDialog((QWidget*)(parent)), m_AllIn(false), m_MaxEntries(500), m_PopupMessageDialog(NULL)
{
    ui.setupUi(this);
    QStringList HeaderList;
    QDesktopWidget Desktop;

    setFixedWidth(static_cast<int>(Desktop.width() * 0.8));
    setFixedHeight(static_cast<int>(Desktop.height() * 0.8));

    m_MainAppCrystalT2 = pMainAppCrystalT2;
    m_PopupMessageDialog = (PopupMessageDialog*)(parent);

    ShowCloseButton(false);
    ShowResetButton(true);
    HeaderList.append(QString("Nr"));
    HeaderList.append(QString("Time"));
    HeaderList.append(QString("Date"));
    HeaderList.append(QString("Status"));
    HeaderList.append(QString("Text"));

    ui.tableMessageWidget->setColumnCount(HeaderList.count());
    ui.tableMessageWidget->setHorizontalHeaderLabels(HeaderList);

    ui.tableMessageWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui.tableMessageWidget->horizontalHeader()->setMinimumWidth(1500);

    ui.tableMessageWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui.tableMessageWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui.tableMessageWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui.tableMessageWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui.tableMessageWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

    connect(ui.pushButtonClearAllMessages, &QPushButton::clicked, this, &MessageDialog::SlotClearAllMessages);
    connect(ui.pushButtonClose, &QPushButton::clicked, this, &MessageDialog::SlotCloseButtonClicked);
    connect(ui.pushButtonResetError, &QPushButton::clicked, this, &MessageDialog::SlotResetErrors);
}
// Class not in use
MessageDialog::~MessageDialog()
{
}
// Class not in use
// Anzeigen unterschiedlicher Fehlertexte enum QtMsgType { QtDebugMsg, QtWarningMsg, QtCriticalMsg, QtFatalMsg, QtInfoMsg, QtSystemMsg = QtCriticalMsg };
void MessageDialog::AddNewMessage(const QString& NewMessage, QtMsgType MsgType)
{
    QStringList NewEntrie;
    int NewRowIndex;
    QTableWidgetItem* NewItem = NULL;
    QString Status;
    QList<QTableWidgetItem*> ItemList;

    switch (MsgType) {
        // case ERROR_CODE_ANY_ERROR:
        // case ERROR_CODE_READ_WRITE:
        case QtMsgType::QtFatalMsg:
            Status = "Error";
            if (!m_AllIn) {
                ItemList = ui.tableMessageWidget->findItems(NewMessage, Qt::MatchExactly);
                if (ItemList.count() > 0) return;
            }
            break;
        case QtMsgType::QtWarningMsg:  
            Status = QString("Warning Level 1");
            if (!m_AllIn) {
                ItemList = ui.tableMessageWidget->findItems(NewMessage, Qt::MatchExactly);
                if (ItemList.count() > 0) return;
            }
            break;
        case QtMsgType::QtCriticalMsg: 
            Status = QString("Warning Level 2");
            break;
        default:
            Status = QString("Info");
            if (!m_AllIn) {
                ItemList = ui.tableMessageWidget->findItems(NewMessage, Qt::MatchExactly);
                if (ItemList.count() > 0) return;
            }
            break;
    }
    NewRowIndex = ui.tableMessageWidget->rowCount();
    NewEntrie.append(QString("%1").arg(NewRowIndex + 1));
    NewEntrie.append(QDateTime::currentDateTime().time().toString("hh:mm:ss.zzz"));
    NewEntrie.append(QDateTime::currentDateTime().date().toString("dd.MM.yyyy"));
    NewEntrie.append(Status);
    NewEntrie.append(NewMessage);
    ui.tableMessageWidget->insertRow(NewRowIndex);
    for (int col = 0; col < NewEntrie.length(); ++col) {
        NewItem = new QTableWidgetItem(NewEntrie.at(col));
        ui.tableMessageWidget->setItem(NewRowIndex, col, NewItem);
        if (col == NewEntrie.length() - 2) {
            if (MsgType == QtMsgType::QtFatalMsg) {
                NewItem->setBackgroundColor(COLOR_STATUS_ALARM);
                NewItem->setForeground(COLOR_CONTENT_BOARD);
            } else {
                if (MsgType == QtMsgType::QtWarningMsg) {
                    NewItem->setBackgroundColor(COLOR_STATUS_WARNING1);
                    NewItem->setForeground(COLOR_CONTENT_BOARD);
                }
            }
        }
    }

    if (MsgType == QtMsgType::QtFatalMsg) {
        if (GetMainAppCrystalT2()) {
            QString Message;
            for (int i = 2; i < NewEntrie.count(); i++) {
                Message.append(NewEntrie.at(i));
                Message.append(QString(" "));
            }
            GetMainAppCrystalT2()->WriteLogFile(Message, QString("ErrorLog.txt"));  // für evtl spätere analyse

            GetMainAppCrystalT2()->SetErrorLight(true);
            GetMainAppCrystalT2()->SetErrorTransfer(false);  
        }
    }

    if (MsgType == QtMsgType::QtWarningMsg) {
        GetMainAppCrystalT2()->SetErrorTransfer(false);  
    }
    if (NewRowIndex >= m_MaxEntries) {
        for (int row = 0; row < ui.tableMessageWidget->rowCount(); row++) {
            QTableWidgetItem* Item = ui.tableMessageWidget->takeItem(row, 0);
            Item->setText(QString("%1").arg(row));
            ui.tableMessageWidget->setItem(row, 0, Item);
        }
        ui.tableMessageWidget->removeRow(0);
    }
    ui.tableMessageWidget->scrollToBottom();
    ui.tableMessageWidget->update();
}
// Class not in use
void MessageDialog::RemoveAllErrorMessages()
{
    int count = ui.tableMessageWidget->rowCount();
    int row = 0;

    for (row = 0; row < count; row++) {
        QTableWidgetItem* Item = ui.tableMessageWidget->item(row, 3);
        if (Item->text() == QString("Error")) {
            ui.tableMessageWidget->removeRow(row);
            count = ui.tableMessageWidget->rowCount();
            row = -1;
        }
    }
}
// Class not in use
bool MessageDialog::IsAnyErrorIn()
{
    bool rv = false;
    QList<QTableWidgetItem*> ListItems = ui.tableMessageWidget->findItems(QString("Error"), Qt::MatchExactly);
    if (ListItems.count() > 0) rv = true;
    return rv;
}
// Class not in use
bool MessageDialog::ResetErrors()
{
    bool rv = false;
    if (BMessageBox::information(this, QString("Reset"), QString("Reset Error?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        RemoveAllErrorMessages();
        GetMainAppCrystalT2()->SetCurrentMaschineState(PluginInterface::MachineState::Off);
        GetMainAppCrystalT2()->SetErrorTransfer(true);  // fehler zurückgesetzt an übergeorneten steuerun
        rv = true;
    }
    return rv;
}
// Class not in use
void MessageDialog::SlotCloseButtonClicked()
{
    if (GetPopupMessageDialog()) GetPopupMessageDialog()->accept();
    accept();
}
// Class not in use
void MessageDialog::SlotClearAllMessages()
{
    if (BMessageBox::information(this, QString("Clear All"), QString("Delete All Messages?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) ClearAllMessages();
}
// Class not in use
void MessageDialog::ShowResetButton(bool sh)
{
    if (sh)
        ui.pushButtonResetError->show();
    else
        ui.pushButtonResetError->hide();
}
// Class not in use
void MessageDialog::ShowClearButton(bool sh)
{
    if (sh)
        ui.pushButtonClearAllMessages->show();
    else
        ui.pushButtonClearAllMessages->hide();
}
// Class not in use
void MessageDialog::EnableCloseButton(bool en)
{
    ui.pushButtonClose->setEnabled(en);
}
// Class not in use
void MessageDialog::ShowCloseButton(bool sh)
{
    if (sh)
        ui.pushButtonClose->show();
    else
        ui.pushButtonClose->hide();
}
// Class not in use
void MessageDialog::ClearAllMessages()
{
    int count = ui.tableMessageWidget->rowCount();
    for (int row = 0; row < count; ++row) ui.tableMessageWidget->removeRow(0);
    ui.tableMessageWidget->clearContents();
}
// Class not in use
void MessageDialog::SlotCloseMessageDialog()
{
    accept();
}
// Class not in use
void MessageDialog::SlotResetErrors()
{
    ResetErrors();
}