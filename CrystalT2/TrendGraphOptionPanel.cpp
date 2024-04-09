#include "TrendGraphOptionPanel.h"
#include "GlobalConst.h"
#include "MainAppCrystalT2.h"
#include "SettingsData.h"
#include "TrendGraphWidget.h"

#include <audittrail.h>

TrendGraphOptionPanel::TrendGraphOptionPanel(MainAppCrystalT2* pMainAppCrystalT2) : OptionPanel(pMainAppCrystalT2), m_FormLayout(NULL), m_ListWidgetLoadFile(NULL), m_SpinBoxRollingMeanSize(NULL)
{
    m_MainAppCrystalT2 = pMainAppCrystalT2;
    setupUi();
    m_SpinBoxRollingMeanSize->hide();
}

void TrendGraphOptionPanel::setupUi()
{
    int flag = 0;

    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        flag = GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag;
    }
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    QString namePiezoTemp = tr("Piezo Temp.");
    QString nameChamperTemp = tr("Chamber Temp.");
    QString nameDropletVolumeMean = tr("Amount Liquid Mean");
    QString nameDropletVolumePoints = tr("Amount Liquid Points");
    QString nameDropletVolumeStd = tr("Amount Liquid Std");

    QGroupBox* groupBoxLeftValve = new QGroupBox(tr("Left Valve"));
    QFormLayout* leftValveFormLayout = new QFormLayout();
    QGroupBox* groupBoxRightValve = new QGroupBox(tr("Right Valve"));
    QFormLayout* rightValveFormLayout = new QFormLayout();

    // Left Valve
    QLabel* LabelShowPiezoLeftValve = new QLabel(namePiezoTemp);
    QCheckBox* CheckBoxShowPiezoLeftValve = new QCheckBox();
    QLabel* LabelShowChamberLeftValve = new QLabel(nameChamperTemp);
    QCheckBox* CheckBoxShowChamperLeftValve = new QCheckBox();
    QLabel* LabelShowDropletLeftValve = new QLabel(nameDropletVolumeMean);
    QCheckBox* CheckBoxShowDropletLeftValve = new QCheckBox();
    QLabel* LabelShowDataPointsLeftValve = new QLabel(nameDropletVolumePoints);
    QCheckBox* CheckBoxShowDataPointsLeftValve = new QCheckBox();
    QLabel* LabelShowStdLeftValve = new QLabel(nameDropletVolumeStd);
    QCheckBox* CheckBoxShowStdLeftValve = new QCheckBox();

    leftValveFormLayout->insertRow(0, LabelShowPiezoLeftValve, CheckBoxShowPiezoLeftValve);
    leftValveFormLayout->insertRow(1, LabelShowChamberLeftValve, CheckBoxShowChamperLeftValve);
    leftValveFormLayout->insertRow(2, LabelShowDropletLeftValve, CheckBoxShowDropletLeftValve);
    leftValveFormLayout->insertRow(3, LabelShowDataPointsLeftValve, CheckBoxShowDataPointsLeftValve);
    leftValveFormLayout->insertRow(4, LabelShowStdLeftValve, CheckBoxShowStdLeftValve);

    groupBoxLeftValve->setLayout(leftValveFormLayout);

    // Right Valve
    QLabel* LabelShowPiezoRightValve = new QLabel(namePiezoTemp);
    QCheckBox* CheckBoxShowPiezoRightValve = new QCheckBox();
    QLabel* LabelShowChamberRightValve = new QLabel(nameChamperTemp);
    QCheckBox* CheckBoxShowChamperRightValve = new QCheckBox();
    QLabel* LabelShowDropletRightValve = new QLabel(nameDropletVolumeMean);
    QCheckBox* CheckBoxShowDropletRightValve = new QCheckBox();
    QLabel* LabelShowDataPointsRightValve = new QLabel(nameDropletVolumePoints);
    QCheckBox* CheckBoxShowDataPointsRightValve = new QCheckBox();
    QLabel* LabelShowStdRightValve = new QLabel(nameDropletVolumeStd);
    QCheckBox* CheckBoxShowStdRightValve = new QCheckBox();

    rightValveFormLayout->insertRow(0, LabelShowPiezoRightValve, CheckBoxShowPiezoRightValve);
    rightValveFormLayout->insertRow(1, LabelShowChamberRightValve, CheckBoxShowChamperRightValve);
    rightValveFormLayout->insertRow(2, LabelShowDropletRightValve, CheckBoxShowDropletRightValve);
    rightValveFormLayout->insertRow(3, LabelShowDataPointsRightValve, CheckBoxShowDataPointsRightValve);
    rightValveFormLayout->insertRow(4, LabelShowStdRightValve, CheckBoxShowStdRightValve);

    groupBoxRightValve->setLayout(rightValveFormLayout);

    QLabel* TextRollingMeanLabel = new QLabel(tr("Rolling Mean Size"));
    m_SpinBoxRollingMeanSize = new QSpinBox();
    m_SpinBoxRollingMeanSize->setMinimum(1);
    m_SpinBoxRollingMeanSize->setMaximum(10000);
    m_ListWidgetLoadFile = new QListWidget();

    QPushButton* LoadButton = new QPushButton(tr("LoadFile"));
    QPushButton* UnloadButton = new QPushButton(tr("UnLoadFile"));
    UnloadButton->setProperty(kAuditTrail, tr("TrendGraph: %1").arg(UnloadButton->text()));
    LoadButton->setProperty(kAuditTrail, tr("TrendGraph: %1").arg(LoadButton->text()));

    connect(LoadButton, &QPushButton::pressed, this, &TrendGraphOptionPanel::SlotLoadFile);
    connect(UnloadButton, &QPushButton::pressed, this, &TrendGraphOptionPanel::SlotUnLoadFile);
    connect(m_SpinBoxRollingMeanSize, &QSpinBox::editingFinished, this, &TrendGraphOptionPanel::SlotRollingMeanSizeChanged);

    QCheckBox* CheckBoxShowBottlesPerMin = new QCheckBox();
    QLabel* LabelShowBottlesPerMin = new QLabel(tr("Bottles Per Min."));

    QCheckBox* CheckBoxShowPreasureTank = new QCheckBox();
    QLabel* LabelShowPreasureTank = new QLabel(tr("Temperature Tank"));

    QCheckBox* CheckBoxWaterCooling = new QCheckBox();
    QLabel* LabelWaterCooling = new QLabel(tr("Water Cooling"));

    QCheckBox* CheckBoxShowHeatingPipe = new QCheckBox();
    QLabel* LabelShowHeatingPipe = new QLabel(tr("Heating Pipe"));

    QCheckBox* CheckBoxShowEjectedBottles = new QCheckBox();
    QLabel* LabelShowEjectedBottles = new QLabel(tr("Camera Ejection"));

    layout->addWidget(groupBoxLeftValve);
    layout->addWidget(groupBoxRightValve);
    QFormLayout* generalFormLayout = new QFormLayout();
    generalFormLayout->insertRow(0, LabelShowBottlesPerMin, CheckBoxShowBottlesPerMin);
    generalFormLayout->insertRow(1, LabelShowPreasureTank, CheckBoxShowPreasureTank);
    generalFormLayout->insertRow(2, LabelShowHeatingPipe, CheckBoxShowHeatingPipe);
    generalFormLayout->insertRow(3, LabelShowEjectedBottles, CheckBoxShowEjectedBottles);
    generalFormLayout->insertRow(4, LabelWaterCooling, CheckBoxWaterCooling);
    QGroupBox* groupBoxGeneralSettings = new QGroupBox(tr("General"));
    groupBoxGeneralSettings->setLayout(generalFormLayout);
    layout->addWidget(groupBoxGeneralSettings);

   /* layout->addWidget(LoadButton);
    layout->addWidget(UnloadButton);
    layout->addWidget(m_ListWidgetLoadFile);*/

    layout->addStretch();

    if (flag & GRAPH_SHOW_DROPLET_LEFT_VALVE_MEAN)
        CheckBoxShowDropletLeftValve->setCheckState(Qt::Checked);
    else
        CheckBoxShowDropletLeftValve->setCheckState(Qt::Unchecked);

    if (flag & GRAPH_SHOW_DROPLET_RIGHT_VALVE_MEAN)
        CheckBoxShowDropletRightValve->setCheckState(Qt::Checked);
    else
        CheckBoxShowDropletRightValve->setCheckState(Qt::Unchecked);
    ////////////////
    if (flag & GRAPH_SHOW_DROPLET_RIGHT_VALVE_RAW_POINTS)
        CheckBoxShowDataPointsRightValve->setCheckState(Qt::Checked);
    else
        CheckBoxShowDataPointsRightValve->setCheckState(Qt::Unchecked);
    if (flag & GRAPH_SHOW_DROPLET_LEFT_VALVE_RAW_POINTS)
        CheckBoxShowDataPointsLeftValve->setCheckState(Qt::Checked);
    else
        CheckBoxShowDataPointsLeftValve->setCheckState(Qt::Unchecked);
    ///////////////
    if (flag & GRAPH_SHOW_DROPLET_LEFT_VALVE_STD)
        CheckBoxShowStdLeftValve->setCheckState(Qt::Checked);
    else
        CheckBoxShowStdLeftValve->setCheckState(Qt::Unchecked);
    if (flag & GRAPH_SHOW_DROPLET_RIGHT_VALVE_STD)
        CheckBoxShowStdRightValve->setCheckState(Qt::Checked);
    else
        CheckBoxShowStdRightValve->setCheckState(Qt::Unchecked);
    ////////////
    if (flag & GRAPH_SHOW_TEMP_PIEZO_LEFT)
        CheckBoxShowPiezoLeftValve->setCheckState(Qt::Checked);
    else
        CheckBoxShowPiezoLeftValve->setCheckState(Qt::Unchecked);
    if (flag & GRAPH_SHOW_TEMP_PIEZO_RIGHT)
        CheckBoxShowPiezoRightValve->setCheckState(Qt::Checked);
    else
        CheckBoxShowPiezoRightValve->setCheckState(Qt::Unchecked);
    if (flag & GRAPH_SHOW_TEMP_CHAMBER_LEFT)
        CheckBoxShowChamperLeftValve->setCheckState(Qt::Checked);
    else
        CheckBoxShowChamperLeftValve->setCheckState(Qt::Unchecked);
    if (flag & GRAPH_SHOW_TEMP_CHAMBER_RIGHT)
        CheckBoxShowChamperRightValve->setCheckState(Qt::Checked);
    else
        CheckBoxShowChamperRightValve->setCheckState(Qt::Unchecked);
    /////////
    if (flag & GRAPH_SHOW_BOTTLES_PER_MIN)
        CheckBoxShowBottlesPerMin->setCheckState(Qt::Checked);
    else
        CheckBoxShowBottlesPerMin->setCheckState(Qt::Unchecked);

    if (flag & GRAPH_SHOW_TEMP_PREASURE_TANK)
        CheckBoxShowPreasureTank->setCheckState(Qt::Checked);
    else
        CheckBoxShowPreasureTank->setCheckState(Qt::Unchecked);

    if (flag & GRAPH_SHOW_HEATING_PIPE)
        CheckBoxShowHeatingPipe->setCheckState(Qt::Checked);
    else
        CheckBoxShowHeatingPipe->setCheckState(Qt::Unchecked);

    if (flag & GRAPH_SHOW_EJECTED_BOTTLES) {
        CheckBoxShowEjectedBottles->setCheckState(Qt::Checked);
    } else {
        CheckBoxShowEjectedBottles->setCheckState(Qt::Unchecked);
    }

    if (flag & GRAPH_SHOW_WATER_COOLING) {
        CheckBoxWaterCooling->setCheckState(Qt::Checked);
    } else {
        CheckBoxWaterCooling->setCheckState(Qt::Unchecked);
    }


    connect(CheckBoxShowDropletLeftValve, &QCheckBox::stateChanged, this, &TrendGraphOptionPanel::SlotSetCheckedShowOnlyLiquidLeftValve);
    connect(CheckBoxShowDataPointsLeftValve, &QCheckBox::stateChanged, this, &TrendGraphOptionPanel::SlotSetCheckedShowDataPointsLeftValve);
    connect(CheckBoxShowStdLeftValve, &QCheckBox::stateChanged, this, &TrendGraphOptionPanel::SlotSetCheckedShowStdLeftValve);

    connect(CheckBoxShowDropletRightValve, &QCheckBox::stateChanged, this, &TrendGraphOptionPanel::SlotSetCheckedShowOnlyLiquidRightValve);
    connect(CheckBoxShowDataPointsRightValve, &QCheckBox::stateChanged, this, &TrendGraphOptionPanel::SlotSetCheckedShowDataPointsRightValve);
    connect(CheckBoxShowStdRightValve, &QCheckBox::stateChanged, this, &TrendGraphOptionPanel::SlotSetCheckedShowStdRightValve);

    connect(CheckBoxShowPiezoLeftValve, &QCheckBox::stateChanged, this, &TrendGraphOptionPanel::SlotCheckBoxShowPiezoLeft);
    connect(CheckBoxShowPiezoRightValve, &QCheckBox::stateChanged, this, &TrendGraphOptionPanel::SlotCheckBoxShowPiezoRight);
    connect(CheckBoxShowChamperLeftValve, &QCheckBox::stateChanged, this, &TrendGraphOptionPanel::SlotCheckBoxShowChamberLeft);
    connect(CheckBoxShowChamperRightValve, &QCheckBox::stateChanged, this, &TrendGraphOptionPanel::SlotCheckBoxShowChamberRight);

    connect(CheckBoxShowPreasureTank, &QCheckBox::stateChanged, this, &TrendGraphOptionPanel::SlotCheckBoxShowPreasureTank);
    connect(CheckBoxShowHeatingPipe, &QCheckBox::stateChanged, this, &TrendGraphOptionPanel::SlotCheckBoxShowHeatingPipe);
    connect(CheckBoxShowBottlesPerMin, &QCheckBox::stateChanged, this, &TrendGraphOptionPanel::SlotSetCheckedBottlesPerMin);
    connect(CheckBoxShowEjectedBottles, &QCheckBox::stateChanged, this, &TrendGraphOptionPanel::SlotSetCheckedEjectedBottles);
    connect(CheckBoxWaterCooling, &QCheckBox::stateChanged, this, &TrendGraphOptionPanel::SlotCheckBoxWaterCooling);

    CheckBoxShowDropletLeftValve->setProperty(kAuditTrail, tr("TrendGraph Left Valve: %1").arg(LabelShowDropletLeftValve->text()));
    CheckBoxShowDataPointsLeftValve->setProperty(kAuditTrail, tr("TrendGraph Left Valve: %1").arg(LabelShowDataPointsLeftValve->text()));
    CheckBoxShowStdLeftValve->setProperty(kAuditTrail, tr("TrendGraph Left Valve: %1").arg(LabelShowStdLeftValve->text()));

    CheckBoxShowDropletRightValve->setProperty(kAuditTrail, tr("TrendGraph Right Valve: %1").arg(LabelShowDropletRightValve->text()));
    CheckBoxShowDataPointsRightValve->setProperty(kAuditTrail, tr("TrendGraph Right Valve: %1").arg(LabelShowDataPointsRightValve->text()));
    CheckBoxShowStdRightValve->setProperty(kAuditTrail, tr("TrendGraph Right Valve: %1").arg(LabelShowStdRightValve->text()));

    CheckBoxShowPiezoLeftValve->setProperty(kAuditTrail, tr("TrendGraph Left Valve: %1").arg(LabelShowPiezoLeftValve->text()));
    CheckBoxShowPiezoRightValve->setProperty(kAuditTrail, tr("TrendGraph Right Valve: %1").arg(LabelShowPiezoRightValve->text()));
    CheckBoxShowChamperLeftValve->setProperty(kAuditTrail, tr("TrendGraph Left Valve: %1").arg(LabelShowChamberLeftValve->text()));
    CheckBoxShowChamperRightValve->setProperty(kAuditTrail, tr("TrendGraph Right Valve: %1").arg(LabelShowChamberRightValve->text()));

    CheckBoxShowPreasureTank->setProperty(kAuditTrail, tr("TrendGraph: %1").arg(LabelShowPreasureTank->text()));
    CheckBoxShowHeatingPipe->setProperty(kAuditTrail, tr("TrendGraph: %1").arg(LabelShowHeatingPipe->text()));
    CheckBoxShowBottlesPerMin->setProperty(kAuditTrail, tr("TrendGraph: %1").arg(LabelShowBottlesPerMin->text()));
    CheckBoxShowEjectedBottles->setProperty(kAuditTrail, tr("TrendGraph: %1").arg(LabelShowEjectedBottles->text()));
}

