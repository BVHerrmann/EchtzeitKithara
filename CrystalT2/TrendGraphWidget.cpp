#include "TrendGraphWidget.h"
#include "GlobalConst.h"
#include "MainAppCrystalT2.h"
#include "PlotTrendGraph.h"
#include "SettingsData.h"

#include <audittrail.h>

TrendGraphWidget::TrendGraphWidget(MainAppCrystalT2* pMainAppCrystalT2)
    : QWidget(pMainAppCrystalT2),
      m_PlotTrendGraphTemperature(NULL),
      m_PlotTrendGraphLiquid(NULL),
      m_MainAppCrystalT2(NULL),
      m_EnableDrawCurrentTrendDataLiquid(true),
      m_EnableDrawCurrentTrendDataTemperature(true),
      m_ComboBoxTrendGraphTimeWindow(NULL),
      m_RangeTimeAxisInMin(3),
      m_LiquidYAxisZoomFactor(1.0),
      m_kLiquidMinAxisYRange(20)
{
    ui.setupUi(this);
    m_MainAppCrystalT2 = pMainAppCrystalT2;

    m_ComboBoxTrendGraphTimeWindow = new QComboBox();

    m_ComboBoxTrendGraphTimeWindow->insertItem(TREND_GRAPH_TIME_WINDOW_INDEX_THREE_MIN, QString("3"));
    m_ComboBoxTrendGraphTimeWindow->insertItem(TREND_GRAPH_TIME_WINDOW_INDEX_FIVE_MIN, QString("5"));
    m_ComboBoxTrendGraphTimeWindow->insertItem(TREND_GRAPH_TIME_WINDOW_INDEX_FIFTEEN_MIN, QString("15"));
    m_ComboBoxTrendGraphTimeWindow->insertItem(TREND_GRAPH_TIME_WINDOW_INDEX_ONE_HOUR, QString("60"));
    m_ComboBoxTrendGraphTimeWindow->insertItem(TREND_GRAPH_TIME_WINDOW_INDEX_THREE_HOUR, QString("180"));

    ui.textEditTimeWindow->setReadOnly(true);
    m_ComboBoxTrendGraphTimeWindow->setCurrentIndex(TREND_GRAPH_TIME_WINDOW_INDEX_THREE_MIN);
    SetTextTimeWindow(m_ComboBoxTrendGraphTimeWindow->currentText());
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        m_ComboBoxTrendGraphTimeWindow->setCurrentIndex(GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphIimeRangeIndex);
        SetTextTimeWindow(m_ComboBoxTrendGraphTimeWindow->currentText());
        SetTimeValueTimeWindowInMin(GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphIimeRangeIndex);

        ui.verticalSliderLiquidMinYValue->setRange(0, GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphAbsolutMaximumLiquid);
        ui.verticalSliderLiquidMaxYValue->setRange(0, GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphAbsolutMaximumLiquid);
        ui.verticalSliderLiquidMinYValue->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphMinRangeLiquid);
        ui.verticalSliderLiquidMaxYValue->setValue(GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphMaxRangeLiquid);
    }
    m_PlotTrendGraphTemperature = new PlotTrendGraph(this, ui.PlotFrameTemperature, TREND_GRAPH_PLOT_ID_TEMPERATURE);
    m_PlotTrendGraphLiquid = new PlotTrendGraph(this, ui.PlotFrameLiquid, TREND_GRAPH_PLOT_ID_LIQUID);

    ui.textEditTimeWindow->setAlignment(Qt::AlignCenter);

    ui.textEditTimeWindow->setProperty(kAuditTrail, tr("TrendGraph: Time Axis"));
    
    connect(ui.timeEditSelectMinTimeLiquid, &QTimeEdit::editingFinished, this, &TrendGraphWidget::SlotApplyRangeLiquid);
    connect(ui.timeEditSelectMaxTimeLiquid, &QTimeEdit::editingFinished, this, &TrendGraphWidget::SlotApplyRangeLiquid);
    connect(ui.pushButtonShowFullRange, &QPushButton::clicked, this, &TrendGraphWidget::SlotShowFullRangeLiquid);

    connect(ui.timeEditSelectMinTimeTemperature, &QTimeEdit::editingFinished, this, &TrendGraphWidget::SlotApplyRangeTemperature);
    connect(ui.timeEditSelectMaxTimeTemperature, &QTimeEdit::editingFinished, this, &TrendGraphWidget::SlotApplyRangeTemperature);
    connect(ui.pushButtonShowFullRangeTemperature, &QPushButton::clicked, this, &TrendGraphWidget::SlotShowFullRangeTemperature);

    connect(ui.pushButtonMinusTimeWindow, &QPushButton::clicked, this, &TrendGraphWidget::SlotMinusTrendGraphTimeWindowChange);
    connect(ui.pushButtonPlusTimeWindow, &QPushButton::clicked, this, &TrendGraphWidget::SlotPlusTrendGraphTimeWindowChange);

    connect(ui.verticalSliderLiquidMinYValue, &QSlider::valueChanged, this, &TrendGraphWidget::SlotSliderLiquidMinYValueChanged);
    connect(ui.verticalSliderLiquidMaxYValue, &QSlider::valueChanged, this, &TrendGraphWidget::SlotSliderLiquidMaxYValueChanged);

    connect(ui.verticalSliderLiquidMinYValue, &QSlider::sliderReleased, this, &TrendGraphWidget::SlotSliderLiquidMinYRelease);
    connect(ui.verticalSliderLiquidMaxYValue, &QSlider::sliderReleased, this, &TrendGraphWidget::SlotSliderLiquidMaxYRelease);

    ui.pushButtonMinusTimeWindow->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    ui.pushButtonPlusTimeWindow->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));

    ui.timeEditSelectMinTimeLiquid->setDisplayFormat("hh:mm:ss");
    ui.timeEditSelectMaxTimeLiquid->setDisplayFormat("hh:mm:ss");

    ui.timeEditSelectMinTimeTemperature->setDisplayFormat("hh:mm:ss");
    ui.timeEditSelectMaxTimeTemperature->setDisplayFormat("hh:mm:ss");

    HideTimeRangeControlsLiquid();
    HideTimeRangeControlsTemperature();
    SetRequiredAccessLevel();
}

