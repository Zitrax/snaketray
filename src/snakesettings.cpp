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

#include <qfile.h>
#include <qdir.h>

SnakeSettings::SnakeSettings(QWidget* parent) : 
	Settings(parent)
{
	// Important, settings might be accessed before the dialog is shown
	readSettings();
} 

void SnakeSettings::readSettings()
{
	if(debug()) qDebug("Reading settings");
	QSettings settings(QSettings::Ini);
	settings.setPath("sourceforge.net/projects/snaketray/","snaketray");
	m_debugCB->setChecked(settings.readBoolEntry("/snaketray/debug",false));
	m_secondsCB->setChecked(settings.readBoolEntry("/snaketray/seconds",true));
	m_walletCB->setChecked(settings.readBoolEntry("/snaketray/wallet",true));
	m_update_timer->setValue(settings.readNumEntry("/snaketray/update_timer",5));
	m_updatesCB->setChecked(settings.readBoolEntry("/snaketray/updates",true));
	m_autostartCB->setChecked(getAutostart());
}

void SnakeSettings::writeSettings()
{
	if(debug()) qDebug("Writing settings");
	QSettings settings(QSettings::Ini);
	settings.setPath("sourceforge.net/projects/snaketray/","snaketray");
	settings.writeEntry("/snaketray/debug",debug());
	settings.writeEntry("/snaketray/seconds",seconds());
	settings.writeEntry("/snaketray/wallet",wallet());
	settings.writeEntry("/snaketray/update_timer",updateInterval());
	settings.writeEntry("/snaketray/updates",checkForUpdates());
	setAutostart(m_autostartCB->isChecked());
}

QString SnakeSettings::autostartFile() const
{
	return QDir::homeDirPath() + "/.kde/Autostart/snaketray";
}

bool SnakeSettings::getAutostart() const
{
	if(debug()) qDebug("Autostart file: %s (%d)",autostartFile().latin1(),QFile::exists(autostartFile()));
	return QFile::exists(autostartFile());
}

void SnakeSettings::setAutostart(bool enable)
{
	if( enable && !getAutostart() )
	{
		if(debug()) qDebug("Enabling autostart");
		system("ln -s /usr/bin/snaketray ~/.kde/Autostart/");
	}
	else if( !enable && getAutostart() )
	{
		if(debug()) qDebug("Disabling autostart");
		QFile::remove(autostartFile());
	}
}
