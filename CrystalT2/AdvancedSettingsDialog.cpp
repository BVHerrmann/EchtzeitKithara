#include "AdvancedSettingsDialog.h"
#include "ControlsWithColorStatus.h"
#include "GlobalConst.h"
#include "ImageData.h"
#include "KitharaCore.h"
#include "MainAppCrystalT2.h"
#include "ProductData.h"
#include "SettingsData.h"
#include "bmessagebox.h"
#include "colors.h"

#include <audittrail.h>

AdvancedSettingsDialog::AdvancedSettingsDialog(MainAppCrystalT2* pMainAppCrystalT2)
    : QWidget(pMainAppCrystalT2),
      m_WindowSetup(false),
      m_MainAppCrystalT2(NULL),
      m_CounterShowPixelSize(0),
      m_SpeedDeviationBetweenCameraAndIS(NULL),
      m_NumberSameMeasureValues(40),
      m_CounterShowNeckDiameter(0),
      m_StatusSpeedDeviationBetweenCameraAndIS(ALARM_LEVEL_OK)
{
    ui.setupUi(this);
    m_MainAppCrystalT2 = pMainAppCrystalT2;

    ui.comboBoxMaxTriggerPosOnScreen->insertItem(0, QString("1"));
    ui.comboBoxMaxTriggerPosOnScreen->insertItem(1, QString("2"));
    ui.comboBoxMaxTriggerPosOnScreen->insertItem(2, QString("4"));
    ui.comboBoxMaxTriggerPosOnScreen->insertItem(3, QString("8"));
    ui.comboBoxMaxTriggerPosOnScreen->insertItem(4, QString("12"));
    ui.comboBoxMaxTriggerPosOnScreen->insertItem(5, QString("16"));
    ui.comboBoxMaxTriggerPosOnScreen->insertItem(6, QString("20"));
    ui.comboBoxMaxTriggerPosOnScreen->insertItem(7, QString("25"));
    ui.comboBoxMaxTriggerPosOnScreen->insertItem(8, QString("36"));
    ui.comboBoxMaxTriggerPosOnScreen->insertItem(9, QString("42"));

    connect(ui.pushButtonApplyNewPixelSize, &QPushButton::clicked, this, &AdvancedSettingsDialog::SlotApplyNewPixelSize);
    connect(ui.doubleSpinBoxUsedPixelSize, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotPixelSizeChanged);

    SetAuditTrailProperties();
    ui.doubleSpinBoxMinimalROIHeight->setMinimum(MINIMUM_ROI_SIZE_IN_PIXEL);

    connect(ui.doubleSpinBoxIntervalCheckCleanImage, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotIntervalCheckCleanImageChanged);
    connect(ui.doubleSpinBoxIntervalSaveTempartureData, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotSaveIntervalShowTrendGraph);

    connect(ui.doubleSpinBoxBackgroundContrast, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotBackgroundContrastChanged);
    connect(ui.doubleSpinBoxMinimalROIHeight, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotMinROIMeasureWindowHeightChanged);
    connect(ui.doubleSpinBoxExposureTime, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotExposureTimeChanged);
    connect(ui.doubleSpinBoxDistanceCameraProduct, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotDistanceCameraProductChanged);
    connect(ui.doubleSpinBoxMinSpeed, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotMinSpeedChanged);
    connect(ui.doubleSpinBoxThresholBinaryImageLiquid, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotThresholBinaryImageLiquidChanged);
    connect(ui.doubleSpinBoxAverageCounterDiameterAndPixelSize, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotAverageCounterDiameterAndPixelSizeChanged);
    connect(ui.doubleSpinBoxMinNumberFoundedInGreenROI, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotSetMinNumberFoundedInROI);
    connect(ui.doubleSpinBoxThresholBinaryImageDegreeOfPollution, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotThresholBinaryImageDegreeOfPollutionChanged);
    connect(ui.doubleSpinBoxMaxMeasurementsProductIsOutOfTol, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotMaxMeasurementsProductIsOutOfTol);
    connect(ui.doubleSpinBoxRollingMeanValueLiquid, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotRollingMeanValueLiquid);
    connect(ui.comboBoxMaxTriggerPosOnScreen, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AdvancedSettingsDialog::SlotSetMaxVideosOnScreen);
    connect(ui.doubleSpinBoxDistancesBetweenValves, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotDistancesBetweenValvesChanged);
    connect(ui.doubleSpinBoxNumIterationROIPosition, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotNumIterationROIPosition);
    connect(ui.doubleSpinBoxNumIterationAccseptanceThreshold, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotNumIterationAccseptanceThreshold);
    connect(ui.doubleSpinBoxFactorWidthSearchBottleTopLine, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotFactorWidthSearchBottleTopLine);
    connect(ui.doubleSpinBoxNumIterationCalculatePixelSize, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotNumIterationCalculatePixelSize);
    connect(ui.doubleSpinBoxThresholdFactor, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotThresholdFactor);
    connect(ui.doubleSpinBoxMinContrastInPercent, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotMinContrastInPercent);
    connect(ui.doubleSpinBoxStartValueROIHeight, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotStartValueROIHeight);
    connect(ui.doubleSpinBoxDiameterToleranceInPercent, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotDiameterTolInPercentAutoCalibrate);
    connect(ui.doubleSpinBoxBottleBaseLineOffsetInPix, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotBottleBaseLineOffsetInPix);
    connect(ui.doubleSpinBoxNumIterationROIYPos, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotNumIterationROIYPos);
    connect(ui.doubleSpinBoxNumIterartionROIHeight, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotNumStepsVariatePosAndWidth);
    connect(ui.doubleSpinBoxRollingMeanBottlesPerMin, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotRollingMeanBottlesPerMinChanged);
    connect(ui.doubleSpinBoxPreasureIncreaseWhenFlushing, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotPreasureIncreaseWhenFlushingChanged);
    connect(ui.doubleSpinBoxDefineNoPreasureValue, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotDefineNoPreasureValueChanged);

    connect(ui.doubleSpinBoxWaterCoolingPIDMinStroke, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotWaterCoolingPIDMinStrokeChanged);
    connect(ui.doubleSpinBoxWaterCoolingPIDMaxStroke, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotWaterCoolingPIDMaxStrokeChanged);
    connect(ui.doubleSpinBoxWaterCoolingPFactor, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotWaterCoolingPFactorChanged);
    connect(ui.doubleSpinBoxWaterCoolingIFactor, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotWaterCoolingIFactorChanged);
    connect(ui.doubleSpinBoxWaterCoolingDFactor, &QDoubleSpinBox::editingFinished, this, &AdvancedSettingsDialog::SlotWaterCoolingDFactorChanged);

    connect(ui.pushButtonGetDebugCount, &QPushButton::clicked, this, &AdvancedSettingsDialog::SlotGetDebugCount);
    //ui.ContentBoardGetDebugCount->hide();
    ui.ContentBoardStartValueROIHeight->hide();
    ui.groupBoxTriggerStatusInfo->hide();
    ui.widgetDashBoardPreasureIncreaseWhenFlushing->hide();//is under adminsettingsdialog

    bool SmallerAsThreshold = false;
    m_SpeedDeviationBetweenCameraAndIS = new ControlsWithColorStatus(ui.doubleSpinBoxDeltaSpeed, SmallerAsThreshold);
}