TrendGraphWidget::~TrendGraphWidget()
{
}

void TrendGraphWidget::SetRequiredAccessLevel()
{
    ui.verticalSliderLiquidMinYValue->setProperty(kRequiredAccessLevel, kAccessLevelUser);
    ui.verticalSliderLiquidMaxYValue->setProperty(kRequiredAccessLevel, kAccessLevelUser);
    
    ui.pushButtonMinusTimeWindow->setProperty(kRequiredAccessLevel, kAccessLevelUser);
    ui.pushButtonPlusTimeWindow->setProperty(kRequiredAccessLevel, kAccessLevelUser);
}

int TrendGraphWidget::GetRollingMeanValueLiquid()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData())
        return GetMainAppCrystalT2()->GetSettingsData()->m_RollingMeanValueLiquid;
    else
        return 100;
}

int TrendGraphWidget::GetSliderLiquidMaxYValue()
{
    return ui.verticalSliderLiquidMaxYValue->value();
}

int TrendGraphWidget::GetSliderLiquidMinYValue()
{
    return ui.verticalSliderLiquidMinYValue->value();
}

int TrendGraphWidget::GetTrendGraphAbsolutMaximumTemperature()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData())
        return GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphAbsolutMaximumTemperature;
    else
        return 100;
}

int TrendGraphWidget::GetTrendGraphAbsolutMinimumTemperature()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData())
        return GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphAbsolutMinimumTemperature;
    else
        return 0;
}

int TrendGraphWidget::GetTrendGraphAbsolutMaximumBottlesPerMin()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData())
        return GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphAbsolutMaximumBottlesPerMin;
    else
        return 400;
}

int TrendGraphWidget::GetTrendGraphAbsolutMinimumBottlesPerMin()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData())
        return GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphAbsolutMinimumBottlesPerMin;
    else
        return 0;
}

void TrendGraphWidget::SlotSliderLiquidMinYRelease()
{
    DrawTrendDataLiquid();
}

void TrendGraphWidget::SlotSliderLiquidMaxYRelease()
{
    DrawTrendDataLiquid();
}

