/***************************************************************************
 *   Copyright (C) 2005-2007 by Daniel Bengtsson                           *
 *   daniel@bengtssons.info                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef _SNAKETRAY_H_
#define _SNAKETRAY_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kactionclasses.h>
#include <ksystemtray.h>
#include <qlabel.h>

#include "snakeparser.h"
#include "snakesettings.h"

/**
 * @short Application Main Window
 * @author Daniel Bengtsson <daniel@bengtssons.info>
 * @version 1.4
 *
 * This is the class that shows up in the tray as a small icon.
 * It will be filled with the remaining minutes that will count
 * down. If 0 minutes are left, 'OK' will be shown.
 *
 * You should only have to create an object of this class to start
 * and things will be handled from there.
 */
class SnakeTray : public KSystemTray
{
	Q_OBJECT
	public:
		virtual ~SnakeTray();

		static bool debug() { return s_instance->_debug(); }
		static SnakeSettings& settings() { return s_instance->m_settings; }
		static SnakeTray* create() 
		{ if(!s_instance) new SnakeTray(); return s_instance; }

		static QString currentVersion() { return "1.4"; }
	
		bool screenSaverOn();
	
	public slots:
		void startParsing();

		void setUpdateTimer(int minutes)
		{ m_update_timer = minutes; }

	protected:
		SnakeTray();	
		void mousePressEvent(QMouseEvent* me);

	private slots:
		void updateTimer(int minutes);
		void readyToRequest();
		void notLoggedIn();
		void tick();	
		void openSettings();
		void unknownContent();
		void about();
		void disable(bool dis);
		void playStream();
 
	private:
	
		// Forbid copying and assignment
		SnakeTray(const SnakeTray&);
		SnakeTray& operator=(const SnakeTray&);
	
		/** 
		 * This function tries to find a suitable 
		 * font for the progress text.
		 * @param test Base the test on this string
        	 */
		void findFont( const QString& test );

		bool _debug() const { return m_settings.debug(); }

		static SnakeTray* s_instance;

		QLabel* m_progress;
		QTime* m_time;
		QTimer* m_resync_timer;
		KToggleAction* m_disable_action;
		bool m_disabled;
		int m_resync_timer_interval;
		SnakeParser* m_parser;
		SnakeSettings m_settings;
		int m_received_minutes;
		bool m_ready; // true if we are ready to request
		bool m_parsing; // currently parsing
		bool m_first_parse; // true until we have started the first parse

		int m_update_timer;

		const int m_size;
};

#endif // _SNAKETRAY_H_
