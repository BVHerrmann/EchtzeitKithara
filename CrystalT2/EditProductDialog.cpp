#include "EditProductDialog.h"
#include "AdminSettingsDialog.h"
#include "GeneralDialog.h"

#include <QThreadPool>
#include "ImageData.h"
#include "LiveImageView.h"
#include "MainAppCrystalT2.h"
#include "PlotLineProfil.h"
#include "ProductData.h"
#include "ProgressBar.h"
#include "QtConcurrent/qtconcurrentrun.h"
#include "Qtgui"
#include "SettingsData.h"
#include "ValveDialog.h"
#include "bmessagebox.h"

#include <audittrail.h>
#include "colors.h"

EditProductDialog::EditProductDialog(MainAppCrystalT2* pMainAppCrystalT2, QWidget* pWidget)
    : QWidget(pWidget),
      m_MainAppCrystalT2(NULL),
      m_WindowSetup(false),
      m_PlotLineProfil(NULL),
      m_CounterShowNeckDiameter(0),
      m_CounterShowPixelSize(0),
      m_NumberSameMeasureValues(40),
      m_MaxValueProductCenterToleranceInMM(0),
      m_PopupProgressBar(NULL),
      m_MeasureWindowSpinBoxesHasSlotConnection(false),
      m_TimerAutorepeatMoveUp(NULL),
      m_TimerAutorepeatMoveDown(NULL),
      m_TimerAutorepeatMoveLeft(NULL),
      m_TimerAutorepeatMoveRight(NULL),
      m_AutoRepeatDelay(400),
      m_AutoRepeatInterval(40),
      m_MinProductCenterTolerance(0.0),
      m_ProductDiameterIsOutOfTol(false),
      m_ProductEdgeNotFound(false),
      m_CounterSizeNotOk(0),
      m_CounterEdgeNotOk(0)
{
    ui.setupUi(this);

    ui.checkBoxSelectSpeedMeasureWindow->setChecked(false);
    ui.checkBoxSelectLiquidMeasureWindow->setChecked(false);
    ui.checkBoxSelectLiquidAndSpeedMeasureWindow->setChecked(true);
    ui.checkBoxSelectCamera->setChecked(false);

    ui.doubleSpinBoxResultInfo->setSpecialValueText(tr("Product Found"));
    ui.doubleSpinBoxResultInfo->setValue(-1.0);
    ui.doubleSpinBoxResultInfo->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Ok");  // Set Background Color
    ui.doubleSpinBoxResultInfo->style()->unpolish(ui.doubleSpinBoxResultInfo);
    ui.doubleSpinBoxResultInfo->style()->polish(ui.doubleSpinBoxResultInfo);

    CheckEnableColorBoxesControls();

    ui.groupBoxSetMeasureWindowMeasureSpeed->hide();
    ui.groupBoxSetMeasureWindowMeasureLiquid->hide();

    m_MainAppCrystalT2 = pMainAppCrystalT2;
    m_PopupProgressBar = new PopupProgressBar(this);
    m_PopupProgressBar->SetInfoTextPleaseWait(tr("Calibrate System Please Wait...."));
    m_PopupProgressBar->SetMessageBoxTextInfo(tr("Calibration Ready"));
    m_PopupProgressBar->SetTitle(tr("Calibrate System"));

    ui.doubleSpinBoxMeasureNeckDiameter->setSpecialValueText(tr("Product Not Found->"));
    qApp->installEventFilter(this);

    connect(ui.doubleSpinBoxProductNeckDiameter, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotNeckDiameterValueChanged);
    connect(ui.doubleSpinBoxProductCenterTol, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotProductCenterToleranceChanged);
    connect(ui.doubleSpinBoxNeckInnerDiameter, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotNeckInnerDiameterChanged);

    connect(ui.doubleSpinBoxPulseLeftValve, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotPulseLeftValveChanged);
    connect(ui.doubleSpinBoxPulseRightValve, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotPulseRightValveChanged);

    connect(ui.pushButtonUp, &QPushButton::clicked, this, &EditProductDialog::SlotMoveUp);
    connect(ui.pushButtonDown, &QPushButton::clicked, this, &EditProductDialog::SlotMoveDown);
    connect(ui.pushButtonLeft, &QPushButton::clicked, this, &EditProductDialog::SlotMoveToLeft);
    connect(ui.pushButtonRight, &QPushButton::clicked, this, &EditProductDialog::SlotMoveToRight);

    ui.pushButtonUp->setAutoRepeat(true);
    ui.pushButtonDown->setAutoRepeat(true);
    ui.pushButtonLeft->setAutoRepeat(true);
    ui.pushButtonRight->setAutoRepeat(true);

    ui.pushButtonUp->setAttribute(Qt::WA_AcceptTouchEvents);
    ui.pushButtonDown->setAttribute(Qt::WA_AcceptTouchEvents);
    ui.pushButtonLeft->setAttribute(Qt::WA_AcceptTouchEvents);
    ui.pushButtonRight->setAttribute(Qt::WA_AcceptTouchEvents);

    ui.doubleSpinBoxHeight->setMaximum(250);
    ui.doubleSpinBoxHeight->setMinimum(MINIMUM_ROI_SIZE_IN_PIXEL);
    ui.doubleSpinBoxWidth->setMaximum(600);
    ui.doubleSpinBoxWidth->setMinimum(MINIMUM_ROI_SIZE_IN_PIXEL);

    connect(ui.doubleSpinBoxWidth, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &EditProductDialog::SlotMeasureWidthChanged);
    connect(ui.doubleSpinBoxHeight, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &EditProductDialog::SlotMeasureHeightChanged);

    connect(ui.checkBoxSelectLiquidMeasureWindow, &QCheckBox::stateChanged, this, &EditProductDialog::SlotSelectLiquidMeasureWindowChanged, Qt::QueuedConnection);
    connect(ui.checkBoxSelectSpeedMeasureWindow, &QCheckBox::stateChanged, this, &EditProductDialog::SlotSelectSpeedMeasureWindowChanged, Qt::QueuedConnection);
    connect(ui.checkBoxSelectLiquidAndSpeedMeasureWindow, &QCheckBox::stateChanged, this, &EditProductDialog::SlotSelectLiquidAndSpeedMeasureWindowChanged, Qt::QueuedConnection);
    connect(ui.checkBoxSelectCamera, &QCheckBox::stateChanged, this, &EditProductDialog::SlotSelectCameraROI, Qt::QueuedConnection);

    connect(this, &EditProductDialog::SignalResetLiquidResults, this, &EditProductDialog::SlotResetLiquidResults);

    /*
    m_MeasureWindowSpinBoxesHasSlotConnection=true;
    connect(ui.doubleSpinBoxMeasureSpeedXPosition, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROIMeasureSpeedXPosChanged);
    connect(ui.doubleSpinBoxMeasureSpeedYPosition, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROIMeasureSpeedYPosChanged);
    connect(ui.doubleSpinBoxMeasureSpeedWidth, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROIMeasureSpeedWidthChanged);
    connect(ui.doubleSpinBoxMeasureSpeedHeight, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROIMeasureSpeedHeightChanged);
    connect(ui.doubleSpinBoxMeasureLiquidXPosition, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROILiquidXPosChanged);
    connect(ui.doubleSpinBoxMeasureLiquidYPosition, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROILiquidYPosChanged);
    connect(ui.doubleSpinBoxMeasureLiquidWidth, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROILiquidWidthChanged);
    connect(ui.doubleSpinBoxMeasureLiquidHeight, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROILiquidHeightChanged);
    */

    double MaxValueInMM = 25.0;

    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        MaxValueInMM = GetMainAppCrystalT2()->GetSettingsData()->m_DistancesBetweenValves;
    }

    ui.doubleSpinBoxInjectionMiddleWindowWidthInMm->setMaximum(MaxValueInMM - 5.0);
    ui.doubleSpinBoxInjectionMiddleWindowWidthInMm->setMinimum(2);

    connect(ui.pushButtonCalibrate, &QPushButton::clicked, this, &EditProductDialog::SlotCalibrate);
    connect(ui.doubleSpinBoxDistanceInjectorBottle, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotDistanceInjectorBottleChanged);
    connect(ui.doubleSpinBoxInjectionAngle, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotInjectorAngleChanged);
    connect(ui.doubleSpinBoxMinAcceptanceThresholdLiquidMiddleROI, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotMinThresholdMiddleROIChanged);
    connect(ui.doubleSpinBoxMaxAcceptanceThresholdLiquidMiddleROI, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotMaxThresholdMiddleROIChanged);
    connect(ui.doubleSpinBoxAcceptanceThresholdLiquidLeftAndRightROI, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotThresholdLeftRightROIChanged);
    connect(ui.pushButtonResetCounters, &QPushButton::clicked, this, &EditProductDialog::SlotResetCounters);
    connect(ui.doubleSpinBoxInjectionMiddleWindowWidthInMm, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotInjectionMiddleWindowInPercentChanged);
    connect(this, &EditProductDialog::SignalEnableGUIElements, this, &EditProductDialog::SlotEnableGUIElements);
    connect(m_PopupProgressBar, &PopupProgressBar::SignalAbortIsClicked, this, &EditProductDialog::SlotCalibrateAbortIsClicked);

    connect(ui.pushButtonCenterCameraViewport, &QPushButton::clicked, this, &EditProductDialog::SlotCenterCameraViewport);

    // nur für die Kamerasimulation
    /*connect(ui.pushButtonRunCameraSimulation, &QPushButton::pressed, this, &EditProductDialog::SlotRunCameraSimulation);
    connect(ui.pushButtonStepCameraSimulation, &QPushButton::pressed, this, &EditProductDialog::SlotStepCameraSimulation);
    connect(ui.pushButtonStopCameraSimulation, &QPushButton::pressed, this, &EditProductDialog::SlotStopCameraSimulation);
    connect(ui.pushButtonRunOneVideoCameraSimulation, &QPushButton::pressed, this, &EditProductDialog::SlotPlayOneVideoCameraSimulation);

    if (!GetMainAppCrystalT2()->GetSettingsData()->m_WorkWithoutCamera) {
        ui.pushButtonRunCameraSimulation->hide();
        ui.pushButtonStopCameraSimulation->hide();
        ui.pushButtonStepCameraSimulation->hide();
        ui.pushButtonRunOneVideoCameraSimulation->hide();
    } else
        ui.labelBandDirectional->hide();*/

    ui.pushButtonRunCameraSimulation->hide();
    ui.pushButtonStopCameraSimulation->hide();
    ui.pushButtonStepCameraSimulation->hide();
    ui.pushButtonRunOneVideoCameraSimulation->hide();

    m_PlotLineProfil = new PlotLineProfil(m_MainAppCrystalT2, ui.frameGradientImage);

    m_TimerAutorepeatMoveUp = new QTimer(this);
    connect(m_TimerAutorepeatMoveUp, &QTimer::timeout, this, &EditProductDialog::SlotAutorepeatMoveUp);
    m_TimerAutorepeatMoveUp->setInterval(m_AutoRepeatInterval);

    m_TimerAutorepeatMoveDown = new QTimer(this);
    connect(m_TimerAutorepeatMoveDown, &QTimer::timeout, this, &EditProductDialog::SlotAutorepeatMoveDown);
    m_TimerAutorepeatMoveDown->setInterval(m_AutoRepeatInterval);

    m_TimerAutorepeatMoveLeft = new QTimer(this);
    connect(m_TimerAutorepeatMoveLeft, &QTimer::timeout, this, &EditProductDialog::SlotAutorepeatMoveLeft);
    m_TimerAutorepeatMoveLeft->setInterval(m_AutoRepeatInterval);

    m_TimerAutorepeatMoveRight = new QTimer(this);
    connect(m_TimerAutorepeatMoveRight, &QTimer::timeout, this, &EditProductDialog::SlotAutorepeatMoveRight);
    m_TimerAutorepeatMoveRight->setInterval(m_AutoRepeatInterval);

    ui.pushButtonCenterCameraViewport->hide();
    ui.labelCameraVieportCoordinates->hide();

    SetAuditTrailProperties();
    SetRequiredAccessLevel();
}

EditProductDialog::~EditProductDialog()
{
}

void EditProductDialog::SetRequiredAccessLevel()
{
    ui.groupBoxMeasureParamter->setProperty(kRequiredAccessLevel, kAccessLevelUser);
    ui.groupBoxValveSettindPulse->setProperty(kRequiredAccessLevel, kAccessLevelUser);
    ui.groupBoxColorBoxesControls->setProperty(kRequiredAccessLevel, kAccessLevelUser);
    ui.groupBoxAlarmThresholdLiquidAndSplash->setProperty(kRequiredAccessLevel, kAccessLevelUser);
    ui.groupBoxInjectionPosition->setProperty(kRequiredAccessLevel, kAccessLevelUser);
    ui.pushButtonCalibrate->setProperty(kRequiredAccessLevel, kAccessLevelUser);
    ui.pushButtonResetCounters->setProperty(kRequiredAccessLevel, kAccessLevelUser);
}

