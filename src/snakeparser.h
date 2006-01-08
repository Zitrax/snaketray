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

#ifndef SNAKEPARSER_H
#define SNAKEPARSER_H

#include <qobject.h>
 
#include <kio/job.h>

/**
 * This class will handle the retrieving of the requestpage
 * and login if necessary.
 */
class SnakeParser : public QObject
{
Q_OBJECT
public:
	SnakeParser(QObject* parent, const char* name);
	~SnakeParser();
	
	/**
	 * Will start retrieving the url
	 * The slots jobResult and jobData will be called
	 * during the transfer.
	 * @param url The url to retrieve
	 */
	void startParsing(const QString& url);
	
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
	/**
	 * Will try to login to snakenet using...
	 * @param user The username
	 * @param pass The password
	 */
	void login(const QString& user, const QString& pass);
	
private:
	/**
	 * Will parse the data that arrived in the jobData() slot.
	 * If it is the requestpage the remaining minutes will be parsed.
	 */
	void parseData();
	/**
	 * Will popup a dialog with fields for username and passwors, 
	 * where you are asked to login.
	 */
	void login();
	/// The received data. (Hopefully the requestpage)
	QString m_snakepage;
	/// Keeps track of if we just tried to login
	bool m_login_tried;
signals:
       /** 
	* This signal will be sent when the parser has 
	* retrieved a value for the time to next request.
	*/
	void timeLeftReceived(int);

	/**
	 * Emitted after we have tried to login.
	 */
	void loginTried();
};
 
#endif
