#include "FileTransferDialog.h"
#include "MainAppCrystalT2.h"
#include "progressbar.h"

#include <audittrail.h>

FileTransferDialog::FileTransferDialog(MainAppCrystalT2* pMainAppCrystalT2, QWidget* parent)
    : QWidget(parent),
      m_ModelDestinationFilterProxy(NULL),
      m_ModelSourceFilterProxy(NULL),
      m_DirModelSource(NULL),
      m_DirModelDestination(NULL),
      m_EnableUSB(true),
      m_PopupProgressBar(NULL),
      m_PopupDialogFileTransfer(NULL),
      m_AbortFileTransferIsClicked(false)
{
    ui.setupUi(this);
    m_ThreadCopyFiles = new ThreadCopyFiles(this);
    m_PopupDialogFileTransfer = (PopupDialogFileTransfer*)(parent);
   
    QFont fnt;
    fnt.setPixelSize(20);
    fnt.setFamily(QString("Bertram"));
    fnt.setBold(true);
    ui.treeViewSource->setFont(fnt);
    ui.treeViewDistination->setFont(fnt);

    m_MainAppCrystalT2 = pMainAppCrystalT2;
    QString VideoFileLocation = "D:/VideoFiles";
    QString BottleEjectedLocation = "D:/BottleEjectedLocation";
    QString TrendGraphDataLocation = "D:/TrendGraphDataLocation";
    QString AudiTrailDataLocation = "D:/AuditTrail";
    QString AlarmMessageLocation = "D:/AlarmMessage";
    QString TriggerFileLocation = "D:/TriggerImages";
    QString ProductDataFileLocation = "D:/ProductData";
    QString ScreenshotPath = "D:/Screenshots";
    if (GetMainAppCrystalT2()) {
        VideoFileLocation = GetMainAppCrystalT2()->GetVideoFileLocation();
        BottleEjectedLocation = GetMainAppCrystalT2()->GetErrorImagePoolLocation();
        TrendGraphDataLocation = GetMainAppCrystalT2()->GetTrendGraphDataLocation();
        TriggerFileLocation = GetMainAppCrystalT2()->GetTriggerImagesFileLocation();
        AudiTrailDataLocation = GetMainAppCrystalT2()->GetAudiTrailDataLocation();
        AlarmMessageLocation = GetMainAppCrystalT2()->GetAlarmMessageLocationm();
        ProductDataFileLocation = GetMainAppCrystalT2()->GetPathNameProducts();
    }
    m_PopupDialogFileTransfer = (PopupDialogFileTransfer*)(parent);

    m_PopupProgressBar = new PopupProgressBar(this);
    m_PopupProgressBar->SetTitle(tr("Copy Files To USB"));

    m_USB_DirName = "default";

    m_RootPathDesitination = "E:/";
    m_RootPathSource = "D:/";

    m_DirModelSource = new FileSystemModelWithCheckBoxes(this, m_RootPathSource);
    m_DirModelDestination = new QFileSystemModel(this);

    m_ListSourceDirs.append(m_RootPathSource);
    m_ListSourceDirs.append(VideoFileLocation);
    m_ListSourceDirs.append(BottleEjectedLocation);
    m_ListSourceDirs.append(TrendGraphDataLocation);
    m_ListSourceDirs.append(TriggerFileLocation);
    m_ListSourceDirs.append(AudiTrailDataLocation);
    m_ListSourceDirs.append(AlarmMessageLocation);
    m_ListSourceDirs.append(ProductDataFileLocation);
    m_ListSourceDirs.append(ScreenshotPath);

    m_ModelSourceFilterProxy = new ModelSourceFilterProxy(this);
    m_ModelDestinationFilterProxy = new ModelDestinationFilterProxy(this);
    // Set filter
    m_DirModelSource->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
    m_DirModelDestination->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);

    scanSourceDirs();

    ui.treeViewSource->hideColumn(1);
    ui.treeViewSource->hideColumn(2);
    ui.treeViewSource->hideColumn(3);

    connect(ui.pushButtonScanUSB, &QPushButton::clicked, this, &FileTransferDialog::SlotScanUSB);
    connect(ui.pushButtonTransfer, &QPushButton::clicked, this, &FileTransferDialog::SlotTransferFiles);
    connect(ui.pushButtonCloseWindowAndEnableUSB, &QPushButton::clicked, this, &FileTransferDialog::SlotCloseDialogAndEnableUSB);

    connect(m_PopupProgressBar, &PopupProgressBar::SignalAbortIsClicked, this, &FileTransferDialog::SlotFileTransferAbortIsClicked);
    connect(this, &FileTransferDialog::SignalSetStatusFileTransfer, this, &FileTransferDialog::SlotSetStatusFileTransfer, Qt::QueuedConnection);

    SetAuditTrailProperties();
}

