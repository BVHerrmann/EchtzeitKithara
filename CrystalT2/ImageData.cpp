#include "ImageData.h"
#include "AdminSettingsDialog.h"
#include "EditProductDialog.h"
#include "GlobalConst.h"
#include "KitharaCore.h"
#include "LiveImageView.h"
#include "MainAppCrystalT2.h"
#include "ProductData.h"
#include "SettingsData.h"
#include "VideoHeader.h"
#include "bmessagebox.h"
#include "qimagereader.h"
#include "qimagewriter.h"

ImageData::ImageData(MainAppCrystalT2* pMainAppCrystalT2)
    : QThread(),
      m_MainAppCrystalT2(NULL),
      m_RealTimeCameraInitialised(false),
      m_TerminateLiveImageView(false),
      m_NumberFramesPerInterval(0),
      m_ImageDataID(1),
      m_DisplayZoomFactor(1.0),
      m_ImageWidth(USED_CAMERA_WIDTH),
      m_ImageHeight(USED_CAMERA_HEIGHT),
      m_ImageOffsetX(0),
      m_ImageOffsetY(0),
      m_ImagePitch(USED_CAMERA_WIDTH),
      m_EnableAddNewImageIntoSharedMemory(true),
      m_ImageCounter(0),
      m_ImageFileCounter(1),
      m_MeasuredSpeedInMMPerms(0.0),
      m_SaveCleanImage(false),
      m_CheckCleanImage(false),
      m_LastCameraImageCounter(0),
      m_AutoCalibrateIsOn(false),
      m_AutoCalibrateBottleTopLineFound(false),
      m_MaxEdgeContrastAutoCalibrate(0.0),
      m_CurrentStepAutoCalibrate(STEP_ONE_AUTO_CALIBRATE_SEARCH_BOTTLE_TOP_LINE),
      m_CounterAutoCalibrate(0),
      m_LastEdgeThreshold(0.0),
      m_DoNotChangeBlueWindow(false),
      m_InfoLevel(0)
{
    m_MainAppCrystalT2 = pMainAppCrystalT2;
    QString PathAndName = GetMainAppCrystalT2()->GetPathCleanImageLocation() + QString("/CleanImage.bmp");
    QFileInfo FileInfo(PathAndName);

    if (QFileInfo::exists(PathAndName)) {
        QString Time = FileInfo.lastModified().time().toString("hh:mm:ss");
        QString Date = FileInfo.lastModified().date().toString("dd.MM.yyyy  ");
        m_DateTimeCleanImage = Date + Time;
        m_CleanImageQt = QImage(PathAndName, "bmp");
    }
    connect(this, &ImageData::SignalSetCalibratetMeasureWindowSpeed, this, &ImageData::SlotSetCalibratetMeasureWindowSpeed, Qt::QueuedConnection);
    connect(this, &ImageData::SignalShowMessageBoxResultsCalibration, this, &ImageData::SlotShowMessageBoxResultsCalibration, Qt::QueuedConnection);
    bool rv = connect(this, &ImageData::SignalDrawAutocalibrateROI, this, &ImageData::SlotDrawAutocalibrateROI, Qt::QueuedConnection);
}

ImageData::~ImageData()
{
    FinishedThreadAndFreeAllAllocations();
    SaveProductData();
}

void ImageData::run()
{
    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
    if (pSettingsData) {
        int rv = ERROR_CODE_NO_ERROR;
        int ScaledWith, ScaledHeight;
        // int WaitTimeShowNextVideoFrameInms = 100;
        int ImageCounter = 0;
        double Area, DegreeOfPollution;
        bool CleanImageIsSet = true;
        unsigned char* pSharedMemoryImageData = NULL;
        unsigned char* pCleanImage = NULL;
        MeasuringResults* pMeasuringResults;
        QString ErrorMsg, PathAndName;
        ImageMetaData LiveImage;
        ImageHeader imageHeader;
        cv::Moments moments;
        QImage QtResultImage;
        unsigned __int64 currentCameraImageCounter = 0;
        int counterAutoCalibrate = 0;

        while (!m_TerminateLiveImageView) {
            if (GetKitharaCore()) {
                ScaledWith = GetImageWidth();
                ScaledHeight = GetImageHeight();
                CalculateDisplayZoomFactor(ScaledWith, ScaledHeight);
                rv = GetKitharaCore()->WaitForNextImage(pSettingsData->m_CameraLiveImageTimeoutInms, currentCameraImageCounter, ErrorMsg);
                if (rv == ERROR_CODE_NO_ERROR) {
                    m_LastCameraImageCounter = currentCameraImageCounter;
                    if (GetKitharaCore()->GetRawImagedata()) {
                        pSharedMemoryImageData = GetKitharaCore()->GetRawImagedata();
                        LiveImage.m_Image = QImage(pSharedMemoryImageData, GetImageWidth(), GetImageHeight(), QImage::Format_Grayscale8).scaled(ScaledWith, ScaledHeight);
                        if (GetKitharaCore()->GetCurrentMeasuringResults() && GetKitharaCore()->GetCurrentMeasuringResultsLiqiud()) {
                            LiveImage.m_CurrentMeasuringResult = *(GetKitharaCore()->GetCurrentMeasuringResults());            // kopiere aktuelles ergebnis
                            LiveImage.m_AveragedMeasuringResults = *(GetKitharaCore()->GetCurrentAveragedMeasuringResults());  // kopiere laufend gemittelte ergebnisse
                            LiveImage.m_CurrentMeasuringResultsLiqiud = *(GetKitharaCore()->GetCurrentMeasuringResultsLiqiud());
                            // wird im Realtime Kontext auf true gesetzt wenn neue ergebnisse vorliegen, in der Windowsanwendung werden nur werte angezeigt wenn status auf true steht
                            GetKitharaCore()->GetCurrentMeasuringResults()->m_StatusResults = false;
                            GetKitharaCore()->GetCurrentMeasuringResultsLiqiud()->m_StatusResults = false;
                            GetKitharaCore()->GetCurrentAveragedMeasuringResults()->m_StatusResults = false;
                            if (m_AutoCalibrateIsOn) {
                                AutoCalibrateSystem(LiveImage);
                            }
                        }
                        if (GetKitharaCore()->GetRawCleanImagedata()) {
                            if (m_SaveCleanImage) {
                                m_CleanImageQt = LiveImage.m_Image.copy();
                                PathAndName = GetMainAppCrystalT2()->GetPathCleanImageLocation() + QString("/CleanImage.bmp");
                                m_CleanImageQt.save(PathAndName);  // neues Referenzbild Speichern
                                emit SignalShowCleanImage(m_CleanImageQt);
                                QString Text = QDateTime::currentDateTime().date().toString("dd.MM.yyyy  ") + QDateTime::currentDateTime().time().toString("hh:mm:ss");
                                emit SignalSetDateTimeCleanImageIsSaved(Text);
                                m_SaveCleanImage = false;
                                emit SignalShowDegreeOfPollution(0.0);
                            } else {
                                if (m_CheckCleanImage && !m_CleanImageQt.isNull()) {
                                    // Adresse des Bildes die Aufnahme wurde direkt nach dem Befüllen gemacht, wenn die Flasche unter dem Ventil durchgelaufen ist
                                    pCleanImage = GetKitharaCore()->GetRawCleanImagedata();
                                    QRect CleanImageRect = GetMeasureWindowCheckCleanImage();  // GetMeasureWindowRect(ROI_ID_MEASURE_LIQUID);//ROI des blauen Messfensters
                                    QImage RefImage = m_CleanImageQt.copy(CleanImageRect);     // Referenzmessfenster auschneiden
                                    QImage CurrentImage = QImage(pCleanImage + sizeof(ImageHeader), GetImageWidth(), GetImageHeight(), QImage::Format_Grayscale8).scaled(ScaledWith, ScaledHeight);
                                    QImage CurrentROIImage = CurrentImage.copy(CleanImageRect);  // Aktuelles Bild, entspricht dem blauen Messfenster
                                    cv::Mat ResultImage;
                                    // QtBilder nach openCV kopieren
                                    cv::Mat RefImageOpenCV(RefImage.height(), RefImage.width(), CV_8U, (void*)RefImage.constBits(), RefImage.bytesPerLine());  // lese Referenz ROI in ein OpenCV Bild
                                    cv::Mat CurrentImageOpenCV(CurrentROIImage.height(), CurrentROIImage.width(), CV_8U, (void*)CurrentROIImage.constBits(),
                                                               CurrentROIImage.bytesPerLine());    // lese Aktuellen ROI in ein OpenCV Bild
                                    cv::absdiff(CurrentImageOpenCV, RefImageOpenCV, ResultImage);  // Vergleich zwischen Referenz und aktuellem Bild (Differenzbild)
                                    ResultImage = cv::Scalar::all(255) - ResultImage;
                                    cv::threshold(ResultImage, ResultImage, pSettingsData->m_ThresholdBinaryImageDegreeOfPollution, 255.0, cv::THRESH_BINARY_INV);  // Differenzbild biniariesieren
                                    // cv::erode(ResultImage, ResultImage, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));
                                    if (GetKitharaCore()->GetInfoLevel() == INFO_LEVEL_SHOW_BIN_CLEAN_IMAGE)  // INFO_LEVEL_SHOW_BIN_IMAGE_DEGREE_OF_POLLUTION)
                                    {                                                                         // Anzege des biniarisierten Bildes nur für testzwecke
                                        QPoint destPos = QPoint(CleanImageRect.x(), CleanImageRect.y());
                                        QtResultImage = QImage(ResultImage.data, ResultImage.cols, ResultImage.rows, static_cast<int>(ResultImage.step), QImage::Format_Grayscale8).copy();
                                        QPainter painter(&CurrentImage);
                                        painter.drawImage(destPos, QtResultImage);
                                        painter.end();
                                        emit SignalShowCleanImage(CurrentImage);
                                        CleanImageIsSet = false;
                                    } else {  // Zeige wieder normales Bild wenn vorher das Binarisierte gezeigt wurde
                                        if (!CleanImageIsSet) {
                                            emit SignalShowCleanImage(m_CleanImageQt);
                                            CleanImageIsSet = true;
                                        }
                                    }
                                    Area = CleanImageRect.width() * CleanImageRect.height();
                                    if (Area > 0.0) {
                                        moments = cv::moments(ResultImage, true);
                                        DegreeOfPollution = (moments.m00 / Area) * 100.0;     // Fläche der Abweichung in Prozent
                                        emit SignalShowDegreeOfPollution(DegreeOfPollution);  // show value on GUI
                                    }
                                    m_CheckCleanImage = false;
                                }
                            }
                        }  // if (GetKitharaCore()->GetRawCleanImagedata())
                    } else {
                        ErrorMsg = tr("No Camera Pixel Data In Image Buffer");
                        emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
                        rv = ERROR_CODE_ANY_ERROR;
                    }
                } else {  // timeout
                    rv = ERROR_CODE_ANY_ERROR;
                }
            }
            if (rv == ERROR_CODE_NO_ERROR)
                emit SignalShowLiveImage(LiveImage);
            else {  // hier timeout
                if (m_LastCameraImageCounter ==
                    currentCameraImageCounter) {  // Wenn Kamerazähler immer gleich dann werden keine Bilde Aufgenommen, Kamera ist tot oder die Bileinzugstask ist nicht mehr aktiv
                    ErrorMsg = tr("Timeout! Camera Task Is Not Working(Count:%1 Last Count:%2)").arg(currentCameraImageCounter).arg(m_LastCameraImageCounter);
                } else {  // Hier Kamera läuft, aber die Mssung funktioniert nicht mehr, die Messtask ist wahrscheinlich abgestürzt
                    ErrorMsg = tr("Timeout! Measure Task Is Not Working(Count:%1 Last Count:%2)").arg(currentCameraImageCounter).arg(m_LastCameraImageCounter);
                }
                emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
            }
        }
        m_WaitLiveImageViewIsDisable.lock();
        m_WaitConditionLiveViewIsDisable.wakeAll();  // signal an gui dass livebild anzeige gestoppt ist
        m_WaitLiveImageViewIsDisable.unlock();
    }
}

QRect ImageData::GetMeasureWindowCheckCleanImage()
{
    int x, y, w, h;
    QRect LiquidRect = GetMeasureWindowRect(ROI_ID_MEASURE_LIQUID);
    QRect RectSpeed = GetMeasureWindowRect(ROI_ID_MEASURE_SPEED);
    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();

    w = 100;
    if (pSettingsData && pSettingsData->m_PixelSize > 0.0) w = static_cast<int>(pSettingsData->m_DistancesBetweenValves / pSettingsData->m_PixelSize);
    x = RectSpeed.x() + RectSpeed.width() / 2 - w / 2.0;
    y = LiquidRect.y();
    h = LiquidRect.height();
    QRect rect(x, y, w, h);
    CheckIsMeasureWindowInRange(rect, ROI_ID_CHECK_CLEAN_IMAGE);
    return rect;
}

