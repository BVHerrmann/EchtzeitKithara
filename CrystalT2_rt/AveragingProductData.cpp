#include "AveragingProductData.h"

AveragingProductData::AveragingProductData()
{
    m_TargeBottleneckDiameterInMM = 0.0;
    m_MaxEntriesPerProduct = 300;  // Maxmal mögliche Anzahle der gemessen Produkte im grünen Messfenster
    m_MaxNumberProducts = 5;
}

AveragingProductData::~AveragingProductData()
{
    ClearProductData();
    ClearSpeedData();
    ClearNeckDiameterData();
    ClearPixelSizeData();
}

// Wird aufgerufen wenn Produkt im grünen Messfenster gefunden, übergeben wird der vorgegebene Durchmesser und der Paramter wieviele Producte gespeichert werdrden
bool AveragingProductData::AddNewProductData(MeasuringResults Element, double TargeBottleneckDiameterInMM, int MaxNumberProducts)
{
    bool full = false;

    m_MaxNumberProducts = MaxNumberProducts;
    m_TargeBottleneckDiameterInMM = TargeBottleneckDiameterInMM;
    m_ListMeasuredProductData.push_back(Element);
    if (m_ListMeasuredProductData.size() > m_MaxEntriesPerProduct) {  // Absicherung damit beim stillstand des Bandes der Speicher nicht anwaechst, Wird nur überschritten wenn Messfenster sehr lang
                                                                      // oder das Band steht und eine Flasche ist im Sichtfeld
        m_ListMeasuredProductData.pop_front();
        full = true;
    }
    return full;
}

void AveragingProductData::RemoveFirstElement()
{
    if (m_ListMeasuredProductData.size() > 0) {
        m_ListMeasuredProductData.erase(m_ListMeasuredProductData.begin());
    }
}

void AveragingProductData::RemoveLastElement()
{
    if (m_ListMeasuredProductData.size() > 2) {
        std::list<MeasuringResults>::iterator it;
        it = prev(m_ListMeasuredProductData.end());
        m_ListMeasuredProductData.erase(it);
    }
}

void AveragingProductData::ClearProductData()
{
    while (!m_ListMeasuredProductData.empty()) m_ListMeasuredProductData.pop_front();
}

void AveragingProductData::ClearSpeedData()
{
    while (!m_ListSpeedValues.empty()) m_ListSpeedValues.pop_front();
}

void AveragingProductData::ClearNeckDiameterData()
{
    while (!m_ListMeasuredBottleNeckDiameterInmm.empty()) m_ListMeasuredBottleNeckDiameterInmm.pop_front();
}

void AveragingProductData::ClearPixelSizeData()
{
    while (!m_ListMeasuredPixelSize.empty()) m_ListMeasuredPixelSize.pop_front();
}

int AveragingProductData::GetNumberResultsPerProducts()
{
    return static_cast<int>(m_ListMeasuredProductData.size());
}

void AveragingProductData::AverageSpeed(double& Speed)
{
    int i = 0;
    int k = 0;
    double DeltaT, DeltaS;
    double PostitionInmm[2];
    unsigned __int64 TimeStampInNs[2];
    unsigned __int64 Int64DeltaT, n;

    Speed = 0.0;
    DeltaT = DeltaS = 0.0;
    Int64DeltaT = 0;
    n = m_ListMeasuredProductData.size() - 1;
    if (n > 0) {  // mitteln ueber ein Product
        for (std::list<MeasuringResults>::iterator it = m_ListMeasuredProductData.begin(); it != m_ListMeasuredProductData.end(); ++it) {
            PostitionInmm[i] = (*it).m_BottleMiddelPositionInmm;
            TimeStampInNs[i] = (*it).m_CurrentTimeStampInns;  // in nano seconds
            i++;
            if (i == 2) {
                Int64DeltaT = Int64DeltaT + abs(static_cast<int>(TimeStampInNs[0] - TimeStampInNs[1]));
                DeltaS = DeltaS + fabs(PostitionInmm[0] - PostitionInmm[1]);
                i = 0;
            }
        }
        DeltaT = static_cast<double>(Int64DeltaT) / ((double)(1000000));  // DeltaT in ms, timestamp comes from Kithara
        DeltaT = DeltaT / n;
        DeltaS = DeltaS / n;
        if (DeltaT != 0.0 && DeltaS != 0.0)
            Speed = DeltaS / DeltaT;
        else
            Speed = 0.0;
        m_ListSpeedValues.push_back(Speed);
    }
    // mitteln uber n Produkte
    if (m_ListSpeedValues.size() > 1)   // m_MaxNumberProducts)//Hier die Geschwindigkeit nur über ein Prduckt
        m_ListSpeedValues.pop_front();  // remove first
    if (m_ListSpeedValues.size() > 0) {
        double sum = 0.0;
        for (std::list<double>::iterator it = m_ListSpeedValues.begin(); it != m_ListSpeedValues.end(); ++it) sum = sum + (*it);
        Speed = sum / m_ListSpeedValues.size();
    }
}

