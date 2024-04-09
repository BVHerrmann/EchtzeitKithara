#pragma once

#include <QObject>
#include "qrect.h"
#include "qhash.h"
#include "ValveData.h"


class MainAppCrystalT2;
class ProductData : public QObject
{
	Q_OBJECT
public:
	ProductData(MainAppCrystalT2 *parent = Q_NULLPTR);
	~ProductData();
	
	double GetBottleNeckDiameter() { return m_BottleNeckDiameter; }
	void   SetBottleNeckDiameter(double set) { m_BottleNeckDiameter = set; }

	double GetBotteleNeckDiameterToleranceInmm() { return m_BotteleNeckDiameterToleranceInmm; }
	void   SetBotteleNeckDiameterToleranceInmm(double set) { m_BotteleNeckDiameterToleranceInmm = set; }
		
	double  GetProductWidth() { return m_ProductWidth; }
	void    SetProductWidth(double set) { m_ProductWidth=set; }

	double  GetBottleNeckInnerDiameter() { return m_BottleNeckInnerDiameter; }
	void    SetBottleNeckInnerDiameter(double set) { m_BottleNeckInnerDiameter=set; }

	int ReadProductData(QString &ErrorMsg);
	int WriteProductData(QString &ErrorMsg);

	MainAppCrystalT2 *GetMainAppCrystalT2() { return m_MainAppCrystalT2; }

	QHash<int, QRect>  *GetMeasureWindowRects() { return &m_MeasureWindowRects; }
	QRect               GetMeasureWindowRect(int Key) { return m_MeasureWindowRects.value(Key); }

	QHash<int, ValveData>   *GetListValveData() { return &m_ListValveData; }
	ValveData                GetValveData(int Key) { return m_ListValveData.value(Key); }
	bool                     ContainsValveKey(int Key) {return m_ListValveData.contains(Key);}
	
	QString              GetProductName() { return m_ProductName; }
	void                 SetProductName(const QString &set) { m_ProductName=set; }

	int                  GetInjectionPositionInPixel() {return m_InjectionPositionInPixel;}
	int                  GetMeasurePositionInPixel()   {return m_MeasurePositionInPixel;}

	void                 SetInjectionPositionInPixel(int set) { m_InjectionPositionInPixel=set; }
	void                 SetMeasurePositionInPixel(int set)   { m_MeasurePositionInPixel=set; }
	 
private:
	MainAppCrystalT2 *m_MainAppCrystalT2;

public:
	double               m_ProductWidth;
	double               m_BottleNeckDiameter;
	double               m_BottleNeckInnerDiameter;
	double               m_BotteleNeckDiameterToleranceInmm;//big tol. value
	double               m_TriggerOffsetFirstValveInmm;
	double               m_TriggerOffsetSecondValveInmm;
	double               m_DefaultPreasureInBar;
	double               m_DefaultPreasureTankTemp;
    double               m_DefaulHeatingPipeTemp;
	double               m_DistanceInjectorBottle;
	double               m_InjectionAngle;
	double               m_DistanceBetweenMeasurePosAndTriggerPosInPixel;
	double               m_MinAcceptanceThresholdLiquidMiddleROI;
    double               m_MaxAcceptanceThresholdLiquidMiddleROI;
	double               m_AcceptanceThresholdLiquidLeftAndRightROI;
	double               m_FormatFromISInmm;
	double               m_InjectionMiddleWindowWidthInMm;
	double               m_ThresholdDegreeOfPollution;
	double               m_LowThresholdDegreeOfPollution;
	double               m_VideoPositionLeftValveInmm;//pos 0 is trigger pos
    double               m_VideoPositionRightValveInmm;//pos 0 is trigger pos

    double m_ValueAirCoolingCameraLitersPerMinute;
    double m_ValueAirCoolingLightLitersPerMinute;
    double m_ValueAirCoolingValveLitersPerMinute;

    double m_ValueWaterCoolingDefaultLitersPerMinute;
    double m_ValueWaterCoolingCircuitDefaultSensor;
    double m_ValveWaterCoolingDefaultTemperature;

