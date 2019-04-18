/*****************************************************************************
**                                                                          **
**    Copyright (C) 2008 Denisss (Denis Novikov). All rights reserved.      **
**                                                                          **
**    This file contains  declaration of class soundSettings                **
**                                                                          **
**    This  file may be used  under the terms of the GNU General  Public    **
**    License  version 2.0 as published by the Free Software Foundation.    **
**    Alternatively  you may (at your option) use any  later  version of    **
**    the GNU General Public License if such license has  been  publicly    **
**    approved by Trolltech ASA (or its  successors, if any) and the KDE    **
**    Free Qt Foundation.                                                   **
**                                                                          **
**        This file is provided AS IS with NO  WARRANTY OF ANY KIND,        **
**          INCLUDING THE  WARRANTY OF DESIGN, MERCHANTABILITY AND          **
**                   FITNESS  FOR A PARTICULAR PURPOSE.                     **
**                                                                          **
*****************************************************************************/


#ifndef SOUNDSETTINGS_H
#define SOUNDSETTINGS_H

#include <QtGui/QWidget>
#include <QList>
#include <QTreeWidgetItem>
//#include "../soundevents.h"
#include "../quticqglobals.h"

#include "ui_soundsettings.h"

class soundSettings : public QWidget
{
    Q_OBJECT

public:
    soundSettings(QWidget *parent = 0);
    ~soundSettings() {};
    void loadSettings(const QString &account);
    void saveSettings(const QString &account);

private slots:
	// Play tab widgets
	void on_systemCombo_currentIndexChanged(int index);
	void on_selAllButton_clicked();
	void on_commandButton_clicked();
	void on_playOnlyTree_itemChanged(QTreeWidgetItem *item, int column);

	// Events tab widgets
	void on_eventsTree_currentItemChanged(QTreeWidgetItem *current,
		QTreeWidgetItem *previous);
	void on_eventsTree_itemChanged(QTreeWidgetItem *item, int column);
	void on_fileEdit_textChanged(const QString &text);
	void on_applyButton_clicked();
	void on_openButton_clicked();
	void on_playButton_clicked();
	void on_exportButton_clicked();
	void on_importButton_clicked();

	void widgetStateChanged() { if (!changed) { emit settingsChanged(); changed = true; } };

signals:
	void settingsChanged();
	void settingsSaved();

private:
    void appendStatus(const QString &statusText);
    void appendEvent(const QString &eventName, const SoundEvent::Events event);
    inline QString getCurrentFile() const;
    inline SoundEvent::SoundSystem getCurrentSoundSystem() const;
    bool changed;

    Ui::SoundSettingsClass ui;
    QString lastDir;
    QList <QTreeWidgetItem *> statusList, eventList;
};

#endif // MAINSETTINGS_H
