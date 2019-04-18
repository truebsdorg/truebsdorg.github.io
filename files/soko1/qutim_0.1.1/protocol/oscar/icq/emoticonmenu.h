/*
    emoticonMenu

    Copyright (c) 2008 by Rustam Chakin <qutim.develop@gmail.com>

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*/


#ifndef EMOTICONMENU_H
#define EMOTICONMENU_H

#include <QtGui>
#include "ui_emoticonmenu.h"
#include <cmath>
#include "movielabel.h"

class emoticonMenu : public QWidget
{
    Q_OBJECT

public:
    emoticonMenu(QWidget *parent = 0);
    ~emoticonMenu();
    void setEmoticons(const QHash<QString, QString> &, const QString &);

signals:
	void insertSmile(const QString &);
	
protected:

	void hideEvent ( QHideEvent * );
	void showEvent ( QShowEvent * );
    
private:
    Ui::emoticonMenuClass ui;
    QList<movieLabel *> labelList;
    QList<QMovie *> movieList;
    QHash<QString, QString>  emotList;
    QString emotPath;
    
    void clearList();
};

#endif // EMOTICONMENU_H
