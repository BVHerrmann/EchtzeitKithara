#include "LiveImageView.h"
#include "EditProductDialog.h"
#include "GlobalConst.h"
#include "ImageData.h"
#include "MainAppCrystalT2.h"
#include "ProductData.h"
#include "SettingsData.h"
#include "qgraphicsitem.h"

LiveImageView::LiveImageView(MainAppCrystalT2* pMainAppCrystalT2)
    : QGraphicsView(pMainAppCrystalT2),
      m_Pixmap(NULL),
      m_GrapicSenceLiveImage(NULL),
      m_MainAppCrystalT2(NULL),
      m_CameraROIItemRight(NULL),
      m_CameraROIItemLeft(NULL),
      m_CameraROIItemBot(NULL),
      m_CameraROIItemTop(NULL),
      m_MousePressEventIsClicked(false),
      m_CurserPosition(CURSER_POSITION_NOT_ON_ROI),
      m_EnableSetCameraViewPortNew(false),
      m_FirstImageIsSet(false),
      m_CurrentMeasureWindowKeyNumber(0),
      m_LineEdgePos(NULL),
      m_LineEdgeArrowLeft(NULL),
      m_LineEdgeArrowRight(NULL),
      m_HorizontalLine(NULL),
      m_VerticalLineLeft(NULL),
      m_VerticalLineRight(NULL),
      m_TextCameraViewPortOffsets(NULL),
      m_HorizontalLineDistanceBottleInjector(NULL),
      m_VerticalLineAngleInjectorLeftValve(NULL),
      m_VerticalLineAngleInjectorRightValve(NULL),
      m_EnableChangeMeasureWindowPosition(true),
      m_MeasurWindowRectGraphicsItemCleanImage(NULL),
      m_ShowPlace(SHOW_PLACE_LIVE_IMAGE_VIEW_EDIT_PRODUCT_DIALOG),
      m_SelectSpeedWindow(true),
      m_SelectLiquidWindow(true),
      m_EnableMoveMeasureWindowWithMouse(false),
      m_AutCalibrateROIItem(NULL)
{
    m_MainAppCrystalT2 = pMainAppCrystalT2;
    m_GrapicSenceLiveImage = new QGraphicsScene(this);
    m_Pixmap = m_GrapicSenceLiveImage->addPixmap(QPixmap());
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) {
        QImage DefaultImage = QImage(GetMainAppCrystalT2()->GetImageData()->GetImageWidth(), GetMainAppCrystalT2()->GetImageData()->GetImageHeight(), QImage::Format_Grayscale8);
        DefaultImage.fill(0);
        m_Pixmap->setPixmap(QPixmap::fromImage(DefaultImage));
    }
    setSceneRect(m_Pixmap->boundingRect());

    setScene(m_GrapicSenceLiveImage);
    setMouseTracking(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    connect(this, &LiveImageView::SignalDrawMeasureWindow, this, &LiveImageView::SlotDrawMeasureWindow);
}

LiveImageView::~LiveImageView()
{
}

void LiveImageView::ShowLineProfilDialog()
{
}

void LiveImageView::hideEvent(QHideEvent* e)
{
    QGraphicsView::hideEvent(e);
}

void LiveImageView::SetMeasureWindowRectGraphicsItem()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) {
        QHash<int, QRect>* MeasureWindowRects = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRects();

        if (MeasureWindowRects) {
            QHashIterator<int, QRect> i(*MeasureWindowRects);
            int key;

            m_MeasurWindowsRectGraphicsItem.clear();
            while (i.hasNext()) {
                i.next();
                key = i.key();
                MeasurWindowRectGraphicsItem NewItem;
                m_MeasurWindowsRectGraphicsItem.insert(i.key(), NewItem);
            }
        }
    }
}

void LiveImageView::setImage(const ImageMetaData& Image)
{
    m_Pixmap->setPixmap(QPixmap::fromImage(Image.m_Image));
    setSceneRect(m_Pixmap->boundingRect());
    if (!Image.m_IsImageFromVideo) {
        if (!m_FirstImageIsSet) {
            SetMeasureWindowRectGraphicsItem();
            if (m_EnableSetCameraViewPortNew)
                DrawCameraROIRect();
            else
                DrawAllMeasureWindows();
            m_FirstImageIsSet = true;
        }

        if (Image.m_CurrentMeasuringResult.m_EdgeLeftFound && Image.m_CurrentMeasuringResult.m_EdgeRightFound && Image.m_CurrentMeasuringResult.m_ProductSizeInTolerance &&
            Image.m_CurrentMeasuringResult.m_ProductContrastOk)
            GetMainAppCrystalT2()->SetProfileLineData(Image.m_CurrentMeasuringResult.m_GradientProfileData, Image.m_CurrentMeasuringResult.m_RawProfileData,
                                                      Image.m_CurrentMeasuringResult.m_SizeProfileData, Image.m_CurrentMeasuringResult.m_LeftEdgeLocation,
                                                      Image.m_CurrentMeasuringResult.m_RightEdgeLocation);
        else
            GetMainAppCrystalT2()->SetProfileLineData(Image.m_CurrentMeasuringResult.m_GradientProfileData, Image.m_CurrentMeasuringResult.m_RawProfileData,
                                                      Image.m_CurrentMeasuringResult.m_SizeProfileData, 0.0, 0.0);

    } else {
        ClearAllGraphicItem();
        m_FirstImageIsSet = false;
    }
}

// void LiveImageView::DrawCameraViewportOffsets()
//{
//    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData() && GetMainAppCrystalT2()->GetSettingsData()) {
//        ClearCameraViewportOffsets();
//        QRect CameraViewport = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRect(ROI_ID_CAMERA);
//        double PixelSize = GetMainAppCrystalT2()->GetSettingsData()->m_PixelSize;
//        QString offsets = tr("x:%1mm<br>y:%2mm</br>").arg(CameraViewport.x() * PixelSize, 0, 'f', 1).arg(CameraViewport.y() * PixelSize, 0, 'f', 1);
//        QFont font;
//
//        font.setFamily(QString::fromUtf8("Siemens Sans"));
//        font.setPixelSize(16);
//        //font.setBold(true);
//        m_TextCameraViewPortOffsets = m_GrapicSenceLiveImage->addText(offsets);
//        m_TextCameraViewPortOffsets->setPos(0, 0);
//        m_TextCameraViewPortOffsets->setFont(font);
//        m_TextCameraViewPortOffsets->setHtml(QString("<div style='background:rgba(255, 255, 255, 100%);'>" + offsets + QString("</div>")));
//    }
//}
//
// void LiveImageView::ClearCameraViewportOffsets()
//{
//    ClearGraphicItem(m_TextCameraViewPortOffsets);
//}

