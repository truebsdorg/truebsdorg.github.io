/*****************************************************************************
**                                                                          **
**    Copyright (C) 2008 Denisss (Denis Novikov). All rights reserved.      **
**                                                                          **
**    This file contains  implementation of class soundSettings.            **
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

#include <QtGui>
#include <QSound>
#include <QtXml>

#include "../soundevents.h"
#include "soundsettings.h"

soundSettings::soundSettings(QWidget *parent)
: QWidget(parent), changed(false)
{
	ui.setupUi(this);

// Statuses:
// online, ffc, away, na, occupied, dnd, invisible, lunch,
// evil, depression, athome, atwork, offline, connecting

	// Filling "play only" table (tree widget)
	appendStatus(tr("Online"));
	appendStatus(tr("Free for chat"));
	appendStatus(tr("Away"));
	appendStatus(tr("Not available"));
	appendStatus(tr("Occupied"));
	appendStatus(tr("DND"));
	appendStatus(tr("Invisible"));
	appendStatus(tr("Lunch"));
	appendStatus(tr("Evil"));
	appendStatus(tr("Depression"));
	appendStatus(tr("At home"));
	appendStatus(tr("At work"));
	appendStatus(tr("Offline"));
	appendStatus(tr("Connecting"));

	// Filling events table (tree widget)
	appendEvent(tr("Startup"), SoundEvent::Startup);
	appendEvent(tr("Connected"), SoundEvent::Connected);
	appendEvent(tr("Disconnected"), SoundEvent::Disconnected);
	appendEvent(tr("System event"), SoundEvent::SystemEvent);
	appendEvent(tr("Outgoing message"), SoundEvent::MessageSend);
	appendEvent(tr("Incoming message"), SoundEvent::MessageGet);
	appendEvent(tr("Contact is online"), SoundEvent::ContactOnline);
	appendEvent(tr("Contact is free for chat"), SoundEvent::ContactFfc);
	appendEvent(tr("Contact is away"), SoundEvent::ContactAway);
	appendEvent(tr("Contact is having lunch"), SoundEvent::ContactLunch);
	appendEvent(tr("Contact is at work"), SoundEvent::ContactAtWork);
	appendEvent(tr("Contact is at home"), SoundEvent::ContactAtHome);
	appendEvent(tr("Contact is angry"), SoundEvent::ContactEvil);
	appendEvent(tr("Contact has fallen into depression"),
		SoundEvent::ContactDepression);
	appendEvent(tr("Contact is not accessible"), SoundEvent::ContactNa);
	appendEvent(tr("Contact is occupied"), SoundEvent::ContactOccupied);
	appendEvent(tr("Contact is DND"), SoundEvent::ContactDnd);
	appendEvent(tr("Contact is invisible"), SoundEvent::ContactInvisible);
	appendEvent(tr("Contact went offline"), SoundEvent::ContactOffline);
	appendEvent(tr("Contact's birthday is comming"), SoundEvent::ContactBirthday);

	ui.playOnlyTree->setEnabled(false);

	// Filling systemCombo
	ui.systemCombo->setItemData(0, SoundEvent::None);
//	ui.systemCombo->addItem(tr("Phonon"), SoundEvent::LibPhonon);
	if (QSound::isAvailable())
		ui.systemCombo->addItem(tr("QSound"), SoundEvent::LibSound);
	ui.systemCombo->addItem(tr("Command"), SoundEvent::UserCommand);

	// disabling events tab
	ui.eventsTab->setEnabled(false);

	// connections
	connect(ui.commandEdit, SIGNAL(textChanged(const QString &)),
		this, SLOT(widgetStateChanged()));
	connect(ui.activeChatCheck, SIGNAL(stateChanged(int)),
		this, SLOT(widgetStateChanged()));
}

inline
void soundSettings::appendStatus(const QString &statusText)
{
	static const Qt::ItemFlags itemFlags =
		  Qt::ItemIsSelectable
		| Qt::ItemIsUserCheckable
		| Qt::ItemIsEnabled;

	QTreeWidgetItem *newItem = new QTreeWidgetItem(ui.playOnlyTree);
	newItem->setFlags(itemFlags);
	newItem->setText(0, statusText);
	newItem->setCheckState(0, Qt::Unchecked);
	statusList << newItem;
}

inline
void soundSettings::appendEvent(const QString &eventName,
	const SoundEvent::Events event)
{
	static const Qt::ItemFlags itemFlags =
		  Qt::ItemIsSelectable
		| Qt::ItemIsUserCheckable
		| Qt::ItemIsEnabled;

	QTreeWidgetItem *newItem = new QTreeWidgetItem(ui.eventsTree);
	newItem->setFlags(itemFlags);
	newItem->setText(0, eventName);
	newItem->setCheckState(0, Qt::Unchecked);
	newItem->setData(0, Qt::UserRole, static_cast<int>(event));
	eventList << newItem;
}

void soundSettings::loadSettings(const QString &account)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."
		+ account, "account");

	settings.beginGroup("sounds");

	ui.systemCombo->setCurrentIndex(ui.systemCombo->findData(settings.value("system",
		0).toInt()));

	QBitArray statuses = settings.value("statuses",
		QBitArray(STATUS_COUNT, true)).toBitArray();
	statuses.resize(STATUS_COUNT);

	int i;
	for (i = 0; i < statusList.size(); i++)
	{
		statusList.at(i)->setCheckState(0,
			(statuses.at(i)) ? Qt::Checked : Qt::Unchecked);
	}

	QString fileName;
	for (i = 0; i < eventList.size(); i++)
	{
		fileName = settings.value("event"
			+ QString::number(eventList.at(i)->data(0, Qt::UserRole).toInt()),
			"-").toString();
		eventList.at(i)->setCheckState(0,
			(fileName.at(0) == '+') ? Qt::Checked : Qt::Unchecked);
		eventList.at(i)->setData(0, Qt::UserRole+1, fileName.mid(1));
	}

	ui.activeChatCheck->setCheckState(settings.value("nochat",
		false).toBool() ? Qt::Checked : Qt::Unchecked);

#ifndef Q_OS_WIN32
	ui.commandEdit->setText(settings.value("command", "play \"%1\"").toString());
#else
	ui.commandEdit->setText(settings.value("command", "").toString());
#endif

	settings.endGroup();

	changed = false;
}

void soundSettings::saveSettings(const QString &account)
{
	// we do not save anything until something has been changed
	if (!changed) return;

	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."
		+ account, "account");

	settings.beginGroup("sounds");

	QString command = ui.commandEdit->text().trimmed();
	if (getCurrentSoundSystem() == SoundEvent::UserCommand)
	{
		if (command.isEmpty())
			ui.systemCombo->setCurrentIndex(0);
		else if (!command.contains("%1")) command += " \"%1\"";
		ui.commandEdit->setText(command);
	}

	settings.setValue("command", command);

	settings.setValue("system",
		ui.systemCombo->itemData(ui.systemCombo->currentIndex()).toInt());

	int i;
	QBitArray statuses(STATUS_COUNT);

	for (i = 0; i < statusList.size(); i++)
	{
		statuses.setBit(i, (statusList.at(i)->checkState(0) == Qt::Checked));
	}
	settings.setValue("statuses", statuses);

	QString fileName;
	for (i = 0; i < eventList.size(); i++)
	{
		fileName = (eventList.at(i)->checkState(0) == Qt::Checked) ? "+" : "-";
		fileName += eventList.at(i)->data(0, Qt::UserRole+1).toString();
		settings.setValue("event"
			+ QString::number(eventList.at(i)->data(0,
			Qt::UserRole).toInt()), fileName);
	}

	settings.setValue("nochat", (ui.activeChatCheck->checkState() == Qt::Checked));

	settings.endGroup();

	changed = false;
	emit settingsSaved();
}

void soundSettings::on_systemCombo_currentIndexChanged(int /* index */)
{
	SoundEvent::SoundSystem sys = getCurrentSoundSystem();

	ui.commandEdit->setEnabled(false);
	ui.commandButton->setEnabled(false);
	if (sys != SoundEvent::None)
	{
		ui.playOnlyTree->setEnabled(true);
		ui.eventsTab->setEnabled(true);
		if (sys == SoundEvent::UserCommand)
		{
			ui.commandEdit->setEnabled(true);
			ui.commandButton->setEnabled(true);
		}
	}
	else
	{
		ui.playOnlyTree->setEnabled(false);
		ui.eventsTab->setEnabled(false);
	}

	widgetStateChanged();
}