AdvancedSettingsDialog::~AdvancedSettingsDialog()
{
}

void AdvancedSettingsDialog::SetAuditTrailProperties()
{
    ui.doubleSpinBoxIntervalCheckCleanImage->setProperty(kAuditTrail, ui.labelContentBoardIntervalCheckCleanImage->text());
    ui.doubleSpinBoxIntervalSaveTempartureData->setProperty(kAuditTrail, ui.labelContentBoardIntervalSaveTempartureData->text());
    ui.comboBoxMaxTriggerPosOnScreen->setProperty(kAuditTrail, ui.labelMaxTriggerPosOnScreen->text());
    ui.pushButtonApplyNewPixelSize->setProperty(kAuditTrail, ui.pushButtonApplyNewPixelSize->text());
    ui.doubleSpinBoxUsedPixelSize->setProperty(kAuditTrail, ui.labelContentBoardUsedPixelSize->text());
    ui.doubleSpinBoxBackgroundContrast->setProperty(kAuditTrail, ui.labelBackgroundContrast->text());
    ui.doubleSpinBoxMinimalROIHeight->setProperty(kAuditTrail, ui.labelMinROIHeight->text());
    ui.doubleSpinBoxExposureTime->setProperty(kAuditTrail, ui.labelExposureTime->text());
    ui.doubleSpinBoxDistanceCameraProduct->setProperty(kAuditTrail, ui.labelDistanceCameraProduct->text());
    ui.doubleSpinBoxMinSpeed->setProperty(kAuditTrail, ui.labelMinSpeed->text());
    ui.doubleSpinBoxThresholBinaryImageLiquid->setProperty(kAuditTrail, ui.labelThresholBinaryImageLiquid->text());
    ui.doubleSpinBoxAverageCounterDiameterAndPixelSize->setProperty(kAuditTrail, ui.labelAverageCounterDiameterAndPixelSize->text());
    ui.doubleSpinBoxMinNumberFoundedInGreenROI->setProperty(kAuditTrail, ui.labelMinNumberFoubdInGreenROI->text());
    ui.doubleSpinBoxThresholBinaryImageDegreeOfPollution->setProperty(kAuditTrail, ui.labelThresholBinaryImageDegreeOfPollution->text());
    ui.doubleSpinBoxMaxMeasurementsProductIsOutOfTol->setProperty(kAuditTrail, ui.labelMaxMeasurementsProductIsOutOfTol->text());
    ui.doubleSpinBoxRollingMeanValueLiquid->setProperty(kAuditTrail, ui.labelRollingMeanValueLiquid->text());
    ui.doubleSpinBoxDistancesBetweenValves->setProperty(kAuditTrail, ui.labelDistancesBetweenValves->text());
    ui.doubleSpinBoxNumIterationROIPosition->setProperty(kAuditTrail, ui.labelNumIterationROIPosition->text());
    ui.doubleSpinBoxNumIterationAccseptanceThreshold->setProperty(kAuditTrail, ui.labelNumIterationAccseptanceThreshold->text());
    ui.doubleSpinBoxFactorWidthSearchBottleTopLine->setProperty(kAuditTrail, ui.labelFactorWidthSearchBottleTopLine->text());
    ui.doubleSpinBoxNumIterationCalculatePixelSize->setProperty(kAuditTrail, ui.labelNumIterationCalculatePixelSize->text());
    ui.doubleSpinBoxThresholdFactor->setProperty(kAuditTrail, ui.labelThresholdFactor->text());
    ui.doubleSpinBoxMinContrastInPercent->setProperty(kAuditTrail, ui.labelMinContrastInPercent->text());
    ui.doubleSpinBoxStartValueROIHeight->setProperty(kAuditTrail, ui.labelStartValueROIHeight->text());
    ui.doubleSpinBoxDiameterToleranceInPercent->setProperty(kAuditTrail, ui.labelDiameterTolerance->text());
    ui.doubleSpinBoxBottleBaseLineOffsetInPix->setProperty(kAuditTrail, ui.labelBottleBaseLineOffsetInPix->text());
    ui.doubleSpinBoxNumIterationROIYPos->setProperty(kAuditTrail, ui.labelNumIterationROIYPos->text());
    ui.doubleSpinBoxNumIterartionROIHeight->setProperty(kAuditTrail, ui.labelIterationROIHeight->text());
    ui.doubleSpinBoxRollingMeanBottlesPerMin->setProperty(kAuditTrail, ui.labelRollingMeanBottlesPerMin->text());
   // ui.doubleSpinBoxPreasureIncreaseWhenFlushing->setProperty(kAuditTrail, ui.labelPreasureIncreaseWhenFlushing->text());

    ui.pushButtonApplyNewPixelSize->setProperty(kAuditTrail, ui.pushButtonApplyNewPixelSize->text());
    ui.doubleSpinBoxUsedPixelSize->setProperty(kAuditTrail, ui.labelContentBoardUsedPixelSize->text());

    ui.doubleSpinBoxWaterCoolingPIDMinStroke->setProperty(kAuditTrail, ui.labelWaterCoolingPIDMinStroke->text());
    ui.doubleSpinBoxWaterCoolingPIDMaxStroke->setProperty(kAuditTrail, ui.labelWaterCoolingPIDMaxStroke->text());
    ui.doubleSpinBoxWaterCoolingPFactor->setProperty(kAuditTrail, ui.labelWaterCoolingPFactor->text());
    ui.doubleSpinBoxWaterCoolingIFactor->setProperty(kAuditTrail, ui.labelWaterCoolinIPFactor->text());
    ui.doubleSpinBoxWaterCoolingDFactor->setProperty(kAuditTrail, ui.labelWaterCoolinIDFactor->text());
}

