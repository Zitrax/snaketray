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
#include <qpixmap.h>
#include <qtooltip.h>
#include <klocale.h>

SnakeTray::SnakeTray()
    : KSystemTray( 0, "SnakeTray" ),
      m_progress( new QLabel( this, "RequestCounter" ) ),
      m_time(new QTime()),
      m_parser(new SnakeParser(this, "Snake Parser")),
      m_received_minutes(-1),
      m_finished(false),
      m_parsing(false),
      m_size(24)
{
	qWarning( "Starting SnakeTray" );
	m_progress->resize( m_size,m_size );

	findFont();

	QToolTip::add( m_progress, tr("Left-click to resync the timer with the server.") );

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

void SnakeTray::findFont()
{
	QFont small("Helvetica",10);
	QString test("000:00");
	QFontMetrics fm(small);
	while( fm.width(test) > m_size )
	{
        	qDebug("Font: width = %i pointsize = %i",fm.width(test), small.pointSize());
		small.setPointSize(small.pointSize()-1);
		fm = QFontMetrics(small);
		if(small.pointSize() < 4)
			break;
	}
    
	m_progress->setFont( small );
}

void SnakeTray::startParsing()
{
    m_parsing = true;

	QPixmap ico("connecting.png");
    if( !ico.isNull() )
    {
        m_progress->hide();
        setPixmap( ico );
    }
    else
    {
        qDebug("Warning - connecting.png icon not found");
        m_progress->setText(tr("X"));
        m_progress->show();
    }

	m_parser->startParsing("http://www.snakenetmetalradio.com/heavymetallounge/requests/status2.asp");
}

void SnakeTray::tick() { updateTimer(-1); }

void SnakeTray::updateTimer(int minutes)
{
	qDebug( "updating timer to %i", minutes );
	if( minutes > 0 )
	{
        m_parsing = false;
		m_received_minutes = minutes;	
		m_time->start();
	}
	else if( minutes == 0 )
	{
		readyToRequest();
	}
	else if( !m_parsing && (minutes == -1) && (m_received_minutes > 0) )
	{
		// Update countdown
		float remaining = m_received_minutes-(m_time->elapsed()/60000.0);
		int minutes = static_cast<int>(remaining);
		int seconds = (remaining - minutes)*60;
	if( remaining > 0 )
        {
		QString sec_str;
		sec_str.sprintf("%02d",seconds);
		m_progress->setText( QString::number(minutes) + ":" + sec_str );
		m_progress->show();
        }
		else
			readyToRequest();
	}
}

void SnakeTray::readyToRequest()
{
	QPixmap ico("snakenet.png");
    if( !ico.isNull() )
    {
        m_progress->hide();
        setPixmap( ico );
    }
    else
    {
        qDebug("Warning - snakenet.png icon not found");
        m_progress->setText(tr("OK"));
        m_progress->show();
    }
        
}

void SnakeTray::mousePressEvent(QMouseEvent* me)
{
    if( me->button() == Qt::LeftButton )
    {
        qDebug("Parse invoked by mousepress");
        startParsing();
    }
    KSystemTray::mousePressEvent(me);
}

#include "snaketray.moc"
