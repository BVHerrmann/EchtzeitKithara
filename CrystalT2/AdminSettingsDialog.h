#pragma once
#include <qwidget.h>
#include "ui_AdminSettingsDialog.h"
#include <interfaces.h>

class MainAppCrystalT2;
//class ControlsWithColorStatus;
class AdminSettingsDialog : public QWidget
{
    Q_OBJECT
  public:
    AdminSettingsDialog(MainAppCrystalT2* pMainAppCrystalT2 = Q_NULLPTR);
    ~AdminSettingsDialog();
    MainAppCrystalT2* GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
    //bool event(QEvent* pEvent);
    void showEvent(QShowEvent*);
    //void SetMeasuredPixelSize(double set, bool Status);
    //void SetMeasuredNeckDiameter(double set, bool Status);
    //void SetNewPixelSize();
    //void SetForceEditingFinishedPixelSize();
    void SetupWindow();
    void SetAuditTrailProperties();
    //void SetSpeed(double set);
    //void SetSpeedFromIS(double set);
    //void SetDeltaSpeed(double set);
    //void SetProductPresentTime(double set);
    //void SetInspectionTime(double set);
    //void SetRealTimeInterval(double set);
    void SetInfoLevel(int set);
   
   // double GetSpeedFromIS();
   // double GetSpeed();
    //int GetStatusSpeedDeviationBetweenCameraAndIS() { return m_StatusSpeedDeviationBetweenCameraAndIS; }
    void SetCurrentMaschineState(PluginInterface::MachineState set);

  signals:
    void SignalMachineStateChanged(int);

  public slots:
    //void SlotApplyNewPixelSize();
    //void SlotPixelSizeChanged();
    void SlotUsedTriggerChanged(int index);
    void SlotUseSpeedCalcEjectionTimeChanged(int index);
    //void SlotIntervalCheckCleanImageChanged();

    void SlotInfoLevelChanged();
    //void SlotSaveIntervalShowTrendGraph();
    void SlotAcceptanceThresholdChanged();

    void SlotTankFillingLevelWarningChanged();
    void SlotTankFillingLevelAlarmChanged();
    void SlotTankFillingLevelMaschineStopChanged();
    void SlotTankFillingLevelMaxValueChanged();

    void SlotTankHeaterLevelWarningChanged();
    void SlotTankHeaterLevelAlarmChanged();
    void SlotTankHeaterLevelMaschineStopChanged();

    void SlotTankPreasureLevelWarningChanged();
    void SlotTankPreasureLevelAlarmChanged();
    void SlotTankPreasureLevelMaschineStopChanged();

    void SlotDegreeOfPollutionMaschineStopChanged();
    void SlotDegreeOfPollutionLevelAlarmChanged();
    void SlotDegreeOfPollutionLevelWarningChanged();

    void SlotValveChamberTemperatureWarningLevelChanged();
    void SlotValveChamberTemperatureAlarmLevelChanged();
    void SlotValveChamberTemperatureMaschineStopInSecChanged();

    void SlotValvePiezoTemperatureWarningLevelChanged();
    void SlotValvePiezoTemperatureAlarmLevelChanged();
    void SlotValvePiezoTemperatureMaschineStopInSecChanged();

    void SlotMaxSpeedDeviationWarningChanged();
    void SlotMaxSpeedDeviationAlarmChanged();
    void SlotMaxSpeedDeviationMaschineStopChanged();

    void SlotMaxEjectedBottlesChanged();
    void SlotSpinBoxMaxBottlesNotFilledChanged();
    void SlotMaschineStateChanged(int);
    void SlotMaxNumberFilesTrendGraphChanged();

    void SlotWarnLevelHeatingPipeChanged();
    void SlotAlarmLevelHeatingPipeChanged();
    void SlotMaschineStopHeatingPipeChanged();

    void SlotCounterProducktNotFilledWarningChanged();
    void SlotCounterProducktNotFilledAlarmChanged();
    void SlotCounterProducktNotFilledMaschineStopChanged();

    void SlotAirCoolingWarningChanged();
    void SlotAirCoolingAlarmChanged();
    void SlotAirCoolingMaschineStopChanged();

    void SlotWaterCoolingWarningChanged();
    void SlotWaterCoolingAlarmChanged();
    void SlotWaterCoolingMaschineStopChanged();

    void SlotAlarmLevelMixerVelocityChanged();
    void SlotWarnLevelMixerVelocityChanged();
    void SlotMaschineStopMixerVelocityChanged();

    void SlotSpeedMixerChanged();
    void SlotPreasureIncreaseWhenFlushingChanged();

  private:
    Ui::AdminSettingsDialog ui;
    MainAppCrystalT2* m_MainAppCrystalT2;
    //ControlsWithColorStatus* m_SpeedDeviationBetweenCameraAndIS;
    //int m_CounterShowPixelSize;
    //int m_NumberSameMeasureValues;
    //int m_CounterShowNeckDiameter;
    //int m_StatusSpeedDeviationBetweenCameraAndIS;
    bool m_WindowSetup;
};