void AdvancedSettingsDialog::SlotWaterCoolingPIDMinStrokeChanged()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_WaterCoolingStrokeMinValue = ui.doubleSpinBoxWaterCoolingPIDMinStroke->value();
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void AdvancedSettingsDialog::SlotWaterCoolingPIDMaxStrokeChanged()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_WaterCoolingStrokeMaxValue = ui.doubleSpinBoxWaterCoolingPIDMaxStroke->value();
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void AdvancedSettingsDialog::SlotWaterCoolingPFactorChanged()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_WaterCoolingPFactor = ui.doubleSpinBoxWaterCoolingPFactor->value();
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void AdvancedSettingsDialog::SlotWaterCoolingIFactorChanged()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_WaterCoolingIFactor = ui.doubleSpinBoxWaterCoolingIFactor->value();
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void AdvancedSettingsDialog::SlotWaterCoolingDFactorChanged()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_WaterCoolingDFactor = ui.doubleSpinBoxWaterCoolingDFactor->value();
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void AdvancedSettingsDialog::SlotDefineNoPreasureValueChanged()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_DefineNoPreasureValue = ui.doubleSpinBoxDefineNoPreasureValue->value();
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void AdvancedSettingsDialog::SlotPreasureIncreaseWhenFlushingChanged()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_PreasureIncreaseWhenFlushing = ui.doubleSpinBoxPreasureIncreaseWhenFlushing->value();
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void AdvancedSettingsDialog::SlotGetDebugCount()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) {
        int rv = GetMainAppCrystalT2()->GetImageData()->GetDebugCounter();
        ui.doubleSpinBoxDebugCount->setValue(rv);
    }
}

