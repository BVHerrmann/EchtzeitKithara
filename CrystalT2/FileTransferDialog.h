#pragma once

#include <QDialog>
#include <QFileSystemModel>
#include "bmessagebox.h"
#include "popupdialog.h"
#include "ui_FileTransferDialog.h"

class ThreadCopyFiles;
class PopupProgressBar;
class ModelSourceFilterProxy;
class ModelDestinationFilterProxy;
class FileSystemModelWithCheckBoxes;
class PopupDialogFileTransfer;
class MainAppCrystalT2;
class FileTransferDialog : public QWidget
{
    Q_OBJECT

  public:
    FileTransferDialog(MainAppCrystalT2* pMainAppCrystalT2, QWidget* parent);
    ~FileTransferDialog();
    MainAppCrystalT2* GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
    void showEvent(QShowEvent*);
    void hideEvent(QHideEvent*);
    bool filterAcceptFilesAndDirsModelSource(QString& path);
    bool filterAcceptFilesAndDirsModelDesination(QString& fileName, QString& filePath);
    void scanSourceDirs();
    void transferFile(const QString& fromDir);
    void enableEjectUSD();
    void transferFiles(const QStringList& list);
    bool checkIsAbortClicked();
    void SetAuditTrailProperties();
    QString getRootUSBPath();
    void usbDriveEject(const QString letter);

  public slots:
    void SlotScanUSB();
    void SlotTransferFiles();
    void SlotFileTransferAbortIsClicked();
    void SlotSetStatusFileTransfer(const QString& Info, int value);
    void SlotCloseDialogAndEnableUSB();
  signals:
    void SignalSetStatusFileTransfer(const QString& Info, int value);

  private:
    Ui::FileTransferDialogClass ui;
    MainAppCrystalT2* m_MainAppCrystalT2;
    PopupDialogFileTransfer* m_PopupDialogFileTransfer;
    FileSystemModelWithCheckBoxes* m_DirModelSource;
    QFileSystemModel* m_DirModelDestination;
    QStringList m_ListSourceDirs;
    QString m_USB_DirName;
    QString m_RootPathDesitination;
    QString m_RootPathSource;
    ModelDestinationFilterProxy* m_ModelDestinationFilterProxy;
    ModelSourceFilterProxy* m_ModelSourceFilterProxy;
    bool m_EnableUSB;
    bool m_AbortFileTransferIsClicked;
    PopupProgressBar* m_PopupProgressBar;
    int m_FileTransferCounter;
    int m_TotalNumberFiles;
    ThreadCopyFiles* m_ThreadCopyFiles;
};

class ThreadCopyFiles : public QThread
{
  public:
    ThreadCopyFiles(FileTransferDialog* pFileTransferDialog) : QThread() { m_FileTransferDialog = pFileTransferDialog; }
    void SetCopyFilesAndDirs(const QStringList& FilesAndDirs) { m_CopyFilesAndDirs = FilesAndDirs; }
    void run()
    {
        if (m_FileTransferDialog) {
            m_FileTransferDialog->transferFiles(m_CopyFilesAndDirs);
        }
    }

  private:
    FileTransferDialog* m_FileTransferDialog;
    QStringList m_CopyFilesAndDirs;
};

class PopupDialogFileTransfer : public PopupDialog
{
  public:
    PopupDialogFileTransfer(MainAppCrystalT2* pMainAppCrystalT2, QWidget* parent) : PopupDialog(parent)
    {
        m_FileTransferDialog = new FileTransferDialog(pMainAppCrystalT2, this);
        QBoxLayout* box = new QVBoxLayout();
        centralWidget()->setLayout(box);
        setWindowTitle(tr("File Transfer to USB"));
        box->addWidget(m_FileTransferDialog);
    }
    FileTransferDialog* GetFileTransferDialog() { return m_FileTransferDialog; }

  private:
    FileTransferDialog* m_FileTransferDialog;
};

class ModelSourceFilterProxy : public QSortFilterProxyModel
{
  public:
    ModelSourceFilterProxy(FileTransferDialog* parent = nullptr) : QSortFilterProxyModel(parent) { m_FileTransferDialog = parent; }
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
    {
        QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
        QString FilePath = static_cast<QFileSystemModel*>(sourceModel())->filePath(index);
        if (m_FileTransferDialog)
            return m_FileTransferDialog->filterAcceptFilesAndDirsModelSource(FilePath);
        else
            return true;
    }

