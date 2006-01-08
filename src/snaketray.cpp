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

#include "snaketray.h"

#include <qlabel.h>
#include <qtimer.h>
#include <qfont.h>
#include <klocale.h>

SnakeTray::SnakeTray()
    : KSystemTray( 0, "SnakeTray" ), 
    m_progress( new QLabel( this, "RequestCounter" ) ),
    m_time(new QTime()),
    m_parser(new SnakeParser(this, "Snake Parser")),
    m_received_minutes(-1),
    m_finished(false)
{
	qWarning( "Starting SnakeTray" );
	m_progress->resize( 24,24 );
	QFont small("Helvetica",6);
	m_progress->setFont( small );
	//m_progress->setPaletteForegroundColor( QColor("red") );
	
	connect( m_parser, SIGNAL(timeLeftReceived(int)), this, SLOT(updateTimer(int) ) );
	connect( m_parser, SIGNAL(loginTried()),          this, SLOT(startParsing()) );
	QTimer* timer = new QTimer();
	connect( timer, SIGNAL(timeout()), this, SLOT(tick()) );
	timer->start(1000);
	
	startParsing();
}

SnakeTray::~ SnakeTray()
{
	delete m_time;
}

void SnakeTray::startParsing()
{
	m_parser->startParsing("http://www.snakenetmetalradio.com/heavymetallounge/requests/status2.asp");
}

void SnakeTray::tick() { updateTimer(-1); }

void SnakeTray::updateTimer(int minutes)
{
	qDebug( "updating timer to %i", minutes );
	if( minutes > 0 )
	{
		m_received_minutes = minutes;	
		m_time->start();
	}
	else if( minutes == 0 )
	{
		m_progress->setText("OK");
	}
	else if( (minutes == -1) && (m_received_minutes > 0) )
	{
		// Update countdown
		float remaining = m_received_minutes-(m_time->elapsed()/60000.0);
		int minutes = static_cast<int>(remaining);
		int seconds = (remaining - minutes)*60;
		if( remaining > 0 )
			m_progress->setText( QString::number(minutes) + ":" + QString::number(seconds) );
		else
			m_progress->setText("OK");
	}
}

#include "snaketray.moc"