void TrendGraphWidget::SlotSliderLiquidMinYValueChanged(int value)
{
    if (value >= (GetSliderLiquidMaxYValue() - m_kLiquidMinAxisYRange)) {
        disconnect(ui.verticalSliderLiquidMinYValue, &QSlider::valueChanged, this, &TrendGraphWidget::SlotSliderLiquidMinYValueChanged);
        ui.verticalSliderLiquidMinYValue->setValue(GetSliderLiquidMaxYValue() - m_kLiquidMinAxisYRange);
        connect(ui.verticalSliderLiquidMinYValue, &QSlider::valueChanged, this, &TrendGraphWidget::SlotSliderLiquidMinYValueChanged);
    } else {
        GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphMinRangeLiquid = value;
        GetMainAppCrystalT2()->SaveSettings();
    }
    QToolTip::showText(mapToGlobal(QPoint(ui.verticalSliderLiquidMinYValue->geometry().x() + ui.verticalSliderLiquidMinYValue->geometry().width(), ui.verticalSliderLiquidMinYValue->geometry().y())),
                       QString::number(value), this);
}

void TrendGraphWidget::SlotSliderLiquidMaxYValueChanged(int value)
{
    if (value <= (GetSliderLiquidMinYValue() + m_kLiquidMinAxisYRange)) {
        disconnect(ui.verticalSliderLiquidMaxYValue, &QSlider::valueChanged, this, &TrendGraphWidget::SlotSliderLiquidMaxYValueChanged);
        ui.verticalSliderLiquidMaxYValue->setValue(GetSliderLiquidMinYValue() + m_kLiquidMinAxisYRange);
        connect(ui.verticalSliderLiquidMaxYValue, &QSlider::valueChanged, this, &TrendGraphWidget::SlotSliderLiquidMaxYValueChanged);
    } else {
        GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphMaxRangeLiquid = value;
        GetMainAppCrystalT2()->SaveSettings();
    }
    QToolTip::showText(mapToGlobal(QPoint(ui.verticalSliderLiquidMaxYValue->geometry().x() + ui.verticalSliderLiquidMaxYValue->geometry().width(), ui.verticalSliderLiquidMaxYValue->geometry().y())),
                       QString::number(value), this);
}

int TrendGraphWidget::WriteLogFile(const QString& data, const QString& FileName)
{
    int rv = 0;
    if (GetMainAppCrystalT2()) rv = GetMainAppCrystalT2()->WriteLogFile(data, FileName);
    return rv;
}

void TrendGraphWidget::SlotMinusTrendGraphTimeWindowChange()
{
    int currentIndex = m_ComboBoxTrendGraphTimeWindow->currentIndex();
    if (currentIndex > 0) {
        currentIndex--;
        m_ComboBoxTrendGraphTimeWindow->setCurrentIndex(currentIndex);
        SetTimeValueTimeWindowInMin(currentIndex);
        if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
            GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphIimeRangeIndex = currentIndex;
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

int TrendGraphWidget::GetTrendGraphFlag()
{
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData())
        return GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphFlag;
    else
        return (GRAPH_SHOW_TEMP_PIEZO_LEFT & GRAPH_SHOW_TEMP_PIEZO_RIGHT & GRAPH_SHOW_TEMP_CHAMBER_LEFT & GRAPH_SHOW_TEMP_CHAMBER_RIGHT & GRAPH_SHOW_TEMP_PREASURE_TANK &
                GRAPH_SHOW_DROPLET_LEFT_VALVE_MEAN & GRAPH_SHOW_DROPLET_RIGHT_VALVE_MEAN);
}

void TrendGraphWidget::SlotPlusTrendGraphTimeWindowChange()
{
    int currentIndex = m_ComboBoxTrendGraphTimeWindow->currentIndex();
    if (currentIndex < 4) {
        currentIndex++;
        m_ComboBoxTrendGraphTimeWindow->setCurrentIndex(currentIndex);
        SetTimeValueTimeWindowInMin(currentIndex);
        if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
            GetMainAppCrystalT2()->GetSettingsData()->m_TrendGraphIimeRangeIndex = currentIndex;
            GetMainAppCrystalT2()->SaveSettings();
        }
    }
}

