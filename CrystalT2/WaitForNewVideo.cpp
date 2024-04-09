#include "WaitForNewVideo.h"
#include "GlobalConst.h"
#include "KitharaCore.h"
#include "MainAppCrystalT2.h"
#include "ProductData.h"
#include "SettingsData.h"
#include "VideoDialog.h"
#include "VideoHeader.h"

WaitForNewVideo::WaitForNewVideo(MainAppCrystalT2* pMainAppCrystalT2)
    : QThread((QObject*)(pMainAppCrystalT2)),
      m_ExitThread(false),
      m_SharedMemoryVideoData(NULL),
      m_PollingTimeReadNewVideoData(200),
      m_TimeOutValueWaitForNewVideo(1000),
      m_StartVideoIndex(0),
      m_ImageBlockSize(0),
      m_BlockSizeTop(0),
      m_BlockSizeBottom(0),
      m_MaxBlockSize(0),
      m_CurrentNumberFrames(0),
      m_FramesPerVideo(0),
      m_ProductPressentTime(0.0),
      m_CurrentSpeedInmmPerms(0.0),
      m_ImageWidth(0),
      m_ImageHeight(0)

{
    m_MainAppCrystalT2 = pMainAppCrystalT2;
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) m_PollingTimeReadNewVideoData = GetMainAppCrystalT2()->GetSettingsData()->m_PollingTimeReadNewVideoData;
}

WaitForNewVideo::~WaitForNewVideo()
{
    FinishedThread();
    if (m_SharedMemoryVideoData) {
        delete m_SharedMemoryVideoData;
        m_SharedMemoryVideoData = NULL;
    }
}

void WaitForNewVideo::FinishedThread()
{
    int count = 0;

    m_ExitThread = true;
    if (GetMainAppCrystalT2()->GetKitharaCore()) GetMainAppCrystalT2()->GetKitharaCore()->ForceSetEventCanCopyVideoData();
    while (isRunning() && count < 5) {
        wait(m_TimeOutValueWaitForNewVideo * 2);
        count++;
    }
}

// Anlegen des Videospeichers auf der Windowsseite
int WaitForNewVideo::AllocateSharedMemoryVideoData(int ImageWidth, int ImageHeight, QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    unsigned __int64 MB = 1024 * 1024;
    VideoHeader videoHeader;
    int SizeImageBlock = (ImageWidth * ImageHeight);

    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData())
        videoHeader.m_MAXVideoBockSize = static_cast<unsigned __int64>(GetMainAppCrystalT2()->GetSettingsData()->m_SizeVideoMemoryInMB) * MB;
    else
        videoHeader.m_MAXVideoBockSize = 512 * MB;

    videoHeader.m_MaxNumberFrames = static_cast<unsigned __int64>(videoHeader.m_MAXVideoBockSize / ((double)(SizeImageBlock)));
    videoHeader.m_ImageWidth = ImageWidth;
    videoHeader.m_ImageHeight = ImageHeight;
    if (m_SharedMemoryVideoData == NULL) m_SharedMemoryVideoData = new SharedMemoryVideoData();
    m_SharedMemoryVideoData->SetKeyName(KEY_NAME_SHARED_MEMORY_VIDEO_DATA);
    if (!m_SharedMemoryVideoData->isAttached()) {
        rv = m_SharedMemoryVideoData->CreateNew(videoHeader.m_MAXVideoBockSize + sizeof(VideoHeader), ErrorMsg);         // shared memory einmalig anlegen
        if (rv == 0) memcpy(m_SharedMemoryVideoData->GetSharedMemoryStartPointer(), &videoHeader, sizeof(VideoHeader));  // copy videoHeader initial value
    }
    return rv;
}

void WaitForNewVideo::StartWaitForNextVideo()
{
    if (!isRunning()) start();
}

