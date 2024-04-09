#pragma once

#include <interfaces.h>
#include <QWidget>
#include "ui_GeneralDialog.h"

class MainAppCrystalT2;
class GeneralDialog : public QWidget
{
    Q_OBJECT
  public:
    GeneralDialog(MainAppCrystalT2* pMainAppCrystalT2 = Q_NULLPTR);
    ~GeneralDialog();
    MainAppCrystalT2* GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
    void showEvent(QShowEvent* ev);
    //int GetAccessLevel() { return m_AccessLevel; }
    //void SetAccessLevel(int set) { m_AccessLevel = set; }
    void SetAuditTrailProperties();
    void SetRequiredAccessLevel();
    //void SetPowerSupplyValveOff();
    void SetPowerSupplyValveOn();
    void SetManualEjectionReady();

  public slots:
    void SlotDefaultPreasureChanged();
    void SlotDistanceBottleEjectionChanged();
    void SlotPreasureTankDefaultTempChanged();
    void SlotBandDirectionalChanged(int index);
    void SlotFormatISChanged(int index);
    void SlotBlowOutLenghtChanged();
    void SlotAddMessageNoPowerSupplyValve();
    void SlotManualEject();
    void SlotNumberEjectedBottlesByUser();
    void SlotDefaultHeatingPipeChanged();

  private:
    Ui::GeneralDialog ui;
    MainAppCrystalT2* m_MainAppCrystalT2;
    bool m_WindowsStartUp;
    //int m_AccessLevel;
    QTimer* m_TimerNoPowerSupplyValve;
};