void TrendGraphWidget::SetTimeValueTimeWindowInMin(int CurrentIndex)
{
    switch (CurrentIndex) {
        case TREND_GRAPH_TIME_WINDOW_INDEX_THREE_MIN:
            m_RangeTimeAxisInMin = 3;
            SetTextTimeWindow(m_ComboBoxTrendGraphTimeWindow->currentText());
            break;
        case TREND_GRAPH_TIME_WINDOW_INDEX_FIVE_MIN:
            m_RangeTimeAxisInMin = 5;
            SetTextTimeWindow(m_ComboBoxTrendGraphTimeWindow->currentText());
            break;
        case TREND_GRAPH_TIME_WINDOW_INDEX_FIFTEEN_MIN:
            m_RangeTimeAxisInMin = 15;
            SetTextTimeWindow(m_ComboBoxTrendGraphTimeWindow->currentText());
            break;
        case TREND_GRAPH_TIME_WINDOW_INDEX_ONE_HOUR:
            m_RangeTimeAxisInMin = 60;
            SetTextTimeWindow(m_ComboBoxTrendGraphTimeWindow->currentText());
            break;
        case TREND_GRAPH_TIME_WINDOW_INDEX_THREE_HOUR:
            m_RangeTimeAxisInMin = 180;
            SetTextTimeWindow(m_ComboBoxTrendGraphTimeWindow->currentText());
            break;
        default:
            m_RangeTimeAxisInMin = 3;
            SetTextTimeWindow(m_ComboBoxTrendGraphTimeWindow->currentText());
            break;
    }
}

void TrendGraphWidget::SetTextTimeWindow(const QString& Time)
{
    QString text = tr("Show the last %1").arg(Time);
    ui.textEditTimeWindow->clear();
    ui.textEditTimeWindow->setText(text);
    ui.textEditTimeWindow->setAlignment(Qt::AlignCenter);
}

void TrendGraphWidget::SlotApplyRangeLiquid()
{
    QTime MinTime = ui.timeEditSelectMinTimeLiquid->time();
    QTime MaxTime = ui.timeEditSelectMaxTimeLiquid->time();
    if (MinTime < MaxTime) DrawTrendDataLiquidInterval(MinTime, MaxTime);
}

void TrendGraphWidget::SlotApplyRangeTemperature()
{
    QTime MinTime = ui.timeEditSelectMinTimeTemperature->time();
    QTime MaxTime = ui.timeEditSelectMaxTimeTemperature->time();
    if (MinTime < MaxTime) DrawTrendDataTemperatureInterval(MinTime, MaxTime);
}

void TrendGraphWidget::SlotShowFullRangeTemperature()
{
    SetFullTimeRangeTemperature();
}

void TrendGraphWidget::SlotShowFullRangeLiquid()
{
    SetFullTimeRangeLiquid();
}

void TrendGraphWidget::showEvent(QShowEvent* ev)
{
    if (GetPlotTrendGraphTemperature()) GetPlotTrendGraphTemperature()->resize(ui.PlotFrameTemperature->width(), ui.PlotFrameTemperature->height());
    if (GetPlotTrendGraphLiquid()) GetPlotTrendGraphLiquid()->resize(ui.PlotFrameLiquid->width(), ui.PlotFrameLiquid->height());
}

bool TrendGraphWidget::WorkWithTwoValves()
{
    bool rv = true;
    if (GetMainAppCrystalT2()) rv = GetMainAppCrystalT2()->WorkWithTwoValves();
    return rv;
}

void TrendGraphWidget::ClearTrendGraphTemperatureData()
{
    m_ListValveTemperatureData.clear();
}

void TrendGraphWidget::ClearTrendGraphLiquidData()
{
    m_ListValveLiquidData.clear();
}

void TrendGraphWidget::ClearTrendGraphTemperatureDataFromFile()
{
    m_ListValveTemperatureDataFromFile.clear();
}

void TrendGraphWidget::ClearTrendGraphLiquidDataFromFile()
{
    m_ListValveLiquidDataFromFile.clear();
}

