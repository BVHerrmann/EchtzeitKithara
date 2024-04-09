#include "PlotTrendGraph.h"
#include "GlobalConst.h"
#include "TrendGraphWidget.h"
#include "colors.h"

#include <boost/accumulators/framework/accumulator_set.hpp>
#include <boost/accumulators/framework/extractor.hpp>
#include <boost/accumulators/statistics/rolling_mean.hpp>
#include <boost/accumulators/statistics/rolling_variance.hpp>
#include <boost/fusion/functional.hpp>

using namespace boost::accumulators;

PlotTrendGraph::PlotTrendGraph(TrendGraphWidget* pTrendGraphWidget, QWidget* FrameParent, int PlotID)
    : QChartView(FrameParent),
      m_Chart(NULL),
      m_xAxis(NULL),
      m_yAxis(NULL),
      m_GrapicSence(NULL),
      m_TrendGraphWidget(NULL),
      m_yAxisBottleEjected(NULL),
      m_PlotID(TREND_GRAPH_PLOT_ID_TEMPERATURE),
      m_TrendGraphFlag(0),
      m_StackTemperatureLeftValve(0),
      m_CurrentTemperatureLeftValve(0),
      m_StackTemperatureRightValve(0),
      m_CurrentTemperatureRightValve(0),
      m_CurrentPreasureTankTemperature(0),
      m_CurrentHeatingPipeTemperature(0),
      m_CurrentWaterCoolingTemperature(0),
      m_TheLeftValveIsFilledFirst(true)
{
    m_RollingMeanSumLiquid = m_RollingMeanLiquidLeftValve = m_RollingMeanLiquidRightValve = -1.0;
    m_RollingStdSumLiquid = m_RollingStdLiquidLeftValve = m_RollingStdLiquidRightValve = -1.0;

    m_NumberBottlesEjected = 0;
    QChar degree = (0x00b0);
    QFont font;
    const int kTickIntervalTemperature = 10;
    const int kTickIntervalBottlesPerMin = 100;
    const int kPixelSize = 18;

    font.setPixelSize(kPixelSize);
    m_TrendGraphWidget = pTrendGraphWidget;
    m_PlotID = PlotID;
    m_Chart = new QChart();
    m_Chart->setBackgroundVisible(false);
    m_Chart->setMargins(QMargins(0, 0, 0, 0));
    m_Chart->legend()->setAlignment(Qt::AlignBottom);
    m_Chart->legend()->setVisible(true);
    m_Chart->setTitleFont(font);
    m_Chart->legend()->setFont(font);

    m_xAxis = new QDateTimeAxis();
    m_xAxis->setTickCount(30);
    m_xAxis->setFormat("h:mm:ss");
    m_xAxis->setMax(QDateTime::currentDateTime().addSecs(60));
    m_xAxis->setMin(QDateTime::currentDateTime());
    m_xAxis->setLabelsColor(HMIColor::Grey);
    m_xAxis->setGridLineColor(HMIColor::LightGrey);
    m_xAxis->setTitleBrush(QBrush(HMIColor::Grey));
    m_Chart->addAxis(m_xAxis, Qt::AlignBottom);

    m_yAxis = new QValueAxis();
    m_yAxis->setTickType(QValueAxis::TicksDynamic);
    if (PlotID == TREND_GRAPH_PLOT_ID_TEMPERATURE) {
        m_yAxis->setLabelFormat(QString("%d"));
        m_yAxis->setRange(GetTrendGraphWidget()->GetTrendGraphAbsolutMinimumTemperature(), GetTrendGraphWidget()->GetTrendGraphAbsolutMaximumTemperature());
        m_yAxis->setTickInterval(kTickIntervalTemperature);
        m_yAxis->setTitleText(tr("Temperature(C)"));
        m_Chart->setTitle(tr("Trend Graph Temperature"));
    } else {
        m_yAxis->setLabelFormat(QString("%d"));
        m_yAxis->setRange(GetTrendGraphWidget()->GetSliderLiquidMinYValue(), GetTrendGraphWidget()->GetSliderLiquidMaxYValue());
        m_yAxis->setTickInterval((GetTrendGraphWidget()->GetSliderLiquidMaxYValue() - GetTrendGraphWidget()->GetSliderLiquidMinYValue()) / 8);
        m_yAxis->setTitleText(tr("Liquid(Pix)"));
        m_Chart->setTitle(tr("Trend Graph Liquid"));
        m_yAxisBottlesPerMinute = new QValueAxis();
        m_yAxisBottlesPerMinute->setTickType(QValueAxis::TicksDynamic);
        m_yAxisBottlesPerMinute->setLabelFormat(QString("%d"));
        m_yAxisBottlesPerMinute->setRange(GetTrendGraphWidget()->GetTrendGraphAbsolutMinimumBottlesPerMin(), GetTrendGraphWidget()->GetTrendGraphAbsolutMaximumBottlesPerMin());
        m_yAxisBottlesPerMinute->setTickInterval(kTickIntervalBottlesPerMin);
        m_yAxisBottlesPerMinute->setTitleText(tr("Bottles/min"));
        m_Chart->addAxis(m_yAxisBottlesPerMinute, Qt::AlignRight);

        m_yAxisBottleEjected = new QValueAxis();
        m_yAxisBottleEjected->setRange(0, 2);
        m_yAxisBottleEjected->setLineVisible(false);
        m_yAxisBottleEjected->setVisible(false);
        m_Chart->addAxis(m_yAxisBottleEjected, Qt::AlignRight);
    }
    m_yAxis->setLabelsColor(HMIColor::Grey);
    m_yAxis->setGridLineColor(HMIColor::LightGrey);
    m_yAxis->setTitleBrush(QBrush(HMIColor::Grey));
    m_Chart->addAxis(m_yAxis, Qt::AlignLeft);

    setChart(m_Chart);
    m_GrapicSence = scene();
}

PlotTrendGraph::~PlotTrendGraph()
{
}

