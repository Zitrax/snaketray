//
// C++ Implementation: snakeupdatechecker
//
// Description: 
//
//
// Author: Daniel Bengtsson <daniel@bengtssons.info>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "snakeupdatechecker.h"

#include "snaketray.h"
#include <kmessagebox.h>
//#include <kio/scheduler.h>

SnakeUpdateChecker::SnakeUpdateChecker(QObject* parent, const QString& updateURL, const QString& currentVersion)
: QObject(parent), m_update_url(updateURL), m_current_version(currentVersion), m_page(QString::null)
{
	if(SnakeTray::debug()) qDebug("Creating SnakeUpdateChecker");
}

void SnakeUpdateChecker::check()
{
	if(SnakeTray::debug()) qDebug("Starting SnakeUpdateChecker on '" + m_update_url + "'");
	m_page = QString::null;
	KIO::TransferJob* job = KIO::get( m_update_url, true, false );
	connect( job,  SIGNAL( result( KIO::Job* ) ), 
	         this, SLOT  ( jobResult( KIO::Job* ) ) );
	connect( job,  SIGNAL( data( KIO::Job*, const QByteArray& ) ), 
	         this, SLOT  ( jobData( KIO::Job*, const QByteArray& ) ) );
}

void SnakeUpdateChecker::jobResult( KIO::Job* job )
{
}

void SnakeUpdateChecker::findVersion()
{
if(SnakeTray::debug()) qDebug("Latest version is '" + m_page + "' INT: " + QString::number(m_page.toFloat()) + ">" + QString::number(m_current_version.toFloat()));
 
	if( m_page.isEmpty() )
		return;
	
	if( m_page.toFloat() > m_current_version.toFloat() )
	{
		if(SnakeTray::debug()) qDebug("New version available!");
		emit updateAvailable( m_page );
		KMessageBox::information(0,"There is a new version available, you can get it at http://snaketray.sourceforge.net/ !", "New version available"/*, "Updates"*/ );
	}
	else
	{
		if(SnakeTray::debug()) qDebug("You are using an up to date version");
	}
		
}

void SnakeUpdateChecker::jobData( KIO::Job* job, const QByteArray& data )
{
	if(!job) return;
	
	if ( job->error() )
	{
		//jobError(*job);
	}
	else if( data.size() == 0 )
	{
		//qDebug("%s", m_snakepage.latin1() );
		job->kill();
		findVersion();
		return;
	}
	
	QCString str(data,data.size()+1);
	m_page.append(str);
}
