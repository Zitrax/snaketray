//
// C++ Implementation: snakelogindialog
//
// Description: 
//
//
// Author: Daniel Bengtsson <daniel@bengtssons.info>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "snakelogindialog.h"

#include "qlineedit.h"

SnakeLoginDialog::SnakeLoginDialog(QWidget* parent)
 : LoginDialog(parent)
{
}

SnakeLoginDialog::~SnakeLoginDialog()
{
}

void SnakeLoginDialog::accept()
{
	emit login(m_username->text(), m_password->text());
	LoginDialog::accept();
}

#include "snakelogindialog.moc"