void LiveImageView::DrawSupportingLines()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) {
        ProductData* pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
        if (pProductData && GetMainAppCrystalT2()->GetSettingsData()) {
            QLine VerticalLineAngleInjectorLeftValve, VerticalLineAngleInjectorRightValve;
            QLine HorizontalLineDistanceBottleInjector;
            QPen PenColor(Qt::darkMagenta);
            QRect MeasureWindoRectMeasurePos = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRect(ROI_ID_MEASURE_SPEED);
            QRect MeasureWindoRectinjectionPos = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRect(ROI_ID_MEASURE_LIQUID);
            double DistanceBottleInjectionInPixel, AngleInjector;
            int YPosHorizontalLineDistanceBottleInjector;
            int RotationPointX, RotationPointY;

            PenColor.setWidth(2);
            ClearGraphicItem(m_HorizontalLineDistanceBottleInjector);
            ClearGraphicItem(m_VerticalLineAngleInjectorLeftValve);
            ClearGraphicItem(m_VerticalLineAngleInjectorRightValve);
            DistanceBottleInjectionInPixel = GetMainAppCrystalT2()->MetricToPixel(pProductData->m_DistanceInjectorBottle);  // vertikaler abstand
            YPosHorizontalLineDistanceBottleInjector = MeasureWindoRectMeasurePos.y() - DistanceBottleInjectionInPixel;
            if (YPosHorizontalLineDistanceBottleInjector < 0) YPosHorizontalLineDistanceBottleInjector = 0;
            HorizontalLineDistanceBottleInjector.setLine(0, YPosHorizontalLineDistanceBottleInjector, GetMainAppCrystalT2()->GetImageData()->GetImageWidth(), YPosHorizontalLineDistanceBottleInjector);
            if (GetMainAppCrystalT2()->IsFirstTriggerOnLeftSide())
                AngleInjector = 90.0 - pProductData->m_InjectionAngle;
            else
                AngleInjector = 90.0 + pProductData->m_InjectionAngle;
            RotationPointX = MeasureWindoRectinjectionPos.x() + MeasureWindoRectinjectionPos.width() / 2;
            RotationPointY = MeasureWindoRectinjectionPos.y() + MeasureWindoRectinjectionPos.height() / 2;
            if (GetMainAppCrystalT2()->GetSettingsData()->m_WorkWithTwoValves) {
                double DistanceBetweenTwoValves2 = GetMainAppCrystalT2()->MetricToPixel(GetMainAppCrystalT2()->GetSettingsData()->m_DistancesBetweenValves) / 2.0;
                RotationPointX = RotationPointX - DistanceBetweenTwoValves2;
                DrawLineAtAngle(VerticalLineAngleInjectorLeftValve, RotationPointX, RotationPointY, RotationPointY * 2, AngleInjector);
                RotationPointX = RotationPointX + DistanceBetweenTwoValves2 * 2.0;
                DrawLineAtAngle(VerticalLineAngleInjectorRightValve, RotationPointX, RotationPointY, RotationPointY * 2, AngleInjector);
                m_VerticalLineAngleInjectorRightValve = m_GrapicSenceLiveImage->addLine(VerticalLineAngleInjectorRightValve, PenColor);
                m_VerticalLineAngleInjectorLeftValve = m_GrapicSenceLiveImage->addLine(VerticalLineAngleInjectorLeftValve, PenColor);
            } else {
                DrawLineAtAngle(VerticalLineAngleInjectorLeftValve, RotationPointX, RotationPointY, RotationPointY * 2, AngleInjector);
                m_VerticalLineAngleInjectorLeftValve = m_GrapicSenceLiveImage->addLine(VerticalLineAngleInjectorLeftValve, PenColor);
            }
            m_HorizontalLineDistanceBottleInjector = m_GrapicSenceLiveImage->addLine(HorizontalLineDistanceBottleInjector, PenColor);
        }
    }
}

void LiveImageView::DrawLineAtAngle(QLine& VerticalLineAngleInjector, double X, double Y, double Size, double Angle)
{
    double RadAngle = (Angle < 0.01) ? (0) : (Angle * M_PI / 180.0);
    double ResultCos = cos(RadAngle);
    double ResultSin = sin(RadAngle);
    double xCos = Size * ResultCos / 2.0;
    double xSin = Size * ResultSin / 2.0;

    long X7 = (long)(X - xCos + 0.5);
    long Y7 = (long)(Y + xSin + 0.5);
    long X8 = (long)(X + xCos + 0.5);
    long Y8 = (long)(Y - xSin + 0.5);

    VerticalLineAngleInjector.setLine(X7, Y7, X8, Y8);
}

void LiveImageView::DrawMeasureResults(const ImageMetaData& Image)
{
    QRect CurrentRect;
    int ImageWidth = Image.m_Image.width();
    int ImageHeight = Image.m_Image.height();
    int LineStartYPos;
    int LineLenght = 100;
    int ArrowLenght = LineLenght * 0.05;
    QString TextMeasuringResults, TextTriggerDelayTime;
    QLine ShowXPos;
    QLine ArrowLeft;
    QLine ArrowRight;
    QPen PenColor(Qt::green);
    double BottleMiddelPos, ZoomFactor, x1, x2, y1, y2;

    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) {
        CurrentRect = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRect(ROI_ID_MEASURE_SPEED);
        ZoomFactor = GetMainAppCrystalT2()->GetDisplayZoomFactor();
        LineStartYPos = CurrentRect.y() + CurrentRect.height();  // -LineLenght;
        y1 = LineStartYPos * ZoomFactor;
        y2 = LineLenght + y1;
        if (y2 > ImageHeight) y2 = ImageHeight;

        ClearGraphicItem(m_LineEdgePos);
        ClearGraphicItem(m_LineEdgeArrowLeft);
        ClearGraphicItem(m_LineEdgeArrowRight);
        if (Image.m_CurrentMeasuringResult.m_EdgeLeftFound && Image.m_CurrentMeasuringResult.m_EdgeRightFound) {
            BottleMiddelPos = (Image.m_CurrentMeasuringResult.m_ResultEdgeRightXPos - Image.m_CurrentMeasuringResult.m_ResultEdgeLeftXPos) / 2.0 + Image.m_CurrentMeasuringResult.m_ResultEdgeLeftXPos;
            x1 = x2 = BottleMiddelPos * ZoomFactor;
            ShowXPos.setLine(x1, y1, x2, y2);
            ArrowLeft.setLine(x1 - 5, y1 + ArrowLenght, x1, y1);
            ArrowRight.setLine(x1 + 5, y1 + ArrowLenght, x1, y1);
        }
        // DrawPreviousResults(Image);
    }
}