void TrendGraphWidget::AddNewTemperatureData(TrendTemperatureData& temperatureData)
{
    m_ListValveTemperatureData.append(temperatureData);
    if (m_ListValveTemperatureData.count() > 1) {
        qint64 minDiff;
        while (true) {
            // maximal die letzten 3 stunden im Speicher halten
            minDiff = m_ListValveTemperatureData.at(0).m_Time.msecsTo(m_ListValveTemperatureData.at(m_ListValveTemperatureData.count() - 1).m_Time) / 60000.0;
            if (minDiff >= 180) {
                m_ListValveTemperatureData.pop_front();
            } else
                break;
        }
    }
}

void TrendGraphWidget::AddNewTemperatureDataFromFile(TrendTemperatureData& temperatureData)
{
    m_ListValveTemperatureDataFromFile.append(temperatureData);
}

void TrendGraphWidget::DrawTrendDataTemperature()
{
    if (m_EnableDrawCurrentTrendDataTemperature && GetPlotTrendGraphTemperature()) {
        QDateTime CurrentDateTime = QDateTime::currentDateTime();
        int startPos = GetStartPosDrawTemperatureData(CurrentDateTime);
        if (startPos > -1) {
            QList<TrendTemperatureData> sublist = m_ListValveTemperatureData.mid(startPos);
            GetPlotTrendGraphTemperature()->DrawTrendDataTemperature(sublist, CurrentDateTime);
        } else {
            GetPlotTrendGraphTemperature()->DrawTrendDataTemperature(m_ListValveTemperatureData, CurrentDateTime);
        }
    }
}

void TrendGraphWidget::DrawTrendDataLiquid()
{
    if (m_EnableDrawCurrentTrendDataLiquid && GetPlotTrendGraphLiquid()) {
        QDateTime CurrentDateTime = QDateTime::currentDateTime();
        int startPos = GetStartPosDrawLiquidData(CurrentDateTime);
        if (startPos > -1) {
            QList<TrendLiquidData> sublist = m_ListValveLiquidData.mid(startPos);
            GetPlotTrendGraphLiquid()->DrawTrendDataLiquid(sublist, CurrentDateTime);
        } else {
            GetPlotTrendGraphLiquid()->DrawTrendDataLiquid(m_ListValveLiquidData, CurrentDateTime);
        }
    }
}

int TrendGraphWidget::GetStartPosDrawTemperatureData(QDateTime& CurrentDateTime)
{
    int pos = -1;
    QTime startTime = CurrentDateTime.time().addSecs(m_RangeTimeAxisInMin * 60 * -1);

    for (int i = 0; i < m_ListValveTemperatureData.count(); i++) {
        if (startTime < m_ListValveTemperatureData.at(i).m_Time) {
            pos = i;
            break;
        }
    }
    return pos;
}

int TrendGraphWidget::GetStartPosDrawLiquidData(QDateTime& CurrentDateTime)
{
    int pos = -1;
    QTime startTime = CurrentDateTime.time().addSecs(m_RangeTimeAxisInMin * 60 * -1);

    for (int i = 0; i < m_ListValveLiquidData.count(); i++) {
        if (startTime < m_ListValveLiquidData.at(i).m_Time) {
            pos = i;
            break;
        }
    }
    return pos;
}

void TrendGraphWidget::AddNewLiquidData(TrendLiquidData& valveLiquidData)
{
    m_ListValveLiquidData.append(valveLiquidData);
    if (m_ListValveLiquidData.count() > 1) {
        qint64 minDiff;
        while (true) {
            // maximal die letzten 3 stunden im Speicher halten
            minDiff = m_ListValveLiquidData.at(0).m_Time.msecsTo(m_ListValveLiquidData.at(m_ListValveLiquidData.count() - 1).m_Time) / 60000.0;
            if (minDiff >= 180) {
                m_ListValveLiquidData.pop_front();
            } else
                break;
        }
    }
}

void TrendGraphWidget::AddNewLiquidDataFromFile(TrendLiquidData& valveLiquidData)
{
    m_ListValveLiquidDataFromFile.append(valveLiquidData);
}

void TrendGraphWidget::DrawTrendDataTemperatureFromFile(QDateTime& CurrentDateTimeFile)
{
    m_CurrentFileDateTemperature = CurrentDateTimeFile;
    SetFullTimeRangeTemperature();
}

