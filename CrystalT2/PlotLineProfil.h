#ifndef PLOTLINEPROFIL_H
#define PLOTLINEPROFIL_H

#include <QtWidgets>
#include <QtCharts>

class MainAppCrystalT2;
class PlotLineProfil : public QChartView
{
public:
	PlotLineProfil(MainAppCrystalT2 *pMainAppCrystalT2, QWidget *FrameParent);
	~PlotLineProfil();
	MainAppCrystalT2 *GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
	void SetLineDataGradient(const double *Array, int size, double EdgeLeftSide, double EdgeRightSide);
private:
	MainAppCrystalT2    *m_MainAppCrystalT2;
	QChart              *m_Chart;
	QValueAxis          *m_xAxis;//Diagrammachsen
	QValueAxis          *m_yAxis;//Diagrammachsen
	QGraphicsScene      *m_GrapicSence;
};
#endif // PLOTLINEPROFIL_H