void ImageData::SetAutoCalibrateIsOn(bool set, bool AbortByUser)
{
    m_AutoCalibrateIsOn = set;
    if (!set) {
        m_CurrentStepAutoCalibrate = STEP_ONE_AUTO_CALIBRATE_SEARCH_BOTTLE_TOP_LINE;
        if (GetKitharaCore()) GetKitharaCore()->SetBotteleNeckDiameterToleranceInmm(m_BottelNeckDiameterTolBeforeCalibrating);  // Alten wert wieder zurück wenn Kalibrierung beendet oder abgebrochen
        if (AbortByUser) {
            SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
            emit SignalSetCalibratetMeasureWindowSpeed(m_MeasureWindowSpeedBeforeCalibrating);  // alte messfensterposition zurück
            if (pSettingsData) {
                pSettingsData->m_EdgeAcceptanceThresholdInPercent = m_LastEdgeThreshold;  // alten Schwellwert zurück
                GetMainAppCrystalT2()->SaveSettings();
                SetEdgeThreshold(pSettingsData->m_EdgeAcceptanceThresholdInPercent);
            }
        }
        QRect RemoveAutoCalibrateROI(0, 0, 0, 0);
        emit SignalDrawAutocalibrateROI(RemoveAutoCalibrateROI);
    }
}

void ImageData::AutoCalibrateSystem(ImageMetaData& LiveImage)
{
    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();

    if (pSettingsData) {
        double FactorCounterAutoCalibrate =
            100.0 / (pSettingsData->m_NumIterationCalculteROIPosition * 2 + pSettingsData->m_NumIterationCalculateAccseptanceThreshold + pSettingsData->m_NumIterationCalculatePixelSize);
        switch (m_CurrentStepAutoCalibrate) {
            case STEP_ONE_AUTO_CALIBRATE_SEARCH_BOTTLE_TOP_LINE:
                // Erster Schritt suche solange bis Flaschenoberkante erkannt. Die Position der Oberkante der Flasche ist die Startposition des Grünen Messfensters
                m_CounterAutoCalibrate = 0;
                m_MaxEdgeContrastAutoCalibrate = 0.0;
                m_BottelNeckDiameterTolBeforeCalibrating = GetBotteleNeckDiameterToleranceInmm();    // Wert sichern wird am ende wieder zurückgesetzt, beim Kalibrieren wird ein anderer Wert genutzt
                m_MeasureWindowSpeedBeforeCalibrating = GetMeasureWindowRect(ROI_ID_MEASURE_SPEED);  // Alten Wert Sichern wenn Kalibrierung abgebrochen
                m_LastEdgeThreshold = pSettingsData->m_EdgeAcceptanceThresholdInPercent;             // Alten Wert Sichern wenn Kalibrierung abgebrochen
                m_ListCalibrateData1.clear();
                m_ListCalibrateData2.clear();
                emit SignalSetCalibrateStatus(tr("Step One: Search Bottle Top Line, Please Wait..."), m_CounterAutoCalibrate * FactorCounterAutoCalibrate);
                if (FindBottleTopLine(LiveImage)) m_CurrentStepAutoCalibrate = STEP_TWO_AUTO_CALIBRATE_SEARCH_OPTIMAL_ROI_POS;
                msleep(1000);
                break;
            case STEP_TWO_AUTO_CALIBRATE_SEARCH_OPTIMAL_ROI_POS:  // Suche nach der optimalen Y-Postion des Grünen Messfenster
                if (FindOptimalROIPosition(LiveImage)) m_CounterAutoCalibrate++;
                if (m_CounterAutoCalibrate >= pSettingsData->m_NumIterationCalculteROIPosition) {
                    if (m_ListCalibrateData1.count() > 0) {  // && m_ListCalibrateData2.count() > 0) {
                        m_CurrentStepAutoCalibrate = STEP_THRE_AUTO_CALIBRATE_SEARCH_OPTIMAL_ROI_HEIGHT;
                        // m_CurrentStepAutoCalibrate = STEP_FOUR_AUTO_CALIBRATE_SEARCH_OPTIMAL_EDGE_THRESHOLD;
                        QRect NewWindow = GetMeasureWindowRect(ROI_ID_MEASURE_SPEED);
                        // int w = static_cast<int>(GetKitharaCore()->GetImageWidth() * pSettingsData->m_FactorMeasureWindowWidthSearchBottleTopPosiition);//
                        int w = NewWindow.width();
                        // int h = static_cast<int>(m_ListCalibrateData2.back());
                        int h = NewWindow.height();
                        int x = NewWindow.x();
                        int y = static_cast<int>(m_ListCalibrateData1.back());
                        NewWindow.setX(x);
                        NewWindow.setY(y);
                        NewWindow.setWidth(w);
                        NewWindow.setHeight(h);
                        emit SignalSetCalibratetMeasureWindowSpeed(NewWindow);
                        m_ListCalibrateData1.clear();
                        m_ListCalibrateData2.clear();
                        m_MaxEdgeContrastAutoCalibrate = 0.0;
                        msleep(1000);
                    }
                } else
                    emit SignalSetCalibrateStatus(tr("Step Two: Search Optimal ROI Pos, Please Wait..."), m_CounterAutoCalibrate * FactorCounterAutoCalibrate);
                break;
            case STEP_THRE_AUTO_CALIBRATE_SEARCH_OPTIMAL_ROI_HEIGHT:  // Suche nach der optimalen Höhe des Grünen Messfenster
                if (FindOptimalROIPosition(LiveImage, false)) m_CounterAutoCalibrate++;
                if (m_CounterAutoCalibrate >= (pSettingsData->m_NumIterationCalculteROIPosition * 2)) {
                    if (m_ListCalibrateData1.count() > 0) {
                        m_CurrentStepAutoCalibrate = STEP_FOUR_AUTO_CALIBRATE_SEARCH_OPTIMAL_EDGE_THRESHOLD;
                        QRect NewWindow = GetMeasureWindowRect(ROI_ID_MEASURE_SPEED);
                        int w = NewWindow.width();
                        int h = static_cast<int>(m_ListCalibrateData1.back());
                        int x = NewWindow.x();
                        int y = NewWindow.y();
                        NewWindow.setX(x);
                        NewWindow.setY(y);
                        NewWindow.setWidth(w);
                        NewWindow.setHeight(h);
                        emit SignalSetCalibratetMeasureWindowSpeed(NewWindow);
                        m_ListCalibrateData1.clear();
                        m_MaxEdgeContrastAutoCalibrate = 0.0;
                        msleep(1000);
                    }
                } else
                    emit SignalSetCalibrateStatus(tr("Step Three: Search Optimal ROI Height, Please Wait..."), m_CounterAutoCalibrate * FactorCounterAutoCalibrate);
                break;
            case STEP_FOUR_AUTO_CALIBRATE_SEARCH_OPTIMAL_EDGE_THRESHOLD:  // Suche nach der optimalen Akzeptanzschwelle für die Bestimmung einer Kante
                if (FindOptimalEdgeThreshold(LiveImage)) m_CounterAutoCalibrate++;
                if (m_ListCalibrateData1.count() >= pSettingsData->m_NumIterationCalculateAccseptanceThreshold) {  // Finished
                    m_CurrentStepAutoCalibrate = STEP_FIVE_AUTO_CALIBRATE_SEARCH_OPTIMAL_PIXEL_SIZE;
                    m_CalculatetEdgeThreshold = GetAverageCalibrationData() * pSettingsData->m_FactorThreshold;
                    m_ListCalibrateData1.clear();
                    pSettingsData->m_EdgeAcceptanceThresholdInPercent = m_CalculatetEdgeThreshold;
                    GetMainAppCrystalT2()->SaveSettings();
                    SetEdgeThreshold(pSettingsData->m_EdgeAcceptanceThresholdInPercent);  // Neuen Schwellwert an die Realtimetask
                    if (GetKitharaCore()) {
                        double DiaTolInAutoCali = GetTargetBottleneckDiameter() * (pSettingsData->m_DiameterTolInPercentAutoCalibrate / 100.0);
                        GetKitharaCore()->SetBotteleNeckDiameterToleranceInmm(DiaTolInAutoCali);
                    }
                    msleep(1000);
                } else
                    emit SignalSetCalibrateStatus(tr("Step Four: Search Optimal Edge Threshold, Please Wait..."), m_CounterAutoCalibrate * FactorCounterAutoCalibrate);
                break;
            case STEP_FIVE_AUTO_CALIBRATE_SEARCH_OPTIMAL_PIXEL_SIZE:  // Einige Zeit abwarten bis sich eine neue Pixelsize eingestellt hat
                emit SignalSetCalibrateStatus(tr("Step Five: Search Optimal PixelSize, Please Wait..."), m_CounterAutoCalibrate * FactorCounterAutoCalibrate);
                if (LiveImage.m_AveragedMeasuringResults.EdgesFound())  // FindOptimalPixelSize(LiveImage))
                {                                                       // Hier nur einige Aufnahmen abwarten bis sich in der Realtimetask ein stabiler wert für die Pixelsize eingestellt hat
                    m_CounterAutoCalibrate++;
                    if (m_CounterAutoCalibrate * FactorCounterAutoCalibrate >= 100) {
                        m_CalculatetPixelSize = LiveImage.m_AveragedMeasuringResults.m_CalculatePixelSizeInMMPerPixel;  // Berechnete PixelSize aus der Realtimetask
                        if (m_CalculatetPixelSize < pSettingsData->m_MinPixelSizeAutocalibration) {
                            m_CalculatetPixelSize = pSettingsData->m_MinPixelSizeAutocalibration;
                            qDebug() << QString("Warning Pixel Size Is Smaller %1").arg(pSettingsData->m_MinPixelSizeAutocalibration);
                        }
                        emit SignalSetCalibrateStatus(QString(""), -1);  // Close Progressbar
                        SetAutoCalibrateIsOn(false, false);              // Finished calibration
                        emit SignalShowMessageBoxResultsCalibration();   // Bestätigung Kalbrierung zu Ende und Berechnung der anderen Messfensterpositionen
                        m_ListCalibrateData1.clear();
                    }
                }
                break;
            default:
                break;
        }
        QApplication::processEvents();
    }
}

