#pragma once

#include <QWidget>
#include "ui_CoolingDialog.h"

class MainAppCrystalT2;
class ControlsWithColorStatus;
class CoolingDialog : public QWidget
{
    Q_OBJECT

  public:
    CoolingDialog(MainAppCrystalT2* pMainAppCrystalT2 = nullptr);
    ~CoolingDialog();
    void showEvent(QShowEvent* ev);
    void SetActualAirCoolingCamera(double dvalue);
    void SetActualAirCoolingCameraLight(double dvalue);
    void SetActualAirCoolingValves(double dvalue);
    void SetActualWaterCooling(double value);
    void SetActualFlowTransmitterWaterCoolingCircuit(double value);
    void SetActualTemperaturWaterCoolingReturn(double value);
    MainAppCrystalT2* GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
    int GetStatusAirCoolingCameraAndLightActualValue() { return m_StatusAirCoolingCameraAndLightActualValue; }
    int GetStatusAirCoolingGlassActualValue() { return m_StatusAirCoolingGlassActualValue; }
    int GetStatusAirCoolingValvesActualValue() { return m_StatusAirCoolingValvesActualValue; }
    int GetStatusWaterCoolingActualValue() { return m_StatusWaterCoolingActualValue; }
    int GetStatusFlowTransmitterWaterCoolingCircuitActualValue() { return m_StatusFlowTransmitterWaterCoolingCircuitActualValue; }
    int GetStatusWaterCoolingTemperatureReturnActualValue() { return m_StatusWaterCoolingTemperatureReturnActualValue; }
    void SetRequiredAccessLevel();
    void SetAuditTrailProperties();
    double GetActualTemperaturWaterCoolingReturn();
    //void SetWaterCoolingStrokeValue(double set);

  public slots:
    void SlotAirCoolingCameraDefaultValueChanged();
    void SlotAirCoolingCameraLightDefaultValueChanged();
    void SlotAirCoolingValvesDefaultValueChanged();

    //void SlotWaterCoolingDefaultValueChanged();
    void SlotWaterCoolingCircuitDefaultSensorChanged();
    void SlotWaterCoolingTemperatureDefaultChanged();

  private:
    Ui::CoolingDialogClass ui;
    MainAppCrystalT2* m_MainAppCrystalT2;
    ControlsWithColorStatus* m_ControlAirCoolingCameraActualValue;
    ControlsWithColorStatus* m_ControlAirCoolingCameraLightActualValue;
    ControlsWithColorStatus* m_ControlAirCoolingValvesActualValue;
    ControlsWithColorStatus* m_ControlWaterCoolingActualValue;
    ControlsWithColorStatus* m_ControlFlowTransmitterWaterCoolingCircuitActualValue;
    ControlsWithColorStatus* m_ControlWaterCoolingTemperatureReturnActualValue;
    int m_StatusAirCoolingCameraAndLightActualValue;
    int m_StatusAirCoolingGlassActualValue;
    int m_StatusAirCoolingValvesActualValue;

    int m_StatusWaterCoolingActualValue;
    int m_StatusFlowTransmitterWaterCoolingCircuitActualValue;
    int m_StatusWaterCoolingTemperatureReturnActualValue;
    bool m_WindowsStartUp;
    double m_TemperaturWaterCooling;
};
