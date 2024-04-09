#include "SerialInterface.h"
#include <QtSerialPort/QSerialPort>
#include "GlobalConst.h"
#include "QtSerialPort/qserialportinfo.h"
#include "ValveDialog.h"

SerialInterface::SerialInterface(ValveDialog* pValveDialog)
    : QThread((QObject*)pValveDialog),
      m_BaudRate(QSerialPort::Baud115200),
      m_Parity(QSerialPort::NoParity),
      m_DataBits(QSerialPort::Data8),
      m_StopBits(QSerialPort::OneStop),
      m_TerminateThread(false),
      m_CurrentSerialCommand(COMMAND_GET_STATUS_ALARM),
      m_LastStateOk(false),
      m_PollingTimeReadStatusValveInms(500),
      m_IsSerialPortOpen(false),
      m_TimeOutReadyRead(true),
      m_TimeOutBytesWritten(true),
      m_TimeOutValueSeriellerPort(2000),
      m_ValveDialog(NULL)
{
    m_ValveDialog = pValveDialog;

    connect(this, &SerialInterface::SignalReadLastErrorCodes, this, &SerialInterface::SlotSetCommandReadLastErrorCodes);
}

SerialInterface::~SerialInterface()
{
    FinishedSerialPort();
}

void SerialInterface::FinishedSerialPort()
{
    if (isRunning()) {
        m_TerminateThread = true;
        m_WaitCondition.wakeOne();
        wait();
    }
}

void SerialInterface::SetSerialCommand(QString& Command)
{
    if (isRunning()) {
        if (IsLastStateOk() || Command == COMMAND_RESET_ERROR || Command == COMMAND_GET_LAST_ERRORS) {
            if (m_Mutex.tryLock(1000)) {
                m_QueueCurrentSerialCommands.enqueue(Command);
                m_WaitCondition.wakeOne();
                m_Mutex.unlock();
            }
        }
    }
}

void SerialInterface::SlotSetCommandReadLastErrorCodes()
{
    QString Command = COMMAND_GET_LAST_ERRORS;
    SetSerialCommand(Command);
}

void SerialInterface::SetComPortParameter(int BaudRate, int Parity, int DataBits, int StopBits, QString& CurrentComPort)
{
    m_BaudRate = BaudRate;
    m_Parity = Parity;
    m_DataBits = DataBits;
    m_StopBits = StopBits;
    m_CurrentComPort = CurrentComPort;
}

int SerialInterface::OpenPortAndStartDataExchangeValve(QString& ErrorMsg)
{
    int count = 0;
    int rv = ERROR_CODE_NO_ERROR;
    if (!isRunning()) {
        m_TerminateThread = false;
        start(QThread::HighestPriority);  //Öffne Seriellen Port und Starte Kommunikation mit dem Nordson Ventil
        msleep(100);
        while (m_TimeOutBytesWritten || m_TimeOutReadyRead)  // Warte solange bis Serieller Port erfolgreich geöffnet ist
        {
            msleep(m_TimeOutValueSeriellerPort);
            count++;
            if (count > 1) {
                ErrorMsg = tr("Error! Valve Serial Connection Error. Port:%1 %2").arg(m_CurrentComPort).arg(GetValveDialog()->GetValveName());
                rv = ERROR_CODE_ANY_ERROR;
                break;
            }
        }
    }
    return rv;
}

bool SerialInterface::CheckParameterChanged()
{
    bool rv = false;
    SettingsData* pSettingsData = GetValveDialog()->GetSettingsData();
    if (pSettingsData) {
        if (m_PollingTimeReadStatusValveInms != pSettingsData->m_PollingTimeReadStatusValveInms || m_TimeOutValueSeriellerPort != pSettingsData->m_TimeOutValueSeriellerPort) {
            m_PollingTimeReadStatusValveInms = pSettingsData->m_PollingTimeReadStatusValveInms;
            m_TimeOutValueSeriellerPort = pSettingsData->m_TimeOutValueSeriellerPort;
        }
        if (m_BaudRate != pSettingsData->m_BaudRate || m_DataBits != pSettingsData->m_DataBits || m_StopBits != pSettingsData->m_StopBits || m_Parity != pSettingsData->m_Parity) {
            m_BaudRate = pSettingsData->m_BaudRate;
            m_Parity = pSettingsData->m_Parity;
            m_DataBits = pSettingsData->m_DataBits;
            m_StopBits = pSettingsData->m_StopBits;
            rv = true;
        }
    }
    return rv;
}