void ImageData::SlotShowMessageBoxResultsCalibration()
{
    if (GetMainAppCrystalT2()) {
        SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
        ProductData* pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
        QString TextMessage = tr("Calculatet PixelSize:%1mm/pix  Apply Data?").arg(m_CalculatetPixelSize, 0, 'f', 5);
        BMessageBox* pMessageBox = new BMessageBox(QMessageBox::Information, tr("Calibration Ready"), TextMessage, GetMainAppCrystalT2()->GetEditProductDialog());

        pMessageBox->addButton(QMessageBox::Yes)->setText(tr("Yes"));
        pMessageBox->addButton(QMessageBox::No)->setText(tr("No"));
        if (pMessageBox->exec() != -1) {
            if (pSettingsData && pProductData && (pMessageBox->standardButton(pMessageBox->clickedButton()) == QMessageBox::Yes)) {
                QRect GreenRect = GetMeasureWindowRect(ROI_ID_MEASURE_SPEED);
                QRect BlueRect = GetMeasureWindowRect(ROI_ID_MEASURE_LIQUID);
                QRect YellowRect;
                QRect NewRect;
                double FactorWidthBlueWindow = 1.0 - pSettingsData->m_FactorMeasureWindowWidthSearchBottleTopPosiition;
                int MaxHeightBlueWindow = 70;
                int YSpaceBlueRectTop = 10;  // Platz damit die Ventildüse nicht ins Messfenster gelangt
                int YSpaceBlueRectBot = 20;  // Abstand zwischen Flaschenoberkante und Messfenster, damit die Flaschenoberkante nicht ins Messfenster gelangt
                int XSpaceBlueRect = 1;      // Linker bzw. Rechter Abstand zu Außenrand
                int NewWidthGreenRect = static_cast<int>(GetImageWidth() * pSettingsData->m_FactorMeasureWindowWidthSearchBottleTopPosiition) - 1;
                int NewWidthBlueRect = static_cast<int>(GetImageWidth() * FactorWidthBlueWindow) - 1;
                int NewHeightBlueRect = static_cast<int>(pProductData->m_DistanceInjectorBottle / m_CalculatetPixelSize) - (YSpaceBlueRectTop + YSpaceBlueRectBot);
                int NewYPosBlueRect = static_cast<int>(GreenRect.y() - (pProductData->m_DistanceInjectorBottle / m_CalculatetPixelSize) + YSpaceBlueRectTop);
                /*int InjectionMiddleWindowWidthInPixel = static_cast<int>(pProductData->m_InjectionMiddleWindowWidthInMm / m_CalculatetPixelSize);
                int ValveDistanceHalf = static_cast<int>((pSettingsData->m_DistancesBetweenValves / m_CalculatetPixelSize) / 2.0);
                int MinBlueWindowWidth = ValveDistanceHalf * 2 + InjectionMiddleWindowWidthInPixel + 6;*/

                pSettingsData->m_PixelSize = m_CalculatetPixelSize;
                SetCurrentPixelSizeInMMPerPixel(pSettingsData->m_PixelSize);
                GetMainAppCrystalT2()->SaveSettings();

                int MinBlueWindowWidth = GetMainAppCrystalT2()->GetMinBlueWindowWidthInPixel();
                if (NewWidthBlueRect < MinBlueWindowWidth) {
                    NewWidthBlueRect = MinBlueWindowWidth;
                    NewWidthGreenRect = (USED_CAMERA_WIDTH - NewWidthBlueRect) - 1;
                }

                if (NewHeightBlueRect > MaxHeightBlueWindow) {
                    NewHeightBlueRect = MaxHeightBlueWindow;  // erst mal auf 70 reduzieren damit die Rechenzeit nicht zu groß wird
                }
                if (GetMainAppCrystalT2()->GetLiveImageView()) {
                    if (!m_DoNotChangeBlueWindow) {
                        if (pSettingsData->m_BandDirectional == BAND_DIRECTIONAL_RIGHT_TO_LEFT)
                            NewRect.setX(XSpaceBlueRect);
                        else
                            NewRect.setX(GetImageWidth() - NewWidthBlueRect - XSpaceBlueRect);
                        NewRect.setY(NewYPosBlueRect);
                        NewRect.setWidth(NewWidthBlueRect);
                        NewRect.setHeight(NewHeightBlueRect);
                        SetMeasureWindowRect(ROI_ID_MEASURE_LIQUID, NewRect);
                        BlueRect = NewRect;
                        GetMainAppCrystalT2()->GetLiveImageView()->DrawMeasureWindow(ROI_ID_MEASURE_LIQUID);
                    }
                }
                if (pSettingsData->m_BandDirectional == BAND_DIRECTIONAL_RIGHT_TO_LEFT) {
                    int GreenXpos = GetImageWidth() - NewWidthGreenRect;
                    int diff = GreenXpos - (BlueRect.x() + BlueRect.width());
                    if (diff < 0) {  // Abfrage ob Überlappung vom Grünen zum Blauen Messfenster
                        GreenXpos = GreenXpos - diff;
                        NewWidthGreenRect = NewWidthGreenRect + diff;
                    }
                    NewRect.setX(GreenXpos);
                } else {
                    int GreenEndXpos = GreenRect.x() + NewWidthGreenRect;
                    int diff = BlueRect.x() - GreenEndXpos;
                    if (diff < 0) {
                        NewWidthGreenRect = NewWidthGreenRect + diff;  // Wenn Überlappung dann Grünes Messfenster reduzieren
                        // GreenXpos = GreenXpos + diff;
                    }
                    NewRect.setX(GreenRect.x());
                }
                NewRect.setY(GreenRect.y());
                NewRect.setWidth(NewWidthGreenRect);
                NewRect.setHeight(GreenRect.height());

                YellowRect.setX(BlueRect.x());
                YellowRect.setY(NewRect.y());
                YellowRect.setHeight(NewRect.height());
                YellowRect.setWidth(BlueRect.width());
                SetMeasureWindowRect(ROI_ID_MEASURE_BOTTLE_UNDER_VALVE, YellowRect);
                SlotSetCalibratetMeasureWindowSpeed(NewRect);  // Draw green and yellow Update Dialog
            }
        }
        delete pMessageBox;
    }
}

double ImageData::GetAverageCalibrationData()
{
    double sum = 0.0;
    for (int i = 0; i < m_ListCalibrateData1.count(); i++) sum = sum + m_ListCalibrateData1.at(i);
    sum = sum / m_ListCalibrateData1.count();
    return sum;
}

bool ImageData::FindBottleTopLine(ImageMetaData& LiveImage)
{
    bool rv = false;
    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();

    if (pSettingsData) {
        // lese Referenz ROI in ein OpenCV Bild
        cv::Mat Image(GetImageHeight(), GetImageWidth(), CV_8U, (void*)LiveImage.m_Image.constBits(), GetImageWidth());
        int w = static_cast<int>(GetImageWidth() * pSettingsData->m_FactorMeasureWindowWidthSearchBottleTopPosiition);  // MetricToPixel(pProductData->m_BottleNeckDiameter)*2.5);
        int h = GetImageHeight();
        int y = 0;
        int x = 0;
        cv::Mat ROIImage;
        double MaxValue, MinValue;
        double EdgeInPercent;
        QRect NewMeasurRect;

        h = h - (pSettingsData->m_BottleBaseLineOffsetInPix * 2);
        if (h < 0) h = GetImageHeight();
        y = pSettingsData->m_BottleBaseLineOffsetInPix;
        if (pSettingsData->m_BandDirectional == BAND_DIRECTIONAL_RIGHT_TO_LEFT) {
            x = GetImageWidth() - w;
        }
        QRect DrawAutoCalibrateROI(x, y, w, h);
        emit SignalDrawAutocalibrateROI(DrawAutoCalibrateROI);

        cv::Mat MeanValue(1, w, CV_32F);
        cv::Rect ROIRect(x, y, w, h);
        CopyROIImage(ROIRect, Image, ROIImage);
        cv::rotate(ROIImage, ROIImage, cv::ROTATE_90_COUNTERCLOCKWISE);
        cv::Mat GradientX(ROIImage.size(), CV_32F);
        cv::Sobel(ROIImage, GradientX, CV_32F, 1, 0, 3);
        cv::reduce(GradientX, MeanValue, 0, cv::REDUCE_AVG);
        cv::minMaxLoc(MeanValue, &MinValue, &MaxValue);
        y = FindFirstEdge(MeanValue, MinValue, GRADIENT_NEGATIVE) + pSettingsData->m_BottleBaseLineOffsetInPix;
        EdgeInPercent = fabs(MinValue) * THRESHOLD_FACTOR_IN_PERCENT_SOBEL_KERNEL_SIZE_3;
        if (EdgeInPercent > pSettingsData->m_MinPossibleContrastValueInPercent) {
            if (pSettingsData->m_BandDirectional == BAND_DIRECTIONAL_RIGHT_TO_LEFT)
                NewMeasurRect.setX(GetImageWidth() - static_cast<int>(GetImageWidth() * pSettingsData->m_FactorMeasureWindowWidthSearchBottleTopPosiition) - 1);
            else
                NewMeasurRect.setX(x);
            NewMeasurRect.setY(y);
            NewMeasurRect.setHeight(pSettingsData->m_MinMeasureWindowHeight);
            NewMeasurRect.setWidth(w);
            rv = true;
            emit SignalSetCalibratetMeasureWindowSpeed(NewMeasurRect);
        }
    }
    return rv;
}

void ImageData::SlotDrawAutocalibrateROI(const QRect& rect)
{
    if (GetMainAppCrystalT2()) {
        GetMainAppCrystalT2()->GetLiveImageView()->DrawAutocalbrateROI(rect);
    }
}

// not in use
bool ImageData::FindOptimalROIPositionEx(ImageMetaData& LiveImage)
{
    bool rv = false;
    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
    QRect NewWindow = GetMeasureWindowRect(ROI_ID_MEASURE_SPEED);
    int w = static_cast<int>(GetImageWidth() * pSettingsData->m_FactorMeasureWindowWidthSearchBottleTopPosiition);  // MetricToPixel(pProductData->m_BottleNeckDiameter)*2.5);
    // int w = NewWindow.width();
    int h = NewWindow.height();
    int x = NewWindow.x();
    int y = NewWindow.y();
    int yTopContrast = -1;
    int hTopContrast = -1;
    double ContrastRightEdge, ContrastLeftEdge;
    double MeanContrast, MeanContrastInPercent;
    cv::Mat ROIImage;
    cv::Mat MeanValue(1, w, CV_32F);
    // lese Referenz ROI in ein OpenCV Bild
    cv::Mat Image(GetImageHeight(), GetImageWidth(), CV_8U, (void*)LiveImage.m_Image.constBits(), GetImageWidth());

    int NumberStepsH = pSettingsData->m_NumIterationROIHeight;
    int NumberStepsY = 0;  // Messfenster soll immer an der Oberkante der Flasche beginnen
    for (int hstep = h; hstep <= (h + NumberStepsH); hstep++) {
        for (int ystep = y; ystep <= (y + NumberStepsY); ystep++) {
            cv::Rect ROIRect(x, ystep, w, hstep);
            CopyROIImage(ROIRect, Image, ROIImage);
            cv::Mat GradientX(ROIImage.size(), CV_32F);
            cv::Sobel(ROIImage, GradientX, CV_32F, 1, 0, 3);
            cv::reduce(GradientX, MeanValue, 0, cv::REDUCE_AVG);
            cv::minMaxLoc(MeanValue, &ContrastLeftEdge, &ContrastRightEdge);
            MeanContrast = (fabs(ContrastLeftEdge) + fabs(ContrastRightEdge)) / 2.0;
            MeanContrastInPercent = MeanContrast * THRESHOLD_FACTOR_IN_PERCENT_SOBEL_KERNEL_SIZE_3;
            if (MeanContrastInPercent > pSettingsData->m_MinPossibleContrastValueInPercent) {  // hier ein gültige Kante
                rv = true;
                if ((MeanContrast) > m_MaxEdgeContrastAutoCalibrate) {  // Suche Messfensterhöhe mit maximalen Kontrast
                    m_MaxEdgeContrastAutoCalibrate = MeanContrast;
                    yTopContrast = ystep;
                    hTopContrast = hstep;
                }
            }
        }
    }
    if (rv) {
        if (yTopContrast != -1) m_ListCalibrateData1.append((double)(yTopContrast));

        if (hTopContrast != -1) m_ListCalibrateData2.append((double)(hTopContrast));
    }
    return rv;
}

bool ImageData::FindOptimalROIPosition(ImageMetaData& LiveImage, bool BestPosition)
{
    bool rv = false;
    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
    QRect NewWindow = GetMeasureWindowRect(ROI_ID_MEASURE_SPEED);
    int w = NewWindow.width();
    int h = NewWindow.height();
    int x = NewWindow.x();
    int y = NewWindow.y();
    int yTopContrast = -1;
    int hTopContrast = -1;
    double ContrastRightEdge, ContrastLeftEdge;
    double MeanContrast, MeanContrastInPercent;
    cv::Mat ROIImage;
    cv::Mat MeanValue(1, w, CV_32F);
    cv::Mat Image(GetImageHeight(), GetImageWidth(), CV_8U, (void*)LiveImage.m_Image.constBits(), GetImageWidth());
    int NumberStepsY = pSettingsData->m_NumIterationROIYPos;
    int NumberStepsH = pSettingsData->m_NumIterationROIHeight;

    if (BestPosition)
        NumberStepsH = 0;  // Suche optimale Y-Top Position
    else
        NumberStepsY = 0;  // Suche optimale Messfensterbreite
    double weightFactorContrast = 0;
    for (int ystep = y; ystep <= (y + NumberStepsY); ystep++) {
        // for (int hstep = h + NumberStepsH; hstep >= h; hstep--) {
        for (int hstep = h; hstep <= (h + NumberStepsH); hstep++) {
            cv::Rect ROIRect(x, ystep, w, hstep);
            CopyROIImage(ROIRect, Image, ROIImage);
            cv::Mat GradientX(ROIImage.size(), CV_32F);
            cv::Sobel(ROIImage, GradientX, CV_32F, 1, 0, 3);
            cv::reduce(GradientX, MeanValue, 0, cv::REDUCE_AVG);
            cv::minMaxLoc(MeanValue, &ContrastLeftEdge, &ContrastRightEdge);
            MeanContrast = (fabs(ContrastLeftEdge) + fabs(ContrastRightEdge)) / 2.0;
            MeanContrastInPercent = MeanContrast * THRESHOLD_FACTOR_IN_PERCENT_SOBEL_KERNEL_SIZE_3;
            if (MeanContrastInPercent > pSettingsData->m_MinPossibleContrastValueInPercent) {  // hier ein gültige Kante
                rv = true;
                if (BestPosition) {
                    if (MeanContrast > m_MaxEdgeContrastAutoCalibrate) {  // Suche optimale Y-Top Position
                        m_MaxEdgeContrastAutoCalibrate = MeanContrast;
                        yTopContrast = ystep;
                        hTopContrast = hstep;
                    }
                } else {  // jetzt optimale Höhe ermitteln
                    weightFactorContrast = weightFactorContrast + 3.0;
                    // Suche optimale Messfensterhöhe Möglichts hoher Kontrast und möglichst große Höhe sind optimal
                    if ((MeanContrast + weightFactorContrast) > m_MaxEdgeContrastAutoCalibrate) {
                        m_MaxEdgeContrastAutoCalibrate = MeanContrast + (weightFactorContrast);
                        yTopContrast = ystep;
                        hTopContrast = hstep;
                    }
                }
            }
        }
    }
    if (rv) {
        if (BestPosition) {
            if (yTopContrast != -1) {
                m_ListCalibrateData1.append((double)(yTopContrast));
            }
        } else {
            if (hTopContrast != -1) m_ListCalibrateData1.append((double)(hTopContrast));
        }
    }
    return rv;
}

