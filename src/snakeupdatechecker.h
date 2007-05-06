//
// C++ Interface: snakeupdatechecker
//
// Description: 
//
//
// Author: Daniel Bengtsson <daniel@bengtssons.info>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef SNAKEUPDATECHECKER_H
#define SNAKEUPDATECHECKER_H

#include <qobject.h>
#include <kio/job.h>

/**
 * This class checks if there is a new version of the program available
 * by looking up the url provided in the constructor.
 *
 * You simply create an object of the class and call check, the default
 * implementation gives you a dialog, but you can also listen to the
 * signal updateAvailable().
 */
class SnakeUpdateChecker : public QObject
{
	Q_OBJECT
public:
	SnakeUpdateChecker(QObject* parent, const QString& updateURL, const QString& currentVersion);
	
public slots:
	void check();
	
signals:
	void updateAvailable(const QString& version);
	
private slots:
	
       /**
        * Will check for errors of the job
        */
	void jobResult( KIO::Job* job );
	
       /**
        * Chunks of data arrive in this slot. When data->size() == 0
	* we are done and can start parse the data using
	* parseData()
	*/
	void jobData( KIO::Job* job, const QByteArray& data );
	
private:
	void findVersion();
	
	QString m_update_url;
	QString m_current_version;
	QString m_page;
};

#endif
