#include "EjectedBottlesDialog.h"
#include "FileTransferDialog.h"
#include "GlobalConst.h"
#include "GraphicsVideoView.h"
#include "KitharaCore.h"
#include "MainAppCrystalT2.h"
#include "ProductData.h"
#include "QtConcurrent/qtconcurrentrun.h"
#include "SettingsData.h"
#include "VideoHeader.h"
#include "WaitForNewVideo.h"
#include "bmessagebox.h"

#include <audittrail.h>

EjectedBottlesDialog::EjectedBottlesDialog(MainAppCrystalT2* pMainAppCrystalT2)
    : QWidget(pMainAppCrystalT2),
      m_MainAppCrystalT2(NULL),
      m_GraphicsViewLeftTrigger(NULL),
      m_GraphicsViewRightTrigger(NULL),
      m_BreakVideo(false),
      m_BreakSaveTriggerImages(false),
      m_ShowVideo(true),
      m_BreakStopWaitForBottlesEjected(false),
      m_SaveVideoEjectedBottle(false),
      m_LastImageTimeStampInNs(0),
      m_CacheMaximumNumberEjectedBottles(100)
{
    ui.setupUi(this);
    m_ImageFileExtension = ".bmp";
    m_MainAppCrystalT2 = pMainAppCrystalT2;
    ui.checkBoxSelectShowTriggerImages->setChecked(true);
    ui.checkBoxSelectShowVideo->setChecked(false);
    m_ShowVideo = false;  // Videodatei speichern momentan immer aus
    m_GraphicsViewLeftTrigger = new GraphicsVideoView(this);
    m_GraphicsViewRightTrigger = new GraphicsVideoView(this);
    ui.comboBoxSelectErrorType->insertItem(ERROR_TYPE_SPLASHES_TOO_MUCH, tr("Too much splashes"));
    ui.comboBoxSelectErrorType->insertItem(ERROR_TYPE_FILLING_QUANTITY_NOT_OK, tr("Filling quantity not ok"));
    ui.comboBoxSelectErrorType->insertItem(ERROR_TYPE_BOTTLE_NOT_UNDER_VALVE, tr("Bottle not under valve"));

    ui.comboBoxSelectErrorType->setCurrentIndex(ERROR_TYPE_SPLASHES_TOO_MUCH);
    m_SelectedSubDirName = ERROR_TYPS_SUB_DIR_NAMES[ERROR_TYPE_SPLASHES_TOO_MUCH];
    if (ui.frameImageLeftTrigger->layout()) {
        ui.frameImageLeftTrigger->layout()->addWidget(m_GraphicsViewLeftTrigger);
    }
    if (ui.frameImageRightTrigger->layout()) {
        ui.frameImageRightTrigger->layout()->addWidget(m_GraphicsViewRightTrigger);
    }
    connect(ui.EjectedBottlesImageList, &QListWidget::itemClicked, this, &EjectedBottlesDialog::SlotShowSelectedDataSet);
    connect(ui.comboBoxSelectErrorType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EjectedBottlesDialog::SlotErrorTypeChanged);
    ui.EjectedBottlesImageList->setSelectionMode(QAbstractItemView::ContiguousSelection);
    connect(this, &EjectedBottlesDialog::SignalSetVideoImage, this, &EjectedBottlesDialog::SlotSetVideoImage, Qt::BlockingQueuedConnection);
    m_StartVideoThread = new QThread(this);
    moveToThread(m_StartVideoThread);

    connect(m_StartVideoThread, &QThread::started, this, &EjectedBottlesDialog::SlotStartVideo);
    connect(this, &EjectedBottlesDialog::SignalStartVideoReady, m_StartVideoThread, &QThread::quit);

    connect(ui.checkBoxSelectShowTriggerImages, &QCheckBox::stateChanged, this, &EjectedBottlesDialog::SlotSelectShowTriggerImages);
    connect(ui.checkBoxSelectShowVideo, &QCheckBox::stateChanged, this, &EjectedBottlesDialog::SlotSelectShowVideo);

    connect(ui.pushButtonDeleteDir, &QPushButton::clicked, this, &EjectedBottlesDialog::SlotDeleteDir);
    connect(ui.pushButtonFileTransfer, &QPushButton::clicked, this, &EjectedBottlesDialog::SlotOpenFileTransferDialog);

    ui.checkBoxSelectShowVideo->hide();
    ui.checkBoxSelectShowTriggerImages->hide();
    ui.labelSelectShowTriggerImages->hide();
    ui.labelSelectShowVideo->hide();
    SetAuditTrailProperties();
    SetRequiredAccessLevel();
}

EjectedBottlesDialog::~EjectedBottlesDialog()
{
}

void EjectedBottlesDialog::SlotOpenFileTransferDialog()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetPopupDialogFileTransfer()) {
        GetMainAppCrystalT2()->GetPopupDialogFileTransfer()->show();
    }
}

void EjectedBottlesDialog::SetRequiredAccessLevel()
{
    ui.pushButtonDeleteDir->setProperty(kRequiredAccessLevel, kAccessLevelUser);
    ui.pushButtonFileTransfer->setProperty(kRequiredAccessLevel, kAccessLevelUser);
}

void EjectedBottlesDialog::SetAuditTrailProperties()
{
    ui.comboBoxSelectErrorType->setProperty(kAuditTrail, ui.labelSelectErrorType->text());
    ui.pushButtonDeleteDir->setProperty(kAuditTrail, ui.pushButtonDeleteDir->text());
    ui.pushButtonFileTransfer->setProperty(kAuditTrail, ui.pushButtonFileTransfer->text());
}

// wird in der MainAppCrystalT2 aufgerufen wenn das Programm gestartet wird -> void MainAppCrystalT2::StartInitRealTimeSystem()
void EjectedBottlesDialog::StartWaitForBottlesEjected()
{
    m_WaitForBottleEjected = QtConcurrent::run(this, &EjectedBottlesDialog::WaitForBottlesEjectedAndCopyData);  // Warten auf Flaschenauswurf und einlesen der Bilddaten
    m_WriteTriggerImages = QtConcurrent::run(this, &EjectedBottlesDialog::SaveTriggerdImages);                  // Thread der das Speichern der Bilder/Video übernimmt wenn eine Flasche ausgeworfen
}