void soundSettings::on_selAllButton_clicked()
{
	QList<QTreeWidgetItem*>::const_iterator i = statusList.constBegin();
	bool bChanged = false;
	for ( ; i != statusList.constEnd(); ++i)
	{
		bChanged |= ((*i)->checkState(0) == Qt::Unchecked);
		(*i)->setCheckState(0, Qt::Checked);
	}
	if (!bChanged)
	{
		i = statusList.constBegin();
		for ( ; i != statusList.constEnd(); ++i)
			(*i)->setCheckState(0, Qt::Unchecked);
	}

	widgetStateChanged();
}

void soundSettings::on_commandButton_clicked()
{
#ifdef Q_OS_WIN32
	QString binName = QFileDialog::getOpenFileName(this,
		tr("Select command path"), QDir::currentPath(),
		tr("Executables (*.exe *.com *.cmd *.bat)\nAll files (*.*)"));
#else
	QString binName = QFileDialog::getOpenFileName(this,
		tr("Select command path"), "/bin",
		tr("Executables"));
#endif // WIN32
	if (binName.isEmpty()) return;

#ifndef Q_OS_WIN32
	ui.commandEdit->setText(binName + " \"%1\"");
#else
	ui.commandEdit->setText(QString("\"%1\" \"%2\"").arg(binName).arg("%1"));
#endif // WIN32
}