void PlotTrendGraph::DrawTrendDataLiquid(QList<TrendLiquidData>& pListData, QDateTime CurrentDateTime, bool fromFile, int MinIndex, int MaxIndex)
{
    QScatterSeries* seriesAmountLiquidLeftValve = NULL;
    QScatterSeries* seriesAmountLiquidRightValve = NULL;
    QScatterSeries* seriesSumAmountLiquid = NULL;
    QLineSeries* seriesRollingMeanAmountLiquidLeftValve = NULL;
    QLineSeries* seriesRollingMeanAmountLiquidRightValve = NULL;
    QLineSeries* seriesRollingMeanSumAmountLiquid = NULL;

    QLineSeries* seriesRollingStdPlusAmountLiquidLeftValve = NULL;
    QLineSeries* seriesRollingStdPlusAmountLiquidRightValve = NULL;
    QLineSeries* seriesRollingStdPlusSumAmountLiquid = NULL;

    QLineSeries* seriesRollingStdMinusAmountLiquidLeftValve = NULL;
    QLineSeries* seriesRollingStdMinusAmountLiquidRightValve = NULL;
    QLineSeries* seriesRollingStdMinusSumAmountLiquid = NULL;

    QLineSeries* seriesBottlesPerMinute = NULL;
    QScatterSeries* seriesBottleEject = NULL;

    accumulator_set<double, stats<tag::rolling_mean>>* pRollingMeanSumAmountLiquid = NULL;
    accumulator_set<double, stats<tag::rolling_mean>>* pRollingMeanAmountLiquidLeftValve = NULL;
    accumulator_set<double, stats<tag::rolling_mean>>* pRollingMeanAmountLiquidRightValve = NULL;

    accumulator_set<double, stats<tag::rolling_mean>>* pRollingVarianceSumAmountLiquid = NULL;
    accumulator_set<double, stats<tag::rolling_mean>>* pRollingVarianceAmountLiquidLeftValve = NULL;
    accumulator_set<double, stats<tag::rolling_mean>>* pRollingVarianceAmountLiquidRightValve = NULL;

    double MaxLiquid = 0;
    double MinLiquid = 4000;
    double YOffsetRange = 100;
    int CountLiquidFirst = -1;
    int CountLiquidSecond = -1;
    int CountLiquidSum = -1;
    int CountBottlesPerMin = -1;
    int CountBottleEjected = -1;
    int RollingMeanSize = 1;  // wird schon auf der Echtzeitseite berechnet
    int OffsetIndex = 1;
    int NumPoints = 0;
    bool SeriesDataIn = true;
    bool WorkWithTwoValves = true;
    int CurrentMaschineState = PluginInterface::MachineState::Off;
    qint64 MSecsSinceEpoch = 0;
    QDateTime MinDateTime, MaxDateTime;
    QList<QLegendMarker*> ListMarkers;
    QRectF PlotArea = m_Chart->plotArea();
    double MaxPointsInView = PlotArea.width();
    QList<QPair<qint64, double>> ListCoordinateMaschineStateChanged;
    bool MaschineStateProductionIsFirst = true;

    if (m_TrendGraphWidget) {
        m_TrendGraphFlag = m_TrendGraphWidget->GetTrendGraphFlag();
    }
    if (MaxIndex == -1) MaxIndex = pListData.count() - 1;
    m_Chart->removeAllSeries();
    if (pListData.count() > 1) {
        m_TheLeftValveIsFilledFirst = pListData.at(0).m_TheLeftValveIsFilledFirst;
        if (m_TrendGraphWidget) {
            WorkWithTwoValves = m_TrendGraphWidget->WorkWithTwoValves();
            MaxLiquid = m_TrendGraphWidget->GetSliderLiquidMaxYValue();
            MinLiquid = m_TrendGraphWidget->GetSliderLiquidMinYValue();
            // RollingMeanSize = m_TrendGraphWidget->GetRollingMeanValueLiquid();
        }
        MinDateTime = MaxDateTime = CurrentDateTime;
        MinDateTime.setTime(pListData.at(MinIndex).m_Time);
        MaxDateTime.setTime(pListData.at(MaxIndex).m_Time);
        m_xAxis->setRange(MinDateTime, MaxDateTime);

        m_yAxis->setTickInterval((MaxLiquid - MinLiquid) / 8);
        m_yAxis->setRange(MinLiquid, MaxLiquid);

        m_RollingMeanSumLiquid = m_RollingMeanLiquidLeftValve = m_RollingMeanLiquidRightValve = -1.0;
        m_RollingStdSumLiquid = m_RollingStdLiquidLeftValve = m_RollingStdLiquidRightValve = -1.0;
        m_BottlesPerMinute = 0.0;
        m_NumberBottlesEjected = 0;
        if (!fromFile) {
            NumPoints = MaxIndex - MinIndex;
            if (MaxPointsInView > 0) {
                OffsetIndex = static_cast<int>((double)NumPoints / MaxPointsInView);
                OffsetIndex++;
            }
        }
        for (int i = MinIndex; i < (MaxIndex + 1); i = i + OffsetIndex) {
            CurrentDateTime.setTime(pListData.at(i).m_Time);
            MSecsSinceEpoch = CurrentDateTime.toMSecsSinceEpoch();
            if (!pListData.at(i).m_SoftwareStart) {
                if (!pListData.at(i).m_SoftwareFinished &&
                    (pListData.at(i).m_MaschineState == PluginInterface::MachineState::Production || pListData.at(i).m_MaschineState == PluginInterface::MachineState::Setup)) {
                    SeriesDataIn = false;
                    if (WorkWithTwoValves) {
                        if (m_TrendGraphFlag & GRAPH_SHOW_DROPLET_SUM_MEAN) {
                            if (pRollingMeanSumAmountLiquid == NULL)
                                pRollingMeanSumAmountLiquid = new accumulator_set<double, stats<tag::rolling_mean>>(tag::rolling_window::window_size = RollingMeanSize);
                            if (pRollingVarianceSumAmountLiquid == NULL)
                                pRollingVarianceSumAmountLiquid = new accumulator_set<double, stats<tag::rolling_mean>>(tag::rolling_window::window_size = RollingMeanSize);
                            pRollingMeanSumAmountLiquid->operator()(pListData.at(i).m_SumAmountLiquid);
                            pRollingVarianceSumAmountLiquid->operator()(pListData.at(i).m_StandardDeviationSumAmountLiquid);
                            m_RollingMeanSumLiquid = rolling_mean(*pRollingMeanSumAmountLiquid);
                            m_RollingStdSumLiquid = (rolling_mean(*pRollingVarianceSumAmountLiquid));

                            if (seriesSumAmountLiquid == NULL && (m_TrendGraphFlag & GRAPH_SHOW_DROPLET_LEFT_VALVE_RAW_POINTS)) seriesSumAmountLiquid = new QScatterSeries();
                            if (seriesRollingMeanSumAmountLiquid == NULL && (m_TrendGraphFlag & GRAPH_SHOW_DROPLET_LEFT_VALVE_MEAN)) seriesRollingMeanSumAmountLiquid = new QLineSeries();
                            if (seriesRollingStdPlusSumAmountLiquid == NULL && (m_TrendGraphFlag & GRAPH_SHOW_DROPLET_LEFT_VALVE_STD)) seriesRollingStdPlusSumAmountLiquid = new QLineSeries();
                            if (seriesRollingStdMinusSumAmountLiquid == NULL && (m_TrendGraphFlag & GRAPH_SHOW_DROPLET_LEFT_VALVE_STD)) seriesRollingStdMinusSumAmountLiquid = new QLineSeries();

                            if (seriesSumAmountLiquid) seriesSumAmountLiquid->append(MSecsSinceEpoch, pListData.at(i).m_SumAmountLiquid);
                            if (seriesRollingMeanSumAmountLiquid) seriesRollingMeanSumAmountLiquid->append(MSecsSinceEpoch, m_RollingMeanSumLiquid);
                            if (seriesRollingStdPlusSumAmountLiquid) seriesRollingStdPlusSumAmountLiquid->append(MSecsSinceEpoch, m_RollingMeanSumLiquid + m_RollingStdSumLiquid);
                            if (seriesRollingStdMinusSumAmountLiquid) seriesRollingStdMinusSumAmountLiquid->append(MSecsSinceEpoch, m_RollingMeanSumLiquid - m_RollingStdSumLiquid);
                        }
                        if (m_TrendGraphFlag & GRAPH_SHOW_DROPLET_LEFT_VALVE_MEAN || m_TrendGraphFlag & GRAPH_SHOW_DROPLET_LEFT_VALVE_STD ||
                            m_TrendGraphFlag & GRAPH_SHOW_DROPLET_LEFT_VALVE_RAW_POINTS) {
                            if (pRollingMeanAmountLiquidLeftValve == NULL)
                                pRollingMeanAmountLiquidLeftValve = new accumulator_set<double, stats<tag::rolling_mean>>(tag::rolling_window::window_size = RollingMeanSize);
                            if (pRollingVarianceAmountLiquidLeftValve == NULL)
                                pRollingVarianceAmountLiquidLeftValve = new accumulator_set<double, stats<tag::rolling_mean>>(tag::rolling_window::window_size = RollingMeanSize);

                            pRollingMeanAmountLiquidLeftValve->operator()(pListData.at(i).m_AmountLiquidLeftValve);
                            pRollingVarianceAmountLiquidLeftValve->operator()(pListData.at(i).m_StandardDeviationAmountLiquidLeftValve);
                            m_RollingMeanLiquidLeftValve = rolling_mean(*pRollingMeanAmountLiquidLeftValve);
                            m_RollingStdLiquidLeftValve = rolling_mean(*pRollingVarianceAmountLiquidLeftValve);

                            if (seriesAmountLiquidLeftValve == NULL && (m_TrendGraphFlag & GRAPH_SHOW_DROPLET_LEFT_VALVE_RAW_POINTS)) seriesAmountLiquidLeftValve = new QScatterSeries();
                            if (seriesRollingMeanAmountLiquidLeftValve == NULL && (m_TrendGraphFlag & GRAPH_SHOW_DROPLET_LEFT_VALVE_MEAN)) seriesRollingMeanAmountLiquidLeftValve = new QLineSeries();
                            if (seriesRollingStdPlusAmountLiquidLeftValve == NULL && (m_TrendGraphFlag & GRAPH_SHOW_DROPLET_LEFT_VALVE_STD))
                                seriesRollingStdPlusAmountLiquidLeftValve = new QLineSeries();
                            if (seriesRollingStdMinusAmountLiquidLeftValve == NULL && (m_TrendGraphFlag & GRAPH_SHOW_DROPLET_LEFT_VALVE_STD))
                                seriesRollingStdMinusAmountLiquidLeftValve = new QLineSeries();

                            if (seriesAmountLiquidLeftValve) seriesAmountLiquidLeftValve->append(MSecsSinceEpoch, pListData.at(i).m_AmountLiquidLeftValve);
                            if (seriesRollingMeanAmountLiquidLeftValve) seriesRollingMeanAmountLiquidLeftValve->append(MSecsSinceEpoch, m_RollingMeanLiquidLeftValve);
                            if (seriesRollingStdPlusAmountLiquidLeftValve)
                                seriesRollingStdPlusAmountLiquidLeftValve->append(MSecsSinceEpoch, m_RollingMeanLiquidLeftValve + m_RollingStdLiquidLeftValve);
                            if (seriesRollingStdMinusAmountLiquidLeftValve)
                                seriesRollingStdMinusAmountLiquidLeftValve->append(MSecsSinceEpoch, m_RollingMeanLiquidLeftValve - m_RollingStdLiquidLeftValve);
                        }
                        if (m_TrendGraphFlag & GRAPH_SHOW_DROPLET_RIGHT_VALVE_MEAN || m_TrendGraphFlag & GRAPH_SHOW_DROPLET_RIGHT_VALVE_STD ||
                            m_TrendGraphFlag & GRAPH_SHOW_DROPLET_RIGHT_VALVE_RAW_POINTS) {
                            if (pRollingMeanAmountLiquidRightValve == NULL)
                                pRollingMeanAmountLiquidRightValve = new accumulator_set<double, stats<tag::rolling_mean>>(tag::rolling_window::window_size = RollingMeanSize);
                            if (pRollingVarianceAmountLiquidRightValve == NULL)
                                pRollingVarianceAmountLiquidRightValve = new accumulator_set<double, stats<tag::rolling_mean>>(tag::rolling_window::window_size = RollingMeanSize);

                            pRollingMeanAmountLiquidRightValve->operator()(pListData.at(i).m_AmountLiquidRightValve);
                            pRollingVarianceAmountLiquidRightValve->operator()(pListData.at(i).m_StandardDeviationAmountLiquidRightValve);
                            m_RollingMeanLiquidRightValve = rolling_mean(*pRollingMeanAmountLiquidRightValve);
                            m_RollingStdLiquidRightValve = rolling_mean(*pRollingVarianceAmountLiquidRightValve);

                            if (seriesAmountLiquidRightValve == NULL && (m_TrendGraphFlag & GRAPH_SHOW_DROPLET_RIGHT_VALVE_RAW_POINTS)) seriesAmountLiquidRightValve = new QScatterSeries();
                            if (seriesRollingMeanAmountLiquidRightValve == NULL && (m_TrendGraphFlag & GRAPH_SHOW_DROPLET_RIGHT_VALVE_MEAN))
                                seriesRollingMeanAmountLiquidRightValve = new QLineSeries();
                            if (seriesRollingStdPlusAmountLiquidRightValve == NULL && (m_TrendGraphFlag & GRAPH_SHOW_DROPLET_RIGHT_VALVE_STD))
                                seriesRollingStdPlusAmountLiquidRightValve = new QLineSeries();
                            if (seriesRollingStdMinusAmountLiquidRightValve == NULL && (m_TrendGraphFlag & GRAPH_SHOW_DROPLET_RIGHT_VALVE_STD))
                                seriesRollingStdMinusAmountLiquidRightValve = new QLineSeries();

                            if (seriesAmountLiquidRightValve) seriesAmountLiquidRightValve->append(MSecsSinceEpoch, pListData.at(i).m_AmountLiquidRightValve);
                            if (seriesRollingMeanAmountLiquidRightValve) seriesRollingMeanAmountLiquidRightValve->append(MSecsSinceEpoch, m_RollingMeanLiquidRightValve);
                            if (seriesRollingStdPlusAmountLiquidRightValve)
                                seriesRollingStdPlusAmountLiquidRightValve->append(MSecsSinceEpoch, m_RollingMeanLiquidRightValve + m_RollingStdLiquidRightValve);
                            if (seriesRollingStdMinusAmountLiquidRightValve)
                                seriesRollingStdMinusAmountLiquidRightValve->append(MSecsSinceEpoch, m_RollingMeanLiquidRightValve - m_RollingStdLiquidRightValve);
                        }
                    } else {
                        if (pRollingMeanSumAmountLiquid == NULL)
                            pRollingMeanSumAmountLiquid = new accumulator_set<double, stats<tag::rolling_mean>>(tag::rolling_window::window_size = RollingMeanSize);
                        if (pRollingVarianceSumAmountLiquid == NULL)
                            pRollingVarianceSumAmountLiquid = new accumulator_set<double, stats<tag::rolling_mean>>(tag::rolling_window::window_size = RollingMeanSize);
                        pRollingMeanSumAmountLiquid->operator()(pListData.at(i).m_SumAmountLiquid);
                        pRollingVarianceSumAmountLiquid->operator()(pListData.at(i).m_StandardDeviationSumAmountLiquid);
                        m_RollingMeanSumLiquid = rolling_mean(*pRollingMeanSumAmountLiquid);
                        m_RollingStdSumLiquid = (rolling_mean(*pRollingVarianceSumAmountLiquid));

                        if (seriesSumAmountLiquid == NULL && (m_TrendGraphFlag & GRAPH_SHOW_DROPLET_LEFT_VALVE_RAW_POINTS)) seriesSumAmountLiquid = new QScatterSeries();
                        if (seriesRollingMeanSumAmountLiquid == NULL && (m_TrendGraphFlag & GRAPH_SHOW_DROPLET_LEFT_VALVE_MEAN)) seriesRollingMeanSumAmountLiquid = new QLineSeries();
                        if (seriesRollingStdPlusSumAmountLiquid == NULL && (m_TrendGraphFlag & GRAPH_SHOW_DROPLET_LEFT_VALVE_STD)) seriesRollingStdPlusSumAmountLiquid = new QLineSeries();
                        if (seriesRollingStdMinusSumAmountLiquid == NULL && (m_TrendGraphFlag & GRAPH_SHOW_DROPLET_LEFT_VALVE_STD)) seriesRollingStdMinusSumAmountLiquid = new QLineSeries();
                        if (seriesSumAmountLiquid) seriesSumAmountLiquid->append(MSecsSinceEpoch, pListData.at(i).m_SumAmountLiquid);
                        if (seriesRollingMeanSumAmountLiquid) seriesRollingMeanSumAmountLiquid->append(MSecsSinceEpoch, m_RollingMeanSumLiquid);
                        if (seriesRollingStdPlusSumAmountLiquid) seriesRollingStdPlusSumAmountLiquid->append(MSecsSinceEpoch, m_RollingMeanSumLiquid + m_RollingStdSumLiquid);
                        if (seriesRollingStdMinusSumAmountLiquid) seriesRollingStdMinusSumAmountLiquid->append(MSecsSinceEpoch, m_RollingMeanSumLiquid - m_RollingStdSumLiquid);
                    }
                    if (seriesBottlesPerMinute == NULL && (m_TrendGraphFlag & GRAPH_SHOW_BOTTLES_PER_MIN)) seriesBottlesPerMinute = new QLineSeries();
                    m_BottlesPerMinute = pListData.at(i).m_BottlesPerMinute;
                    if (seriesBottlesPerMinute && m_TrendGraphFlag & GRAPH_SHOW_BOTTLES_PER_MIN) {
                        double MaxBottlesPerMinute = -1;
                        double MinBottlesPerMinute = DBL_MAX;
                        const double kFactorBottlesPerMinuteOffset = 1.5;

                        GetMaxMinBottlesPerMin(pListData, MaxBottlesPerMinute, MinBottlesPerMinute);
                        if (MaxBottlesPerMinute > -1) {
                            MaxBottlesPerMinute = MaxBottlesPerMinute * kFactorBottlesPerMinuteOffset;
                            MinBottlesPerMinute = 0;
                            m_yAxisBottlesPerMinute->setTickInterval((MaxBottlesPerMinute - MinBottlesPerMinute) / 8);
                            m_yAxisBottlesPerMinute->setRange(MinBottlesPerMinute, MaxBottlesPerMinute);
                        }
                        seriesBottlesPerMinute->append(MSecsSinceEpoch, pListData.at(i).m_BottlesPerMinute);
                    }
                    if (pListData.at(i).m_MaschineState == PluginInterface::MachineState::Production) {
                        if (CurrentMaschineState != pListData.at(i).m_MaschineState) {
                            QPair<qint64, double> xyPos;
                            xyPos.first = MSecsSinceEpoch;
                            xyPos.second = 2;
                            if (ListCoordinateMaschineStateChanged.count() == 0) {
                                MaschineStateProductionIsFirst = true;
                            }
                            ListCoordinateMaschineStateChanged.append(xyPos);
                            CurrentMaschineState = PluginInterface::MachineState::Production;
                        }
                    }
                    if (pListData.at(i).m_MaschineState == PluginInterface::MachineState::Setup) {
                        if (CurrentMaschineState != pListData.at(i).m_MaschineState) {
                            QPair<qint64, double> xyPos;
                            xyPos.first = MSecsSinceEpoch;
                            xyPos.second = 2;
                            if (ListCoordinateMaschineStateChanged.count() == 0) {
                                MaschineStateProductionIsFirst = false;
                            }
                            ListCoordinateMaschineStateChanged.append(xyPos);
                            CurrentMaschineState = PluginInterface::MachineState::Setup;
                        }
                    }

                    if (seriesBottleEject == NULL && (m_TrendGraphFlag & GRAPH_SHOW_EJECTED_BOTTLES)) seriesBottleEject = new QScatterSeries();
                    if (seriesBottleEject && (m_TrendGraphFlag & GRAPH_SHOW_EJECTED_BOTTLES)) {
                        if (pListData.at(i).m_EjectBottle && pListData.at(i).m_EjectBottleValid && pListData.at(i).m_MaschineState == PluginInterface::MachineState::Production) {
                            seriesBottleEject->append(MSecsSinceEpoch, 1);
                            m_NumberBottlesEjected = seriesBottleEject->count();
                        }
                    }
                } else {  // software wurde beendet oder keine Produktion  ende einer Serie
                    m_RollingMeanSumLiquid = m_RollingMeanLiquidLeftValve = m_RollingMeanLiquidRightValve = -1.0;
                    m_RollingStdSumLiquid = m_RollingStdLiquidLeftValve = m_RollingStdLiquidRightValve = -1.0;
                    m_BottlesPerMinute = 0.0;
                    AddNewSeriesLiquid(seriesAmountLiquidLeftValve, seriesRollingMeanAmountLiquidLeftValve, seriesRollingStdPlusAmountLiquidLeftValve, seriesRollingStdMinusAmountLiquidLeftValve,
                                       seriesAmountLiquidRightValve, seriesRollingMeanAmountLiquidRightValve, seriesRollingStdPlusAmountLiquidRightValve, seriesRollingStdMinusAmountLiquidRightValve,
                                       seriesSumAmountLiquid, seriesRollingMeanSumAmountLiquid, seriesRollingStdPlusSumAmountLiquid, seriesRollingStdMinusSumAmountLiquid, seriesBottlesPerMinute,
                                       seriesBottleEject);
                    AddSeriesMaschineState(ListCoordinateMaschineStateChanged, MaschineStateProductionIsFirst, MSecsSinceEpoch);
                    SeriesDataIn = true;
                    // Reset Accumulator for new series
                    if (pRollingMeanSumAmountLiquid != NULL) {
                        delete pRollingMeanSumAmountLiquid;
                        pRollingMeanSumAmountLiquid = NULL;
                    }
                    if (pRollingVarianceSumAmountLiquid != NULL) {
                        delete pRollingVarianceSumAmountLiquid;
                        pRollingVarianceSumAmountLiquid = NULL;
                    }
                    if (pRollingMeanAmountLiquidLeftValve != NULL) {
                        delete pRollingMeanAmountLiquidLeftValve;
                        pRollingMeanAmountLiquidLeftValve = NULL;
                    }
                    if (pRollingVarianceAmountLiquidLeftValve != NULL) {
                        delete pRollingVarianceAmountLiquidLeftValve;
                        pRollingVarianceAmountLiquidLeftValve = NULL;
                    }
                    if (pRollingMeanAmountLiquidRightValve != NULL) {
                        delete pRollingMeanAmountLiquidRightValve;
                        pRollingMeanAmountLiquidRightValve = NULL;
                    }
                    if (pRollingVarianceAmountLiquidRightValve != NULL) {
                        delete pRollingVarianceAmountLiquidRightValve;
                        pRollingVarianceAmountLiquidRightValve = NULL;
                    }
                }
            }
        }
        if (MaxLiquid > -1 && !SeriesDataIn) {
            AddNewSeriesLiquid(seriesAmountLiquidLeftValve, seriesRollingMeanAmountLiquidLeftValve, seriesRollingStdPlusAmountLiquidLeftValve, seriesRollingStdMinusAmountLiquidLeftValve,
                               seriesAmountLiquidRightValve, seriesRollingMeanAmountLiquidRightValve, seriesRollingStdPlusAmountLiquidRightValve, seriesRollingStdMinusAmountLiquidRightValve,
                               seriesSumAmountLiquid, seriesRollingMeanSumAmountLiquid, seriesRollingStdPlusSumAmountLiquid, seriesRollingStdMinusSumAmountLiquid, seriesBottlesPerMinute,
                               seriesBottleEject);
            AddSeriesMaschineState(ListCoordinateMaschineStateChanged, MaschineStateProductionIsFirst, MSecsSinceEpoch);
        }
        ListMarkers = m_Chart->legend()->markers();
        for (int i = 0; i < ListMarkers.count(); i++) {  // all markers unsichtbar machen
            ListMarkers.at(i)->setVisible(false);
            if (ListMarkers.at(i)->brush().color() == Qt::darkGreen) CountLiquidFirst = i;
            if (ListMarkers.at(i)->brush().color() == Qt::darkBlue) CountLiquidSecond = i;
            if (ListMarkers.at(i)->brush().color() == Qt::darkMagenta) CountLiquidSum = i;
            if (ListMarkers.at(i)->brush().color() == Qt::darkYellow) CountBottlesPerMin = i;
            if (ListMarkers.at(i)->brush().color() == Qt::darkRed) CountBottleEjected = i;
        }
        // nur die letzte serie/legnde sichtbar machen da dort die aktuellen Messwerte angezeigt werden
        if (CountLiquidFirst != -1) ListMarkers.at(CountLiquidFirst)->setVisible(true);
        if (CountLiquidSecond != -1) ListMarkers.at(CountLiquidSecond)->setVisible(true);
        if (CountLiquidSum != -1) ListMarkers.at(CountLiquidSum)->setVisible(true);
        if (CountBottlesPerMin != -1) ListMarkers.at(CountBottlesPerMin)->setVisible(true);
        if (CountBottleEjected != -1) ListMarkers.at(CountBottleEjected)->setVisible(true);
    }

    if (pRollingMeanSumAmountLiquid != NULL) {
        delete pRollingMeanSumAmountLiquid;
        pRollingMeanSumAmountLiquid = NULL;
    }
    if (pRollingVarianceSumAmountLiquid != NULL) {
        delete pRollingVarianceSumAmountLiquid;
        pRollingVarianceSumAmountLiquid = NULL;
    }
    if (pRollingMeanAmountLiquidLeftValve != NULL) {
        delete pRollingMeanAmountLiquidLeftValve;
        pRollingMeanAmountLiquidLeftValve = NULL;
    }
    if (pRollingVarianceAmountLiquidLeftValve != NULL) {
        delete pRollingVarianceAmountLiquidLeftValve;
        pRollingVarianceAmountLiquidLeftValve = NULL;
    }
    if (pRollingMeanAmountLiquidRightValve != NULL) {
        delete pRollingMeanAmountLiquidRightValve;
        pRollingMeanAmountLiquidRightValve = NULL;
    }
    if (pRollingVarianceAmountLiquidRightValve != NULL) {
        delete pRollingVarianceAmountLiquidRightValve;
        pRollingVarianceAmountLiquidRightValve = NULL;
    }
}

