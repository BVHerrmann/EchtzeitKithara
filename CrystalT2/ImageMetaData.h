#pragma once
#include "SharedData.h"
#include "qimage.h"


class ImageMetaData
{
public:
    ImageMetaData()
    {
		m_CurrentFifoSize=0;
		m_IsImageFromVideo = false;
    }
public:
	QImage m_Image;
	int m_CurrentFifoSize;
	bool m_IsImageFromVideo;
	MeasuringResults m_CurrentMeasuringResult;
	MeasuringResults m_AveragedMeasuringResults;
	MeasuringResultsLiquid m_CurrentMeasuringResultsLiqiud;
	
};
Q_DECLARE_METATYPE(ImageMetaData);