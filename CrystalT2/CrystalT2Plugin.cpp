#include "CrystalT2Plugin.h"
#include <interfaces.h>
#include <plugin.h>
#include <qdebug.h>
#include <QtCore>
#include <QtWidgets>
#include "AdminSettingsDialog.h"
#include "DeactivateAlarmLevelOptionPanel.h"
#include "GeneralDialog.h"
#include "GlobalConst.h"
#include "MainAppCrystalT2.h"
#include "MainTabWidget.h"
#include "OverviewDialog.h"
#include "ValveDialog.h"

CrystalT2Plugin::CrystalT2Plugin(QObject* parent)
    : Plugin(parent),
      m_CurrentMaschineState(MachineState::Starting),
      m_MainAppCrystalT2(NULL),
      m_ImageThirdLevelNavigationWidget(NULL),
      m_SettingsThirdLevelNavigationWidget(NULL),
      m_ParameterDialogThirdLevelNavigationWidget(NULL),
      m_SelectTriggerPosOptionPanel(NULL),
      m_TrendGraphWidget(NULL),
      m_ValveDialog(NULL),
      m_CleanImageDialog(NULL),
      m_EnableShowProductWindow(true),
      m_CodeCounter(1),
      m_StartupInitReady(false),
      m_NumberMainWidgets(0),
      m_TrendGraphOptionPanel(NULL),
      m_EjectedBottlesDialog(NULL),
      m_DeactivateAlarmLevelOptionPanel(NULL)
{
    m_MainAppCrystalT2 = new MainAppCrystalT2(this);
    if (m_MainAppCrystalT2) {
        m_OverviewDialog = GetMainAppCrystalT2()->GetOverviewDialog();
        m_ImageThirdLevelNavigationWidget = GetMainAppCrystalT2()->GetImageThirdLevelNavigationWidget();
        m_SettingsThirdLevelNavigationWidget = GetMainAppCrystalT2()->GetSettingsThirdLevelNavigationWidget();
        m_ParameterDialogThirdLevelNavigationWidget = GetMainAppCrystalT2()->GetParameterDialogThirdLevelNavigationWidget();
        m_SelectTriggerPosOptionPanel = GetMainAppCrystalT2()->GetSelectTriggerPosOptionPanel();
        m_TrendGraphWidget = GetMainAppCrystalT2()->GetTrendGraphWidget();
        m_TrendGraphOptionPanel = GetMainAppCrystalT2()->GetTrendGraphOptionPanel();
        m_CleanImageDialog = GetMainAppCrystalT2()->GetCleanImageDialog();
        m_EjectedBottlesDialog = GetMainAppCrystalT2()->GetEjectedBottlesDialog();
        m_DeactivateAlarmLevelOptionPanel = GetMainAppCrystalT2()->GetDeactivateAlarmLevelOptionPanel();
    }

    QSettings settings;
    settings.setValue("Inspector/EnableLoginWithUsername", true);
}

CrystalT2Plugin::~CrystalT2Plugin()
{
    if (m_MainAppCrystalT2) {
        delete m_MainAppCrystalT2;
        m_MainAppCrystalT2 = NULL;
    }
}

void CrystalT2Plugin::SaveAuditTrail(QVariant& fileName)
{
    QString Name = "AuditTrail/store";
    emit valueChanged(Name, fileName);
}

void CrystalT2Plugin::SaveAlarmMessage(QVariant& fileName)
{
    QString Name = "Inspector/AlarmMessages/store";
    emit valueChanged(Name, fileName);
}

int CrystalT2Plugin::GetCurrentAccessLevel()
{
    return currentAccessLevel();
}

void CrystalT2Plugin::SetStartupInitReady(bool set)
{
    m_StartupInitReady = set;
    m_NumberMainWidgets = 7;
}

void CrystalT2Plugin::SetCurrentMaschineState(PluginInterface::MachineState set)
{
    m_CurrentMaschineState = set;
    if (set == PluginInterface::MachineState::Production)
        SetEnableShowProductWindow(false);
    else
        SetEnableShowProductWindow(true);
}

void CrystalT2Plugin::showProductWindow()
{
    if (GetMainAppCrystalT2()) GetMainAppCrystalT2()->OpenProductDialog();
}
// Anzeigen unterschiedlicher Fehlertexte enum QtMsgType { QtDebugMsg, QtWarningMsg, QtCriticalMsg, QtFatalMsg, QtInfoMsg, QtSystemMsg = QtCriticalMsg };
void CrystalT2Plugin::SetMessage(const QString& message, QtMsgType MsgType)
{
    bool found = false;
    for (int i = 0; i < m_Messages.count(); i++) {
        if (message == m_Messages.at(i).message) {
            found = true;
            break;
        }
    }
    if (!found) {
        m_Messages << PluginInterface::Message(m_CodeCounter, message, MsgType);
        PluginInterface::updateMessages(m_Messages);
        m_CodeCounter++;
    }
    if (m_CodeCounter >= 500) {
        ClearAllMessages();
    }
}

void CrystalT2Plugin::ClearAllMessages()
{
    clearMessages();
    m_Messages.clear();
    m_CodeCounter = 0;
}

// Entfernt alle Textnachrichen aus dem Fenster
void CrystalT2Plugin::reset()
{
    ClearAllMessages();
    if (GetMainAppCrystalT2()) {
        GetMainAppCrystalT2()->ResetIsClicked();
    }
}