void LiveImageView::SetHTMLGraphicTextItem(QGraphicsTextItem* Item, QString& text, int xpos, int ypos)
{
    int w;
    Item->setHtml(QString("<p style='background:rgba(0,0,0, 100%);'>" + text + QString("</p>")));
    Item->setDefaultTextColor(Qt::white);

    w = Item->boundingRect().width() / 2.0;
    Item->setPos(xpos - w, ypos);
}

void LiveImageView::DrawAutocalbrateROI(const QRect& CurrentRect)
{
    QPen PenColor(Qt::blue);

    ClearGraphicItem(m_AutCalibrateROIItem);
    if (!CurrentRect.isEmpty()) m_AutCalibrateROIItem = m_GrapicSenceLiveImage->addRect(CurrentRect, PenColor);
}

void LiveImageView::DrawCameraROIRect()
{
    QPen PenColor(Qt::blue);
    QBrush BrushColor(Qt::white);
    QRect TopSquareMiddelPos, BottomSquareMiddelPos, LeftSquareMiddelPos, RightSquareMiddelPos;
    QRect CurrentRect = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRect(ROI_ID_CAMERA);
    double ZoomFactor = GetMainAppCrystalT2()->GetDisplayZoomFactor();

    GetRectSupportPoints(CurrentRect, TopSquareMiddelPos, BottomSquareMiddelPos, LeftSquareMiddelPos, RightSquareMiddelPos);
    if (ZoomFactor != 1.0) {
        ZoomRect(CurrentRect, ZoomFactor);
        ZoomRect(TopSquareMiddelPos, ZoomFactor);
        ZoomRect(BottomSquareMiddelPos, ZoomFactor);
        ZoomRect(LeftSquareMiddelPos, ZoomFactor);
        ZoomRect(RightSquareMiddelPos, ZoomFactor);
    }
    ClearCameraROIWindow();
    m_CameraROIItem = m_GrapicSenceLiveImage->addRect(CurrentRect, PenColor);
    m_CameraROIItem->setToolTip(
        tr("ROI Rect: x:%1 y:%2 w:%3 h:%4").arg(CurrentRect.x() / ZoomFactor).arg(CurrentRect.y() / ZoomFactor).arg(CurrentRect.width() / ZoomFactor).arg(CurrentRect.height() / ZoomFactor));
    m_CameraROIItemRight = m_GrapicSenceLiveImage->addRect(RightSquareMiddelPos, PenColor, BrushColor);
    m_CameraROIItemLeft = m_GrapicSenceLiveImage->addRect(LeftSquareMiddelPos, PenColor, BrushColor);
    m_CameraROIItemBot = m_GrapicSenceLiveImage->addRect(BottomSquareMiddelPos, PenColor, BrushColor);
    m_CameraROIItemTop = m_GrapicSenceLiveImage->addRect(TopSquareMiddelPos, PenColor, BrushColor);
}

void LiveImageView::ClearCameraROIWindow()
{
    ClearGraphicItem(m_CameraROIItemRight);
    ClearGraphicItem(m_CameraROIItemLeft);
    ClearGraphicItem(m_CameraROIItemBot);
    ClearGraphicItem(m_CameraROIItemTop);
    ClearGraphicItem(m_CameraROIItem);
}