void TrendGraphWidget::SetFullTimeRangeTemperature()
{
    if (GetPlotTrendGraphTemperature()) {
        if (m_ListValveTemperatureDataFromFile.count() > 2) {
            QTime MinTime = m_ListValveTemperatureDataFromFile.at(0).m_Time;
            QTime MaxTime = m_ListValveTemperatureDataFromFile.at(m_ListValveTemperatureDataFromFile.count() - 1).m_Time;

            disconnect(ui.timeEditSelectMinTimeTemperature, &QTimeEdit::editingFinished, this, &TrendGraphWidget::SlotApplyRangeTemperature);
            disconnect(ui.timeEditSelectMaxTimeTemperature, &QTimeEdit::editingFinished, this, &TrendGraphWidget::SlotApplyRangeTemperature);

            ui.timeEditSelectMinTimeTemperature->setTime(MinTime);
            ui.timeEditSelectMaxTimeTemperature->setTime(MaxTime);

            connect(ui.timeEditSelectMinTimeTemperature, &QTimeEdit::editingFinished, this, &TrendGraphWidget::SlotApplyRangeTemperature);
            connect(ui.timeEditSelectMaxTimeTemperature, &QTimeEdit::editingFinished, this, &TrendGraphWidget::SlotApplyRangeTemperature);
        }
        GetPlotTrendGraphTemperature()->DrawTrendDataTemperature(m_ListValveTemperatureDataFromFile, m_CurrentFileDateTemperature, true);
    }
}

void TrendGraphWidget::DrawTrendDataLiquidFromFile(QDateTime& CurrentFileDateLiquid)
{
    m_CurrentFileDateLiquid = CurrentFileDateLiquid;
    SetFullTimeRangeLiquid();
}

void TrendGraphWidget::SetFullTimeRangeLiquid()
{
    if (GetPlotTrendGraphLiquid()) {
        if (m_ListValveLiquidDataFromFile.count() > 2) {
            QTime MinTime = m_ListValveLiquidDataFromFile.at(0).m_Time;
            QTime MaxTime = m_ListValveLiquidDataFromFile.at(m_ListValveLiquidDataFromFile.count() - 1).m_Time;

            disconnect(ui.timeEditSelectMinTimeLiquid, &QTimeEdit::editingFinished, this, &TrendGraphWidget::SlotApplyRangeLiquid);
            disconnect(ui.timeEditSelectMaxTimeLiquid, &QTimeEdit::editingFinished, this, &TrendGraphWidget::SlotApplyRangeLiquid);

            ui.timeEditSelectMinTimeLiquid->setTime(MinTime);
            ui.timeEditSelectMaxTimeLiquid->setTime(MaxTime);

            connect(ui.timeEditSelectMinTimeLiquid, &QTimeEdit::editingFinished, this, &TrendGraphWidget::SlotApplyRangeLiquid);
            connect(ui.timeEditSelectMaxTimeLiquid, &QTimeEdit::editingFinished, this, &TrendGraphWidget::SlotApplyRangeLiquid);
        }
        GetPlotTrendGraphLiquid()->DrawTrendDataLiquid(m_ListValveLiquidDataFromFile, m_CurrentFileDateLiquid, true);
    }
}

void TrendGraphWidget::ShowTimeRangeControlsLiquid()
{
    ui.widgetShowControlsLiquid->show();
}

void TrendGraphWidget::HideTimeRangeControlsLiquid()
{
    ui.widgetShowControlsLiquid->hide();
}

void TrendGraphWidget::ShowTimeRangeControlsTemperature()
{
    ui.widgetShowControlsTemperature->show();
}

void TrendGraphWidget::HideTimeRangeControlsTemperature()
{
    ui.widgetShowControlsTemperature->hide();
}

void TrendGraphWidget::ShowControlsTimeIntervall(bool sh)
{
    if (sh) {
        ui.widgetControlsTimeWindow->show();
    } else {
        ui.widgetControlsTimeWindow->hide();
    }
}

void TrendGraphWidget::DrawTrendDataLiquidInterval(QTime MinTime, QTime MaxTime)
{
    if (m_ListValveLiquidDataFromFile.count() > 3) {
        int MinIndex = GetIndexTrendDataLiquidByTime(MinTime);
        int MaxIndex = GetIndexTrendDataLiquidByTime(MaxTime);

        if (MinIndex < MaxIndex) {
            GetPlotTrendGraphLiquid()->DrawTrendDataLiquid(m_ListValveLiquidDataFromFile, m_CurrentFileDateLiquid, true, MinIndex, MaxIndex);
        }
    }
}

