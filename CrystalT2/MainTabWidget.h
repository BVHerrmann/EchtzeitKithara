#pragma once
#include "ThirdLevelNavigationWidget.h"
//#include "interfaces.h"
#include "MainAppCrystalT2.h"


class MainTabWidget : public ThirdLevelNavigationWidget
{
public:
	MainTabWidget(MainAppCrystalT2 *pMainAppCrystalT2, const QString &ObjectName) : ThirdLevelNavigationWidget()
	{
		m_MainAppCrystalT2 = pMainAppCrystalT2;
		//m_AccessLevel      = kAccessLevelGuest;
		setObjectName(ObjectName);
		//if (GetMainAppCrystalT2())
		//	m_AccessLevel = GetMainAppCrystalT2()->GetAccessLevel(ObjectName);
	}
	//int GetAccessLevel() {return m_AccessLevel;}
	//void SetAccessLevel(int set) { m_AccessLevel = set; }
	MainAppCrystalT2 *GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
private:
	//int m_AccessLevel;
	MainAppCrystalT2 *m_MainAppCrystalT2;
};