unsigned char* WaitForNewVideo::GetSharedMemoryPointer()
{
    if (m_SharedMemoryVideoData)
        return m_SharedMemoryVideoData->GetSharedMemoryStartPointer();
    else
        return NULL;
}
/*
          Shared Memory Block

     Index _________________  _<--Start address                         _
        10|                 | _ m_ImageBlockSize                         |
        11|                 |                                            |
        12|                 |                                            > m_BlockSizeTop
        13|                 |                                            |
        14|                 |                                           _|
        15|_________________| <-- End Video(Last Image)                 _
        0 |                 | <-- Begin Video = m_StartVideoIndex        |
        1 |                 |                                            |
        2 |                 |                                            |
        3 |                 |                                            |
        4 |                 |                                            > m_BlockSizeBottom
        5 |                 |                                            |
        6 |                 |                                            |
        7 |                 |                                            |
        8 |                 |                                           _|
        9 |_________________|<-- End Address
*/
// aufruf wenn kontinuierlich Bilder angezeigt werden sollen
bool WaitForNewVideo::SpeedCopyVideoFromRealTimeSharedMemoryToApplicationMemory()
{
    unsigned char* pSharedMemoryVideoRealTimeContext;
    unsigned char* pSharedMemoryApplicationContext;
    VideoHeader videoHeader;
    bool rv = true;

    if (m_SharedMemoryVideoData && GetMainAppCrystalT2()->GetKitharaCore()) {
        if (m_SharedMemoryVideoData->GetCurrentSharedMemorySize() > 0 && GetMainAppCrystalT2()->GetKitharaCore()->GetRawVideodata() != NULL) {
            // m_SharedMemoryVideoData->lock();
            if (m_SharedMemoryVideoData->GetSharedMemoryStartPointer() != NULL) {
                memcpy(m_SharedMemoryVideoData->GetSharedMemoryStartPointer(), GetMainAppCrystalT2()->GetKitharaCore()->GetRawVideodata(), sizeof(VideoHeader));  // copiere VideoHeader
                memcpy(&videoHeader, m_SharedMemoryVideoData->GetSharedMemoryStartPointer(), sizeof(VideoHeader));                                                // read video header
                m_ProductPressentTime = videoHeader.m_ProductPressentTime;
                m_ImageWidth = videoHeader.m_ImageWidth;
                m_ImageHeight = videoHeader.m_ImageHeight;
                m_ImageBlockSize = (m_ImageWidth * m_ImageHeight) + sizeof(ImageHeader);
                m_CurrentNumberFrames = videoHeader.m_CurrentNumberFrames;  // Aktuelle Anzahl von Bildern im Sharedmemory
                m_StartVideoIndex = videoHeader.m_FrameIndex;               // Ist der Index des zuletzt aufgenommen Bildes
                m_MaxNumberFrames = videoHeader.m_MaxNumberFrames;          // ist immer Konstant, wird beim Anlegen des Shared Memory festgelegt, bzw. ist in der Konfiguration definiert
                m_MaxBlockSize = m_CurrentNumberFrames * m_ImageBlockSize;
                m_BlockSizeBottom = m_MaxBlockSize - (m_ImageBlockSize * m_StartVideoIndex);
                m_BlockSizeTop = m_MaxBlockSize - m_BlockSizeBottom;
                m_FramesPerVideo = ReadFramesPerVideo(videoHeader);
                m_CurrentSpeedInmmPerms = videoHeader.m_CurrentSpeedInmmPerms;
                // nur bilder kopieren die noch nicht auf der Windowsseite vorhanden sind
                for (int i = 0; i < m_CurrentNumberFrames; i++) {
                    if (videoHeader.m_ImageIndexIsCopied[i] == 0) {
                        pSharedMemoryApplicationContext = m_SharedMemoryVideoData->GetSharedMemoryStartPointer() + sizeof(VideoHeader) + (i * m_ImageBlockSize);
                        pSharedMemoryVideoRealTimeContext = GetMainAppCrystalT2()->GetKitharaCore()->GetRawVideodata() + sizeof(VideoHeader) + (i * m_ImageBlockSize);
                        memcpy(pSharedMemoryApplicationContext, pSharedMemoryVideoRealTimeContext, m_ImageBlockSize);
                        videoHeader.m_ImageIndexIsCopied[i] = 1;  // als kopiert kennzeichnen
                    }
                }
                memcpy(GetMainAppCrystalT2()->GetKitharaCore()->GetRawVideodata(), &videoHeader, sizeof(VideoHeader));  // write video header
            }
        } else
            rv = false;
    }
    GetMainAppCrystalT2()->GetKitharaCore()->SetReadVideoFromRealTimeContextOk(false);  // freigabe im Realtimeteil das wieder Videosbilder gespeichert werden können
    return rv;
}

