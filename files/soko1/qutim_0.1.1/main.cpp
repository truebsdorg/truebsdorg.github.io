/*
    main.cpp

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

#include <QApplication>

#include "src/qutim.h"

#if defined(STATIC_IMAGE_PLUGINS)
Q_IMPORT_PLUGIN(qjpeg)
Q_IMPORT_PLUGIN(qgif)
#endif

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	if ( QFile::exists(QCoreApplication::applicationDirPath() + "/config"))
		QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationDirPath() + "/config");
//	QTranslator tra;
//	tra.load("qutim_ru");
//	a.installTranslator(&tra);
	qutIM w;
//	QFile file("qss/simple.qss");
//    if(file.open(QFile::ReadOnly))
//    {
//    	qApp->setStyleSheet("");
//	    QString styleSheet = QLatin1String(file.readAll());
//	    a.setStyleSheet(styleSheet);
//	    file.close();
//    }
//    w.show();
//    a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
	if (w.isShouldRun())
		return a.exec();
	else
		return 0;					// User decide to do not sign in
}
