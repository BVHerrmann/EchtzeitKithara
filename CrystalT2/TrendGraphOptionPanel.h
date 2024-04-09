#pragma once
#include <optionpanel.h>
#include <QtWidgets>

class MainAppCrystalT2;
class TrendGraphOptionPanel : public OptionPanel
{
    Q_OBJECT
  public:
    TrendGraphOptionPanel(MainAppCrystalT2* pMainAppCrystalT2);
    MainAppCrystalT2* GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
    QFileInfoList GetTrendFileInfoList();
    void showEvent(QShowEvent* ev);

  private:
    void setupUi();

  public slots:
    void SlotUnLoadFile();
    void SlotLoadFile();
    void SlotSetCheckedShowOnlySumLiquid(int set);
    void SlotSetCheckedShowOnlyLiquidLeftValve(int set);
    void SlotSetCheckedShowOnlyLiquidRightValve(int set);
    void SlotSetCheckedShowDataPointsRightValve(int set);
    void SlotSetCheckedShowStdRightValve(int set);
    void SlotSetCheckedShowDataPointsLeftValve(int set);
    void SlotSetCheckedShowStdLeftValve(int set);
    void SlotSetCheckedBottlesPerMin(int set);
    void SlotSetCheckedEjectedBottles(int set);
    void SlotCheckBoxShowPiezoLeft(int set);
    void SlotCheckBoxShowPiezoRight(int set);
    void SlotCheckBoxShowChamberLeft(int set);
    void SlotCheckBoxShowChamberRight(int set);
    void SlotCheckBoxShowPreasureTank(int set);
    void SlotCheckBoxShowHeatingPipe(int set);
    void SlotRollingMeanSizeChanged();
    void SlotCheckBoxWaterCooling(int set);

  private:
    MainAppCrystalT2* m_MainAppCrystalT2;
    QFormLayout* m_FormLayout;
    QListWidget* m_ListWidgetLoadFile;
    QSpinBox* m_SpinBoxRollingMeanSize;
};