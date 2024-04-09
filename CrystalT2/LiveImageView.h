#pragma once

#include <interfaces.h>
#include <QGraphicsView>
#include <QtGui>
#include "ImageMetaData.h"

class MeasurWindowRectGraphicsItem
{
  public:
    MeasurWindowRectGraphicsItem()
    {
        m_MeasureWindowItem = NULL;
        m_MeasureWindowItemRight = NULL;
        m_MeasureWindowItemLeft = NULL;
        m_MeasureWindowItemBot = NULL;
        m_MeasureWindowItemTop = NULL;
        m_MeasureWindowAmountLiquidLeft = NULL;
        m_MeasureWindowAmountLiquidRight = NULL;
    }

  public:
    QGraphicsRectItem *m_MeasureWindowItem, *m_MeasureWindowItemRight, *m_MeasureWindowItemLeft, *m_MeasureWindowItemBot, *m_MeasureWindowItemTop, *m_MeasureWindowAmountLiquidLeft,
        *m_MeasureWindowAmountLiquidRight;
    QGraphicsLineItem* m_DimensionLine;
    QGraphicsTextItem* m_TextItem;
};

class MeasureWindowWithKeyNumber
{
  public:
    QRect m_WindowRect;
    int m_WindowKeyIndex;
};
class LineProfileDialog;
class MainAppCrystalT2;
class LiveImageView : public QGraphicsView
{
    Q_OBJECT

  public:
    LiveImageView(MainAppCrystalT2* pMainAppCrystalT2);
    ~LiveImageView();
    void setImage(const ImageMetaData& Image);
    MainAppCrystalT2* GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
    void hideEvent(QHideEvent* e);
    void DrawCameraROIRect();
    void DrawMeasureWindow(int MeasureWindowID);
    void DrawAllMeasureWindows();
    void DrawMeasureResults(const ImageMetaData& Image);
    void GetRectSupportPoints(QRect& AOIRect, QRect& TopSquareMiddelPos, QRect& BottomSquareMiddelPos, QRect& LeftSquareMiddelPos, QRect& RightSquareMiddelPos);
    void ClearGraphicItem(QGraphicsItem* Item);
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void SetCurserForMeasureWindowROI(int x, int y);
    void MoveROIRect(QRect& ROIRect, int NewXPos, int NewYPos, double Zoom);
    void SetEnableSetCameraViewPortNew(bool set) { m_EnableSetCameraViewPortNew = set; }
    void ZoomRect(QRect& rect, double ZoomFaktor);
    void CalculateDistanceBetweenValveAndBottle();
    void DrawSupportingLines();
    static bool SortROIRectBYArea(MeasureWindowWithKeyNumber& p1, MeasureWindowWithKeyNumber& p2);
    void SetHTMLGraphicTextItem(QGraphicsTextItem* Item, QString& text, int xpos, int ypos);
    void ClearAllGraphicItem();
    void ClearCameraROIWindow();
    void SetMeasureWindowRectGraphicsItem();
    void ShowLineProfilDialog();
    void MoveAndResizeMeasureWindow(int NewXpos, int NewYpos, int MeasureWindowID, int CurserPosition, double Zoom);
    void DrawLineAtAngle(QLine& VerticalLineAngleInjector, double X, double Y, double Size, double Angle);
    void SetCurrentMaschineState(PluginInterface::MachineState set);
    void DrawCheckCleanImageROI();
    bool GetEnableChangeMeasureWindowPosition() { return m_EnableChangeMeasureWindowPosition; }
    bool EnableChangeMeasureWindowPosition();
    bool DisableChangeMeasureWindowPosition();
    void SetShowPlace(int set);
    int GetShowPlace() { return m_ShowPlace; }
    void SetOnCurrentProductionState();
    void DrawAutocalbrateROI(const QRect &CurrentRect);
        
    void SetSelectSpeedWindow(bool set)  {m_SelectSpeedWindow = set;}
    void SetSelectLiquidWindow(bool set) {m_SelectLiquidWindow = set;}

    bool IsSpeedWindowSelected() {  return m_SelectSpeedWindow; }
    bool IsLiquidWindowSelected() { return m_SelectLiquidWindow; }

    //void DrawCameraViewportOffsets();
    //void ClearCameraViewportOffsets();
     
  signals:
    void SignalDrawMeasureWindow();

  public slots:
    void SlotDrawMeasureWindow();

  private:
    QGraphicsScene* m_GrapicSenceLiveImage;
    MainAppCrystalT2* m_MainAppCrystalT2;
    QGraphicsPixmapItem* m_Pixmap;
    QGraphicsRectItem *m_AutCalibrateROIItem;
    QGraphicsRectItem *m_CameraROIItem, *m_CameraROIItemRight, *m_CameraROIItemLeft, *m_CameraROIItemBot, *m_CameraROIItemTop;
    QGraphicsLineItem *m_LineEdgePos, *m_LineEdgeArrowLeft, *m_LineEdgeArrowRight;
    QGraphicsLineItem *m_HorizontalLine, *m_VerticalLineLeft, *m_VerticalLineRight;
    QGraphicsLineItem *m_HorizontalLineDistanceBottleInjector, *m_VerticalLineAngleInjectorLeftValve, *m_VerticalLineAngleInjectorRightValve;
    QGraphicsTextItem* m_TextCameraViewPortOffsets;
    QPointF m_ClickPos;
    bool m_MousePressEventIsClicked;
    bool m_EnableSetCameraViewPortNew;
    bool m_FirstImageIsSet;
    bool m_EnableChangeMeasureWindowPosition;
    bool m_SelectSpeedWindow;
    bool m_SelectLiquidWindow;
    bool m_EnableMoveMeasureWindowWithMouse;
    int m_CurserPosition;
    int m_CurrentMeasureWindowKeyNumber;
    int m_ShowPlace;
    QHash<int, MeasurWindowRectGraphicsItem> m_MeasurWindowsRectGraphicsItem;
    LineProfileDialog* m_LineProfileDialog;
    QGraphicsRectItem* m_MeasurWindowRectGraphicsItemCleanImage;
};
