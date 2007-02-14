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

#include "snaketray.h"
#include "snakelogindialog.h"

#include <iostream>

#include <qmessagebox.h>
#include <qregexp.h>

#include <kio/scheduler.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kwallet.h>
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
	if(SnakeTray::debug()) qDebug("Creating SnakeParser");
}

SnakeParser::~SnakeParser()
{
	if(SnakeTray::debug()) qDebug("Destroying SnakeParser");
	if(SnakeTray::debug()) qDebug("%s", m_snakepage.latin1() );
	delete m_snakepage;
}

void SnakeParser::startParsing(const QString& url)
{
	if(SnakeTray::debug()) qDebug("SnakeParser::startParsing(%s)", url.latin1());
	
    if( !m_login_tried && m_parsing )
    {
        if(SnakeTray::debug()) qDebug("Already parsing");
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

void SnakeParser::jobError( KIO::Job& job)
{
	if(SnakeTray::debug()) qDebug("Job data error");
	job.showErrorDialog();
	job.kill();
	emit loginAborted();
	m_parsing = false;	
}

void SnakeParser::jobResult( KIO::Job* job )
{
	if(SnakeTray::debug()) qDebug("Got a job result");
	if ( job->error() )
		jobError(*job);
}

void SnakeParser::jobData( KIO::Job* job, const QByteArray& data )
{
	if(!job) return;

	if(SnakeTray::debug()) qDebug("Got job data data.size = %i", data.size() );
	if ( job->error() )
	{
		jobError(*job);
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
	if(SnakeTray::debug()) qDebug("m_snakepage now contains %i chars", m_snakepage.length() );
}

void SnakeParser::parseData()
{
	if(SnakeTray::debug()) qDebug("Parsing data...");
	
	QRegExp minutes("([0-9]{1,4}) minutes");
	
	if( m_snakepage.contains( "memberlogin.jpg" ) )
	{
		if(SnakeTray::debug()) qDebug("It's the login page");
		if( !m_relogin && !m_user.isEmpty() && !m_pass.isEmpty() )
		{
			if(SnakeTray::debug()) qDebug("Will try to relogin using old info...");
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
		if(SnakeTray::debug()) qDebug( "%i minutes left", min_left );
		emit timeLeftReceived(min_left);
	}
	else if( m_snakepage.contains("Less than one minute remaining") )
	{
		m_relogin = false;
		if(SnakeTray::debug()) qDebug("Less than one minute remaining");
		emit timeLeftReceived(1);
	}
	else if( m_snakepage.contains("You may now make a request") )
	{
		m_relogin = false;
		if(SnakeTray::debug()) qDebug("You may now make a request");
		emit timeLeftReceived(0);
	}
	else
	{
		m_relogin = false;
		if(SnakeTray::debug()) qDebug("Unknown content");
		emit unknownContent();
	}
}

bool SnakeParser::removeCookie()
{
	system("dcop kded kcookiejar deleteCookiesFromDomain .snakenetmetalradio.com");

	/*  For some reason this is not working
	qDebug("SnakeParser::removeCookie()");
	QByteArray domain, reply;
	QCString reply_type;
	QDataStream arg(domain, IO_WriteOnly);
	arg << ".snakenetmetalradio.com";
	if( !kapp->dcopClient()->call("kded","kcookiejar", "deleteCookiesFromDomain(QString)", domain, reply_type, reply) )
	{
		qDebug("Could not send cookie dcop!");
		return false;
	}
	*/

	return true;
}

void SnakeParser::login(const QString& user, const QString& pass)
{
	removeCookie();

	m_user = user; 
	m_pass = pass;

	if( SnakeTray::settings().wallet() )
	{
		// Check if we have this entry in the wallet
		KWallet::Wallet* wallet = KWallet::Wallet::openWallet( "kdewallet" );
		if( wallet )
		{
			if( !wallet->hasFolder("SnakeTray") )
				wallet->createFolder("SnakeTray");
			if( wallet->setFolder("SnakeTray") )
			{
				QString wuser, wpass;
				wallet->readPassword("User",wuser);
				wallet->readPassword("Pass",wpass);
				if( (wuser!=user) || (wpass!=pass) )
				{
					if( KMessageBox::questionYesNo(0,"Do you want to save the password in the wallet?","Store password")
						== KMessageBox::Yes )
					{
						wallet->writePassword("User",user);
						wallet->writePassword("Pass",pass);
					}
				} 
			}
		}
	}

	QString login_url("http://www.snakenetmetalradio.com/heavymetallounge/login.asp");
	login_url = login_url + "?email=" + user + "&password=" + pass;
	if(SnakeTray::debug()) qDebug("Will login using - " + login_url);
	m_login_tried = true;
	startParsing(login_url);
}

void SnakeParser::login()
{
	// First check if we have stored passwords in kwallet
	
	KWallet::Wallet* wallet = 0;
	if( SnakeTray::settings().wallet() )
	{
		wallet = KWallet::Wallet::openWallet( "kdewallet" );
	}

	if( wallet )
	{
		if( wallet->setFolder("SnakeTray") )
		{
			QString user, pass;
			wallet->readPassword("User",user);
			wallet->readPassword("Pass",pass);
			if(SnakeTray::debug()) qDebug("Wallet : '" + user + "','" + pass + "'");
			if( !user.isEmpty() && !pass.isEmpty() )
			{
				login(user,pass);
				return;
			}
		}
	}

	SnakeLoginDialog dialog(0);
	connect(&dialog, SIGNAL(login(const QString&, const QString& )),
		this,      SLOT(login(const QString&, const QString& )));
	int ret = dialog.exec();
	if(ret == QDialog::Rejected )
		emit loginAborted();
}