void AveragingProductData::AverageBottleNeckDiameterInmm(double& BottleneckDiameterInmm)
{
    double sum = 0.0;

    if ((m_ListMeasuredProductData.size() - 1) > 0) {
        for (std::list<MeasuringResults>::iterator it = m_ListMeasuredProductData.begin(); it != m_ListMeasuredProductData.end(); ++it) {
            sum = sum + (*it).m_MeasuredBottleNeckDiameterInmm;
        }
        BottleneckDiameterInmm = sum / m_ListMeasuredProductData.size();
        m_ListMeasuredBottleNeckDiameterInmm.push_back(BottleneckDiameterInmm);
    }
    if (m_ListMeasuredBottleNeckDiameterInmm.size() > m_MaxNumberProducts) m_ListMeasuredBottleNeckDiameterInmm.pop_front();  // remove first
    if (m_ListMeasuredBottleNeckDiameterInmm.size() > 0) {
        sum = 0.0;
        for (std::list<double>::iterator it = m_ListMeasuredBottleNeckDiameterInmm.begin(); it != m_ListMeasuredBottleNeckDiameterInmm.end(); ++it) sum = sum + (*it);
        BottleneckDiameterInmm = sum / m_ListMeasuredBottleNeckDiameterInmm.size();
    }
}

void AveragingProductData::AveragePixelSize(double& PixelSizeInMMPerPixel)
{
    double MeasuredBottelneckWidthInPixel, sum;

    MeasuredBottelneckWidthInPixel = 0.0;
    if ((m_ListMeasuredProductData.size() - 1) > 0) {
        for (std::list<MeasuringResults>::iterator it = m_ListMeasuredProductData.begin(); it != m_ListMeasuredProductData.end(); ++it) {
            MeasuredBottelneckWidthInPixel = MeasuredBottelneckWidthInPixel + fabs((*it).m_ResultEdgeLeftXPos - (*it).m_ResultEdgeRightXPos);
        }
        MeasuredBottelneckWidthInPixel = MeasuredBottelneckWidthInPixel / m_ListMeasuredProductData.size();
        if (MeasuredBottelneckWidthInPixel > 0.0) {
            PixelSizeInMMPerPixel = m_TargeBottleneckDiameterInMM / MeasuredBottelneckWidthInPixel;
            m_ListMeasuredPixelSize.push_back(PixelSizeInMMPerPixel);
        }
    }
    if (m_ListMeasuredPixelSize.size() > m_MaxNumberProducts) m_ListMeasuredPixelSize.pop_front();  // remove first
    if (m_ListMeasuredPixelSize.size() > 0) {
        sum = 0.0;
        for (std::list<double>::iterator it = m_ListMeasuredPixelSize.begin(); it != m_ListMeasuredPixelSize.end(); ++it) sum = sum + (*it);
        PixelSizeInMMPerPixel = sum / m_ListMeasuredPixelSize.size();
    }
}