void EjectedBottlesDialog::StopRunnigThreads()
{
    if (m_FutureRunVideoFillingProcess.isRunning()) {
        m_BreakVideo = true;
        m_FutureRunVideoFillingProcess.cancel();
    }
    if (m_WaitForBottleEjected.isRunning()) {
        m_BreakStopWaitForBottlesEjected = true;
        if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetKitharaCore()) {
            GetMainAppCrystalT2()->GetKitharaCore()->ForceSetEventBottleEjected();
        }
        m_WaitForBottleEjected.cancel();
    }
    if (m_WriteTriggerImages.isRunning()) {
        m_BreakSaveTriggerImages = true;
        m_WriteTriggerImages.cancel();
    }
}

void EjectedBottlesDialog::SlotSelectShowTriggerImages(int state)
{
    if (state == Qt::Checked) {
        ui.groupBoxShowRightTrigger->show();
        ui.textEditResultRightTrigger->show();
        m_ShowVideo = false;

        disconnect(ui.checkBoxSelectShowVideo, &QCheckBox::stateChanged, this, &EjectedBottlesDialog::SlotSelectShowVideo);
        ui.checkBoxSelectShowVideo->setChecked(false);
        connect(ui.checkBoxSelectShowVideo, &QCheckBox::stateChanged, this, &EjectedBottlesDialog::SlotSelectShowVideo);
        m_BreakVideo = true;
        QListWidgetItem* item = ui.EjectedBottlesImageList->currentItem();
        if (item) {
            SlotShowSelectedDataSet(item);
        }
    } else {
        ui.checkBoxSelectShowVideo->setChecked(true);
    }
}

void EjectedBottlesDialog::SlotSelectShowVideo(int state)
{
    if (state == Qt::Checked) {
        ui.groupBoxShowRightTrigger->hide();
        ui.textEditResultRightTrigger->hide();
        m_ShowVideo = true;

        disconnect(ui.checkBoxSelectShowTriggerImages, &QCheckBox::stateChanged, this, &EjectedBottlesDialog::SlotSelectShowTriggerImages);
        ui.checkBoxSelectShowTriggerImages->setChecked(false);
        connect(ui.checkBoxSelectShowTriggerImages, &QCheckBox::stateChanged, this, &EjectedBottlesDialog::SlotSelectShowTriggerImages);
        QListWidgetItem* item = ui.EjectedBottlesImageList->currentItem();
        if (item) {
            SlotShowSelectedDataSet(item);
        }
    } else {
        ui.checkBoxSelectShowTriggerImages->setChecked(true);
    }
}

void EjectedBottlesDialog::showEvent(QShowEvent* ev)
{
    LoadEjectedBottlesList();
}

void EjectedBottlesDialog::SlotSetVideoImage(const QImage& Image)
{
    SetImageLeftTriggerOn(Image);
}

void EjectedBottlesDialog::SlotErrorTypeChanged(int index)
{
    switch (index) {
        case ERROR_TYPE_SPLASHES_TOO_MUCH:
            m_SelectedSubDirName = ERROR_TYPS_SUB_DIR_NAMES[index];
            break;
        case ERROR_TYPE_FILLING_QUANTITY_NOT_OK:
            m_SelectedSubDirName = ERROR_TYPS_SUB_DIR_NAMES[index];
            break;
        case ERROR_TYPE_BOTTLE_NOT_UNDER_VALVE:
            m_SelectedSubDirName = ERROR_TYPS_SUB_DIR_NAMES[index];
            break;
        default:
            m_SelectedSubDirName = ERROR_TYPS_SUB_DIR_NAMES[ERROR_TYPE_SPLASHES_TOO_MUCH];
            break;
    }
    LoadEjectedBottlesList();
}

void EjectedBottlesDialog::SetImageLeftTriggerOn(const QImage& Image)
{
    if (m_GraphicsViewLeftTrigger) {
        m_GraphicsViewLeftTrigger->setImage(Image, 1.0, 1.0);
    }
}

void EjectedBottlesDialog::SetImageRightTriggerOn(const QImage& Image)
{
    if (m_GraphicsViewRightTrigger) {
        m_GraphicsViewRightTrigger->setImage(Image, 1.0, 1.0);
    }
}

void EjectedBottlesDialog::LoadEjectedBottlesList()
{
    QString DirName = GetMainAppCrystalT2()->GetErrorImagePoolLocation() + m_SelectedSubDirName;
    QDir EjectedBottlesDir(DirName);

    QFileInfoList dirList = EjectedBottlesDir.entryInfoList(QStringList("*"), QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::Name | QDir::IgnoreCase | QDir::Reversed);

    ui.EjectedBottlesImageList->clear();
    if (dirList.count() == 0) {
        ui.EjectedBottlesImageList->addItem(tr("No entries"));
    } else {
        ui.EjectedBottlesImageList->addItem(tr("Number entries:%1").arg(dirList.count()));
    }

    foreach (QFileInfo dir, dirList) {
        ui.EjectedBottlesImageList->addItem(dir.baseName());
    }
}

void EjectedBottlesDialog::SlotDeleteDir()
{
    QString text = ui.comboBoxSelectErrorType->currentText();
    if (QMessageBox::Yes == BMessageBox::information(this, tr("Delete Directory"), tr("Delete Directory -> %1?").arg(text), QMessageBox::Yes | QMessageBox::No)) {
        DeleteDir();
        LoadEjectedBottlesList();
    }
}

void EjectedBottlesDialog::DeleteDir()
{
    QString SubDir = GetMainAppCrystalT2()->GetErrorImagePoolLocation() + m_SelectedSubDirName;
    QDir ImageDir(SubDir);
    ImageDir.removeRecursively();
}

