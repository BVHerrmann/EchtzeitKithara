#pragma once

#include <QtCore>
#include "QtSerialPort/qserialport.h"
#include "SettingsData.h"

class ValveDialog;
class SerialInterface : public QThread
{
	Q_OBJECT
public:
	SerialInterface(ValveDialog *pValveDialog);
	~SerialInterface(); 
	
	ValveDialog *GetValveDialog() {return m_ValveDialog;}
	void FinishedSerialPort();
	bool CheckParameterChanged();
	virtual void run();
	void SetSerialCommand(QString &Command);
	int OpenControllerAndSetParameter(QSerialPort *pSerialPort, QString &ErrorMsg);
	bool  IsLastStateOk() { return m_LastStateOk; }
	void SetComPortParameter(int BaudRate, int Parity, int DataBits, int StopBits, QString &ComPortName);
	int  OpenPortAndStartDataExchangeValve(QString &ErrorMsg);
	bool IsSerialPortOpen() { return m_IsSerialPortOpen; }
	void ParsingResponse(QSerialPort &SerialPort, QString &CurrentSerialCommand);
	void ClearSerialCommands();
    void SaveLastErrorTextInfo(QString& Respone);
	
signals:
	void SignalShowInfo(const QString &InfoData, QtMsgType MsgType);
	void SignalCommadSuccessfullySet(const QString &Command,bool Successfully);
	void SignalShowValveTemperature(const QString &StatusHeaters,int ValveID);
	void SignalReadLastErrorCodes();
	void SignalRawResponseString(const QString &Response);


public slots:
	void SlotSetCommandReadLastErrorCodes();

private:
	ValveDialog   *m_ValveDialog;
	QString        m_CurrentComPort;
	QString        m_CurrentSerialCommand;
    QMutex         m_Mutex;
	QWaitCondition m_WaitCondition;
	QQueue<QString> m_QueueCurrentSerialCommands;
	bool           m_TerminateThread;
	bool           m_LastStateOk;
	bool           m_TimeOutReadyRead;
	bool           m_TimeOutBytesWritten;
	bool           m_IsSerialPortOpen;
	int            m_BaudRate;
    int            m_Parity;
    int            m_DataBits;
    int            m_StopBits;
	int            m_PollingTimeReadStatusValveInms;
	int            m_TimeOutValueSeriellerPort;
};
