/*
    contactListSettings

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

#include "contactlistsettings.h"

contactListSettings::contactListSettings(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
	changed = false;
	connect (ui.showGroupsBox , SIGNAL(stateChanged(int)),
							this, SLOT(widgetStateChanged()));
	connect (ui.hideEmptyBox , SIGNAL(stateChanged(int)),
								this, SLOT(widgetStateChanged()));
	connect (ui.offlineBox , SIGNAL(stateChanged(int)),
								this, SLOT(widgetStateChanged()));
	connect (ui.separatorBox , SIGNAL(stateChanged(int)),
									this, SLOT(widgetStateChanged()));
	connect (ui.nilBox , SIGNAL(stateChanged(int)),
								this, SLOT(widgetStateChanged()));
	connect (ui.authorizedBox , SIGNAL(stateChanged(int)),
								this, SLOT(widgetStateChanged()));
	connect (ui.birthdayBox , SIGNAL(stateChanged(int)),
								this, SLOT(widgetStateChanged()));
	connect (ui.expandBox , SIGNAL(stateChanged(int)),
									this, SLOT(widgetStateChanged()));
	connect (ui.accountFontBox, SIGNAL(stateChanged(int)),
										this, SLOT(widgetStateChanged()));
	connect (ui.groupFontBox , SIGNAL(stateChanged(int)),
										this, SLOT(widgetStateChanged()));
	connect (ui.onlineFontBox , SIGNAL(stateChanged(int)),
										this, SLOT(widgetStateChanged()));
	connect (ui.offlineFontBox , SIGNAL(stateChanged(int)),
										this, SLOT(widgetStateChanged()));
	connect (ui.accountFontComboBox , SIGNAL(currentFontChanged ( const QFont & )),
											this, SLOT(widgetStateChanged()));
	connect (ui.groupFontComboBox , SIGNAL(currentFontChanged ( const QFont & )),
												this, SLOT(widgetStateChanged()));
	connect (ui.onlineFontComboBox , SIGNAL(currentFontChanged ( const QFont & )),
												this, SLOT(widgetStateChanged()));
	connect (ui.offlineFontComboBox , SIGNAL(currentFontChanged ( const QFont & )),
												this, SLOT(widgetStateChanged()));
	connect (ui.accountFontSizeBox , SIGNAL(valueChanged ( int )),
													this, SLOT(widgetStateChanged()));
	connect (ui.groupFontSizeBox , SIGNAL(valueChanged ( int )),
														this, SLOT(widgetStateChanged()));
	connect (ui.onlineFontSizeBox , SIGNAL(valueChanged ( int )),
														this, SLOT(widgetStateChanged()));
	connect (ui.offlineFontSizeBox , SIGNAL(valueChanged ( int )),
														this, SLOT(widgetStateChanged()));
	accountCol.setRgb(0,0,0);
	groupCol.setRgb(0,0,0);
	onlineCol.setRgb(0,0,0);
	offlineCol.setRgb(0,0,0);
}

contactListSettings::~contactListSettings()
{

}

void contactListSettings::loadSettings(const QString &account)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+account, "account");
	settings.beginGroup("contactlist");
	ui.showGroupsBox->setChecked(settings.value("showgroups", true).toBool());
	ui.hideEmptyBox->setChecked(settings.value("hideempty", false).toBool());
	ui.offlineBox->setChecked(settings.value("hideoff", false).toBool());
	ui.separatorBox->setChecked(settings.value("hidesep", false).toBool());
	ui.nilBox->setChecked(settings.value("nilclear", false).toBool());
	ui.authorizedBox->setChecked(settings.value("auth", false).toBool());
	ui.birthdayBox->setChecked(settings.value("birthday", false).toBool());
	ui.expandBox->setChecked(settings.value("expanded", true).toBool());
	ui.accountFontBox->setChecked(settings.value("acccustomf", false).toBool());
	ui.groupFontBox->setChecked(settings.value("grpcustomf", false).toBool());
	ui.onlineFontBox->setChecked(settings.value("onlcustomf", false).toBool());
	ui.offlineFontBox->setChecked(settings.value("offcustomf", false).toBool());
	ui.accountFontComboBox->setCurrentFont(QFont(settings.value("acccustomfam").toString()));
	ui.groupFontComboBox->setCurrentFont(QFont(settings.value("grpcustomfam").toString()));
	ui.onlineFontComboBox->setCurrentFont(QFont(settings.value("onlcustomfam").toString()));
	ui.offlineFontComboBox->setCurrentFont(QFont(settings.value("offcustomfam").toString()));
	int accSiz = settings.value("acccustomsizf", 9).toInt();
	accSiz = accSiz < 6 ? 6 : accSiz;
	accSiz = accSiz > 24 ? 24 : accSiz;
	ui.accountFontSizeBox->setValue(accSiz);
	
	int grpSiz = settings.value("grpcustomsizf", 7).toInt();
	grpSiz = grpSiz < 6 ? 6 : grpSiz;
	grpSiz = accSiz > 24 ? 24 : grpSiz;
	ui.groupFontSizeBox->setValue(grpSiz);
		
	int onlSiz = settings.value("onlcustomsizf", 9).toInt();
	onlSiz = onlSiz < 6 ? 6 : onlSiz;
	onlSiz = onlSiz > 24 ? 24 : onlSiz;
	ui.onlineFontSizeBox->setValue(onlSiz);
			
	int offSiz = settings.value("offcustomsizf", 9).toInt();
	offSiz = offSiz < 6 ? 6 : offSiz;
	offSiz = offSiz > 24 ? 24 : offSiz;
	ui.offlineFontSizeBox->setValue(offSiz);
	
	
	QVariant accCol = settings.value("acccustomcol");
	if ( accCol.canConvert<QColor>())
		accountCol = accCol.value<QColor>();
	
	QVariant grpCol = settings.value("grpcustomcol");
		if ( grpCol.canConvert<QColor>())
			groupCol = grpCol.value<QColor>();
		
	QVariant onlCol = settings.value("onlcustomcol");
		if ( onlCol.canConvert<QColor>())
			onlineCol = onlCol.value<QColor>();
			
	QVariant offCol = settings.value("offcustomcol");
		if ( offCol.canConvert<QColor>())
			offlineCol = offCol.value<QColor>();
	
	
	settings.endGroup();
}

void contactListSettings::saveSettings(const QString &account)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+account, "account");
	settings.beginGroup("contactlist");
	settings.setValue("showgroups",ui.showGroupsBox->isChecked());	
	settings.setValue("hideempty",ui.hideEmptyBox->isChecked());
	settings.setValue("hideoff",ui.offlineBox->isChecked());
	settings.setValue("hidesep",ui.separatorBox->isChecked());
	settings.setValue("nilclear",ui.nilBox->isChecked());
	settings.setValue("auth",ui.authorizedBox->isChecked());
	settings.setValue("birthday",ui.birthdayBox->isChecked());
	settings.setValue("expanded",ui.expandBox->isChecked());
	settings.setValue("acccustomf",ui.accountFontBox->isChecked());
	settings.setValue("grpcustomf",ui.groupFontBox->isChecked());
	settings.setValue("onlcustomf",ui.onlineFontBox->isChecked());
	settings.setValue("offcustomf",ui.offlineFontBox->isChecked());
	settings.setValue("acccustomfam",ui.accountFontComboBox->currentFont().family());
	settings.setValue("grpcustomfam",ui.groupFontComboBox->currentFont().family());
	settings.setValue("onlcustomfam",ui.onlineFontComboBox->currentFont().family());
	settings.setValue("offcustomfam",ui.offlineFontComboBox->currentFont().family());
	settings.setValue("acccustomsizf",ui.accountFontSizeBox->value());
	settings.setValue("grpcustomsizf",ui.groupFontSizeBox->value());
	settings.setValue("onlcustomsizf",ui.onlineFontSizeBox->value());
	settings.setValue("offcustomsizf",ui.offlineFontSizeBox->value());
	settings.setValue("acccustomcol", accountCol);
	settings.setValue("grpcustomcol", groupCol);
	settings.setValue("onlcustomcol", onlineCol);
	settings.setValue("offcustomcol", offlineCol);
	settings.endGroup();
	
	
	if ( changed )
			emit settingsSaved();
	changed = false;
}

void contactListSettings::on_accountColorButton_clicked()
{

	
	QColor color = 	QColorDialog::getColor(accountCol, this); 
	if ( color.isValid() && color != accountCol )
	{
		emit widgetStateChanged();
		accountCol.setRgb(color.rgb());
	}
}

void contactListSettings::on_groupColorButton_clicked()
{

	
	QColor color = 	QColorDialog::getColor(groupCol, this); 
	if ( color.isValid() && color != groupCol )
	{
		emit widgetStateChanged();
		groupCol.setRgb(color.rgb());
	}
}
void contactListSettings::on_onlineColorButton_clicked()
{

	
	QColor color = 	QColorDialog::getColor(onlineCol, this); 
	if ( color.isValid() && color != onlineCol )
	{
		emit widgetStateChanged();
		onlineCol.setRgb(color.rgb());
	}
}
void contactListSettings::on_offlineColorButton_clicked()
{

	
	QColor color = 	QColorDialog::getColor(offlineCol, this); 
	if ( color.isValid() && color != offlineCol )
	{
		emit widgetStateChanged();
		offlineCol.setRgb(color.rgb());
	}
}