void EditProductDialog::SetAuditTrailProperties()
{
    ui.pushButtonResetCounters->setProperty(kAuditTrail, tr("Button Reset Counters"));
    ui.pushButtonCalibrate->setProperty(kAuditTrail, ui.pushButtonCalibrate->text());

    ui.doubleSpinBoxProductNeckDiameter->setProperty(kAuditTrail, ui.labelContentBoardProductDiameter->text());
    ui.doubleSpinBoxProductCenterTol->setProperty(kAuditTrail, ui.labelDashBoardProductDiameterTol->text());
    ui.doubleSpinBoxNeckInnerDiameter->setProperty(kAuditTrail, ui.labelDashBoardNeckInnerDiameter->text());

    ui.doubleSpinBoxPulseLeftValve->setProperty(kAuditTrail, ui.labelContentBoardPulseLeftValve->text());
    ui.doubleSpinBoxPulseRightValve->setProperty(kAuditTrail, ui.labelContentBoardPulseRightValve->text());

    ui.doubleSpinBoxMinAcceptanceThresholdLiquidMiddleROI->setProperty(kAuditTrail, ui.labelContentBoardMinAcceptanceThresholdLiquidMiddleROI->text());
    ui.doubleSpinBoxMaxAcceptanceThresholdLiquidMiddleROI->setProperty(kAuditTrail, ui.labelContentBoardMaxAcceptanceThresholdLiquidMiddleROI->text());
    ui.doubleSpinBoxAcceptanceThresholdLiquidLeftAndRightROI->setProperty(kAuditTrail, ui.labelContentBoardAcceptanceThresholdLiquidLeftAndRightROI->text());

    ui.doubleSpinBoxDistanceInjectorBottle->setProperty(kAuditTrail, ui.labelDashBoardDistanceInjectorBottle->text());
    ui.doubleSpinBoxInjectionAngle->setProperty(kAuditTrail, ui.labelDashBoardInjectionAngle->text());
    ui.doubleSpinBoxInjectionMiddleWindowWidthInMm->setProperty(kAuditTrail, ui.labelContentBoardROIInjectionMiddleWindowWidthInMm->text());

    ui.pushButtonUp->setProperty(kAuditTrail, tr("Blue/Green Measuerment Window Up"));
    ui.pushButtonDown->setProperty(kAuditTrail, tr("Blue/Green Measuerment Window Down"));
    ui.pushButtonLeft->setProperty(kAuditTrail, tr("Blue/Green Measuerment Window To Left"));
    ui.pushButtonRight->setProperty(kAuditTrail, tr("Blue/Green Measuerment Window To Right"));

    ui.doubleSpinBoxHeight->setProperty(kAuditTrail, tr("Blue/Green Measuerment Window Height"));
    ui.doubleSpinBoxWidth->setProperty(kAuditTrail, tr("Blue/Green Measuerment Window Width"));

    ui.checkBoxSelectSpeedMeasureWindow->setProperty(kAuditTrail, ui.labelSelectGreen->text());
    ui.checkBoxSelectLiquidAndSpeedMeasureWindow->setProperty(kAuditTrail, ui.labelSelectBlueAndGreen->text());
    ui.checkBoxSelectLiquidMeasureWindow->setProperty(kAuditTrail, ui.labelSelectBlue->text());
    ui.checkBoxSelectCamera->setProperty(kAuditTrail, ui.labelSelectCamera->text());

    ui.pushButtonCenterCameraViewport->setProperty(kAuditTrail, ui.pushButtonCenterCameraViewport->text());
}

void EditProductDialog::SetCheckBoxSpeed(bool set)
{
    ui.checkBoxSelectSpeedMeasureWindow->setChecked(set);
}

void EditProductDialog::SetCheckBoxLiquid(bool set)
{
    ui.checkBoxSelectLiquidMeasureWindow->setChecked(set);
}

void EditProductDialog::MoveCameraROI(int MoveDirection)
{
    if (GetMainAppCrystalT2()) {
        QRect CurrentRect = GetMainAppCrystalT2()->GetMeasureWindowRect(ROI_ID_CAMERA);
        int NewXOffset = CurrentRect.x();
        int NewYOffset = CurrentRect.y();
        int MoveOffsetx = 16;
        int MoveOffsety = 16;
        bool ChangeXOff = false;

        if (GetMainAppCrystalT2()->GetSettingsData()) {
            MoveOffsetx = GetMainAppCrystalT2()->GetSettingsData()->m_CameraMoveOffsetInX;
            MoveOffsety = GetMainAppCrystalT2()->GetSettingsData()->m_CameraMoveOffsetInY;
        }
        switch (MoveDirection) {
            case MOVE_CAMERA_ROI_DIRECTION::MOVE_UP:
                NewYOffset = NewYOffset - MoveOffsetx;
                break;
            case MOVE_CAMERA_ROI_DIRECTION::MOVE_DOWN:
                NewYOffset = NewYOffset + MoveOffsety;
                break;
            case MOVE_CAMERA_ROI_DIRECTION::MOVE_LEFT:
                NewXOffset = NewXOffset - MoveOffsetx;
                ChangeXOff = true;
                break;
            case MOVE_CAMERA_ROI_DIRECTION::MOVE_RIGHT:
                NewXOffset = NewXOffset + MoveOffsety;
                ChangeXOff = true;
                break;
            default:
                qDebug() << "Unknown Camera Move Direction";
                break;
        }
        if (GetMainAppCrystalT2()->GetImageData()) {
            QString ErrorMsg;
            int rv;

            if (ChangeXOff) {
                rv = GetMainAppCrystalT2()->GetImageData()->SetCameraXOffset(NewXOffset, ErrorMsg);
            } else {
                rv = GetMainAppCrystalT2()->GetImageData()->SetCameraYOffset(NewYOffset, ErrorMsg);
            }
            if (rv != ERROR_CODE_NO_ERROR) {
                GetMainAppCrystalT2()->SlotAddNewMessage(ErrorMsg, QtMsgType::QtWarningMsg);
            } else {
                DrawCameraViewportOffsets();
            }
        }
    }
}

void EditProductDialog::SlotCenterCameraViewport()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) {
        QString ErrorMsg;
        int rv;
        int NewXOffset = 0;
        int NewYOffset = 0;
        int MaxW = GetMainAppCrystalT2()->GetImageData()->GetMaxImageWidth();
        int MaxH = GetMainAppCrystalT2()->GetImageData()->GetMaxImageHeight();

        NewXOffset = static_cast<int>((MaxW - USED_CAMERA_WIDTH) / 2.0);
        NewYOffset = static_cast<int>((MaxH - USED_CAMERA_HEIGHT) / 2.0);
        rv = GetMainAppCrystalT2()->GetImageData()->SetCameraXOffset(NewXOffset, ErrorMsg);
        if (rv == ERROR_CODE_NO_ERROR) {
            rv = GetMainAppCrystalT2()->GetImageData()->SetCameraYOffset(NewYOffset, ErrorMsg);
            if (rv == ERROR_CODE_NO_ERROR) {
                DrawCameraViewportOffsets();
            }
        }
    }
}

void EditProductDialog::SlotMoveUp()
{
    if (ui.checkBoxSelectSpeedMeasureWindow->isChecked() || ui.checkBoxSelectLiquidAndSpeedMeasureWindow->isChecked()) {
        int NewYPos = static_cast<int>(ui.doubleSpinBoxMeasureSpeedYPosition->value()) - 1;
        ui.doubleSpinBoxMeasureSpeedYPosition->setValue(NewYPos);
        ROISpeedXYPositionChanged();
    }
    if (ui.checkBoxSelectLiquidMeasureWindow->isChecked() || ui.checkBoxSelectLiquidAndSpeedMeasureWindow->isChecked()) {
        int NewYPos = static_cast<int>(ui.doubleSpinBoxMeasureLiquidYPosition->value()) - 1;
        ui.doubleSpinBoxMeasureLiquidYPosition->setValue(NewYPos);
        ROILiquidXYPositionChanged();
    }
    if (ui.checkBoxSelectCamera->isChecked()) {
        MoveCameraROI(MOVE_CAMERA_ROI_DIRECTION::MOVE_UP);
    }
}

void EditProductDialog::SlotAutorepeatMoveUp()
{
    SlotMoveUp();
    if (m_TimerAutorepeatMoveUp) {
        m_TimerAutorepeatMoveUp->setInterval(m_AutoRepeatInterval);
    }
}

void EditProductDialog::SlotMoveDown()
{
    if (ui.checkBoxSelectSpeedMeasureWindow->isChecked() || ui.checkBoxSelectLiquidAndSpeedMeasureWindow->isChecked()) {
        int NewYPos = static_cast<int>(ui.doubleSpinBoxMeasureSpeedYPosition->value()) + 1;
        ui.doubleSpinBoxMeasureSpeedYPosition->setValue(NewYPos);
        ROISpeedXYPositionChanged();
    }
    if (ui.checkBoxSelectLiquidMeasureWindow->isChecked() || ui.checkBoxSelectLiquidAndSpeedMeasureWindow->isChecked()) {
        int NewYPos = static_cast<int>(ui.doubleSpinBoxMeasureLiquidYPosition->value()) + 1;
        ui.doubleSpinBoxMeasureLiquidYPosition->setValue(NewYPos);
        ROILiquidXYPositionChanged();
    }
    if (ui.checkBoxSelectCamera->isChecked()) {
        MoveCameraROI(MOVE_CAMERA_ROI_DIRECTION::MOVE_DOWN);
    }
}

void EditProductDialog::SlotAutorepeatMoveDown()
{
    SlotMoveDown();
    if (m_TimerAutorepeatMoveDown) m_TimerAutorepeatMoveDown->setInterval(m_AutoRepeatInterval);
}

void EditProductDialog::SlotMoveToLeft()
{
    int NewXPos;

    if (ui.checkBoxSelectCamera->isChecked()) {
        MoveCameraROI(MOVE_CAMERA_ROI_DIRECTION::MOVE_LEFT);
    } else {
        if (!ui.checkBoxSelectSpeedMeasureWindow->isChecked() && !ui.checkBoxSelectLiquidMeasureWindow->isChecked()) {
            if (GetMainAppCrystalT2()->IsFirstTriggerOnLeftSide()) {
                NewXPos = static_cast<int>(ui.doubleSpinBoxMeasureSpeedXPosition->value()) - 1;
                if (NewXPos >= 0) {
                    ui.doubleSpinBoxMeasureSpeedXPosition->setValue(NewXPos);
                    ROISpeedXYPositionChanged();
                    NewXPos = static_cast<int>(ui.doubleSpinBoxMeasureLiquidXPosition->value()) - 1;
                    ui.doubleSpinBoxMeasureLiquidXPosition->setValue(NewXPos);
                    ROILiquidXYPositionChanged();
                }
            } else {
                NewXPos = static_cast<int>(ui.doubleSpinBoxMeasureLiquidXPosition->value()) - 1;
                if (NewXPos >= 0) {
                    ui.doubleSpinBoxMeasureLiquidXPosition->setValue(NewXPos);
                    ROILiquidXYPositionChanged();
                    NewXPos = static_cast<int>(ui.doubleSpinBoxMeasureSpeedXPosition->value()) - 1;
                    ui.doubleSpinBoxMeasureSpeedXPosition->setValue(NewXPos);
                    ROISpeedXYPositionChanged();
                }
            }
        } else {
            if (ui.checkBoxSelectSpeedMeasureWindow->isChecked()) {
                NewXPos = static_cast<int>(ui.doubleSpinBoxMeasureSpeedXPosition->value()) - 1;
                if (GetMainAppCrystalT2()->IsFirstTriggerOnLeftSide()) {
                    if (NewXPos >= 0) {
                        ui.doubleSpinBoxMeasureSpeedXPosition->setValue(NewXPos);
                        ROISpeedXYPositionChanged();
                    }
                } else {
                    if (NewXPos >= (ui.doubleSpinBoxMeasureLiquidXPosition->value() + ui.doubleSpinBoxMeasureLiquidWidth->value())) {
                        ui.doubleSpinBoxMeasureSpeedXPosition->setValue(NewXPos);
                        ROISpeedXYPositionChanged();
                    }
                }
            }
            if (ui.checkBoxSelectLiquidMeasureWindow->isChecked()) {
                NewXPos = static_cast<int>(ui.doubleSpinBoxMeasureLiquidXPosition->value()) - 1;
                if (GetMainAppCrystalT2()->IsFirstTriggerOnLeftSide()) {
                    if (NewXPos >= (ui.doubleSpinBoxMeasureSpeedXPosition->value() + ui.doubleSpinBoxMeasureSpeedWidth->value())) {
                        ui.doubleSpinBoxMeasureLiquidXPosition->setValue(NewXPos);
                        ROILiquidXYPositionChanged();
                    }
                } else {
                    if (NewXPos >= 0) {
                        ui.doubleSpinBoxMeasureLiquidXPosition->setValue(NewXPos);
                        ROILiquidXYPositionChanged();
                    }
                }
            }
        }
    }
}

void EditProductDialog::SlotAutorepeatMoveLeft()
{
    SlotMoveToLeft();
    if (m_TimerAutorepeatMoveLeft) m_TimerAutorepeatMoveLeft->setInterval(m_AutoRepeatInterval);
}

void EditProductDialog::SlotMoveToRight()
{
    int NewXPos;

    if (ui.checkBoxSelectCamera->isChecked()) {
        MoveCameraROI(MOVE_CAMERA_ROI_DIRECTION::MOVE_RIGHT);
    } else {
        if (!ui.checkBoxSelectSpeedMeasureWindow->isChecked() && !ui.checkBoxSelectLiquidMeasureWindow->isChecked()) {
            if (GetMainAppCrystalT2()->IsFirstTriggerOnLeftSide()) {
                NewXPos = static_cast<int>(ui.doubleSpinBoxMeasureLiquidXPosition->value()) + 1;
                if ((NewXPos + ui.doubleSpinBoxMeasureLiquidWidth->value()) < GetMainAppCrystalT2()->GetImageData()->GetImageWidth()) {
                    ui.doubleSpinBoxMeasureLiquidXPosition->setValue(NewXPos);
                    ROILiquidXYPositionChanged();
                    NewXPos = static_cast<int>(ui.doubleSpinBoxMeasureSpeedXPosition->value()) + 1;
                    ui.doubleSpinBoxMeasureSpeedXPosition->setValue(NewXPos);
                    ROISpeedXYPositionChanged();
                }
            } else {
                NewXPos = static_cast<int>(ui.doubleSpinBoxMeasureSpeedXPosition->value()) + 1;
                if ((NewXPos + ui.doubleSpinBoxMeasureSpeedWidth->value()) < GetMainAppCrystalT2()->GetImageData()->GetImageWidth()) {
                    ui.doubleSpinBoxMeasureSpeedXPosition->setValue(NewXPos);
                    ROISpeedXYPositionChanged();
                    NewXPos = static_cast<int>(ui.doubleSpinBoxMeasureLiquidXPosition->value()) + 1;
                    ui.doubleSpinBoxMeasureLiquidXPosition->setValue(NewXPos);
                    ROILiquidXYPositionChanged();
                }
            }
        } else {
            if (ui.checkBoxSelectSpeedMeasureWindow->isChecked()) {
                NewXPos = static_cast<int>(ui.doubleSpinBoxMeasureSpeedXPosition->value()) + 1;
                if (GetMainAppCrystalT2()->IsFirstTriggerOnLeftSide()) {
                    if ((NewXPos + ui.doubleSpinBoxMeasureSpeedWidth->value()) <= ui.doubleSpinBoxMeasureLiquidXPosition->value()) {
                        ui.doubleSpinBoxMeasureSpeedXPosition->setValue(NewXPos);
                        ROISpeedXYPositionChanged();
                    }
                } else {
                    if ((NewXPos + ui.doubleSpinBoxMeasureSpeedWidth->value()) < GetMainAppCrystalT2()->GetImageData()->GetImageWidth()) {
                        ui.doubleSpinBoxMeasureSpeedXPosition->setValue(NewXPos);
                        ROISpeedXYPositionChanged();
                    }
                }
            }
            if (ui.checkBoxSelectLiquidMeasureWindow->isChecked()) {
                NewXPos = static_cast<int>(ui.doubleSpinBoxMeasureLiquidXPosition->value()) + 1;
                if (GetMainAppCrystalT2()->IsFirstTriggerOnLeftSide()) {
                    if ((NewXPos + ui.doubleSpinBoxMeasureLiquidWidth->value()) < GetMainAppCrystalT2()->GetImageData()->GetImageWidth()) {
                        ui.doubleSpinBoxMeasureLiquidXPosition->setValue(NewXPos);
                        ROILiquidXYPositionChanged();
                    }
                } else {
                    if ((NewXPos + ui.doubleSpinBoxMeasureLiquidWidth->value()) <= ui.doubleSpinBoxMeasureSpeedXPosition->value()) {
                        ui.doubleSpinBoxMeasureLiquidXPosition->setValue(NewXPos);
                        ROILiquidXYPositionChanged();
                    }
                }
            }
        }
    }
}