int WaitForNewVideo::ReadFramesPerVideo(VideoHeader& Header)
{
    double TimeInms = (Header.m_CurrentTimeStampInns - Header.m_LastTimeStampInns) / ((double)(1000000.0));
    double FPS = 1000.0 / TimeInms;
    double ProductPresentTimeInSec = (Header.m_ProductPressentTime / 1000.0);
    return static_cast<int>(FPS * ProductPresentTimeInSec);
}

/*
          Shared Memory Block

     Index _________________  _<--Start address                                       _
        10|                 | _ m_ImageBlockSize                                       |
        11|                 |                                                          |
        12|                 |                                                          > m_BlockSizeTop
        13|                 |                                                          |
        14|                 |                                                         _|
        15|_________________| <-- End Video(Last Image/Neustes Bild)                  _
        0 |                 | <-- Begin Video = m_StartVideoIndex(älteste bild)        |
        1 |                 |                                                          |
        2 |                 |                                                          |
        3 |                 |                                                          |
        4 |                 |                                                          > m_BlockSizeBottom
        5 |                 |                                                          |
        6 |                 |                                                          |
        7 |                 |                                                          |
        8 |                 |                                                         _|
        9 |_________________|<-- End Address
*/
unsigned char* WaitForNewVideo::GetImageStartPointer(int ImageIndex)
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

int WaitForNewVideo::GetCurrentVideoTimeOffset(int usedTrigger)
{
    int TimeOffsetInms = 0;
    double VideoPositionInmm = 0.0;
    double CurrentSpeedInMMPerms = 0.0;
    if (GetMainAppCrystalT2()) {
        CurrentSpeedInMMPerms = GetCurrentSpeedInmmPerms();  // Kommt aus dem VideoHeader von der Realtime App
        ProductData* pProductData = GetMainAppCrystalT2()->GetCurrentProductData();

        if (pProductData && GetMainAppCrystalT2()->GetSettingsData()) {
            if (GetMainAppCrystalT2()->GetSettingsData()->m_BandDirectional == BAND_DIRECTIONAL_RIGHT_TO_LEFT) {
                if (usedTrigger == LEFT_VIDEO_POS) {
                    VideoPositionInmm = pProductData->m_VideoPositionLeftValveInmm * (-1.0);
                } else {
                    VideoPositionInmm = pProductData->m_VideoPositionRightValveInmm * (-1.0);
                }
            } else {
                if (usedTrigger == LEFT_VIDEO_POS) {
                    VideoPositionInmm = pProductData->m_VideoPositionLeftValveInmm;
                } else {
                    VideoPositionInmm = pProductData->m_VideoPositionRightValveInmm;
                }
            }
        }
        if (CurrentSpeedInMMPerms > 0.0) TimeOffsetInms = static_cast<int>(VideoPositionInmm / CurrentSpeedInMMPerms);
    }
    return TimeOffsetInms;
}

unsigned char* WaitForNewVideo::GetImageStartPointerByTimeOffset(int StartImageIndexTrigger, long long TimeOffsetInms)
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

