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

#include "snakesettings.h"

#include <qlabel.h>
#include <qtimer.h>
#include <qfont.h>
#include <qpixmap.h>
#include <qtooltip.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kiconloader.h>

SnakeTray::SnakeTray() : KSystemTray( 0, "SnakeTray" ),
      m_progress( new QLabel( this, "RequestCounter" ) ),
      m_time(new QTime()),
      m_parser(new SnakeParser(this, "Snake Parser")),
      m_received_minutes(-1),
      m_ready(false),
      m_parsing(false),
      m_first_parse(true),
      m_size(24)
{
	qWarning( "Starting SnakeTray" );
	
	/*
	kapp->dcopClient()->attach();
	qDebug( "isAttached() = %i", kapp->dcopClient()->isAttached() );
	*/

	m_progress->resize( m_size,m_size );
	
	QPixmap ico("/usr/share/app-install/icons/snakenet.png");
	if( !ico.isNull() )
		contextMenu()->changeTitle(-1, ico, contextMenu()->title());
	
	contextMenu()->insertItem(SmallIconSet("configure"), tr("Settings"), this, SLOT(openSettings()));

	QToolTip::add( m_progress, tr("Left-click to resync the timer with the server.") );
	QToolTip::add( this,       tr("Left-click to resync the timer with the server.") );

	connect( m_parser, SIGNAL(timeLeftReceived(int)), this, SLOT(updateTimer(int) ) );
	connect( m_parser, SIGNAL(loginTried()),          this, SLOT(startParsing()) );
	connect( m_parser, SIGNAL(loginAborted()),        this, SLOT(notLoggedIn()) );
	connect( m_parser, SIGNAL(unknownContent()),      this, SLOT(unknownContent()) );
	
	QTimer* timer = new QTimer();
	connect( timer, SIGNAL(timeout()), this, SLOT(tick()) );
	timer->start(1000);
 
	startParsing();
}

SnakeTray::~ SnakeTray()
{
	delete m_time;
}

void SnakeTray::findFont( const QString& test )
{
	static int last_length = 0;
	
	if( last_length == test.length() )
		return;
	
	last_length = test.length();
	
	QFont small("Helvetica",10);
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
	
	if( m_ready || m_first_parse )
	{
		m_first_parse = false;
		m_parser->removeCookie();
	}
	
	m_ready = false;
	
	QPixmap ico("/usr/share/app-install/icons/connecting.png");
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

	m_parser->startParsing("http://www.snakenetmetalradio.com/heavymetallounge/requests/status.asp");
}

void SnakeTray::tick() { updateTimer(-1); }

void SnakeTray::updateTimer(int minutes)
{
	//qDebug( "updating timer to %i", minutes );
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
			QString rem = QString::number(minutes) + ":" + sec_str;
			findFont( rem );
			m_progress->setText( rem );
			m_progress->show();
		}
		else
			readyToRequest();
	}
}

void SnakeTray::readyToRequest()
{
	m_ready = true;
	QPixmap ico("/usr/share/app-install/icons/snakenet.png");
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

void SnakeTray::notLoggedIn()
{
	m_ready = false;
	QPixmap ico("/usr/share/app-install/icons/snakenet.png");
	if( !ico.isNull() )
	{
		QIconSet is(ico);
		m_progress->hide();
		setPixmap( is.pixmap(QIconSet::Automatic,QIconSet::Disabled) );
	}
	else
	{
		qDebug("Warning - snakenet.png icon not found");
		m_progress->setText(tr("NA"));
		m_progress->show();
	}
}

void SnakeTray::unknownContent()
{
	qDebug("Setting unknown content");
	m_progress->setText(tr("ERR"));
	m_progress->show();
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

void SnakeTray::openSettings()
{
	SnakeSettings ss(this);
	ss.exec();
}

#include "snaketray.moc"
