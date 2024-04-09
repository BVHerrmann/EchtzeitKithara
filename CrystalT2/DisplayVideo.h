#pragma once

#include <QWidget>
#include "ui_DisplayVideo.h"

class ProductVideo;
class DisplayVideo : public QWidget
{
	Q_OBJECT

public:
	DisplayVideo(ProductVideo *pProductVideo);
	~DisplayVideo();
	ProductVideo *GetProductVideo() {return m_ProductVideo;}
	void AddWidget(QWidget *parent);
	void ShowControls();
    void HideControls();
	void SetMaximumVideoSlider(int MaxValue);
	void SetCurrentFrameNumber(int Number);
	void SetCurrentFrameLCDNumber(int Number);
	void SetLCDFramesPerSecond(int pos);
	void SetFramesPerSecond(int pos);
	void SetAuditTrailProperties();
    void SetRequiredAccessLevel();
	
public slots:
	void SlotSliderVideoFramerateChanged(int);
	void SlotSliderVideoFrameNumberChanged(int);
	void SlotPlayVideo();
	void SlotStopVideo();
	void SlotSkipBackward();
	void SlotSkipForward();
	void SlotSaveVideo();
	void SlotGetNewVideo();

signals:
	void SignalSliderVideoFramerateChanged(int);
	void SignalSliderVideoFrameNumberChanged(int);
	void SignalPlayVideo();
	void SignalStopVideo();
	void SignalSkipBackward();
	void SignalSkipForward();
	void SignalSaveVideo(QString &);

private:
	Ui::DisplayVideo ui;
	bool m_EnableSliderSetFrameNumber;
	ProductVideo *m_ProductVideo;
};
