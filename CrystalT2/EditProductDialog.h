#pragma once

#include <interfaces.h>
#include <QWidget>
#include "GlobalConst.h"
#include "bmessagebox.h"
#include "ui_EditProductDialog.h"

class PopupProgressBar;
class PlotLineProfil;
class MainAppCrystalT2;
class MeasuringResultsLiquid;
class EditProductDialog : public QWidget
{
    Q_OBJECT
  public:
    EditProductDialog(MainAppCrystalT2* pMainAppCrystalT2, QWidget* pWidget = NULL);
    ~EditProductDialog();
    bool event(QEvent* pEvent);
    void mousePressEvent(QMouseEvent* e);
    MainAppCrystalT2* GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
    void UpdateProductData();
    void AddLiveImageWidget(QWidget* w);
    void RemoveLiveImageWidget(QWidget* w);
    PlotLineProfil* GetPlotLineProfil() { return m_PlotLineProfil; }
    void SetLineDataGradient(const double* Array, int size, double EdgeLeftSide, double EdgeRightSide);
    bool IsPlotLineProfilVisible();
    void SetLiveImageSize(int w, int h);
    void UpdateMesureWindow(int MeasureWindowID);
    void ROISpeedXYPositionChanged();
    void ROILiquidXYPositionChanged();
    double GetFactorCalculateNeckDiameterTolerance();
    double GetThresholdSplashes();

    QString GetSuffixStandardDev(double standardDevation);
    void SetMeasuredNeckDiameter(double set, bool status);
    // Amount left trigger
    void SetAmountOfSplashesLeftOnLeftTriggerSide(int Amount);
    void SetAmountOfSplashesRightOnLeftTriggerSide(int Amount);
    void SetAmountOfLiquidMiddleLeftTriggerSide(double Amount, double standardDevation, double SumAmountLiqiud);
    void ResetAmountOfLiquidMiddleLeftTriggerSide();
    // Amount right trigger
    void SetAmountOfSplashesLeftOnRightTriggerSide(int Amount);
    void SetAmountOfSplashesRightOnRightTriggerSide(int Amount);
    void SetAmountOfLiquidMiddleRightTriggerSide(double Amount, double standardDevation, double SumAmountLiqiud);
    void ResetAmountOfLiquidMiddleRightTriggerSide();
    void SetAmountOfLiquidMiddle(double Amount, double standardDevation);
    void SetupWindow();

    void SetCounterBottleFilled(int set);
    void SetCounterBottleNotFilled(int set);
    void SetCounterMiddleTooLow(int set);
    void SetCounterLeftTooBig(int set);
    void SetCounterRightTooBig(int set);
    void SetCounterBottleNotInPos(int set);

    void SetCounterContrastIsOutOfTol(unsigned long long set);
    void SetCounterSizeIsOutOfTol(unsigned long long set);
    void SetCounterEdgeIsOutOfTol(unsigned long long set);
    void SetSuffixProductCenterTol(double BottleNeckInnerDiameter);
    void SetSuffixBottleNeckTol(double Tol);
    void SetCurrentMaschineState(PluginInterface::MachineState set);
    void SetAuditTrailProperties();
    void SetRequiredAccessLevel();
    void SetForceEditingFinishedSpeedWindow();
    void SetForceEditingFinishedInjectionWindow();
    void SetResponseValuePulseFromController(int ValveID, double set);
    void CheckEnableColorBoxesControls();
    void SetCheckBoxSpeed(bool);
    void SetCheckBoxLiquid(bool);
    void ResetIsClicked();
    void CalculateBottleNeckDiameterTolFromProductCenterTol();
    bool eventFilter(QObject*, QEvent* pEvent);
    void MoveCameraROI(int MoveDirection);
    void DrawCameraViewportOffsets();
    void ClearCameraViewportOffsets();

  signals:
    void SignalEnableGUIElements(int);
    void SignalMaschineStateProduction(bool ProductionOn);
    void SignalManuelTriggerReady(const int ValveID);
    void SignalResetLiquidResults();

