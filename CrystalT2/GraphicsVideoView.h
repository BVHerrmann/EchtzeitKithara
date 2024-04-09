#pragma once

#include <QGraphicsView>


class VideoDialog;
class GraphicsVideoView : public QGraphicsView
{
	Q_OBJECT
public:
	GraphicsVideoView(QWidget *parent,VideoDialog  *VideoDialog=NULL);
	~GraphicsVideoView();
	void setImage(const QImage& Image, double ZoomFactorWidth, double ZoomFactorHeight);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void ClearGraphicItem(QGraphicsItem *Item);
	void DrawGreenColorFrame();
	void OpenSaveVideoDialog();
	void SetEnableMouseEvents(bool set) {m_EnableMouseEvents = set;}
	void ClearGreenFrame();

signals:
	void SignalWheelEvent(bool);

private:
	QGraphicsScene        *m_GrapicSenceLiveImage;
	QGraphicsPixmapItem   *m_Pixmap;
	QGraphicsRectItem     *m_ImageFrameItem;
	bool m_LeftMousePressed;
	int m_StartX, m_StartY;
	bool m_EnableMouseEvents;
	VideoDialog  *m_VideoDialog;
	QImage      m_LastImage;

};