void TrendGraphWidget::DrawTrendDataTemperatureInterval(QTime MinTime, QTime MaxTime)
{
    if (m_ListValveTemperatureDataFromFile.count() > 3) {
        int MinIndex = GetIndexTrendDataTemperatureByTime(MinTime);
        int MaxIndex = GetIndexTrendDataTemperatureByTime(MaxTime);

        if (MinIndex < MaxIndex) {
            GetPlotTrendGraphTemperature()->DrawTrendDataTemperature(m_ListValveTemperatureDataFromFile, m_CurrentFileDateTemperature, true, MinIndex, MaxIndex);
        }
    }
}

int TrendGraphWidget::GetIndexTrendDataLiquidByTime(QTime time)
{
    int rv = 0;
    for (int i = 0; i < m_ListValveLiquidDataFromFile.count(); i++) {
        if (m_ListValveLiquidDataFromFile.at(i).m_Time >= time) {
            rv = i;
            break;
        }
    }
    return rv;
}

int TrendGraphWidget::GetIndexTrendDataTemperatureByTime(QTime time)
{
    int rv = 0;
    for (int i = 0; i < m_ListValveTemperatureDataFromFile.count(); i++) {
        if (m_ListValveTemperatureDataFromFile.at(i).m_Time >= time) {
            rv = i;
            break;
        }
    }
    return rv;
}

QList<TrendTemperatureData>* TrendGraphWidget::GetListTemperatureData()
{
    return &m_ListValveTemperatureData;
}

QList<TrendLiquidData>* TrendGraphWidget::GetListLiquidData()
{
    return &m_ListValveLiquidData;
}

QList<TrendTemperatureData>* TrendGraphWidget::GetListTemperatureDataFromFile()
{
    return &m_ListValveTemperatureDataFromFile;
}

QList<TrendLiquidData>* TrendGraphWidget::GetListLiquidDataFromFile()
{
    return &m_ListValveLiquidDataFromFile;
}

void TrendGraphWidget::CheckFileContent(QString& FileName, int PlotID)
{
    QFile CurrentFile(FileName);
    QDateTime dateTime;
    if (CurrentFile.exists()) {
        if (PlotID == TREND_GRAPH_PLOT_ID_TEMPERATURE) {
            if (m_ListValveTemperatureData.count() == 0) LoadDataFromCSVFile(FileName, PlotID, dateTime);
        } else {
            if (m_ListValveLiquidData.count() == 0) LoadDataFromCSVFile(FileName, PlotID, dateTime);
        }
    }
}

int TrendGraphWidget::LoadDataFromCSVFile(QString& FileName, int PlotID, QDateTime& dateTime)
{
    int retVal = ERROR_CODE_NO_ERROR;
    QFile CurrentFile;
    QString DateString, line;
    QStringList fields;
    CurrentFile.setFileName(FileName);
    QFileInfo FileInfo(CurrentFile);
    QString BaseName = FileInfo.baseName();
    QStringList sl = BaseName.split(QRegularExpression("\\[|\\]"));

    if (sl.size() > 1) DateString = sl.at(1);
    QDate date = QDate::fromString(DateString, "dd_MM_yyyy");

    dateTime.setDate(date);
    if (PlotID == TREND_GRAPH_PLOT_ID_TEMPERATURE)
        ClearTrendGraphTemperatureDataFromFile();
    else
        ClearTrendGraphLiquidDataFromFile();

    if (CurrentFile.open(QIODevice::ReadOnly | QIODevice::Text)) {  // read data
        QTextStream in(&CurrentFile);
        int linecount = 0;
        while (!in.atEnd()) {
            line = in.readLine();
            if (linecount > 0)  // ignor first line
            {
                fields = line.split(CSV_FILE_SEPERATOR);
                if (PlotID == TREND_GRAPH_PLOT_ID_TEMPERATURE)
                    InsertNewTemperatureData(fields);
                else
                    InsertNewLiquidData(fields);
            }
            linecount++;
        }
        CurrentFile.close();
    } else
        retVal = ERROR_CODE_ANY_ERROR;

    return retVal;
}

