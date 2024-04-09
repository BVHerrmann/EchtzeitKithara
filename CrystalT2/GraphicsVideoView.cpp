#include "GraphicsVideoView.h"
#include <colors.h>
#include <QWheelEvent>
#include "GlobalConst.h"
#include "SaveVideoDialog.h"
#include "qgraphicsitem.h"
#include "VideoDialog.h"
#include "MainAppCrystalT2.h"

GraphicsVideoView::GraphicsVideoView(QWidget* parent, VideoDialog* pVideoDialog)
    : QGraphicsView(parent), m_LeftMousePressed(false), m_EnableMouseEvents(false), m_ImageFrameItem(NULL),  m_Pixmap(NULL)
{
    m_VideoDialog = pVideoDialog;
    m_GrapicSenceLiveImage = new QGraphicsScene(this);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setScene(m_GrapicSenceLiveImage);
}

GraphicsVideoView::~GraphicsVideoView()
{
}

void GraphicsVideoView::OpenSaveVideoDialog()
{
    if (m_Pixmap && m_VideoDialog && m_VideoDialog->GetMainAppCrystalT2()) {
        m_VideoDialog->GetMainAppCrystalT2()->OpenSaveTriggerImageManual(m_LastImage, m_Pixmap->pixmap());
    }
}

void GraphicsVideoView::setImage(const QImage& Image, double ZoomFactorWidth, double ZoomFactorHeight)
{
    m_LastImage = Image;
    int w = Image.width();
    int h = Image.height();
    if (ZoomFactorWidth != 1.0 && ZoomFactorHeight != 1.0) {
        w = static_cast<int>(w * ZoomFactorWidth);
        h = static_cast<int>(h * ZoomFactorHeight);
        if (m_Pixmap == NULL) {
            m_Pixmap = m_GrapicSenceLiveImage->addPixmap(QPixmap(w, h));
        }
        m_Pixmap->setPixmap(QPixmap::fromImage(Image.scaled(w, h)));
       
    } else {
        if (m_Pixmap == NULL) {
            m_Pixmap = m_GrapicSenceLiveImage->addPixmap(QPixmap(w, h));
        }
        m_Pixmap->setPixmap(QPixmap::fromImage(Image));

    }
    setSceneRect(m_Pixmap->boundingRect());
    ClearGraphicItem(m_ImageFrameItem);
}

void GraphicsVideoView::DrawGreenColorFrame()
{
    if (m_Pixmap) {
        QRectF CurrentRect = m_Pixmap->boundingRect();
        QPen PenColor(HMIColor::green);
        PenColor.setWidth(7);
        ClearGraphicItem(m_ImageFrameItem);
        m_ImageFrameItem = m_GrapicSenceLiveImage->addRect(CurrentRect, PenColor);
    }
}

void GraphicsVideoView::ClearGreenFrame()
{
    ClearGraphicItem(m_ImageFrameItem);
    m_ImageFrameItem = NULL;
}

void GraphicsVideoView::mousePressEvent(QMouseEvent* ev)
{
    if (m_EnableMouseEvents && ev->button() == Qt::LeftButton && !m_LastImage.isNull()) {
        OpenSaveVideoDialog();
        return;
    }

    QGraphicsView::mousePressEvent(ev);
}

void GraphicsVideoView::mouseReleaseEvent(QMouseEvent* ev)
{
    QGraphicsView::mouseReleaseEvent(ev);
}

void GraphicsVideoView::mouseMoveEvent(QMouseEvent* ev)
{
    QGraphicsView::mouseMoveEvent(ev);
}

/*void GraphicsVideoView::wheelEvent(QWheelEvent *event)
{
        if (event->delta() > 0)
        {
                emit SignalWheelEvent(true);
        }
        else
        {
                emit SignalWheelEvent(false);
        }
}
*/

void GraphicsVideoView::ClearGraphicItem(QGraphicsItem* Item)
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
