#pragma once

#include <QWidget>
#include "ui_OverviewDialog.h"

class MainAppCrystalT2;
class ControlsWithColorStatus;
class OverviewDialog : public QWidget
{
    Q_OBJECT
  public:
    explicit OverviewDialog(MainAppCrystalT2* pMainAppCrystalT2, QWidget* parent = NULL);
    MainAppCrystalT2* GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
    void SetValveChamberTemp(double set, int ValveID);
    void SetValvePiezoTemp(double set, int ValveID);
    void SetCounterProductNotFilled(int set);
    void SetCounterProductOk(int set);
    void SetLiquidTankPreasure(double set);
    void SetLiquidTankFillingLevel(double set);
    void SetLiquidTankTemp(double set);
    void SetCounterNumberBottlesRealEjected(double value);
    double GetCurrentPreasureTankTemperature();
    double GetCurrentPreasureTankFillingLevel();
    double GetCurrentHeatingPipeTemperature();
    double GetCurrentLiquidTankPreasure();
    void SetBottlesPerMiniute(double BottlesPerMin);
    void AddMultiImageWidget(QWidget* w);
    void SetDegreeOfPollution(double Value);
    void ShowCheckBoxFirstTrigger(bool show);
    void ShowCheckBoxSecondTrigger(bool show);
    void CheckCheckBoxFirstTrigger(bool check);
    void CheckCheckBoxSecondTrigger(bool check);
    void SetAuditTrailProperties();
    void SetHeatingPipeTemperature(double set);

    int GetStatusLiquidTankPreasure() { return m_StatusLiquidTankPreasure; }
    int GetStatusLiquidTankFilling() { return m_StatusLiquidTankFilling; }
    int GetStatusLiquidTankTemperature() { return m_StatusLiquidTankTemperature; }
    int GetStatusDegreeOfPollution() { return m_StatusDegreeOfPollution; }
    int GetStatusPiezoTempLeftValve() { return m_StatusPiezoTempLeftValve; }
    int GetStatusChamperTempLeftValve() { return m_StatusChamperTempLeftValve; }
    int GetStatusPiezoTempRightValve() { return m_StatusPiezoTempRightValve; }
    int GetStatusChamperTempRightValve() { return m_StatusChamperTempRightValve; }
    int GetStatusHeatingPipeTemperature() { return m_StatusHeatingPipeTemperature; }
    int GetStatusCounterProducNotFilled() {return m_StatusCounterProducNotFilled; }

  public slots:
    void SlotCheckBoxShowFirstChanged(int state);
    void SlotCheckBoxShowSecondChanged(int state);
    void SlotCheckBoxShowBothChanged(int state);

  private:
    Ui::OverviewDialog ui;
    MainAppCrystalT2* m_MainAppCrystalT2;
    ControlsWithColorStatus* m_ControlDegreeOfPollution;
    ControlsWithColorStatus* m_ControlLiquidTankPreasure;
    ControlsWithColorStatus* m_ControlLiquidTankFilling;
    ControlsWithColorStatus* m_ControlLiquidTankTemperature;
    ControlsWithColorStatus* m_ControlPiezoTemperatureLeftValve;
    ControlsWithColorStatus* m_ControlPiezoTemperatureRightValve;
    ControlsWithColorStatus* m_ControlChamberTemperatureLeftValve;
    ControlsWithColorStatus* m_ControlChamberTemperatureRightValve;
    ControlsWithColorStatus* m_ControlChamberHeatingPipeTemperatur;
    ControlsWithColorStatus* m_ControlCounterProducNotFilled;
    int m_StatusLiquidTankPreasure;
    int m_StatusLiquidTankFilling;
    int m_StatusLiquidTankTemperature;
    int m_StatusDegreeOfPollution;
    int m_StatusPiezoTempLeftValve;
    int m_StatusChamperTempLeftValve;
    int m_StatusPiezoTempRightValve;
    int m_StatusChamperTempRightValve;
    int m_StatusHeatingPipeTemperature;
    int m_StatusCounterProducNotFilled;
    int m_NumberErrorLiquidTankPressure;
};