	int                  m_InjectionPositionInPixel;
	int                  m_MeasurePositionInPixel;
	int                  m_UsedTriggerOutputs;
	QHash<int, QRect>    m_MeasureWindowRects;
	QString              m_ProductName;
	QHash<int, ValveData>    m_ListValveData;


	ProductData& operator=(const ProductData& other)
	{
		if (this != &other)
		{
		    m_BottleNeckDiameter                            = other.m_BottleNeckDiameter;
			m_BotteleNeckDiameterToleranceInmm              = other.m_BotteleNeckDiameterToleranceInmm;
			m_BottleNeckInnerDiameter                       = other.m_BottleNeckInnerDiameter;
			m_TriggerOffsetFirstValveInmm                   = other.m_TriggerOffsetFirstValveInmm;
			m_TriggerOffsetSecondValveInmm                  = other.m_TriggerOffsetSecondValveInmm;
			m_InjectionPositionInPixel                      = other.m_InjectionPositionInPixel;
			m_MeasurePositionInPixel                        = other.m_MeasurePositionInPixel;
			m_ProductWidth                                  = other.m_ProductWidth;
			m_DefaultPreasureInBar                          = other.m_DefaultPreasureInBar;
			m_DefaultPreasureTankTemp                       = other.m_DefaultPreasureTankTemp;
            m_DefaulHeatingPipeTemp                         = other.m_DefaulHeatingPipeTemp;
			m_DistanceInjectorBottle                        = other.m_DistanceInjectorBottle;
			m_InjectionAngle                                = other.m_InjectionAngle;
			m_VideoPositionLeftValveInmm                    = other.m_VideoPositionLeftValveInmm;
            m_VideoPositionRightValveInmm                   = other.m_VideoPositionRightValveInmm;
			m_DistanceBetweenMeasurePosAndTriggerPosInPixel = other.m_DistanceBetweenMeasurePosAndTriggerPosInPixel;
			m_MinAcceptanceThresholdLiquidMiddleROI         = other.m_MinAcceptanceThresholdLiquidMiddleROI;
            m_MaxAcceptanceThresholdLiquidMiddleROI         = other.m_MaxAcceptanceThresholdLiquidMiddleROI;
			m_AcceptanceThresholdLiquidLeftAndRightROI      = other.m_AcceptanceThresholdLiquidLeftAndRightROI;
			m_FormatFromISInmm                              = other.m_FormatFromISInmm;
			m_InjectionMiddleWindowWidthInMm                = other.m_InjectionMiddleWindowWidthInMm;
			m_ThresholdDegreeOfPollution                    = other.m_ThresholdDegreeOfPollution;
			m_LowThresholdDegreeOfPollution                 = other.m_LowThresholdDegreeOfPollution;
            m_ValueAirCoolingCameraLitersPerMinute          = other.m_ValueAirCoolingCameraLitersPerMinute;
            m_ValueAirCoolingLightLitersPerMinute           = other.m_ValueAirCoolingLightLitersPerMinute;
            m_ValueAirCoolingValveLitersPerMinute           = other.m_ValueAirCoolingValveLitersPerMinute;
            m_ValueWaterCoolingDefaultLitersPerMinute       = other.m_ValueWaterCoolingDefaultLitersPerMinute;
            m_ValueWaterCoolingCircuitDefaultSensor         = other.m_ValueWaterCoolingCircuitDefaultSensor;
            m_ValveWaterCoolingDefaultTemperature           = other.m_ValveWaterCoolingDefaultTemperature;
			QHashIterator<int, QRect> i(other.m_MeasureWindowRects);
			while (i.hasNext())
			{
				i.next();
				m_MeasureWindowRects.insert(i.key(), i.value());
			}
			QHashIterator<int, ValveData> j(other.m_ListValveData);
			while (j.hasNext())
			{
				j.next();
				m_ListValveData.insert(j.key(), j.value());
			}
		}
		return *this;
	}
};