int SerialInterface::OpenControllerAndSetParameter(QSerialPort* pSerialPort, QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    pSerialPort->setPortName(m_CurrentComPort);
    if (pSerialPort->open(QIODevice::ReadWrite)) {
        pSerialPort->setBaudRate(static_cast<QSerialPort::BaudRate>(m_BaudRate));
        pSerialPort->setDataBits(static_cast<QSerialPort::DataBits>(m_DataBits));
        pSerialPort->setStopBits(static_cast<QSerialPort::StopBits>(m_StopBits));
        pSerialPort->setParity(static_cast<QSerialPort::Parity>(m_Parity));
    } else {
        ErrorMsg = (tr("Error! Valve Controller: Can't Open Serial Device %1. Error Code %2.").arg(GetValveDialog()->GetValveName()).arg(pSerialPort->error()));
        rv = ERROR_CODE_ANY_ERROR;
    }
    return rv;
}

// const QString RESPONSE_END_TAG = "<3";
// const QString RESPONSE_ALARM_TAG = "Alarm";
// const QString END_TAG_UNKNOWN_COMMAND = "<?";
// const QString RESPONSE_INVALID_VALUE = "Invalid";

// const QString COMMAND_GET_STATUS_ALARM = "stat";
// const QString COMMAND_RESET_ERROR = "arst";
// const QString COMMAND_GET_STATUS_HEATERS = "rhtr";
// const QString COMMAND_GET_STATUS_VALVE = "rdr1";
// const QString COMMAND_GET_LAST_ERRORS = "ralr";
void SerialInterface::run()
{  // Wird ein Parameter für die Serielle Kommunikation von der GUI aus geändert wird der Wert hier auf true gesetzt, der Port wird geschlossen der Parameter gesetzt und dann neu geöffnet
    bool ParmChanged = false;
    QString CurrentSerialCommand, ErrorMsg;
    int rv;
    QByteArray responseData;
    QString ResponseString;
    QSerialPort SerialPort;
    qint64 WriteBytes;
    bool Repeat = false;
    int TimeoutCount = 0;

    CurrentSerialCommand = COMMAND_GET_STATUS_ALARM;  //
    while (!m_TerminateThread) {
        if (ParmChanged || !SerialPort.isOpen())  // Beim allerersten Aufruf ist der Serielle Port noch geschlossen
        {
            SerialPort.close();
            rv = OpenControllerAndSetParameter(&SerialPort, ErrorMsg);                             // Port öffnen und die Kommunikationparameter setzen
            if (rv == ERROR_CODE_ANY_ERROR) emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
        }
        if (SerialPort.isOpen()) {
            m_IsSerialPortOpen = true;
            TimeoutCount = 0;
            do {
                if (m_TerminateThread) break;
                QByteArray SerialCommand = CurrentSerialCommand.toUtf8();
                SerialCommand.append(0x0D);                    // append enter
                WriteBytes = SerialPort.write(SerialCommand);  // write command
                if (WriteBytes != -1) {
                    SerialPort.flush();
                    if (SerialPort.waitForBytesWritten(m_TimeOutValueSeriellerPort))  // wait data is written
                    {
                        m_TimeOutBytesWritten = false;
                        if (SerialPort.waitForReadyRead(m_TimeOutValueSeriellerPort))  // wait until response
                        {
                            m_TimeOutReadyRead = false;
                            Repeat = false;
                            ParsingResponse(SerialPort, CurrentSerialCommand);
                        } else {
                            if (CurrentSerialCommand != COMMAND_GET_LAST_ERRORS) {
                                TimeoutCount++;
                                if (TimeoutCount > 1) {
                                    ErrorMsg = tr("Error! %1 Timeout, Command:%2").arg(GetValveDialog()->GetValveName()).arg(CurrentSerialCommand);
                                    emit SignalShowInfo(ErrorMsg, QtMsgType::QtWarningMsg);
                                    SerialPort.readAll();
                                    SerialPort.close();  // Schließe Port weil wahrscheinlich Verbindung unterbrochen, Versuch, dann die Verbindung wieder aufzubauen
                                    m_TimeOutReadyRead = true;
                                    Repeat = false;
                                    ClearSerialCommands();

                                } else {
                                    SerialPort.readAll();
                                    ErrorMsg = tr("Info! Repeat Serial Command");
                                    emit SignalShowInfo(ErrorMsg, QtMsgType::QtInfoMsg);
                                    Repeat = true;  // versuch das Kommando nochmal zu senden
                                    msleep(100);
                                }
                            } else {  // Wenn keine Fehlermeldungen vorhanden dann liefert das Ventil auch keine Daten bzw. Antwort
                                if (GetValveDialog()->IsPicCommandDialogVisible()) emit SignalRawResponseString(tr("No Errors In."));
                            }
                        }
                    } else {
                        ErrorMsg = tr("Error! %1 Wait Write Request Serial Port, Timeout:%2ms").arg(GetValveDialog()->GetValveName()).arg(m_TimeOutValueSeriellerPort);
                        emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
                        SerialPort.close();
                        m_TimeOutBytesWritten = true;
                        ClearSerialCommands();
                    }
                } else {  // Write failed
                    ErrorMsg = tr("Error! %1 Write Data Serial Port Failed, Command:%2").arg(GetValveDialog()->GetValveName()).arg(CurrentSerialCommand);
                    emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
                }
            } while (Repeat);
        }  // if (SerialPort.isOpen())
        else
            m_IsSerialPortOpen = false;

        m_Mutex.lock();
        m_WaitCondition.wait(&m_Mutex, m_PollingTimeReadStatusValveInms);  //
        if (!m_QueueCurrentSerialCommands.isEmpty()) {
            CurrentSerialCommand = m_QueueCurrentSerialCommands.dequeue();  // ein neuese Kommado wurde von aussen gesetzt
            msleep(50);
        } else {  // Kein Kommando in der Pipe, dann Fehlerstatus alle 1000ms abfragen
            CurrentSerialCommand = COMMAND_GET_STATUS_ALARM;
        }
        ParmChanged = CheckParameterChanged();  // wurde ein Parameter über die GUI geändert
        if (m_TerminateThread) {                // der thread soll beendet werden i.d.R. wird dann die Application beendet
            if (SerialPort.isOpen()) SerialPort.close();
            m_Mutex.unlock();
            return;
        }
        m_Mutex.unlock();
    }
    if (SerialPort.isOpen())  // thread wurde beendet, dann Seriellen Port schliessen
        SerialPort.close();
}

