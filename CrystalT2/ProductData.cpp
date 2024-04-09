#include "ProductData.h"
#include "GlobalConst.h"
#include "MainAppCrystalT2.h"
#include "SettingsData.h"
#include "SharedData.h"
#include "ValveData.h"

QDataStream& operator<<(QDataStream& ds, ProductData* pProductData)
{
    QHash<int, QRect>* pMeasureWindowRects = pProductData->GetMeasureWindowRects();
    QHashIterator<int, QRect> m(*pMeasureWindowRects);
    QHash<int, ValveData>* pValveData = pProductData->GetListValveData();
    QHashIterator<int, ValveData> v(*pValveData);

    ds << pProductData->m_BottleNeckDiameter;
    ds << pProductData->m_BotteleNeckDiameterToleranceInmm;
    ds << pProductData->m_TriggerOffsetFirstValveInmm;
    ds << pProductData->m_TriggerOffsetSecondValveInmm;

    while (m.hasNext()) {
        m.next();
        ds << m.key();
        ds << m.value().x();
        ds << m.value().y();
        ds << m.value().width();
        ds << m.value().height();
    }

    while (v.hasNext()) {
        v.next();
        ds << v.key();
        ds << v.value().m_ValvePulseTimeInms;
        ds << v.value().m_ValvePauseTimeInms;
        ds << v.value().m_ValveCount;
        ds << v.value().m_ValveModus;
        ds << v.value().m_ValveCloseVoltage;
        ds << v.value().m_ValveStrokeInPercent;
        ds << v.value().m_ValveCloseTimeInms;
        ds << v.value().m_ValveOpenTimeInms;
        ds << v.value().m_ValveTemperature;
        ds << v.value().m_ValveModeTemperature;
    }

    ds << pProductData->m_ProductWidth;
    ds << pProductData->m_DefaultPreasureInBar;
    ds << pProductData->m_DefaultPreasureTankTemp;
    ds << pProductData->m_DistanceInjectorBottle;
    ds << pProductData->m_InjectionAngle;
    ds << pProductData->m_VideoPositionLeftValveInmm;
    ds << pProductData->m_DistanceBetweenMeasurePosAndTriggerPosInPixel;
    ds << pProductData->m_BottleNeckInnerDiameter;
    ds << pProductData->m_MinAcceptanceThresholdLiquidMiddleROI;
    ds << pProductData->m_AcceptanceThresholdLiquidLeftAndRightROI;
    ds << pProductData->m_FormatFromISInmm;
    ds << pProductData->m_ThresholdDegreeOfPollution;
    ds << pProductData->m_LowThresholdDegreeOfPollution;
    ds << pProductData->m_InjectionMiddleWindowWidthInMm;
    ds << pProductData->m_UsedTriggerOutputs;
    ds << pProductData->m_VideoPositionRightValveInmm;
    ds << pProductData->m_MaxAcceptanceThresholdLiquidMiddleROI;
    ds << pProductData->m_DefaulHeatingPipeTemp;
    ds << pProductData->m_ValueAirCoolingCameraLitersPerMinute;
    ds << pProductData->m_ValueAirCoolingLightLitersPerMinute;
    ds << pProductData->m_ValueAirCoolingValveLitersPerMinute;
    ds << pProductData->m_ValueWaterCoolingDefaultLitersPerMinute;
    ds << pProductData->m_ValueWaterCoolingCircuitDefaultSensor;
    ds << pProductData->m_ValveWaterCoolingDefaultTemperature;
    return ds;
}

