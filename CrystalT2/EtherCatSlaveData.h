#pragma once
#include "GlobalConst.h"
#include "QtCore"

class EtherCatSlaveData
{
public:
	EtherCatSlaveData()
	{
		m_Position=0;
		m_VendorID=0;
		m_ProductID=0;
		m_NumberChannels=0;
		m_FirstByte = 0;
		m_NumberBitsPerChannel = 0;
	}
	
public:
	QString m_TerminalType;
	QString m_TerminalDisplayName;
	QStringList m_ListChannelDisplayNames;
	//QStringList m_ListAccessNames;
	QHash <QString, int> m_ListChannelNumbers;
	int m_Position;
	int m_VendorID;
	int m_ProductID;
	int m_NumberChannels;
	int m_NumberBitsPerChannel;
	int m_FirstByte;
};