MeasuringResults AveragingProductData::AverageProductData()
{
    int k = 0;

    m_AveragedMeasureResults.ClearResults();
    m_AveragedMeasureResults.m_EdgeLeftFound = true;
    m_AveragedMeasureResults.m_EdgeRightFound = true;
    m_AveragedMeasureResults.m_ProductSizeInTolerance = true;
    m_AveragedMeasureResults.m_ProductContrastOk = true;

    m_AveragedMeasureResults.m_CurrentTimeStampInns = 0;
    m_AveragedMeasureResults.m_BottleMiddelPositionInmm = 0.0;
    m_AveragedMeasureResults.m_BottleContrastInPercent = 0.0;
    m_AveragedMeasureResults.m_BottleMatchScoreInPercent = 0.0;
    m_AveragedMeasureResults.m_EdgeLeftContrastInPercent = 0.0;
    m_AveragedMeasureResults.m_EdgeRightContrastInPercent = 0.0;
    m_AveragedMeasureResults.m_MeasuredBottleNeckDiameterInmm = 0.0;
    m_AveragedMeasureResults.m_ResultEdgeLeftXPos = 0.0;
    m_AveragedMeasureResults.m_ResultEdgeRightXPos = 0.0;
    m_AveragedMeasureResults.m_MeasuringTimeInms = 0.0;

    if (m_ListMeasuredProductData.size() > 0) {
        for (std::list<MeasuringResults>::iterator it = m_ListMeasuredProductData.begin(); it != m_ListMeasuredProductData.end(); ++it) {
            m_AveragedMeasureResults.m_CurrentTimeStampInms = m_AveragedMeasureResults.m_CurrentTimeStampInms + (*it).m_CurrentTimeStampInns / ((double)(1000000));
            m_AveragedMeasureResults.m_BottleMiddelPositionInmm = m_AveragedMeasureResults.m_BottleMiddelPositionInmm + (*it).m_BottleMiddelPositionInmm;
            m_AveragedMeasureResults.m_BottleContrastInPercent = m_AveragedMeasureResults.m_BottleContrastInPercent + (*it).m_BottleContrastInPercent;
            m_AveragedMeasureResults.m_BottleMatchScoreInPercent = m_AveragedMeasureResults.m_BottleMatchScoreInPercent + (*it).m_BottleMatchScoreInPercent;
            m_AveragedMeasureResults.m_EdgeLeftContrastInPercent = m_AveragedMeasureResults.m_EdgeLeftContrastInPercent + (*it).m_EdgeLeftContrastInPercent;
            m_AveragedMeasureResults.m_EdgeRightContrastInPercent = m_AveragedMeasureResults.m_EdgeRightContrastInPercent + (*it).m_EdgeRightContrastInPercent;
            m_AveragedMeasureResults.m_MeasuredBottleNeckDiameterInmm = m_AveragedMeasureResults.m_MeasuredBottleNeckDiameterInmm + (*it).m_MeasuredBottleNeckDiameterInmm;
            m_AveragedMeasureResults.m_ResultEdgeLeftXPos = m_AveragedMeasureResults.m_ResultEdgeLeftXPos + (*it).m_ResultEdgeLeftXPos;
            m_AveragedMeasureResults.m_ResultEdgeRightXPos = m_AveragedMeasureResults.m_ResultEdgeRightXPos + (*it).m_ResultEdgeRightXPos;
            m_AveragedMeasureResults.m_MeasuringTimeInms = m_AveragedMeasureResults.m_MeasuringTimeInms + (*it).m_MeasuringTimeInms;
        }
        m_AveragedMeasureResults.m_CurrentTimeStampInms = m_AveragedMeasureResults.m_CurrentTimeStampInms / m_ListMeasuredProductData.size();
        m_AveragedMeasureResults.m_BottleMiddelPositionInmm = m_AveragedMeasureResults.m_BottleMiddelPositionInmm / m_ListMeasuredProductData.size();
        m_AveragedMeasureResults.m_BottleContrastInPercent = m_AveragedMeasureResults.m_BottleContrastInPercent / m_ListMeasuredProductData.size();
        m_AveragedMeasureResults.m_BottleMatchScoreInPercent = m_AveragedMeasureResults.m_BottleMatchScoreInPercent / m_ListMeasuredProductData.size();
        m_AveragedMeasureResults.m_EdgeLeftContrastInPercent = m_AveragedMeasureResults.m_EdgeLeftContrastInPercent / m_ListMeasuredProductData.size();
        m_AveragedMeasureResults.m_EdgeRightContrastInPercent = m_AveragedMeasureResults.m_EdgeRightContrastInPercent / m_ListMeasuredProductData.size();
        m_AveragedMeasureResults.m_MeasuredBottleNeckDiameterInmm = m_AveragedMeasureResults.m_MeasuredBottleNeckDiameterInmm / m_ListMeasuredProductData.size();
        m_AveragedMeasureResults.m_ResultEdgeLeftXPos = m_AveragedMeasureResults.m_ResultEdgeLeftXPos / m_ListMeasuredProductData.size();
        m_AveragedMeasureResults.m_ResultEdgeRightXPos = m_AveragedMeasureResults.m_ResultEdgeRightXPos / m_ListMeasuredProductData.size();
        m_AveragedMeasureResults.m_MeasuringTimeInms = m_AveragedMeasureResults.m_MeasuringTimeInms / m_ListMeasuredProductData.size();
    }
    return m_AveragedMeasureResults;
}