void AdvancedSettingsDialog::SlotRollingMeanValueLiquid()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_RollingMeanValueLiquid = ui.doubleSpinBoxRollingMeanValueLiquid->value();
            GetMainAppCrystalT2()->SaveSettings();
            GetMainAppCrystalT2()->GetImageData()->SetRollingMeanValueLiquid(pSettingsData->m_RollingMeanValueLiquid);
        }
    }
}

void AdvancedSettingsDialog::SlotNumIterationROIPosition()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_NumIterationCalculteROIPosition = ui.doubleSpinBoxNumIterationROIPosition->value();
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void AdvancedSettingsDialog::SlotNumStepsVariatePosAndWidth()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_NumIterationROIHeight = ui.doubleSpinBoxNumIterartionROIHeight->value();
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void AdvancedSettingsDialog::SlotNumIterationROIYPos()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_NumIterationROIYPos = ui.doubleSpinBoxNumIterationROIYPos->value();
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void AdvancedSettingsDialog::SlotNumIterationAccseptanceThreshold()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_NumIterationCalculateAccseptanceThreshold = ui.doubleSpinBoxNumIterationAccseptanceThreshold->value();
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void AdvancedSettingsDialog::SlotFactorWidthSearchBottleTopLine()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_FactorMeasureWindowWidthSearchBottleTopPosiition = ui.doubleSpinBoxFactorWidthSearchBottleTopLine->value();
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void AdvancedSettingsDialog::SlotNumIterationCalculatePixelSize()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_NumIterationCalculatePixelSize = ui.doubleSpinBoxNumIterationCalculatePixelSize->value();
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void AdvancedSettingsDialog::SlotThresholdFactor()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_FactorThreshold = ui.doubleSpinBoxThresholdFactor->value();
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void AdvancedSettingsDialog::SlotMinContrastInPercent()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_MinPossibleContrastValueInPercent = ui.doubleSpinBoxMinContrastInPercent->value();
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void AdvancedSettingsDialog::SlotDiameterTolInPercentAutoCalibrate()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_DiameterTolInPercentAutoCalibrate = ui.doubleSpinBoxDiameterToleranceInPercent->value();
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void AdvancedSettingsDialog::SlotStartValueROIHeight()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_StartValueMeasureWindowHeightInPixel = ui.doubleSpinBoxStartValueROIHeight->value();
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void AdvancedSettingsDialog::SlotBottleBaseLineOffsetInPix()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_BottleBaseLineOffsetInPix = ui.doubleSpinBoxBottleBaseLineOffsetInPix->value();
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void AdvancedSettingsDialog::SlotMinROIMeasureWindowHeightChanged()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_MinMeasureWindowHeight = ui.doubleSpinBoxMinimalROIHeight->value();
            GetMainAppCrystalT2()->SaveSettings();
            GetMainAppCrystalT2()->GetImageData()->SetDistancesBetweenValves(pSettingsData->m_MinMeasureWindowHeight);
        }
    }
}

void AdvancedSettingsDialog::SlotDistancesBetweenValvesChanged()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_DistancesBetweenValves = ui.doubleSpinBoxDistancesBetweenValves->value();
            GetMainAppCrystalT2()->SaveSettings();
            GetMainAppCrystalT2()->GetImageData()->SetDistancesBetweenValves(pSettingsData->m_DistancesBetweenValves);
        }
    }
}

void AdvancedSettingsDialog::SlotMinSpeedChanged()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_MinSpeedInMMPerMs = ui.doubleSpinBoxMinSpeed->value() / 60.0;
            GetMainAppCrystalT2()->SaveSettings();
            GetMainAppCrystalT2()->GetImageData()->SetMinSpeedInMMPerMs(pSettingsData->m_MinSpeedInMMPerMs);
        }
    }
}

void AdvancedSettingsDialog::SlotMaxMeasurementsProductIsOutOfTol()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_MaxMeasurementsProductIsOutOfTol = ui.doubleSpinBoxMaxMeasurementsProductIsOutOfTol->value();
            GetMainAppCrystalT2()->SaveSettings();
            GetMainAppCrystalT2()->GetImageData()->SetMaxMeasurementsProductIsOutOfTol(pSettingsData->m_MaxMeasurementsProductIsOutOfTol);
        }
    }
}

void AdvancedSettingsDialog::SlotThresholBinaryImageLiquidChanged()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_ThresholdBinaryImageLiquid = static_cast<int>(ui.doubleSpinBoxThresholBinaryImageLiquid->value());
            GetMainAppCrystalT2()->SaveSettings();
            GetMainAppCrystalT2()->GetImageData()->SetThresholdBinaryImageLiquid(pSettingsData->m_ThresholdBinaryImageLiquid);
        }
    }
}