void EditProductDialog::SlotAutorepeatMoveRight()
{
    SlotMoveToRight();
    if (m_TimerAutorepeatMoveRight) m_TimerAutorepeatMoveRight->setInterval(m_AutoRepeatInterval);
}

void EditProductDialog::SlotMeasureWidthChanged(double value)
{
    int MaxWidth;
    if (ui.checkBoxSelectSpeedMeasureWindow->isChecked()) {
        if (GetMainAppCrystalT2()->IsFirstTriggerOnLeftSide()) {
            MaxWidth = static_cast<int>(ui.doubleSpinBoxMeasureLiquidXPosition->value());
            if ((static_cast<int>(ui.doubleSpinBoxMeasureSpeedXPosition->value()) + value) >= MaxWidth) {
                SlotMoveToLeft();
            }
        } else {
            MaxWidth = GetMainAppCrystalT2()->GetImageData()->GetImageWidth();
            if ((static_cast<int>(ui.doubleSpinBoxMeasureSpeedXPosition->value()) + value) >= MaxWidth) {
                SlotMoveToLeft();
            } else {
                SlotMoveToRight();
            }
        }
        ui.doubleSpinBoxMeasureSpeedWidth->setValue(value);
        SlotROIMeasureSpeedWidthChanged();
    } else {
        if (ui.checkBoxSelectLiquidMeasureWindow->isChecked()) {
            if (GetMainAppCrystalT2()->IsFirstTriggerOnLeftSide()) {
                MaxWidth = GetMainAppCrystalT2()->GetImageData()->GetImageWidth();
                if ((static_cast<int>(ui.doubleSpinBoxMeasureLiquidXPosition->value()) + value) >= MaxWidth) {
                    SlotMoveToLeft();
                } else {
                    SlotMoveToRight();
                }
            } else {
                MaxWidth = static_cast<int>(ui.doubleSpinBoxMeasureSpeedXPosition->value());
                if ((static_cast<int>(ui.doubleSpinBoxMeasureLiquidXPosition->value()) + value) >= MaxWidth) {
                    SlotMoveToLeft();
                }
            }
            ui.doubleSpinBoxMeasureLiquidWidth->setValue(value);
            SlotROILiquidWidthChanged();
        }
    }
}

void EditProductDialog::SlotMeasureHeightChanged(double value)
{
    if (ui.checkBoxSelectSpeedMeasureWindow->isChecked()) {
        if (value >= MINIMUM_ROI_SIZE_IN_PIXEL) {
            ui.doubleSpinBoxMeasureSpeedHeight->setValue(value);
            SlotROIMeasureSpeedHeightChanged();
        }
    } else {
        if (ui.checkBoxSelectLiquidMeasureWindow->isChecked()) {
            ui.doubleSpinBoxMeasureLiquidHeight->setValue(value);
            SlotROILiquidHeightChanged();
        }
    }
}

void EditProductDialog::SlotSelectCameraROI(int state)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetLiveImageView()) {
        if (state == Qt::Checked) {
            ui.pushButtonCenterCameraViewport->show();
            if (ui.checkBoxSelectSpeedMeasureWindow->isChecked()) {
                ui.checkBoxSelectSpeedMeasureWindow->setChecked(false);
            }
            if (ui.checkBoxSelectLiquidAndSpeedMeasureWindow->isChecked()) {
                ui.checkBoxSelectLiquidAndSpeedMeasureWindow->setChecked(false);
            }
            if (ui.checkBoxSelectLiquidMeasureWindow->isChecked()) {
                ui.checkBoxSelectLiquidMeasureWindow->setChecked(false);
            }
            GetMainAppCrystalT2()->GetLiveImageView()->SetSelectSpeedWindow(false);
            GetMainAppCrystalT2()->GetLiveImageView()->SetSelectLiquidWindow(false);
            GetMainAppCrystalT2()->GetLiveImageView()->DrawMeasureWindow(ROI_ID_MEASURE_SPEED);
            GetMainAppCrystalT2()->GetLiveImageView()->DrawMeasureWindow(ROI_ID_MEASURE_LIQUID);
            DrawCameraViewportOffsets();
            CheckEnableColorBoxesControls();
        } else {
            if (!ui.checkBoxSelectSpeedMeasureWindow->isChecked() && !ui.checkBoxSelectLiquidMeasureWindow->isChecked() && !ui.checkBoxSelectLiquidAndSpeedMeasureWindow->isChecked()) {
                ui.checkBoxSelectLiquidAndSpeedMeasureWindow->setChecked(true);
            }
            if (GetMainAppCrystalT2()->GetEditProductDialog()) {
                GetMainAppCrystalT2()->GetEditProductDialog()->ClearCameraViewportOffsets();
            }
            ui.pushButtonCenterCameraViewport->hide();
        }
    }
}

void EditProductDialog::DrawCameraViewportOffsets()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData() && GetMainAppCrystalT2()->GetSettingsData()) {
        ui.labelCameraVieportCoordinates->show();
        QRect CameraViewport = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRect(ROI_ID_CAMERA);
        double PixelSize = GetMainAppCrystalT2()->GetSettingsData()->m_PixelSize;
        QString offsets = tr("x:%1mm\ny:%2mm").arg(CameraViewport.x() * PixelSize, 0, 'f', 1).arg(CameraViewport.y() * PixelSize, 0, 'f', 1);
        QFont font;

        font.setFamily(QString::fromUtf8("Siemens Sans"));
        font.setPixelSize(16);
        font.setBold(true);
        ui.labelCameraVieportCoordinates->setFont(font);
        ui.labelCameraVieportCoordinates->setText(offsets);
    }
}

void EditProductDialog::ClearCameraViewportOffsets()
{
    ui.labelCameraVieportCoordinates->hide();
}

void EditProductDialog::SlotSelectLiquidMeasureWindowChanged(int state)
{
    if (state == Qt::Checked) {
        double h = ui.doubleSpinBoxMeasureLiquidHeight->value();
        double w = ui.doubleSpinBoxMeasureLiquidWidth->value();

        if (ui.checkBoxSelectSpeedMeasureWindow->isChecked()) {
            ui.checkBoxSelectSpeedMeasureWindow->setChecked(false);
        }
        if (ui.checkBoxSelectLiquidAndSpeedMeasureWindow->isChecked()) {
            ui.checkBoxSelectLiquidAndSpeedMeasureWindow->setChecked(false);
        }
        if (ui.checkBoxSelectCamera->isChecked()) {
            ui.checkBoxSelectCamera->setChecked(false);
        }

        if (GetMainAppCrystalT2()->GetLiveImageView()->IsSpeedWindowSelected()) {
            GetMainAppCrystalT2()->GetLiveImageView()->SetSelectSpeedWindow(false);
            GetMainAppCrystalT2()->GetLiveImageView()->DrawMeasureWindow(ROI_ID_MEASURE_SPEED);
        }

        if (!GetMainAppCrystalT2()->GetLiveImageView()->IsLiquidWindowSelected()) {
            GetMainAppCrystalT2()->GetLiveImageView()->SetSelectLiquidWindow(true);
            GetMainAppCrystalT2()->GetLiveImageView()->DrawMeasureWindow(ROI_ID_MEASURE_LIQUID);
        }
        disconnect(ui.doubleSpinBoxWidth, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &EditProductDialog::SlotMeasureWidthChanged);
        disconnect(ui.doubleSpinBoxHeight, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &EditProductDialog::SlotMeasureHeightChanged);
        ui.doubleSpinBoxHeight->setValue(h);
        ui.doubleSpinBoxWidth->setValue(w);
        connect(ui.doubleSpinBoxWidth, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &EditProductDialog::SlotMeasureWidthChanged);
        connect(ui.doubleSpinBoxHeight, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &EditProductDialog::SlotMeasureHeightChanged);
        CheckEnableColorBoxesControls();
    } else {
        if (!ui.checkBoxSelectLiquidAndSpeedMeasureWindow->isChecked() && !ui.checkBoxSelectSpeedMeasureWindow->isChecked() && !ui.checkBoxSelectCamera->isChecked()) {
            ui.checkBoxSelectLiquidAndSpeedMeasureWindow->setChecked(true);
        }
    }
}

void EditProductDialog::SlotSelectSpeedMeasureWindowChanged(int state)
{
    if (state == Qt::Checked) {
        double h = ui.doubleSpinBoxMeasureSpeedHeight->value();
        double w = ui.doubleSpinBoxMeasureSpeedWidth->value();

        if (ui.checkBoxSelectLiquidMeasureWindow->isChecked()) {
            ui.checkBoxSelectLiquidMeasureWindow->setChecked(false);
        }
        if (ui.checkBoxSelectLiquidAndSpeedMeasureWindow->isChecked()) {
            ui.checkBoxSelectLiquidAndSpeedMeasureWindow->setChecked(false);
        }
        if (ui.checkBoxSelectCamera->isChecked()) {
            ui.checkBoxSelectCamera->setChecked(false);
        }

        if (!GetMainAppCrystalT2()->GetLiveImageView()->IsSpeedWindowSelected()) {
            GetMainAppCrystalT2()->GetLiveImageView()->SetSelectSpeedWindow(true);
            GetMainAppCrystalT2()->GetLiveImageView()->DrawMeasureWindow(ROI_ID_MEASURE_SPEED);
        }

        if (GetMainAppCrystalT2()->GetLiveImageView()->IsLiquidWindowSelected()) {
            GetMainAppCrystalT2()->GetLiveImageView()->SetSelectLiquidWindow(false);
            GetMainAppCrystalT2()->GetLiveImageView()->DrawMeasureWindow(ROI_ID_MEASURE_LIQUID);
        }

        disconnect(ui.doubleSpinBoxWidth, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &EditProductDialog::SlotMeasureWidthChanged);
        disconnect(ui.doubleSpinBoxHeight, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &EditProductDialog::SlotMeasureHeightChanged);
        ui.doubleSpinBoxHeight->setValue(h);
        ui.doubleSpinBoxWidth->setValue(w);
        connect(ui.doubleSpinBoxWidth, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &EditProductDialog::SlotMeasureWidthChanged);
        connect(ui.doubleSpinBoxHeight, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &EditProductDialog::SlotMeasureHeightChanged);
        CheckEnableColorBoxesControls();
    } else {
        if (!ui.checkBoxSelectLiquidMeasureWindow->isChecked() && !ui.checkBoxSelectLiquidAndSpeedMeasureWindow->isChecked() && !ui.checkBoxSelectCamera->isChecked()) {
            ui.checkBoxSelectLiquidMeasureWindow->setChecked(true);
        }
    }
}

void EditProductDialog::SlotSelectLiquidAndSpeedMeasureWindowChanged(int state)
{
    if (state == Qt::Checked) {
        if (ui.checkBoxSelectSpeedMeasureWindow->isChecked()) {
            ui.checkBoxSelectSpeedMeasureWindow->setChecked(false);
        }
        if (ui.checkBoxSelectLiquidMeasureWindow->isChecked()) {
            ui.checkBoxSelectLiquidMeasureWindow->setChecked(false);
        }
        if (ui.checkBoxSelectCamera->isChecked()) {
            ui.checkBoxSelectCamera->setChecked(false);
        }

        if (!GetMainAppCrystalT2()->GetLiveImageView()->IsSpeedWindowSelected()) {
            GetMainAppCrystalT2()->GetLiveImageView()->SetSelectSpeedWindow(true);
            GetMainAppCrystalT2()->GetLiveImageView()->DrawMeasureWindow(ROI_ID_MEASURE_SPEED);
        }

        if (!GetMainAppCrystalT2()->GetLiveImageView()->IsLiquidWindowSelected()) {
            GetMainAppCrystalT2()->GetLiveImageView()->SetSelectLiquidWindow(true);
            GetMainAppCrystalT2()->GetLiveImageView()->DrawMeasureWindow(ROI_ID_MEASURE_LIQUID);
        }
        CheckEnableColorBoxesControls();
    } else {
        if (!ui.checkBoxSelectSpeedMeasureWindow->isChecked() && !ui.checkBoxSelectLiquidMeasureWindow->isChecked() && !ui.checkBoxSelectCamera->isChecked()) {
            ui.checkBoxSelectSpeedMeasureWindow->setChecked(true);
        }
    }
}