// Wird von der obergeordnetet Instanz aufgerufen
QWidget* CrystalT2Plugin::mainWidget(const int idx) const
{
    switch (idx) {
        case OVERVIEW_DIALOG_INDEX:
            if (m_StartupInitReady)
                return (QWidget*)m_OverviewDialog;  // m_MainWidget;//enthaelt den OverviewDialog
            else
                return NULL;
            break;
        case IMAGEVIEW_DIALOG_INDEX:
            if (m_StartupInitReady)
                return (QWidget*)m_ImageThirdLevelNavigationWidget;
            else
                return NULL;
            break;
        case PARAMETER_DIALOG_INDEX:
            if (m_StartupInitReady)
                return (QWidget*)m_ParameterDialogThirdLevelNavigationWidget;
            else
                return NULL;
            break;
        case SETTINGS_DIALOG_INDEX:
            if (m_StartupInitReady)
                return (QWidget*)m_SettingsThirdLevelNavigationWidget;
            else
                return NULL;
            break;
        case TREND_GRAPH_DIALOG_INDEX:
            if (m_StartupInitReady)
                return (QWidget*)m_TrendGraphWidget;
            else
                return NULL;
            break;
        case CLEAN_IMAGE_DIALOG_INDEX:
            if (m_StartupInitReady)
                return (QWidget*)m_CleanImageDialog;
            else
                return NULL;
            break;
        case EJECTED_BOTTLES_DIALOG_INDEX:
            if (m_StartupInitReady)
                return (QWidget*)m_EjectedBottlesDialog;
            else
                return NULL;
            break;
        default:
            return NULL;
            break;
    }
}

// MainWindowInterface
const MainWindowInterface::WidgetType CrystalT2Plugin::widgetType(const int idx) const
{
    if (SETTINGS_DIALOG_INDEX == idx)
        return Settings;
    else
        return Application;
}

OptionPanel* CrystalT2Plugin::optionPanel(const int idx) const
{
    switch (idx) {
        case OVERVIEW_DIALOG_INDEX: {
            if (m_StartupInitReady)
                return (OptionPanel*)(m_SelectTriggerPosOptionPanel);
            else
                return nullptr;
            break;
        }
        case TREND_GRAPH_DIALOG_INDEX: {
            if (m_StartupInitReady)
                return (OptionPanel*)(m_TrendGraphOptionPanel);
            else
                return nullptr;
            break;
        }
        case SETTINGS_DIALOG_INDEX: {
            if (m_StartupInitReady)
                return (OptionPanel*)(m_DeactivateAlarmLevelOptionPanel);
            else
                return nullptr;
            break;
        }

        default:
            return nullptr;
    }
}

// regelt den Zugriffslevel für die verschiedenen GUI-Widgets
int CrystalT2Plugin::requiredWidgetAccessLevel(const int idx) const
{
    int rv = kAccessLevelGuest;
    switch (idx) {
        case OVERVIEW_DIALOG_INDEX:
            rv = kAccessLevelGuest;
            break;
        case IMAGEVIEW_DIALOG_INDEX:
            if (m_ImageThirdLevelNavigationWidget) rv = kAccessLevelGuest;
            break;
        case PARAMETER_DIALOG_INDEX:
            if (m_ParameterDialogThirdLevelNavigationWidget) rv = kAccessLevelGuest;
            break;
        case SETTINGS_DIALOG_INDEX:
            if (m_SettingsThirdLevelNavigationWidget) rv = kAccessLevelAdmin;
            break;
        default:
            rv = kAccessLevelGuest;
            break;
    }
    return rv;
}

// Legt den Titel für die verschiedenen GUI-Elemnte fest
const QString CrystalT2Plugin::title(const int idx) const
{
    QString Name;
    switch (idx) {
        case OVERVIEW_DIALOG_INDEX:
            Name = tr("Overview");
            break;
        case IMAGEVIEW_DIALOG_INDEX:
            Name = tr("Image Settings");
            break;
        case PARAMETER_DIALOG_INDEX:
            Name = tr("Parameter");
            break;
        case SETTINGS_DIALOG_INDEX:
            Name = name();
            break;
        case TREND_GRAPH_DIALOG_INDEX:
            Name = tr("Trend Graph");
            break;
        case CLEAN_IMAGE_DIALOG_INDEX:
            Name = tr("Cleaner");
            break;
        case EJECTED_BOTTLES_DIALOG_INDEX:
            Name = tr("Ejected Bottles");
            break;
        default:
            Name = tr("NoName");
            break;
    }
    return Name;
}

void CrystalT2Plugin::initialize()
{
    if (GetMainAppCrystalT2()) GetMainAppCrystalT2()->Initialize();
}

void CrystalT2Plugin::uninitialize()
{
}

// wird aufgerufen wenn im Dialog der Zustand gesetzt wird
void CrystalT2Plugin::requestMachineState(const PluginInterface::MachineState machineState)
{
    if (machineState == PluginInterface::MachineState::Terminate) {
        ClearAllMessages();
        if (GetMainAppCrystalT2()) GetMainAppCrystalT2()->FinishedRealTimeSystem();
        thread()->msleep(4000);
    }
    if (GetMainAppCrystalT2()) GetMainAppCrystalT2()->SetCurrentMaschineState(machineState);
}

void CrystalT2Plugin::setValue(const QString& name, const QVariant& value)
{
#ifdef DEBUG
    qDebug() << this << "ignored setValue" << name << value;
#else
    Q_UNUSED(value);
#endif
}

void CrystalT2Plugin::setValues(const QHash<QString, QVariant>& values)
{
#ifdef DEBUG
    qDebug() << this << "ignored setValues" << values;
#else
    Q_UNUSED(values);
#endif
}