void LiveImageView::DrawMeasureWindow(int MeasureWindowID)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) {
        QString TextInfo;
        QPen PenColorInjection(Qt::darkBlue);
        QPen PenColorMeasureSpeed(Qt::darkGreen);
        QPen PenColorTriggerPos(Qt::yellow);
        QPen PenColor;
        QBrush BrushColor(Qt::white);
        QLine DimensionLine;
        QLine DimensionLineHorizontal;
        QRect TopSquareMiddelPos, BottomSquareMiddelPos, LeftSquareMiddelPos, RightSquareMiddelPos;
        QRect CurrentRect = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRect(MeasureWindowID);
        QRect CurrentROIRectSpeedBottleOnTriggerPos = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRect(ROI_ID_MEASURE_BOTTLE_UNDER_VALVE);
        MeasurWindowRectGraphicsItem MeasurWindowRectGraphicItem = m_MeasurWindowsRectGraphicsItem.value(MeasureWindowID);
        MeasurWindowRectGraphicsItem MeasurWindowRectGraphicItemBottleOnTriggerPos = m_MeasurWindowsRectGraphicsItem.value(ROI_ID_MEASURE_BOTTLE_UNDER_VALVE);
        double ZoomFactor = GetMainAppCrystalT2()->GetDisplayZoomFactor();
        int MiddelPos = static_cast<int>(CurrentRect.x() + CurrentRect.width() / 2.0 + 0.5);
        GetRectSupportPoints(CurrentRect, TopSquareMiddelPos, BottomSquareMiddelPos, LeftSquareMiddelPos, RightSquareMiddelPos);

        if (ZoomFactor != 1.0) {
            ZoomRect(CurrentRect, ZoomFactor);
            ZoomRect(TopSquareMiddelPos, ZoomFactor);
            ZoomRect(BottomSquareMiddelPos, ZoomFactor);
            ZoomRect(LeftSquareMiddelPos, ZoomFactor);
            ZoomRect(RightSquareMiddelPos, ZoomFactor);
        }
        ClearGraphicItem(m_MeasurWindowRectGraphicsItemCleanImage);
        DimensionLine.setLine(TopSquareMiddelPos.x() + SUPPORT_RECT_SIZE_IN_PIXEL / 2, 25, TopSquareMiddelPos.x() + SUPPORT_RECT_SIZE_IN_PIXEL / 2, TopSquareMiddelPos.y());
        ClearGraphicItem(MeasurWindowRectGraphicItem.m_MeasureWindowItemRight);
        ClearGraphicItem(MeasurWindowRectGraphicItem.m_MeasureWindowItemLeft);
        ClearGraphicItem(MeasurWindowRectGraphicItem.m_MeasureWindowItemBot);
        ClearGraphicItem(MeasurWindowRectGraphicItem.m_MeasureWindowItemTop);
        ClearGraphicItem(MeasurWindowRectGraphicItem.m_MeasureWindowItem);
        ClearGraphicItem(MeasurWindowRectGraphicItemBottleOnTriggerPos.m_MeasureWindowItem);
        if (MeasureWindowID == ROI_ID_MEASURE_SPEED) {
            PenColor = PenColorMeasureSpeed;
        } else {
            if (MeasureWindowID == ROI_ID_MEASURE_LIQUID) {
                PenColor = PenColorInjection;
                ClearGraphicItem(MeasurWindowRectGraphicItem.m_MeasureWindowAmountLiquidLeft);  // rects are insde blue rect
                ClearGraphicItem(MeasurWindowRectGraphicItem.m_MeasureWindowAmountLiquidRight);
                // Draw rect to measure amount of liquid inside blue rect
                if (GetMainAppCrystalT2()->GetSettingsData()->m_WorkWithTwoValves) {
                    QPair<QRect, QRect> FlowChannelRects = GetMainAppCrystalT2()->GetLeftRighFlowChannelLiquid(CurrentRect);
                    MeasurWindowRectGraphicItem.m_MeasureWindowAmountLiquidLeft = m_GrapicSenceLiveImage->addRect(FlowChannelRects.first, PenColor);
                    MeasurWindowRectGraphicItem.m_MeasureWindowAmountLiquidRight = m_GrapicSenceLiveImage->addRect(FlowChannelRects.second, PenColor);
                } else {
                    ProductData* pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
                    if (pProductData) {
                        int MiddleWidtInPixel = static_cast<int>(pProductData->m_InjectionMiddleWindowWidthInMm / GetMainAppCrystalT2()->GetSettingsData()->m_PixelSize);
                        QRect FlowChannelRect(CurrentRect.x() + CurrentRect.width() / 2 - MiddleWidtInPixel / 2, CurrentRect.y(), MiddleWidtInPixel, CurrentRect.height());
                        MeasurWindowRectGraphicItem.m_MeasureWindowAmountLiquidLeft = m_GrapicSenceLiveImage->addRect(FlowChannelRect, PenColor);
                    }
                }
            }
        }
        if (m_ShowPlace == SHOW_PLACE_LIVE_IMAGE_VIEW_EDIT_PRODUCT_DIALOG) {
            MeasurWindowRectGraphicItemBottleOnTriggerPos.m_MeasureWindowItem = m_GrapicSenceLiveImage->addRect(CurrentROIRectSpeedBottleOnTriggerPos, PenColorTriggerPos);
            m_MeasurWindowsRectGraphicsItem.insert(ROI_ID_MEASURE_BOTTLE_UNDER_VALVE, MeasurWindowRectGraphicItemBottleOnTriggerPos);
        }
        if (m_EnableChangeMeasureWindowPosition) {
            if (MeasureWindowID == ROI_ID_MEASURE_SPEED && m_SelectSpeedWindow || MeasureWindowID == ROI_ID_MEASURE_LIQUID && m_SelectLiquidWindow) {
                // MeasurWindowRectGraphicItem.m_MeasureWindowItemRight = m_GrapicSenceLiveImage->addRect(RightSquareMiddelPos, PenColor, BrushColor);
                // MeasurWindowRectGraphicItem.m_MeasureWindowItemLeft = m_GrapicSenceLiveImage->addRect(LeftSquareMiddelPos, PenColor, BrushColor);
                // MeasurWindowRectGraphicItem.m_MeasureWindowItemBot = m_GrapicSenceLiveImage->addRect(BottomSquareMiddelPos, PenColor, BrushColor);
                // MeasurWindowRectGraphicItem.m_MeasureWindowItemTop = m_GrapicSenceLiveImage->addRect(TopSquareMiddelPos, PenColor, BrushColor);
                PenColor.setStyle(Qt::DashLine);
            }
        }
        MeasurWindowRectGraphicItem.m_MeasureWindowItem = m_GrapicSenceLiveImage->addRect(CurrentRect, PenColor);
        MeasurWindowRectGraphicItem.m_MeasureWindowItem->setToolTip(tr("Rect-> ID:%1 x:%2 y:%3 w:%4 h:%5")
                                                                        .arg(MeasureWindowID)
                                                                        .arg(CurrentRect.x() / ZoomFactor)
                                                                        .arg(CurrentRect.y() / ZoomFactor)
                                                                        .arg(CurrentRect.width() / ZoomFactor)
                                                                        .arg(CurrentRect.height() / ZoomFactor));
        m_MeasurWindowsRectGraphicsItem.insert(MeasureWindowID, MeasurWindowRectGraphicItem);
        CalculateDistanceBetweenValveAndBottle();
        DrawSupportingLines();
    }
}

void LiveImageView::DrawCheckCleanImageROI()
{
    QPen PenColorTriggerPos(Qt::darkYellow);
    QRect MeasureWindowCheckCleanImage = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowCheckCleanImage();
    m_MeasurWindowRectGraphicsItemCleanImage = m_GrapicSenceLiveImage->addRect(MeasureWindowCheckCleanImage, PenColorTriggerPos);
}