void EditProductDialog::CheckEnableColorBoxesControls()
{
    if (!ui.checkBoxSelectLiquidMeasureWindow->isChecked() && !ui.checkBoxSelectSpeedMeasureWindow->isChecked()) {
        ui.doubleSpinBoxHeight->setEnabled(false);
        ui.doubleSpinBoxWidth->setEnabled(false);
    } else {
        if (ui.checkBoxSelectLiquidMeasureWindow->isChecked() || ui.checkBoxSelectSpeedMeasureWindow->isChecked()) {
            ui.doubleSpinBoxHeight->setEnabled(true);
            ui.doubleSpinBoxWidth->setEnabled(true);
        } else {
            ui.doubleSpinBoxHeight->setEnabled(false);
            ui.doubleSpinBoxWidth->setEnabled(false);
        }
    }
}

void EditProductDialog::SlotPulseLeftValveChanged()
{
    if (GetMainAppCrystalT2()) {
        ValveDialog* pValveDialog = GetMainAppCrystalT2()->GetValveDialogByID(LEFT_VALVE_ID);
        if (pValveDialog) {
            pValveDialog->WriteValveParameterPulse(ui.doubleSpinBoxPulseLeftValve->value());
        }
    }
}

void EditProductDialog::SlotPulseRightValveChanged()
{
    if (GetMainAppCrystalT2()) {
        ValveDialog* pValveDialog = GetMainAppCrystalT2()->GetValveDialogByID(RIGHT_VALVE_ID);
        if (pValveDialog) {
            pValveDialog->WriteValveParameterPulse(ui.doubleSpinBoxPulseRightValve->value());
        }
    }
}

void EditProductDialog::SetResponseValuePulseFromController(int ValveID, double set)
{
    if (ValveID == LEFT_VALVE_ID) {
        disconnect(ui.doubleSpinBoxPulseLeftValve, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotPulseLeftValveChanged);
        ui.doubleSpinBoxPulseLeftValve->setValue(set);
        connect(ui.doubleSpinBoxPulseLeftValve, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotPulseLeftValveChanged);
    }

    if (ValveID == RIGHT_VALVE_ID) {
        disconnect(ui.doubleSpinBoxPulseRightValve, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotPulseRightValveChanged);
        ui.doubleSpinBoxPulseRightValve->setValue(set);
        connect(ui.doubleSpinBoxPulseRightValve, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotPulseRightValveChanged);
    }
}

bool EditProductDialog::eventFilter(QObject* obj, QEvent* pEvent)
{
    if (obj == ui.pushButtonUp || obj == ui.pushButtonDown || obj == ui.pushButtonLeft || obj == ui.pushButtonRight) {
        switch (pEvent->type()) {
            case QEvent::TouchBegin:
                if (obj == ui.pushButtonUp) {
                    SlotMoveUp();
                    if (m_TimerAutorepeatMoveUp) {
                        m_TimerAutorepeatMoveUp->setInterval(m_AutoRepeatDelay);
                        m_TimerAutorepeatMoveUp->start();
                    }
                    ui.pushButtonUp->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::green.red()).arg(HMIColor::green.green()).arg(HMIColor::green.blue()));
                }
                if (obj == ui.pushButtonDown) {
                    SlotMoveDown();
                    if (m_TimerAutorepeatMoveDown) {
                        m_TimerAutorepeatMoveDown->setInterval(m_AutoRepeatDelay);
                        m_TimerAutorepeatMoveDown->start();
                    }
                    ui.pushButtonDown->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::green.red()).arg(HMIColor::green.green()).arg(HMIColor::green.blue()));
                }
                if (obj == ui.pushButtonLeft) {
                    SlotMoveToLeft();
                    if (m_TimerAutorepeatMoveLeft) {
                        m_TimerAutorepeatMoveLeft->setInterval(m_AutoRepeatDelay);
                        m_TimerAutorepeatMoveLeft->start();
                    }
                    ui.pushButtonLeft->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::green.red()).arg(HMIColor::green.green()).arg(HMIColor::green.blue()));
                }
                if (obj == ui.pushButtonRight) {
                    SlotMoveToRight();
                    if (m_TimerAutorepeatMoveRight) {
                        m_TimerAutorepeatMoveRight->setInterval(m_AutoRepeatDelay);
                        m_TimerAutorepeatMoveRight->start();
                    }
                    ui.pushButtonRight->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::green.red()).arg(HMIColor::green.green()).arg(HMIColor::green.blue()));
                }
                return true;
                break;
            case QEvent::TouchEnd:
                if (obj == ui.pushButtonUp) {
                    if (m_TimerAutorepeatMoveUp) {
                        m_TimerAutorepeatMoveUp->stop();
                    }
                    ui.pushButtonUp->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::Grey.red()).arg(HMIColor::Grey.green()).arg(HMIColor::Grey.blue()));
                }
                if (obj == ui.pushButtonDown) {
                    if (m_TimerAutorepeatMoveDown) {
                        m_TimerAutorepeatMoveDown->stop();
                    }
                    ui.pushButtonDown->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::Grey.red()).arg(HMIColor::Grey.green()).arg(HMIColor::Grey.blue()));
                }
                if (obj == ui.pushButtonLeft) {
                    if (m_TimerAutorepeatMoveLeft) {
                        m_TimerAutorepeatMoveLeft->stop();
                    }
                    ui.pushButtonLeft->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::Grey.red()).arg(HMIColor::Grey.green()).arg(HMIColor::Grey.blue()));
                }
                if (obj == ui.pushButtonRight) {
                    if (m_TimerAutorepeatMoveRight) {
                        m_TimerAutorepeatMoveRight->stop();
                    }
                    ui.pushButtonRight->setStyleSheet(QString("background-color:rgb(%1, %2, %3);").arg(HMIColor::Grey.red()).arg(HMIColor::Grey.green()).arg(HMIColor::Grey.blue()));
                }
                return true;
                break;
            default:
                break;
        }
    }
    return false;
}

void EditProductDialog::SlotRunCameraSimulation()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) {
        GetMainAppCrystalT2()->GetImageData()->SetVideoStateCameraSimulation(PLAY_VIDEO_CAMERA_SIMULATION);
    }
}

void EditProductDialog::SlotStepCameraSimulation()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) {
        GetMainAppCrystalT2()->GetImageData()->SetVideoStateCameraSimulation(STEP_VIDEO_CAMERA_SIMULATION);
    }
}

void EditProductDialog::SlotStopCameraSimulation()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) {
        GetMainAppCrystalT2()->GetImageData()->SetVideoStateCameraSimulation(STOP_VIDEO_CAMERA_SIMULATION);
    }
}

void EditProductDialog::SlotPlayOneVideoCameraSimulation()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) {
        GetMainAppCrystalT2()->GetImageData()->SetVideoStateCameraSimulation(PLAY_ONE_VIDEO_CAMERA_SIMULATION);
    }
}

// void EditProductDialog::StartManuelTriggerValve(int ValveID, int triggerCounts)
//{
//    ValveDialog* pValveDialog = GetMainAppCrystalT2()->GetValveDialogByID(ValveID);
//    if (pValveDialog) {
//        ProductData* pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
//        if (pProductData && pProductData->ContainsValveKey(ValveID)) {
//            ValveData CurrentValveData = pProductData->GetValveData(ValveID);
//            int InjectionTime = CurrentValveData.m_ValvePauseTimeInms * triggerCounts * 1.2;
//            pValveDialog->WriteValveParameterCount(triggerCounts);
//            pValveDialog->WaitCommandIsSet(1000);
//            pValveDialog->SetTrigger(true);  // Trigger kurz auslösen
//            QThread::currentThread()->msleep(50);
//            pValveDialog->SetTrigger(false);
//            QThread::currentThread()->msleep(InjectionTime);
//            pValveDialog->WriteValveParameterCount(1);  // Auf alten wert zurück
//            pValveDialog->WaitCommandIsSet(1000);
//        }
//    }
//    emit SignalManuelTriggerReady(ValveID);
//}
//
// void EditProductDialog::_StartManuelTriggerValve(int valveID, int triggerCounts)
//{
//    if (GetMainAppCrystalT2()) {
//        ValveDialog* pValveDialog = GetMainAppCrystalT2()->GetValveDialogByID(valveID);
//        if (pValveDialog) {
//            ProductData* pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
//            if (pProductData && pProductData->ContainsValveKey(valveID)) {
//                if (GetMainAppCrystalT2()->GetSettingsData()) {
//                    for (int i = 0; i < triggerCounts; i++) {
//                        pValveDialog->SetTrigger(true);  // Trigger kurz auslösen
//                        QThread::currentThread()->msleep(50);
//                        pValveDialog->SetTrigger(false);
//                        QThread::currentThread()->msleep(GetMainAppCrystalT2()->GetSettingsData()->m_PauseTriggerManualInMs);
//                    }
//                }
//            }
//        }
//        emit SignalManuelTriggerReady(valveID);
//    }
//}

void EditProductDialog::SlotCalibrateAbortIsClicked()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) GetMainAppCrystalT2()->GetImageData()->SetAutoCalibrateIsOn(false, true);  // hier Info Abbruch durch Benutzer
}

void EditProductDialog::SlotSetCalibrateStatus(const QString& Info, int value)
{
    if (m_PopupProgressBar) {
        QString text = Info;
        if (value < 100)
            m_PopupProgressBar->SetInfoTextPleaseWait(text);
        else
            m_PopupProgressBar->SetMessageBoxSecondTextInfo(text);
        m_PopupProgressBar->SlotSetValue(value);
    }
}

void EditProductDialog::SetCurrentMaschineState(PluginInterface::MachineState set)
{
    emit SignalEnableGUIElements(set);
}

void EditProductDialog::SlotEnableGUIElements(int State)
{
    if (State == PluginInterface::MachineState::Production) {
        ui.pushButtonCalibrate->hide();
        ui.groupBoxColorBoxesControls->hide();
        SlotSelectLiquidMeasureWindowChanged(Qt::Checked);  // Damit Camera - Viewport Offsetanzeige im Livebild verschwindet
    } else {
        ui.pushButtonCalibrate->show();
        ui.groupBoxColorBoxesControls->show();
    }
}

void EditProductDialog::SetCounterBottleFilled(int set)
{
    ui.doubleSpinBoxBottleFilled->setValue(set);
}

void EditProductDialog::SetCounterBottleNotFilled(int set)
{
    ui.doubleSpinBoxBottleNotFilled->setValue(set);
}

void EditProductDialog::SetCounterMiddleTooLow(int set)
{
    ui.doubleSpinBoxCounterMiddleTooLow->setValue(set);
}

void EditProductDialog::SetCounterLeftTooBig(int set)
{
    ui.doubleSpinBoxCounterLeftTooBig->setValue(set);
}

void EditProductDialog::SetCounterRightTooBig(int set)
{
    ui.doubleSpinBoxCounterRightTooBig->setValue(set);
}

void EditProductDialog::SetCounterBottleNotInPos(int set)
{
    ui.doubleSpinBoxCounterBottleNotInPos->setValue(set);
}

void EditProductDialog::SetCounterSizeIsOutOfTol(unsigned long long set)
{
    if (set == 0) {
        m_ProductDiameterIsOutOfTol = false;
        m_CounterSizeNotOk = set;
    } else {
        if (set != m_CounterSizeNotOk) {
            m_ProductDiameterIsOutOfTol = true;
            m_ProductEdgeNotFound = false;
            m_CounterSizeNotOk = set;
        }
    }
}

void EditProductDialog::SetCounterEdgeIsOutOfTol(unsigned long long set)
{
    if (set == 0) {
        m_ProductEdgeNotFound = false;
        m_CounterEdgeNotOk = set;
    } else {
        if (set != m_CounterEdgeNotOk) {
            m_ProductEdgeNotFound = true;
            m_ProductDiameterIsOutOfTol = false;
            m_CounterEdgeNotOk = set;
        }
    }
}

void EditProductDialog::SetCounterContrastIsOutOfTol(unsigned long long set)
{
    // ui.doubleSpinBoxCounterContrastNotOk->setValue(set);
}

void EditProductDialog::SlotResetCounters()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) {
        QString Time = QDateTime::currentDateTime().time().toString("hh:mm:ss");
        QString Date = QDateTime::currentDateTime().date().toString("dd.MM.yyyy ");
        QString Text = Date + Time;
        ui.pushButtonResetCounters->setText(tr("Reset Counters[Last:%1]").arg(Text));
        SetCounterBottleFilled(0);
        SetCounterBottleNotFilled(0);
        SetCounterMiddleTooLow(0);
        SetCounterLeftTooBig(0);
        SetCounterRightTooBig(0);
        SetCounterBottleNotInPos(0);
        SetCounterSizeIsOutOfTol(0);
        SetCounterEdgeIsOutOfTol(0);
        SetCounterContrastIsOutOfTol(0);
        GetMainAppCrystalT2()->GetImageData()->ResetAllCounters();
    }
}

