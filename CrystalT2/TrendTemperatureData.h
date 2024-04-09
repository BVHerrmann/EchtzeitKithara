#pragma once

#include "qtime"

class TrendTemperatureData
{
  public:
    TrendTemperatureData()
    {
        m_StackTemperatureLeftValve = 0.0;
        m_CurrentTemperatureLeftValve = 0.0;
        m_StackTemperatureRightValve = 0.0;
        m_CurrentTemperatureRightValve = 0.0;
        m_CurrentPreasureTankTemperature = 0.0;
        m_CurrentHeatingPipeTemperature = 0.0;
        m_CurrentWaterCoolingTemperature = 0.0;
        m_SoftwareStart = false;
        m_SoftwareFinished = false;
    }

  public:
    double m_StackTemperatureLeftValve;
    double m_CurrentTemperatureLeftValve;
    double m_StackTemperatureRightValve;
    double m_CurrentTemperatureRightValve;
    double m_CurrentPreasureTankTemperature;
    double m_CurrentHeatingPipeTemperature;
    double m_CurrentWaterCoolingTemperature;
    bool m_SoftwareStart;
    bool m_SoftwareFinished;
    QTime m_Time;
};