QDataStream& operator>>(QDataStream& ds, ProductData* pProductData)
{
    int valveDataID, MeasurRectKey, x, y, w, h;
    QRect MeasureRect;
    ValveData valveData;
    QHash<int, QRect>* pMeasureWindowRects = pProductData->GetMeasureWindowRects();
    QHashIterator<int, QRect> i(*pMeasureWindowRects);
    QHash<int, ValveData>* pValveData = pProductData->GetListValveData();
    QHashIterator<int, ValveData> v(*pValveData);

    if (!ds.atEnd()) ds >> pProductData->m_BottleNeckDiameter;
    if (!ds.atEnd()) ds >> pProductData->m_BotteleNeckDiameterToleranceInmm;
    if (!ds.atEnd()) ds >> pProductData->m_TriggerOffsetFirstValveInmm;
    if (!ds.atEnd()) ds >> pProductData->m_TriggerOffsetSecondValveInmm;

    if (!ds.atEnd()) {
        while (i.hasNext()) {
            i.next();
            MeasureRect = i.value();
            ds >> MeasurRectKey;
            ds >> x;
            ds >> y;
            ds >> w;
            ds >> h;
            MeasureRect.setX(x);
            MeasureRect.setY(y);
            MeasureRect.setWidth(w);
            MeasureRect.setHeight(h);
            pMeasureWindowRects->insert(MeasurRectKey, MeasureRect);
        }
    }

    if (!ds.atEnd()) {
        while (v.hasNext()) {
            v.next();
            ds >> valveData.m_ValveID;
            ds >> valveData.m_ValvePulseTimeInms;
            ds >> valveData.m_ValvePauseTimeInms;
            ds >> valveData.m_ValveCount;
            ds >> valveData.m_ValveModus;
            ds >> valveData.m_ValveCloseVoltage;
            ds >> valveData.m_ValveStrokeInPercent;
            ds >> valveData.m_ValveCloseTimeInms;
            ds >> valveData.m_ValveOpenTimeInms;
            ds >> valveData.m_ValveTemperature;
            ds >> valveData.m_ValveModeTemperature;
            pValveData->insert(valveData.m_ValveID, valveData);
        }
    }
    if (!ds.atEnd()) ds >> pProductData->m_ProductWidth;
    if (!ds.atEnd()) ds >> pProductData->m_DefaultPreasureInBar;
    if (!ds.atEnd()) ds >> pProductData->m_DefaultPreasureTankTemp;
    if (!ds.atEnd()) ds >> pProductData->m_DistanceInjectorBottle;
    if (!ds.atEnd()) ds >> pProductData->m_InjectionAngle;
    if (!ds.atEnd()) ds >> pProductData->m_VideoPositionLeftValveInmm;
    if (!ds.atEnd()) ds >> pProductData->m_DistanceBetweenMeasurePosAndTriggerPosInPixel;
    if (!ds.atEnd()) ds >> pProductData->m_BottleNeckInnerDiameter;
    if (!ds.atEnd()) ds >> pProductData->m_MinAcceptanceThresholdLiquidMiddleROI;
    if (!ds.atEnd()) ds >> pProductData->m_AcceptanceThresholdLiquidLeftAndRightROI;
    if (!ds.atEnd()) ds >> pProductData->m_FormatFromISInmm;
    if (!ds.atEnd()) ds >> pProductData->m_ThresholdDegreeOfPollution;
    if (!ds.atEnd()) ds >> pProductData->m_LowThresholdDegreeOfPollution;
    if (!ds.atEnd()) ds >> pProductData->m_InjectionMiddleWindowWidthInMm;
    if (!ds.atEnd()) ds >> pProductData->m_UsedTriggerOutputs;
    if (!ds.atEnd()) ds >> pProductData->m_VideoPositionRightValveInmm;
    if (!ds.atEnd()) ds >> pProductData->m_MaxAcceptanceThresholdLiquidMiddleROI;
    if (!ds.atEnd()) ds >> pProductData->m_DefaulHeatingPipeTemp;
    if (!ds.atEnd()) ds >> pProductData->m_ValueAirCoolingCameraLitersPerMinute;
    if (!ds.atEnd()) ds >> pProductData->m_ValueAirCoolingLightLitersPerMinute;
    if (!ds.atEnd()) ds >> pProductData->m_ValueAirCoolingValveLitersPerMinute;
    if (!ds.atEnd()) ds >> pProductData->m_ValueWaterCoolingDefaultLitersPerMinute;
    if (!ds.atEnd()) ds >> pProductData->m_ValueWaterCoolingCircuitDefaultSensor;
    if (!ds.atEnd()) ds >> pProductData->m_ValveWaterCoolingDefaultTemperature;

    return ds;
}

