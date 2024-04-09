#pragma once
#include <list> 
#include "KrtsBertram.h"
#include "SharedData.h"


class TimeStampsSetDO
{
public:
	TimeStampsSetDO()
	{
		m_TimeStamp = 0;
		m_SetOn = false;
		m_ChannelIndex = EtherCATConfigData::DO_CAMERA_LIGHT;
	}
public:
	unsigned long long             m_TimeStamp;
	bool                           m_SetOn;
	EtherCATConfigData::IOChannels m_ChannelIndex;
};


class ImageData;
class ExchangeMemory;
class ProgramLogicController
{
public:
	ProgramLogicController(ImageData *pImageData);
	~ProgramLogicController();

	void RunPLC(ExchangeMemory* SharedData, unsigned long long TimeStamp);
	bool GetDigitalIOState(ExchangeMemory* SharedData, EtherCATConfigData::IOChannels Channel);
	void SetDigitalOutput(ExchangeMemory* SharedData, bool Value, EtherCATConfigData::IOChannels Channel);
	void CalculateSpeedFromIS(ExchangeMemory* SharedData, unsigned long long TimeStamp);
	void TriggerTimeReached(ExchangeMemory* SharedData, unsigned long long TimeStamp);
	void EjectionTimeReached(ExchangeMemory* SharedData, unsigned long long TimeStamp);
	void SetDigitalOutputTimeReached(ExchangeMemory* SharedData, unsigned long long TimeStamp);
	//void CalculateTriggeringPointInTimeSetBottelEjectionOff(ExchangeMemory* SharedData, unsigned long long TimeStamp);
	//void SetDebugInfo(ExchangeMemory* SharedData, std::string DebugString);
	ImageData *GetImageData() { return m_ImageData; }
	void DebugFormat(const char* format, ...);
	void GetCurrentTimeStampIn100nsUnits(ExchangeMemory* SharedData,unsigned long long &TimeStamp);

private:
	ImageData *m_ImageData;
	std::list< TimeStampsSetDO > m_ListSetDigtalOutputs;               //FIFO 
	std::list< TimeStampsSetDO > m_ListDigtalOutputsWhiteLight;
	bool       m_LastClockSignaleFromIS;
	bool       m_LastStateEjectionControl;
	unsigned long long m_LastTimeStamp;
	unsigned long long m_Counter;

};