void LiveImageView::CalculateDistanceBetweenValveAndBottle()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) {
        QRectF RectMeasurePos = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRect(ROI_ID_MEASURE_SPEED);     // Measuring Pos
        QRectF RectInjectionPos = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRect(ROI_ID_MEASURE_LIQUID);  // Injektion Pos
        int ReferenceMeasurePositionInPixel = static_cast<int>(RectMeasurePos.x() + RectMeasurePos.width() / 2.0 + 0.5);
        int ReferenceInjectionPositionInPixel = static_cast<int>(RectInjectionPos.x() + RectInjectionPos.width() / 2.0 + 0.5);
        double ZoomFactor = GetMainAppCrystalT2()->GetDisplayZoomFactor();
        double DistanceBetweenMeasurePosAndTriggerPosInPixel = fabs(RectMeasurePos.x() + RectMeasurePos.width() / 2.0 - (RectInjectionPos.x() + RectInjectionPos.width() / 2.0));
        double MiddleInjectionPos = (RectInjectionPos.x() + RectInjectionPos.width() / 2.0) * ZoomFactor;
        double MiddleMeasurePos = (RectMeasurePos.x() + RectMeasurePos.width() / 2.0) * ZoomFactor;
        double HalfBottleNeckDiameter = 0.0;
        double DistanceBetweenTwoValves2 = 0.0;

        if (GetMainAppCrystalT2()->GetSettingsData()->m_WorkWithTwoValves)
            DistanceBetweenTwoValves2 = GetMainAppCrystalT2()->MetricToPixel(GetMainAppCrystalT2()->GetSettingsData()->m_DistancesBetweenValves) / 2.0;
        ProductData* pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
        if (pProductData) HalfBottleNeckDiameter = GetMainAppCrystalT2()->MetricToPixel(pProductData->m_BottleNeckDiameter / 2.0);
        // Berechnung Distanz Flaschenposition zum ersten Ventil
        DistanceBetweenMeasurePosAndTriggerPosInPixel = DistanceBetweenMeasurePosAndTriggerPosInPixel - HalfBottleNeckDiameter - DistanceBetweenTwoValves2;
        GetMainAppCrystalT2()->GetImageData()->SetReferenceInjectionPositionInPixel(ReferenceInjectionPositionInPixel);
        GetMainAppCrystalT2()->GetImageData()->SetReferenceMeasurePositionInPixel(ReferenceMeasurePositionInPixel);
        // trigger nicht erst ausloesen wenn Flasche mittig unter dem Ventil, sondern kurz vorher
        GetMainAppCrystalT2()->GetImageData()->SetDistanceBetweenMeasurePosAndTriggerPosInPixel(DistanceBetweenMeasurePosAndTriggerPosInPixel);
    }
}

void LiveImageView::DrawAllMeasureWindows()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) {
        QHash<int, QRect>* MeasureWindowRects = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRects();
        if (MeasureWindowRects) {
            QHashIterator<int, QRect> i(*MeasureWindowRects);
            int key;

            if (m_ShowPlace == SHOW_PLACE_LIVE_IMAGE_VIEW_EDIT_PRODUCT_DIALOG) {
                while (i.hasNext()) {
                    i.next();
                    key = i.key();
                    if (key != ROI_ID_CAMERA && key != ROI_ID_MEASURE_BOTTLE_UNDER_VALVE) {
                        DrawMeasureWindow(key);
                    }
                }
                DrawSupportingLines();
            } else {
                if (m_ShowPlace == SHOW_PLACE_LIVE_IMAGE_VIEW_CLEAN_IMAGE_DIALOG) {
                    DrawCheckCleanImageROI();
                } else {
                    if (m_ShowPlace == SHOW_PLACE_LIVE_IMAGE_VIEW_DROP_ESTIMATION_DIALOG) {
                        DrawSupportingLines();
                    }
                }
            }
        }
    }
}

void LiveImageView::SetShowPlace(int set)
{
    m_ShowPlace = set;
}

void LiveImageView::SetOnCurrentProductionState()
{
    if (GetMainAppCrystalT2()) {
        if (GetMainAppCrystalT2()->GetCurrentMaschineState() == PluginInterface::MachineState::Production) {
            DisableChangeMeasureWindowPosition();
        } else {
            EnableChangeMeasureWindowPosition();
        }
    }
}

// damit während der Produktion die Messfenster nicht verschiebbar sind
void LiveImageView::SetCurrentMaschineState(PluginInterface::MachineState set)
{
    if (m_ShowPlace == SHOW_PLACE_LIVE_IMAGE_VIEW_EDIT_PRODUCT_DIALOG) {
        if (set == PluginInterface::MachineState::Production) {
            DisableChangeMeasureWindowPosition();
        } else {
            EnableChangeMeasureWindowPosition();
        }
    }
}

bool LiveImageView::EnableChangeMeasureWindowPosition()
{
    bool ReDrawMeasureWindow = false;
    if (!m_EnableChangeMeasureWindowPosition) {
        m_EnableChangeMeasureWindowPosition = true;
        emit SignalDrawMeasureWindow();
        ReDrawMeasureWindow = true;
    }
    return ReDrawMeasureWindow;
}

bool LiveImageView::DisableChangeMeasureWindowPosition()
{
    bool ReDrawMeasureWindow = false;
    if (m_EnableChangeMeasureWindowPosition) {
        m_EnableChangeMeasureWindowPosition = false;
        emit SignalDrawMeasureWindow();
        ReDrawMeasureWindow = true;
    }
    return ReDrawMeasureWindow;
}

void LiveImageView::SlotDrawMeasureWindow()
{
    DrawAllMeasureWindows();
}

void LiveImageView::GetRectSupportPoints(QRect& AOIRect, QRect& TopSquareMiddelPos, QRect& BottomSquareMiddelPos, QRect& LeftSquareMiddelPos, QRect& RightSquareMiddelPos)
{
    if (GetMainAppCrystalT2()) {
        double Zoom = GetMainAppCrystalT2()->GetDisplayZoomFactor();
        int AOIRectWidth2;
        int AOIRectHeight2;
        int ScanWidth2;
        int ScanWidth;

        AOIRectWidth2 = static_cast<int>(AOIRect.width() / 2.0 + 0.5);
        AOIRectHeight2 = static_cast<int>(AOIRect.height() / 2.0 + 0.5);
        ScanWidth2 = ((SUPPORT_RECT_SIZE_IN_PIXEL / 2.0 + 0.5) / Zoom);  //
        ScanWidth = ScanWidth2 * 2;

        TopSquareMiddelPos.setX(AOIRect.topLeft().x() + AOIRectWidth2 - ScanWidth2);  // top left position
        TopSquareMiddelPos.setY(AOIRect.topLeft().y() - ScanWidth2);                  // top left position
        TopSquareMiddelPos.setWidth(ScanWidth);
        TopSquareMiddelPos.setHeight(ScanWidth);

        BottomSquareMiddelPos.setX(AOIRect.bottomLeft().x() + AOIRectWidth2 - ScanWidth2);
        BottomSquareMiddelPos.setY(AOIRect.bottomLeft().y() - ScanWidth2);
        BottomSquareMiddelPos.setWidth(ScanWidth);
        BottomSquareMiddelPos.setHeight(ScanWidth);

        LeftSquareMiddelPos.setX(AOIRect.topLeft().x() - ScanWidth2);
        LeftSquareMiddelPos.setY(AOIRect.topLeft().y() + AOIRectHeight2 - ScanWidth2);
        LeftSquareMiddelPos.setWidth(ScanWidth);
        LeftSquareMiddelPos.setHeight(ScanWidth);

        RightSquareMiddelPos.setX(AOIRect.bottomRight().x() - ScanWidth2);
        RightSquareMiddelPos.setY(AOIRect.bottomRight().y() - AOIRectHeight2 - ScanWidth2);
        RightSquareMiddelPos.setWidth(ScanWidth);
        RightSquareMiddelPos.setHeight(ScanWidth);
    }
}