bool ImageData::FindOptimalEdgeThreshold(ImageMetaData& LiveImage)
{
    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
    QRect GreenRect = GetMeasureWindowRect(ROI_ID_MEASURE_SPEED);
    double MeanContrast, ContrastRightEdge, ContrastLeftEdge, LeftPos, RightPos;
    cv::Mat CameraImage(GetImageHeight(), GetImageWidth(), CV_8U, (void*)LiveImage.m_Image.constBits(), GetImageWidth());
    cv::Mat ROIImage;
    cv::Rect ROIRect(GreenRect.x(), GreenRect.y(), GreenRect.width(), GreenRect.height());
    bool rv = false;

    CopyROIImage(ROIRect, CameraImage, ROIImage);
    GetContrastAndPosLeftRightEdge(ROIImage, ContrastLeftEdge, ContrastRightEdge, LeftPos, RightPos);
    // Für die Berechnung der Kantenschwelle wird der kleineste Kontrast der beiden Kanten verwendet
    if (fabs(ContrastLeftEdge) < fabs(ContrastRightEdge)) {
        MeanContrast = fabs(ContrastLeftEdge);
    } else {
        MeanContrast = fabs(ContrastRightEdge);
    }
    // MeanContrast = (fabs(ContrastLeftEdge) + fabs(ContrastRightEdge)) / 2.0;
    MeanContrast = MeanContrast * THRESHOLD_FACTOR_IN_PERCENT_SOBEL_KERNEL_SIZE_3;
    if (MeanContrast > pSettingsData->m_MinPossibleContrastValueInPercent) {
        m_ListCalibrateData1.append(MeanContrast);
        rv = true;
    }
    return rv;
}

void ImageData::GetContrastAndPosLeftRightEdge(cv::Mat& ROIImage, double& ContrastLeftEdge, double& ContrastRightEdge, double& LeftPos, double& RightPos)
{
    int rv = 0;
    cv::Mat GradientX(ROIImage.rows, ROIImage.cols, CV_32F);
    cv::Mat MeanValue(1, ROIImage.cols, CV_32F);
    cv::Mat MeanValue180(1, ROIImage.cols, CV_32F);  // buffer Mittelwert um 180 Grad gedrehtes Bild
    cv::Point2d LeftEdgeLocation, RightEdgeLocation;
    cv::Point2d LeftEdgeLocationSubPixel, RightEdgeLocationSubPixel;

    cv::Sobel(ROIImage, GradientX, CV_32F, 1, 0, 3);
    cv::reduce(GradientX, MeanValue, 0, cv::REDUCE_AVG);
    cv::minMaxLoc(MeanValue, &ContrastLeftEdge, &ContrastRightEdge);
    LeftPos = FindFirstEdge(MeanValue, ContrastLeftEdge, GRADIENT_NEGATIVE);
    LeftEdgeLocation = cv::Point2d(LeftPos, 0.0);
    cv::flip(MeanValue, MeanValue180, 1);
    RightPos = FindFirstEdge(MeanValue180, ContrastRightEdge, GRADIENT_POSITIVE);
    RightEdgeLocation = cv::Point2d(static_cast<double>((ROIImage.cols - 1)) - RightPos, 0.0);
    MeanValue = cv::abs(MeanValue);
    rv = MinMaxLocSubPix(&LeftEdgeLocationSubPixel, MeanValue, &LeftEdgeLocation);
    if (rv == 0) LeftEdgeLocation = LeftEdgeLocationSubPixel;
    rv = MinMaxLocSubPix(&RightEdgeLocationSubPixel, MeanValue, &RightEdgeLocation);
    if (rv == 0) RightEdgeLocation = RightEdgeLocationSubPixel;

    LeftPos = LeftEdgeLocation.x;
    RightPos = RightEdgeLocation.x;
}

int ImageData::MinMaxLocSubPix(cv::Point2d* SubPixLoc, cv::Mat& Image, cv::Point2d* LocIn)
{
    int rv = 0;
    cv::Size MaxScan;
    cv::Point ScanRectMin;  // I used two Points instead of a Rect to prevent having Rect compute right/left values in each loop below
    cv::Point ScanRectMax;
    float FloatValueChange = 0.1f;  // FLT_EPSILON * 200.0f; // smallest change that we can do math on with some meaningful result.
    float SrcStartingPoint;
    // results
    cv::Point ScanRight;
    cv::Point ScanLeft;
    cv::Point ScanUp;
    cv::Point ScanDown;
    cv::Point Center = *LocIn;
    cv::Point2d A, B, C;
    cv::Point2d ResultX, ResultY;

    // set default result in case we fail
    SubPixLoc->x = LocIn->x;
    SubPixLoc->y = LocIn->y;
    MaxScan.width = Image.cols >> 4;
    MaxScan.height = Image.rows >> 4;

    ScanRectMin.x = static_cast<int>(LocIn->x) - MaxScan.width;
    if (ScanRectMin.x < 0) ScanRectMin.x = 0;

    ScanRectMin.y = static_cast<int>(LocIn->y) - MaxScan.height;
    if (ScanRectMin.y < 0) ScanRectMin.y = 0;

    ScanRectMax.x = static_cast<int>(LocIn->x) + MaxScan.width;
    if (ScanRectMax.x >= Image.cols) ScanRectMax.x = Image.cols - 1;

    ScanRectMax.y = static_cast<int>(LocIn->y) + MaxScan.height;
    if (ScanRectMax.y >= Image.rows) ScanRectMax.y = Image.rows - 1;

    // scan to find area to use. this can get complicated since we may be given a point near any of the edges of the blob we want to use.
    SrcStartingPoint = Image.at<float>(static_cast<int>(LocIn->y), static_cast<int>(LocIn->x));

    if (Image.cols >= 20) {
        ScanRight = Center;
        while (true) {
            ++ScanRight.x;                               // no point checking the passed location. so inc first
            if (ScanRight.x > ScanRectMax.x) return -1;  // ran out of room to scan
            if (abs(Image.at<float>(ScanRight.y, ScanRight.x) - SrcStartingPoint) > FloatValueChange) break;
        }
        ScanLeft = Center;
        while (true) {
            --ScanLeft.x;                               // no point checking the passed location. so inc first
            if (ScanLeft.x < ScanRectMin.x) return -1;  // ran out of room to scan
            if (abs(Image.at<float>(ScanLeft.y, ScanLeft.x) - SrcStartingPoint) > FloatValueChange) break;
        }
    }
    if (Image.rows >= 20) {
        ScanUp = Center;
        while (true) {
            ++ScanUp.y;                               // assume G cords. The actual direction of Up in the image is not important since the math is symmetrical
            if (ScanUp.y > ScanRectMax.y) return -1;  // ran out of room to scan
            if (abs(Image.at<float>(ScanUp.y, ScanUp.x) - SrcStartingPoint) > FloatValueChange) break;
        }
        ScanDown = Center;
        while (true) {
            --ScanDown.y;                               // assume G cords. The actual direction of Up in the image is not important since the math is symmetrical
            if (ScanDown.y < ScanRectMin.y) return -1;  // ran out of room to scan
            if (abs(Image.at<float>(ScanDown.y, ScanDown.x) - SrcStartingPoint) > FloatValueChange) break;
        }
    }
    if (Image.cols >= 20) {
        // B is highest, A and C are on the sides
        // X axis
        A.x = ScanLeft.x;                               // The pixel cords
        A.y = Image.at<float>(ScanLeft.y, ScanLeft.x);  // the Heat map value

        B.x = Center.x;                             // The pixel cords
        B.y = Image.at<float>(Center.y, Center.x);  // the Heat map value

        C.x = ScanRight.x;                                // The pixel cords
        C.y = Image.at<float>(ScanRight.y, ScanRight.x);  // the Heat map value
        rv = SubPixFitParabola(&ResultX, A, B, C);
        if (rv == 0) {
            // we throw away the y and use the x
            // clip and set error
            if (ResultX.x < ScanLeft.x) {
                ResultX.x = ScanLeft.x;
                // ErrorVal = 1;
            }
            if (ResultX.x > ScanRight.x) {
                ResultX.x = ScanRight.x;
                // ErrorVal = 1;
            }
            SubPixLoc->x = ResultX.x;
        }
    }
    if (Image.rows >= 20) {
        // Y axis
        // this time we swap x and y since the parabola is always found in the x
        A.x = ScanDown.y;                               // The pixel cords
        A.y = Image.at<float>(ScanDown.y, ScanDown.x);  // the Heat map value

        B.x = Center.y;                             // The pixel cords
        B.y = Image.at<float>(Center.y, Center.x);  // the Heat map value

        C.x = ScanUp.y;                             // The pixel cords
        C.y = Image.at<float>(ScanUp.y, ScanUp.x);  // the Heat map value
        rv = SubPixFitParabola(&ResultY, A, B, C);
        if (rv == 0) {
            // we throw away the y and use the x
            ResultY.y = ResultY.x;
            // clip and set error
            if (ResultY.y < ScanDown.y) {
                ResultY.y = ScanDown.y;
                // ErrorVal = 1;
            }
            if (ResultY.y > ScanUp.y) {
                ResultY.y = ScanUp.y;
                // ErrorVal = 1;
            }
            SubPixLoc->y = ResultY.y;
        }
    }
    return rv;
}

int ImageData::SubPixFitParabola(cv::Point2d* Result, cv::Point2d& P1, cv::Point2d& P2, cv::Point2d& P3)
{
    int rv = 0;
    // A x1^2 + B x1 + C = y1
    // A x2^2 + B x2 + C = y2
    // A x3^2 + B x3 + C = y3
    double denom = (P1.x - P2.x) * (P1.x - P3.x) * (P2.x - P3.x);
    if (denom != 0.0) {
        double A = (P3.x * (P2.y - P1.y) + P2.x * (P1.y - P3.y) + P1.x * (P3.y - P2.y)) / denom;
        double B = ((P3.x * P3.x) * (P1.y - P2.y) + (P2.x * P2.x) * (P3.y - P1.y) + (P1.x * P1.x) * (P2.y - P3.y)) / denom;
        double C = (P2.x * P3.x * (P2.x - P3.x) * P1.y + P3.x * P1.x * (P3.x - P1.x) * P2.y + P1.x * P2.x * (P1.x - P2.x) * P3.y) / denom;
        // y = A * x^2 + B * x + C
        // finde maximum
        if (A != 0.0) {
            Result->x = -B / (2 * A);           // erste ableitung null setzen maximum suchen
            Result->y = C - (B * B) / (4 * A);  // dann x in gleichung einsetzen um y zu berechnen
        } else
            rv = -1;
    } else
        rv = -1;
    return rv;
}