void EjectedBottlesDialog::SlotShowSelectedDataSet(QListWidgetItem* item)
{
    QString SubDir = GetMainAppCrystalT2()->GetErrorImagePoolLocation() + m_SelectedSubDirName + QString("/") + item->text();
    QDir ImageDir(SubDir);
    QStringList filter;

    m_BreakVideo = true;
    filter << "*.avi";
    QFileInfoList FileInfoList = ImageDir.entryInfoList(filter, QDir::Files | QDir::NoDotAndDotDot);
    if (FileInfoList.count() > 0) {
        ui.checkBoxSelectShowVideo->show();
        ui.checkBoxSelectShowTriggerImages->show();
        ui.labelSelectShowTriggerImages->show();
        ui.labelSelectShowVideo->show();
    } else {
        ui.checkBoxSelectShowVideo->hide();
        ui.checkBoxSelectShowTriggerImages->hide();
        ui.labelSelectShowTriggerImages->hide();
        ui.labelSelectShowVideo->hide();

        ui.groupBoxShowLeftTrigger->show();
        ui.textEditResultLeftTrigger->show();
        m_ShowVideo = false;

        disconnect(ui.checkBoxSelectShowTriggerImages, &QCheckBox::stateChanged, this, &EjectedBottlesDialog::SlotSelectShowTriggerImages);
        ui.checkBoxSelectShowTriggerImages->setChecked(true);
        connect(ui.checkBoxSelectShowTriggerImages, &QCheckBox::stateChanged, this, &EjectedBottlesDialog::SlotSelectShowTriggerImages);

        disconnect(ui.checkBoxSelectShowVideo, &QCheckBox::stateChanged, this, &EjectedBottlesDialog::SlotSelectShowVideo);
        ui.checkBoxSelectShowVideo->setChecked(false);
        connect(ui.checkBoxSelectShowVideo, &QCheckBox::stateChanged, this, &EjectedBottlesDialog::SlotSelectShowVideo);
    }
    if (!m_ShowVideo) {
        filter.clear();
        filter << QString("*%1").arg(m_ImageFileExtension);
        FileInfoList = ImageDir.entryInfoList(filter, QDir::Files | QDir::NoDotAndDotDot);
    }

    SetImageLeftTriggerOn(QImage());
    SetImageRightTriggerOn(QImage());
    ui.textEditResultLeftTrigger->clear();
    ui.textEditResultRightTrigger->clear();

    QImage TriggerImage;
    QString resultText;
    QString Filename;
    QStringList listErrorText;
    if (!m_ShowVideo) {
        foreach (QFileInfo file, FileInfoList) {
            Filename = file.absoluteFilePath();
            listErrorText.clear();
            TriggerImage.load(Filename);
            Filename = Filename.replace(m_ImageFileExtension, ".txt");

            QFile ResultFile(Filename);
            if (ResultFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream instream(&ResultFile);
                while (!instream.atEnd()) listErrorText.append(instream.readLine());
                ResultFile.close();
            }
            if (Filename.contains("Left")) {
                SetImageLeftTriggerOn(TriggerImage);
                foreach (QString line, listErrorText)
                    ui.textEditResultLeftTrigger->append(line);
            } else {
                SetImageRightTriggerOn(TriggerImage);
                foreach (QString line, listErrorText)
                    ui.textEditResultRightTrigger->append(line);
            }
        }
    } else {  // zweig wird momentan nicht erreicht da kein Videoabgespeichert wird bzw. deaktiviert
        if (FileInfoList.count() > 0) {
            QFileInfo fileInfo = FileInfoList.at(0);
            QStringList FullListErrorText;
            m_VideoFilename = fileInfo.absoluteFilePath();
            if (m_StartVideoThread) {
                m_StartVideoThread->start();
            }
            filter.clear();
            filter << "*.txt";
            FileInfoList = ImageDir.entryInfoList(filter, QDir::Files | QDir::NoDotAndDotDot);

            foreach (QFileInfo file, FileInfoList) {
                listErrorText.clear();
                Filename = file.absoluteFilePath();
                QFile ResultFile(Filename);
                if (ResultFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream instream(&ResultFile);
                    while (!instream.atEnd()) listErrorText.append(instream.readLine());
                    ResultFile.close();
                }
                foreach (QString line, listErrorText) {
                    if (!FullListErrorText.contains(line)) FullListErrorText.append(line);
                }
            }
            foreach (QString line, FullListErrorText) {
                ui.textEditResultLeftTrigger->append(line);
            }
        }
    }
}

// Wartet wenn eine Flasche vom System ausgeworfen wird, aber nur dann wenn System auf Produktion steht
void EjectedBottlesDialog::WaitForBottlesEjectedAndCopyData()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetKitharaCore()) {
        int rv;
        QString Msg;
       
        while (true) {
            if (m_BreakStopWaitForBottlesEjected) break;
            rv = GetMainAppCrystalT2()->GetKitharaCore()->WaitForBottleEjected(2000, Msg);  // Warte auf Flaschenauswurf
            if (rv == ERROR_CODE_NO_ERROR) {                                                // Eine Flasche soll ausgeworfen werden
                if (m_BreakStopWaitForBottlesEjected) break;
                CopyVideoDataBottleEject();  // Auslesen der Bilder/Video wenn Flasche ausgeworfen
            }
        }
    }
    emit SignalWaitForBottlesEjectedFinished();
}

