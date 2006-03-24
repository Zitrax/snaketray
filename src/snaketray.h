/***************************************************************************
 *   Copyright (C) 2005 by Daniel Bengtsson                                *
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

#include <ksystemtray.h>
#include <qlabel.h>

#include "snakeparser.h"

/**
 * @short Application Main Window
 * @author Daniel Bengtsson <daniel@bengtssons.info>
 * @version 0.1
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
		SnakeTray();
		virtual ~SnakeTray();

	public slots:
		void startParsing();

		void setUpdateTimer(int minutes)
		{ m_update_timer = minutes; }

	protected:

		void mousePressEvent(QMouseEvent* me);
    
	private slots:
		void updateTimer(int minutes);
		void readyToRequest();
		void notLoggedIn();
		void tick();	
		void openSettings();
    
	private:
		/** 
		 * This function tries to find a suitable 
		 * font for the progress text.
        	 */
		void findFont();

		QLabel* m_progress;
		QTime* m_time;
		SnakeParser* m_parser;
		int m_received_minutes;
		bool m_ready; // true if we are ready to request
		bool m_parsing; // currently parsing
		bool m_first_parse; // true until we have started the first parse

		int m_update_timer;

		const int m_size;
};

#endif // _SNAKETRAY_H_
