//
// C++ Interface: snakesettings
//
// Description: 
//
//
// Author: Daniel Bengtsson <daniel@bengtssons.info>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SNAKESETTINGS_H
#define SNAKESETTINGS_H

#include <settings.h>

/**
 * @author Daniel Bengtsson
 *
 * This is the settings for SnakeTray
 */
class SnakeSettings : public Settings
{
	Q_OBJECT
			
	public:

		SnakeSettings(QWidget* parent): Settings(parent) {}
 
		virtual ~SnakeSettings(){};
	
	protected slots:

		void accept(){ Settings::accept();}
		
};

#endif