void EjectedBottlesDialog::CopyVideoDataBottleEject()
{
    if (m_ListSavedTriggerImages.count() < m_CacheMaximumNumberEjectedBottles) {
        VideoHeader videoHeader;
        ImageHeader imageHeader;
        bool foundEjectedBottle = false;
        QString DirName;
        QList<QPair<QString, int>> listErrorStrings;

        if (GetMainAppCrystalT2()->GetCurrentMaschineState() == PluginInterface::MachineState::Production) {
            if (GetMainAppCrystalT2()->GetKitharaCore()) {
                unsigned char* pRawVideodata = GetMainAppCrystalT2()->GetKitharaCore()->GetRawVideodataBottleEjected();  // Zeiger auf seperaten Shared Memory für den Flaschenauswurf
                if (pRawVideodata != NULL) {
                    memcpy(&videoHeader, pRawVideodata, sizeof(VideoHeader));  // copiere VideoHeader vom Realtimekontext
                    m_StartVideoIndex = videoHeader.m_FrameIndex;              // Ist der Index des zuletzt aufgenommen Bildes
                    m_ImageBlockSize = (videoHeader.m_ImageWidth * videoHeader.m_ImageHeight) + sizeof(ImageHeader);
                    m_CurrentNumberFrames = videoHeader.m_CurrentNumberFrames;  // Aktuelle Anzahl von Bildern im Sharedmemory
                    m_MaxNumberFrames = videoHeader.m_MaxNumberFrames;          // ist immer Konstant, wird beim Anlegen des Shared Memory festgelegt, bzw. ist in der Konfiguration definiert
                    m_MaxBlockSize = m_CurrentNumberFrames * m_ImageBlockSize;
                    m_BlockSizeBottom = m_MaxBlockSize - (m_ImageBlockSize * m_StartVideoIndex);
                    m_BlockSizeTop = m_MaxBlockSize - m_BlockSizeBottom;
                    m_CurrentSpeedInmmPerms = videoHeader.m_CurrentSpeedInmmPerms;
                    m_ImageTimeIntervalInMs = (videoHeader.m_CurrentTimeStampInns - videoHeader.m_LastTimeStampInns) / ((double)(1000000.0));
                    // Erster Schritt suche Position im Speicher bei dem die Entscheidung getroffen wurde das die Flasche ausgeworfen wird
                    int StartIndex = m_CurrentNumberFrames - 1;
                    for (int i = m_CurrentNumberFrames - 1; i >= 0; i--) {
                        unsigned char* pImageStartPointer = GetImageStartPointer(i);
                        if (pImageStartPointer) {
                            memcpy(&imageHeader, pImageStartPointer - sizeof(ImageHeader), sizeof(ImageHeader));
                            if (imageHeader.m_TimeStampInns > m_LastImageTimeStampInNs) {  // sicherstellen das ein Flaschenauswurf nicht doppelt gespeichert wird
                                if (imageHeader.m_ProductionOn && (imageHeader.m_ImageState & IMAGE_CAPTURE_TYPE_BOTTLE_EJECTED) &&
                                    (imageHeader.m_ImageState & IMAGE_CAPTURE_TYPE_LIQUID_MEASURING_READY)) {  // Bei diesem Bild ist der Befüllvorgang beendet. Zu diesem Zeitpunkt ist die
                                                                                                               // Entscheidung getroffen worden die Flasche auszuwerfen
                                    StartIndex = i;
                                    m_LastImageTimeStampInNs = imageHeader.m_TimeStampInns;
                                    foundEjectedBottle = true;
                                    int ErrorType = ERROR_TYPE_SPLASHES_TOO_MUCH;
                                    MeasuringResultsLiquid CurrentMeasuringResultsLiqiud = imageHeader.m_CurrentMeasuringResultsLiqiud;
                                    listErrorStrings = GetResultsLiquidMeasuringAsString(CurrentMeasuringResultsLiqiud, ErrorType);
                                    QString SubDirName = ERROR_TYPS_SUB_DIR_NAMES[ErrorType];
                                    QString Time = QDateTime::currentDateTime().time().toString("hh-mm-ss-zzz");
                                    QString Date = QDateTime::currentDateTime().date().toString("dd-MM-yyyy");
                                    DirName = GetMainAppCrystalT2()->GetErrorImagePoolLocation() + SubDirName + "/ImagesEjectedBottle" + Date + QString("_") + Time;
                                    break;
                                }
                            }
                        }
                    }
                    if (foundEjectedBottle) {
                        bool foundLeftTriggerImage = false;
                        bool foundRightTriggerImage = false;
                        if (!GetMainAppCrystalT2()->WorkWithTwoValves()) {
                            foundRightTriggerImage = true;
                        }
                        SetTriggerImagesIntoList(StartIndex, listErrorStrings, DirName, foundLeftTriggerImage, foundRightTriggerImage);
                        if ((!foundLeftTriggerImage && foundRightTriggerImage) || (foundLeftTriggerImage && !foundRightTriggerImage)) {
                            if (m_ListSavedTriggerImages.size() > 0) {
                                m_ListSavedTriggerImages.removeLast();
                            }
                            if (m_ListSavedErrorText.size() > 0) {
                                m_ListSavedErrorText.removeLast();
                            }
                            QDir dir(DirName);
                            dir.removeRecursively();
                        }
                        if ((!foundLeftTriggerImage && !foundRightTriggerImage)) {
                            QDir dir(DirName);
                            dir.removeRecursively();
                        }
                    }
                }
            }
        }
    } else {
        qDebug() << "Cache is full";
    }
}