void SerialInterface::ClearSerialCommands()
{
    m_Mutex.lock();
    m_QueueCurrentSerialCommands.clear();
    m_Mutex.unlock();
}

void SerialInterface::ParsingResponse(QSerialPort& SerialPort, QString& CurrentSerialCommand)
{
    QString ResponseString;
    bool ResponseComplete = false;

    ResponseString = SerialPort.readAll();  // read response
    ResponseComplete = false;
    for (int i = 0; i < 40 && !ResponseComplete; i++)  // is response complete?
    {
        if (!ResponseString.contains(RESPONSE_END_TAG))  // && !ResponseString.contains(END_TAG_UNKNOWN_COMMAND))
        {
            SerialPort.waitForReadyRead(10);
            ResponseString += SerialPort.readAll();
        } else {
            ResponseComplete = true;
            break;
        }
    }
    if (ResponseComplete) {
        // das Kommando COMMAND_GET_STATUS_HEATERS und COMMAND_GET_STATUS_ALARM werde zyklisch abgefragt
        if (GetValveDialog()->IsPicCommandDialogVisible() && CurrentSerialCommand != COMMAND_GET_STATUS_HEATERS && CurrentSerialCommand != COMMAND_GET_STATUS_ALARM)  // hier nur für testzweke
            emit SignalRawResponseString(ResponseString);
        if (ResponseString.length() > 4) {  // remove first and last "\n"
            ResponseString = ResponseString.remove(0, 2);
            ResponseString = ResponseString.remove(ResponseString.length() - 4, 2);
        }
        if (CurrentSerialCommand == COMMAND_GET_STATUS_ALARM) {
            if (ResponseString.contains(RESPONSE_ALARM_TAG)) {                           // device is in Error state
                ResponseString = ResponseString.remove(ResponseString.length() - 2, 2);  // remove End Tag(<3)
                ResponseString = tr("Alarm %1! Nordson (Hex)Error Code: %2").arg(GetValveDialog()->GetValveName()).arg(ResponseString);
                QString ValveNameWithoutSpace = GetValveDialog()->GetValveName().remove(QChar(' '));
                
                emit SignalShowInfo(ResponseString, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);//show response
                if (m_LastStateOk) {
                    QString LastErrors = COMMAND_GET_LAST_ERRORS;
                    SetSerialCommand(LastErrors);
                }
                  
                // emit SignalReadLastErrorCodes();    
                // Signal an die GUI das Kommando COMMAND_GET_LAST_ERRORS zu setzen um eine genauere Fehlerbeschreibung auszulesen
                m_LastStateOk = false;
            } else {  // device is Ok
                if (!m_LastStateOk) {
                    m_LastStateOk = true;
                    ResponseString = ResponseString.remove(ResponseString.length() - 2, 2);  // remove End Tag(<3)
                    emit SignalShowInfo(ResponseString, QtMsgType::QtInfoMsg);
                }
            }
        } else {
            if (CurrentSerialCommand == COMMAND_RESET_ERROR) {
                ResponseString = tr("Info! Reset Error Command Is Set. %1").arg(GetValveDialog()->GetValveName());
                emit SignalShowInfo(ResponseString, QtMsgType::QtInfoMsg);  // ERROR_CODE_NO_ERROR);
            } else {
                if (CurrentSerialCommand == COMMAND_GET_LAST_ERRORS) {           // Ein Fehler ist aufgetreten, lese Fehlertext aus dem Nordson Ventil
                    //emit SignalShowInfo(ResponseString, QtMsgType::QtFatalMsg);  // Anzeige des Fehlertextes
                    SaveLastErrorTextInfo(ResponseString);
                } else {
                    if (CurrentSerialCommand == COMMAND_GET_STATUS_HEATERS) {
                        emit SignalShowValveTemperature(ResponseString, GetValveDialog()->GetValveID());
                    } else {
                        if (CurrentSerialCommand == COMMAND_GET_STATUS_VALVE) {
                            // emit SignalShowValveTemperature(ResponseString, GetValveDialog()->GetValveID());
                            ;
                        } else {  // set normal parameter
                            if (ResponseString.contains(RESPONSE_INVALID_VALUE) || ResponseString.contains(END_TAG_UNKNOWN_COMMAND)) {
                                emit SignalCommadSuccessfullySet(CurrentSerialCommand, false);  // can not set value, value is invalid
                                ResponseString = tr("Error! %1 Invalid Command %2.").arg(GetValveDialog()->GetValveName()).arg(CurrentSerialCommand);
                                emit SignalShowInfo(ResponseString, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);//info an den message dialog
                            } else {
                                ResponseString = ResponseString.remove(ResponseString.length() - 2, 2);  // remove End Tag(<3)
                                QString temp = GetValveDialog()->GetValveName() + QString(" ") + CurrentSerialCommand + QString(" -> ") + ResponseString;
                                emit SignalCommadSuccessfullySet(CurrentSerialCommand, true);  // info an den parameter dialog das Wert erfolgreich gesetzt
                                emit SignalShowInfo(temp, QtMsgType::QtInfoMsg);               // ERROR_CODE_NO_ERROR);//info an den message dialog
                            }
                        }
                    }
                }
            }
        }
    } else {
        QString res = ResponseString.toUtf8().toHex();
        QString InfoString = tr("Info! %1 Response:%2 Not Completely").arg(GetValveDialog()->GetValveName()).arg(res);
        emit SignalShowInfo(InfoString, QtMsgType::QtInfoMsg);  // ERROR_CODE_ANY_ERROR);//show response
    }
}

void SerialInterface::SaveLastErrorTextInfo(QString& Response)
{
    QStringList lines = Response.split("\r");
    if (lines.count() > 0) {
        QString FirstLine = lines.at(0);

        QString last2 = FirstLine.right(2);
        QString LineNumber = QString("Code # %1 Mem Location").arg(last2);
        for (int i = 0; i < lines.count(); i++)
        {
            if (lines.at(i).contains(LineNumber)) {
                //QString FileName = QString("ValveErrorText%1ValveID.txt").arg(GetValveDialog()->GetValveID());
                QString ErrorLine = lines.at(i);
                ErrorLine.remove("\n");
                ErrorLine.remove("Mem Location #");
                QString ErrorText = tr("Nordson Err:%1").arg(ErrorLine);
                emit SignalShowInfo(ErrorText, QtMsgType::QtFatalMsg);
                break;
            }
        }
       
    }
}