void EditProductDialog::SetMeasuredNeckDiameter(double set, bool Status)
{
    if (Status) {  // neuer messwert
        ui.doubleSpinBoxMeasureNeckDiameter->setValue(set);
        m_CounterShowNeckDiameter = 0;
        ui.doubleSpinBoxMeasureNeckDiameter->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Ok");  // Set Background Color
        ui.doubleSpinBoxMeasureNeckDiameter->style()->unpolish(ui.doubleSpinBoxMeasureNeckDiameter);
        ui.doubleSpinBoxMeasureNeckDiameter->style()->polish(ui.doubleSpinBoxMeasureNeckDiameter);

        ui.doubleSpinBoxResultInfo->setSpecialValueText(tr("Product Found"));
        ui.doubleSpinBoxResultInfo->setValue(-1.0);
        ui.doubleSpinBoxResultInfo->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Ok");  // Set Background Color
        ui.doubleSpinBoxResultInfo->style()->unpolish(ui.doubleSpinBoxResultInfo);
        ui.doubleSpinBoxResultInfo->style()->polish(ui.doubleSpinBoxResultInfo);
        m_ProductDiameterIsOutOfTol = false;
        m_ProductEdgeNotFound = false;
    } else {  // noch alter messwert
        m_CounterShowNeckDiameter++;
        if (m_CounterShowNeckDiameter > m_NumberSameMeasureValues) {  // wenn n mal der selbe Wert, dann Product nicht gefunden
            ui.doubleSpinBoxMeasureNeckDiameter->setValue(-1.0);
            ui.doubleSpinBoxMeasureNeckDiameter->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Warning");  // Set Background Color
            ui.doubleSpinBoxMeasureNeckDiameter->style()->unpolish(ui.doubleSpinBoxMeasureNeckDiameter);
            ui.doubleSpinBoxMeasureNeckDiameter->style()->polish(ui.doubleSpinBoxMeasureNeckDiameter);
            m_CounterShowNeckDiameter = 0;

            if (m_ProductEdgeNotFound) {
                ui.doubleSpinBoxResultInfo->setSpecialValueText(tr("No Edge Found"));
                ui.doubleSpinBoxResultInfo->setValue(-1.0);
                ui.doubleSpinBoxResultInfo->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Warning");  // Set Background Color
                ui.doubleSpinBoxResultInfo->style()->unpolish(ui.doubleSpinBoxResultInfo);
                ui.doubleSpinBoxResultInfo->style()->polish(ui.doubleSpinBoxResultInfo);
            } else {
                if (m_ProductDiameterIsOutOfTol) {
                    ui.doubleSpinBoxResultInfo->setSpecialValueText(tr("Size Not In Tol."));
                    ui.doubleSpinBoxResultInfo->setValue(-1.0);
                    ui.doubleSpinBoxResultInfo->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Warning");  // Set Background Color
                    ui.doubleSpinBoxResultInfo->style()->unpolish(ui.doubleSpinBoxResultInfo);
                    ui.doubleSpinBoxResultInfo->style()->polish(ui.doubleSpinBoxResultInfo);
                } else {
                    ui.doubleSpinBoxResultInfo->setSpecialValueText(tr("No Edge Found"));
                    ui.doubleSpinBoxResultInfo->setValue(-1.0);
                    ui.doubleSpinBoxResultInfo->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Warning");  // Set Background Color
                    ui.doubleSpinBoxResultInfo->style()->unpolish(ui.doubleSpinBoxResultInfo);
                    ui.doubleSpinBoxResultInfo->style()->polish(ui.doubleSpinBoxResultInfo);
                }
            }
        }
    }
}

//       side==LEFT_TRIGGER_SIDE_INDEX       side==RIGHT_TRIGGER_SIDE_INDEX
//                    v1 <-Valvedistance-> v2
// x,y _____________________________________________________
//     |            |     |             |      |            |
//     |            |     |             |      |            |
//     |  Left ROI  |Mid. |Right  Left  | Mid. | Right ROI  |   h
//     |            | ROI | ROI    ROI  | ROI  |            |
//     |____________|_____|_____________|______|____________|
//
//
//     |________________________|
//                     V
//    Messbereich erster Befüllvorgang Breite w Linke Triggerseite
//
//                              |___________________________|
//                                                V
//                               Messbereich zweiter Befüllvorgang Breite w Rechte Triggerseite
void EditProductDialog::SetAmountOfSplashesLeftOnLeftTriggerSide(int Amount)
{
    ui.doubleSpinBoxAmountOfLiquidLeftLeftTriggerSide->setValue(Amount);

    if (Amount > GetThresholdSplashes())
        ui.doubleSpinBoxAmountOfLiquidLeftLeftTriggerSide->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Error");  // Set Background Color
    else
        ui.doubleSpinBoxAmountOfLiquidLeftLeftTriggerSide->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Ok");  // Set Background Color
    ui.doubleSpinBoxAmountOfLiquidLeftLeftTriggerSide->style()->unpolish(ui.doubleSpinBoxAmountOfLiquidLeftLeftTriggerSide);
    ui.doubleSpinBoxAmountOfLiquidLeftLeftTriggerSide->style()->polish(ui.doubleSpinBoxAmountOfLiquidLeftLeftTriggerSide);
}

void EditProductDialog::SetAmountOfSplashesLeftOnRightTriggerSide(int Amount)
{
    ui.doubleSpinBoxAmountOfLiquidLeftRightTriggerSide->setValue(Amount);
    if (Amount > GetThresholdSplashes())
        ui.doubleSpinBoxAmountOfLiquidLeftRightTriggerSide->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Error");  // Set Background Color
    else
        ui.doubleSpinBoxAmountOfLiquidLeftRightTriggerSide->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Ok");  // Set Background Color
    ui.doubleSpinBoxAmountOfLiquidLeftRightTriggerSide->style()->unpolish(ui.doubleSpinBoxAmountOfLiquidLeftRightTriggerSide);
    ui.doubleSpinBoxAmountOfLiquidLeftRightTriggerSide->style()->polish(ui.doubleSpinBoxAmountOfLiquidLeftRightTriggerSide);
}

void EditProductDialog::SetAmountOfSplashesRightOnLeftTriggerSide(int Amount)
{
    ui.doubleSpinBoxAmountOfLiquidRightLeftTriggerSide->setValue(Amount);
    if (Amount > GetThresholdSplashes())
        ui.doubleSpinBoxAmountOfLiquidRightLeftTriggerSide->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Error");  // Set Background Color
    else
        ui.doubleSpinBoxAmountOfLiquidRightLeftTriggerSide->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Ok");  // Set Background Color
    ui.doubleSpinBoxAmountOfLiquidRightLeftTriggerSide->style()->unpolish(ui.doubleSpinBoxAmountOfLiquidRightLeftTriggerSide);
    ui.doubleSpinBoxAmountOfLiquidRightLeftTriggerSide->style()->polish(ui.doubleSpinBoxAmountOfLiquidRightLeftTriggerSide);
}

void EditProductDialog::SetAmountOfSplashesRightOnRightTriggerSide(int Amount)
{
    ui.doubleSpinBoxAmountOfLiquidRightRightTriggerSide->setValue(Amount);
    if (Amount > GetThresholdSplashes())
        ui.doubleSpinBoxAmountOfLiquidRightRightTriggerSide->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Error");  // Set Background Color
    else
        ui.doubleSpinBoxAmountOfLiquidRightRightTriggerSide->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Ok");  // Set Background Color
    ui.doubleSpinBoxAmountOfLiquidRightRightTriggerSide->style()->unpolish(ui.doubleSpinBoxAmountOfLiquidRightRightTriggerSide);
    ui.doubleSpinBoxAmountOfLiquidRightRightTriggerSide->style()->polish(ui.doubleSpinBoxAmountOfLiquidRightRightTriggerSide);
}

void EditProductDialog::SetAmountOfLiquidMiddleRightTriggerSide(double AmountLiquidRightTrigger, double standardDevation, double SumLeftRightAmountLiqiud)
{
    Q_UNUSED(SumLeftRightAmountLiqiud);  // nicht genutzt da Schwellwert sich auf ein Ventil bezieht
    QString suffix = GetSuffixStandardDev(standardDevation);

    ui.doubleSpinBoxAmountOfLiquidMiddleRightSide->setValue(AmountLiquidRightTrigger);
    ui.doubleSpinBoxAmountOfLiquidMiddleRightSide->setSuffix(suffix);

    if (AmountLiquidRightTrigger < ui.doubleSpinBoxMinAcceptanceThresholdLiquidMiddleROI->value() || AmountLiquidRightTrigger > ui.doubleSpinBoxMaxAcceptanceThresholdLiquidMiddleROI->value())
        ui.doubleSpinBoxAmountOfLiquidMiddleRightSide->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Error");  // Amount of Liquid is out of range
    else
        ui.doubleSpinBoxAmountOfLiquidMiddleRightSide->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Ok");  // Amount of Liquid is ok
    ui.doubleSpinBoxAmountOfLiquidMiddleRightSide->style()->unpolish(ui.doubleSpinBoxAmountOfLiquidMiddleRightSide);
    ui.doubleSpinBoxAmountOfLiquidMiddleRightSide->style()->polish(ui.doubleSpinBoxAmountOfLiquidMiddleRightSide);
}

void EditProductDialog::ResetAmountOfLiquidMiddleRightTriggerSide()
{
    ui.doubleSpinBoxAmountOfLiquidMiddleRightSide->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Ok");  // Amount of Liquid is ok
    ui.doubleSpinBoxAmountOfLiquidMiddleRightSide->style()->unpolish(ui.doubleSpinBoxAmountOfLiquidMiddleRightSide);
    ui.doubleSpinBoxAmountOfLiquidMiddleRightSide->style()->polish(ui.doubleSpinBoxAmountOfLiquidMiddleRightSide);
}

void EditProductDialog::SetAmountOfLiquidMiddleLeftTriggerSide(double AmountLiqiudLeftTrigger, double standardDevation, double SumLeftRightAmountLiqiud)
{
    Q_UNUSED(SumLeftRightAmountLiqiud);  // nicht genutzt da Schwellwert sich auf ein Ventil bezieht
    QString suffix = GetSuffixStandardDev(standardDevation);

    ui.doubleSpinBoxAmountOfLiquidMiddleLeftSide->setValue(AmountLiqiudLeftTrigger);
    ui.doubleSpinBoxAmountOfLiquidMiddleLeftSide->setSuffix(suffix);

    if (AmountLiqiudLeftTrigger < ui.doubleSpinBoxMinAcceptanceThresholdLiquidMiddleROI->value() || AmountLiqiudLeftTrigger > ui.doubleSpinBoxMaxAcceptanceThresholdLiquidMiddleROI->value())
        ui.doubleSpinBoxAmountOfLiquidMiddleLeftSide->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Error");  // Amount of Liquid is out of range
    else
        ui.doubleSpinBoxAmountOfLiquidMiddleLeftSide->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Ok");  // Amount of Liquid is ok
    ui.doubleSpinBoxAmountOfLiquidMiddleLeftSide->style()->unpolish(ui.doubleSpinBoxAmountOfLiquidMiddleLeftSide);
    ui.doubleSpinBoxAmountOfLiquidMiddleLeftSide->style()->polish(ui.doubleSpinBoxAmountOfLiquidMiddleLeftSide);
}

void EditProductDialog::ResetAmountOfLiquidMiddleLeftTriggerSide()
{
    ui.doubleSpinBoxAmountOfLiquidMiddleLeftSide->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Ok");  // Amount of Liquid is ok
    ui.doubleSpinBoxAmountOfLiquidMiddleLeftSide->style()->unpolish(ui.doubleSpinBoxAmountOfLiquidMiddleLeftSide);
    ui.doubleSpinBoxAmountOfLiquidMiddleLeftSide->style()->polish(ui.doubleSpinBoxAmountOfLiquidMiddleLeftSide);
}

void EditProductDialog::ResetIsClicked()
{
    emit SignalResetLiquidResults();
}
// reset is clicked
void EditProductDialog::SlotResetLiquidResults()
{
    SetAmountOfSplashesLeftOnLeftTriggerSide(0);
    SetAmountOfSplashesLeftOnRightTriggerSide(0);
    SetAmountOfSplashesRightOnLeftTriggerSide(0);
    SetAmountOfSplashesRightOnRightTriggerSide(0);
    ResetAmountOfLiquidMiddleLeftTriggerSide();
    ResetAmountOfLiquidMiddleRightTriggerSide();
    SlotResetCounters();
}

double EditProductDialog::GetThresholdSplashes()
{
    return ui.doubleSpinBoxAcceptanceThresholdLiquidLeftAndRightROI->value();
}

QString EditProductDialog::GetSuffixStandardDev(double standardDevation)
{
    QChar PlusMinusSymbol = (0x00B1);
    QString frontText = QString(" %1").arg(PlusMinusSymbol);
    const int numChars = 8;

    return QString("%1%2").arg(frontText).arg(standardDevation, -numChars, 'f', 0);
}

// not in use
void EditProductDialog::SetAmountOfLiquidMiddle(double Amount, double standardDevation)
{
    QString suffix = GetSuffixStandardDev(standardDevation);

    ui.doubleSpinBoxAmountOfLiquidMiddleLeftSide->setValue(Amount);
    ui.doubleSpinBoxAmountOfLiquidMiddleLeftSide->setSuffix(suffix);
    if (Amount < ui.doubleSpinBoxMinAcceptanceThresholdLiquidMiddleROI->value())
        ui.doubleSpinBoxAmountOfLiquidMiddleLeftSide->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Error");  // Set Background Color
    else
        ui.doubleSpinBoxAmountOfLiquidMiddleLeftSide->setProperty(PROPERTY_NAME_CHANGE_BACKGROUND_COLOR.toLatin1(), "Ok");  // Set Background Color
    ui.doubleSpinBoxAmountOfLiquidMiddleLeftSide->style()->unpolish(ui.doubleSpinBoxAmountOfLiquidMiddleLeftSide);
    ui.doubleSpinBoxAmountOfLiquidMiddleLeftSide->style()->polish(ui.doubleSpinBoxAmountOfLiquidMiddleLeftSide);
}

void EditProductDialog::SetLiveImageSize(int w, int h)
{
    w = w + 8;
    h = h + 8;
    ui.ImageFrame->setMaximumWidth(w);
    ui.ImageFrame->setMaximumHeight(h);
    ui.ImageFrame->setMinimumWidth(w);
    ui.ImageFrame->setMinimumHeight(h);
    ui.frameBandDirection->setMaximumWidth(w);
    ui.frameBandDirection->setMinimumWidth(w);
}

void EditProductDialog::SetLineDataGradient(const double* Array, int size, double EdgeLeftSide, double EdgeRightSide)
{
    if (GetPlotLineProfil()) GetPlotLineProfil()->SetLineDataGradient(Array, size, EdgeLeftSide, EdgeRightSide);
}

bool EditProductDialog::IsPlotLineProfilVisible()
{
    if (GetPlotLineProfil())
        return GetPlotLineProfil()->isVisible();
    else
        return true;
}

void EditProductDialog::SetForceEditingFinishedSpeedWindow()
{
    ui.doubleSpinBoxMeasureSpeedHeight->setFocus();
    ui.doubleSpinBoxMeasureSpeedYPosition->setFocus();
    ui.doubleSpinBoxMeasureSpeedWidth->setFocus();
    ui.doubleSpinBoxMeasureSpeedHeight->setFocus();

    ui.doubleSpinBoxMeasureSpeedHeight->clearFocus();
    ui.doubleSpinBoxMeasureSpeedYPosition->clearFocus();
    ui.doubleSpinBoxMeasureSpeedWidth->clearFocus();
    ui.doubleSpinBoxMeasureSpeedHeight->clearFocus();
}

