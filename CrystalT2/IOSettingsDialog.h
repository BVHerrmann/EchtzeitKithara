#pragma once

#include <QWidget>
#include "SharedData.h"
#include "qthread.h"
#include "ui_IOSettingsDialog.h"

class BSwitch;
class MainAppCrystalT2;
class ControlsWithColorStatus;
class IOSettingsDialog : public QWidget
{
    Q_OBJECT
  public:
    IOSettingsDialog(MainAppCrystalT2* parent = Q_NULLPTR);
    ~IOSettingsDialog();
    MainAppCrystalT2* GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
    void showEvent(QShowEvent*);
    void hideEvent(QHideEvent* event);
    void SetDigitalOutput(EtherCATConfigData::IOChannels ChannelIndex, bool State);
    void SetEtherCatCyclus(const QString& Info);
    void SetTestDefaultParameter();
    void SaveFactorOffsetSettings(double factor, double offset);
    double GetMixerSpeedSimulationInRPM();

    /* void SetLiquidTankPreasureSimualtion();
     void SetLiquidTankFillingLevelSimulation();
     void SetLiquidTankTemperatureSimulation();
     void SetHeatingPipeTemperatureSimulation();
     void SetSetControlVotageSimulation();*/

    void SetAuditTrailProperties();
    void SlotAddToListCalibrate();
    void SlotCalibrateFillingLevel();
    void SlotRestoreLastCalibrationData();

    void SetActualPreasureValue(double value);
    void SetActualTankFillingLevel(double value);
    void SetActualAirCoolingCamera(double value);
    void SetActualAirCoolingCameraLight(double value);
    void SetActualAirCoolingValves(double value);
    void SetActualWaterCooling(double value);
    void SetActualFlowTransmitterWaterCoolingCircuit(double value);
    void SetActualTemperaturWaterCoolingReturn(double value);

    void SetENCStatusMixer(ushort m_ENCStatusMixer);
    void SetStatusMixer(ushort StatusMixer);
    void SetPOSStausMixer(double Status);
    void SetActualVelocityMixerRPM(double ActualVelocityMixer);

    int GetStatusMixerVelocity() { return m_StatusControlMixerVelocity; }

    enum AnalogInputCh {
        AI_ACTUAL_AIR_COOLING_CAMERA,
        AI_ACTUAL_AIR_COOLING_CAMERA_LIGHT,
        AI_ACTUAL_AIR_COOLING_VALVE,
        AI_ACTUAL_WATER_COOLING,
        AI_ACTUAL_FLOW_TRANSMITTER_WATER_COOLING_CIRCUIT,
        AI_ACTUAL_TEMPERATURE_WATER_COOLING_RETURN,
        AI_ACTUAL_PREASURE,
        AI_ACTUAL_TANK_FILLING_LEVEL
    };

  public slots:
    void SlotCycleTimeIOTaskChanged();
    void SlotTimePeriodTriggerOutputActive();
    void SlotCameraLight(bool State);
    void SlotTrigger1Valve(bool State);
    void SlotTrigger2Valve(bool State);
    void SlotErrorTransfer(bool State);
    void SlotCounterEjectionTransfer(bool State);
    void SlotPressureTanksHeater(bool State);
    void SlotPressureTanksValve(bool State);
    // void SlotPressureTanksValve2(bool State);
    void SlotValveController(bool State);
    void SlotBottleEjection(bool State);
    void SlotErrorLight(bool State);
    void SlotWarningLight(bool State);
    void SlotTimePeriodDigitalOutputActive();
    void SlotShowDigitalOutputs();

    void SlotActualValvePreasureSimulation();
    void SlotPreasureTankFillingLevelSimulation();
    void SlotPreasureTankTemperatureSimulation();
    void SlotActualAirCoolingCameraSimulation();
    void SlotActualAirCoolingCameraLightSimulation();
    void SlotActualAirCoolingValvesSimulation();
    void SlotActualWaterCoolingSimulation();
    void SlotActualFlowTransmitterWaterCoolingCircuitSimulation();
    void SlotActualTemperaturWaterCoolingReturnSimulation();

    void SlotSetControlVotageSimulation(bool State);
    void SlotSetRealEjectionSimulation(bool State);
    void SlotBlueLightPermanentEjectionIS(bool State);
    void SlotWhiteLightBottleEjection(bool State);
    void SlotEjectionIS(bool State);
    void SlotHeatingPipeTemperatureSimulation();
    void SlotSelectAnalogInputTypeChanged(int);
    void SlotStartMixer();
    void SlotStopMixer();
    //void SlotSetMixerSpeedSimulation();
    void SlotSetPowerMixerSimulation(bool State);
    void SlotSetMixerStartStopSimulation(bool State);

  private:
    Ui::IOSettingsDialog ui;
    MainAppCrystalT2* m_MainAppCrystalT2;
    ushort m_StatusMixer;
    ushort m_ENCStatusMixer;
    bool m_SetupWindow;
    bool m_testBool;

    double m_LastOffsetAnalogInputAirCoolingCamera;
    double m_LastFactorAnalogInputAirCoolingCamera;
    double m_LastOffsetAnalogInputAirCoolingCameraLight;
    double m_LastFactorAnalogInputAirCoolingCameraLight;
    double m_LastOffsetAnalogInputAirCoolingValves;
    double m_LastFactorAnalogInputAirCoolingValves;
    double m_LastOffsetAnalogInputWaterCooling;
    double m_LastFactorAnalogInputWaterCooling;
    double m_LastOffsetAnalogInputFlowTransmitterWaterCoolingCircuit;
    double m_LastFactorAnalogInputFlowTransmitterWaterCoolingCircuit;
    double m_LastOffsetAnalogInputTemperaturWaterCoolingReturn;
    double m_LastFactorAnalogInputTemperaturWaterCoolingReturn;
    double m_LastOffsetAnalogInputCurrentPreasure;
    double m_LastFactorAnalogInputCurrentPreasure;
    double m_LastOffsetAnalogInputTankFillingLevel;
    double m_LastFactorAnalogInputTankFillingLevel;

    // int m_ControlVoltageOnOffTestValue;
    int m_CurrentCalibrateAnalogInputIndex;
    BSwitch* m_SwitchSetControlVoltage;
    BSwitch* m_SwitchRealEjection;
    BSwitch* m_SwitchCameraLight;
    BSwitch* m_SwitchSetTrigger1Valve;
    BSwitch* m_SwitchSetTrigger2Valve;
    BSwitch* m_SwitchSlotErrorLight;
    BSwitch* m_SwitchSlotErrorTransfer;
    BSwitch* m_SwitchCounterEjectionTransfer;
    BSwitch* m_SwitchPressureTanksHeater;
    BSwitch* m_SwitchPressureTanksValve;
    BSwitch* m_SwitchValveController;
    BSwitch* m_SwitchBottleEjection;
    BSwitch* m_SwitchBlueLightPermanentEjectionIS;
    BSwitch* m_SwitchWhiteLightBottleEjection;
    BSwitch* m_SwitchOrangeLightWarning;
    BSwitch* m_SwitchPressureTanksValve2;
    BSwitch* m_SwitchEjetionIS;
    BSwitch* m_MixerPowerOnOffSimulation;
    BSwitch* m_MixerStartStopSimulation;
    QTimer* m_TimerShowDigitalOutputs;

    ControlsWithColorStatus* m_ControlMixerVelocity;
    int m_StatusControlMixerVelocity;
    unsigned long long m_NumberErrorMixer;
};