FileTransferDialog::~FileTransferDialog()
{
    m_AbortFileTransferIsClicked = true;
}

void FileTransferDialog::SetAuditTrailProperties()
{
    ui.pushButtonScanUSB->setProperty(kAuditTrail, ui.pushButtonScanUSB->text());
    ui.pushButtonTransfer->setProperty(kAuditTrail, ui.pushButtonTransfer->text());
}

void FileTransferDialog::SlotFileTransferAbortIsClicked()
{
    m_AbortFileTransferIsClicked = true;
}

void FileTransferDialog::showEvent(QShowEvent*)
{
    ui.treeViewSource->expandToDepth(0);
    SlotScanUSB();
}

bool FileTransferDialog::filterAcceptFilesAndDirsModelSource(QString& FilePath)
{
    QFileInfo fileInfo(FilePath);
    QStringList subDirs = FilePath.split("/");

    bool rv = false;
    if (fileInfo.isDir()) {
        for (int i = 0; i < m_ListSourceDirs.count(); i++) {
            if (FilePath.toUpper() == m_ListSourceDirs[i].toUpper()) {
                rv = true;
                break;
            }
        }
        if (!rv) {
            if (subDirs.count() > 2) {
                rv = true;
            }
        }
    } else {
        if (subDirs.count() > 2) {
            rv = true;
        }
    }
    return rv;
}

bool FileTransferDialog::filterAcceptFilesAndDirsModelDesination(QString& FileName, QString& FilePath)
{
    bool rv = false;
    if (m_EnableUSB) {
        QFileInfo fileInfo(FilePath);
        QString RootUSBPath = getRootUSBPath();
        if (!RootUSBPath.isEmpty()) {
            RootUSBPath = RootUSBPath.left(2);  // get only E: for exsample
        }
        if (!RootUSBPath.isEmpty() && FileName.contains(RootUSBPath)) {
            rv = true;
            m_USB_DirName = FilePath;
        } else {
            if ((fileInfo.isDir() || fileInfo.isFile()) && FilePath.contains(m_USB_DirName)) {
                rv = true;
            }
        }
    }
    return rv;
}

void FileTransferDialog::SlotCloseDialogAndEnableUSB()
{
    enableEjectUSD();
    if (m_PopupDialogFileTransfer) m_PopupDialogFileTransfer->close();
}

void FileTransferDialog::hideEvent(QHideEvent*)
{
    //enableEjectUSD();
}

