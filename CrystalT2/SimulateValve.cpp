#include "SimulateValve.h"
#include "ImageData.h"
#include "MainAppCrystalT2.h"
#include "SettingsData.h"
#include "qprocess.h"

SimulateValve::SimulateValve(MainAppCrystalT2* pMainAppCrystalT2)
    : QObject(), m_ClientValveLeft(NULL), m_ClientValveRight(NULL), m_ValveLeftProcess(NULL), m_ValveRightProcess(NULL), m_counterLeft(0), m_counterRight(0)
{
    m_MainAppCrystalT2 = pMainAppCrystalT2;
    m_ValveLeftName = "ValveLeft";
    m_ValveRightName = "ValveRight";
    m_TimerConnectToServerLeft = new QTimer(this);
    m_TimerConnectToServerRight = new QTimer(this);

    connect(m_TimerConnectToServerLeft, &QTimer::timeout, this, &SimulateValve::SlotTryConnectToServerLeft);
    connect(m_TimerConnectToServerRight, &QTimer::timeout, this, &SimulateValve::SlotTryConnectToServerRight);

    StartProcess();
}

SimulateValve::~SimulateValve()
{
    KillProcess();
}

void SimulateValve::StartProcess()
{
    QString ProgramValveLeft = "c://Bertram//NordsonVentilSimu//NordsonVentilSimulator1.exe";
    QStringList ArgumentsValveLeft;
    QString ProgramValveRight = "c://Bertram//NordsonVentilSimu//NordsonVentilSimulator2.exe";
    QStringList ArgumentsValveRight;

    if (m_ValveLeftProcess) {
        delete m_ValveLeftProcess;
        m_ValveLeftProcess = NULL;
    }
    if (m_ValveRightProcess) {
        delete m_ValveRightProcess;
        m_ValveRightProcess = NULL;
    }

    if (m_ValveLeftProcess == NULL) m_ValveLeftProcess = new QProcess();
    if (m_ValveRightProcess == NULL) m_ValveRightProcess = new QProcess();

    if (m_ClientValveLeft == NULL) m_ClientValveLeft = new QLocalSocket(this);
    connect(m_ClientValveLeft, SIGNAL(connected()), this, SLOT(SlotConnectedValveLeft()));
    if (m_ClientValveRight == NULL) m_ClientValveRight = new QLocalSocket(this);
    connect(m_ClientValveLeft, SIGNAL(connected()), this, SLOT(SlotConnectedValveRight()));

    connect(m_ValveLeftProcess, &QProcess::started, this, &SimulateValve::SlotProcessValveLeftIsRunning);
    connect(m_ValveRightProcess, &QProcess::started, this, &SimulateValve::SlotProcessValveRightIsRunning);


    QString PortNameValveLeft = "COM3";
    QString PortNameValveRight = "COM4";
    if (GetMainAppCrystalT2() && GetMainAppCrystalT2()->GetSettingsData()) {
        PortNameValveLeft = GetMainAppCrystalT2()->GetSettingsData()->m_ValveControllerPortName1;
        PortNameValveRight = GetMainAppCrystalT2()->GetSettingsData()->m_ValveControllerPortName2;
    }
    ArgumentsValveLeft << PortNameValveLeft << m_ValveLeftName;
    ArgumentsValveRight << PortNameValveRight << m_ValveRightName;

    m_ValveLeftProcess->start(ProgramValveLeft, ArgumentsValveLeft);
    if (m_MainAppCrystalT2->GetSettingsData()->m_WorkWithTwoValves) m_ValveRightProcess->start(ProgramValveRight, ArgumentsValveRight);
}

bool SimulateValve::WaitForStarted()
{
    bool rv = false;
    if (m_ValveLeftProcess) rv = m_ValveLeftProcess->waitForStarted();
    if (m_ValveRightProcess) rv = m_ValveRightProcess->waitForStarted();
    return rv;
}

void SimulateValve::KillProcess()
{
    if (m_TimerConnectToServerLeft) m_TimerConnectToServerLeft->stop();
    if (m_TimerConnectToServerRight) m_TimerConnectToServerRight->stop();
    Disconnect();
    if (m_ValveLeftProcess) {
        m_ValveLeftProcess->terminate();
    }
    if (m_ValveRightProcess) {
        m_ValveRightProcess->terminate();
    }
}

void SimulateValve::Disconnect()
{
    if (m_ValveLeftProcess && (m_ValveLeftProcess->state() == QProcess::Running)) {
        m_ClientValveLeft->disconnectFromServer();
    }
    if (m_ValveRightProcess && (m_ValveRightProcess->state() == QProcess::Running)) {
        m_ClientValveRight->disconnectFromServer();
    }
}