void EditProductDialog::SetForceEditingFinishedInjectionWindow()
{
    ui.doubleSpinBoxMeasureLiquidXPosition->setFocus();
    ui.doubleSpinBoxMeasureLiquidYPosition->setFocus();
    ui.doubleSpinBoxMeasureLiquidWidth->setFocus();
    ui.doubleSpinBoxMeasureLiquidHeight->setFocus();

    ui.doubleSpinBoxMeasureLiquidXPosition->clearFocus();
    ui.doubleSpinBoxMeasureLiquidYPosition->clearFocus();
    ui.doubleSpinBoxMeasureLiquidWidth->clearFocus();
    ui.doubleSpinBoxMeasureLiquidHeight->clearFocus();
}

void EditProductDialog::UpdateMesureWindow(int MeasureWindowID)
{
    m_WindowSetup = false;
    QRect CurrentRect;

    if (GetMainAppCrystalT2() && (MeasureWindowID == ROI_ID_MEASURE_SPEED || MeasureWindowID == ROI_ID_MEASURE_LIQUID)) {
        CurrentRect = GetMainAppCrystalT2()->GetMeasureWindowRect(MeasureWindowID);

        if (MeasureWindowID == ROI_ID_MEASURE_SPEED) {
            ui.doubleSpinBoxMeasureSpeedXPosition->setValue(CurrentRect.x());
            ui.doubleSpinBoxMeasureSpeedYPosition->setValue(CurrentRect.y());
            ui.doubleSpinBoxMeasureSpeedWidth->setValue(CurrentRect.width());
            ui.doubleSpinBoxMeasureSpeedHeight->setValue(CurrentRect.height());

            if (ui.checkBoxSelectSpeedMeasureWindow->isChecked()) {
                ui.doubleSpinBoxWidth->setValue(CurrentRect.width());
                ui.doubleSpinBoxHeight->setValue(CurrentRect.height());
            }
            // Setze Focus für AudiTaril damit editingFinished aufgerufen wird
            SetForceEditingFinishedSpeedWindow();
        } else {
            ui.doubleSpinBoxMeasureLiquidXPosition->setValue(CurrentRect.x());
            ui.doubleSpinBoxMeasureLiquidYPosition->setValue(CurrentRect.y());
            ui.doubleSpinBoxMeasureLiquidWidth->setValue(CurrentRect.width());
            ui.doubleSpinBoxMeasureLiquidHeight->setValue(CurrentRect.height());

            if (ui.checkBoxSelectLiquidMeasureWindow->isChecked()) {
                ui.doubleSpinBoxWidth->setValue(CurrentRect.width());
                ui.doubleSpinBoxHeight->setValue(CurrentRect.height());
            }
            // Setze Focus für AudiTaril damit editingFinished aufgerufen wird
            SetForceEditingFinishedInjectionWindow();
        }
    }
    m_WindowSetup = true;
}

void EditProductDialog::SlotROIMeasureSpeedXPosChanged()
{
    ROISpeedXYPositionChanged();
}

void EditProductDialog::SlotROIMeasureSpeedYPosChanged()
{
    ROISpeedXYPositionChanged();
}

void EditProductDialog::ROISpeedXYPositionChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetLiveImageView() && GetMainAppCrystalT2()->GetImageData()) {
        QRect NewROIRectMeasureSpeed, CurrentROIRectSpeed;
        QRect NewROIRectMeasureBottleOnTriggerPos, CurrentROIRectBottleOnTriggerPos;
        int NewXPos = static_cast<int>(ui.doubleSpinBoxMeasureSpeedXPosition->value());
        int NewYPos = static_cast<int>(ui.doubleSpinBoxMeasureSpeedYPosition->value());
        int MaxWidth = GetMainAppCrystalT2()->GetImageData()->GetImageWidth();

        if (GetMainAppCrystalT2()->IsFirstTriggerOnLeftSide()) MaxWidth = static_cast<int>(ui.doubleSpinBoxMeasureLiquidXPosition->value());
        CurrentROIRectSpeed = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRect(ROI_ID_MEASURE_SPEED);
        CurrentROIRectBottleOnTriggerPos = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRect(ROI_ID_MEASURE_BOTTLE_UNDER_VALVE);
        // grenzen überprüfen
        if (NewXPos < 0) NewXPos = 0;
        if (NewYPos < 0) NewYPos = 0;
        if ((NewXPos + CurrentROIRectSpeed.width()) >= MaxWidth) {
            NewXPos = MaxWidth - CurrentROIRectSpeed.width();
            if (m_MeasureWindowSpinBoxesHasSlotConnection) {
                disconnect(ui.doubleSpinBoxMeasureSpeedXPosition, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROIMeasureSpeedXPosChanged);
            }
            ui.doubleSpinBoxMeasureSpeedXPosition->setValue(NewXPos);
            if (m_MeasureWindowSpinBoxesHasSlotConnection) {
                connect(ui.doubleSpinBoxMeasureSpeedXPosition, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROIMeasureSpeedXPosChanged);
            }
        }
        if ((NewYPos + CurrentROIRectSpeed.height()) >= GetMainAppCrystalT2()->GetImageData()->GetImageHeight()) {
            NewYPos = GetMainAppCrystalT2()->GetImageData()->GetImageHeight() - CurrentROIRectSpeed.height();
            if (m_MeasureWindowSpinBoxesHasSlotConnection) {
                disconnect(ui.doubleSpinBoxMeasureSpeedYPosition, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROIMeasureSpeedYPosChanged);
            }
            ui.doubleSpinBoxMeasureSpeedYPosition->setValue(NewYPos);
            if (m_MeasureWindowSpinBoxesHasSlotConnection) {
                connect(ui.doubleSpinBoxMeasureSpeedYPosition, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROIMeasureSpeedYPosChanged);
            }
        }
        NewROIRectMeasureSpeed.setX(NewXPos);
        NewROIRectMeasureSpeed.setY(NewYPos);
        NewROIRectMeasureSpeed.setWidth(CurrentROIRectSpeed.width());
        NewROIRectMeasureSpeed.setHeight(CurrentROIRectSpeed.height());

        NewROIRectMeasureBottleOnTriggerPos.setX(CurrentROIRectBottleOnTriggerPos.x());
        NewROIRectMeasureBottleOnTriggerPos.setY(NewYPos);
        NewROIRectMeasureBottleOnTriggerPos.setWidth(CurrentROIRectBottleOnTriggerPos.width());
        NewROIRectMeasureBottleOnTriggerPos.setHeight(CurrentROIRectBottleOnTriggerPos.height());

        GetMainAppCrystalT2()->GetImageData()->SetMeasureWindowRect(ROI_ID_MEASURE_BOTTLE_UNDER_VALVE, NewROIRectMeasureBottleOnTriggerPos);
        GetMainAppCrystalT2()->GetImageData()->SetMeasureWindowRect(ROI_ID_MEASURE_SPEED, NewROIRectMeasureSpeed);
        GetMainAppCrystalT2()->GetLiveImageView()->DrawMeasureWindow(ROI_ID_MEASURE_SPEED);
    }
}

void EditProductDialog::SlotROIMeasureSpeedWidthChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetLiveImageView() && GetMainAppCrystalT2()->GetImageData()) {
        int NewWidth = static_cast<int>(ui.doubleSpinBoxMeasureSpeedWidth->value());
        int MaxWidth = GetMainAppCrystalT2()->GetImageData()->GetImageWidth();
        QRect NewROIRectMeasureSpeed;
        QRect CurrentROIRectSpeed = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRect(ROI_ID_MEASURE_SPEED);
        bool OnLimit = false;

        if (GetMainAppCrystalT2()->IsFirstTriggerOnLeftSide()) MaxWidth = static_cast<int>(ui.doubleSpinBoxMeasureLiquidXPosition->value());

        if ((static_cast<int>(ui.doubleSpinBoxMeasureSpeedXPosition->value()) + NewWidth) >= MaxWidth) {
            NewWidth = MaxWidth - static_cast<int>(ui.doubleSpinBoxMeasureSpeedXPosition->value()) - 1;
            if (m_MeasureWindowSpinBoxesHasSlotConnection) {
                disconnect(ui.doubleSpinBoxMeasureSpeedWidth, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROIMeasureSpeedWidthChanged);
            }
            ui.doubleSpinBoxMeasureSpeedWidth->setValue(NewWidth);
            if (m_MeasureWindowSpinBoxesHasSlotConnection) {
                connect(ui.doubleSpinBoxMeasureSpeedWidth, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROIMeasureSpeedWidthChanged);
            }
            OnLimit = true;
        }

        if (NewWidth < MINIMUM_ROI_SIZE_IN_PIXEL) {
            NewWidth = MINIMUM_ROI_SIZE_IN_PIXEL;
            if (m_MeasureWindowSpinBoxesHasSlotConnection) {
                disconnect(ui.doubleSpinBoxMeasureSpeedWidth, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROIMeasureSpeedWidthChanged);
            }
            ui.doubleSpinBoxMeasureSpeedWidth->setValue(NewWidth);
            if (m_MeasureWindowSpinBoxesHasSlotConnection) {
                connect(ui.doubleSpinBoxMeasureSpeedWidth, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROIMeasureSpeedWidthChanged);
            }
            OnLimit = true;
        }
        if (OnLimit) {
            disconnect(ui.doubleSpinBoxWidth, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &EditProductDialog::SlotMeasureWidthChanged);
            ui.doubleSpinBoxWidth->setValue(NewWidth);
            connect(ui.doubleSpinBoxWidth, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &EditProductDialog::SlotMeasureWidthChanged);
        }
        NewROIRectMeasureSpeed.setX(CurrentROIRectSpeed.x());
        NewROIRectMeasureSpeed.setY(CurrentROIRectSpeed.y());
        NewROIRectMeasureSpeed.setWidth(NewWidth);
        NewROIRectMeasureSpeed.setHeight(CurrentROIRectSpeed.height());

        GetMainAppCrystalT2()->GetImageData()->SetMeasureWindowRect(ROI_ID_MEASURE_SPEED, NewROIRectMeasureSpeed);
        GetMainAppCrystalT2()->GetLiveImageView()->DrawMeasureWindow(ROI_ID_MEASURE_SPEED);
    }
}

void EditProductDialog::SlotROIMeasureSpeedHeightChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetLiveImageView() && GetMainAppCrystalT2()->GetImageData() && GetMainAppCrystalT2()->GetSettingsData()) {
        int NewHeight = static_cast<int>(ui.doubleSpinBoxMeasureSpeedHeight->value());
        QRect NewROIRectMeasureSpeed, CurrentROIRectSpeed;
        QRect NewROIRectMeasureBottleOnTriggerPos, CurrentROIRectSpeedBottleOnTriggerPos;

        CurrentROIRectSpeed = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRect(ROI_ID_MEASURE_SPEED);
        CurrentROIRectSpeedBottleOnTriggerPos = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRect(ROI_ID_MEASURE_BOTTLE_UNDER_VALVE);

        if ((static_cast<int>(ui.doubleSpinBoxMeasureSpeedYPosition->value()) + NewHeight) >= GetMainAppCrystalT2()->GetImageData()->GetImageHeight()) {
            NewHeight = GetMainAppCrystalT2()->GetImageData()->GetImageHeight() - static_cast<int>(ui.doubleSpinBoxMeasureSpeedYPosition->value()) - 1;
            if (m_MeasureWindowSpinBoxesHasSlotConnection) {
                disconnect(ui.doubleSpinBoxMeasureSpeedHeight, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROIMeasureSpeedHeightChanged);
            }
            ui.doubleSpinBoxMeasureSpeedHeight->setValue(NewHeight);
            if (m_MeasureWindowSpinBoxesHasSlotConnection) {
                connect(ui.doubleSpinBoxMeasureSpeedHeight, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROIMeasureSpeedHeightChanged);
            }
        }
        if (NewHeight < GetMainAppCrystalT2()->GetSettingsData()->m_MinMeasureWindowHeight) {
            NewHeight = GetMainAppCrystalT2()->GetSettingsData()->m_MinMeasureWindowHeight;
            if (m_MeasureWindowSpinBoxesHasSlotConnection) {
                disconnect(ui.doubleSpinBoxMeasureSpeedHeight, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROIMeasureSpeedHeightChanged);
            }
            ui.doubleSpinBoxMeasureSpeedHeight->setValue(NewHeight);
            if (m_MeasureWindowSpinBoxesHasSlotConnection) {
                connect(ui.doubleSpinBoxMeasureSpeedHeight, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROIMeasureSpeedHeightChanged);
            }
        }

        NewROIRectMeasureSpeed.setX(CurrentROIRectSpeed.x());
        NewROIRectMeasureSpeed.setY(CurrentROIRectSpeed.y());
        NewROIRectMeasureSpeed.setWidth(CurrentROIRectSpeed.width());
        NewROIRectMeasureSpeed.setHeight(NewHeight);

        NewROIRectMeasureBottleOnTriggerPos.setX(CurrentROIRectSpeedBottleOnTriggerPos.x());
        NewROIRectMeasureBottleOnTriggerPos.setY(CurrentROIRectSpeedBottleOnTriggerPos.y());
        NewROIRectMeasureBottleOnTriggerPos.setWidth(CurrentROIRectSpeedBottleOnTriggerPos.width());
        NewROIRectMeasureBottleOnTriggerPos.setHeight(NewHeight);

        GetMainAppCrystalT2()->GetImageData()->SetMeasureWindowRect(ROI_ID_MEASURE_BOTTLE_UNDER_VALVE, NewROIRectMeasureBottleOnTriggerPos);
        GetMainAppCrystalT2()->GetImageData()->SetMeasureWindowRect(ROI_ID_MEASURE_SPEED, NewROIRectMeasureSpeed);
        GetMainAppCrystalT2()->GetLiveImageView()->DrawMeasureWindow(ROI_ID_MEASURE_SPEED);
    }
}

void EditProductDialog::SlotROILiquidXPosChanged()
{
    ROILiquidXYPositionChanged();
}

void EditProductDialog::SlotROILiquidYPosChanged()
{
    ROILiquidXYPositionChanged();
}