void PlotTrendGraph::GetMaxMinTemperature(QList<TrendTemperatureData>& pListData, double& MaxTemp, double& MinTemp)
{
    for (int i = 0; i < pListData.count(); i++) {
        if (!pListData.at(i).m_SoftwareFinished && !pListData.at(i).m_SoftwareStart) {
            if (m_TrendGraphFlag & GRAPH_SHOW_TEMP_CHAMBER_LEFT) {
                MaxTemp = qMax(pListData.at(i).m_CurrentTemperatureLeftValve, MaxTemp);
                MinTemp = qMin(pListData.at(i).m_CurrentTemperatureLeftValve, MinTemp);
            }
            if (m_TrendGraphFlag & GRAPH_SHOW_TEMP_CHAMBER_RIGHT) {
                MaxTemp = qMax(pListData.at(i).m_CurrentTemperatureRightValve, MaxTemp);
                MinTemp = qMin(pListData.at(i).m_CurrentTemperatureRightValve, MinTemp);
            }
            if (m_TrendGraphFlag & GRAPH_SHOW_TEMP_PIEZO_LEFT) {
                MaxTemp = qMax(pListData.at(i).m_StackTemperatureLeftValve, MaxTemp);
                MinTemp = qMin(pListData.at(i).m_StackTemperatureLeftValve, MinTemp);
            }
            if (m_TrendGraphFlag & GRAPH_SHOW_TEMP_PIEZO_RIGHT) {
                MaxTemp = qMax(pListData.at(i).m_StackTemperatureRightValve, MaxTemp);
                MinTemp = qMin(pListData.at(i).m_StackTemperatureRightValve, MinTemp);
            }
            if (m_TrendGraphFlag & GRAPH_SHOW_TEMP_PREASURE_TANK) {
                MaxTemp = qMax(pListData.at(i).m_CurrentPreasureTankTemperature, MaxTemp);
                MinTemp = qMin(pListData.at(i).m_CurrentPreasureTankTemperature, MinTemp);
            }
            if (m_TrendGraphFlag & GRAPH_SHOW_HEATING_PIPE) {
                MaxTemp = qMax(pListData.at(i).m_CurrentHeatingPipeTemperature, MaxTemp);
                MinTemp = qMin(pListData.at(i).m_CurrentHeatingPipeTemperature, MinTemp);
            }
            if (m_TrendGraphFlag & GRAPH_SHOW_WATER_COOLING) {
                MaxTemp = qMax(pListData.at(i).m_CurrentWaterCoolingTemperature, MaxTemp);
                MinTemp = qMin(pListData.at(i).m_CurrentWaterCoolingTemperature, MinTemp);
            }
        }
    }
}

