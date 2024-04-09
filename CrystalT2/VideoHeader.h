#pragma once
#include "SharedData.h"

class VideoHeader
{
public:
	VideoHeader()
	{
		m_FrameIndex           = 0;
		m_ImageWidth           = 0;
		m_ImageHeight          = 0;
		m_MaxNumberFrames      = 0;
		m_CurrentNumberFrames  = 0;
		m_MAXVideoBockSize     = 0;
		m_BitsPerPixel         = 8;
		m_PixelFormat          = 1;//Mono8
		m_ProductPressentTime  = 0.0;
		m_CurrentTimeStampInns = 0;
		m_LastTimeStampInns    = 0;
		m_CurrentSpeedInmmPerms=-1.0;
		memset(m_ImageIndexIsCopied, 0, 32768);// MAX_IMAGES_IN_VIDEO_BUFFER);
        m_EnableWriteVideoData=true;
	}
public:
	double             m_ProductPressentTime;
	double             m_CurrentSpeedInmmPerms;
	int                m_ImageWidth;
	int                m_ImageHeight;
	int                m_BitsPerPixel;
	int                m_PixelFormat;
	unsigned __int64   m_FrameIndex;
	unsigned __int64   m_CurrentNumberFrames;
	unsigned __int64   m_MaxNumberFrames;
	unsigned __int64   m_MAXVideoBockSize;
	unsigned long long m_CurrentTimeStampInns;
	unsigned long long m_LastTimeStampInns;
	unsigned char      m_ImageIndexIsCopied[32768];// [MAX_IMAGES_IN_VIDEO_BUFFER];
    bool               m_EnableWriteVideoData;
};

class ImageHeader
{
public:
	ImageHeader()
	{
		m_ImageState    = 0x00;
		m_TimeStampInns = 0;
        m_CameraImageID = 0;
        m_ProductionOn = false;
        m_CurrentMeasuringResultsLiqiud.ClearResults();
	}
public:
	unsigned char m_ImageState;
	unsigned long long m_TimeStampInns;
    unsigned long long m_CameraImageID;
    bool m_ProductionOn;
    MeasuringResultsLiquid m_CurrentMeasuringResultsLiqiud;

};