void ImageData::SlotSetCalibratetMeasureWindowSpeed(const QRect& NewWindow)
{
    QRect newWindow = NewWindow;
    SetMeasureWindowRect(ROI_ID_MEASURE_SPEED, newWindow);
    if (GetMainAppCrystalT2()) {
        QRect TriggerRect = GetMeasureWindowRect(ROI_ID_MEASURE_BOTTLE_UNDER_VALVE);
        QRect NewTriggerRect(TriggerRect.x(), newWindow.y(), TriggerRect.width(), TriggerRect.height());

        SetMeasureWindowRect(ROI_ID_MEASURE_BOTTLE_UNDER_VALVE, NewTriggerRect);
        GetMainAppCrystalT2()->GetLiveImageView()->DrawMeasureWindow(ROI_ID_MEASURE_SPEED);
        GetMainAppCrystalT2()->GetEditProductDialog()->hide();  // Update Dialog elemnts
        GetMainAppCrystalT2()->GetEditProductDialog()->show();
        GetMainAppCrystalT2()->GetEditProductDialog()->SetForceEditingFinishedSpeedWindow();
        GetMainAppCrystalT2()->GetEditProductDialog()->SetForceEditingFinishedInjectionWindow();
    }
}

double ImageData::FindFirstEdge(cv::Mat& MeanValue, double& Contrast, int dir)
{
    double Pos = 0.0;
    float Threshold = 0.5;

    for (int i = 0; i < MeanValue.cols - 1; i++) {
        float value = MeanValue.at<float>(0, i);
        float next = MeanValue.at<float>(0, i + 1);

        if (dir == GRADIENT_NEGATIVE) {
            if (value < Contrast * Threshold && value < next) {
                Pos = static_cast<double>(i);
                Contrast = static_cast<double>(value);
                break;
            }
        } else {
            if (value > Contrast * Threshold && value > next) {
                Pos = static_cast<double>(i);
                Contrast = static_cast<double>(value);
                break;
            }
        }
    }
    return Pos;
}

void ImageData::CopyROIImage(cv::Rect& ROIRect, cv::Mat& Source, cv::Mat& ROIImage)
{
    cv::Mat ROIImageRef(Source, ROIRect);

    ROIImage = cv::Mat::zeros(ROIRect.height, ROIRect.width, CV_8UC1);
    ROIImageRef.copyTo(ROIImage(cv::Rect(0, 0, ROIRect.width, ROIRect.height)));
}

// nur für simulation wenn keine Kamera angeschlossen
void ImageData::DrawTextOnQImage(QImage& img)
{
    QPainter p;
    QPen RedPen(Qt::red);

    p.begin(&img);
    p.setPen(RedPen);
    p.setFont(QFont("Times", 14, QFont::Bold));
    p.drawText(img.rect(), Qt::AlignCenter, tr("CAMERA SIMULATION !!!!!!!!!"));
    p.end();
}

// hier werden die Produktdaten/Parameter in den Echtzeitbereich übergeben. Wird immer beim Start aufgerufen oder wenn das Produkt gewechselt wird
void ImageData::SetCurrentProductDataToImageProcessingUnit()
{
    ProductData* pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
    SettingsData* pSettingsData = GetMainAppCrystalT2()->GetSettingsData();
    if (pProductData && pSettingsData) {
        QHash<int, QRect>* pMeasureWindowRects = pProductData->GetMeasureWindowRects();
        QHashIterator<int, QRect> i(*pMeasureWindowRects);
        int MeasurRectID;
        QRect MeasureRect;
        // zuerst die Produkabhängigen daten
        m_KernelParameter.m_MeasuringParameter.m_ReferenceInjectionPositionInPixel = pProductData->m_InjectionPositionInPixel;
        m_KernelParameter.m_MeasuringParameter.m_ReferenceMeasurePositionInPixel = pProductData->m_MeasurePositionInPixel;
        m_KernelParameter.m_MeasuringParameter.m_TriggerOffsetFirstValveInmm = pProductData->m_TriggerOffsetFirstValveInmm;
        m_KernelParameter.m_MeasuringParameter.m_TriggerOffsetSecondValveInmm = pProductData->m_TriggerOffsetSecondValveInmm;
        m_KernelParameter.m_MeasuringParameter.m_BottleneckDiameter = pProductData->m_BottleNeckDiameter;
        m_KernelParameter.m_MeasuringParameter.m_BotteleNeckDiameterToleranceInmm = pProductData->m_BotteleNeckDiameterToleranceInmm;
        m_KernelParameter.m_MeasuringParameter.m_ProductWidthInmm = pProductData->m_ProductWidth;
        m_KernelParameter.m_MeasuringParameter.m_AcceptanceThresholdLiquidLeftAndRightROI = pProductData->m_AcceptanceThresholdLiquidLeftAndRightROI;
        m_KernelParameter.m_MeasuringParameter.m_MinAcceptanceThresholdLiquidMiddleROI = pProductData->m_MinAcceptanceThresholdLiquidMiddleROI;
        m_KernelParameter.m_MeasuringParameter.m_MaxAcceptanceThresholdLiquidMiddleROI = pProductData->m_MaxAcceptanceThresholdLiquidMiddleROI;
        m_KernelParameter.m_MeasuringParameter.m_InjectonWindowMiddleWidthInMm = pProductData->m_InjectionMiddleWindowWidthInMm;
        m_KernelParameter.m_MeasuringParameter.m_UsedTriggerOutputs = pProductData->m_UsedTriggerOutputs;
        m_KernelParameter.m_MeasuringParameter.m_FormatFromISInmm = pProductData->m_FormatFromISInmm;
        // Parameter für die IO's
        m_KernelParameter.m_EtherCATConfigData.m_CyclusTimeIOTaskInms = pSettingsData->m_CycleTimeIOTask;
        m_KernelParameter.m_EtherCATConfigData.m_TimePeriodTriggerOutputOnInms = pSettingsData->m_TimePeriodTriggerOutputOnInms;
        m_KernelParameter.m_EtherCATConfigData.m_TimePeriodDigitalOutputOnInms = pSettingsData->m_TimePeriodDigitalOutputOnInms;
        m_KernelParameter.m_EtherCATConfigData.m_EtherCatDataSet.m_MixerSTMVelocity.Velocity = pSettingsData->m_SpeedMixerStepperValue;
        // Produktunabhängige Parameter
        m_KernelParameter.m_MeasuringParameter.m_ImageBackgroundContrast = pSettingsData->m_BackgroundContrast;
        m_KernelParameter.m_MeasuringParameter.m_FilterKernelSize = 3;
        m_KernelParameter.m_MeasuringParameter.m_EdgeThreshold = pSettingsData->m_EdgeAcceptanceThresholdInPercent;
        m_KernelParameter.m_MeasuringParameter.m_PixelSizeInMMPerPixel = pSettingsData->m_PixelSize;
        m_KernelParameter.m_MeasuringParameter.m_DistanceBottleEjectionInmm = pSettingsData->m_DistanceBottleEjectionInmm;
        m_KernelParameter.m_MeasuringParameter.m_BlowOutEjectorNormallyClosed = pSettingsData->m_BlowOutEjectorNormallyClosed;
        m_KernelParameter.m_MeasuringParameter.m_RightTriggerIsFirst = pSettingsData->m_RightTriggerIsFirst;
        m_KernelParameter.m_MeasuringParameter.m_TargetProcessorIOTask = pSettingsData->m_TargetProcessorIOTask;
        m_KernelParameter.m_MeasuringParameter.m_TargetProcessorMeasureTask = pSettingsData->m_TargetProcessorMeasureTask;
        m_KernelParameter.m_MeasuringParameter.m_MinSpeedInMMPerMs = pSettingsData->m_MinSpeedInMMPerMs;
        m_KernelParameter.m_MeasuringParameter.m_DistancesBetweenValves = pSettingsData->m_DistancesBetweenValves;
        m_KernelParameter.m_MeasuringParameter.m_ThresholdBinaryImageLiquid = pSettingsData->m_ThresholdBinaryImageLiquid;
        m_KernelParameter.m_MeasuringParameter.m_NumberProductsAverageBottleNeckAndPixelSize = pSettingsData->m_NumberProductsAverageBottleNeckAndPixelSize;
        m_KernelParameter.m_MeasuringParameter.m_MinNumberFoundedInROI = pSettingsData->m_MinNumberFoundedInROI;
        m_KernelParameter.m_MeasuringParameter.m_MaxMeasurementsProductIsOutOfTol = pSettingsData->m_MaxMeasurementsProductIsOutOfTol;
        m_KernelParameter.m_MeasuringParameter.m_WorkWithTwoValves = pSettingsData->m_WorkWithTwoValves;
        m_KernelParameter.m_MeasuringParameter.m_BandDirectional = pSettingsData->m_BandDirectional;
        m_KernelParameter.m_MeasuringParameter.m_RollingMeanValueLiquid = pSettingsData->m_RollingMeanValueLiquid;
        m_KernelParameter.m_MeasuringParameter.m_TimePeriodNotBlowInms = pSettingsData->m_TimePeriodNotBlowInms;
        m_KernelParameter.m_MeasuringParameter.m_BottleOffsetOutOfROIInmm = pSettingsData->m_BottleOffsetOutOfROIInmm;
        m_KernelParameter.m_MeasuringParameter.m_UseSpeedFromISCalcEjectionTime = pSettingsData->m_UseSpeedFromISCalcEjectionTime;
        // insgesamt gibt es vier messfenster 1.Kameramessfenster(i.d.R. immer die volle Auflösung) 2.Messfenster(Grün) indem die Geschwindigkeit der flasch berechnet wird  3. Messfenster(blau) um die
        // Flüssigkeitsmenge zu berechnen. 4. Messfenster um zu kontrollieren ob die Flasche an der Dosierposition steht
        while (i.hasNext()) {
            i.next();
            MeasurRectID = i.key();
            MeasureRect = i.value();

            if (MeasurRectID == ROI_ID_CAMERA) {
                m_ImageWidth = MeasureRect.width();
                m_ImageHeight = MeasureRect.height();
                m_ImageOffsetX = MeasureRect.topLeft().x();
                m_ImageOffsetY = MeasureRect.topLeft().y();
                m_ImagePitch = m_ImageWidth;
                m_KernelParameter.m_MeasuringParameter.m_MeasureWindowWidth[ROI_INDEX_CAMERA] = MeasureRect.width();
                m_KernelParameter.m_MeasuringParameter.m_MeasureWindowHeight[ROI_INDEX_CAMERA] = MeasureRect.height();
                m_KernelParameter.m_MeasuringParameter.m_MeasureWindowOffsetX[ROI_INDEX_CAMERA] = MeasureRect.topLeft().x();
                m_KernelParameter.m_MeasuringParameter.m_MeasureWindowOffsetY[ROI_INDEX_CAMERA] = MeasureRect.topLeft().y();
            } else {
                if (MeasurRectID == ROI_ID_MEASURE_SPEED) {
                    m_KernelParameter.m_MeasuringParameter.m_MeasureWindowWidth[ROI_INDEX_MEASURE_SPEED] = MeasureRect.width();
                    m_KernelParameter.m_MeasuringParameter.m_MeasureWindowHeight[ROI_INDEX_MEASURE_SPEED] = MeasureRect.height();
                    m_KernelParameter.m_MeasuringParameter.m_MeasureWindowOffsetX[ROI_INDEX_MEASURE_SPEED] = MeasureRect.topLeft().x();
                    m_KernelParameter.m_MeasuringParameter.m_MeasureWindowOffsetY[ROI_INDEX_MEASURE_SPEED] = MeasureRect.topLeft().y();
                    // data for ROI_INDEX_MEASURE_BOTTLE_UNDER_VALVE
                    // dieses Messfenster hat die gleiche Höhe und Y-pos wie das Messfenster indem die Geschwindigkeit(ID=ROI_ID_MEASURE_SPEED in grüner Farbe) ge
                    m_KernelParameter.m_MeasuringParameter.m_MeasureWindowHeight[ROI_INDEX_MEASURE_BOTTLE_UNDER_VALVE] = MeasureRect.height();
                    m_KernelParameter.m_MeasuringParameter.m_MeasureWindowOffsetY[ROI_INDEX_MEASURE_BOTTLE_UNDER_VALVE] = MeasureRect.topLeft().y();
                }
                if (MeasurRectID == ROI_ID_MEASURE_LIQUID) {
                    m_KernelParameter.m_MeasuringParameter.m_MeasureWindowWidth[ROI_INDEX_MEASURE_LIQUID] = MeasureRect.width();
                    m_KernelParameter.m_MeasuringParameter.m_MeasureWindowHeight[ROI_INDEX_MEASURE_LIQUID] = MeasureRect.height();
                    m_KernelParameter.m_MeasuringParameter.m_MeasureWindowOffsetX[ROI_INDEX_MEASURE_LIQUID] = MeasureRect.topLeft().x();
                    m_KernelParameter.m_MeasuringParameter.m_MeasureWindowOffsetY[ROI_INDEX_MEASURE_LIQUID] = MeasureRect.topLeft().y();
                    // data for ROI_INDEX_MEASURE_BOTTLE_UNDER_VALVE
                    // dieses Messfenster hat die gleiche Breite und x-Pos wie das Messfenster indem die Geschwindigkeit(ID=ROI_ID_MEASURE_SPEED in grüner Farbe) gemessen wird
                    m_KernelParameter.m_MeasuringParameter.m_MeasureWindowWidth[ROI_INDEX_MEASURE_BOTTLE_UNDER_VALVE] = MeasureRect.width();
                    m_KernelParameter.m_MeasuringParameter.m_MeasureWindowOffsetX[ROI_INDEX_MEASURE_BOTTLE_UNDER_VALVE] = MeasureRect.topLeft().x();
                }
            }
        }
        if (GetKitharaCore()) {
            GetKitharaCore()->SetKernelParameter(&m_KernelParameter);
            GetKitharaCore()->SetKernelParameterToRealTime();  // in den Echtzeitbereich
        }
        SaveProductData();
    } else {
        QString ErrorMsg = tr("Can Not Set ProductData No Products Defined");
        emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
    }
}