void EjectedBottlesDialog::SetTriggerImagesIntoList(int StartIndex, QList<QPair<QString, int>>& listErrorStrings, const QString& DirName, bool& foundLeft, bool& foundRight)
{
    ImageHeader imageHeader;
    int EndIndex = 0;
    int IndexLeftTrigger = -1;
    int IndexRightTrigger = -1;

    // Aktuelle Videoposition auslesen
    int TimeOffsetLeftVideoPosInms = GetCurrentVideoTimeOffset(LEFT_VIDEO_POS);
    int TimeOffsetRightVideoPosInms = GetCurrentVideoTimeOffset(RIGHT_VIDEO_POS);

    for (; StartIndex > EndIndex; StartIndex--) {
        unsigned char* pImageStartPointer = GetImageStartPointer(StartIndex) - sizeof(ImageHeader);
        if (pImageStartPointer) {
            memcpy(&imageHeader, pImageStartPointer, sizeof(ImageHeader));
            if (!foundLeft && imageHeader.m_ProductionOn && (imageHeader.m_ImageState & IMAGE_CAPTURE_TYPE_LEFT_TRIGGER_IS_SET)) {
                foundLeft = true;
                IndexLeftTrigger = StartIndex;
                pImageStartPointer = GetImageStartPointerByTimeOffset(IndexLeftTrigger, TimeOffsetLeftVideoPosInms);
                if (pImageStartPointer) {
                    QPair<cv::Mat, QString> ImageAndPath;
                    ImageAndPath.first = cv::Mat(USED_CAMERA_HEIGHT, USED_CAMERA_WIDTH, CV_8U, (void*)(pImageStartPointer), USED_CAMERA_WIDTH);
                    DrawLiqudWidowIntoErrorImage(ImageAndPath.first, imageHeader.m_CameraImageID, imageHeader);
                    ImageAndPath.first = ImageAndPath.first.clone();
                    ImageAndPath.second = DirName + QString("/LeftValve%1").arg(m_ImageFileExtension);
                    m_ListSavedTriggerImages.append(ImageAndPath);
                }
                QPair<QString, QString> ResultsAndPath;
                ResultsAndPath.first = GetErrorStringLeftTriggerOn(listErrorStrings);
                ResultsAndPath.second = DirName + QString("/LeftValve.txt");
                m_ListSavedErrorText.append(ResultsAndPath);
            }
            if (!foundRight && imageHeader.m_ProductionOn && (imageHeader.m_ImageState & IMAGE_CAPTURE_TYPE_RIGHT_TRIGGER_IS_SET)) {
                foundRight = true;
                IndexRightTrigger = StartIndex;
                pImageStartPointer = GetImageStartPointerByTimeOffset(IndexRightTrigger, TimeOffsetRightVideoPosInms);
                if (pImageStartPointer) {
                    QPair<cv::Mat, QString> ImageAndPath;
                    ImageAndPath.first = cv::Mat(USED_CAMERA_HEIGHT, USED_CAMERA_WIDTH, CV_8U, (void*)(pImageStartPointer), USED_CAMERA_WIDTH);
                    DrawLiqudWidowIntoErrorImage(ImageAndPath.first, imageHeader.m_CameraImageID, imageHeader);
                    ImageAndPath.first = ImageAndPath.first.clone();
                    ImageAndPath.second = DirName + QString("/RightValve%1").arg(m_ImageFileExtension);
                    m_ListSavedTriggerImages.append(ImageAndPath);
                }
                QPair<QString, QString> ResultsAndPath;
                ResultsAndPath.first = GetErrorStringRightTriggerOn(listErrorStrings);
                ResultsAndPath.second = DirName + QString("/RightValve.txt");
                m_ListSavedErrorText.append(ResultsAndPath);
            }
            if (foundLeft && foundRight) break;
        }
    }
}

int EjectedBottlesDialog::GetCurrentVideoTimeOffset(int TriggerPos)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetWaitForNewVideo()) {
        return GetMainAppCrystalT2()->GetWaitForNewVideo()->GetCurrentVideoTimeOffset(TriggerPos);
    } else {
        return 0;
    }
}

unsigned char* EjectedBottlesDialog::GetSharedMemoryPointer()
{
    if (GetMainAppCrystalT2()->GetKitharaCore() && GetMainAppCrystalT2()->GetKitharaCore()->GetRawVideodataBottleEjected()) {
        return GetMainAppCrystalT2()->GetKitharaCore()->GetRawVideodataBottleEjected();
    } else {
        return NULL;
    }
}

unsigned char* EjectedBottlesDialog::GetImageStartPointer(int ImageIndex)
{
    unsigned char* CurrentAddress = NULL;
    unsigned char* StartAddress = GetSharedMemoryPointer() + sizeof(VideoHeader) + sizeof(ImageHeader);
    int ImageOffsetInBytes = ImageIndex * m_ImageBlockSize;  // ImageBlockSize ist Bildgroesse + Bildheader

    if (m_CurrentNumberFrames >= m_MaxNumberFrames) {  // Videospeicher ist voll, ein neues Bild überschreibt das älteste Bild
        if (ImageIndex < m_MaxNumberFrames) {
            CurrentAddress = StartAddress + m_BlockSizeTop + ImageOffsetInBytes;  // BlockSizeTop-Position ist beginn des Videos, StartPointer ist der Begin des Videospeicher des ersten Bildes
            unsigned char* EndAddress = StartAddress + m_BlockSizeTop + m_BlockSizeBottom;
            if (CurrentAddress >= EndAddress)  // Ist die berechnete Adresse groesser oder gleich der Endadresse
                CurrentAddress = StartAddress + (ImageOffsetInBytes - m_BlockSizeBottom);
        }
    } else {  // Videospeicher ist noch nicht voll, dann Berechnung einfach
        if (ImageIndex < m_CurrentNumberFrames) CurrentAddress = StartAddress + ImageOffsetInBytes;
    }
    return CurrentAddress;
}

