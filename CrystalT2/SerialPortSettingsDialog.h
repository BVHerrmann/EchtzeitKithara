#pragma once

#include <QWidget>
#include "ui_SerialPortSettingsDialog.h"

class MainAppCrystalT2;
class SerialPortSettingsDialog : public QWidget
{
	Q_OBJECT
public:
	SerialPortSettingsDialog(MainAppCrystalT2 *pMainAppCrystalT2 = Q_NULLPTR);
	~SerialPortSettingsDialog();
	MainAppCrystalT2 *GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
	void showEvent(QShowEvent *);
	void SetAuditTrailProperties();

public slots:
	void SlotParityChanged(int index);
	void SlotDataBitsChanged(int index);
	void SlotStopBitsChanged(int index);
	void SlotBaudrateChanged(int index);
	void SlotInterchangeSerialPort(int state);
	void SlotInterchangeTriggerOutput(int state);
	void SlotNumberValvesChanged(int index);

private:
	Ui::SerialPortSettingsDialog  ui;
	MainAppCrystalT2             *m_MainAppCrystalT2;
	bool                          m_WindowSetup;
};