QHash<int, QRect>* ImageData::GetMeasureWindowRects()
{
    QHash<int, QRect>* pMeasureWindowRects = NULL;
    if (GetMainAppCrystalT2()) {
        ProductData* pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
        if (pProductData) {
            pMeasureWindowRects = pProductData->GetMeasureWindowRects();
        }
    }
    return pMeasureWindowRects;
}

void ImageData::SaveProductData()
{
    if (GetMainAppCrystalT2()) {
        ProductData* pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
        if (pProductData) {
            QHash<int, QRect>* pMeasureWindowRects = pProductData->GetMeasureWindowRects();
            QHashIterator<int, QRect> i(*pMeasureWindowRects);
            int MeasurRectID;
            QRect MeasureRect;
            QString ErrorMsg;

            pProductData->m_InjectionPositionInPixel = m_KernelParameter.m_MeasuringParameter.m_ReferenceInjectionPositionInPixel;
            pProductData->m_MeasurePositionInPixel = m_KernelParameter.m_MeasuringParameter.m_ReferenceMeasurePositionInPixel;
            pProductData->m_DistanceBetweenMeasurePosAndTriggerPosInPixel = m_KernelParameter.m_MeasuringParameter.m_DistanceBetweenMeasureMiddlePosAndTriggerPosInPixel;
            pProductData->m_TriggerOffsetFirstValveInmm = m_KernelParameter.m_MeasuringParameter.m_TriggerOffsetFirstValveInmm;
            pProductData->m_TriggerOffsetSecondValveInmm = m_KernelParameter.m_MeasuringParameter.m_TriggerOffsetSecondValveInmm;
            pProductData->m_BottleNeckDiameter = m_KernelParameter.m_MeasuringParameter.m_BottleneckDiameter;
            pProductData->m_BotteleNeckDiameterToleranceInmm = m_KernelParameter.m_MeasuringParameter.m_BotteleNeckDiameterToleranceInmm;
            pProductData->m_ProductWidth = m_KernelParameter.m_MeasuringParameter.m_ProductWidthInmm;
            pProductData->m_AcceptanceThresholdLiquidLeftAndRightROI = m_KernelParameter.m_MeasuringParameter.m_AcceptanceThresholdLiquidLeftAndRightROI;
            pProductData->m_MinAcceptanceThresholdLiquidMiddleROI = m_KernelParameter.m_MeasuringParameter.m_MinAcceptanceThresholdLiquidMiddleROI;
            pProductData->m_MaxAcceptanceThresholdLiquidMiddleROI = m_KernelParameter.m_MeasuringParameter.m_MaxAcceptanceThresholdLiquidMiddleROI;
            pProductData->m_InjectionMiddleWindowWidthInMm = m_KernelParameter.m_MeasuringParameter.m_InjectonWindowMiddleWidthInMm;
            pProductData->m_UsedTriggerOutputs = m_KernelParameter.m_MeasuringParameter.m_UsedTriggerOutputs;
            pProductData->m_FormatFromISInmm = m_KernelParameter.m_MeasuringParameter.m_FormatFromISInmm;
            while (i.hasNext()) {
                i.next();
                MeasurRectID = i.key();
                MeasureRect = i.value();
                if (MeasurRectID == ROI_ID_CAMERA) {
                    MeasureRect.setWidth(m_ImageWidth);
                    MeasureRect.setHeight(m_ImageHeight);
                    MeasureRect.topLeft().setX(m_ImageOffsetX);
                    MeasureRect.topLeft().setY(m_ImageOffsetY);
                    pMeasureWindowRects->insert(ROI_ID_CAMERA, MeasureRect);
                } else {
                    if (MeasurRectID == ROI_ID_MEASURE_SPEED) {
                        MeasureRect.setWidth(m_KernelParameter.m_MeasuringParameter.m_MeasureWindowWidth[ROI_INDEX_MEASURE_SPEED]);
                        MeasureRect.setHeight(m_KernelParameter.m_MeasuringParameter.m_MeasureWindowHeight[ROI_INDEX_MEASURE_SPEED]);
                        MeasureRect.topLeft().setX(m_KernelParameter.m_MeasuringParameter.m_MeasureWindowOffsetX[ROI_INDEX_MEASURE_SPEED]);
                        MeasureRect.topLeft().setY(m_KernelParameter.m_MeasuringParameter.m_MeasureWindowOffsetY[ROI_INDEX_MEASURE_SPEED]);
                        pMeasureWindowRects->insert(ROI_ID_MEASURE_SPEED, MeasureRect);
                    }
                    if (MeasurRectID == ROI_ID_MEASURE_LIQUID) {
                        MeasureRect.setWidth(m_KernelParameter.m_MeasuringParameter.m_MeasureWindowWidth[ROI_INDEX_MEASURE_LIQUID]);
                        MeasureRect.setHeight(m_KernelParameter.m_MeasuringParameter.m_MeasureWindowHeight[ROI_INDEX_MEASURE_LIQUID]);
                        MeasureRect.topLeft().setX(m_KernelParameter.m_MeasuringParameter.m_MeasureWindowOffsetX[ROI_INDEX_MEASURE_LIQUID]);
                        MeasureRect.topLeft().setY(m_KernelParameter.m_MeasuringParameter.m_MeasureWindowOffsetY[ROI_INDEX_MEASURE_LIQUID]);
                        pMeasureWindowRects->insert(ROI_ID_MEASURE_LIQUID, MeasureRect);
                    }
                    if (MeasurRectID == ROI_ID_MEASURE_BOTTLE_UNDER_VALVE) {
                        MeasureRect.setWidth(m_KernelParameter.m_MeasuringParameter.m_MeasureWindowWidth[ROI_INDEX_MEASURE_BOTTLE_UNDER_VALVE]);
                        MeasureRect.setHeight(m_KernelParameter.m_MeasuringParameter.m_MeasureWindowHeight[ROI_INDEX_MEASURE_BOTTLE_UNDER_VALVE]);
                        MeasureRect.topLeft().setX(m_KernelParameter.m_MeasuringParameter.m_MeasureWindowOffsetX[ROI_INDEX_MEASURE_BOTTLE_UNDER_VALVE]);
                        MeasureRect.topLeft().setY(m_KernelParameter.m_MeasuringParameter.m_MeasureWindowOffsetY[ROI_INDEX_MEASURE_BOTTLE_UNDER_VALVE]);
                        pMeasureWindowRects->insert(ROI_ID_MEASURE_BOTTLE_UNDER_VALVE, MeasureRect);
                    }
                }
            }
            pProductData->WriteProductData(ErrorMsg);
        } else {
            QString ErrorMsg = tr("Can Not Save ProductData No Products Defined");
            emit SignalShowInfo(ErrorMsg, QtMsgType::QtFatalMsg);  // ERROR_CODE_ANY_ERROR);
        }
    }
}

void ImageData::ResetCountersBottlesEjectionAndLiquidTooLow()
{
    if (GetKitharaCore()) GetKitharaCore()->ResetCountersBottlesEjectionAndLiquidTooLow();
}

void ImageData::ResetAllCounters()
{
    if (GetKitharaCore()) GetKitharaCore()->ResetAllCounters();
}

KitharaCore* ImageData::GetKitharaCore()
{
    if (GetMainAppCrystalT2())
        return GetMainAppCrystalT2()->GetKitharaCore();
    else
        return NULL;
}

// Wird beim Starten der Software einmalig aufgerufen
int ImageData::InitRealTimeSystem(QString& ErrorMsg)
{
    QString VideoFileNameCameraSimulation = "VideoFile17.avi";
    int rv = ERROR_CODE_NO_ERROR;
    int CameraIndex = 0;
    int DeviceIndex = 0;
    int NetworkAdapterID = 0;

    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        DeviceIndex = GetMainAppCrystalT2()->GetSettingsData()->m_DeviceIndexXHCIController;
        NetworkAdapterID = GetMainAppCrystalT2()->GetSettingsData()->m_NetworkAdapterID;
        VideoFileNameCameraSimulation = GetMainAppCrystalT2()->GetSettingsData()->m_VideoFileNameCameraSimulation;
    }
    if (GetKitharaCore()) {
        GetKitharaCore()->SetKernelParameter(&m_KernelParameter);
        rv = GetKitharaCore()->InitRealTimeSystem(CameraIndex, DeviceIndex, NetworkAdapterID, VideoFileNameCameraSimulation);
        if (rv != ERROR_CODE_NO_ERROR)
            m_RealTimeCameraInitialised = false;
        else
            m_RealTimeCameraInitialised = true;
    }
    return rv;
}

bool ImageData::IsCameraResolutionGreaterThanDefaultResolution()
{
    if (GetKitharaCore()) {
        return GetKitharaCore()->IsCameraResolutionGreaterThanDefaultResolution();
    } else {
        return false;
    }
}

int ImageData::GetMaxImageWidth()
{
    int rv = USED_CAMERA_WIDTH;
    if (GetKitharaCore()) {
        rv = GetKitharaCore()->GetMaxImageWidth();
    }
    return rv;
}

int ImageData::GetMaxImageHeight()
{
    int rv = USED_CAMERA_HEIGHT;
    if (GetKitharaCore()) {
        rv = GetKitharaCore()->GetMaxImageHeight();
    }
    return rv;
}

int ImageData::SetCameraViewPort(int offsetX, int offsetY, int width, int height, QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    if (GetKitharaCore()) {
        rv = GetKitharaCore()->SetCameraViewPort(offsetX, offsetY, width, height, ErrorMsg);
    }
    return rv;
}

int ImageData::SetCameraXOffset(int newOffsetX, QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    if (GetKitharaCore()) {
        rv = GetKitharaCore()->SetCameraXOffset(newOffsetX, ErrorMsg);
    }
    return rv;
}

int ImageData::SetCameraYOffset(int newOffsetY, QString& ErrorMsg)
{
    int rv = ERROR_CODE_NO_ERROR;
    if (GetKitharaCore()) {
        rv = GetKitharaCore()->SetCameraYOffset(newOffsetY, ErrorMsg);
    }
    return rv;
}

int ImageData::StartCameraAcquisition()
{
    int rv = ERROR_CODE_NO_ERROR;
    if (GetKitharaCore()) {
        rv = GetKitharaCore()->StartCameraAcquisition();
    }
    return rv;
}

int ImageData::StopCameraAcquisition()
{
    int rv = ERROR_CODE_NO_ERROR;
    if (GetKitharaCore()) {
        rv = GetKitharaCore()->StopCameraAcquisition();
    }
    return rv;
}

bool ImageData::IsKitharaDriverOpen()
{
    if (GetKitharaCore())
        return GetKitharaCore()->IsKitharaDriverOpen();
    else
        return false;
}

void ImageData::DisableLiveImageView()
{
    if (isRunning()) {  // thread läuft noch
        m_WaitLiveImageViewIsDisable.lock();
        m_WaitConditionLiveViewIsDisable.wait(&m_WaitLiveImageViewIsDisable, 2000);  // warte bis livebildanzeige beendet
        m_WaitLiveImageViewIsDisable.unlock();
    }
}

void ImageData::FinishedThread()
{
    if (isRunning()) {  // thread läuft noch
        m_TerminateLiveImageView = true;
        if (GetKitharaCore()) GetKitharaCore()->ForceSetEventGetNewImage();
        m_WaitLiveImageViewIsDisable.lock();
        m_WaitConditionLiveViewIsDisable.wait(&m_WaitLiveImageViewIsDisable, 2000);  // warte bis livebildanzeige beendet
        m_WaitLiveImageViewIsDisable.unlock();
    }
}