void TrendGraphOptionPanel::SlotUnLoadFile()
{
    if (GetMainAppCrystalT2()) {
        GetMainAppCrystalT2()->UnLoadTrendGraphData();
    }
}

void TrendGraphOptionPanel::SlotLoadFile()
{
    if (GetMainAppCrystalT2()) {
        QList<QListWidgetItem*> List = m_ListWidgetLoadFile->selectedItems();
        QString BaseName;

        if (List.count() > 0) BaseName = List.at(0)->text();
        GetMainAppCrystalT2()->LoadTrendGraphDataFromFile(BaseName);
    }
}

// const int GRAPH_SHOW_DROPLET_LEFT_VALVE_MEAN = 0x01;
// const int GRAPH_SHOW_DROPLET_RIGHT_VALVE_MEAN = 0x02;
// const int GRAPH_SHOW_DROPLET_SUM_MEAN = 0x04;
void TrendGraphOptionPanel::SlotSetCheckedShowOnlySumLiquid(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        int flag = GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag;
        if (set == Qt::Checked)
            flag = flag | GRAPH_SHOW_DROPLET_SUM_MEAN;
        else
            flag = flag & ~GRAPH_SHOW_DROPLET_SUM_MEAN;
        GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag = flag;
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void TrendGraphOptionPanel::SlotSetCheckedShowOnlyLiquidLeftValve(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        int flag = GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag;
        if (set == Qt::Checked)
            flag = flag | GRAPH_SHOW_DROPLET_LEFT_VALVE_MEAN;
        else
            flag = flag & ~GRAPH_SHOW_DROPLET_LEFT_VALVE_MEAN;
        GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag = flag;
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void TrendGraphOptionPanel::SlotSetCheckedShowOnlyLiquidRightValve(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        int flag = GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag;
        if (set == Qt::Checked)
            flag = flag | GRAPH_SHOW_DROPLET_RIGHT_VALVE_MEAN;
        else
            flag = flag & ~GRAPH_SHOW_DROPLET_RIGHT_VALVE_MEAN;
        GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag = flag;
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void TrendGraphOptionPanel::SlotSetCheckedShowDataPointsLeftValve(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        int flag = GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag;
        if (set == Qt::Checked)
            flag = flag | GRAPH_SHOW_DROPLET_LEFT_VALVE_RAW_POINTS;
        else
            flag = flag & ~GRAPH_SHOW_DROPLET_LEFT_VALVE_RAW_POINTS;
        GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag = flag;
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void TrendGraphOptionPanel::SlotSetCheckedShowStdLeftValve(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        int flag = GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag;
        if (set == Qt::Checked)
            flag = flag | GRAPH_SHOW_DROPLET_LEFT_VALVE_STD;
        else
            flag = flag & ~GRAPH_SHOW_DROPLET_LEFT_VALVE_STD;
        GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag = flag;
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void TrendGraphOptionPanel::SlotSetCheckedShowDataPointsRightValve(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        int flag = GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag;
        if (set == Qt::Checked)
            flag = flag | GRAPH_SHOW_DROPLET_RIGHT_VALVE_RAW_POINTS;
        else
            flag = flag & ~GRAPH_SHOW_DROPLET_RIGHT_VALVE_RAW_POINTS;
        GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag = flag;
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void TrendGraphOptionPanel::SlotSetCheckedShowStdRightValve(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        int flag = GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag;
        if (set == Qt::Checked)
            flag = flag | GRAPH_SHOW_DROPLET_RIGHT_VALVE_STD;
        else
            flag = flag & ~GRAPH_SHOW_DROPLET_RIGHT_VALVE_STD;
        GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag = flag;
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void TrendGraphOptionPanel::SlotSetCheckedBottlesPerMin(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        int flag = GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag;
        if (set == Qt::Checked)
            flag = flag | GRAPH_SHOW_BOTTLES_PER_MIN;
        else
            flag = flag & ~GRAPH_SHOW_BOTTLES_PER_MIN;
        GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag = flag;
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void TrendGraphOptionPanel::SlotSetCheckedEjectedBottles(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        int flag = GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag;
        if (set == Qt::Checked)
            flag = flag | GRAPH_SHOW_EJECTED_BOTTLES;
        else
            flag = flag & ~GRAPH_SHOW_EJECTED_BOTTLES;
        GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag = flag;
        GetMainAppCrystalT2()->SaveSettings();
    }
}

// Show Temperature
void TrendGraphOptionPanel::SlotCheckBoxShowPiezoLeft(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        int flag = GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag;
        if (set == Qt::Checked)
            flag = flag | GRAPH_SHOW_TEMP_PIEZO_LEFT;
        else
            flag = flag & ~GRAPH_SHOW_TEMP_PIEZO_LEFT;
        GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag = flag;
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void TrendGraphOptionPanel::SlotCheckBoxShowPiezoRight(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        int flag = GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag;
        if (set == Qt::Checked)
            flag = flag | GRAPH_SHOW_TEMP_PIEZO_RIGHT;
        else
            flag = flag & ~GRAPH_SHOW_TEMP_PIEZO_RIGHT;
        GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag = flag;
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void TrendGraphOptionPanel::SlotCheckBoxShowChamberLeft(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        int flag = GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag;
        if (set == Qt::Checked)
            flag = flag | GRAPH_SHOW_TEMP_CHAMBER_LEFT;
        else
            flag = flag & ~GRAPH_SHOW_TEMP_CHAMBER_LEFT;
        GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag = flag;
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void TrendGraphOptionPanel::SlotCheckBoxShowChamberRight(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        int flag = GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag;
        if (set == Qt::Checked)
            flag = flag | GRAPH_SHOW_TEMP_CHAMBER_RIGHT;
        else
            flag = flag & ~GRAPH_SHOW_TEMP_CHAMBER_RIGHT;
        GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag = flag;
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void TrendGraphOptionPanel::SlotCheckBoxShowPreasureTank(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        int flag = GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag;
        if (set == Qt::Checked)
            flag = flag | GRAPH_SHOW_TEMP_PREASURE_TANK;
        else
            flag = flag & ~GRAPH_SHOW_TEMP_PREASURE_TANK;
        GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag = flag;
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void TrendGraphOptionPanel::SlotCheckBoxShowHeatingPipe(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        int flag = GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag;
        if (set == Qt::Checked)
            flag = flag | GRAPH_SHOW_HEATING_PIPE;
        else
            flag = flag & ~GRAPH_SHOW_HEATING_PIPE;
        GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag = flag;
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void TrendGraphOptionPanel::SlotCheckBoxWaterCooling(int set)
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        int flag = GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag;
        if (set == Qt::Checked)
            flag = flag | GRAPH_SHOW_WATER_COOLING;
        else
            flag = flag & ~GRAPH_SHOW_WATER_COOLING;
        GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag = flag;
        GetMainAppCrystalT2()->SaveSettings();
    }
}

void TrendGraphOptionPanel::SlotRollingMeanSizeChanged()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphRollingMeanSizeBottlesPerMin = m_SpinBoxRollingMeanSize->value();
        GetMainAppCrystalT2()->SaveSettings();
    }
}

QFileInfoList TrendGraphOptionPanel::GetTrendFileInfoList()
{
    QString PathTrendGraphData = GetMainAppCrystalT2()->GetTrendGraphDataLocation();
    QDir Path(PathTrendGraphData);
    QStringList filters;

    filters << "*.csv";
    Path.setFilter(QDir::Files);
    Path.setNameFilters(filters);
    Path.setSorting(QDir::Time);
    return Path.entryInfoList();
}

void TrendGraphOptionPanel::showEvent(QShowEvent* ev)
{
    QFileInfoList list = GetTrendFileInfoList();
    QString BaseName;
    int row = 0;

    m_ListWidgetLoadFile->clear();
    for (int i = 0; i < list.count(); i++) {
        BaseName = list.at(i).baseName();
        m_ListWidgetLoadFile->insertItem(row, BaseName);
        row++;
    }
    m_ListWidgetLoadFile->setCurrentRow(0, QItemSelectionModel::Select);
    m_ListWidgetLoadFile->setFocus();
    if (m_SpinBoxRollingMeanSize && GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData())
        m_SpinBoxRollingMeanSize->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphRollingMeanSizeBottlesPerMin);
}