void AdvancedSettingsDialog::SlotThresholBinaryImageDegreeOfPollutionChanged()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_ThresholdBinaryImageDegreeOfPollution = static_cast<int>(ui.doubleSpinBoxThresholBinaryImageDegreeOfPollution->value());
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void AdvancedSettingsDialog::SlotAverageCounterDiameterAndPixelSizeChanged()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_NumberProductsAverageBottleNeckAndPixelSize = ui.doubleSpinBoxAverageCounterDiameterAndPixelSize->value();
            GetMainAppCrystalT2()->SaveSettings();
            GetMainAppCrystalT2()->GetImageData()->SetNumberProductsAverageBottleNeckAndPixelSize(pSettingsData->m_NumberProductsAverageBottleNeckAndPixelSize);
        }
    }
}

void AdvancedSettingsDialog::SlotSetMinNumberFoundedInROI()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_MinNumberFoundedInROI = ui.doubleSpinBoxMinNumberFoundedInGreenROI->value();
            GetMainAppCrystalT2()->SaveSettings();
            GetMainAppCrystalT2()->GetImageData()->SetMinNumberFoundedInROI(pSettingsData->m_MinNumberFoundedInROI);
        }
    }
}
void AdvancedSettingsDialog::SlotRollingMeanBottlesPerMinChanged()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_TrendGraphRollingMeanSizeBottlesPerMin = ui.doubleSpinBoxRollingMeanBottlesPerMin->value();
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void AdvancedSettingsDialog::showEvent(QShowEvent*)
{
    m_WindowSetup = false;
    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();

    if (pSettingsData) {
        ui.doubleSpinBoxIntervalCheckCleanImage->setValue(pSettingsData->m_TimerIntervalCheckCleanImageInMin);
        ui.doubleSpinBoxIntervalSaveTempartureData->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_IntervalUpdateTrendGraph);
        ui.doubleSpinBoxUsedPixelSize->setValue(pSettingsData->m_PixelSize);
        ui.doubleSpinBoxExposureTime->setValue(pSettingsData->m_ExposureTime);
        ui.doubleSpinBoxMinSpeed->setValue(pSettingsData->m_MinSpeedInMMPerMs * 60.0);  // in m/min
        ui.doubleSpinBoxThresholBinaryImageLiquid->setValue(pSettingsData->m_ThresholdBinaryImageLiquid);
        ui.doubleSpinBoxAverageCounterDiameterAndPixelSize->setValue(pSettingsData->m_NumberProductsAverageBottleNeckAndPixelSize);
        ui.doubleSpinBoxMinNumberFoundedInGreenROI->setValue(pSettingsData->m_MinNumberFoundedInROI);
        ui.doubleSpinBoxMaxMeasurementsProductIsOutOfTol->setValue(pSettingsData->m_MaxMeasurementsProductIsOutOfTol);
        ui.doubleSpinBoxThresholBinaryImageDegreeOfPollution->setValue(pSettingsData->m_ThresholdBinaryImageDegreeOfPollution);
        ui.doubleSpinBoxBackgroundContrast->setValue(pSettingsData->m_BackgroundContrast);
        ui.doubleSpinBoxMinimalROIHeight->setValue(pSettingsData->m_MinMeasureWindowHeight);
        ui.doubleSpinBoxRollingMeanValueLiquid->setValue(pSettingsData->m_RollingMeanValueLiquid);
        ui.doubleSpinBoxDistanceCameraProduct->setValue(pSettingsData->m_DistanceCameraProduct);
        ui.doubleSpinBoxDistancesBetweenValves->setValue(pSettingsData->m_DistancesBetweenValves);
        ui.doubleSpinBoxNumIterationROIPosition->setValue(pSettingsData->m_NumIterationCalculteROIPosition);
        ui.doubleSpinBoxNumIterartionROIHeight->setValue(pSettingsData->m_NumIterationROIHeight);
        ui.doubleSpinBoxNumIterationAccseptanceThreshold->setValue(pSettingsData->m_NumIterationCalculateAccseptanceThreshold);
        ui.doubleSpinBoxFactorWidthSearchBottleTopLine->setValue(pSettingsData->m_FactorMeasureWindowWidthSearchBottleTopPosiition);
        ui.doubleSpinBoxNumIterationCalculatePixelSize->setValue(pSettingsData->m_NumIterationCalculatePixelSize);
        ui.doubleSpinBoxThresholdFactor->setValue(pSettingsData->m_FactorThreshold);
        ui.doubleSpinBoxMinContrastInPercent->setValue(pSettingsData->m_MinPossibleContrastValueInPercent);
        ui.doubleSpinBoxStartValueROIHeight->setValue(pSettingsData->m_StartValueMeasureWindowHeightInPixel);
        ui.doubleSpinBoxDiameterToleranceInPercent->setValue(pSettingsData->m_DiameterTolInPercentAutoCalibrate);
        ui.doubleSpinBoxBottleBaseLineOffsetInPix->setValue(pSettingsData->m_BottleBaseLineOffsetInPix);
        ui.doubleSpinBoxNumIterationROIYPos->setValue(pSettingsData->m_NumIterationROIYPos);
        ui.doubleSpinBoxRollingMeanBottlesPerMin->setValue(pSettingsData->m_TrendGraphRollingMeanSizeBottlesPerMin);
        ui.doubleSpinBoxPreasureIncreaseWhenFlushing->setValue(pSettingsData->m_PreasureIncreaseWhenFlushing);
        ui.doubleSpinBoxDefineNoPreasureValue->setValue(pSettingsData->m_DefineNoPreasureValue);

        ui.doubleSpinBoxWaterCoolingPIDMinStroke->setValue(pSettingsData->m_WaterCoolingStrokeMinValue);
        ui.doubleSpinBoxWaterCoolingPIDMaxStroke->setValue(pSettingsData->m_WaterCoolingStrokeMaxValue);
        ui.doubleSpinBoxWaterCoolingPFactor->setValue(pSettingsData->m_WaterCoolingPFactor);
        ui.doubleSpinBoxWaterCoolingIFactor->setValue(pSettingsData->m_WaterCoolingIFactor);
        ui.doubleSpinBoxWaterCoolingDFactor->setValue(pSettingsData->m_WaterCoolingDFactor);

        ui.comboBoxMaxTriggerPosOnScreen->setProperty(kAuditTrail, QVariant());
        switch (pSettingsData->m_MaxTriggerImagesOnScreen) {
            case 1:
                ui.comboBoxMaxTriggerPosOnScreen->setCurrentIndex(0);
                break;
            case 2:
                ui.comboBoxMaxTriggerPosOnScreen->setCurrentIndex(1);
                break;
            case 4:
                ui.comboBoxMaxTriggerPosOnScreen->setCurrentIndex(2);
                break;
            case 6:
                ui.comboBoxMaxTriggerPosOnScreen->setCurrentIndex(3);
                break;
            case 12:
                ui.comboBoxMaxTriggerPosOnScreen->setCurrentIndex(4);
                break;
            case 16:
                ui.comboBoxMaxTriggerPosOnScreen->setCurrentIndex(5);
                break;
            case 20:
                ui.comboBoxMaxTriggerPosOnScreen->setCurrentIndex(6);
                break;
            case 25:
                ui.comboBoxMaxTriggerPosOnScreen->setCurrentIndex(7);
                break;
            case 36:
                ui.comboBoxMaxTriggerPosOnScreen->setCurrentIndex(8);
                break;
            case 42:
                ui.comboBoxMaxTriggerPosOnScreen->setCurrentIndex(9);
                break;
            default:
                ui.comboBoxMaxTriggerPosOnScreen->setCurrentIndex(6);
                break;
        }
        ui.comboBoxMaxTriggerPosOnScreen->setProperty(kAuditTrail, ui.labelMaxTriggerPosOnScreen->text());
    }
    m_WindowSetup = true;
}