void TrendGraphWidget::InsertNewTemperatureData(QStringList& List)
{
    TrendTemperatureData temperatureData;
    QString Time = List.at(0);
    QString CurrentTempLeft = List.at(1);

    if (CurrentTempLeft.contains(TREND_GRAPH_FLAG_START_SOFTWARE) || CurrentTempLeft.contains(TREND_GRAPH_FLAG_FINISHED_SOFTWARE)) {
        if (CurrentTempLeft.contains(TREND_GRAPH_FLAG_START_SOFTWARE))
            temperatureData.m_SoftwareStart = true;
        else
            temperatureData.m_SoftwareFinished = true;
    } else {
        QString StackTempLeft = List.at(2);
        QString CurrentTempRight = List.at(3);
        QString StackTempRight = List.at(4);
        QString PreasureTankTemp = List.at(5);

        temperatureData.m_CurrentTemperatureLeftValve = CurrentTempLeft.toDouble();
        temperatureData.m_StackTemperatureLeftValve = StackTempLeft.toDouble();
        temperatureData.m_CurrentTemperatureRightValve = CurrentTempRight.toDouble();
        temperatureData.m_StackTemperatureRightValve = StackTempRight.toDouble();
        temperatureData.m_CurrentPreasureTankTemperature = PreasureTankTemp.toDouble();
    }
    temperatureData.m_Time = QTime::fromString(Time, Qt::ISODate);
    AddNewTemperatureDataFromFile(temperatureData);
}

void TrendGraphWidget::InsertNewLiquidData(QStringList& List)
{
    TrendLiquidData liquidData;
    QString Time = List.at(0);
    QString AmountLiquidLeftValve = List.at(1);

    if (AmountLiquidLeftValve.contains(TREND_GRAPH_FLAG_START_SOFTWARE) || AmountLiquidLeftValve.contains(TREND_GRAPH_FLAG_FINISHED_SOFTWARE)) {
        if (AmountLiquidLeftValve.contains(TREND_GRAPH_FLAG_START_SOFTWARE))
            liquidData.m_SoftwareStart = true;
        else
            liquidData.m_SoftwareFinished = true;
    } else {
        QString AmountLiquidStdLeftValve = List.at(2);
        QString AmountLiquidRightValve = List.at(3);
        QString AmountLiquidStdRightValve = List.at(4);
        QString SumAmountLiquid = List.at(5);
        QString SumAmountLiquidStd = List.at(6);
        QString EjectBottles = List.at(7);
        QString BottlesPerMin = List.at(8);
        QString MaschineState = List.at(9);

        AmountLiquidLeftValve.replace(QString(","), QString("."));
        AmountLiquidStdLeftValve.replace(QString(","), QString("."));

        AmountLiquidRightValve.replace(QString(","), QString("."));
        AmountLiquidStdRightValve.replace(QString(","), QString("."));

        SumAmountLiquid.replace(QString(","), QString("."));
        SumAmountLiquidStd.replace(QString(","), QString("."));

        BottlesPerMin = BottlesPerMin.replace(QString(","), QString("."));

        liquidData.m_AmountLiquidLeftValve = AmountLiquidLeftValve.toDouble();
        liquidData.m_AmountLiquidRightValve = AmountLiquidRightValve.toDouble();
        liquidData.m_SumAmountLiquid = SumAmountLiquid.toDouble();
        liquidData.m_StandardDeviationAmountLiquidLeftValve = AmountLiquidStdLeftValve.toDouble();
        liquidData.m_StandardDeviationAmountLiquidRightValve = AmountLiquidStdRightValve.toDouble();
        liquidData.m_StandardDeviationSumAmountLiquid = SumAmountLiquidStd.toDouble();
        liquidData.m_BottlesPerMinute = BottlesPerMin.toDouble();
        liquidData.m_MaschineState = MaschineState.toInt();
        liquidData.m_EjectBottle = EjectBottles.toInt();
    }
    liquidData.m_Time = QTime::fromString(Time, Qt::ISODate);
    AddNewLiquidDataFromFile(liquidData);
}
