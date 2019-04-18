/*
    statusDialog

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


#include "statusdialog.h"

statusDialog::statusDialog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
	setWindowTitle(tr("Set status message"));
	setWindowIcon(QIcon(":/icons/crystal_project/statuses.png"));
	setAttribute(Qt::WA_QuitOnClose, false);
	move(desktopCenter());
	setFixedSize(size());
	closeSec = 11;
	connect( &closeTimer, SIGNAL(timeout()), this , SLOT(closeAfter()));
}

statusDialog::~statusDialog()
{

}

QPoint statusDialog::desktopCenter()
{
	QDesktopWidget desktop;
	return QPoint(desktop.width() / 2 - size().width() / 2, desktop.height() / 2 - size().height() / 2);
}

void statusDialog::closeAfter()
{
	closeSec--;
	ui.closeAfter->setText(tr("Close this dialog after %1 s").arg(closeSec));
	if ( closeSec )
		closeTimer.start(1000);
	else
		on_okButton_clicked();
}

void statusDialog::setStatusMessage(const QString &status)
{
	QString t = status;
	ui.statusEdit->setPlainText(t.replace("\n", "<br>"));
	closeAfter();
}

void statusDialog::on_okButton_clicked()
{
	statusMessage = ui.statusEdit->toPlainText();
	dontShow = ui.dontShowBox->isChecked();
	accept();
}

void statusDialog::on_statusEdit_textChanged()
{
	closeTimer.stop();
	ui.closeAfter->clear();
}