void AdvancedSettingsDialog::SlotSetMaxVideosOnScreen(int index)
{
    if (m_WindowSetup && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();

        switch (index) {
            case 0:
                pSettingsData->m_MaxTriggerImagesOnScreen = 1;
                break;
            case 1:
                pSettingsData->m_MaxTriggerImagesOnScreen = 2;
                break;
            case 2:
                pSettingsData->m_MaxTriggerImagesOnScreen = 4;
                break;
            case 3:
                pSettingsData->m_MaxTriggerImagesOnScreen = 8;
                break;
            case 4:
                pSettingsData->m_MaxTriggerImagesOnScreen = 12;
                break;
            case 5:
                pSettingsData->m_MaxTriggerImagesOnScreen = 16;
                break;
            case 6:
                pSettingsData->m_MaxTriggerImagesOnScreen = 20;
                break;
            case 7:
                pSettingsData->m_MaxTriggerImagesOnScreen = 25;
                break;
            case 8:
                pSettingsData->m_MaxTriggerImagesOnScreen = 36;
                break;
            case 9:
                pSettingsData->m_MaxTriggerImagesOnScreen = 42;
                break;
            default:
                pSettingsData->m_MaxTriggerImagesOnScreen = 16;
                break;
        }
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void AdvancedSettingsDialog::SlotExposureTimeChanged()
{
    if (GetMainAppCrystalT2() && m_WindowSetup) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_ExposureTime = ui.doubleSpinBoxExposureTime->value();
            GetMainAppCrystalT2()->SaveSettings();
            GetMainAppCrystalT2()->GetImageData()->SetExposureTime(ui.doubleSpinBoxExposureTime->value());
        }
    }
}

void AdvancedSettingsDialog::SlotBackgroundContrastChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_BackgroundContrast = ui.doubleSpinBoxBackgroundContrast->value();
            GetMainAppCrystalT2()->SaveSettings();
            GetMainAppCrystalT2()->GetImageData()->SetImageBackgroundContrast(static_cast<int>(ui.doubleSpinBoxBackgroundContrast->value()));
        }
    }
}