  public:
    FileTransferDialog* m_FileTransferDialog;
};

class ModelDestinationFilterProxy : public QSortFilterProxyModel
{
  public:
    ModelDestinationFilterProxy(FileTransferDialog* parent = nullptr) : QSortFilterProxyModel(parent) { m_FileTransferDialog = parent; }
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
    {
        QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
        QString fileName = static_cast<QFileSystemModel*>(sourceModel())->fileName(index);
        QString filePath = static_cast<QFileSystemModel*>(sourceModel())->filePath(index);
        if (m_FileTransferDialog)
            return m_FileTransferDialog->filterAcceptFilesAndDirsModelDesination(fileName, filePath);
        else
            return true;
    }

  public:
    FileTransferDialog* m_FileTransferDialog;
};

class FileSystemModelWithCheckBoxes : public QFileSystemModel
{
  public:
    FileSystemModelWithCheckBoxes(QWidget* parent, const QString& RootPath) : QFileSystemModel(parent) { m_RootPath = RootPath; };

    /*bool FileSystemModelWithCheckBoxes::hasChildren(const QModelIndex& parent = QModelIndex()) const
    {
        if (parent.flags() & Qt::ItemNeverHasChildren) return false;
        return QDirIterator(filePath(parent), filter(), QDirIterator::NoIteratorFlags).hasNext();
    }*/

    QVariant FileSystemModelWithCheckBoxes::data(const QModelIndex& index, int role) const
    {
        if (role == Qt::CheckStateRole) {
            if (checklist.contains(index)) {
                return checklist[index];
            } else {
                return Qt::Unchecked;
            }
        }
        return QFileSystemModel::data(index, role);
    }

    Qt::ItemFlags FileSystemModelWithCheckBoxes::flags(const QModelIndex& index) const { return QFileSystemModel::flags(index) | Qt::ItemIsUserCheckable; }

    bool FileSystemModelWithCheckBoxes::setData(const QModelIndex& index, const QVariant& value, int role)
    {
        if (role == Qt::CheckStateRole) {
            QString temp = filePath(index);
            if (temp.toUpper() != m_RootPath.toUpper()) {
                setNodeCheckState(index, value, role);
            }
        }
        return QFileSystemModel::setData(index, value, role);
    }

    bool FileSystemModelWithCheckBoxes::setNodeCheckState(const QModelIndex& index, const QVariant& value, int role)
    {
        if (role == Qt::CheckStateRole) {
            Qt::CheckState currentNodeCheckState = static_cast<Qt::CheckState>(value.toUInt());
            if (checklist.contains(index)) {
                checklist[index] = currentNodeCheckState;
            } else {
                checklist.insert(index, currentNodeCheckState);
            }
            setChildNodesCheck(index, value);

            emit dataChanged(index, index);
            return true;
        }
        return QFileSystemModel::setData(index, value, role);
    }

    bool FileSystemModelWithCheckBoxes::setChildNodesCheck(const QModelIndex& index, const QVariant& value)
    {
        if (canFetchMore(index)) {
            fetchMore(index);
        }
        int childrenCount = rowCount(index);
        for (int i = 0; i < childrenCount; i++) {
            QModelIndex child = QFileSystemModel::index(i, 0, index);
            setNodeCheckState(child, value, Qt::CheckStateRole);
        }
        return true;
    }

    QStringList FileSystemModelWithCheckBoxes::GetCheckedFilesNandDirs()
    {
        QStringList listCheckedFilesAndDirs;
        QMapIterator<QPersistentModelIndex, Qt::CheckState> i(checklist);
        while (i.hasNext()) {
            i.next();

            if (i.value()) {
                // is checked
                QModelIndex checkeModelIndex = i.key();
                listCheckedFilesAndDirs.append(filePath(checkeModelIndex));
            }
        }
        return listCheckedFilesAndDirs;
    }

  private:
    QMap<QPersistentModelIndex, Qt::CheckState> checklist;
    QString m_RootPath;
};
