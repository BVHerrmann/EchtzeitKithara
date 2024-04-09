#pragma once

#include <QWidget>
#include "ui_AdvancedSettingsDialog.h"

class MainAppCrystalT2;
class ControlsWithColorStatus;
class AdvancedSettingsDialog : public QWidget
{
    Q_OBJECT
  public:
    AdvancedSettingsDialog(MainAppCrystalT2* parent = Q_NULLPTR);
    ~AdvancedSettingsDialog();
    MainAppCrystalT2* GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
    void showEvent(QShowEvent*);
    void SetMeasuredPixelSize(double set, bool Status);
    void SetMeasuredNeckDiameter(double set, bool Status);
    void SetTriggerState(int State, int ValveID);
    void SetAuditTrailProperties();
    void SetNewPixelSize();
    void SetForceEditingFinishedPixelSize();
    void SetSpeed(double set);
    void SetSpeedFromIS(double set);
    void SetDeltaSpeed(double set);
    void SetProductPresentTime(double set);
    void SetInspectionTime(double set);
    void SetRealTimeInterval(double set);
    double GetSpeedFromIS();
    double GetSpeed();

    int GetStatusSpeedDeviationBetweenCameraAndIS() { return m_StatusSpeedDeviationBetweenCameraAndIS; }

  public slots:
    void SlotExposureTimeChanged();
    void SlotDistanceCameraProductChanged();
    void SlotSetMaxVideosOnScreen(int index);
    void SlotMinSpeedChanged();
    void SlotThresholBinaryImageLiquidChanged();
    void SlotAverageCounterDiameterAndPixelSizeChanged();
    void SlotSetMinNumberFoundedInROI();
    void SlotMaxMeasurementsProductIsOutOfTol();
    void SlotThresholBinaryImageDegreeOfPollutionChanged();
    void SlotDistancesBetweenValvesChanged();
    void SlotBackgroundContrastChanged();
    void SlotMinROIMeasureWindowHeightChanged();
    void SlotRollingMeanValueLiquid();
    void SlotNumIterationROIPosition();
    void SlotNumStepsVariatePosAndWidth();
    void SlotNumIterationAccseptanceThreshold();
    void SlotFactorWidthSearchBottleTopLine();
    void SlotNumIterationCalculatePixelSize();
    void SlotThresholdFactor();
    void SlotMinContrastInPercent();
    void SlotStartValueROIHeight();
    void SlotDiameterTolInPercentAutoCalibrate();
    void SlotBottleBaseLineOffsetInPix();
    void SlotGetDebugCount();
    void SlotNumIterationROIYPos();
    void SlotApplyNewPixelSize();
    void SlotPixelSizeChanged();
    void SlotSaveIntervalShowTrendGraph();
    void SlotIntervalCheckCleanImageChanged();
    void SlotRollingMeanBottlesPerMinChanged();
    void SlotPreasureIncreaseWhenFlushingChanged();
    void SlotDefineNoPreasureValueChanged();
    void SlotWaterCoolingPIDMinStrokeChanged();
    void SlotWaterCoolingPIDMaxStrokeChanged();
    void SlotWaterCoolingPFactorChanged();
    void SlotWaterCoolingIFactorChanged();
    void SlotWaterCoolingDFactorChanged();

  private:
    Ui::AdvancedSettingsDialog ui;
    MainAppCrystalT2* m_MainAppCrystalT2;
    int m_CounterShowPixelSize;
    int m_NumberSameMeasureValues;
    int m_CounterShowNeckDiameter;
    int m_StatusSpeedDeviationBetweenCameraAndIS;
    bool m_WindowSetup;
    ControlsWithColorStatus* m_SpeedDeviationBetweenCameraAndIS;
};
