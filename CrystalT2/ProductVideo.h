#pragma once

#include <QObject>
#include <qwidget.h>

class GraphicsVideoView;
class ProductVideoData;
class DisplayVideo;
class VideoDialog;
class ProductVideo : public QWidget
{
	Q_OBJECT
public:
	ProductVideo(int VideoNumber, VideoDialog *pVideoDialog);
	~ProductVideo();
	void SetProductVideoSettings(int ImageWidth, int ImageHeight, int BitsPerPixel,int StartFrameIndex, int NumberFrames,unsigned long long TimeStampInnns);
	int GetImageWidth() { return m_ImageWidth; }
	int GetImageHeight() { return m_ImageHeight; }
	int GetBitsPerPixel() { return m_BitsPerPixel; }
    unsigned char *GetImageStartPointer(int OffsetIndex);
	unsigned char *GetImageStartPointerByTimeOffset(long long TimeOffset);
	int GetNumberFrames() { return m_NumberFrames; }
	DisplayVideo      *GetDisplayVideo() { return m_DisplayVideo; }
	ProductVideoData  *GetProductVideoData() {return m_ProductVideoData;}
	VideoDialog       *GetVideoDialog() { return m_VideoDialog; }
	GraphicsVideoView *GetGraphicsVideoView() { return m_GraphicsVideoView; }
	void StartImagAcquisition();
	void StopImagAcquisition();
	void PlayVideo();
	void StopVideo();
	void SkipForward();
	void SkipBackward();
	void SaveVideo(QString &FileName);
	void SliderVideoFramerateChanged(int set);
	void HideControls();
	void ResetStartIndex();
    int CopyAndShowSingleImageByTimeOffset(int TimeOffset, QString& ErrorMsg, int TriggerNumber);
	//void SetDisplayZoomFactor(double set);
    void SetDisplayZoomFactorWidth(double set);
    void SetDisplayZoomFactorHeight(double set);
	void SetEnableMouseEvents(bool set);
	void DrawGreenColorFrame();
	void ClearGreenFrame();

public slots:
	void SlotShowLiveImage(const QImage &);
	void SlotSetCurrentVideoFrameNumber(int CurrentVidoFrame);
	void SlotPlayVideo();
	void SlotStopVideo();
	void SlotSkipForward();
	void SlotSkipBackward();
	void SlotSliderVideoFramerateChanged(int);
	void SlotSaveVideo(QString &PathAndName);
	void SlotAddNewDebugInfo(const QString &ErrorMsg, int InfoCode);

private:
	GraphicsVideoView *m_GraphicsVideoView;
	ProductVideoData  *m_ProductVideoData;
	DisplayVideo      *m_DisplayVideo;
	VideoDialog       *m_VideoDialog;
	int                m_ImageWidth;
	int                m_ImageHeight;
	int                m_BitsPerPixel;
	int                m_NumberFrames;
	int                m_StartFrameIndex;
	int                m_ImageCounter;
	bool               m_ActionForAll;
	unsigned long long m_TimeStampTrigger;
};