void EditProductDialog::ROILiquidXYPositionChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetLiveImageView() && GetMainAppCrystalT2()->GetImageData()) {
        QRect NewROIRectMeasureLiquid, CurrentROIRectLiquid;
        QRect NewROIRectMeasureBottleOnTriggerPos, CurrentROIRectBottleOnTriggerPos;
        int NewXPos = static_cast<int>(ui.doubleSpinBoxMeasureLiquidXPosition->value());
        int NewYPos = static_cast<int>(ui.doubleSpinBoxMeasureLiquidYPosition->value());
        int MinXPos = 0;

        if (GetMainAppCrystalT2()->IsFirstTriggerOnLeftSide()) MinXPos = static_cast<int>(ui.doubleSpinBoxMeasureSpeedXPosition->value() + ui.doubleSpinBoxMeasureSpeedWidth->value());
        CurrentROIRectLiquid = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRect(ROI_ID_MEASURE_LIQUID);
        CurrentROIRectBottleOnTriggerPos = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRect(ROI_ID_MEASURE_BOTTLE_UNDER_VALVE);
        // grenzen überprüfen
        if (NewXPos < MinXPos) NewXPos = MinXPos;
        if (NewYPos < 0) NewYPos = 0;
        if ((NewXPos + CurrentROIRectLiquid.width()) >= GetMainAppCrystalT2()->GetImageData()->GetImageWidth()) {
            NewXPos = GetMainAppCrystalT2()->GetImageData()->GetImageWidth() - CurrentROIRectLiquid.width();
            if (m_MeasureWindowSpinBoxesHasSlotConnection) {
                disconnect(ui.doubleSpinBoxMeasureLiquidXPosition, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROILiquidXPosChanged);
            }
            ui.doubleSpinBoxMeasureLiquidXPosition->setValue(NewXPos);
            if (m_MeasureWindowSpinBoxesHasSlotConnection) {
                connect(ui.doubleSpinBoxMeasureLiquidXPosition, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROILiquidXPosChanged);
            }
        }
        if ((NewYPos + CurrentROIRectLiquid.height()) >= GetMainAppCrystalT2()->GetImageData()->GetImageHeight()) {
            NewYPos = GetMainAppCrystalT2()->GetImageData()->GetImageHeight() - CurrentROIRectLiquid.height();
            if (m_MeasureWindowSpinBoxesHasSlotConnection) {
                disconnect(ui.doubleSpinBoxMeasureLiquidYPosition, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROILiquidYPosChanged);
            }
            ui.doubleSpinBoxMeasureLiquidYPosition->setValue(NewYPos);
            if (m_MeasureWindowSpinBoxesHasSlotConnection) {
                connect(ui.doubleSpinBoxMeasureSpeedYPosition, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROILiquidXPosChanged);
            }
        }
        NewROIRectMeasureLiquid.setX(NewXPos);
        NewROIRectMeasureLiquid.setY(NewYPos);
        NewROIRectMeasureLiquid.setWidth(CurrentROIRectLiquid.width());
        NewROIRectMeasureLiquid.setHeight(CurrentROIRectLiquid.height());

        NewROIRectMeasureBottleOnTriggerPos.setX(NewXPos);
        NewROIRectMeasureBottleOnTriggerPos.setY(CurrentROIRectBottleOnTriggerPos.y());
        NewROIRectMeasureBottleOnTriggerPos.setWidth(CurrentROIRectBottleOnTriggerPos.width());
        NewROIRectMeasureBottleOnTriggerPos.setHeight(CurrentROIRectBottleOnTriggerPos.height());

        GetMainAppCrystalT2()->GetImageData()->SetMeasureWindowRect(ROI_ID_MEASURE_LIQUID, NewROIRectMeasureLiquid);
        GetMainAppCrystalT2()->GetImageData()->SetMeasureWindowRect(ROI_ID_MEASURE_BOTTLE_UNDER_VALVE, NewROIRectMeasureBottleOnTriggerPos);
        GetMainAppCrystalT2()->GetLiveImageView()->DrawMeasureWindow(ROI_ID_MEASURE_LIQUID);
    }
}

void EditProductDialog::SlotROILiquidWidthChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetLiveImageView() && GetMainAppCrystalT2()->GetImageData()) {
        int NewWidth = static_cast<int>(ui.doubleSpinBoxMeasureLiquidWidth->value());
        int MaxWidth = GetMainAppCrystalT2()->GetImageData()->GetImageWidth();
        QRect NewROIRectMeasureLiquid, CurrentROIRectLiquid;
        QRect NewROIRectMeasureBottleOnTriggerPos, CurrentROIRectSpeedBottleOnTriggerPos;
        bool OnLimit = false;
        // SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        // ProductData*  pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
        // int MinWidthInPixelBlueWindow = 150;
        int MinWidthInPixelBlueWindow = GetMainAppCrystalT2()->GetMinBlueWindowWidthInPixel();

        CurrentROIRectLiquid = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRect(ROI_ID_MEASURE_LIQUID);
        CurrentROIRectSpeedBottleOnTriggerPos = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRect(ROI_ID_MEASURE_BOTTLE_UNDER_VALVE);

        if (!GetMainAppCrystalT2()->IsFirstTriggerOnLeftSide()) MaxWidth = static_cast<int>(ui.doubleSpinBoxMeasureSpeedXPosition->value());

        if ((static_cast<int>(ui.doubleSpinBoxMeasureLiquidXPosition->value()) + NewWidth) >= MaxWidth) {
            NewWidth = MaxWidth - static_cast<int>(ui.doubleSpinBoxMeasureLiquidXPosition->value()) - 1;
            if (m_MeasureWindowSpinBoxesHasSlotConnection) {
                disconnect(ui.doubleSpinBoxMeasureLiquidWidth, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROILiquidWidthChanged);
            }
            ui.doubleSpinBoxMeasureLiquidWidth->setValue(NewWidth);
            if (m_MeasureWindowSpinBoxesHasSlotConnection) {
                connect(ui.doubleSpinBoxMeasureLiquidWidth, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROILiquidWidthChanged);
            }
            OnLimit = true;
        }

        if (NewWidth < MinWidthInPixelBlueWindow) {
            NewWidth = MinWidthInPixelBlueWindow;
            if (m_MeasureWindowSpinBoxesHasSlotConnection) {
                disconnect(ui.doubleSpinBoxMeasureLiquidWidth, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROILiquidWidthChanged);
            }
            ui.doubleSpinBoxMeasureLiquidWidth->setValue(NewWidth);
            if (m_MeasureWindowSpinBoxesHasSlotConnection) {
                connect(ui.doubleSpinBoxMeasureLiquidWidth, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROILiquidWidthChanged);
            }
            OnLimit = true;
        }

        if (OnLimit) {
            disconnect(ui.doubleSpinBoxWidth, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &EditProductDialog::SlotMeasureWidthChanged);
            ui.doubleSpinBoxWidth->setValue(NewWidth);
            connect(ui.doubleSpinBoxWidth, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &EditProductDialog::SlotMeasureWidthChanged);
        }

        NewROIRectMeasureLiquid.setX(CurrentROIRectLiquid.x());
        NewROIRectMeasureLiquid.setY(CurrentROIRectLiquid.y());
        NewROIRectMeasureLiquid.setWidth(NewWidth);
        NewROIRectMeasureLiquid.setHeight(CurrentROIRectLiquid.height());

        NewROIRectMeasureBottleOnTriggerPos.setX(CurrentROIRectSpeedBottleOnTriggerPos.x());
        NewROIRectMeasureBottleOnTriggerPos.setY(CurrentROIRectSpeedBottleOnTriggerPos.y());
        NewROIRectMeasureBottleOnTriggerPos.setWidth(NewWidth);
        NewROIRectMeasureBottleOnTriggerPos.setHeight(CurrentROIRectSpeedBottleOnTriggerPos.height());

        GetMainAppCrystalT2()->GetImageData()->SetMeasureWindowRect(ROI_ID_MEASURE_LIQUID, NewROIRectMeasureLiquid);
        GetMainAppCrystalT2()->GetLiveImageView()->DrawMeasureWindow(ROI_ID_MEASURE_LIQUID);

        GetMainAppCrystalT2()->GetImageData()->SetMeasureWindowRect(ROI_ID_MEASURE_BOTTLE_UNDER_VALVE, NewROIRectMeasureBottleOnTriggerPos);
    }
}

void EditProductDialog::SlotROILiquidHeightChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetLiveImageView() && GetMainAppCrystalT2()->GetImageData()) {
        int MeasureWindowID = ROI_ID_MEASURE_LIQUID;
        int NewHeight = static_cast<int>(ui.doubleSpinBoxMeasureLiquidHeight->value());
        QRect NewROIRect, ROIRect;

        ROIRect = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRect(MeasureWindowID);
        if ((static_cast<int>(ui.doubleSpinBoxMeasureLiquidYPosition->value()) + NewHeight) >= GetMainAppCrystalT2()->GetImageData()->GetImageHeight()) {
            NewHeight = GetMainAppCrystalT2()->GetImageData()->GetImageHeight() - static_cast<int>(ui.doubleSpinBoxMeasureLiquidYPosition->value()) - 1;
            if (m_MeasureWindowSpinBoxesHasSlotConnection) {
                disconnect(ui.doubleSpinBoxMeasureLiquidHeight, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROILiquidHeightChanged);
            }
            ui.doubleSpinBoxMeasureLiquidHeight->setValue(NewHeight);
            if (m_MeasureWindowSpinBoxesHasSlotConnection) {
                connect(ui.doubleSpinBoxMeasureLiquidHeight, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROILiquidHeightChanged);
            }
        }
        if (NewHeight < MINIMUM_ROI_SIZE_IN_PIXEL) {
            NewHeight = MINIMUM_ROI_SIZE_IN_PIXEL;
            if (m_MeasureWindowSpinBoxesHasSlotConnection) {
                disconnect(ui.doubleSpinBoxMeasureLiquidHeight, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROILiquidHeightChanged);
            }
            ui.doubleSpinBoxMeasureLiquidHeight->setValue(NewHeight);
            if (m_MeasureWindowSpinBoxesHasSlotConnection) {
                connect(ui.doubleSpinBoxMeasureLiquidHeight, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotROILiquidHeightChanged);
            }
        }

        NewROIRect.setX(ROIRect.x());
        NewROIRect.setY(ROIRect.y());
        NewROIRect.setWidth(ROIRect.width());
        NewROIRect.setHeight(NewHeight);

        GetMainAppCrystalT2()->GetImageData()->SetMeasureWindowRect(MeasureWindowID, NewROIRect);
        GetMainAppCrystalT2()->GetLiveImageView()->DrawMeasureWindow(MeasureWindowID);
    }
}

void EditProductDialog::SlotCalibrate()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData() && GetMainAppCrystalT2()->GetAdminSettingsDialog()) {
        InputCalibrateDialog* pInputCalibrateDialog = new InputCalibrateDialog(this);
        if (QDialog::Accepted == pInputCalibrateDialog->exec()) {
            GetMainAppCrystalT2()->GetImageData()->SetInfoLevel(0);
            GetMainAppCrystalT2()->GetAdminSettingsDialog()->SetInfoLevel(0);
            GetMainAppCrystalT2()->GetImageData()->SetAutoCalibrateIsOn(true);
            if (pInputCalibrateDialog->GetCheckBoxState() == Qt::Checked)
                GetMainAppCrystalT2()->GetImageData()->SetDoNotChangeBlueWindow(true);
            else
                GetMainAppCrystalT2()->GetImageData()->SetDoNotChangeBlueWindow(false);
        }
        delete pInputCalibrateDialog;
    }
}

void EditProductDialog::AddLiveImageWidget(QWidget* w)
{
    if (ui.ImageFrame->layout()) ui.ImageFrame->layout()->addWidget(w);
}

void EditProductDialog::RemoveLiveImageWidget(QWidget* w)
{
    if (ui.ImageFrame->layout()) ui.ImageFrame->layout()->removeWidget(w);
}

void EditProductDialog::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton) {
        if (isVisible())
            e->ignore();
        else
            QWidget::mousePressEvent(e);
    } else
        QWidget::mousePressEvent(e);
}