void soundSettings::on_playOnlyTree_itemChanged(QTreeWidgetItem * /* item */, int column)
{
	if (column) return;

	widgetStateChanged();
}

void soundSettings::on_eventsTree_currentItemChanged(QTreeWidgetItem *current,
	QTreeWidgetItem * /* previous */)
{
	if (!current) return;

	if (current->checkState(0) == Qt::Checked)
	{
		ui.fileEdit->setEnabled(true);
		ui.openButton->setEnabled(true);
	}
	else
	{
		ui.fileEdit->setEnabled(false);
		ui.openButton->setEnabled(false);
	}
	if (ui.fileEdit->text() == getCurrentFile())
		on_fileEdit_textChanged(getCurrentFile());
	else
		ui.fileEdit->setText(getCurrentFile());
}

void soundSettings::on_eventsTree_itemChanged(QTreeWidgetItem *item, int column)
{
	if (column) return;
	ui.fileEdit->setEnabled(item->checkState(0) == Qt::Checked);
	on_fileEdit_textChanged(ui.fileEdit->text());
	ui.openButton->setEnabled(ui.fileEdit->isEnabled());

	widgetStateChanged();
}

void soundSettings::on_fileEdit_textChanged(const QString &text)
{
	if (ui.fileEdit->isEnabled())
	{
		if (text.isEmpty())
		{
			ui.applyButton->setEnabled(true);
			ui.playButton->setEnabled(false);
		}
		else if (QFile::exists(text))
		{
			QTreeWidgetItem *item = ui.eventsTree->currentItem();
			if (item)
				ui.applyButton->setEnabled(getCurrentFile() != text);
			else
				ui.applyButton->setEnabled(false);

			ui.playButton->setEnabled(true);
		}
		else
		{
			ui.applyButton->setEnabled(false);
			ui.playButton->setEnabled(false);
		}
	}
	else
	{
		ui.applyButton->setEnabled(false);
		ui.playButton->setEnabled(false);
	}
}

void soundSettings::on_applyButton_clicked()
{
	ui.eventsTree->currentItem()->setData(0, Qt::UserRole+1, ui.fileEdit->text());
	ui.applyButton->setEnabled(false);

	widgetStateChanged();
}

