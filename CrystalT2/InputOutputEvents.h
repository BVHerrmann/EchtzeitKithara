#pragma once

#include <QThread>
#include <Qtcore>

#include <boost/accumulators/framework/accumulator_set.hpp>
#include <boost/accumulators/framework/extractor.hpp>
#include <boost/accumulators/statistics/rolling_mean.hpp>
//#include <boost/accumulators/statistics/rolling_variance.hpp>

using namespace boost::accumulators;

class KitharaCore;
class MainAppCrystalT2;
class InputOutputEvents : public QThread
{
    Q_OBJECT
  public:
    InputOutputEvents(MainAppCrystalT2* pMainAppCrystalT2);
    ~InputOutputEvents();
    virtual void run();
    KitharaCore* GetKitharaCore();
    MainAppCrystalT2* GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
    void FinishedThread();
    void ShowIsControlPowerOn();
    void ShowSpeedFromISMaschine();
    void ShowPreasure();
    void ShowFillingLevel();
    void ShowAirCoolingCamera();
    void ShowlAirCoolingCameraLight();
    void ShowlAirCoolingValves();
    void ShowWaterCooling();
    void ShowFlowTransmitterWaterCoolingCircuit();
    void ShowTemperaturWaterCoolingReturn();
    void ShowPreasureTankTemperature();
    void ShowHeatingPipeTemperature();
    void ShowCounterNumberBottlesRealEjected();
    void ShowStatusMixer();
    double GetMixerSpeed();

  signals:
    void SignalShowInfo(const QString& InfoData, int InfoType);
    void SignalShowDebugInfo(const QString& InfoData, int InfoType);

  private:
    bool m_TerminateEventsInpuOutput;
    QMutex m_MutexForEventInputOutput;
    QWaitCondition m_WaitConditionEventInputOutput;
    MainAppCrystalT2* m_MainAppCrystalT2;
    accumulator_set<int32_t, stats<tag::rolling_sum> > *m_counter_sum;
    accumulator_set<int64_t, stats<tag::rolling_sum> > *m_delay_sum;
    uint32_t m_previousInternalPosition;
    int64_t m_previousTimestampInternalPositionInms;
};