void LiveImageView::ZoomRect(QRect& rect, double ZoomFactor)
{
    QPoint TopLeft, BottomRight;
    TopLeft = rect.topLeft() * ZoomFactor;
    BottomRight = rect.bottomRight() * ZoomFactor;
    rect.setTopLeft(TopLeft);
    rect.setBottomRight(BottomRight);
}

void LiveImageView::ClearGraphicItem(QGraphicsItem* Item)
{
    if (Item) {
        QList<QGraphicsItem*> items = m_GrapicSenceLiveImage->items();
        for (int i = 0; i < items.size(); i++) {
            QGraphicsItem* item = items.at(i);
            if (item == Item) {
                m_GrapicSenceLiveImage->removeItem(item);
                delete item;
                break;
            }
        }
    }
}

void LiveImageView::ClearAllGraphicItem()
{
    QList<QGraphicsItem*> items = m_GrapicSenceLiveImage->items();
    for (int i = 0; i < items.size(); i++) {
        QGraphicsItem* item = items.at(i);
        if (item != m_Pixmap) {
            m_GrapicSenceLiveImage->removeItem(item);
            delete item;
        }
    }
}

void LiveImageView::mousePressEvent(QMouseEvent* e)
{
    if (m_EnableChangeMeasureWindowPosition) {
        if (e->button() == Qt::LeftButton) {
            m_ClickPos = mapToScene(e->pos());
            SetCurserForMeasureWindowROI(m_ClickPos.x(), m_ClickPos.y());
            m_MousePressEventIsClicked = true;
            if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetEditProductDialog()) {
                if (m_CurserPosition == CURSER_POSITION_MOVE_IN_ROI) {
                    if (m_CurrentMeasureWindowKeyNumber == ROI_ID_MEASURE_SPEED) {
                        GetMainAppCrystalT2()->GetEditProductDialog()->SetCheckBoxSpeed(true);
                    }
                    if (m_CurrentMeasureWindowKeyNumber == ROI_ID_MEASURE_LIQUID) {
                        GetMainAppCrystalT2()->GetEditProductDialog()->SetCheckBoxLiquid(true);
                    }
                }
            }
        }
    }
    QGraphicsView::mousePressEvent(e);
}

void LiveImageView::mouseMoveEvent(QMouseEvent* e)
{
    if (m_EnableMoveMeasureWindowWithMouse) {
        QPointF CurrentPoint = mapToScene(e->pos());
        if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData() && m_MousePressEventIsClicked) {
            if (CurrentPoint.x() >= 0 && CurrentPoint.y() >= 0 && CurrentPoint.x() < m_Pixmap->pixmap().width() && CurrentPoint.y() < m_Pixmap->pixmap().height()) {
                double Zoom = GetMainAppCrystalT2()->GetDisplayZoomFactor();
                int NewXPos = (static_cast<int>(CurrentPoint.x() / Zoom + 0.5));
                int NewYPos = (static_cast<int>(CurrentPoint.y() / Zoom + 0.5));
                QPen PenColor(Qt::white);
                if (NewXPos < 0) NewXPos = 0;
                if (NewYPos < 0) NewYPos = 0;

                MoveAndResizeMeasureWindow(NewXPos, NewYPos, m_CurrentMeasureWindowKeyNumber, m_CurserPosition, Zoom);
                GetMainAppCrystalT2()->MeasureWindowChangedByMouse(m_CurrentMeasureWindowKeyNumber);
                if (m_EnableSetCameraViewPortNew)
                    DrawCameraROIRect();
                else
                    DrawMeasureWindow(m_CurrentMeasureWindowKeyNumber);
            }
        }
    }
    QGraphicsView::mouseMoveEvent(e);
}

