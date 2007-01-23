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

#include "snaketray.h"

#include <qlabel.h>
#include <qtimer.h>
#include <qfont.h>
#include <qpixmap.h>
#include <qregexp.h>
#include <qtooltip.h>
#include <dcopclient.h>
#include <kapplication.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kiconloader.h>

SnakeTray* SnakeTray::s_instance = 0;

SnakeTray::SnakeTray() : KSystemTray( 0, "SnakeTray" ),
      m_progress( new QLabel( this, "RequestCounter" ) ),
      m_time(new QTime()),
      m_resync_timer(new QTimer(this)),
      m_settings(this),
      m_received_minutes(-1),
      m_ready(false),
      m_parsing(false),
      m_first_parse(true),
      m_size(24)
{
	qWarning( "Starting SnakeTray" );
	
	s_instance = this;
	m_parser = new SnakeParser(this, "Snake Parser");
	
	kapp->dcopClient()->attach();
	qDebug( "isAttached() = %i", kapp->dcopClient()->isAttached() );

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
	
	connect( m_resync_timer, SIGNAL(timeout()),       this, SLOT(startParsing()) );

	QTimer* timer = new QTimer();
	connect( timer, SIGNAL(timeout()), this, SLOT(tick()) );
	timer->start(1000);
 
	startParsing();
}

SnakeTray::~ SnakeTray()
{
	s_instance = 0;
	delete m_time;
}

void SnakeTray::findFont( const QString& test )
{
	static unsigned int last_length = 0;
	
	if( last_length == test.length() )
		return;
	
	last_length = test.length();
	
	QString test_f = test;
	QRegExp re("[0-9]");
	test_f.replace(re,"8");
	
	QFont small("Helvetica",10);
	QFontMetrics fm(small);
	while( fm.width(test_f) > m_size )
	{
		if(debug()) qDebug("Font: width = %i pointsize = %i",fm.width(test_f), small.pointSize());
		small.setPointSize(small.pointSize()-1);
		fm = QFontMetrics(small);
		if(small.pointSize() < 4)
			break;
	}
    
	m_progress->setFont( small );
}

void SnakeTray::startParsing()
{
	if(debug()) qDebug("Start Parsing");

	// Don't bother if the screensaver is on
	if(screenSaverOn())
		return;
	
	m_parsing = true;
	
	if( m_ready || m_first_parse )
	{
		m_first_parse = false;
		m_parser->removeCookie();
	}
	
	m_ready = false;
	
	QMovie mov("/usr/share/app-install/icons/snakenet_anim.gif");
	if( !mov.isNull() )
	{
		m_progress->hide();
		setMovie( mov );
	}
	else
	{
		if(debug()) qDebug("Warning - snakenet_anim.gif icon not found");
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
			m_resync_timer->stop();
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
		if(debug()) qDebug("Warning - snakenet.png icon not found");
		m_progress->setText(tr("OK"));
		m_progress->show();
	}
	if( !m_resync_timer->isActive() )
	{
		if(debug()) 
			qDebug("Starting timer with %i min interval.",m_settings.updateInterval());
		m_resync_timer_interval = m_settings.updateInterval();
		m_resync_timer->start(m_resync_timer_interval*60000);
	}
	else if(m_settings.updateInterval()!=m_resync_timer_interval)
	{
		if(debug()) qDebug("Changing interval from %i to %i",
		                   m_resync_timer_interval,
		                   m_settings.updateInterval());
		m_resync_timer->changeInterval(m_settings.updateInterval()*60000);
		m_resync_timer_interval = m_settings.updateInterval();
	}
}

void SnakeTray::notLoggedIn()
{
	m_ready = false;
	m_resync_timer->stop();
	QPixmap ico("/usr/share/app-install/icons/snakenet.png");
	if( !ico.isNull() )
	{
		QIconSet is(ico);
		m_progress->hide();
		setPixmap( is.pixmap(QIconSet::Automatic,QIconSet::Disabled) );
	}
	else
	{
		if(debug()) qDebug("Warning - snakenet.png icon not found");
		m_progress->setText(tr("NA"));
		m_progress->show();
	}
}

void SnakeTray::unknownContent()
{
	if(debug()) qDebug("Setting unknown content");
	m_progress->setText(tr("ERR"));
	m_progress->show();
}

void SnakeTray::mousePressEvent(QMouseEvent* me)
{
	if( me->button() == Qt::LeftButton )
	{
		if(debug()) qDebug("Parse invoked by mousepress");
		startParsing();
	}
	KSystemTray::mousePressEvent(me);
}

void SnakeTray::openSettings()
{
	m_settings.exec();
}

bool SnakeTray::screenSaverOn()
{
	QByteArray data,returnValue;
	QCString returnType;
	if (!kapp->dcopClient()->call("kdesktop","KScreensaverIface","isBlanked()",
	                              data,returnType,returnValue,true))
	{
		if(debug()) qDebug("Check for screensaver failed.");
	}
	else if (returnType == "bool")
	{
		bool b;
		QDataStream reply(returnValue,IO_ReadOnly);
		reply >> b;
		if(debug()) qDebug("Screensaver is %i",b);
		return b;
	}
	return false;
}

#include "snaketray.moc"
