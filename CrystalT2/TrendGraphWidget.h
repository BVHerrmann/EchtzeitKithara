#pragma once
#include <qwidget.h>

#include "TrendLiquidData.h"
#include "TrendTemperatureData.h"
#include "ui_TrendGraphWidget.h"

class PlotTrendGraph;
class MainAppCrystalT2;
class TrendGraphWidget : public QWidget
{
    Q_OBJECT
  public:
    TrendGraphWidget(MainAppCrystalT2* pMainAppCrystalT2);
    ~TrendGraphWidget();
    MainAppCrystalT2* GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
    PlotTrendGraph* GetPlotTrendGraphTemperature() { return m_PlotTrendGraphTemperature; }
    PlotTrendGraph* GetPlotTrendGraphLiquid() { return m_PlotTrendGraphLiquid; }
    void showEvent(QShowEvent* ev);
    void ClearTrendGraphTemperatureData();
    void ClearTrendGraphLiquidData();
    void ClearTrendGraphTemperatureDataFromFile();
    void ClearTrendGraphLiquidDataFromFile();

    void AddNewTemperatureData(TrendTemperatureData& valveTemperatureData);
    void AddNewTemperatureDataFromFile(TrendTemperatureData& temperatureData);
    void DrawTrendDataTemperature();
    void DrawTrendDataTemperatureFromFile(QDateTime& CurrentDateTime);
    void DrawTrendDataTemperatureInterval(QTime MinTime, QTime MaxTime);
    int GetIndexTrendDataTemperatureByTime(QTime time);
    void AddNewLiquidData(TrendLiquidData& valveLiquidData);
    void AddNewLiquidDataFromFile(TrendLiquidData& valveLiquidData);
    void DrawTrendDataLiquid();
    void DrawTrendDataLiquidFromFile(QDateTime& CurrentDateTime);
    void DrawTrendDataLiquidInterval(QTime MinTime, QTime MaxTime);
    int GetIndexTrendDataLiquidByTime(QTime time);
    QList<TrendTemperatureData>* GetListTemperatureData();
    QList<TrendLiquidData>* GetListLiquidData();
    QList<TrendTemperatureData>* GetListTemperatureDataFromFile();
    QList<TrendLiquidData>* GetListLiquidDataFromFile();
    void CheckFileContent(QString& FileName, int PlotID);
    int LoadDataFromCSVFile(QString& FileName, int PlotID, QDateTime& dateTime);
    void InsertNewTemperatureData(QStringList& List);
    void InsertNewLiquidData(QStringList& List);
    bool WorkWithTwoValves();

    void ShowTimeRangeControlsLiquid();
    void HideTimeRangeControlsLiquid();
    void SetFullTimeRangeLiquid();

    void ShowTimeRangeControlsTemperature();
    void HideTimeRangeControlsTemperature();
    void SetFullTimeRangeTemperature();

    void SetEnableDrawCurrentTrendDataLiquid(bool set) { m_EnableDrawCurrentTrendDataLiquid = set; }
    void SetEnableDrawCurrentTrendDataTemperature(bool set) { m_EnableDrawCurrentTrendDataTemperature = set; }

    double GetLiquidYAxisZoomFactor() { return m_LiquidYAxisZoomFactor; }
    void   SetLiquidYAxisZoomFactor(double set) { m_LiquidYAxisZoomFactor = set; }

    void ShowControlsTimeIntervall(bool sh);
    int GetRollingMeanValueLiquid();
    int GetTrendGraphFlag();
    int GetSliderLiquidMaxYValue();
    int GetSliderLiquidMinYValue();

    int GetTrendGraphAbsolutMaximumTemperature();
    int GetTrendGraphAbsolutMinimumTemperature();
    int GetTrendGraphAbsolutMaximumBottlesPerMin();
    int GetTrendGraphAbsolutMinimumBottlesPerMin();

    void SetTimeValueTimeWindowInMin(int CurrentIndex);

    int WriteLogFile(const QString& data, const QString& FileName);

    void SetTextTimeWindow(const QString& text);

    int GetStartPosDrawLiquidData(QDateTime& CurrentDateTime);
    int GetStartPosDrawTemperatureData(QDateTime& CurrentDateTime);

    void SetRequiredAccessLevel();

  public slots:

    void SlotApplyRangeLiquid();
    void SlotShowFullRangeLiquid();

    void SlotApplyRangeTemperature();
    void SlotShowFullRangeTemperature();

    void SlotMinusTrendGraphTimeWindowChange();
    void SlotPlusTrendGraphTimeWindowChange();

    void SlotSliderLiquidMinYValueChanged(int);
    void SlotSliderLiquidMaxYValueChanged(int);

    void SlotSliderLiquidMinYRelease();
    void SlotSliderLiquidMaxYRelease();

  private:
    Ui::TrendGraphWidget ui;
    MainAppCrystalT2* m_MainAppCrystalT2;
    PlotTrendGraph* m_PlotTrendGraphTemperature;
    PlotTrendGraph* m_PlotTrendGraphLiquid;
    QList<TrendTemperatureData> m_ListValveTemperatureData;
    QList<TrendLiquidData> m_ListValveLiquidData;

    QList<TrendTemperatureData> m_ListValveTemperatureDataFromFile;
    QList<TrendLiquidData> m_ListValveLiquidDataFromFile;
    bool m_EnableDrawCurrentTrendDataLiquid;
    bool m_EnableDrawCurrentTrendDataTemperature;
    QDateTime m_CurrentFileDateLiquid, m_CurrentFileDateTemperature;
    QComboBox* m_ComboBoxTrendGraphTimeWindow;
    int m_RangeTimeAxisInMin;
    double m_LiquidYAxisZoomFactor;
    const int m_kLiquidMinAxisYRange;
};