ProductData::ProductData(MainAppCrystalT2* parent)
    : QObject(parent),
      m_MainAppCrystalT2(NULL),
      m_BottleNeckDiameter(20.0),
      m_BottleNeckInnerDiameter(m_BottleNeckDiameter / 1.7),
      m_ProductWidth(100.0),
      m_BotteleNeckDiameterToleranceInmm(0.5),
      m_TriggerOffsetFirstValveInmm(0.0),
      m_TriggerOffsetSecondValveInmm(0.0),
      m_ProductName("DefaultProduct"),
      m_DefaultPreasureInBar(1.0),
      m_DefaultPreasureTankTemp(20.0),
      m_DefaulHeatingPipeTemp(20.0),
      m_DistanceInjectorBottle(30.0)  // in mm
      ,
      m_InjectionAngle(-5.0)  // in degree
      ,
      m_VideoPositionLeftValveInmm(0.0)  // in mm
      ,
      m_VideoPositionRightValveInmm(0.0)  // in mm
      ,
      m_DistanceBetweenMeasurePosAndTriggerPosInPixel(200),
      m_MinAcceptanceThresholdLiquidMiddleROI(1500)  // Anzahl Pixel. Berechneter Wert muss groeßer sein als die Schwelle
      ,
      m_MaxAcceptanceThresholdLiquidMiddleROI(3000)  // Anzahl Pixel. Berechneter Wert muss kleiner sein als die Schwelle
      ,
      m_AcceptanceThresholdLiquidLeftAndRightROI(200)  // Anzahl Pixel. Berechneter Wert muss kleiner sein als diese Schwelle
      ,
      m_FormatFromISInmm(FIVE_AND_1T4_INCH)  // 5(1/4) Zoll default
      ,
      m_InjectionMiddleWindowWidthInMm(8.0),
      m_ThresholdDegreeOfPollution(10.0)  // in Percent
      ,
      m_LowThresholdDegreeOfPollution(5.0)  // in Percent
      ,
      m_UsedTriggerOutputs(USE_BOTH_VALVES)  // beide Triggerausgänge sind aktiv
      ,
      m_ValueAirCoolingCameraLitersPerMinute(10.0),
      m_ValueAirCoolingLightLitersPerMinute(10.0),
      m_ValueAirCoolingValveLitersPerMinute(10.0),
      m_ValueWaterCoolingDefaultLitersPerMinute(10.0),
      m_ValueWaterCoolingCircuitDefaultSensor(15.0),
      m_ValveWaterCoolingDefaultTemperature(20.0)
{
    m_MainAppCrystalT2 = parent;
    QRect CameraROIRect;
    CameraROIRect.setX(0);
    CameraROIRect.setY(0);
    CameraROIRect.setWidth(USED_CAMERA_WIDTH);
    CameraROIRect.setHeight(USED_CAMERA_HEIGHT);
    m_MeasureWindowRects.insert(ROI_ID_CAMERA, CameraROIRect);  // camera ROI

    QRect MeasureWindowRect1;
    MeasureWindowRect1.setX(0);  // default value comes from valid settings
    MeasureWindowRect1.setY(341);
    MeasureWindowRect1.setWidth(302);
    MeasureWindowRect1.setHeight(18);
    m_MeasureWindowRects.insert(ROI_ID_MEASURE_SPEED, MeasureWindowRect1);

    QRect MeasureWindowRect2;
    MeasureWindowRect2.setX(403);  // default value comes from valid settings
    MeasureWindowRect2.setY(257);
    MeasureWindowRect2.setWidth(217);
    MeasureWindowRect2.setHeight(63);
    m_MeasureWindowRects.insert(ROI_ID_MEASURE_LIQUID, MeasureWindowRect2);

    QRect MeasureWindowRect3;
    MeasureWindowRect3.setX(MeasureWindowRect2.x());
    MeasureWindowRect3.setY(MeasureWindowRect1.y());
    MeasureWindowRect3.setWidth(MeasureWindowRect2.width());
    MeasureWindowRect3.setHeight(MeasureWindowRect1.height());
    m_MeasureWindowRects.insert(ROI_ID_MEASURE_BOTTLE_UNDER_VALVE, MeasureWindowRect3);

    m_InjectionPositionInPixel = MeasureWindowRect2.x() + MeasureWindowRect2.width() / 2.0;
    m_MeasurePositionInPixel = MeasureWindowRect1.x() + MeasureWindowRect1.width() / 2.0;

    ValveData RightValveData, LeftValveData;
    LeftValveData.m_ValvePulseTimeInms = 5.0;   // min value 0,1ms
    LeftValveData.m_ValvePauseTimeInms = 20.0;  // min 2ms - 10sec
    LeftValveData.m_ValveCount = 1;             // standard 1
    LeftValveData.m_ValveModus = VALVE_MODUS_TIMED;
    LeftValveData.m_ValveCloseVoltage = 100.0;
    LeftValveData.m_ValveStrokeInPercent = 80.0;
    LeftValveData.m_ValveCloseTimeInms = 2.0;
    LeftValveData.m_ValveOpenTimeInms = 0.25;
    LeftValveData.m_ValveTemperature = 20.0;
    LeftValveData.m_ValveModeTemperature = VALVE_MODE_HEATER_OFF;
    LeftValveData.m_ValveID = LEFT_VALVE_ID;
    RightValveData = LeftValveData;
    RightValveData.m_ValveID = RIGHT_VALVE_ID;
    m_ListValveData.insert(LEFT_VALVE_ID, LeftValveData);
    m_ListValveData.insert(RIGHT_VALVE_ID, RightValveData);
}