  public slots:
    void SlotNeckDiameterValueChanged();
    void SlotProductCenterToleranceChanged();
    void SlotNeckInnerDiameterChanged();
    void SlotROIMeasureSpeedXPosChanged();
    void SlotROIMeasureSpeedYPosChanged();
    void SlotROIMeasureSpeedWidthChanged();
    void SlotROIMeasureSpeedHeightChanged();
    void SlotROILiquidXPosChanged();
    void SlotROILiquidYPosChanged();
    void SlotROILiquidWidthChanged();
    void SlotROILiquidHeightChanged();
    void SlotDistanceInjectorBottleChanged();
    void SlotInjectorAngleChanged();
    void SlotMinThresholdMiddleROIChanged();
    void SlotMaxThresholdMiddleROIChanged();
    void SlotThresholdLeftRightROIChanged();
    void SlotResetCounters();
    void SlotInjectionMiddleWindowInPercentChanged();
    void SlotEnableGUIElements(int);
    void SlotCalibrate();
    void SlotSetCalibrateStatus(const QString& Info, int value);
    void SlotCalibrateAbortIsClicked();
    void SlotRunCameraSimulation();
    void SlotStepCameraSimulation();
    void SlotStopCameraSimulation();
    void SlotPlayOneVideoCameraSimulation();
    void SlotPulseLeftValveChanged();
    void SlotPulseRightValveChanged();
    void SlotMoveUp();
    void SlotMoveDown();
    void SlotMoveToLeft();
    void SlotMoveToRight();
    void SlotAutorepeatMoveUp();
    void SlotAutorepeatMoveDown();
    void SlotAutorepeatMoveLeft();
    void SlotAutorepeatMoveRight();
    void SlotMeasureWidthChanged(double);
    void SlotMeasureHeightChanged(double);
    void SlotSelectLiquidMeasureWindowChanged(int);
    void SlotSelectSpeedMeasureWindowChanged(int);
    void SlotSelectLiquidAndSpeedMeasureWindowChanged(int);
    void SlotSelectCameraROI(int);
    void SlotResetLiquidResults();
    void SlotCenterCameraViewport();

  private:
    Ui::EditProductDialog ui;
    MainAppCrystalT2* m_MainAppCrystalT2;
    PlotLineProfil* m_PlotLineProfil;
    QString m_ProductName;
    bool m_WindowSetup;
    int m_CounterShowNeckDiameter;
    int m_CounterShowPixelSize;
    int m_NumberSameMeasureValues;
    int m_StatusDegreeOfPollution;
    int m_AutoRepeatDelay;
    int m_AutoRepeatInterval;
    double m_MaxValueProductCenterToleranceInMM;
    double m_MinProductCenterTolerance;
    QTimer *m_TimerAutorepeatMoveUp, *m_TimerAutorepeatMoveDown, *m_TimerAutorepeatMoveLeft, *m_TimerAutorepeatMoveRight;
    PopupProgressBar* m_PopupProgressBar;
    bool m_MeasureWindowSpinBoxesHasSlotConnection;
    bool m_ProductDiameterIsOutOfTol;
    bool m_ProductEdgeNotFound;
    unsigned long long m_CounterSizeNotOk;
    unsigned long long m_CounterEdgeNotOk;
    QFuture<void> m_ManualTriggerReadyV1;
   // QRect m_CameraROI;
    enum MOVE_CAMERA_ROI_DIRECTION { MOVE_LEFT, MOVE_RIGHT, MOVE_UP, MOVE_DOWN };
};

class InputCalibrateDialog : public PopupDialog
{
    Q_OBJECT

  public:
    InputCalibrateDialog(QWidget* parent) : PopupDialog(parent)  //(QMessageBox::Information, tr("Calibrate"), tr("The System Should Be Calibrated ?"))
    {
        m_CheckBox = new QCheckBox();
        m_CheckBoxText = new QLabel(tr("Don't Change Blue Window"));
        QBoxLayout* Hbox = new QHBoxLayout();
        QBoxLayout* Vbox = new QVBoxLayout();
        centralWidget()->setLayout(Vbox);

        Hbox->addWidget(m_CheckBox);
        Hbox->addWidget(m_CheckBoxText);
        setWindowTitle(tr("Calibrate"));

        QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No);
        // button_box->button(QDialogButtonBox::Yes)->setObjectName(tr("Yes"));
        // button_box->button(QDialogButtonBox::No)->setObjectName(tr("No"));

        button_box->button(QDialogButtonBox::Yes)->setText(tr("Yes"));
        button_box->button(QDialogButtonBox::No)->setText(tr("No"));

        connect(button_box, &QDialogButtonBox::clicked, [=](QAbstractButton* button) {
            switch (button_box->standardButton(button)) {
                case QDialogButtonBox::Yes:
                    accept();
                    this->close();
                    break;
                case QDialogButtonBox::No:
                    reject();
                    this->close();
                    break;
                default:
                    break;
            }
        });

        Vbox->addWidget(button_box);
        Vbox->addLayout(Hbox);
    }
    ~InputCalibrateDialog() {}
    int GetCheckBoxState() { return m_CheckBox->checkState(); }

  private:
    QCheckBox* m_CheckBox;
    QLabel* m_CheckBoxText;
};