void LiveImageView::MoveAndResizeMeasureWindow(int NewXPos, int NewYPos, int MeasureWindowID, int CurserPosition, double Zoom)
{
    QRect ROIRect = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRect(MeasureWindowID);
    QRect CurrentROIRectBottleOnTriggerPos = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRect(ROI_ID_MEASURE_BOTTLE_UNDER_VALVE);
    QRect NewRectBottleOnTriggerPos;
    int x = ROIRect.x();
    int y = ROIRect.y();
    int w = ROIRect.width();
    int h = ROIRect.height();
    int DeltaX, NewWidth;
    int DeltaY, NewHeight;
    int MinHeight = MINIMUM_ROI_SIZE_IN_PIXEL;
    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
    int MinWidthInPixelBlueWindow = 150;
    if (pSettingsData) MinWidthInPixelBlueWindow = pSettingsData->m_MinWidthInPixelBlueWindow;

    switch (CurserPosition) {
        case CURSER_POSITION_RESIZE_TOP:
            DeltaY = ROIRect.y() - NewYPos;
            NewHeight = ROIRect.height() + DeltaY;
            if (MeasureWindowID == ROI_ID_MEASURE_SPEED) {
                if (GetMainAppCrystalT2()->GetSettingsData()) {
                    MinHeight = GetMainAppCrystalT2()->GetSettingsData()->m_MinMeasureWindowHeight;
                }
            }
            if (NewYPos > 0 && NewHeight > MinHeight) {
                ROIRect.setX(x);
                ROIRect.setY(NewYPos);
                ROIRect.setHeight(NewHeight);
                ROIRect.setWidth(w);
                GetMainAppCrystalT2()->GetImageData()->SetMeasureWindowRect(MeasureWindowID, ROIRect);
                if (MeasureWindowID == ROI_ID_MEASURE_SPEED) {
                    NewRectBottleOnTriggerPos.setX(CurrentROIRectBottleOnTriggerPos.x());
                    NewRectBottleOnTriggerPos.setY(NewYPos);
                    NewRectBottleOnTriggerPos.setHeight(NewHeight);
                    NewRectBottleOnTriggerPos.setWidth(CurrentROIRectBottleOnTriggerPos.width());
                    GetMainAppCrystalT2()->GetImageData()->SetMeasureWindowRect(ROI_ID_MEASURE_BOTTLE_UNDER_VALVE, NewRectBottleOnTriggerPos);
                }
            }
            break;
        case CURSER_POSITION_RESIZE_BOTTOM:
            DeltaY = ROIRect.y() + ROIRect.height() - NewYPos;
            NewHeight = ROIRect.height() - DeltaY;
            if (MeasureWindowID == ROI_ID_MEASURE_SPEED) {
                if (GetMainAppCrystalT2()->GetSettingsData()) {
                    MinHeight = GetMainAppCrystalT2()->GetSettingsData()->m_MinMeasureWindowHeight;
                }
            }
            if (NewYPos < GetMainAppCrystalT2()->GetImageData()->GetImageHeight() && NewHeight > MinHeight) {
                ROIRect.setX(x);
                ROIRect.setY(y);
                ROIRect.setHeight(NewHeight);
                ROIRect.setWidth(w);
                GetMainAppCrystalT2()->GetImageData()->SetMeasureWindowRect(MeasureWindowID, ROIRect);
                if (MeasureWindowID == ROI_ID_MEASURE_SPEED) {
                    NewRectBottleOnTriggerPos.setX(CurrentROIRectBottleOnTriggerPos.x());
                    NewRectBottleOnTriggerPos.setY(CurrentROIRectBottleOnTriggerPos.y());
                    NewRectBottleOnTriggerPos.setHeight(NewHeight);
                    NewRectBottleOnTriggerPos.setWidth(CurrentROIRectBottleOnTriggerPos.width());
                    GetMainAppCrystalT2()->GetImageData()->SetMeasureWindowRect(ROI_ID_MEASURE_BOTTLE_UNDER_VALVE, NewRectBottleOnTriggerPos);
                }
            }
            break;
        case CURSER_POSITION_RESIZE_LEFT:
            DeltaX = ROIRect.x() - NewXPos;
            NewWidth = ROIRect.width() + DeltaX;
            if (NewXPos > 0 && NewWidth > MINIMUM_ROI_SIZE_IN_PIXEL) {
                if (MeasureWindowID == ROI_ID_MEASURE_LIQUID) {
                    if (NewWidth < MinWidthInPixelBlueWindow) NewWidth = MinWidthInPixelBlueWindow;
                    if ((NewXPos + NewWidth) > GetMainAppCrystalT2()->GetImageData()->GetImageWidth()) {
                        NewXPos = GetMainAppCrystalT2()->GetImageData()->GetImageWidth() - NewWidth - 1;
                    }
                }
                ROIRect.setX(NewXPos);
                ROIRect.setY(y);
                ROIRect.setHeight(h);
                ROIRect.setWidth(NewWidth);
                GetMainAppCrystalT2()->GetImageData()->SetMeasureWindowRect(MeasureWindowID, ROIRect);
                if (MeasureWindowID == ROI_ID_MEASURE_LIQUID) {
                    NewRectBottleOnTriggerPos.setX(NewXPos);
                    NewRectBottleOnTriggerPos.setY(CurrentROIRectBottleOnTriggerPos.y());
                    NewRectBottleOnTriggerPos.setHeight(CurrentROIRectBottleOnTriggerPos.height());
                    NewRectBottleOnTriggerPos.setWidth(NewWidth);
                    GetMainAppCrystalT2()->GetImageData()->SetMeasureWindowRect(ROI_ID_MEASURE_BOTTLE_UNDER_VALVE, NewRectBottleOnTriggerPos);
                }
            }
            break;
        case CURSER_POSITION_RESIZE_RIGHT:
            DeltaX = NewXPos - (ROIRect.x() + ROIRect.width());
            NewWidth = ROIRect.width() + DeltaX;
            if ((NewXPos < GetMainAppCrystalT2()->GetImageData()->GetImageWidth()) && NewWidth > MINIMUM_ROI_SIZE_IN_PIXEL) {
                if (MeasureWindowID == ROI_ID_MEASURE_LIQUID) {
                    if (NewWidth < MinWidthInPixelBlueWindow) NewWidth = MinWidthInPixelBlueWindow;
                }
                ROIRect.setX(x);
                ROIRect.setY(y);
                ROIRect.setHeight(h);
                ROIRect.setWidth(NewWidth);
                GetMainAppCrystalT2()->GetImageData()->SetMeasureWindowRect(MeasureWindowID, ROIRect);
                if (MeasureWindowID == ROI_ID_MEASURE_LIQUID) {
                    NewRectBottleOnTriggerPos.setX(CurrentROIRectBottleOnTriggerPos.x());
                    NewRectBottleOnTriggerPos.setY(CurrentROIRectBottleOnTriggerPos.y());
                    NewRectBottleOnTriggerPos.setHeight(CurrentROIRectBottleOnTriggerPos.height());
                    NewRectBottleOnTriggerPos.setWidth(NewWidth);
                    GetMainAppCrystalT2()->GetImageData()->SetMeasureWindowRect(ROI_ID_MEASURE_BOTTLE_UNDER_VALVE, NewRectBottleOnTriggerPos);
                }
            }
            break;
        case CURSER_POSITION_MOVE_IN_ROI:
            MoveROIRect(ROIRect, NewXPos, NewYPos, Zoom);
            break;
        default:
            break;
    }
}

void LiveImageView::mouseReleaseEvent(QMouseEvent* e)
{
    if (m_MousePressEventIsClicked) {
        m_MousePressEventIsClicked = false;
        QApplication::restoreOverrideCursor();
        m_CurserPosition = CURSER_POSITION_NOT_ON_ROI;
    }

    QGraphicsView::mouseReleaseEvent(e);
}

