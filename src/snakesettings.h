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
#ifndef SNAKESETTINGS_H
#define SNAKESETTINGS_H

#include "settings.h"

#include <qsettings.h>
#include <qcheckbox.h>
#include <qspinbox.h>

/**
 * @author Daniel Bengtsson
 *
 * This is the settings for SnakeTray
 */
class SnakeSettings : public Settings
{
	Q_OBJECT
			
	public:

		SnakeSettings(QWidget* parent);
 
		virtual ~SnakeSettings(){};

		bool debug() const {return m_debugCB->isChecked();}
		bool seconds() const { return m_secondsCB->isChecked();}
		bool wallet() const { return m_walletCB->isChecked(); }
		int updateInterval() const {return m_update_timer->value();}
		bool checkForUpdates() const { return m_updatesCB->isChecked();}
	
	protected slots:

		void accept(){ writeSettings(); Settings::accept();}
	
	private:
		void readSettings();
		void writeSettings();
};

#endif
