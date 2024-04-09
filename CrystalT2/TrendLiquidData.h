#pragma once

#include <interfaces.h>
#include "qtime"

class TrendLiquidData
{
  public:
    TrendLiquidData()
    {
        m_AmountLiquidLeftValve = 0.0;
        m_StandardDeviationAmountLiquidLeftValve = 0.0;
        m_AmountLiquidRightValve = 0.0;
        m_StandardDeviationAmountLiquidRightValve = 0.0;
        m_SumAmountLiquid = 0.0;
        m_StandardDeviationSumAmountLiquid = 0.0;
        m_SoftwareStart = false;
        m_SoftwareFinished = false;
        m_MaschineState = PluginInterface::MachineState::Off;
        m_BottlesPerMinute = 0.0;
        m_EjectBottle = true;
        m_EjectBottleValid = false;
        m_TheLeftValveIsFilledFirst = true;
    }

  public:
    double m_AmountLiquidLeftValve;
    double m_StandardDeviationAmountLiquidLeftValve;
    double m_AmountLiquidRightValve;
    double m_StandardDeviationAmountLiquidRightValve;
    double m_SumAmountLiquid;
    double m_StandardDeviationSumAmountLiquid;
    bool m_SoftwareStart;
    bool m_SoftwareFinished;
    bool m_EjectBottle;
    bool m_EjectBottleValid;
    bool m_TheLeftValveIsFilledFirst;
    int m_MaschineState;
    double m_BottlesPerMinute;
    QTime m_Time;
};