void soundSettings::on_openButton_clicked()
{
	QString filter;
	if (getCurrentSoundSystem() == SoundEvent::LibSound)
		filter = tr("Sound files (*.wav)\n");
	filter += tr("All files (*.*)");

	if (lastDir.isEmpty())
	{
		if (!ui.fileEdit->text().isEmpty() && QFile::exists(ui.fileEdit->text()))
			lastDir = ui.fileEdit->text().section('/', 0, -2);
		else
			lastDir = QDir::currentPath();
	}

	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Open sound file"), lastDir, filter);
	if (fileName.isEmpty()) return;

	ui.fileEdit->setText(fileName);
	if (getCurrentFile().isEmpty())
	{
		ui.applyButton->setEnabled(true);
		ui.applyButton->animateClick(500);
	}
	lastDir = fileName.section('/', 0, -2);
}

void soundSettings::on_playButton_clicked()
{
	SoundEvent::SoundSystem sys = getCurrentSoundSystem();

	if (sys == SoundEvent::UserCommand)
	{
		QString command = ui.commandEdit->text().trimmed();
		if (command.isEmpty())
		{
			ui.systemCombo->setCurrentIndex(0);
			return;
		}
		if (!command.contains("%1"))
		{
			command += " \"%1\"";
			ui.commandButton->setText(command);
		}
		SoundEvents::play(sys, command.arg(ui.fileEdit->text()));
	}
	else
		SoundEvents::play(sys, ui.fileEdit->text());
}

void soundSettings::on_exportButton_clicked()
{
	QString fileName(QFileDialog::getSaveFileName(this, tr("Export sound style"),
		lastDir, tr("XML files (*.xml)")));
	if (fileName.isEmpty()) return;

	if (!fileName.endsWith(".xml")) fileName += ".xml";

	QString exportDir = fileName.section('/', 0, -2);
	lastDir = exportDir;

	if (QMessageBox::question(this, tr("Export..."),
		tr("All sound files will be copied into \"%1/\" directory. " \
			"Existing files will be replaced without any prompts.\n" \
			"Do You want to continue?").arg(exportDir),
			QMessageBox::Yes | QMessageBox::No,
			QMessageBox::No) != QMessageBox::Yes)
		return;

	exportDir += "/%1";

	QFile file(fileName);
	if (!file.open(QFile::WriteOnly | QFile::Text))
	{
		QMessageBox::critical(this, tr("Error"),
			tr("Could not open file \"%1\" for writing.").arg(fileName));
		return;
	}

	QDomDocument doc("qutimsounds");
	QDomProcessingInstruction process =
		doc.createProcessingInstruction("xml",
		"version=\"1.0\" encoding=\"utf-8\"");
	doc.appendChild(process);

	QDomElement rootElement = doc.createElement("qutimsounds");
	doc.appendChild(rootElement);

	QDomElement soundsElement = doc.createElement("sounds");

	int i;
	QString soundFile;
	QString newFile;
	for (i = 0; i < eventList.size(); i++)
	{
		soundFile = eventList.at(i)->data(0, Qt::UserRole+1).toString();
		if (soundFile.isEmpty()) continue;
		if (!QFile::exists(soundFile)) continue;
		QDomElement soundElement = doc.createElement("sound");
		QDomAttr eventAttr = doc.createAttribute("event");
		eventAttr.setValue(SoundEvent::XmlEventNames[eventList.at(i)->data(0,
			Qt::UserRole).toInt()]);
		soundElement.setAttributeNode(eventAttr);

		QDomElement fileElement = doc.createElement("file");
		QDomText fileText =
			doc.createTextNode(soundFile.mid(soundFile.lastIndexOf('/')+1));

		newFile = exportDir.arg(fileText.data());

		// check if directories are different
		if (newFile.section('/', 0, -2) != soundFile.section('/', 0, -2))
		{
			if (QFile::exists(newFile))
				QFile::remove(newFile);

			if (!QFile::copy(soundFile, newFile))
			{
				QMessageBox::critical(this, tr("Error"),
					tr("Could not copy file \"%1\" to \"%2\".").arg(soundFile).arg(newFile));
				file.close();
				return;
			}
		}

		fileElement.appendChild(fileText);
		soundElement.appendChild(fileElement);

		soundsElement.appendChild(soundElement);
	}

	rootElement.appendChild(soundsElement);

	QTextStream fileStream(&file);
	fileStream.setCodec("UTF-8");
	fileStream << doc.toString();
	file.close();

	QMessageBox::information(this, tr("Export..."),
		tr("Sound events successfully exported to file \"%1\".").arg(fileName));
}