void WaitForNewVideo::ShowAllTimeStamps()
{
    unsigned char* CurrentAddress = NULL;
    ImageHeader imageHeader;
    unsigned long long CurrentTimeStampInns;
    QString Info;

    for (int StartIndex = 0; StartIndex < GetCurrentNumberFrames(); StartIndex++) {
        CurrentAddress = GetImageStartPointer(StartIndex) - sizeof(ImageHeader);  // pSharedMemoryImageData + sizeof(VideoHeader) + (ImageBlockSize * i);
        if (CurrentAddress) {
            memcpy(&imageHeader, CurrentAddress, sizeof(ImageHeader));
            CurrentTimeStampInns = imageHeader.m_TimeStampInns;
            if (imageHeader.m_ImageState & 0x04)  // trigger pos
                Info = QString("Index:%1  TimeStamp:%2ms  TriggerPos").arg(StartIndex).arg(CurrentTimeStampInns / 1000000.0, 0, 'f', 5);
            else {
                if (imageHeader.m_ImageState & 0x02)  // first occur
                    Info = QString("Index:%1  TimeStamp:%2ms  First Occur").arg(StartIndex).arg(CurrentTimeStampInns / 1000000.0, 0, 'f', 5);
                else
                    Info = QString("Index:%1  TimeStamp:%2ms  Normal Image").arg(StartIndex).arg(CurrentTimeStampInns / 1000000.0, 0, 'f', 5);
            }
            emit SignalShowInfo(Info, 0);
            msleep(20);
        }
    }
}

void WaitForNewVideo::run()
{
    QString ErrorMsg;
    int rv = ERROR_CODE_NO_ERROR;
    int DialogID = 0;

    while (true) {
        QApplication::processEvents();
        if (m_ExitThread) break;
        rv = ERROR_CODE_NO_ERROR;
        if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetKitharaCore()) {
            DialogID = GetVisibileDialogID();
            switch (DialogID) {
                case VIDEO_DIALOG_SHOW_FULL_VIDEO:
                    rv = GetMainAppCrystalT2()->GetKitharaCore()->WaitForCanCopyNewVideo(m_TimeOutValueWaitForNewVideo, ErrorMsg);  // Event wird von der Realtimetask generiert
                    if (rv == ERROR_CODE_NO_ERROR) {
                        SpeedCopyVideoFromRealTimeSharedMemoryToApplicationMemory();
                        emit SignalOneVideoReady();  // Anzeige komplettes Video
                    }
                    break;
                case VIDEO_DIALOG_SHOW_TRIGGER_POSITION:
                    SpeedCopyVideoFromRealTimeSharedMemoryToApplicationMemory();  // Aktualisieren des Vidospeicher auf der Windowsseite mit dem auf der Realtimeseite
                    emit SignalShowTriggerImage();
                    break;
                case VIDEO_DIALOG_SHOW_PRODUCT_VIDEO:
                    SpeedCopyVideoFromRealTimeSharedMemoryToApplicationMemory();  // Aktualisieren des Vidospeicher auf der Windowsseite mit dem auf der Realtimeseite
                    emit SignalShowProductVideos();
                    break;
                default:
                    SpeedCopyVideoFromRealTimeSharedMemoryToApplicationMemory();
                    msleep(m_PollingTimeReadNewVideoData);  // Kein Dialog sichtbar
                    break;
            }
            if (m_ExitThread) break;
            if (DialogID != 0 && rv == ERROR_CODE_NO_ERROR) {
                msleep(m_PollingTimeReadNewVideoData);
            }
        } else
            m_ExitThread = true;
    }
}

int WaitForNewVideo::GetVisibileDialogID()
{
    int rv = 0;
    if (GetMainAppCrystalT2()) {
        if (GetMainAppCrystalT2()->GetVideoDialogFullVideo() && GetMainAppCrystalT2()->GetVideoDialogFullVideo()->isVisible()) {  // warte auf freigabe einmaliges Kopieren des Videospeichers
            rv = VIDEO_DIALOG_SHOW_FULL_VIDEO;
        } else {  // kontinuierliches lesen aus dem Videospeicher und Liveanzeige für eine besimmte Position, hier die Triggerposition
            if (GetMainAppCrystalT2()->GetVideoDialogShowTriggerPos() && GetMainAppCrystalT2()->GetVideoDialogShowTriggerPos()->isVisible()) {
                rv = VIDEO_DIALOG_SHOW_TRIGGER_POSITION;
            } else {
                if (GetMainAppCrystalT2()->GetVideoDialogShowProductVideos() && GetMainAppCrystalT2()->GetVideoDialogShowProductVideos()->isVisible()) {
                    rv = VIDEO_DIALOG_SHOW_PRODUCT_VIDEO;
                }
            }
        }
    }
    return rv;
}