void PlotTrendGraph::GetMaxMinBottlesPerMin(QList<TrendLiquidData>& pListData, double& resultMaxValue, double& resultMinValue)
{
    for (int i = 0; i < pListData.count(); i++) {
        if (pListData.at(i).m_MaschineState == PluginInterface::MachineState::Production) {
            if (!pListData.at(i).m_SoftwareFinished && !pListData.at(i).m_SoftwareStart) {
                resultMaxValue = qMax(pListData.at(i).m_BottlesPerMinute, resultMaxValue);
                resultMinValue = qMin(pListData.at(i).m_BottlesPerMinute, resultMinValue);
            }
        }
    }
}

void PlotTrendGraph::GetMaxMinLiquid(QList<TrendLiquidData>& pListData, double& MaxLiquid, double& MinLiquid, bool WorkWithTwoValves)
{
    double MaxValue, MinValue;
    for (int i = 0; i < pListData.count(); i++) {
        if (pListData.at(i).m_MaschineState == PluginInterface::MachineState::Production) {
            if (!pListData.at(i).m_SoftwareFinished && !pListData.at(i).m_SoftwareStart) {
                if (WorkWithTwoValves) {
                    if (m_TrendGraphFlag & GRAPH_SHOW_DROPLET_SUM_MEAN) {
                        MaxValue = pListData.at(i).m_SumAmountLiquid + pListData.at(i).m_StandardDeviationSumAmountLiquid;
                        MinValue = pListData.at(i).m_SumAmountLiquid - pListData.at(i).m_StandardDeviationSumAmountLiquid;

                        MaxLiquid = qMax(MaxValue, qMax(MaxValue, MaxLiquid));
                        MinLiquid = qMin(MinValue, qMin(MinValue, MinLiquid));
                    }
                    if (m_TrendGraphFlag & GRAPH_SHOW_DROPLET_LEFT_VALVE_MEAN) {
                        MaxValue = pListData.at(i).m_AmountLiquidLeftValve + pListData.at(i).m_StandardDeviationAmountLiquidLeftValve;
                        MinValue = pListData.at(i).m_AmountLiquidLeftValve - pListData.at(i).m_StandardDeviationAmountLiquidLeftValve;

                        MaxLiquid = qMax(MaxValue, qMax(MaxValue, MaxLiquid));
                        MinLiquid = qMin(MinValue, qMin(MinValue, MinLiquid));
                    }
                    if (m_TrendGraphFlag & GRAPH_SHOW_DROPLET_RIGHT_VALVE_MEAN) {
                        MaxValue = pListData.at(i).m_AmountLiquidRightValve + pListData.at(i).m_StandardDeviationAmountLiquidRightValve;
                        MinValue = pListData.at(i).m_AmountLiquidRightValve - pListData.at(i).m_StandardDeviationAmountLiquidRightValve;

                        MaxLiquid = qMax(MaxValue, qMax(MaxValue, MaxLiquid));
                        MinLiquid = qMin(MinValue, qMin(MinValue, MinLiquid));
                    }
                } else {
                    if (m_TrendGraphFlag & GRAPH_SHOW_DROPLET_SUM_MEAN) {
                        MaxValue = pListData.at(i).m_SumAmountLiquid + pListData.at(i).m_StandardDeviationSumAmountLiquid;
                        MinValue = pListData.at(i).m_SumAmountLiquid - pListData.at(i).m_StandardDeviationSumAmountLiquid;

                        MaxLiquid = qMax(MaxValue, qMax(MaxValue, MaxLiquid));
                        MinLiquid = qMin(MinValue, qMin(MinValue, MinLiquid));
                    }
                }
            }
        }
    }
}

