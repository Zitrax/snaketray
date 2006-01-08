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

class SnakeParser : public QObject
{
Q_OBJECT
public:
	SnakeParser(QObject* parent, const char* name);
	~SnakeParser();
	
	void startParsing(const QString& url);
	
private slots:
	void jobResult( KIO::Job* job );
	void jobData( KIO::Job* job, const QByteArray& data );
	void login(const QString& user, const QString& pass);
	
private:
	void parseData();
	void login();
	QString m_snakepage;
	bool m_login_tried;
signals:
       /** 
	* This signal will be sent when the parser has 
	* retrieved a value for the time to next request.
	*/
	void timeLeftReceived(int);

	void loginTried();
};
 
#endif
