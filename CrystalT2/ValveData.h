#pragma once

class ValveData
{
public:
	double               m_ValvePulseTimeInms;
	double               m_ValvePauseTimeInms;
	double               m_ValveCloseVoltage;
	double               m_ValveStrokeInPercent;
	double               m_ValveCloseTimeInms;
	double               m_ValveOpenTimeInms;
	double               m_ValveTemperature;
	int                  m_ValveModeTemperature;
	int                  m_ValveCount;
	int                  m_ValveModus;
	int                  m_ValveID;

	ValveData& operator=(const ValveData& other)
	{
		if (this != &other)
		{
			m_ValvePulseTimeInms   = other.m_ValvePulseTimeInms;
			m_ValvePauseTimeInms   = other.m_ValvePauseTimeInms;
			m_ValveCloseVoltage    = other.m_ValveCloseVoltage;
			m_ValveStrokeInPercent = other.m_ValveStrokeInPercent;
			m_ValveCloseTimeInms   = other.m_ValveCloseTimeInms;
			m_ValveOpenTimeInms    = other.m_ValveOpenTimeInms;
			m_ValveCount           = other.m_ValveCount;
			m_ValveModus           = other.m_ValveModus;
			m_ValveTemperature     = other.m_ValveTemperature;
			m_ValveModeTemperature = other.m_ValveModeTemperature;
			m_ValveID              = other.m_ValveID;
		}
		return *this;
	}
};
