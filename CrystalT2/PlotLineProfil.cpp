#include "PlotLineProfil.h"
#include "MainAppCrystalT2.h"
#include "Settingsdata.h"
#include "colors.h"

PlotLineProfil::PlotLineProfil(MainAppCrystalT2* pMainAppCrystalT2, QWidget* FrameParent)
    : QChartView(FrameParent), m_Chart(NULL), m_xAxis(NULL), m_yAxis(NULL), m_GrapicSence(NULL), m_MainAppCrystalT2(NULL)
{
    m_MainAppCrystalT2 = pMainAppCrystalT2;
    resize(FrameParent->width(), FrameParent->height());
    m_Chart = new QChart();
    m_Chart->setBackgroundVisible(false);
    m_Chart->setMargins(QMargins(0, 0, 0, 0));
    m_Chart->legend()->setAlignment(Qt::AlignBottom);
    m_Chart->legend()->setVisible(false);

    m_xAxis = new QValueAxis();
    m_xAxis->setRange(0, 200);
    m_xAxis->setTickInterval(50);
    m_xAxis->setTickType(QValueAxis::TicksDynamic);
    m_xAxis->setLabelFormat(" ");
    m_xAxis->setLabelsColor(HMIColor::Grey);
    m_xAxis->setGridLineColor(HMIColor::LightGrey);
    m_xAxis->setTitleBrush(QBrush(HMIColor::Grey));
    m_Chart->addAxis(m_xAxis, Qt::AlignBottom);

    m_yAxis = new QValueAxis();
    m_yAxis->setRange(0, 100);
    m_yAxis->setTickInterval(20);
    m_yAxis->setTickType(QValueAxis::TicksDynamic);
    m_yAxis->setLabelFormat("%d%");
    m_yAxis->setLabelsColor(HMIColor::Grey);
    m_yAxis->setGridLineColor(HMIColor::LightGrey);
    m_yAxis->setTitleBrush(QBrush(HMIColor::Grey));
    m_Chart->addAxis(m_yAxis, Qt::AlignLeft);

    setChart(m_Chart);
    m_GrapicSence = scene();
}

PlotLineProfil::~PlotLineProfil()
{
}

void PlotLineProfil::SetLineDataGradient(const double* Array, int size, double EdgePosLeftSide, double EdgePosRightSide)
{
    QList<QPointF> GradientLinePonts, ThresholdLine;
    double MaxXValue = size;
    double AcceptanceThreshold = 20.0;
    double YValue = 0.0;
    double LeftRightRim = 10;
    int Start = 0;
    int End = size;
    int x = 0;
    bool ViewVarianteShowOnlyIfProductFound = true;
    QLineSeries* seriesGradientLinePonts = new QLineSeries();
    QLineSeries* seriesThresholdLine = new QLineSeries();

    m_Chart->removeAllSeries();
    if (!ViewVarianteShowOnlyIfProductFound) {                    // hier werden immer daten angezeigt unabhängig ob das Produkt gefunden worden ist oder nicht
        if (EdgePosLeftSide != 0.0 || EdgePosRightSide != 0.0) {  // beide Kanten gefunden
            Start = static_cast<int>(EdgePosLeftSide - LeftRightRim);
            End = static_cast<int>(EdgePosRightSide + LeftRightRim);
            if (End > size) End = size;
            if (Start < 0) Start = 0;
        }
        for (int i = Start; i < End; i++) {
            YValue = Array[i] * THRESHOLD_FACTOR_IN_PERCENT_SOBEL_KERNEL_SIZE_3;
            if (YValue > 100.0) YValue = 100.0;
            GradientLinePonts.push_back(QPointF(x, YValue));
            x++;
        }
    } else {                                                      // Kurve wird nur dann angezeigt wenn Produkt gefunden ansonsten werden keine daten angezeigt
        if (EdgePosLeftSide != 0.0 || EdgePosRightSide != 0.0) {  // beide Kanten gefunden
            Start = static_cast<int>(EdgePosLeftSide - LeftRightRim);
            End = static_cast<int>(EdgePosRightSide + LeftRightRim);
            if (End > size) End = size;
            if (Start < 0) Start = 0;

            for (int i = Start; i < End; i++) {
                YValue = Array[i] * THRESHOLD_FACTOR_IN_PERCENT_SOBEL_KERNEL_SIZE_3;
                if (YValue > 100.0) YValue = 100.0;
                GradientLinePonts.push_back(QPointF(x, YValue));
                x++;
            }
        } else {
            for (int i = Start; i < End; i++) {
                GradientLinePonts.push_back(QPointF(x, YValue));
                x++;
            }
        }
    }

    if (GradientLinePonts.count() > 0) {
        MaxXValue = GradientLinePonts.count() + 1;
        seriesGradientLinePonts->append(GradientLinePonts);
        seriesGradientLinePonts->setColor(Qt::darkGreen);
        m_Chart->addSeries(seriesGradientLinePonts);
        seriesGradientLinePonts->attachAxis(m_xAxis);
        seriesGradientLinePonts->attachAxis(m_yAxis);
    }

    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) AcceptanceThreshold = GetMainAppCrystalT2()->GetSettingsData()->m_EdgeAcceptanceThresholdInPercent;
    ThresholdLine.push_back(QPointF(0.0, AcceptanceThreshold));
    ThresholdLine.push_back(QPointF(MaxXValue, AcceptanceThreshold));
    seriesThresholdLine->append(ThresholdLine);
    seriesThresholdLine->setColor(Qt::red);
    m_Chart->addSeries(seriesThresholdLine);
    seriesThresholdLine->attachAxis(m_xAxis);
    seriesThresholdLine->attachAxis(m_yAxis);

    m_xAxis->setRange(0.0, MaxXValue);
    m_xAxis->setTickInterval((static_cast<int>(MaxXValue)));
}
