#pragma once

#include <QtCharts>
#include <QtWidgets>

class TrendTemperatureData;
class TrendGraphWidget;
class TrendLiquidData;
class PlotTrendGraph : public QChartView
{
    Q_OBJECT
  public:
    PlotTrendGraph(TrendGraphWidget* pTrendGraphWidget, QWidget* FrameParent, int PlotID);
    ~PlotTrendGraph();
    TrendGraphWidget* GetTrendGraphWidget() { return m_TrendGraphWidget; }
    void DrawTrendDataTemperature(QList<TrendTemperatureData>& pListData, QDateTime CurrentDateTime, bool fromFile = false, int MinIndex = 0, int MaxIndex = -1);
    void DrawTrendDataLiquid(QList<TrendLiquidData>& pListData, QDateTime CurrentDateTime, bool fromFile = false, int MinIndex = 0, int MaxIndex = -1);
    void GetMaxMinTemperature(QList<TrendTemperatureData>& pListData, double& MaxTemp, double& MinTemp);
    void GetMaxMinLiquid(QList<TrendLiquidData>& pListData, double& MaxLiquid, double& MinLiquid, bool WorkWithTwoValves);
    void GetMaxMinBottlesPerMin(QList<TrendLiquidData>& pListData, double& MaxValue, double& MinValue);
    void AddNewSeriesTemperature(QLineSeries*& seriesStackTemperatureLeftValve, QLineSeries*& seriesCurrentTemperatureLeftValve, QLineSeries*& seriesStackTemperatureRightValve,
                                 QLineSeries*& seriesCurrentTemperatureRightValve, QLineSeries*& seriesPreasuretankTemperature, QLineSeries*& seriesHeatingPipeTemperature,
                                 QLineSeries*& seriesWaterCooling);
    void AddNewSeriesLiquid(QScatterSeries*& seriesAmountLiquidLeftValve, QLineSeries*& seriesRollingMeanAmountLiquidLeftValve, QLineSeries*& seriesRollingStdPlusAmountLiquidLeftValve,
                            QLineSeries*& seriesRollingStdMinusAmountLiquidLeftValve, QScatterSeries*& seriesAmountLiquidRightValve, QLineSeries*& seriesRollingMeanAmountLiquidRightValve,
                            QLineSeries*& seriesRollingStdPlusAmountLiquidRightValve, QLineSeries*& seriesRollingStdMinusAmountLiquidRightValve, QScatterSeries*& seriesSumAmountLiquid,
                            QLineSeries*& seriesRollingMeanSumAmountLiquid, QLineSeries*& seriesRollingStdPlusSumAmountLiquid, QLineSeries*& seriesRollingStdMinusSumAmountLiquid,
                            QLineSeries*& seriesBottlesPerminute, QScatterSeries*& seriesBottleEjected);

    void AddSeriesMaschineState(QList<QPair<qint64, double>>& ListCoordinateMaschineStaeChanged, bool MaschineStateProductionIsFirst, qint64 MSecsSinceEpoch);

  private:
    TrendGraphWidget* m_TrendGraphWidget;
    QChart* m_Chart;
    QDateTimeAxis* m_xAxis;               // Diagrammachsen
    QValueAxis* m_yAxis;                  // Diagrammachsen
    QValueAxis* m_yAxisBottlesPerMinute;  // Diagrammachsen
    QValueAxis* m_yAxisBottleEjected;     // Diagrammachsen
    QGraphicsScene* m_GrapicSence;
    QRubberBand *m_rubberBand;
    bool m_TheLeftValveIsFilledFirst;
    int m_PlotID;
    int m_NumberBottlesEjected;
    int m_TrendGraphFlag;
    double m_RollingMeanSumLiquid, m_RollingMeanLiquidLeftValve, m_RollingMeanLiquidRightValve;
    double m_RollingStdSumLiquid, m_RollingStdLiquidLeftValve, m_RollingStdLiquidRightValve;
    double m_BottlesPerMinute;
    double m_StackTemperatureLeftValve, m_CurrentTemperatureLeftValve, m_StackTemperatureRightValve, m_CurrentTemperatureRightValve, m_CurrentHeatingPipeTemperature, 
        m_CurrentPreasureTankTemperature,m_CurrentWaterCoolingTemperature;
};