void soundSettings::on_importButton_clicked()
{
	int i, eventPos;
	bool bChanged = false;
	QString fileName(QFileDialog::getOpenFileName(this, tr("Import sound style"),
		lastDir, tr("XML files (*.xml)")));
	if (fileName.isEmpty()) return;

	QString importDir = fileName.section('/', 0, -2);
	lastDir = importDir;

	importDir += "/%1";

	QFile file(fileName);
	if (!file.open(QFile::ReadOnly | QFile::Text))
	{
		QMessageBox::critical(this, tr("Error"),
			tr("Could not open file \"%1\" for reading.").arg(fileName));
		return;
	}

	QDomDocument doc("qutimsounds");
	if (!doc.setContent(&file))
	{
		file.close();
		QMessageBox::critical(this, tr("Error"),
			tr("An error occured while reading file \"%1\".").arg(fileName));
		return;
	}
	file.close();

	QDomElement rootElement = doc.documentElement();

	QDomNodeList soundsNodeList = rootElement.elementsByTagName("sounds");
	if (soundsNodeList.count() != 1)
	{
		QMessageBox::critical(this, tr("Error"),
			tr("File \"%1\" has wrong format.").arg(fileName));
		return;
	}

	// Preparing QHash for faster search;
	QHash<QString, int> eventHash;
	int event;
	for (i = 0; i < eventList.size(); i++)
	{
		event = eventList.at(i)->data(0, Qt::UserRole).toInt();
		if (!SoundEvent::XmlEventNames[event]) continue;
		eventHash.insert(SoundEvent::XmlEventNames[event], i);

		// clear old files
		eventList.at(i)->setData(0, Qt::UserRole+1, QString());
		eventList.at(i)->setCheckState(0, Qt::Unchecked);
	}

	QDomElement soundsElement = soundsNodeList.at(0).toElement();
	soundsNodeList = soundsElement.elementsByTagName("sound");

	QDomElement soundElement;
	QString eventName, soundFileName;

	for (i = 0; i < soundsNodeList.count(); i++)
	{
		soundElement = soundsNodeList.at(i).toElement();
		eventName = soundElement.attribute("event");
		if (eventName.isEmpty()) continue;

		if (!eventHash.contains(eventName)) continue;

		if (!soundElement.elementsByTagName("file").count()) continue;

		soundFileName = importDir.arg(soundElement.elementsByTagName("file").at(0).toElement().text());
		if (!QFile::exists(soundFileName)) continue;

		eventPos = eventHash.value(eventName);
		eventList.at(eventPos)->setData(0,
			Qt::UserRole+1, soundFileName);

		eventList.at(eventPos)->setCheckState(0, Qt::Checked);

		bChanged = true;
	}

	if (bChanged) widgetStateChanged();

	on_eventsTree_currentItemChanged(ui.eventsTree->currentItem(), NULL);
	}


inline QString soundSettings::getCurrentFile() const
{
	QTreeWidgetItem *item = ui.eventsTree->currentItem();
	if (item)
		return item->data(0, Qt::UserRole+1).toString();
	else
		return QString();
}

inline SoundEvent::SoundSystem soundSettings::getCurrentSoundSystem() const
{
	return static_cast<SoundEvent::SoundSystem>(ui.systemCombo->itemData(ui.systemCombo->currentIndex()).toInt());
}