unsigned char* EjectedBottlesDialog::GetImageStartPointerByTimeOffset(int StartImageIndexTrigger, long long TimeOffsetInms)
{
    unsigned char* CurrentAddress = NULL;
    ImageHeader imageHeader;
    unsigned long long TargetTimeStampInns;
    unsigned long long TriggerTimeStampInns;
    int DeltaT;
    unsigned int LastDeltaT;
    bool Addressok = false;

    LastDeltaT = UINT_MAX;
    CurrentAddress = GetImageStartPointer(StartImageIndexTrigger) - sizeof(ImageHeader);  // Image address triggerpoint
    if (CurrentAddress) {
        memcpy(&imageHeader, CurrentAddress, sizeof(ImageHeader));
        TriggerTimeStampInns = imageHeader.m_TimeStampInns;
        TargetTimeStampInns = TriggerTimeStampInns + (TimeOffsetInms * 1000000);  // triggertimestamp + offset
        if (TimeOffsetInms >= 0) {
            for (int StartIndex = StartImageIndexTrigger; StartIndex < GetCurrentNumberFrames(); StartIndex++) {
                CurrentAddress = GetImageStartPointer(StartIndex) - sizeof(ImageHeader);
                if (CurrentAddress) {
                    memcpy(&imageHeader, CurrentAddress, sizeof(ImageHeader));
                    DeltaT = imageHeader.m_TimeStampInns - TargetTimeStampInns;
                    if (DeltaT >= 0) {
                        if (abs(DeltaT) <= LastDeltaT)
                            CurrentAddress = CurrentAddress + sizeof(ImageHeader);
                        else
                            CurrentAddress = GetImageStartPointer(StartIndex - 1);
                        Addressok = true;
                        break;
                    }
                    LastDeltaT = abs(DeltaT);
                }
            }
        } else {
            for (int StartIndex = 0; StartIndex < StartImageIndexTrigger; StartIndex++) {
                CurrentAddress = GetImageStartPointer(StartIndex) - sizeof(ImageHeader);
                if (CurrentAddress) {
                    memcpy(&imageHeader, CurrentAddress, sizeof(ImageHeader));
                    DeltaT = imageHeader.m_TimeStampInns - TargetTimeStampInns;
                    if (DeltaT >= 0) {
                        if (abs(DeltaT) <= LastDeltaT)
                            CurrentAddress = CurrentAddress + sizeof(ImageHeader);
                        else
                            CurrentAddress = GetImageStartPointer(StartIndex - 1);
                        Addressok = true;
                        break;
                    }
                    LastDeltaT = abs(DeltaT);
                }
            }
        }
        if (Addressok) {
            return CurrentAddress;
        } else {
            return NULL;
        }
    } else {
        return NULL;
    }
}

void EjectedBottlesDialog::SaveTriggerdImages()
{
    bool dirExist = false;
    bool saveNewSet = false;
    while (true) {
        saveNewSet = false;
        dirExist = false;
        if (m_BreakSaveTriggerImages) break;
        for (int i = 0; i < 2; i++) {
            if (m_ListSavedTriggerImages.count() > 0) {
                if (!dirExist) {
                    QDir d = QFileInfo(m_ListSavedTriggerImages.at(0).second).absoluteDir();
                    QString DirName = d.absolutePath();
                    QDir().mkpath(DirName);
                    dirExist = true;
                }

                cv::imwrite(m_ListSavedTriggerImages.at(0).second.toLatin1().data(), m_ListSavedTriggerImages.at(0).first);
                m_ListSavedTriggerImages.removeFirst();
                saveNewSet = true;
            }
            if (m_ListSavedErrorText.count() > 0) {
                if (!dirExist) {
                    QDir d = QFileInfo(m_ListSavedErrorText.at(0).second).absoluteDir();
                    QString DirName = d.absolutePath();
                    QDir().mkpath(DirName);
                    dirExist = true;
                }

                QFile file(m_ListSavedErrorText.at(0).second);
                if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream outstream(&file);
                    outstream << m_ListSavedErrorText.at(0).first;
                    file.close();
                }
                m_ListSavedErrorText.removeFirst();
            }
        }
        if (m_ListSavedFillingProcessVideos.count() > 0) {
            cv::VideoWriter video;
            video.open(m_ListSavedFillingProcessVideos.at(0).second.toLatin1().data(), cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, cv::Size(USED_CAMERA_WIDTH, USED_CAMERA_HEIGHT));
            if (video.isOpened()) {
                for (int i = m_ListSavedFillingProcessVideos.at(0).first.count() - 1; i >= 0; i--) {
                    cv::Mat colorFrame;
                    cvtColor(m_ListSavedFillingProcessVideos.at(0).first.at(i), colorFrame, cv::COLOR_GRAY2BGR);
                    video.write(colorFrame);
                }
                video.release();
            }
            m_ListSavedFillingProcessVideos.removeFirst();
        }
        if (saveNewSet) emit SignalSaveNewSet();
        QThread::currentThread()->msleep(1000);
    }
}

void EjectedBottlesDialog::DrawLiqudWidowIntoErrorImage(const cv::Mat& MatImage, int index, ImageHeader& imageHeader)
{
    if (GetMainAppCrystalT2()) {
        QRect liquidRect = GetMainAppCrystalT2()->GetMeasureWindowRect(ROI_ID_MEASURE_LIQUID);
        QPair<QRect, QRect> FlowChannelRects = GetMainAppCrystalT2()->GetLeftRighFlowChannelLiquid(liquidRect);

        cv::rectangle(MatImage, cv::Point(liquidRect.topLeft().x(), liquidRect.topLeft().y()), cv::Point(liquidRect.bottomRight().x(), liquidRect.bottomRight().y()), cv::Scalar(128, 128, 128), 1, 8,
                      0);
        cv::rectangle(MatImage, cv::Point(FlowChannelRects.first.topLeft().x(), FlowChannelRects.first.topLeft().y()),
                      cv::Point(FlowChannelRects.first.bottomRight().x(), FlowChannelRects.first.bottomRight().y()), cv::Scalar(128, 128, 128), 1, 8, 0);
        cv::rectangle(MatImage, cv::Point(FlowChannelRects.second.topLeft().x(), FlowChannelRects.second.topLeft().y()),
                      cv::Point(FlowChannelRects.second.bottomRight().x(), FlowChannelRects.second.bottomRight().y()), cv::Scalar(128, 128, 128), 1, 8, 0);

        if (GetMainAppCrystalT2()->GetInfoLevel() == INFO_LEVEL_SHOW_INFO_DATA_EJECTED_BOTTLE) {
            if (index != -1) {
                QString IndexNumber = QString("ID:%1 ").arg(index);
                if (imageHeader.m_ImageState & IMAGE_CAPTURE_TYPE_LEFT_TRIGGER_IS_SET) {
                    IndexNumber = IndexNumber + QString("Left ");
                }
                if (imageHeader.m_ImageState & IMAGE_CAPTURE_TYPE_RIGHT_TRIGGER_IS_SET) {
                    IndexNumber = IndexNumber + QString("Right ");
                }
                if (imageHeader.m_ImageState & IMAGE_CAPTURE_TYPE_LIQUID_MEASURING_READY) {
                    IndexNumber = IndexNumber + QString("Ready ");
                }
                if ((imageHeader.m_ImageState & IMAGE_CAPTURE_TYPE_BOTTLE_EJECTED) && (imageHeader.m_ImageState & IMAGE_CAPTURE_TYPE_LIQUID_MEASURING_READY)) {
                    IndexNumber = IndexNumber + QString("Eject ");
                }
                IndexNumber = IndexNumber + QString("StateHex:%1 ").arg(imageHeader.m_ImageState, 2, 16);
                cv::putText(MatImage, IndexNumber.toLatin1().data(), cv::Point(liquidRect.x() + 5, 20), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(128, 128, 128), 0);
            }
        }
    }
}

