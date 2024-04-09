#pragma once
#include <optionpanel.h>

#include <QtWidgets>

class MainAppCrystalT2;
class DeactivateAlarmLevelOptionPanel : public OptionPanel
{
    Q_OBJECT
  public:
    DeactivateAlarmLevelOptionPanel(MainAppCrystalT2* pMainAppCrystalT2);

    MainAppCrystalT2* GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
    void ShowRow(int row, bool sh);
    void SetAuditTrailProperties();
    void SetRequiredAccessLevel();

  private slots:
    void SlotEnableStatusDegreeOfPolution(int);
    void SlotEnableStatusCurrentLiquidTankPreasure(int);
    void SlotEnableStatusCurrentLiquidTankFilling(int);
    void SlotEnableStatusCurrentLiquidTankTemp(int);
    void SlotEnableStatusPiezoTempLeftValve(int);
    void SlotEnableStatusChamperTempLeftValve(int);
    void SlotEnableStatusPiezoTempRightValve(int);
    void SlotEnableStatusChamperTempRightValve(int);
    void SlotEnableStatusSpeedDeviationBetweenCameraAndIS(int);
    void SlotEnableStatusHeatingPipeTemp(int);
    void SlotEnableStatusCounterProducNotFilled(int);
    void SlotEnableStatusCounterBottleEjectedOneAfterTheOther(int);
    void SlotEnableStatusCounterMiddleTooLowOneAfterTheOther(int);
    void SlotEnableStatusAirCooling(int);
    void SlotEnableStatusWaterCooling(int);
    void SlotEnableStatusMixerRunning(int);

  private:
    MainAppCrystalT2* m_MainAppCrystalT2;

    QCheckBox* m_CheckBoxEnableStatusDegreeOfPolution;
    QCheckBox* m_CheckBoxEnableStatusCurrentLiquidTankPreasure;
    QCheckBox* m_CheckBoxEnableStatusCurrentLiquidTankFilling;
    QCheckBox* m_CheckBoxEnableStatusCurrentLiquidTankTemp;
    QCheckBox* m_CheckBoxEnableStatusPiezoTempLeftValve;
    QCheckBox* m_CheckBoxEnableStatusChamperTempLeftValve;
    QCheckBox* m_CheckBoxEnableStatusPiezoTempRightValve;
    QCheckBox* m_CheckBoxEnableStatusChamperTempRightValve;
    QCheckBox* m_CheckBoxEnableStatusSpeedDeviationBetweenCameraAndIS;
    QCheckBox* m_CheckBoxEnableStatusHeatingPipeTemp;
    QCheckBox* m_CheckBoxEnableStatusCounterProducNotFilled;
    QCheckBox* m_CheckBoxEnableStatusCounterBottleEjectedOneAfterTheOther;
    QCheckBox* m_CheckBoxEnableStatusCounterMiddleTooLowOneAfterTheOther;
    QCheckBox* m_CheckBoxEnableStatusAirCooling;
    QCheckBox* m_CheckBoxEnableStatusWaterCooling;
    QCheckBox* m_CheckBoxEnableStatusMixer;


    QLabel* m_TextEnableStatusDegreeOfPolution;
    QLabel* m_TextEnableStatusCurrentLiquidTankPreasure;
    QLabel* m_TextEnableStatusCurrentLiquidTankFilling;
    QLabel* m_TextEnableStatusCurrentLiquidTankTemp;
    QLabel* m_TextEnableStatusPiezoTempLeftValve;
    QLabel* m_TextEnableStatusChamperTempLeftValve;
    QLabel* m_TextEnableStatusPiezoTempRightValve;
    QLabel* m_TextEnableStatusChamperTempRightValve;
    QLabel* m_TextEnableStatusSpeedDeviationBetweenCameraAndIS;
    QLabel* m_TextEnableStatusHeatingPipeTemp;
    QLabel* m_TextEnableStatusCounterProducNotFilled;
    QLabel* m_TextEnableStatusCounterBottleEjectedOneAfterTheOther;
    QLabel* m_TextEnableStatusCounterMiddleTooLowOneAfterTheOther;
    QLabel* m_TextEnableStatusAirCooling;
    QLabel* m_TextEnableStatusWaterCooling;
    QLabel* m_TextEnableStatusMixer;
};