void PlotTrendGraph::AddSeriesMaschineState(QList<QPair<qint64, double>>& ListCoordinateMaschineStaeChanged, bool MaschineStateProductionIsFirst, qint64 MSecsSinceEpoch)
{
    QPair<qint64, double> xyset;
    xyset.first = MSecsSinceEpoch;
    xyset.second = 2.0;
    ListCoordinateMaschineStaeChanged.append(xyset);
    QPair<qint64, double> linePointFirst, linePointSecond;
    bool StartState = MaschineStateProductionIsFirst;
    QList<QPair<qint64, double>> ListProduction;
    QList<QPair<qint64, double>> ListSetup;

    if (ListCoordinateMaschineStaeChanged.count() > 1) {
        do {
            if (StartState) {
                linePointFirst = ListCoordinateMaschineStaeChanged.takeFirst();
                linePointSecond = ListCoordinateMaschineStaeChanged[0];
                ListProduction.append(linePointFirst);
                ListProduction.append(linePointSecond);
                StartState = !StartState;
            } else {
                linePointFirst = ListCoordinateMaschineStaeChanged.takeFirst();
                linePointSecond = ListCoordinateMaschineStaeChanged[0];
                ListSetup.append(linePointFirst);
                ListSetup.append(linePointSecond);
                StartState = !StartState;
            }
        } while (ListCoordinateMaschineStaeChanged.count() > 1);

        while (ListProduction.count() > 1) {
            QLineSeries* seriesMaschineState = new QLineSeries();
            linePointFirst = ListProduction.takeFirst();
            linePointSecond = ListProduction.takeFirst();
            seriesMaschineState->append(linePointFirst.first, linePointFirst.second);
            seriesMaschineState->append(linePointSecond.first, linePointSecond.second);
            QPen penMaschineState(QColor(1, 1, 1));
            penMaschineState.setWidth(2);
            seriesMaschineState->setPen(penMaschineState);
            m_Chart->addSeries(seriesMaschineState);
            seriesMaschineState->attachAxis(m_xAxis);
            seriesMaschineState->attachAxis(m_yAxisBottleEjected);
        }
        while (ListSetup.count() > 1) {
            QLineSeries* seriesSetupState = new QLineSeries();
            linePointFirst = ListSetup.takeFirst();
            linePointSecond = ListSetup.takeFirst();
            seriesSetupState->append(linePointFirst.first, linePointFirst.second);
            seriesSetupState->append(linePointSecond.first, linePointSecond.second);
            QPen penSetupState(Qt::darkGray);
            penSetupState.setWidth(2);
            seriesSetupState->setPen(penSetupState);
            m_Chart->addSeries(seriesSetupState);
            seriesSetupState->attachAxis(m_xAxis);
            seriesSetupState->attachAxis(m_yAxisBottleEjected);
        }
    }
}

