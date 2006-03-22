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

#include "snakeparser.h"

#include "snakelogindialog.h"

#include <iostream>

#include <qmessagebox.h>
#include <qregexp.h>

#include <kio/scheduler.h>
#include <kapplication.h>
#include <dcopclient.h>

using namespace std;

SnakeParser::SnakeParser(QObject* parent, const char* name) 
	: QObject(parent,name),
	m_login_tried(false),
	m_parsing(false),
	m_user(QString::null),
	m_pass(QString::null),
	m_relogin(false)
{
	qDebug("Creating SnakeParser");
}

SnakeParser::~SnakeParser()
{
	qDebug("Destroying SnakeParser");
	qDebug("%s", m_snakepage.latin1() );
	delete m_snakepage;
}

void SnakeParser::startParsing(const QString& url)
{
	qDebug("SnakeParser::startParsing(%s)", url.latin1());

    if( !m_login_tried && m_parsing )
    {
        qDebug("Already parsing");
        return;
    }

	m_snakepage = QString::null;
    m_parsing = true;

	KIO::SimpleJob* cache_job = KIO::http_update_cache( url, true, 0);
	if( cache_job )
	{
		KIO::Scheduler::doJob(cache_job);
		cache_job->kill();
	}

	KIO::TransferJob* job = KIO::get( url, true, false );
	connect( job,  SIGNAL( result( KIO::Job* ) ), 
	         this, SLOT  ( jobResult( KIO::Job* ) ) );
	connect( job,  SIGNAL( data( KIO::Job*, const QByteArray& ) ), 
	         this, SLOT  ( jobData( KIO::Job*, const QByteArray& ) ) );
}

void SnakeParser::jobResult( KIO::Job* job )
{
	qDebug("Got a job result");
	if ( job->error() )
		job->showErrorDialog();
}

void SnakeParser::jobData( KIO::Job* job, const QByteArray& data )
{
	qDebug("Got job data data.size = %i", data.size() );
	if ( job->error() )
	{
		qDebug("Job data error");
		job->showErrorDialog();
		job->kill();
        m_parsing = false;
	}
	else if( data.size() == 0 )
	{
		//qDebug("%s", m_snakepage.latin1() );
		job->kill();
		if( !m_login_tried )
			parseData();
		else
		{
			m_login_tried = false;
			emit loginTried();
		}
		m_parsing = false;
		return;
	}
	
	QCString str(data,data.size()+1);
	m_snakepage.append(str);
	qDebug("m_snakepage now contains %i chars", m_snakepage.length() );
}

void SnakeParser::parseData()
{
	qDebug("Parsing data...");
	
	QRegExp minutes("<b>([0-9]{1,4})</b> minutes");
	
	if( m_snakepage.contains( "memberlogin.jpg" ) )
	{
		qDebug("It's the login page");
		if( !m_relogin && !m_user.isEmpty() && !m_pass.isEmpty() )
		{
			qDebug("Will try to relogin using old info...");
			m_relogin = true;
			login( m_user, m_pass );
		}
		else
		{
			m_relogin = false; m_pass = QString::null; m_user = QString::null;
			login();
		}
	}
	else if( minutes.search(m_snakepage) != -1 )
	{
		m_relogin = false;
		int min_left = minutes.cap(1).toInt();
		qDebug( "%i minutes left", min_left );
		emit timeLeftReceived(min_left);
	}
	else if( m_snakepage.contains("You may now make a request") )
	{
		m_relogin = false;
		qDebug("You may now make a request");
		emit timeLeftReceived(0);
	}
}

bool SnakeParser::removeCookie()
{
	qDebug("SnakeParser::removeCookie()");
	QByteArray domain;
	QDataStream arg(domain, IO_WriteOnly);
	arg << "http://www.snakenetmetalradio.com/";
	if( !KApplication::dcopClient()->send("kcookiejar","kcookiejar", "deleteCookiesFromDomain(QString)", domain) )
	{
		qDebug("Could not send cookie dcop!");
		return false;
	}
	return true;
}

void SnakeParser::login(const QString& user, const QString& pass)
{
	// When this work we should enable removeCookie()
	// removeCookie();

	m_user = user; m_pass = pass;

	QString login_url("http://www.snakenetmetalradio.com/heavymetallounge/login.asp");
	login_url = login_url + "?email=" + user + "&password=" + pass;
	qDebug("Will login using - " + login_url);
	m_login_tried = true;
	startParsing(login_url);
}

void SnakeParser::login()
{
	SnakeLoginDialog dialog(0);
	connect(&dialog, SIGNAL(login(const QString&, const QString& )),
		this,      SLOT(login(const QString&, const QString& )));
	dialog.exec();
}