void AdvancedSettingsDialog::SlotDistanceCameraProductChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) {
            pSettingsData->m_DistanceCameraProduct = ui.doubleSpinBoxDistanceCameraProduct->value();
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void AdvancedSettingsDialog::SetTriggerState(int State, int ValveID)
{
    if (ValveID == LEFT_VALVE_ID) {
        if (State == TRIGGER_ON) {
            ui.doubleSpinBoxValueTrigger1OnOff->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "TriggerOn");  // Set Background Color
            ui.doubleSpinBoxValueTrigger1OnOff->setSpecialValueText(tr("On"));
        } else {
            ui.doubleSpinBoxValueTrigger1OnOff->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "TriggerOff");
            ui.doubleSpinBoxValueTrigger1OnOff->setSpecialValueText(tr("Off"));
        }
        ui.doubleSpinBoxValueTrigger1OnOff->style()->unpolish(ui.doubleSpinBoxValueTrigger1OnOff);
        ui.doubleSpinBoxValueTrigger1OnOff->style()->polish(ui.doubleSpinBoxValueTrigger1OnOff);
    } else {
        if (State == TRIGGER_ON) {
            ui.doubleSpinBoxValueTrigger2OnOff->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "TriggerOn");  // Set Background Color
            ui.doubleSpinBoxValueTrigger2OnOff->setSpecialValueText(tr("On"));
        } else {
            ui.doubleSpinBoxValueTrigger2OnOff->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "TriggerOff");
            ui.doubleSpinBoxValueTrigger2OnOff->setSpecialValueText(tr("Off"));
        }
        ui.doubleSpinBoxValueTrigger2OnOff->style()->unpolish(ui.doubleSpinBoxValueTrigger2OnOff);
        ui.doubleSpinBoxValueTrigger2OnOff->style()->polish(ui.doubleSpinBoxValueTrigger2OnOff);
    }
}

void AdvancedSettingsDialog::SlotApplyNewPixelSize()
{
    double value = ui.doubleSpinBoxMeasuredPixelSize->value();
    if (value > 0.0001) {
        BMessageBox* pMessageBox = new BMessageBox(QMessageBox::Information, tr("New Pixel Size"), tr("Apply New Pixel Size: %1 mm/pix").arg(value, 0, 'f', 5));
        pMessageBox->addButton(QMessageBox::Yes)->setText(tr("Yes"));
        pMessageBox->addButton(QMessageBox::No)->setText(tr("No"));

        if (pMessageBox->exec() != -1) {
            if (pMessageBox->standardButton(pMessageBox->clickedButton()) == QMessageBox::Yes) {
                ui.doubleSpinBoxUsedPixelSize->setValue(value);
                SetForceEditingFinishedPixelSize();
                SetNewPixelSize();
            }
        }
        delete pMessageBox;
    }
}

void AdvancedSettingsDialog::SetNewPixelSize()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_PixelSize = ui.doubleSpinBoxUsedPixelSize->value();
        GetMainAppCrystalT2()->SaveSettings();
        GetMainAppCrystalT2()->GetImageData()->SetCurrentPixelSizeInMMPerPixel(ui.doubleSpinBoxUsedPixelSize->value());  // to real time task
    }
}

void AdvancedSettingsDialog::SetForceEditingFinishedPixelSize()
{
    ui.doubleSpinBoxUsedPixelSize->setFocus();
    ui.doubleSpinBoxUsedPixelSize->clearFocus();
}

void AdvancedSettingsDialog::SlotPixelSizeChanged()
{
    if (m_WindowSetup) {
        SetNewPixelSize();
    }
}

void AdvancedSettingsDialog::SlotSaveIntervalShowTrendGraph()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_IntervalUpdateTrendGraph = ui.doubleSpinBoxIntervalSaveTempartureData->value();
        GetMainAppCrystalT2()->SaveSettings();
        GetMainAppCrystalT2()->StartTimerUpdateTrendGraph(pSettingsData->m_IntervalUpdateTrendGraph);
    }
}

void AdvancedSettingsDialog::SlotIntervalCheckCleanImageChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        if (pSettingsData) pSettingsData->m_TimerIntervalCheckCleanImageInMin = ui.doubleSpinBoxIntervalCheckCleanImage->value();
        GetMainAppCrystalT2()->SaveSettings();
        GetMainAppCrystalT2()->StartTimerIntervalCleanImage(pSettingsData->m_TimerIntervalCheckCleanImageInMin);
    }
}

