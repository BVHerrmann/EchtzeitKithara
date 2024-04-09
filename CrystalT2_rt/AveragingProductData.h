#pragma once
#include <list> 
#include "SharedData.h"


class AveragingProductData
{
public:
	AveragingProductData();
	~AveragingProductData();
	bool AddNewProductData(MeasuringResults Element,double TargeBottleneckDiameterInMM,int MaxNumberProducts=5);
	void ClearProductData();
	void ClearSpeedData();
	void ClearNeckDiameterData();
	void ClearPixelSizeData();
	void AverageSpeed(double &Speed);
	void AveragePixelSize(double &PixelSize);
	void AverageBottleNeckDiameterInmm(double &BottleneckDiameterInmm);
	MeasuringResults AverageProductData();
	MeasuringResults GetAveragedMeasureResults() {return m_AveragedMeasureResults;}
	int GetNumberResultsPerProducts();
	void SetMaxNumberProducts(int set) { m_MaxNumberProducts = set; }
    void RemoveFirstElement();
    void RemoveLastElement();
	
private:
	std::list< MeasuringResults > m_ListMeasuredProductData;
	std::list< double > m_ListSpeedValues;
	std::list< double > m_ListMeasuredBottleNeckDiameterInmm;
	std::list< double > m_ListMeasuredPixelSize;
	double m_TargeBottleneckDiameterInMM;
	int    m_MaxEntriesPerProduct;
	int    m_MaxNumberProducts;
	MeasuringResults m_AveragedMeasureResults;
};