void PlotTrendGraph::AddNewSeriesLiquid(QScatterSeries*& seriesAmountLiquidLeftValve, QLineSeries*& seriesRollingMeanAmountLiquidLeftValve, QLineSeries*& seriesRollingStdPlusAmountLiquidLeftValve,
                                        QLineSeries*& seriesRollingStdMinusAmountLiquidLeftValve, QScatterSeries*& seriesAmountLiquidRightValve, QLineSeries*& seriesRollingMeanAmountLiquidRightValve,
                                        QLineSeries*& seriesRollingStdPlusAmountLiquidRightValve, QLineSeries*& seriesRollingStdMinusAmountLiquidRightValve, QScatterSeries*& seriesSumAmountLiquid,
                                        QLineSeries*& seriesRollingMeanSumAmountLiquid, QLineSeries*& seriesRollingStdPlusSumAmountLiquid, QLineSeries*& seriesRollingStdMinusSumAmountLiquid,
                                        QLineSeries*& seriesBottlesPerminute, QScatterSeries*& seriesBottleEjected)
{
    int MarkerSize = 4;
    QString LeftValveName = tr("Left Valve");
    QString RightValveName = tr("Right Valve");

    // data first valve
    if (seriesAmountLiquidLeftValve) {
        seriesAmountLiquidLeftValve->setName(LeftValveName);
        seriesAmountLiquidLeftValve->setColor(Qt::darkGreen);
        m_Chart->addSeries(seriesAmountLiquidLeftValve);
        seriesAmountLiquidLeftValve->attachAxis(m_xAxis);
        seriesAmountLiquidLeftValve->attachAxis(m_yAxis);
        seriesAmountLiquidLeftValve->setMarkerSize(MarkerSize);
        seriesAmountLiquidLeftValve->setPen(QColor(Qt::transparent));
        seriesAmountLiquidLeftValve = NULL;
    }
    if (seriesRollingStdPlusAmountLiquidLeftValve) {
        seriesRollingStdPlusAmountLiquidLeftValve->setColor(Qt::green);
        m_Chart->addSeries(seriesRollingStdPlusAmountLiquidLeftValve);
        seriesRollingStdPlusAmountLiquidLeftValve->attachAxis(m_xAxis);
        seriesRollingStdPlusAmountLiquidLeftValve->attachAxis(m_yAxis);
        seriesRollingStdPlusAmountLiquidLeftValve = NULL;
    }
    if (seriesRollingStdMinusAmountLiquidLeftValve) {
        seriesRollingStdMinusAmountLiquidLeftValve->setColor(Qt::green);
        m_Chart->addSeries(seriesRollingStdMinusAmountLiquidLeftValve);
        seriesRollingStdMinusAmountLiquidLeftValve->attachAxis(m_xAxis);
        seriesRollingStdMinusAmountLiquidLeftValve->attachAxis(m_yAxis);
        seriesRollingStdMinusAmountLiquidLeftValve = NULL;
    }
    if (seriesRollingMeanAmountLiquidLeftValve) {
        if (m_RollingMeanLiquidLeftValve != -1 && m_RollingStdLiquidLeftValve != -1)
            seriesRollingMeanAmountLiquidLeftValve->setName(tr("%1(Mean:%2(+/-%3))").arg(LeftValveName).arg(m_RollingMeanLiquidLeftValve, 0, 'f', 0).arg(m_RollingStdLiquidLeftValve, 0, 'f', 1));
        else {
            if (m_RollingMeanLiquidLeftValve != -1)
                seriesRollingMeanAmountLiquidLeftValve->setName(tr("%1(Mean:%2)").arg(LeftValveName).arg(m_RollingMeanLiquidLeftValve, 0, 'f', 0));
            else
                seriesRollingMeanAmountLiquidLeftValve->setName(LeftValveName);
        }
        seriesRollingMeanAmountLiquidLeftValve->setColor(Qt::darkGreen);
        m_Chart->addSeries(seriesRollingMeanAmountLiquidLeftValve);
        seriesRollingMeanAmountLiquidLeftValve->attachAxis(m_xAxis);
        seriesRollingMeanAmountLiquidLeftValve->attachAxis(m_yAxis);
        seriesRollingMeanAmountLiquidLeftValve = NULL;
    }
    // data second Valve
    if (seriesAmountLiquidRightValve) {
        seriesAmountLiquidRightValve->setName(RightValveName);
        seriesAmountLiquidRightValve->setColor(Qt::darkBlue);
        m_Chart->addSeries(seriesAmountLiquidRightValve);
        seriesAmountLiquidRightValve->attachAxis(m_xAxis);
        seriesAmountLiquidRightValve->attachAxis(m_yAxis);
        seriesAmountLiquidRightValve->setMarkerSize(MarkerSize);
        seriesAmountLiquidRightValve->setPen(QColor(Qt::transparent));
        seriesAmountLiquidRightValve = NULL;
    }
    if (seriesRollingStdPlusAmountLiquidRightValve) {
        seriesRollingStdPlusAmountLiquidRightValve->setColor(Qt::blue);
        m_Chart->addSeries(seriesRollingStdPlusAmountLiquidRightValve);
        seriesRollingStdPlusAmountLiquidRightValve->attachAxis(m_xAxis);
        seriesRollingStdPlusAmountLiquidRightValve->attachAxis(m_yAxis);
        seriesRollingStdPlusAmountLiquidRightValve = NULL;
    }
    if (seriesRollingStdMinusAmountLiquidRightValve) {
        seriesRollingStdMinusAmountLiquidRightValve->setColor(Qt::blue);
        m_Chart->addSeries(seriesRollingStdMinusAmountLiquidRightValve);
        seriesRollingStdMinusAmountLiquidRightValve->attachAxis(m_xAxis);
        seriesRollingStdMinusAmountLiquidRightValve->attachAxis(m_yAxis);
        seriesRollingStdMinusAmountLiquidRightValve = NULL;
    }
    if (seriesRollingMeanAmountLiquidRightValve) {
        if (m_RollingMeanLiquidRightValve != -1 && m_RollingStdLiquidRightValve != -1)
            seriesRollingMeanAmountLiquidRightValve->setName(tr("%1(Mean:%2(+/-%3))").arg(RightValveName).arg(m_RollingMeanLiquidRightValve, 0, 'f', 0).arg(m_RollingStdLiquidRightValve, 0, 'f', 1));
        else {
            if (m_RollingMeanLiquidRightValve != -1)
                seriesRollingMeanAmountLiquidRightValve->setName(tr("%1(Mean:%2)").arg(RightValveName).arg(m_RollingMeanLiquidRightValve, 0, 'f', 0));
            else
                seriesRollingMeanAmountLiquidRightValve->setName(RightValveName);
        }
        seriesRollingMeanAmountLiquidRightValve->setColor(Qt::darkBlue);
        m_Chart->addSeries(seriesRollingMeanAmountLiquidRightValve);
        seriesRollingMeanAmountLiquidRightValve->attachAxis(m_xAxis);
        seriesRollingMeanAmountLiquidRightValve->attachAxis(m_yAxis);
        seriesRollingMeanAmountLiquidRightValve = NULL;
    }
    // data total
    if (seriesSumAmountLiquid) {
        seriesSumAmountLiquid->setName(tr("Total"));
        seriesSumAmountLiquid->setColor(Qt::darkMagenta);
        m_Chart->addSeries(seriesSumAmountLiquid);
        seriesSumAmountLiquid->attachAxis(m_xAxis);
        seriesSumAmountLiquid->attachAxis(m_yAxis);
        seriesSumAmountLiquid->setMarkerSize(MarkerSize);
        seriesSumAmountLiquid->setPen(QColor(Qt::transparent));
        seriesSumAmountLiquid = NULL;
    }
    if (seriesRollingStdPlusSumAmountLiquid) {
        seriesRollingStdPlusSumAmountLiquid->setColor(Qt::magenta);
        m_Chart->addSeries(seriesRollingStdPlusSumAmountLiquid);
        seriesRollingStdPlusSumAmountLiquid->attachAxis(m_xAxis);
        seriesRollingStdPlusSumAmountLiquid->attachAxis(m_yAxis);
        seriesRollingStdPlusSumAmountLiquid = NULL;
    }
    if (seriesRollingStdMinusSumAmountLiquid) {
        seriesRollingStdMinusSumAmountLiquid->setColor(Qt::magenta);
        m_Chart->addSeries(seriesRollingStdMinusSumAmountLiquid);
        seriesRollingStdMinusSumAmountLiquid->attachAxis(m_xAxis);
        seriesRollingStdMinusSumAmountLiquid->attachAxis(m_yAxis);
        seriesRollingStdMinusSumAmountLiquid = NULL;
    }
    if (seriesRollingMeanSumAmountLiquid) {
        if (m_RollingMeanSumLiquid != -1 && m_RollingStdSumLiquid != -1)
            seriesRollingMeanSumAmountLiquid->setName(tr("Total(Mean:%1(+/-%2))").arg(m_RollingMeanSumLiquid, 0, 'f', 0).arg(m_RollingStdSumLiquid, 0, 'f', 1));
        else {
            if (m_RollingMeanSumLiquid != -1)
                seriesRollingMeanSumAmountLiquid->setName(tr("Total(Mean:%1)").arg(m_RollingMeanSumLiquid, 0, 'f', 0));
            else
                seriesRollingMeanSumAmountLiquid->setName(tr("Total Mean"));
        }
        seriesRollingMeanSumAmountLiquid->setColor(Qt::darkMagenta);
        m_Chart->addSeries(seriesRollingMeanSumAmountLiquid);
        seriesRollingMeanSumAmountLiquid->attachAxis(m_xAxis);
        seriesRollingMeanSumAmountLiquid->attachAxis(m_yAxis);
        seriesRollingMeanSumAmountLiquid = NULL;
    }
    // data num bottles/min
    if (seriesBottlesPerminute) {
        seriesBottlesPerminute->setName(tr("%1 Bottles/min").arg(static_cast<int>(m_BottlesPerMinute + 0.5)));
        seriesBottlesPerminute->setColor(Qt::darkYellow);
        m_Chart->addSeries(seriesBottlesPerminute);
        seriesBottlesPerminute->attachAxis(m_xAxis);
        seriesBottlesPerminute->attachAxis(m_yAxisBottlesPerMinute);
        // seriesBottlesPerminute->setPen(QColor(Qt::transparent));
        seriesBottlesPerminute = NULL;
    }

    if (seriesBottleEjected) {
        seriesBottleEjected->setName(tr("Ejected(%1)").arg(m_NumberBottlesEjected));
        seriesBottleEjected->setColor(Qt::darkRed);
        m_Chart->addSeries(seriesBottleEjected);
        seriesBottleEjected->attachAxis(m_xAxis);
        seriesBottleEjected->attachAxis(m_yAxisBottleEjected);
        seriesBottleEjected->setMarkerSize(MarkerSize * 1.5);
        // seriesBottleEjected->setPen(QColor(Qt::transparent));
        seriesBottleEjected = NULL;
    }
}