void LiveImageView::SetCurserForMeasureWindowROI(int x, int y)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) {
        QRect AOIRect;
        QRect TopSquareMiddelPos, BottomSquareMiddelPos, LeftSquareMiddelPos, RightSquareMiddelPos;
        double Zoom = GetMainAppCrystalT2()->GetDisplayZoomFactor();
        int XPos = static_cast<int>(x / Zoom + 0.5);
        int YPos = static_cast<int>(y / Zoom + 0.5);
        MeasureWindowWithKeyNumber CurrentRect;
        QList<MeasureWindowWithKeyNumber> ListROISortByArea;
        QHash<int, QRect>* pMeasureWindowRect = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRects();
        QHashIterator<int, QRect> i(*pMeasureWindowRect);

        if (m_EnableSetCameraViewPortNew) {
            CurrentRect.m_WindowRect = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRect(ROI_ID_CAMERA);
            CurrentRect.m_WindowKeyIndex = ROI_ID_CAMERA;
            ListROISortByArea.append(CurrentRect);
        } else {
            while (i.hasNext()) {
                i.next();
                if (i.key() != ROI_ID_CAMERA && i.key() != ROI_ID_MEASURE_BOTTLE_UNDER_VALVE) {
                    CurrentRect.m_WindowRect = i.value();
                    CurrentRect.m_WindowKeyIndex = i.key();
                    ListROISortByArea.append(CurrentRect);
                }
            }
            qSort(ListROISortByArea.begin(), ListROISortByArea.end(), LiveImageView::SortROIRectBYArea);
        }

        for (int i = 0; i < ListROISortByArea.count(); i++) {  // sortierte liste nach groesse des messfensters, kleinstes messfenster steht in der Z-Order ganz oben
            AOIRect = ListROISortByArea.at(i).m_WindowRect;
            m_CurrentMeasureWindowKeyNumber = ListROISortByArea.at(i).m_WindowKeyIndex;
            GetRectSupportPoints(AOIRect, TopSquareMiddelPos, BottomSquareMiddelPos, LeftSquareMiddelPos, RightSquareMiddelPos);

            if (TopSquareMiddelPos.contains(XPos, YPos)) {
                QApplication::setOverrideCursor(Qt::SizeVerCursor);
                m_CurserPosition = CURSER_POSITION_RESIZE_TOP;
                break;
            } else if (BottomSquareMiddelPos.contains(XPos, YPos)) {
                QApplication::setOverrideCursor(Qt::SizeVerCursor);
                m_CurserPosition = CURSER_POSITION_RESIZE_BOTTOM;
                break;
            } else if (LeftSquareMiddelPos.contains(XPos, YPos)) {
                QApplication::setOverrideCursor(Qt::SizeHorCursor);
                m_CurserPosition = CURSER_POSITION_RESIZE_LEFT;
                break;
            } else if (RightSquareMiddelPos.contains(XPos, YPos)) {
                QApplication::setOverrideCursor(Qt::SizeHorCursor);
                m_CurserPosition = CURSER_POSITION_RESIZE_RIGHT;
                break;
            } else if (AOIRect.contains(XPos, YPos)) {
                QApplication::setOverrideCursor(Qt::SizeAllCursor);
                m_CurserPosition = CURSER_POSITION_MOVE_IN_ROI;
                break;
            } else {
                QApplication::restoreOverrideCursor();  // SetCursor(Qt::ArrowCursor);
                m_CurserPosition = CURSER_POSITION_NOT_ON_ROI;
            }
        }
    }
}

bool LiveImageView::SortROIRectBYArea(MeasureWindowWithKeyNumber& p1, MeasureWindowWithKeyNumber& p2)
{
    return ((p1.m_WindowRect.width() * p1.m_WindowRect.height()) < (p2.m_WindowRect.width() * p2.m_WindowRect.height()));
}

void LiveImageView::MoveROIRect(QRect& ROIRect, int NewXPos, int NewYPos, double Zoom)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) {
        QRect NewROIRect;
        QRect CurrentROIRectBottleOnTriggerPos = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRect(ROI_ID_MEASURE_BOTTLE_UNDER_VALVE);
        QRect NewRectBottleOnTriggerPos;
        int DeltaX = NewXPos - static_cast<int>(m_ClickPos.x() / Zoom + 0.5);
        int DeltaY = NewYPos - static_cast<int>(m_ClickPos.y() / Zoom + 0.5);
        m_ClickPos.setX(static_cast<int>(NewXPos * Zoom + 0.5));
        m_ClickPos.setY(static_cast<int>(NewYPos * Zoom + 0.5));
        NewXPos = ROIRect.x() + DeltaX;
        NewYPos = ROIRect.y() + DeltaY;
        // grenzen überprüfen
        if (NewXPos < 0) NewXPos = 0;
        if (NewYPos < 0) NewYPos = 0;
        if ((NewXPos + ROIRect.width()) >= GetMainAppCrystalT2()->GetImageData()->GetImageWidth()) NewXPos = GetMainAppCrystalT2()->GetImageData()->GetImageWidth() - ROIRect.width();
        if ((NewYPos + ROIRect.height()) >= GetMainAppCrystalT2()->GetImageData()->GetImageHeight()) NewYPos = GetMainAppCrystalT2()->GetImageData()->GetImageHeight() - ROIRect.height();
        NewROIRect.setX(NewXPos);
        NewROIRect.setY(NewYPos);
        NewROIRect.setWidth(ROIRect.width());
        NewROIRect.setHeight(ROIRect.height());
        GetMainAppCrystalT2()->GetImageData()->SetMeasureWindowRect(m_CurrentMeasureWindowKeyNumber, NewROIRect);
        if (m_CurrentMeasureWindowKeyNumber == ROI_ID_MEASURE_LIQUID) {
            NewRectBottleOnTriggerPos.setX(NewXPos);
            NewRectBottleOnTriggerPos.setY(CurrentROIRectBottleOnTriggerPos.y());
            NewRectBottleOnTriggerPos.setHeight(CurrentROIRectBottleOnTriggerPos.height());
            NewRectBottleOnTriggerPos.setWidth(CurrentROIRectBottleOnTriggerPos.width());
            GetMainAppCrystalT2()->GetImageData()->SetMeasureWindowRect(ROI_ID_MEASURE_BOTTLE_UNDER_VALVE, NewRectBottleOnTriggerPos);
        }
        if (m_CurrentMeasureWindowKeyNumber == ROI_ID_MEASURE_SPEED) {
            NewRectBottleOnTriggerPos.setX(CurrentROIRectBottleOnTriggerPos.x());
            NewRectBottleOnTriggerPos.setY(NewYPos);
            NewRectBottleOnTriggerPos.setHeight(CurrentROIRectBottleOnTriggerPos.height());
            NewRectBottleOnTriggerPos.setWidth(CurrentROIRectBottleOnTriggerPos.width());
            GetMainAppCrystalT2()->GetImageData()->SetMeasureWindowRect(ROI_ID_MEASURE_BOTTLE_UNDER_VALVE, NewRectBottleOnTriggerPos);
        }
    }
}
