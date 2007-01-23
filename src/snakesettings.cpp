//
// C++ Interface: snakesettings
//
// Description: 
//
//
// Author: Daniel Bengtsson <daniel@bengtssons.info>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "snakesettings.h"

SnakeSettings::SnakeSettings(QWidget* parent) : 
	Settings(parent)
{
	readSettings();
} 

void SnakeSettings::readSettings()
{
	QSettings settings(QSettings::Ini);
	settings.setPath("sourceforge.net/projects/snaketray/","snaketray");
	m_debugCB->setChecked(settings.readBoolEntry("/snaketray/debug",false));
	m_update_timer->setValue(settings.readNumEntry("/snaketray/update_timer",5));
}

void SnakeSettings::writeSettings()
{
	QSettings settings(QSettings::Ini);
	settings.setPath("sourceforge.net/projects/snaketray/","snaketray");
	settings.writeEntry("/snaketray/debug",debug());
	settings.writeEntry("/snaketray/update_timer",updateInterval());
}