QString EjectedBottlesDialog::GetErrorStringLeftTriggerOn(QList<QPair<QString, int>>& listErrorText)
{
    QString ErrorText;

    for (int i = 0; i < listErrorText.count(); i++) {
        if (listErrorText.at(i).second == ErrorTextAssignment::BOTH_TRIGGER_SIDE || listErrorText.at(i).second == ErrorTextAssignment::LEFT_TRIGGER_SIDE) {
            ErrorText = ErrorText + listErrorText.at(i).first;
        }
    }
    return ErrorText;
}

QString EjectedBottlesDialog::GetErrorStringRightTriggerOn(QList<QPair<QString, int>>& listErrorText)
{
    QString ErrorText;

    for (int i = 0; i < listErrorText.count(); i++) {
        if (listErrorText.at(i).second == ErrorTextAssignment::BOTH_TRIGGER_SIDE || listErrorText.at(i).second == ErrorTextAssignment::RIGHT_TRIGGER_SIDE) {
            ErrorText = ErrorText + listErrorText.at(i).first;
        }
    }
    return ErrorText;
}

//       Valves on left side
//    IsFirstTriggerOnLeftSide = false
//
// right valve         left valve
//   ______              ______
//  |      |            |      |
//  |      |            |      |     <--- first valve
//   |    |              |    |
//    |  |                |  |
//
// _______________________________
//|   |  |                |  |    |
//|___|__|________________|__|____|
//  L       R          L       R    <-- splashes side
//                                        <--- bottles from right to left
//                                        ____________         ____________
//                                       |            |       |            |
//
//////////////////////////////////////////////////////////////////////////////////
//                                                Valves on right side
//                                            IsFirstTriggerOnLeftSide = true
//
//                                            left valve            right valve
//                                               ______              ______
//                       first valve --->       |      |            |      |
//                                              |      |            |      |
//                                               |    |              |    |
//                                                |  |                |  |
//                                             _______________________________
//                                            |   |  |                |  |    |
//                                            |___|__|________________|__|____|
//                           splashes side->    L       R          L        R
//    bottles from left to right -->
//  ____________         ____________
// |            |       |            |
//
QList<QPair<QString, int>> EjectedBottlesDialog::GetResultsLiquidMeasuringAsString(MeasuringResultsLiquid& results, int& ErrorType)
{
    QString ErrorText = "Error results:\n";
    int ThresholdSplashes = 200;
    ProductData* pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
    QList<QPair<QString, int>> listErrorText;
    double MinAcceptanceThresholdLiquidMiddleROI = 0.0;
    double MaxAcceptanceThresholdLiquidMiddleROI = DBL_MAX;

    if (pProductData) {
        ThresholdSplashes = pProductData->m_AcceptanceThresholdLiquidLeftAndRightROI;
        MinAcceptanceThresholdLiquidMiddleROI = pProductData->m_MinAcceptanceThresholdLiquidMiddleROI;
        MaxAcceptanceThresholdLiquidMiddleROI = pProductData->m_MaxAcceptanceThresholdLiquidMiddleROI;
    }

    if (!results.m_AmountSplashLeftOk) {
        QPair<QString, int> Value;
        if (results.m_MaxAmountSplashLeftROIPerSide[LEFT_TRIGGER_SIDE_INDEX] > ThresholdSplashes) {
            Value.second = ErrorTextAssignment::LEFT_TRIGGER_SIDE;
            Value.first = tr("Too many splashes(%1) left valve on left side. Threshold %2").arg(results.m_MaxAmountSplashLeftROIPerSide[LEFT_TRIGGER_SIDE_INDEX]).arg(ThresholdSplashes);
            Value.first.append("\n");
            listErrorText.append(Value);
        }
        if (results.m_MaxAmountSplashLeftROIPerSide[RIGHT_TRIGGER_SIDE_INDEX] > ThresholdSplashes) {
            Value.second = ErrorTextAssignment::RIGHT_TRIGGER_SIDE;
            Value.first = tr("Too many splashes(%1) right valve on left side. Threshold %2").arg(results.m_MaxAmountSplashLeftROIPerSide[RIGHT_TRIGGER_SIDE_INDEX]).arg(ThresholdSplashes);
            Value.first.append("\n");
            listErrorText.append(Value);
        }
        ErrorType = ERROR_TYPE_SPLASHES_TOO_MUCH;
    }
    if (!results.m_AmountSplashRightOk) {
        QPair<QString, int> Value;
        if (results.m_MaxAmountSplashRightROIPerSide[LEFT_TRIGGER_SIDE_INDEX] > ThresholdSplashes) {
            Value.second = ErrorTextAssignment::LEFT_TRIGGER_SIDE;
            Value.first = tr("Too many splashes(%1) left valve on right side. Threshold %2").arg(results.m_MaxAmountSplashRightROIPerSide[LEFT_TRIGGER_SIDE_INDEX]).arg(ThresholdSplashes);
            Value.first.append("\n");
            listErrorText.append(Value);
        }
        if (results.m_MaxAmountSplashRightROIPerSide[RIGHT_TRIGGER_SIDE_INDEX] > ThresholdSplashes) {
            Value.second = ErrorTextAssignment::RIGHT_TRIGGER_SIDE;
            Value.first = tr("Too many splashes(%1) right valve on right side. Threshold %2").arg(results.m_MaxAmountSplashRightROIPerSide[RIGHT_TRIGGER_SIDE_INDEX]).arg(ThresholdSplashes);
            Value.first.append("\n");
            listErrorText.append(Value);
        }
        ErrorType = ERROR_TYPE_SPLASHES_TOO_MUCH;
    }
    if (!results.m_BottleIsUnderValve) {
        QPair<QString, int> Value;
        Value.first = tr("Bottle not under valve");
        Value.first.append("\n");
        Value.second = ErrorTextAssignment::BOTH_TRIGGER_SIDE;
        listErrorText.append(Value);
        ErrorType = ERROR_TYPE_BOTTLE_NOT_UNDER_VALVE;
    }
    if (!results.m_AmountLiquidOk) {
        QString LiquidLeftTooLow = tr("Amount liquid left valve too low");
        QString LiquidRightTooLow = tr("Amount liquid right valve too low");
        QString LiquidLeftTooMuch = tr("Amount liquid left valve too much");
        QString LiquidRightTooMuch = tr("Amount liquid right valve too much");
        int sumLeft, sumRight;
        QString ErrorText;
        QPair<QString, int> Value;
        if (GetMainAppCrystalT2()->IsFirstTriggerOnLeftSide()) {
            sumLeft = results.m_SumAmountLiquidFirstTrigger;
            sumRight = results.m_SumAmountLiquidSecondTrigger;
            if (!results.m_SumAmountLiquidFirstTriggerOk) {
                if (!results.m_SumAmountLiquidFirstTriggerMinThresholdOk) {
                    ErrorText = tr("%1 %2. Threshold:%3").arg(LiquidLeftTooLow).arg(sumLeft).arg(MinAcceptanceThresholdLiquidMiddleROI);
                } else {
                    ErrorText = tr("%1 %2. Threshold:%3").arg(LiquidLeftTooMuch).arg(sumLeft).arg(MaxAcceptanceThresholdLiquidMiddleROI);
                }
                Value.first = ErrorText;
                Value.second = ErrorTextAssignment::LEFT_TRIGGER_SIDE;
                listErrorText.append(Value);
            }
            if (!results.m_SumAmountLiquidSecondTriggerOk) {
                if (!results.m_SumAmountLiquidSecondTriggerMinThresholdOk) {
                    ErrorText = tr("%1 %2. Threshold:%3").arg(LiquidRightTooLow).arg(sumRight).arg(MinAcceptanceThresholdLiquidMiddleROI);
                } else {
                    ErrorText = tr("%1 %2. Threshold:%3").arg(LiquidRightTooMuch).arg(sumRight).arg(MaxAcceptanceThresholdLiquidMiddleROI);
                }
                Value.first = ErrorText;
                Value.second = ErrorTextAssignment::RIGHT_TRIGGER_SIDE;
                listErrorText.append(Value);
            }
        } else {
            sumRight = results.m_SumAmountLiquidFirstTrigger;
            sumLeft = results.m_SumAmountLiquidSecondTrigger;
            if (!results.m_SumAmountLiquidFirstTriggerOk) {
                if (!results.m_SumAmountLiquidFirstTriggerMinThresholdOk) {
                    ErrorText = tr("%1 %2. Threshold:%3").arg(LiquidRightTooLow).arg(sumRight).arg(MinAcceptanceThresholdLiquidMiddleROI);
                } else {
                    ErrorText = tr("%1 %2. Threshold:%3").arg(LiquidRightTooMuch).arg(sumRight).arg(MaxAcceptanceThresholdLiquidMiddleROI);
                }
                Value.first = ErrorText;
                Value.second = ErrorTextAssignment::RIGHT_TRIGGER_SIDE;
                listErrorText.append(Value);
            }

            if (!results.m_SumAmountLiquidSecondTriggerOk) {
                if (!results.m_SumAmountLiquidSecondTriggerMinThresholdOk) {
                    ErrorText = tr("%1:%2. Threshold:%3").arg(LiquidLeftTooLow).arg(sumLeft).arg(MinAcceptanceThresholdLiquidMiddleROI);
                } else {
                    ErrorText = tr("%1:%2. Threshold:%3").arg(LiquidLeftTooMuch).arg(sumLeft).arg(MaxAcceptanceThresholdLiquidMiddleROI);
                }
                Value.first = ErrorText;
                Value.second = ErrorTextAssignment::LEFT_TRIGGER_SIDE;
                listErrorText.append(Value);
            }
        }
        ErrorType = ERROR_TYPE_FILLING_QUANTITY_NOT_OK;
    }
    return listErrorText;
}
// not in use
void EjectedBottlesDialog::SlotStartVideo()
{
    if (m_FutureRunVideoFillingProcess.isRunning()) {
        m_BreakVideo = true;
        m_FutureRunVideoFillingProcess.cancel();
    }
    m_FutureRunVideoFillingProcess = QtConcurrent::run(this, &EjectedBottlesDialog::ReadVideo, m_VideoFilename);
    emit SignalStartVideoReady();
}
// not in use
int EjectedBottlesDialog::ReadVideo(const QString& VideoFile)
{
    int rv = 0;
    cv::VideoCapture cap(VideoFile.toLatin1().data());
    QImage Image;

    m_BreakVideo = false;
    if (!cap.isOpened()) {
        return -1;
    }
    while (true) {
        if (m_BreakVideo) break;
        cv::Mat frame, GrayIamge;
        cap >> frame;
        if (frame.empty()) {
            rv = -1;
            break;
        }
        cvtColor(frame, GrayIamge, cv::COLOR_BGR2GRAY);
        Image = QImage(GrayIamge.data, frame.cols, frame.rows, QImage::Format_Grayscale8);

        emit SignalSetVideoImage(Image);
        cv::waitKey(100);
    }
    cap.release();
    return rv;
}