void EditProductDialog::SetupWindow()
{
    m_WindowSetup = false;
    UpdateProductData();

    // if (GetPlotLineProfil()) GetPlotLineProfil()->resize(ui.frameGradientImage->width(), ui.frameGradientImage->height());
    if (GetMainAppCrystalT2()) {
        if (GetMainAppCrystalT2()->GetLiveImageView()) {
            AddLiveImageWidget((QWidget*)(GetMainAppCrystalT2()->GetLiveImageView()));
            GetMainAppCrystalT2()->GetLiveImageView()->SetOnCurrentProductionState();
        }
        QRect CurrentRectMeasureSpeed = GetMainAppCrystalT2()->GetMeasureWindowRect(ROI_ID_MEASURE_SPEED);
        QRect CurrentRectLiquid = GetMainAppCrystalT2()->GetMeasureWindowRect(ROI_ID_MEASURE_LIQUID);

        ui.doubleSpinBoxMeasureSpeedXPosition->setValue(CurrentRectMeasureSpeed.x());
        ui.doubleSpinBoxMeasureSpeedYPosition->setValue(CurrentRectMeasureSpeed.y());
        ui.doubleSpinBoxMeasureSpeedWidth->setValue(CurrentRectMeasureSpeed.width());
        ui.doubleSpinBoxMeasureSpeedHeight->setValue(CurrentRectMeasureSpeed.height());

        if (ui.checkBoxSelectSpeedMeasureWindow->isChecked()) {
            ui.doubleSpinBoxWidth->setValue(CurrentRectMeasureSpeed.width());
            ui.doubleSpinBoxHeight->setValue(CurrentRectMeasureSpeed.height());
        }

        if (ui.checkBoxSelectLiquidMeasureWindow->isChecked()) {
            ui.doubleSpinBoxWidth->setValue(CurrentRectLiquid.width());
            ui.doubleSpinBoxHeight->setValue(CurrentRectLiquid.height());
        }

        ui.doubleSpinBoxMeasureLiquidXPosition->setValue(CurrentRectLiquid.x());
        ui.doubleSpinBoxMeasureLiquidYPosition->setValue(CurrentRectLiquid.y());
        ui.doubleSpinBoxMeasureLiquidWidth->setValue(CurrentRectLiquid.width());
        ui.doubleSpinBoxMeasureLiquidHeight->setValue(CurrentRectLiquid.height());

        if (GetMainAppCrystalT2()->GetCurrentProductData()) {
            ProductData* pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
            if (pProductData) {
                ui.doubleSpinBoxInjectionAngle->setValue(pProductData->m_InjectionAngle);                  // = ui.doubleSpinBoxInjectionAngle->value();
                ui.doubleSpinBoxDistanceInjectorBottle->setValue(pProductData->m_DistanceInjectorBottle);  // = ui.doubleSpinBoxDistanceInjectorBottle->value();
                ui.doubleSpinBoxNeckInnerDiameter->setValue(pProductData->m_BottleNeckInnerDiameter);
                ui.doubleSpinBoxAcceptanceThresholdLiquidLeftAndRightROI->setValue(pProductData->m_AcceptanceThresholdLiquidLeftAndRightROI);
                ui.doubleSpinBoxMinAcceptanceThresholdLiquidMiddleROI->setValue(pProductData->m_MinAcceptanceThresholdLiquidMiddleROI);
                ui.doubleSpinBoxMaxAcceptanceThresholdLiquidMiddleROI->setValue(pProductData->m_MaxAcceptanceThresholdLiquidMiddleROI);
                ui.doubleSpinBoxInjectionMiddleWindowWidthInMm->setValue(pProductData->m_InjectionMiddleWindowWidthInMm);
                ui.groupBoxAmountOfLiquidAndSplashLeftSide->setEnabled(true);
                ui.groupBoxAmountOfLiquidAndSplashRightSide->setEnabled(true);
                if (pProductData->m_UsedTriggerOutputs == USE_ONLY_RIGHT_VALVE) {
                    ui.groupBoxAmountOfLiquidAndSplashLeftSide->setEnabled(false);
                    SetAmountOfSplashesLeftOnLeftTriggerSide(0);
                    SetAmountOfSplashesRightOnLeftTriggerSide(0);
                    ResetAmountOfLiquidMiddleLeftTriggerSide();
                }
                if (pProductData->m_UsedTriggerOutputs == USE_ONLY_LEFT_VALVE) {
                    ui.groupBoxAmountOfLiquidAndSplashRightSide->setEnabled(false);
                    SetAmountOfSplashesLeftOnRightTriggerSide(0);
                    SetAmountOfSplashesRightOnRightTriggerSide(0);
                    ResetAmountOfLiquidMiddleRightTriggerSide();
                }
            }
        }
        GetMainAppCrystalT2()->ShowOptionPanelImageTab(false);
        if (GetMainAppCrystalT2()->GetImageData()) {
            if (!GetMainAppCrystalT2()->GetImageData()->IsCameraResolutionGreaterThanDefaultResolution()) {
                ui.labelSelectCamera->hide();
                ui.checkBoxSelectCamera->hide();
            }
        }
    }
    m_WindowSetup = true;
}

bool EditProductDialog::event(QEvent* pEvent)
{
    if (pEvent->type() == QEvent::Show) {
        SetupWindow();
    } else if (pEvent->type() == QEvent::Hide) {
        if (GetMainAppCrystalT2()->GetLiveImageView()) RemoveLiveImageWidget((QWidget*)(GetMainAppCrystalT2()->GetLiveImageView()));
    }
    return QWidget::event(pEvent);
}

void EditProductDialog::UpdateProductData()
{
    if (GetMainAppCrystalT2()->GetCurrentProductData()) {
        m_MinProductCenterTolerance = GetMainAppCrystalT2()->GetCurrentProductData()->GetBottleNeckInnerDiameter() / (1.4142 * 2.0);
        ui.doubleSpinBoxProductNeckDiameter->setValue(GetMainAppCrystalT2()->GetCurrentProductData()->GetBottleNeckDiameter());
        ui.doubleSpinBoxNeckInnerDiameter->setValue(GetMainAppCrystalT2()->GetCurrentProductData()->GetBottleNeckInnerDiameter());

        double Factor = GetFactorCalculateNeckDiameterTolerance();
        if (Factor >= 0.0) {
            ui.doubleSpinBoxProductCenterTol->blockSignals(true);
            ui.doubleSpinBoxProductCenterTol->setValue(GetMainAppCrystalT2()->GetCurrentProductData()->m_BotteleNeckDiameterToleranceInmm / Factor);
            ui.doubleSpinBoxProductCenterTol->blockSignals(false);
        }
        SetSuffixProductCenterTol(GetMainAppCrystalT2()->GetCurrentProductData()->GetBottleNeckInnerDiameter());
        CalculateBottleNeckDiameterTolFromProductCenterTol();

        if (GetMainAppCrystalT2()->IsFirstTriggerOnLeftSide())
            ui.labelBandDirectional->setPixmap(QPixmap(":/SiemensHMI/ArrowLeftToRight.png"));
        else
            ui.labelBandDirectional->setPixmap(QPixmap(":/SiemensHMI/ArrowRightToLeft.png"));
    }
    if (ui.checkBoxSelectCamera->isChecked()) {
        DrawCameraViewportOffsets();
    } else {
        ClearCameraViewportOffsets();
    }
}

void EditProductDialog::SlotNeckDiameterValueChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) {
        double InnerDiameter = ui.doubleSpinBoxNeckInnerDiameter->value();
        double OuterDiameter = ui.doubleSpinBoxProductNeckDiameter->value();
        if (OuterDiameter <= InnerDiameter) {  // Verhindern das der Innerndurchmesser größer ist als der Aussendurchmesser
            InnerDiameter = OuterDiameter * 0.8;
            disconnect(ui.doubleSpinBoxNeckInnerDiameter, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotNeckInnerDiameterChanged);
            ui.doubleSpinBoxNeckInnerDiameter->setValue(InnerDiameter);
            connect(ui.doubleSpinBoxNeckInnerDiameter, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotNeckInnerDiameterChanged);
            SlotNeckInnerDiameterChanged();
        }
        GetMainAppCrystalT2()->GetImageData()->SetTargetBottleneckDiameter(ui.doubleSpinBoxProductNeckDiameter->value());
    }
}

void EditProductDialog::SlotMinThresholdMiddleROIChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) {
        double currentMaxValue = ui.doubleSpinBoxMaxAcceptanceThresholdLiquidMiddleROI->value();
        if (ui.doubleSpinBoxMinAcceptanceThresholdLiquidMiddleROI->value() >= currentMaxValue) {
            ui.doubleSpinBoxMinAcceptanceThresholdLiquidMiddleROI->blockSignals(true);
            ui.doubleSpinBoxMinAcceptanceThresholdLiquidMiddleROI->setValue(currentMaxValue - 1);
            ui.doubleSpinBoxMinAcceptanceThresholdLiquidMiddleROI->blockSignals(false);
        }
        GetMainAppCrystalT2()->GetImageData()->SetMinAcceptanceThresholdLiquidMiddleROI(ui.doubleSpinBoxMinAcceptanceThresholdLiquidMiddleROI->value());
    }
}

void EditProductDialog::SlotMaxThresholdMiddleROIChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) {
        double CurrentMinValue = ui.doubleSpinBoxMinAcceptanceThresholdLiquidMiddleROI->value();
        if (ui.doubleSpinBoxMaxAcceptanceThresholdLiquidMiddleROI->value() <= CurrentMinValue) {
            ui.doubleSpinBoxMaxAcceptanceThresholdLiquidMiddleROI->blockSignals(true);
            ui.doubleSpinBoxMaxAcceptanceThresholdLiquidMiddleROI->setValue(CurrentMinValue + 1);
            ui.doubleSpinBoxMaxAcceptanceThresholdLiquidMiddleROI->blockSignals(false);
        }
        GetMainAppCrystalT2()->GetImageData()->SetMaxAcceptanceThresholdLiquidMiddleROI(ui.doubleSpinBoxMaxAcceptanceThresholdLiquidMiddleROI->value());
    }
}

void EditProductDialog::SlotThresholdLeftRightROIChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData())
        GetMainAppCrystalT2()->GetImageData()->SetAcceptanceThresholdLiquidLeftAndRightROI(ui.doubleSpinBoxAcceptanceThresholdLiquidLeftAndRightROI->value());
}

void EditProductDialog::SlotProductCenterToleranceChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) {
        if (ui.doubleSpinBoxProductCenterTol->value() < m_MinProductCenterTolerance) {
            ui.doubleSpinBoxProductCenterTol->blockSignals(true);
            ui.doubleSpinBoxProductCenterTol->setValue(m_MinProductCenterTolerance);
            ui.doubleSpinBoxProductCenterTol->blockSignals(false);
        }
        CalculateBottleNeckDiameterTolFromProductCenterTol();
    }
}

void EditProductDialog::CalculateBottleNeckDiameterTolFromProductCenterTol()
{
    double Tol = GetFactorCalculateNeckDiameterTolerance() * ui.doubleSpinBoxProductCenterTol->value();
    SetSuffixBottleNeckTol(Tol);
}

void EditProductDialog::SlotInjectionMiddleWindowInPercentChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetImageData()) {
        QRect BlueRect = GetMainAppCrystalT2()->GetImageData()->GetMeasureWindowRect(ROI_ID_MEASURE_LIQUID);
        double BlueWindowWidtInMM = BlueRect.width() * GetMainAppCrystalT2()->GetSettingsData()->m_PixelSize - 1;
        double ValveDistanceInMM = GetMainAppCrystalT2()->GetSettingsData()->m_DistancesBetweenValves;

        if ((ValveDistanceInMM + ui.doubleSpinBoxInjectionMiddleWindowWidthInMm->value()) < BlueWindowWidtInMM) {
            GetMainAppCrystalT2()->GetImageData()->SetInjectonWindowMiddleWidthInMm(ui.doubleSpinBoxInjectionMiddleWindowWidthInMm->value());
            GetMainAppCrystalT2()->GetLiveImageView()->DrawMeasureWindow(ROI_ID_MEASURE_LIQUID);
        }
    }
}

double EditProductDialog::GetFactorCalculateNeckDiameterTolerance()
{
    double rv = 0;
    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
    if (pSettingsData && pSettingsData->m_DistanceCameraProduct > 0.0) {
        ProductData* pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
        if (pProductData) {
            double HalfProductSize = pProductData->m_BottleNeckDiameter / 2.0;
            double TanAlpha = HalfProductSize / pSettingsData->m_DistanceCameraProduct;
            double MinValue = TanAlpha * (pSettingsData->m_DistanceCameraProduct - 1);
            rv = fabs(HalfProductSize - MinValue) * 2.0;
        }
    }
    return rv;
}

void EditProductDialog::SlotNeckInnerDiameterChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        QString ErrorMsg;
        ProductData* pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
        if (pProductData) {
            double OuterDiameter = ui.doubleSpinBoxProductNeckDiameter->value();
            if (ui.doubleSpinBoxNeckInnerDiameter->value() > OuterDiameter) {
                disconnect(ui.doubleSpinBoxNeckInnerDiameter, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotNeckInnerDiameterChanged);
                ui.doubleSpinBoxNeckInnerDiameter->setValue(OuterDiameter);
                connect(ui.doubleSpinBoxNeckInnerDiameter, &QDoubleSpinBox::editingFinished, this, &EditProductDialog::SlotNeckInnerDiameterChanged);
            }
            pProductData->m_BottleNeckInnerDiameter = ui.doubleSpinBoxNeckInnerDiameter->value();
            pProductData->WriteProductData(ErrorMsg);  // save to productdata
            SetSuffixProductCenterTol(pProductData->m_BottleNeckInnerDiameter);
        }
    }
}

void EditProductDialog::SetSuffixProductCenterTol(double BottleNeckInnerDiameter)
{
    m_MinProductCenterTolerance = BottleNeckInnerDiameter / (1.4142 * 2.0);
    QString Suffix = QString("mm(max:%1mm)").arg(m_MinProductCenterTolerance, 0, 'f', 2);
    ui.doubleSpinBoxProductCenterTol->blockSignals(true);
    ui.doubleSpinBoxProductCenterTol->setSuffix(Suffix);
    if (ui.doubleSpinBoxProductCenterTol->value() < m_MinProductCenterTolerance) {
        ui.doubleSpinBoxProductCenterTol->setValue(m_MinProductCenterTolerance);
    } else {
        double Factor = GetFactorCalculateNeckDiameterTolerance();
        if (Factor >= 0.0) {
            ui.doubleSpinBoxProductCenterTol->setValue(GetMainAppCrystalT2()->GetCurrentProductData()->m_BotteleNeckDiameterToleranceInmm / Factor);
        }
    }
    ui.doubleSpinBoxProductCenterTol->blockSignals(false);
}

void EditProductDialog::SetSuffixBottleNeckTol(double Tol)
{
    GetMainAppCrystalT2()->GetImageData()->SetBotteleNeckDiameterToleranceInmm(Tol);  // +/-
    QString Suffix = QString("mm(+/-%1mm)").arg(Tol, 0, 'f', 2);
    ui.doubleSpinBoxProductNeckDiameter->setSuffix(Suffix);
}

void EditProductDialog::SlotDistanceInjectorBottleChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        QString ErrorMsg;
        ProductData* pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
        if (pProductData) {
            pProductData->m_DistanceInjectorBottle = ui.doubleSpinBoxDistanceInjectorBottle->value();
            pProductData->WriteProductData(ErrorMsg);  // save to productdata
            if (GetMainAppCrystalT2()->GetLiveImageView()) {
                GetMainAppCrystalT2()->GetLiveImageView()->DrawSupportingLines();
            }
        }
    }
}

void EditProductDialog::SlotInjectorAngleChanged()
{
    if (m_WindowSetup && GetMainAppCrystalT2()) {
        QString ErrorMsg;
        ProductData* pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
        if (pProductData) {
            pProductData->m_InjectionAngle = ui.doubleSpinBoxInjectionAngle->value();
            pProductData->WriteProductData(ErrorMsg);  // save to productdata
            if (GetMainAppCrystalT2()->GetLiveImageView()) {
                GetMainAppCrystalT2()->GetLiveImageView()->DrawSupportingLines();
            }
        }
    }
}
