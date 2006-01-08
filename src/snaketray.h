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
 */
class SnakeTray : public KSystemTray
{
    Q_OBJECT
public:
    SnakeTray();
    virtual ~SnakeTray();

public slots:
	void startParsing();

    
private slots:
	void updateTimer(int minutes);
	void tick();	
    
private:
	QLabel* m_progress;
	QTime* m_time;
	SnakeParser* m_parser;
	int m_received_minutes;
	bool m_finished;
};

#endif // _SNAKETRAY_H_