void AdvancedSettingsDialog::SetMeasuredPixelSize(double set, bool Status)
{
    if (Status) {  // neuer messwert
        ui.doubleSpinBoxMeasuredPixelSize->setValue(set);
        m_CounterShowPixelSize = 0;
        ui.doubleSpinBoxMeasuredPixelSize->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Ok");  // Set Background Color
        ui.doubleSpinBoxMeasuredPixelSize->style()->unpolish(ui.doubleSpinBoxMeasuredPixelSize);
        ui.doubleSpinBoxMeasuredPixelSize->style()->polish(ui.doubleSpinBoxMeasuredPixelSize);
    } else {  // noch alter messwert
        m_CounterShowPixelSize++;
        if (m_CounterShowPixelSize > m_NumberSameMeasureValues) {  // Jetzt Annahme kein Produkt gefunden
            ui.doubleSpinBoxMeasuredPixelSize->setValue(-1.0);
            ui.doubleSpinBoxMeasuredPixelSize->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Warning");  // Set Background Color
            ui.doubleSpinBoxMeasuredPixelSize->style()->unpolish(ui.doubleSpinBoxMeasuredPixelSize);
            ui.doubleSpinBoxMeasuredPixelSize->style()->polish(ui.doubleSpinBoxMeasuredPixelSize);
            m_CounterShowPixelSize = 0;
        }
    }
}

void AdvancedSettingsDialog::SetMeasuredNeckDiameter(double set, bool Status)
{
    if (Status) {  // neuer messwert
        ui.doubleSpinBoxMeasureNeckDiameter->setValue(set);
        m_CounterShowNeckDiameter = 0;
        ui.doubleSpinBoxMeasureNeckDiameter->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Ok");  // Set Background Color
        ui.doubleSpinBoxMeasureNeckDiameter->style()->unpolish(ui.doubleSpinBoxMeasureNeckDiameter);
        ui.doubleSpinBoxMeasureNeckDiameter->style()->polish(ui.doubleSpinBoxMeasureNeckDiameter);
    } else {  // noch alter messwert
        m_CounterShowNeckDiameter++;
        if (m_CounterShowNeckDiameter > m_NumberSameMeasureValues) {  // wenn n mal der selbe Wert, dann Product nicht gefunden
            ui.doubleSpinBoxMeasureNeckDiameter->setValue(-1.0);
            ui.doubleSpinBoxMeasureNeckDiameter->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Warning");  // Set Background Color
            ui.doubleSpinBoxMeasureNeckDiameter->style()->unpolish(ui.doubleSpinBoxMeasureNeckDiameter);
            ui.doubleSpinBoxMeasureNeckDiameter->style()->polish(ui.doubleSpinBoxMeasureNeckDiameter);
            m_CounterShowNeckDiameter = 0;
        }
    }
}

void AdvancedSettingsDialog::SetProductPresentTime(double set)
{
    ui.doubleSpinBoxProductPresentTime->setValue(set);
}

void AdvancedSettingsDialog::SetInspectionTime(double set)
{
    ui.doubleSpinBoxInspectionTime->setValue(set);
}

void AdvancedSettingsDialog::SetRealTimeInterval(double set)
{
    ui.doubleSpinBoxRealTimeInterval->setValue(set);
}

double AdvancedSettingsDialog::GetSpeedFromIS()
{
    return ui.doubleSpinBoxSpeedFromIS->value();
}

double AdvancedSettingsDialog::GetSpeed()
{
    return ui.doubleSpinBoxSpeed->value();
}

void AdvancedSettingsDialog::SetSpeedFromIS(double set)
{
    ui.doubleSpinBoxSpeedFromIS->setValue(set);
}

void AdvancedSettingsDialog::SetSpeed(double set)
{
    double DeltaSpeed = fabs(set - ui.doubleSpinBoxSpeedFromIS->value());
    ui.doubleSpinBoxSpeed->setValue(set);
    if (set == -1.0)
        SetDeltaSpeed(set);
    else
        SetDeltaSpeed(DeltaSpeed);
}

void AdvancedSettingsDialog::SetDeltaSpeed(double set)
{
    if (m_SpeedDeviationBetweenCameraAndIS && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData() &&
        GetMainAppCrystalT2()->GetSettingsData()->m_EnableStatusSpeedDeviationBetweenCameraAndIS) {
        bool MaschineStopInMin = true;
        double Warn = GetMainAppCrystalT2()->GetSettingsData()->m_SpeedDeviationWarningLevelInMPerMin;
        double Alarm = GetMainAppCrystalT2()->GetSettingsData()->m_SpeedDeviationAlarmLevelInMPerMin;
        double MaschineStop = GetMainAppCrystalT2()->GetSettingsData()->m_SpeedDeviationMaschineStopTimeInSec;
        double Default = 0.0;
        m_StatusSpeedDeviationBetweenCameraAndIS = m_SpeedDeviationBetweenCameraAndIS->SetValueAndAlarmStatus(set, set, Default, Warn, Alarm, MaschineStop, MaschineStopInMin);
    } else {
        ui.doubleSpinBoxDeltaSpeed->setValue(set);
        m_StatusSpeedDeviationBetweenCameraAndIS = ALARM_LEVEL_OK;
        if (m_SpeedDeviationBetweenCameraAndIS) {
            m_SpeedDeviationBetweenCameraAndIS->StopTimerColorStatus();
            m_SpeedDeviationBetweenCameraAndIS->SetBackgroundColor(HMIColor::ContentBoard);
        }
    }
}
