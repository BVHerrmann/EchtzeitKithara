#pragma once

#include <interfaces.h>
#include <plugin.h>
#include <QObject>

// class CoolingDialog;
class TrendGraphOptionPanel;
class SelectTriggerPosOptionPanel;
class OverviewDialog;
class ValveDialog;
// class GeneralDialog;
class TrendGraphWidget;
class MainTabWidget;
class MainAppCrystalT2;
class CleanImageDialog;
// class MaintenanceDialog;
class EjectedBottlesDialog;
class DeactivateAlarmLevelOptionPanel;
class CrystalT2Plugin : public Plugin, MainWindowInterface, PluginInterface, CommunicationInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "de.bertram-bildverarbeitung.CrystalT2Plugin")
    Q_INTERFACES(MainWindowInterface)
    Q_INTERFACES(PluginInterface)
    Q_INTERFACES(CommunicationInterface)
  public:
    explicit CrystalT2Plugin(QObject* parent = 0);
    virtual ~CrystalT2Plugin();

    // PluginInterface
    const QString identifier() const { return "Clear n'Safe"; }
    const QString name() const { return tr("Clear n'Safe"); }
    QThread::Priority priority() const { return QThread::IdlePriority; }
    const QString currentProductName() const { return m_CurrentProductName; }
    void SetMessage(const QString& message, QtMsgType MsgType);
    bool canShowProductWindow() const { return m_EnableShowProductWindow; }
    const MachineState machineState() const { return m_CurrentMaschineState; }

    // MainWindowInterface
    const WidgetType widgetType(const int idx) const;
    int preferredTabIndex(const int idx) const override { return idx + 1; }
    const QString title(const int idx = 0) const;
    QWidget* mainWidget(const int idx = 0) const;
    int requiredWidgetAccessLevel(const int idx = 0) const;
    int mainWidgetCount() const { return m_NumberMainWidgets; }

    OptionPanel* optionPanel(const int idx) const;

    MainAppCrystalT2* GetMainAppCrystalT2() { return m_MainAppCrystalT2; }

    void SetCurrentMaschineState(PluginInterface::MachineState set);
    PluginInterface::MachineState GetCurrentMaschineState() { return m_CurrentMaschineState; }
    void SetCurrentProductName(const QString& set) { m_CurrentProductName = set; }
    void SetEnableShowProductWindow(bool set) { m_EnableShowProductWindow = set; }
    void SetStartupInitReady(bool set);  // { m_StartupInitReady = set; }
    bool IsStartupInitReady() { return m_StartupInitReady; }

    int GetCurrentAccessLevel();
    void ClearAllMessages();
    void SaveAuditTrail(QVariant& fileName);
    void SaveAlarmMessage(QVariant& fileName);

  signals:
    // CommunicationInterface
    void valuesChanged(const QHash<QString, QVariant>& values);
    void valueChanged(const QString& name, const QVariant& value);

  public slots:
    // PluginInterface
    void initialize();
    void uninitialize();
    // void currentMachineState(const PluginInterface::MachineState machineState, const PluginInterface::DiagState diagState);
    void requestMachineState(const PluginInterface::MachineState state);
    void showProductWindow();
    void reset();

    // CommunicationInterface
    void setValue(const QString& name, const QVariant& value);
    void setValues(const QHash<QString, QVariant>& values);

    /*void setValue(const QObject* sender, const QString& name, const QVariant& value)
    {
        if (sender != this) setValue(name, value);
    }
*/
  private:
    PluginInterface::MachineState m_CurrentMaschineState;
    bool m_EnableShowProductWindow;
    bool m_StartupInitReady;
    QString m_CurrentProductName;
    MainAppCrystalT2* m_MainAppCrystalT2;  // Hauptanwendung
    // GeneralDialog* m_GeneralDialog;
    TrendGraphWidget* m_TrendGraphWidget;
    ValveDialog* m_ValveDialog;
    OverviewDialog* m_OverviewDialog;
    MainTabWidget* m_ImageThirdLevelNavigationWidget;  // Anzeigetab fuer das Livebild die Videos und die Haupteinstellungen
    MainTabWidget* m_SettingsThirdLevelNavigationWidget;
    MainTabWidget* m_ParameterDialogThirdLevelNavigationWidget;
    SelectTriggerPosOptionPanel* m_SelectTriggerPosOptionPanel;
    TrendGraphOptionPanel* m_TrendGraphOptionPanel;
    CleanImageDialog* m_CleanImageDialog;
    DeactivateAlarmLevelOptionPanel* m_DeactivateAlarmLevelOptionPanel;
    // MaintenanceDialog* m_MaintenanceDialog;
    // CoolingDialog* m_CoolingDialog;
    // AdminSettingsDialog          *m_AdminSettingsDialog;
    EjectedBottlesDialog* m_EjectedBottlesDialog;
    QList<PluginInterface::Message> m_Messages;
    int m_CodeCounter;
    int m_NumberMainWidgets;
};
