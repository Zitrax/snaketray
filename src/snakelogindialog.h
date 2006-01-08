//
// C++ Interface: snakelogindialog
//
// Description: 
//
//
// Author: Daniel Bengtsson <daniel@bengtssons.info>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SNAKELOGINDIALOG_H
#define SNAKELOGINDIALOG_H

#include <login_dialog.h>

/**
@author Daniel Bengtsson
*/
class SnakeLoginDialog : public LoginDialog
{
Q_OBJECT

public:
    SnakeLoginDialog(QWidget* parent);

    ~SnakeLoginDialog();

protected slots:

    void accept();

signals:

    void login(const QString& username, const QString& password);

};

#endif