void ImageData::FinishedThreadAndFreeAllAllocations()
{
    FinishedThread();
    if (GetKitharaCore()) GetKitharaCore()->FreeAllKithara();
}

void ImageData::CalculateDisplayZoomFactor(int& DisplayWidth, int& DisplayHeight)
{
    m_DisplayZoomFactor = 1.0;
    while (DisplayWidth > MAX_IMAGE_DISPLAY_RESOLUTION || DisplayHeight > MAX_IMAGE_DISPLAY_RESOLUTION) {
        DisplayWidth /= 2;
        DisplayHeight /= 2;
        m_DisplayZoomFactor /= 2.0;
    }
}

double ImageData::MetricToPixel(double MM)
{
    double rv = m_KernelParameter.m_MeasuringParameter.m_PixelSizeInMMPerPixel;

    if (rv <= 0.0) rv = 1.0;
    return MM / rv;
}

void ImageData::SetUsedTriggerOutput(int Value)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetUsedTriggerOutput(Value);
        m_KernelParameter.m_MeasuringParameter.m_UsedTriggerOutputs = Value;
        SaveProductData();
    }
}

void ImageData::SetReferenceMeasurePositionInPixel(int set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetReferenceMeasurePositionInPixel(set);
        m_KernelParameter.m_MeasuringParameter.m_ReferenceMeasurePositionInPixel = set;
        SaveProductData();
    }
}

void ImageData::SetReferenceInjectionPositionInPixel(int set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetReferenceInjectionPositionInPixel(set);
        m_KernelParameter.m_MeasuringParameter.m_ReferenceInjectionPositionInPixel = set;
        SaveProductData();
    }
}

void ImageData::SetDistanceBetweenMeasurePosAndTriggerPosInPixel(double set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetDistanceBetweenMeasurePosAndTriggerPosInPixel(set);
        m_KernelParameter.m_MeasuringParameter.m_DistanceBetweenMeasureMiddlePosAndTriggerPosInPixel = set;
        SaveProductData();
    }
}

void ImageData::SetInjectonWindowMiddleWidthInMm(double set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetInjectonWindowMiddleWidthInMm(set);
        m_KernelParameter.m_MeasuringParameter.m_InjectonWindowMiddleWidthInMm = set;
        SaveProductData();
    }
}

double ImageData::PixelToMetric(double Pix)
{
    double rv = m_KernelParameter.m_MeasuringParameter.m_PixelSizeInMMPerPixel;

    if (rv <= 0.0) rv = 1.0;
    return rv * Pix;
}

void ImageData::WriteQImage(QImage& image)
{
    QString FileNamePNG = QCoreApplication::applicationDirPath() + QString("%1").arg("/Images/") + QString("Image.png");
    QImageWriter writer(FileNamePNG, "png");
    writer.write(image);
}

int ImageData::ReadQImage(QImage& image, QString& ErrorMsg)
{
    int rv = 0;
    QString FileName = QCoreApplication::applicationDirPath() + QString("%1").arg("/Images/") + QString("Image%1.jpg").arg(m_ImageFileCounter);
    QImageReader reader(FileName);

    if (!reader.read(&image)) {
        if (m_ImageFileCounter != 1) {
            m_ImageFileCounter = 1;
            FileName = QCoreApplication::applicationDirPath() + QString("%1").arg("/Images/") + QString("Image%1.jpg").arg(m_ImageFileCounter);
            if (!reader.read(&image)) {
                ErrorMsg = tr("Can Not Read Image: %1").arg(FileName);
                rv = ERROR_CODE_ANY_ERROR;
            }
        } else {
            ErrorMsg = tr("Can Not Read Image: %1").arg(FileName);
            rv = ERROR_CODE_ANY_ERROR;
        }
    } else
        m_ImageFileCounter++;
    return rv;
}


void ImageData::SetVideoStateCameraSimulation(int set)
{
    if (GetKitharaCore()) GetKitharaCore()->SetVideoStateCameraSimulation(set);
}

void ImageData::SetEdgeThreshold(int set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetEdgeThreshold(set);
        m_KernelParameter.m_MeasuringParameter.m_EdgeThreshold = set;
        // SaveProductData();
    }
}

int ImageData::GetEdgeThreshold()
{
    return m_KernelParameter.m_MeasuringParameter.m_EdgeThreshold;
}

void ImageData::SetFilterKernelSize(int set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetFilterKernelSize(set);
        m_KernelParameter.m_MeasuringParameter.m_FilterKernelSize = set;
        SaveProductData();
    }
}

int ImageData::GetFilterKernelSize()
{
    return m_KernelParameter.m_MeasuringParameter.m_FilterKernelSize;
}

void ImageData::SetExposureTime(double set)
{
    if (GetKitharaCore()) GetKitharaCore()->SetExposureTime(set);
}

void ImageData::SetRollingMeanValueLiquid(int set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetRollingMeanValueLiquid(set);
        m_KernelParameter.m_MeasuringParameter.m_RollingMeanValueLiquid = set;
    }
}

void ImageData::SetImageBackgroundContrast(int set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetImageBackgroundContrast(set);
        m_KernelParameter.m_MeasuringParameter.m_ImageBackgroundContrast = set;
    }
}

int ImageData::GetImageBackgroundContrast()
{
    return m_KernelParameter.m_MeasuringParameter.m_ImageBackgroundContrast;
}

void ImageData::SetPollingTimeIOTaskInms(double set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetPollingTimeIOTaskInms(set);
        m_KernelParameter.m_EtherCATConfigData.m_CyclusTimeIOTaskInms = set;
    }
}

double ImageData::GetPollingTimeIOTaskInms()
{
    return m_KernelParameter.m_EtherCATConfigData.m_CyclusTimeIOTaskInms;
}

void ImageData::SetTimePeriodTriggerOutputOnInms(double set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetTimePeriodTriggerOutputOnInms(set);
        m_KernelParameter.m_EtherCATConfigData.m_TimePeriodTriggerOutputOnInms = set;
    }
}

void ImageData::SetTimePeriodDigitalOutputOnInms(double set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetTimePeriodDigitalOutputOnInms(set);
        m_KernelParameter.m_EtherCATConfigData.m_TimePeriodDigitalOutputOnInms = set;
    }
}

void ImageData::SetDefaultPreasure(short value)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetAnalogOutputValue(QString(AO_NAME_SET_POINT_PREASURE_VALUE), value);
    }
}

void ImageData::SetAirCoolingCamera(short value)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetAnalogOutputValue(QString(AO_NAME_SET_POINT_AIR_COOLING_CAMERA_AND_BACK_LIGHT), value);
    }
}

void ImageData::SetAirCoolingLight(short value)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetAnalogOutputValue(QString(AO_NAME_SET_POINT_AIR_COOLING_GLASS), value);
    }
}

void ImageData::SetAirCoolingValve(short value)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetAnalogOutputValue(QString(AO_NAME_SET_POINT_AIR_COOLING_VALVES), value);
    }
}

void ImageData::SetWaterCoolingDefault(short value)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetAnalogOutputValue(QString(AO_NAME_SET_POINT_WATER_COOLING), value);
    }
}

void ImageData::SetWaterCoolingSensor(short value)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetAnalogOutputValue(QString(AO_NAME_SET_POINT_WATER_COOLING_SENSOR), value);
    }
}

void ImageData::SetAnalogOutputValue(const QString& Name, short Value)
{
    if (GetKitharaCore()) GetKitharaCore()->SetAnalogOutputValue(Name, Value);
}

void ImageData::SetAnalogInputValue(const QString& ChannelName, short Value)  // Nur für tests
{
    if (GetKitharaCore()) GetKitharaCore()->SetAnalogInputValue(ChannelName, Value);
}

void ImageData::SetAnalogInputValue2Byte(const QString& ChannelName, short& Value)  // Nur für Tests
{
    if (GetKitharaCore()) GetKitharaCore()->SetAnalogInputValue2Byte(ChannelName, Value);
}

void ImageData::SetChangeTriggerOutputOrder(bool set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetChangeTriggerOutputOrder(set);
        m_KernelParameter.m_MeasuringParameter.m_RightTriggerIsFirst = set;
    }
}

void ImageData::SetDistanceBottleEjection(double set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetDistanceBottleEjection(set);
        m_KernelParameter.m_MeasuringParameter.m_DistanceBottleEjectionInmm = set;
    }
}

/*void ImageData::SetInvertBottleEjection(bool set)
{
        if (GetKitharaCore())
        {
                GetKitharaCore()->SetInvertBottleEjection(set);
                m_KernelParameter.m_MeasuringParameter.m_InjectionOnIfGap = set;
        }
}
*/

// nur für die Simulation
void ImageData::SetENCStatusMixerSimulation(ushort set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetENCStatusMixerSimulation(set);
    }
}
// nur für die Simulation
void ImageData::SetStatusMixerSimulation(ushort set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetStatusMixerSimulation(set);
    }
}
//nur für die Simulation
void ImageData::SetActualMixerVelocity(ushort velocity)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetActualVelocityMixer(velocity);
    }
}
//Setze Vorgabegeschwindigkeit
void ImageData::SetMixerVelocity(int velocity)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetMixerVelocity(velocity);
        m_KernelParameter.m_EtherCATConfigData.m_EtherCatDataSet.m_MixerSTMVelocity.Velocity = velocity;
    }
}

void ImageData::SetUseSpeedFromISCalcEjectionTime(bool set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetUseSpeedFromISCalcEjectionTime(set);
        m_KernelParameter.m_MeasuringParameter.m_UseSpeedFromISCalcEjectionTime = set;
    }
}

void ImageData::SetDistancesBetweenValves(double set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetDistancesBetweenValves(set);
        m_KernelParameter.m_MeasuringParameter.m_DistancesBetweenValves = set;
    }
}

void ImageData::SetMinSpeedInMMPerMs(double set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetMinSpeedInMMPerMs(set);
        m_KernelParameter.m_MeasuringParameter.m_MinSpeedInMMPerMs = set;
    }
}

void ImageData::SetMaxMeasurementsProductIsOutOfTol(int set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetMaxMeasurementsProductIsOutOfTol(set);
        m_KernelParameter.m_MeasuringParameter.m_MaxMeasurementsProductIsOutOfTol = set;
    }
}

void ImageData::SetThresholdBinaryImageLiquid(int set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetThresholdBinaryImageLiquid(set);
        m_KernelParameter.m_MeasuringParameter.m_ThresholdBinaryImageLiquid = set;
    }
}

void ImageData::SetNumberProductsAverageBottleNeckAndPixelSize(int set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetNumberProductsAverageBottleNeckAndPixelSize(set);
        m_KernelParameter.m_MeasuringParameter.m_NumberProductsAverageBottleNeckAndPixelSize = set;
    }
}

void ImageData::SetMinNumberFoundedInROI(int set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetMinNumberFoundedInROI(set);
        m_KernelParameter.m_MeasuringParameter.m_MinNumberFoundedInROI = set;
    }
}

void ImageData::SetButtonIsClickedEjectTheNextnBottles()
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetButtonIsClickedEjectTheNextnBottles();
    }
}

void ImageData::SetNumberEjectedBottlesByUser(int set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetNumberEjectedBottlesByUser(set);
    }
}

double ImageData::GetTimePeriodTriggerOutputInms()
{
    return m_KernelParameter.m_EtherCATConfigData.m_TimePeriodTriggerOutputOnInms;
}

double ImageData::GetTimePeriodDigitalOutputInms()
{
    return m_KernelParameter.m_EtherCATConfigData.m_TimePeriodTriggerOutputOnInms;
}

void ImageData::SetBotteleNeckDiameterToleranceInmm(double set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetBotteleNeckDiameterToleranceInmm(set);
        m_KernelParameter.m_MeasuringParameter.m_BotteleNeckDiameterToleranceInmm = set;
        SaveProductData();
    }
}

double ImageData::GetBotteleNeckDiameterToleranceInmm()
{
    return m_KernelParameter.m_MeasuringParameter.m_BotteleNeckDiameterToleranceInmm;
}

/*void  ImageData::SetProductToleranceInPercent(double set)
{
        if (GetKitharaCore())
        {
                GetKitharaCore()->SetProductToleranceInPercent(set);
                m_KernelParameter.m_MeasuringParameter.m_ProductToleranceInPercent = set;
                SaveProductData();
        }
}
*/