ProductData::~ProductData()
{
}

int ProductData::ReadProductData(QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    if (GetMainAppCrystalT2()) {
        QString NameAndPathProductFile = GetMainAppCrystalT2()->GetPathNameProducts() + QString("/") + m_ProductName + QString(".dat");
        QFile File(NameAndPathProductFile);

        if (File.open(QFile::ReadWrite)) {
            QDataStream in(&File);

            in >> this;
            File.close();
            if (GetMainAppCrystalT2()->GetSettingsData()) {
                if (!(GetMainAppCrystalT2()->GetSettingsData()->m_WorkWithSecondTriggerSlider))  // wird nicht mit dem zweiten Triggerslider gearbeitet, muss der Offset auf 0 gesetzt werden
                    m_TriggerOffsetSecondValveInmm = 0;
            }
        } else {
            ErrorMsg = tr("Can Not Open File(Read Product Data) %1").arg(NameAndPathProductFile);
            rv = ERROR_CODE_READ_WRITE;
        }
    }
    return rv;
}

int ProductData::WriteProductData(QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    if (GetMainAppCrystalT2()) {
        QString NameAndPathProductFile = GetMainAppCrystalT2()->GetPathNameProducts() + QString("/") + m_ProductName + QString(".dat");
        QFile File(NameAndPathProductFile);

        if (File.open(QFile::ReadWrite)) {
            QDataStream out(&File);

            out << this;
            File.close();
        } else {
            ErrorMsg = tr("Can Not Open File(Write Product Data) %1").arg(NameAndPathProductFile);
            rv = ERROR_CODE_READ_WRITE;
        }
    }
    return rv;
}