void PlotTrendGraph::DrawTrendDataTemperature(QList<TrendTemperatureData>& pListData, QDateTime CurrentDateTime, bool fromFile, int MinIndex, int MaxIndex)
{
    QLineSeries* seriesStackTemperatureLeftValve = NULL;
    QLineSeries* seriesCurrentTemperatureLeftValve = NULL;
    QLineSeries* seriesStackTemperatureRightValve = NULL;
    QLineSeries* seriesCurrentTemperatureRightValve = NULL;
    QLineSeries* seriesPreasuretankTemperature = NULL;
    QLineSeries* seriesHeatingPipeTemperature = NULL;
    QLineSeries* seriesWaterCooling = NULL;
    qint64 MSecsSinceEpoch = 0;
    double MaxTemp = -1;
    double MinTemp = DBL_MAX;
    double Factor = 1.0;
    QDateTime MinDateTime, MaxDateTime;
    QList<QLegendMarker*> ListMarkers;
    int CountStackFirst = -1;
    int CountCurrentFirst = -1;
    int CountStackSecond = -1;
    int CountCurrentSecond = -1;
    int CountPreasureTank = -1;
    int CountHeatingPipe = -1;
    int CountWaterCooling = -1;
    const int kRangeTemperaturOffset = 10;
    int NumPoints = 0;
    int OffsetIndex = 1;
    bool SeriesDataIn = true;
    QRectF PlotArea = m_Chart->plotArea();
    double MaxPointsInView = PlotArea.width();

    if (m_TrendGraphWidget) m_TrendGraphFlag = m_TrendGraphWidget->GetTrendGraphFlag();
    if (MaxIndex == -1) MaxIndex = pListData.count() - 1;
    m_Chart->removeAllSeries();
    if (pListData.count() > 1) {
        MinDateTime = MaxDateTime = CurrentDateTime;
        MinDateTime.setTime(pListData.at(MinIndex).m_Time);
        MaxDateTime.setTime(pListData.at(MaxIndex).m_Time);
        m_xAxis->setRange(MinDateTime, MaxDateTime);
        GetMaxMinTemperature(pListData, MaxTemp, MinTemp);

        if (MaxTemp > -1) {
            MaxTemp = MaxTemp + kRangeTemperaturOffset;
            MinTemp = MinTemp - kRangeTemperaturOffset;
            if (MinTemp < 0) MinTemp = 0;
            m_yAxis->setTickInterval((MaxTemp - MinTemp) / 8);
            m_yAxis->setRange(MinTemp - 5, MaxTemp + 5);
        }
        if (!fromFile) {
            NumPoints = MaxIndex - MinIndex;
            if (MaxPointsInView > 0) {
                OffsetIndex = static_cast<int>((double)NumPoints / MaxPointsInView);
                OffsetIndex++;
            }
        }
        m_StackTemperatureLeftValve = m_CurrentTemperatureLeftValve = m_StackTemperatureRightValve = m_CurrentTemperatureRightValve = m_CurrentPreasureTankTemperature =
            m_CurrentHeatingPipeTemperature = m_CurrentWaterCoolingTemperature = -1.0;
        for (int i = 0; i < pListData.count(); i = i + OffsetIndex) {
            CurrentDateTime.setTime(pListData.at(i).m_Time);
            MSecsSinceEpoch = CurrentDateTime.toMSecsSinceEpoch();
            if (!pListData.at(i).m_SoftwareStart) {
                if (!pListData.at(i).m_SoftwareFinished) {
                    SeriesDataIn = false;
                    if (m_TrendGraphFlag & GRAPH_SHOW_TEMP_PIEZO_LEFT) {
                        if (seriesStackTemperatureLeftValve == NULL) seriesStackTemperatureLeftValve = new QLineSeries();
                        m_StackTemperatureLeftValve = pListData.at(i).m_StackTemperatureLeftValve;
                        seriesStackTemperatureLeftValve->append(MSecsSinceEpoch, m_StackTemperatureLeftValve);
                    }
                    if (m_TrendGraphFlag & GRAPH_SHOW_TEMP_CHAMBER_LEFT) {
                        if (seriesCurrentTemperatureLeftValve == NULL) seriesCurrentTemperatureLeftValve = new QLineSeries();
                        m_CurrentTemperatureLeftValve = pListData.at(i).m_CurrentTemperatureLeftValve;
                        seriesCurrentTemperatureLeftValve->append(MSecsSinceEpoch, m_CurrentTemperatureLeftValve);
                    }
                    if (m_TrendGraphFlag & GRAPH_SHOW_TEMP_PIEZO_RIGHT) {
                        if (seriesStackTemperatureRightValve == NULL) seriesStackTemperatureRightValve = new QLineSeries();
                        m_StackTemperatureRightValve = pListData.at(i).m_StackTemperatureRightValve;
                        seriesStackTemperatureRightValve->append(MSecsSinceEpoch, m_StackTemperatureRightValve);
                    }
                    if (m_TrendGraphFlag & GRAPH_SHOW_TEMP_CHAMBER_RIGHT) {
                        if (seriesCurrentTemperatureRightValve == NULL) seriesCurrentTemperatureRightValve = new QLineSeries();
                        m_CurrentTemperatureRightValve = pListData.at(i).m_CurrentTemperatureRightValve;
                        seriesCurrentTemperatureRightValve->append(MSecsSinceEpoch, pListData.at(i).m_CurrentTemperatureRightValve);
                    }
                    if (m_TrendGraphFlag & GRAPH_SHOW_TEMP_PREASURE_TANK) {
                        if (seriesPreasuretankTemperature == NULL) seriesPreasuretankTemperature = new QLineSeries();
                        m_CurrentPreasureTankTemperature = pListData.at(i).m_CurrentPreasureTankTemperature;
                        seriesPreasuretankTemperature->append(MSecsSinceEpoch, m_CurrentPreasureTankTemperature);
                    }
                    if (m_TrendGraphFlag & GRAPH_SHOW_HEATING_PIPE) {
                        if (seriesHeatingPipeTemperature == NULL) seriesHeatingPipeTemperature = new QLineSeries();
                        m_CurrentHeatingPipeTemperature = pListData.at(i).m_CurrentHeatingPipeTemperature;
                        seriesHeatingPipeTemperature->append(MSecsSinceEpoch, m_CurrentHeatingPipeTemperature);
                    }
                    if (m_TrendGraphFlag & GRAPH_SHOW_WATER_COOLING) {
                        if (seriesWaterCooling == NULL) seriesWaterCooling = new QLineSeries();
                        m_CurrentWaterCoolingTemperature = pListData.at(i).m_CurrentWaterCoolingTemperature;
                        seriesWaterCooling->append(MSecsSinceEpoch, m_CurrentWaterCoolingTemperature);
                    }
                } else {  // dieser Zweig wird nur erreicht wenn Daten aus der Datei gelesen
                    AddNewSeriesTemperature(seriesStackTemperatureLeftValve, seriesCurrentTemperatureLeftValve, seriesStackTemperatureRightValve, seriesCurrentTemperatureRightValve,
                                            seriesPreasuretankTemperature, seriesHeatingPipeTemperature, seriesWaterCooling);
                    SeriesDataIn = true;
                }
            }
        }
        if (MaxTemp > -1 && !SeriesDataIn)
            AddNewSeriesTemperature(seriesStackTemperatureLeftValve, seriesCurrentTemperatureLeftValve, seriesStackTemperatureRightValve, seriesCurrentTemperatureRightValve,
                                    seriesPreasuretankTemperature, seriesHeatingPipeTemperature, seriesWaterCooling);
        ListMarkers = m_Chart->legend()->markers();

        for (int i = 0; i < ListMarkers.count(); i++) {  // all markers unsichtbar machen
            ListMarkers.at(i)->setVisible(false);
            if (ListMarkers.at(i)->brush().color() == Qt::darkGreen) CountStackFirst = i;
            if (ListMarkers.at(i)->brush().color() == Qt::green) CountCurrentFirst = i;
            if (ListMarkers.at(i)->brush().color() == Qt::darkBlue) CountStackSecond = i;
            if (ListMarkers.at(i)->brush().color() == Qt::blue) CountCurrentSecond = i;
            if (ListMarkers.at(i)->brush().color() == Qt::darkYellow) CountPreasureTank = i;
            if (ListMarkers.at(i)->brush().color() == Qt::darkCyan) CountHeatingPipe = i;
            if (ListMarkers.at(i)->brush().color() == Qt::darkMagenta) CountWaterCooling = i;
        }
        if (CountStackFirst != -1) ListMarkers.at(CountStackFirst)->setVisible(true);
        if (CountCurrentFirst != -1) ListMarkers.at(CountCurrentFirst)->setVisible(true);
        if (CountStackSecond != -1) ListMarkers.at(CountStackSecond)->setVisible(true);
        if (CountCurrentSecond != -1) ListMarkers.at(CountCurrentSecond)->setVisible(true);
        if (CountPreasureTank != -1) ListMarkers.at(CountPreasureTank)->setVisible(true);
        if (CountHeatingPipe != -1) ListMarkers.at(CountHeatingPipe)->setVisible(true);
        if (CountWaterCooling != -1) ListMarkers.at(CountWaterCooling)->setVisible(true);
    }
}

