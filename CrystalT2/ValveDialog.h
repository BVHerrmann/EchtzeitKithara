#pragma once

#include <QWidget>
#include "ui_ValveDialog.h"
#include "QtCore"

class BSwitch;
class SettingsData;
class SerialInterface;
class ValveData;
class MainAppCrystalT2;
class ValveDialog : public QWidget
{
	Q_OBJECT
public:
	ValveDialog(MainAppCrystalT2 *pMainAppCrystalT2, int ValveID);
	~ValveDialog();
	MainAppCrystalT2 *GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
	SerialInterface  *GetSerialInterface() { return m_SerialInterface; }
	SettingsData     *GetSettingsData();
	void             showEvent(QShowEvent *);
	int               GetValveID() { return m_ValveID; }
	void SetPICOComand(const QString &Command, double Value);
	void SetPICOComandDecimal(const QString &command, double Value, int decimal = 2);
	int  SetAllValveParametersToDevice(QString &ErrorMsg);
	void WriteValveParameterCount(double Value);
	void WriteValveParameterPulse(double Value);
	void WriteValveParameterPause(double Value);
	void WriteValveParameterModus(int value);
	void WriteValveParameterCloseVoltage(double value);
	void WriteValveParameterStroke(double value);
	void WriteValveParameterClose(double value);
	void WriteValveParameterOpen(double value);
	void WriteValveParameterTemperature(double Value);
	void WriteValveParameterTemperatureMode(int Value);
    void WriteValveParameterPiezoCurrentOnOff(int Value);
	bool IsValveStatusClicked();
	void HidePicoCommandsDialog();
	void ShowPicoCommandsDialog();
	void HideValveParameterDialog();
	void ShowValveParameterDialog();
	void ShowCycleTime(ValveData &data);
	bool IsValveNotInErrorState();
	void SetSerialCommand(QString &Command);
	void SetComPortParameter(int BaudRate, int Parity, int DataBits, int StopBits, QString &ComPortName);
	int OpenPortAndStartDataExchangeValve(QString &ErrorMsg);
	bool IsSerialPortOpen();
	void SetValveName(const QString &Name) { m_ValveName = Name; }
	void SetValveID(int set) { m_ValveID = set; }
	QString  GetValveName() { return m_ValveName; }
	bool  PortFound();
	int WriteLogFile(const QString &data, const QString &FileName);
	void StopSerialCommuniction();
	void EnableValve();
	void SetGroupName();
	bool IsPicCommandDialogVisible(){return m_IsPicCommandDialogVisible;}
	void SetAuditTrailProperties();
	int  GetCurrentValveModus() { return m_CurrentValveModus; }
	void SetTrigger(bool set);
	bool IsValveHeaterOn();
	double  GetValveHeaterTargetTempterature();
    void WaitCommandIsSet(int TimeOut);
    void ResetIsClicked();
	

public slots:
	//void SlotSendCommand();
	void SlotGetValveStatus();
	void SlotGetHeatersStatus();
	void SlotResetError();
	void SlotShowValveResponse(const QString &ErrorMsg);
	void SlotGetLastErrors();
	void SlotPauseChanged();
	void SlotPulseChanged();
	void SlotCountChanged();
	void SlotCloseVoltageChanged();
	void SlotStrokeChanged();
	void SlotCloseChanged();
	void SlotOpenChanged();
	void SlotTemperatureChanged();
	void SlotModusChanged(int value);
	void SlotTemperatureModeChanged(int value);
	void SlotClearList();
	void SlotCommadSuccessfullySet(const QString &Command, bool Successfully);
	//void SlotTriggerValve(bool State);

signals:
	void SignalShowInfo(const QString &InfoData, QtMsgType MsgType);

private:
	Ui::ValveDialog       ui;
	SerialInterface      *m_SerialInterface;
	MainAppCrystalT2     *m_MainAppCrystalT2;
	QString               m_ComPortName;
	QString               m_ValveName;
	bool                  m_IsValveStatusClicked;
	bool                  m_WindowSetup;
	bool                  m_IsPicCommandDialogVisible;
    bool                  m_ParamterChangedByUser;
	int                   m_ValveID;
	int                   m_CurrentValveModus;
	//BSwitch *m_SwitchSetTrigger;
    QMutex m_MutexWaitForCommandIsSet;
    QWaitCondition m_WaitForCommandIsSet;
};