void FileTransferDialog::usbDriveEject(const QString letter)
{
    TCHAR devicepath[16];
    QString device_path = QStringLiteral("%1\\").arg(letter);
    const char* temp = "\\\\.\\";
    char device_path1[10] = {0};
    memcpy(device_path1, temp, strlen(temp));
    QByteArray dp = device_path.toLocal8Bit();

    if (dp.size() > 1) 
    {
        device_path1[4] = dp.at(0);
        device_path1[5] = dp.at(1);
        DWORD dwRet = 0;

        HANDLE hVol = CreateFileA(device_path1, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (hVol == INVALID_HANDLE_VALUE) return;

        /*if (!DeviceIoControl(hVol, FSCTL_LOCK_VOLUME, 0, 0, 0, 0, &dwRet, 0)) {
            CloseHandle(hVol);
            return;
        }

        if (!DeviceIoControl(hVol, FSCTL_DISMOUNT_VOLUME, 0, 0, 0, 0, &dwRet, 0)) {
            CloseHandle(hVol);
            return;
        }
       */
        DeviceIoControl(hVol, IOCTL_STORAGE_EJECT_MEDIA, 0, 0, 0, 0, &dwRet, 0);

        CloseHandle(hVol);
    }
}

void FileTransferDialog::enableEjectUSD()
{
    m_EnableUSB = false;
    m_DirModelDestination->setRootPath(m_RootPathDesitination);
    m_ModelDestinationFilterProxy->setSourceModel(m_DirModelDestination);
    ui.treeViewDistination->setModel(m_ModelDestinationFilterProxy);
    ui.treeViewDistination->setRootIndex(m_DirModelDestination->index(m_DirModelDestination->rootPath()));
    QString RootPathDesitinationWithoutSlash = m_RootPathDesitination.left(2);
    usbDriveEject(RootPathDesitinationWithoutSlash);
}

void FileTransferDialog::SlotScanUSB()
{
    m_EnableUSB = true;
    m_RootPathDesitination = getRootUSBPath();

    if (!m_RootPathDesitination.isEmpty()) {
        if (m_DirModelDestination != NULL) {
            delete m_DirModelDestination;
        }
        m_DirModelDestination = new QFileSystemModel(this);
        m_DirModelDestination->setRootPath(m_RootPathDesitination);
        m_ModelDestinationFilterProxy->setSourceModel(m_DirModelDestination);
        ui.treeViewDistination->setModel(m_ModelDestinationFilterProxy);
        ui.treeViewDistination->setRootIndex(m_DirModelDestination->index(m_DirModelDestination->rootPath()));
        ui.treeViewDistination->hideColumn(1);
        ui.treeViewDistination->hideColumn(2);
        ui.treeViewDistination->hideColumn(3);
    }
}

void FileTransferDialog::scanSourceDirs()
{
    m_DirModelSource->setRootPath(m_RootPathSource);
    m_ModelSourceFilterProxy->setSourceModel(m_DirModelSource);
    ui.treeViewSource->setModel(m_ModelSourceFilterProxy);
    ui.treeViewSource->setRootIndex(m_DirModelSource->index(0, 0));
}

QString FileTransferDialog::getRootUSBPath()
{
    DWORD test = GetLogicalDrives();
    QString RootUSBPath;
    DWORD mask = 1;

    for (int i = 0; i < 32; i++) {
        if (test & mask) {
            QString wdrive((char)('A' + i));
            wdrive.append(":/");
            LPCWSTR drive = reinterpret_cast<LPCWSTR>(wdrive.utf16());
            if (GetDriveTypeW(drive) == DRIVE_REMOVABLE) {
                RootUSBPath = QString::fromWCharArray(drive);
                break;
            }
        }
        mask = mask << 1;
    }
    return RootUSBPath;
}

void FileTransferDialog::SlotSetStatusFileTransfer(const QString& Info, int value)
{
    if (m_PopupProgressBar) {
        QString text = Info;
        if (value < 100) {
            m_PopupProgressBar->SetInfoTextPleaseWait(text);
            m_PopupProgressBar->SlotSetValue(value);
        } else {
            m_PopupProgressBar->SlotSetValue(-1);  // hide dialog
        }
    }
}

void FileTransferDialog::SlotTransferFiles()
{
    QStringList FilesAndDirs = m_DirModelSource->GetCheckedFilesNandDirs();
    if (m_ModelDestinationFilterProxy->rowCount() > 0 && FilesAndDirs.count() > 0) {  // Wenn gleich Null dann kein USB-Stick aktiv
        m_AbortFileTransferIsClicked = false;
        SlotSetStatusFileTransfer(tr("Transfer Files To USB, Please Wait ...."), 0);
        if (m_ThreadCopyFiles == NULL) {
            m_ThreadCopyFiles = new ThreadCopyFiles(this);
        }
        m_ThreadCopyFiles->SetCopyFilesAndDirs(FilesAndDirs);
        m_ThreadCopyFiles->start();
    } else {
        if (FilesAndDirs.count() == 0) {
            BMessageBox::information(this, tr("Copy Files"), tr("Can Not Copy, No Files Selected"), QMessageBox::Ok);
        } else {
            BMessageBox::information(this, tr("Copy Files"), tr("Can Not Copy, USB-Stick Is Not Active"), QMessageBox::Ok);
        }
    }
}

bool FileTransferDialog::checkIsAbortClicked()
{
    if (m_AbortFileTransferIsClicked) {
        emit SignalSetStatusFileTransfer(QString(""), -1);
        return true;
    } else {
        return false;
    }
}

void FileTransferDialog::transferFiles(const QStringList& FilesAndDirs)
{
    QStringList ListOnlyFiles, ListOnlyDirs, ListDirSelectetWithoutFileSelection;

    m_TotalNumberFiles = m_FileTransferCounter = 0;
    for (const QString FileOrDir : FilesAndDirs) {
        if (FileOrDir != m_RootPathSource) {
            QFileInfo fileInfoFileOrdir(FileOrDir);

            if (fileInfoFileOrdir.isDir()) {
                ListOnlyDirs.append(FileOrDir);  // Liste der Verzeichnisse die markiert sind
            }
            if (fileInfoFileOrdir.isFile()) {
                ListOnlyFiles.append(FileOrDir);  // Liste der Dateien die markiert sind
            }
            if (checkIsAbortClicked()) {
                m_AbortFileTransferIsClicked = false;
                return;
            }
        }
    }
    // Prüfung der Verzeichnisse die markiert sind aber deren unterverzeichnisse/dateien nicht markiert sind
    for (QString DirName : ListOnlyDirs) {
        bool dirHasSelectedFiles = false;
        for (const QString FileName : ListOnlyFiles) {
            if (FileName.contains(DirName)) {
                dirHasSelectedFiles = true;
                break;
            }
            if (checkIsAbortClicked()) {
                m_AbortFileTransferIsClicked = false;
                return;
            }
        }
        if (!dirHasSelectedFiles) {
            ListDirSelectetWithoutFileSelection.append(DirName);
        }
    }
    for (QString DirName : ListDirSelectetWithoutFileSelection) {
        QDirIterator it(DirName, QStringList() << "*.*", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            ListOnlyFiles.append(it.next());
            if (checkIsAbortClicked()) {
                m_AbortFileTransferIsClicked = false;
                return;
            }
        }
    }
    // Anzahl der zu kopierenden Dateieen
    m_TotalNumberFiles = ListOnlyFiles.count();
    if (m_TotalNumberFiles > 0) {
        for (const QString FileName : ListOnlyFiles) {
            if (FileName != m_RootPathSource) {
                transferFile(FileName);
            }
            if (checkIsAbortClicked()) {
                m_AbortFileTransferIsClicked = false;
                return;
            }
        }
    }
    if (m_AbortFileTransferIsClicked) {
        m_AbortFileTransferIsClicked = false;
    }
    emit SignalSetStatusFileTransfer(tr("Copy Finished"), 100);
}

void FileTransferDialog::transferFile(const QString& SourceData)
{
    QFileInfo fileInfo(SourceData);
    if (fileInfo.isFile()) {
        QString destinationFile = SourceData;
        destinationFile.replace(m_RootPathSource, m_RootPathDesitination);
        QString targetPath(QFileInfo(destinationFile).absolutePath());
        QDir().mkpath(targetPath);
        QString file = fileInfo.absoluteFilePath();
        int length = file.count();
        int MaxSize = 100;
        int MaxSizeLeftRigth = MaxSize / 2 - 5;
        if (length > MaxSize) {
            QString SubStringFirst = file.left(MaxSizeLeftRigth);
            QString MiddelSpace = " . . . . ";
            QString SubStringLast = file.right(MaxSizeLeftRigth);
            file = SubStringFirst + MiddelSpace + SubStringLast;
            length = file.count();
        }
        int fillNumberSpace = MaxSize - length;
        for (int i = 0; i < fillNumberSpace; i++) {
            file.append(".");
        }
        length = file.count();
        double PercentValue = ((double)(m_FileTransferCounter) / m_TotalNumberFiles) * 100.0;
        emit SignalSetStatusFileTransfer(file, static_cast<int>(PercentValue));
        m_FileTransferCounter++;
        QFile::copy(SourceData, destinationFile);
    }
}