void SimulateValve::SlotTryConnectToServerLeft()
{
    if (m_ClientValveLeft) {
        if (m_ClientValveLeft->state() == QLocalSocket::ConnectedState) {
            m_TimerConnectToServerLeft->stop();
        } else {
            m_ClientValveLeft->connectToServer(m_ValveLeftName);
        }
    } else {
        if (m_TimerConnectToServerLeft) {
            m_TimerConnectToServerLeft->stop();
        }
    }
}

void SimulateValve::SlotTryConnectToServerRight()
{
    if (m_ClientValveRight) {
        if (m_ClientValveRight->state() == QLocalSocket::ConnectedState) {
            m_TimerConnectToServerRight->stop();
        } else {
            m_ClientValveRight->connectToServer(m_ValveRightName);
        }
    } else {
        if (m_TimerConnectToServerRight) {
            m_TimerConnectToServerRight->stop();
        }
    }
}

void SimulateValve::SlotProcessValveLeftIsRunning()
{
    m_TimerConnectToServerLeft->setInterval(500);
    m_TimerConnectToServerLeft->start();
    /* int count = 0;
     do {
         if (m_Valve1Process->state() == QProcess::Running) {
             Sleep(500);
             m_ClientValve1->connectToServer(m_Valve1Name);
             break;
         } else {
             Sleep(100);
             count++;
         }
     } while (count < 50);
     */
}

void SimulateValve::SlotProcessValveRightIsRunning()
{
    m_TimerConnectToServerRight->setInterval(500);
    m_TimerConnectToServerRight->start();
    /*int count = 0;
    do {
        if (m_Valve1Process->state() == QProcess::Running) {
            Sleep(500);
            m_ClientValve2->connectToServer(m_Valve2Name);
            break;
        } else {
            Sleep(100);
            count++;
        }
    } while (count < 50);
    */
}

void SimulateValve::SetValveLeftName()
{
    QString IsFirst = "LeftValve";
    SettingsData* pSettingsData = m_MainAppCrystalT2->GetSettingsData();
    if (pSettingsData && m_ValveLeftConnected) {
        /*if (!pSettingsData->m_FirstPortIsFirst) {
            IsFirst = "SecondValve";
        }*/
        m_ClientValveLeft->write(IsFirst.toLatin1());
        m_ClientValveLeft->flush();
    }
}

void SimulateValve::SetValveRightName()
{
    QString IsFirst = "RightValve";
    SettingsData* pSettingsData = m_MainAppCrystalT2->GetSettingsData();
    if (pSettingsData && m_ValveRightConnected) {
        /*if (pSettingsData->m_FirstPortIsFirst) {
            IsFirst = "SecondValve";
        }*/
        m_ClientValveRight->write(IsFirst.toLatin1());
        m_ClientValveRight->flush();
    }
}

void SimulateValve::SlotConnectedValveLeft()
{
    m_ValveLeftConnected = true;
    SetValveLeftName();
}

void SimulateValve::SlotConnectedValveRight()
{
    m_ValveRightConnected = true;
    SetValveRightName();
}

void SimulateValve::LeftTriggerOn()
{
    if (m_ValveLeftConnected) {
        QString data = QString("TriggerLeft On");
        if (m_ValveLeftConnected) {
            m_ClientValveLeft->write(data.toLatin1());
            m_ClientValveLeft->flush();
        }
    }
}

void SimulateValve::RightTriggerOn()
{
    if (m_ValveRightConnected) {
        QString data = QString("TriggerRight On");
        if (m_ValveRightConnected) {
            m_ClientValveRight->write(data.toLatin1());
            m_ClientValveRight->flush();
        }
    }
}

void SimulateValve::SlotLeftTriggerOff()
{
    if (m_ValveLeftConnected) {
        QString data = QString("TriggerLeft Off");
        if (m_ValveLeftConnected) {
            m_ClientValveLeft->write(data.toLatin1());
            m_ClientValveLeft->flush();
        }
    }
}

void SimulateValve::SlotRightTriggerOff()
{
    if (m_ValveRightConnected) {
        QString data = QString("TriggerRight Off");
        if (m_ValveRightConnected) {
            m_ClientValveRight->write(data.toLatin1());
            m_ClientValveRight->flush();
        }
    }
}

void SimulateValve::StartResetTriggerLeft()
{
    QTimer::singleShot(100, this, &SimulateValve::SlotLeftTriggerOff);
}

void SimulateValve::StartResetTriggerRight()
{
    QTimer::singleShot(100, this, &SimulateValve::SlotRightTriggerOff);
}
