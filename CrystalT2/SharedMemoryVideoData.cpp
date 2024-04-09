#include "SharedMemoryVideoData.h"

SharedMemoryVideoData::SharedMemoryVideoData()
{
    m_EventIDNewDataInSharedMemory = NULL;
    m_KeyName = "";
    m_CurrentSharedMemorySize = 0;
}

SharedMemoryVideoData::~SharedMemoryVideoData()
{
    bool rv;
    if (isAttached()) rv = detach();
    if (m_EventIDNewDataInSharedMemory) CloseHandle(m_EventIDNewDataInSharedMemory);
}

int SharedMemoryVideoData::OpenSharedMemory(QString& ErrorMsg)
{
    int rv = 0;
    if (!isAttached()) {
        if (!attach()) {
            rv = -1;
            ErrorMsg = tr("Server Program Is Not Active. Invalid Handle. Can Not Open Shared Memory Area!");
        }
    }
    return rv;
}

void SharedMemoryVideoData::CloseSharedMemory()
{
    bool rv;
    if (isAttached()) rv = detach();
}

void SharedMemoryVideoData::SetKeyName(const QString& Name)
{
    m_KeyName = Name;
    setKey(m_KeyName);
    CreateNewEventHandle(m_KeyName);
}

void SharedMemoryVideoData::CreateNewEventHandle(QString& Name)
{
    QString EventName = "EVENT_";
    wchar_t* pWCharEventName = NULL;

    EventName = EventName + Name;
    pWCharEventName = new wchar_t[EventName.size() + 1];
    EventName.toWCharArray(pWCharEventName);
    pWCharEventName[EventName.size()] = 0;

    if (m_EventIDNewDataInSharedMemory) CloseHandle(m_EventIDNewDataInSharedMemory);
    m_EventIDNewDataInSharedMemory = CreateEventW(NULL, FALSE, FALSE, pWCharEventName);
    delete[] pWCharEventName;
}

unsigned char* SharedMemoryVideoData::GetSharedMemoryStartPointer()
{
    return (unsigned char*)data();
}

void SharedMemoryVideoData::SetEventNewData()
{
    SetEvent(m_EventIDNewDataInSharedMemory);
}

int SharedMemoryVideoData::CreateNew(unsigned __int64 SaredSize, QString& ErrorMsg)
{
    m_CurrentSharedMemorySize = SaredSize;
    if (!create(m_CurrentSharedMemorySize)) {
        QString ErrorText;
        switch (error()) {
            case QSharedMemory::PermissionDenied:  // The operation failed because the caller didn't have the required permissions.
                ErrorText = tr("PermissionDenied");
                break;
            case QSharedMemory::InvalidSize:  // A create operation failed because the requested size was invalid.
                ErrorText = tr("InvalidSize");
                break;
            case QSharedMemory::KeyError:  // The operation failed because of an invalid key.
                ErrorText = tr("KeyError");
                break;
            case QSharedMemory::AlreadyExists:  // A create() operation failed because a shared memory segment with the specified key already existed.
                ErrorText = tr("AlreadyExists");
                break;
            case QSharedMemory::NotFound:  // An attach() failed because a shared memory segment with the specified key could not be found.
                ErrorText = tr("NotFound");
                break;
            case QSharedMemory::LockError:  // The attempt to lock() the shared memory segment failed because create() or attach() failed and returned false, or because a system error occurred in
                                            // QSystemSemaphore::acquire().
                ErrorText = tr("LockError");
                break;
            case QSharedMemory::OutOfResources:  // A create() operation failed because there was not enough memory available to fill the request.
                ErrorText = tr("OutOfResources");
                break;
            case QSharedMemory::UnknownError:  // Something else happened and it was bad.
                ErrorText = tr("UnknownError");
                break;
            default:
                ErrorText = tr("System Unknown Error");
                break;
        }
        ErrorMsg = tr("Error Create Video Memory. (%1) Size:%2 Bytes").arg(ErrorText).arg(m_CurrentSharedMemorySize);
        m_CurrentSharedMemorySize = 0;
        return -1;
    } else {
        return 0;
    }
}