void PlotTrendGraph::AddNewSeriesTemperature(QLineSeries*& seriesStackTemperatureLeftValve, QLineSeries*& seriesCurrentTemperatureLeftValve, QLineSeries*& seriesStackTemperatureRightValve,
                                             QLineSeries*& seriesCurrentTemperatureRightValve, QLineSeries*& seriesPreasuretankTemperature, QLineSeries*& seriesHeatingPipeTemperature,
                                             QLineSeries*& seriesWaterCooling)
{
    QString ValveName = tr("Left Valve");
    if (seriesStackTemperatureLeftValve) {
        if (m_StackTemperatureLeftValve != -1.0) {
            seriesStackTemperatureLeftValve->setName(tr("Piezo %1(%2 C)").arg(ValveName).arg(m_StackTemperatureLeftValve, 0, 'f', 1));
        }
        seriesStackTemperatureLeftValve->setColor(Qt::darkGreen);
        m_Chart->addSeries(seriesStackTemperatureLeftValve);
        seriesStackTemperatureLeftValve->attachAxis(m_xAxis);
        seriesStackTemperatureLeftValve->attachAxis(m_yAxis);
        seriesStackTemperatureLeftValve = NULL;
    }
    if (seriesCurrentTemperatureLeftValve) {
        if (m_CurrentTemperatureLeftValve != -1.0) {
            seriesCurrentTemperatureLeftValve->setName(tr("Chamber %1(%2 C)").arg(ValveName).arg(m_CurrentTemperatureLeftValve, 0, 'f', 1));
        }
        seriesCurrentTemperatureLeftValve->setColor(Qt::green);
        m_Chart->addSeries(seriesCurrentTemperatureLeftValve);
        seriesCurrentTemperatureLeftValve->attachAxis(m_xAxis);
        seriesCurrentTemperatureLeftValve->attachAxis(m_yAxis);
        seriesCurrentTemperatureLeftValve = NULL;
    }
    ValveName = tr("Right Valve");
    if (seriesStackTemperatureRightValve) {
        if (m_StackTemperatureRightValve != -1.0) {
            seriesStackTemperatureRightValve->setName(tr("Piezo %1(%2 C)").arg(ValveName).arg(m_StackTemperatureRightValve, 0, 'f', 1));
        }
        seriesStackTemperatureRightValve->setColor(Qt::darkBlue);
        m_Chart->addSeries(seriesStackTemperatureRightValve);
        seriesStackTemperatureRightValve->attachAxis(m_xAxis);
        seriesStackTemperatureRightValve->attachAxis(m_yAxis);
        seriesStackTemperatureRightValve = NULL;
    }
    if (seriesCurrentTemperatureRightValve) {
        if (m_CurrentTemperatureRightValve != -1.0) {
            seriesCurrentTemperatureRightValve->setName(tr("Chamber %1(%2 C)").arg(ValveName).arg(m_CurrentTemperatureRightValve, 0, 'f', 1));
        }
        seriesCurrentTemperatureRightValve->setColor(Qt::blue);
        m_Chart->addSeries(seriesCurrentTemperatureRightValve);
        seriesCurrentTemperatureRightValve->attachAxis(m_xAxis);
        seriesCurrentTemperatureRightValve->attachAxis(m_yAxis);
        seriesCurrentTemperatureRightValve = NULL;
    }
    if (seriesPreasuretankTemperature) {
        if (m_CurrentPreasureTankTemperature != -1.0) {
            seriesPreasuretankTemperature->setName(tr("Temperature Tank(%1 C)").arg(m_CurrentPreasureTankTemperature, 0, 'f', 1));
        }
        seriesPreasuretankTemperature->setColor(Qt::darkYellow);
        m_Chart->addSeries(seriesPreasuretankTemperature);
        seriesPreasuretankTemperature->attachAxis(m_xAxis);
        seriesPreasuretankTemperature->attachAxis(m_yAxis);
        seriesPreasuretankTemperature = NULL;
    }
    if (seriesHeatingPipeTemperature) {
        if (m_CurrentHeatingPipeTemperature != -1.0) {
            seriesHeatingPipeTemperature->setName(tr("Heating Pipe(%1 C)").arg(m_CurrentHeatingPipeTemperature, 0, 'f', 1));
        }
        seriesHeatingPipeTemperature->setColor(Qt::darkCyan);
        m_Chart->addSeries(seriesHeatingPipeTemperature);
        seriesHeatingPipeTemperature->attachAxis(m_xAxis);
        seriesHeatingPipeTemperature->attachAxis(m_yAxis);
        seriesHeatingPipeTemperature = NULL;
    }
    if (seriesWaterCooling) {
        if (m_CurrentWaterCoolingTemperature != -1.0) {
            seriesWaterCooling->setName(tr("Water Cooling(%1 C)").arg(m_CurrentWaterCoolingTemperature, 0, 'f', 1));
        }
        seriesWaterCooling->setColor(Qt::darkMagenta);
        m_Chart->addSeries(seriesWaterCooling);
        seriesWaterCooling->attachAxis(m_xAxis);
        seriesWaterCooling->attachAxis(m_yAxis);
        seriesWaterCooling = NULL;
    }
}