void ImageData::SetAcceptanceThresholdLiquidLeftAndRightROI(int set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetAcceptanceThresholdLiquidLeftAndRightROI(set);
        m_KernelParameter.m_MeasuringParameter.m_AcceptanceThresholdLiquidLeftAndRightROI = set;
        SaveProductData();
    }
}

void ImageData::SetMinAcceptanceThresholdLiquidMiddleROI(int set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetMinAcceptanceThresholdLiquidMiddleROI(set);
        m_KernelParameter.m_MeasuringParameter.m_MinAcceptanceThresholdLiquidMiddleROI = set;
        SaveProductData();
    }
}

void ImageData::SetMaxAcceptanceThresholdLiquidMiddleROI(int set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetMaxAcceptanceThresholdLiquidMiddleROI(set);
        m_KernelParameter.m_MeasuringParameter.m_MaxAcceptanceThresholdLiquidMiddleROI = set;
        SaveProductData();
    }
}

/*double ImageData::GetProductToleranceInPercent()
{
        return	m_KernelParameter.m_MeasuringParameter.m_ProductToleranceInPercent;
}
*/

void ImageData::SetCurrentPixelSizeInMMPerPixel(double set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetCurrentPixelSizeInMMPerPixel(set);
        m_KernelParameter.m_MeasuringParameter.m_PixelSizeInMMPerPixel = set;
    }
}

void ImageData::SetInfoLevel(int set)
{
    m_InfoLevel = set;
    if (GetKitharaCore()) {
        GetKitharaCore()->SetInfoLevel(set);
    }
}

int ImageData::GetInfoLevel()
{
    return m_InfoLevel;
}

double ImageData::GetCurrentPixelSizeInMMPerPixel()
{
    return m_KernelParameter.m_MeasuringParameter.m_PixelSizeInMMPerPixel;
}

void ImageData::SetTriggerOffsetInmm(double set, int ValveID)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetTriggerOffsetInmm(set, ValveID);
        if (ValveID == LEFT_VALVE_ID) {
            if (m_KernelParameter.m_MeasuringParameter.m_BandDirectional == BAND_DIRECTIONAL_RIGHT_TO_LEFT) {
                m_KernelParameter.m_MeasuringParameter.m_TriggerOffsetSecondValveInmm = set;
            } else {
                m_KernelParameter.m_MeasuringParameter.m_TriggerOffsetFirstValveInmm = set;
            }
        } else {
            if (ValveID == RIGHT_VALVE_ID) {
                if (m_KernelParameter.m_MeasuringParameter.m_BandDirectional == BAND_DIRECTIONAL_RIGHT_TO_LEFT) {
                    m_KernelParameter.m_MeasuringParameter.m_TriggerOffsetFirstValveInmm = set;
                } else {
                    m_KernelParameter.m_MeasuringParameter.m_TriggerOffsetSecondValveInmm = set;
                }
            }
        }
        SaveProductData();
    }
}

void ImageData::MirrorMeasureWindows()  // int set)
{
    QRect MeasureRect = GetMeasureWindowRect(ROI_ID_MEASURE_SPEED);
    QRect InjectionRect = GetMeasureWindowRect(ROI_ID_MEASURE_LIQUID);
    QRect TriggerRect = GetMeasureWindowRect(ROI_ID_MEASURE_BOTTLE_UNDER_VALVE);
    int YposMeasureRect = MeasureRect.y();
    int YposInjectionRect = InjectionRect.y();
    int MeasureRectXposPlusWidht = MeasureRect.x() + MeasureRect.width();
    int InjectionRectXposPlusWidht = InjectionRect.x() + InjectionRect.width();
    double MeasureRectXposPlusWidhtInCarCorr = TransformImageXCoorToCartXCoor((double)(MeasureRectXposPlusWidht)) * (-1.0);
    double InjectionRectXposPlusWidhtInCarCorr = TransformImageXCoorToCartXCoor((double)(InjectionRectXposPlusWidht)) * (-1.0);
    int MirrorXposMesureWindow = static_cast<int>(TransformCartXCoorToImageXCoor(MeasureRectXposPlusWidhtInCarCorr));
    int MirrorXposInjectionWindow = static_cast<int>(TransformCartXCoorToImageXCoor(InjectionRectXposPlusWidhtInCarCorr));
    QRect NewMeasurRect(MirrorXposMesureWindow, YposMeasureRect, MeasureRect.width(), MeasureRect.height());
    QRect NewInjectionRect(MirrorXposInjectionWindow, YposInjectionRect, InjectionRect.width(), InjectionRect.height());
    QRect NewTriggerRect(MirrorXposInjectionWindow, TriggerRect.y(), TriggerRect.width(), TriggerRect.height());
    SetMeasureWindowRect(ROI_ID_MEASURE_SPEED, NewMeasurRect);
    SetMeasureWindowRect(ROI_ID_MEASURE_LIQUID, NewInjectionRect);
    SetMeasureWindowRect(ROI_ID_MEASURE_BOTTLE_UNDER_VALVE, NewTriggerRect);
}

int ImageData::GetBandDirectional()
{
    return m_KernelParameter.m_MeasuringParameter.m_BandDirectional;
}

void ImageData::SetBandDirectional(int set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetBandDirectional(set);
        m_KernelParameter.m_MeasuringParameter.m_BandDirectional = set;
        // SaveProductData();
    }
}

void ImageData::SetMeasureWindowRect(int Key, QRect& NewRect)
{
    QRect rect;
    ProductData* pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
    if (pProductData) {
        QHash<int, QRect>* pMeasureWindowRects = pProductData->GetMeasureWindowRects();
        if (pMeasureWindowRects) {
            if (Key != ROI_ID_CAMERA) {
                CheckIsMeasureWindowInRange(NewRect, Key);
            }
            pMeasureWindowRects->insert(Key, NewRect);
            SetCurrentProductDataToImageProcessingUnit();
            int Index = -1;
            if (Key == ROI_ID_MEASURE_LIQUID)
                Index = ROI_INDEX_MEASURE_LIQUID;
            else {
                if (Key == ROI_ID_MEASURE_SPEED)
                    Index = ROI_INDEX_MEASURE_SPEED;
                else {
                    if (Key == ROI_ID_MEASURE_BOTTLE_UNDER_VALVE) {
                        Index = ROI_INDEX_MEASURE_BOTTLE_UNDER_VALVE;
                    } else {
                        if (Key == ROI_ID_CAMERA) {
                            Index = ROI_INDEX_CAMERA;
                        }
                    }
                }
            }
            if (GetKitharaCore() && Index != -1) {
                GetKitharaCore()->SetMeasureWindowPosToRealTime(Index, NewRect.x(), NewRect.y(), NewRect.width(), NewRect.height());
            }
        }
    }
}

void ImageData::CheckIsMeasureWindowInRange(QRect& NewRect, int key)
{
    int MaxH = GetImageHeight();
    int MaxW = GetImageWidth();
    int MaxXpos = MaxW - MINIMUM_ROI_SIZE_IN_PIXEL;
    int MaxYpos = MaxH - MINIMUM_ROI_SIZE_IN_PIXEL;
    int x = NewRect.x();
    int y = NewRect.y();
    int w = NewRect.width();
    int h = NewRect.height();
    int delta = 0;
    bool valueChange = false;
    QString ErrorText;

    if (x < 0) {
        valueChange = true;
        x = 0;
        ErrorText = "X < 0";
    }
    if (x > MaxXpos) {
        valueChange = true;
        x = MaxXpos;
        ErrorText = "X > max";
    }
    if (y < 0) {
        valueChange = true;
        y = 0;
        ErrorText = "y < 0";
    }
    if (y > MaxYpos) {
        valueChange = true;
        y = MaxYpos;
        ErrorText = "y > max";
    }
    if ((x + w) > MaxW) {
        valueChange = true;
        delta = (x + w) - MaxW;
        w = w - delta;
        w--;
        ErrorText = "x + w > width";
    }
    if ((y + h) > MaxH) {
        valueChange = true;
        delta = (y + h) - MaxH;
        h = h - delta;
        h--;
        ErrorText = "y + h > height";
    }
    if (valueChange) {
        QRect CorrectRect(x, y, w, h);
        QString text;
        if (key == ROI_ID_MEASURE_LIQUID) {
            text = ("blue window ");
        } else {
            if (key == ROI_ID_MEASURE_SPEED) {
                text = ("green window ");
            } else {
                if (key == ROI_ID_MEASURE_BOTTLE_UNDER_VALVE) {
                    text = ("yellow window ");
                } else {
                    if (key == ROI_ID_CAMERA) {
                        text = ("Camera ROI ");
                    } else {
                        text = ("Unknown ROI ");
                    }
                }
            }
        }
        NewRect = CorrectRect;
        text = text + ErrorText;
        qDebug() << QString("Rect Value Correctet x:%1 y:%2 w:%3 h%4").arg(x).arg(y).arg(w).arg(h);
        emit SignalShowInfo(QString("Rect %1 Value Correctet x:%2 y:%3 w:%4 h%5").arg(text).arg(x).arg(y).arg(w).arg(h), QtMsgType::QtWarningMsg);
    }
}

QRect ImageData::GetMeasureWindowRect(int Key)
{
    QRect rect;
    ProductData* pProductData = GetMainAppCrystalT2()->GetCurrentProductData();
    if (pProductData) {
        QHash<int, QRect>* pMeasureWindowRects = pProductData->GetMeasureWindowRects();
        if (pMeasureWindowRects) rect = pMeasureWindowRects->value(Key);
    }
    return rect;
}

void ImageData::SetTargetBottleneckDiameter(double set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetTargetBottleneckDiameter(set);
        m_KernelParameter.m_MeasuringParameter.m_BottleneckDiameter = set;
        SaveProductData();
    }
}

void ImageData::SetFormatFromISInmm(double set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetFormatFromISInmm(set);
        m_KernelParameter.m_MeasuringParameter.m_FormatFromISInmm = set;
        SaveProductData();
    }
}

void ImageData::SetProductWidth(double set)
{
    if (GetKitharaCore()) {
        GetKitharaCore()->SetProductWidth(set);
        m_KernelParameter.m_MeasuringParameter.m_ProductWidthInmm = set;
        SaveProductData();
    }
}

double ImageData::GetProductWidth()
{
    return m_KernelParameter.m_MeasuringParameter.m_ProductWidthInmm;
}

double ImageData::GetTargetBottleneckDiameter()
{
    return m_KernelParameter.m_MeasuringParameter.m_BottleneckDiameter;
}

bool ImageData::GetDigitalInput(EtherCATConfigData::IOChannels Channel)
{
    if (GetKitharaCore()) {
        return GetKitharaCore()->GetDigitalInput(Channel);
    } else
        return false;
}

void ImageData::TriggerGetNewVideoFromRealTimeContext()
{
    if (GetKitharaCore()) {
        GetKitharaCore()->TriggerGetNewVideoFromRealTimeContext();
    }
}

int ImageData::GetDebugCounter()
{
    if (GetKitharaCore())
        return GetKitharaCore()->GetDebugCounter();
    else
        return -1;
}

double ImageData::GetProductPresentTime(double SpeedInMMPerSecond)
{
    if (GetKitharaCore())
        return GetKitharaCore()->GetProductPresentTime(SpeedInMMPerSecond);
    else
        return -1.0;
}

bool ImageData::ReadImageFromFile(QString& ErrorMsg)
{
    if (ReadQImage(m_ImageFromFile, ErrorMsg) == 0)
        return true;
    else
        return false;
}

int ImageData::GetCounterNumberBottlesRealEjected()
{
    if (GetKitharaCore())
        return GetKitharaCore()->GetCounterNumberBottlesRealEjected();
    else
        return 0.0;
}

void ImageData::SetMixerOn(bool on)
{
    if (GetKitharaCore()) GetKitharaCore()->SetMixerOn(on);
}

/*void ImageData::SetVideoFlag(int set)
{
        if (GetKitharaCore())
                GetKitharaCore()->SetVideoFlag(set);
}
*/

void ImageData::SetClearVideoBuffer()
{
    if (GetKitharaCore()) GetKitharaCore()->SetClearVideoBuffer();
}

double ImageData::TransformImageXCoorToCartXCoor(double pos)
{
    return (pos - GetImageWidth() * 0.5);
}

double ImageData::TransformImageYCoorToCartYCoor(double pos)
{
    return (GetImageHeight() * 0.5 - pos);
}

double ImageData::TransformCartXCoorToImageXCoor(double xpos)
{
    return (xpos + GetImageWidth() * 0.5);
}

double ImageData::TransformCartYCoorToImageYCoor(double ypos)
{
    return (GetImageHeight() * 0.5) - ypos;
}
