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

#include "snakeupdatechecker.h"

#include <qlabel.h>
#include <qtimer.h>
#include <qfile.h>
#include <qfont.h>
#include <qpixmap.h>
#include <qregexp.h>
#include <qtooltip.h>
#include <dcopclient.h>
#include <kaboutdialog.h>
#include <kapplication.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kiconloader.h>
#include <khtmlview.h>

SnakeTray* SnakeTray::s_instance = 0;

SnakeTray::SnakeTray() : KSystemTray( 0, "SnakeTray" ),
      m_progress( new QLabel( this, "RequestCounter" ) ),
      m_time(new QTime()),
      m_resync_timer(new QTimer(this)),
      m_disable_action(0),
      m_disabled(false),
      m_settings(this),
      m_received_minutes(-1),
      m_ready(false),
      m_parsing(false),
      m_first_parse(true),
      m_display_queue(false),
      m_queue_window(0),
      m_size(24)
{
	qWarning( "Starting SnakeTray" );
	
	s_instance = this;
	m_parser = new SnakeParser(this, "Snake Parser");
	
	kapp->dcopClient()->attach();
	qDebug( "isAttached() = %i", kapp->dcopClient()->isAttached() );

	m_progress->resize( m_size,m_size );
	m_progress->setAlignment(Qt::AlignHCenter);
	
	QPixmap ico("/usr/share/app-install/icons/snakenet.png");
	if( !ico.isNull() )
		contextMenu()->changeTitle(contextMenu()->idAt(0), ico, "SnakeTray");
	
	contextMenu()->insertItem(SmallIconSet("player_play"), tr("Play stream"), this, SLOT(playStream()));
	contextMenu()->insertItem(SmallIconSet("html"), tr("Display queue"), this, SLOT(displayQueue()));	

	m_disable_action = new KToggleAction( tr("Disable"), 0, 0, 0, contextMenu(), "Disable");
	connect( m_disable_action, SIGNAL( toggled(bool) ), this, SLOT( disable(bool) ));
	m_disable_action->plug(contextMenu());

	contextMenu()->insertItem(SmallIconSet("configure"), tr("Settings"), this, SLOT(openSettings()));
	contextMenu()->insertItem(SmallIconSet("info"), tr("About"), this, SLOT(about()));
	
	QToolTip::add( m_progress, tr("Left-click to resync the timer with the server.") );
	QToolTip::add( this,       tr("Left-click to resync the timer with the server.") );

	connect( m_parser, SIGNAL(timeLeftReceived(int)), this, SLOT(updateTimer(int)) );
	connect( m_parser, SIGNAL(loginTried()),          this, SLOT(loginTried()) );
	connect( m_parser, SIGNAL(loginAborted()),        this, SLOT(notLoggedIn()) );
	connect( m_parser, SIGNAL(unknownContent()),      this, SLOT(unknownContent()) );
	
	connect( m_resync_timer, SIGNAL(timeout()),       this, SLOT(startParsing()) );

	if( m_settings.checkForUpdates() )
	{
		SnakeUpdateChecker* uc = new SnakeUpdateChecker(this,"http://snaketray.sourceforge.net/version.txt", SnakeTray::currentVersion() );
		QTimer::singleShot( 60000, uc, SLOT(check()) );
	}

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

void SnakeTray::loginTried()
{
	if( m_display_queue )
		displayQueue();
	startParsing();
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
	while( (fm.width(test_f)+2) > m_size )
	{
		small.setPointSize(small.pointSize()-1);
		fm = QFontMetrics(small);
		if(debug()) qDebug("Font: width = %i pointsize = %i target=%i",fm.width(test_f), small.pointSize(), m_size);
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
	
	m_disabled = false;
	m_disable_action->setChecked(false);
	
	if( m_ready || m_first_parse )
	{
		m_first_parse = false;
		m_parser->removeCookie();
	}
	
	m_ready = false;
	
	QString anim = "/usr/share/app-install/icons/snakenet_anim.gif";
	QMovie mov(anim);
	if( QFile::exists(anim) && !mov.isNull() )
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
	if( m_disabled )
		return;

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
		int seconds = static_cast<int>((remaining - minutes)*60);
		if( remaining > 0 )
		{
			m_resync_timer->stop();
			QString sec_str;
			sec_str.sprintf("%02d",seconds);
			QString rem;
			if( m_settings.seconds() )
				rem = QString::number(minutes) + ":" + sec_str;
			else
			{
				if( minutes == 0 )
					rem = QString::number(seconds);
				else
					rem = QString::number(minutes);
			}
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
	m_display_queue = false;
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
	m_display_queue = false;
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

void SnakeTray::about()
{
	KAboutDialog about;
	QPixmap logo("/usr/share/app-install/icons/snakenet.png");
	about.setLogo(logo);
	about.setAuthor("Programmed by: Daniel Bengtsson","daniel@bengtssons.info","http://www.bengtssons.info/daniel","");
	about.setVersion("SnakeTray Version " + SnakeTray::currentVersion() + " (2006-2007)");
	about.exec();
}

void SnakeTray::disable(bool dis)
{
	m_disabled = dis;

	if(dis)
		notLoggedIn();
	else
		startParsing();
}

void SnakeTray::playStream()
{
	system("kfmclient exec http://www.snakenetmetalradio.com/snakenet96.pls");
}

void SnakeTray::displayQueueAndLogin(const QString& url)
{
	if(debug()) qDebug("DisplayQueue: url is %s",url.latin1());
	if( url == "http://www.snakenetmetalradio.com/heavymetallounge/login.asp" )
	{
		if( m_queue_window )
		{
			m_queue_window->closeURL();
			m_queue_window->begin();
			m_queue_window->write("<html><body><p>Logging in and loading queue...</p></body></html>");
			m_queue_window->end();
		}
		m_display_queue = true;
		startParsing();
	}
}

void SnakeTray::displayQueue()
{
	if(debug()) qDebug("DisplayQueue");
	m_display_queue = false;
	KURL url = "http://www.snakenetmetalradio.com/heavymetallounge/requests/RequestQueue.asp";
	if( !m_queue_window )
	{
		// This is a guarded pointer so we do not need to care about whether it's being deleted.
 		m_queue_window = new KHTMLPart();
		// Required such that we can click links
		connect(m_queue_window->browserExtension(), SIGNAL(openURLRequest(const KURL &,const KParts::URLArgs &)),
			m_queue_window,                     SLOT(openURL(const KURL &)));
		connect(m_queue_window->browserExtension(), SIGNAL(setLocationBarURL(const QString&)),
			this,                               SLOT(displayQueueAndLogin(const QString&)));
	}
	m_queue_window->begin();
	m_queue_window->write("<html><body><p>Loading Queue...</p></body></html>");
	m_queue_window->end();
	m_queue_window->view()->resize(500, 400);
	m_queue_window->show();
	m_queue_window->openURL(url);
}

#include "snaketray.moc"
