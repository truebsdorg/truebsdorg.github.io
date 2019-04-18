/*
    historyXML

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

//#include <QtCore>
#include <QtXml>
#include "chatwindow.h"
#include "treebuddyitem.h"

#include "historyxml.h"

historyXML::historyXML(const QString &path) : historyPath(path)
{
	
}

historyXML::~historyXML()
{
	
}

void historyXML::saveHistoryMessage(const QString &historyUin, const QString &from, const QDateTime &date, bool in, const QString &msg)
{
	QString filename = historyUin + date.toString(".yyyyMM") + ".xml";
	QFile historyFile(historyPath + "/" + filename);
	QDir historyDir(historyPath);
	if ( !historyDir.exists() )
		historyDir.mkpath(historyPath);

	if ( historyFile.exists() )
	{
		if (historyFile.open(QIODevice::ReadOnly) )
		{
			QDomDocument doc;
			if ( doc.setContent(&historyFile) )
			{
				QDomElement rootElement = doc.documentElement();
				
				QDomElement messageXml = doc.createElement("msg");
				if ( !from.isEmpty() )
				{
					QDomAttr nickAttr = doc.createAttribute("nick");
					nickAttr.setValue(from);
					messageXml.setAttributeNode(nickAttr);
				}
				
				QDomAttr inAttr = doc.createAttribute("in");
				inAttr.setValue(QString::number(in));
				messageXml.setAttributeNode(inAttr);
				
				QDomAttr dayAttr = doc.createAttribute("day");
				dayAttr.setValue(QString::number(date.date().day()));
				messageXml.setAttributeNode(dayAttr);
				
				QDomAttr timeAttr = doc.createAttribute("time");
				timeAttr.setValue(date.time().toString("h:m:s"));
				messageXml.setAttributeNode(timeAttr);
				
				QDomText messageText = doc.createTextNode(msg);
				messageXml.appendChild(messageText);
				
				rootElement.insertAfter(messageXml, rootElement.lastChild());
				
				
			}
			
			
			historyFile.close();
			
			if ( historyFile.open(QIODevice::WriteOnly) )
			{
				
				QTextStream historyStream(&historyFile);
				historyStream.setCodec("UTF-8");
				historyStream<<doc.toString();
				historyFile.close();
			}
			
			
			
		}
		
		
	} else {
		if ( historyFile.open(QIODevice::WriteOnly) )
		{
			QDomDocument doc("qutimhistory");
			QDomProcessingInstruction process = doc.createProcessingInstruction(
			        "xml", "version=\"1.0\" encoding=\"utf-8\"");
			doc.appendChild(process);
			QDomElement rootElement = doc.createElement("qutimhistory");
			doc.appendChild(rootElement);
			
			
			QDomElement dateElement = doc.createElement("date");
			QDomAttr monthAttribute = doc.createAttribute("month");
			monthAttribute.setValue(QString::number(date.date().month()));
			QDomAttr yearAttribute = doc.createAttribute("year");
			yearAttribute.setValue(QString::number(date.date().year()));
			dateElement.setAttributeNode(monthAttribute);
			dateElement.setAttributeNode(yearAttribute);
			
			QDomElement messageXml = doc.createElement("msg");
			if ( !from.isEmpty() )
			{
				QDomAttr nickAttr = doc.createAttribute("nick");
				nickAttr.setValue(from);
				messageXml.setAttributeNode(nickAttr);
			}
			
			QDomAttr inAttr = doc.createAttribute("in");
			inAttr.setValue(QString::number(in));
			messageXml.setAttributeNode(inAttr);
			
			QDomAttr dayAttr = doc.createAttribute("day");
			dayAttr.setValue(QString::number(date.date().day()));
			messageXml.setAttributeNode(dayAttr);
			
			QDomAttr timeAttr = doc.createAttribute("time");
			timeAttr.setValue(date.time().toString("h:m:s"));
			messageXml.setAttributeNode(timeAttr);
			
			QDomText messageText = doc.createTextNode(msg);
			messageXml.appendChild(messageText);
			
			rootElement.appendChild(dateElement);
			rootElement.appendChild(messageXml);
			
			QTextStream historyStream(&historyFile);
			historyStream.setCodec("UTF-8");
			historyStream<<doc.toString();
			historyFile.close();
		}
	}
	
	
}

void historyXML::saveServiceMessage(const QString &from, const QDateTime &date, int type, const QString &msg)
{
	QString filename = "log" + date.toString(".yyyyMM") + ".xml";
	QFile historyFile(historyPath + "/" + filename);
	QDir historyDir(historyPath);
	if ( !historyDir.exists() )
		historyDir.mkpath(historyPath);

	if ( historyFile.exists() )
	{
		if (historyFile.open(QIODevice::ReadOnly) )
		{
			QDomDocument doc;
			if ( doc.setContent(&historyFile) )
			{
				QDomElement rootElement = doc.documentElement();
				
				QDomElement serviceXml;
				QDomText messageText;
							switch( type )
							{
							case 0:			
								serviceXml = doc.createElement("onl");
								break;
							case 1:			
								serviceXml = doc.createElement("off");
								break;
							case 2:			
								serviceXml = doc.createElement("readaway");
								break;
							case 3:
								serviceXml = doc.createElement("block");
								messageText = doc.createTextNode(msg);
								serviceXml.appendChild(messageText);
								break;
							default:
								;
							
							}
				
				
				if ( !from.isEmpty() )
				{
					QDomAttr nickAttr = doc.createAttribute("nick");
					nickAttr.setValue(from);
					serviceXml.setAttributeNode(nickAttr);
				}

				
				QDomAttr dayAttr = doc.createAttribute("day");
				dayAttr.setValue(QString::number(date.date().day()));
				serviceXml.setAttributeNode(dayAttr);
				
				QDomAttr timeAttr = doc.createAttribute("time");
				timeAttr.setValue(date.time().toString("h:m:s"));
				serviceXml.setAttributeNode(timeAttr);

				
				rootElement.insertAfter(serviceXml, rootElement.lastChild());
				
				
			}
			
			
			historyFile.close();
			
			if ( historyFile.open(QIODevice::WriteOnly) )
			{
				
				QTextStream historyStream(&historyFile);
				historyStream.setCodec("UTF-8");
				historyStream<<doc.toString();
				historyFile.close();
			}
			
			
			
		}
		
	} else {
		if ( historyFile.open(QIODevice::WriteOnly) )
		{
			QDomDocument doc("qutimlog");
			QDomProcessingInstruction process = doc.createProcessingInstruction(
						        "xml", "version=\"1.0\" encoding=\"utf-8\"");
			doc.appendChild(process);
			QDomElement rootElement = doc.createElement("qutimlog");
			doc.appendChild(rootElement);
			
			
			QDomElement dateElement = doc.createElement("date");
			QDomAttr monthAttribute = doc.createAttribute("month");
			monthAttribute.setValue(QString::number(date.date().month()));
			QDomAttr yearAttribute = doc.createAttribute("year");
			yearAttribute.setValue(QString::number(date.date().year()));
			dateElement.setAttributeNode(monthAttribute);
			dateElement.setAttributeNode(yearAttribute);
			
			QDomElement serviceXml;
			QDomText messageText;
			switch( type )
			{
			case 0:			
				serviceXml = doc.createElement("onl");
				break;
			case 1:			
				serviceXml = doc.createElement("off");
				break;
			case 2:			
				serviceXml = doc.createElement("readaway");
				break;
			case 3:
				serviceXml = doc.createElement("block");
				messageText = doc.createTextNode(msg);
				serviceXml.appendChild(messageText);
				break;
			default:
				;
			
			}
			
			if ( !from.isEmpty() )
			{
				QDomAttr nickAttr = doc.createAttribute("nick");
				nickAttr.setValue(from);
				serviceXml.setAttributeNode(nickAttr);
			}
			
			
			QDomAttr dayAttr = doc.createAttribute("day");
			dayAttr.setValue(QString::number(date.date().day()));
			serviceXml.setAttributeNode(dayAttr);
			
			QDomAttr timeAttr = doc.createAttribute("time");
			timeAttr.setValue(date.time().toString("h:m:s"));
			serviceXml.setAttributeNode(timeAttr);
			
			
			rootElement.appendChild(dateElement);
			rootElement.appendChild(serviceXml);
			
			QTextStream historyStream(&historyFile);
			historyStream.setCodec("UTF-8");
			historyStream<<doc.toString();
			historyFile.close();
		}
	}
	
		
}

void historyXML::setRecentMessages(chatWindow *w, int count)
{
	QDir historyDir(historyPath);
	if ( historyDir.exists() )
	{
		QStringList fileList = historyDir.entryList().filter(w->chatWith);
		if ( fileList.count())
		{
			QFile historyFile(historyPath + "/" + fileList.last());
			if (historyFile.open(QIODevice::ReadOnly) )
			{
				QDomDocument doc;
				if ( doc.setContent(&historyFile) )
					{
						QDomElement rootElement = doc.documentElement();
						int msgCount = rootElement.elementsByTagName("msg").count();
						
						int setCount = msgCount < count ? msgCount : count;
						QDomElement msg = rootElement.lastChildElement("msg");
						
						for( int i = 0; i < setCount - 1; i++)
						{
							msg = msg.previousSiblingElement("msg");
						}
							for( int i = 0; i < setCount; i++)
							{
								
								if ( !msg.isNull() )
								{
								QDateTime msgDate;
								QDomElement dateElement = rootElement.firstChildElement("date");
								msgDate.setDate(QDate(dateElement.attribute("year").toUInt(), dateElement.attribute("month").toUInt(), 
										msg.attribute("day").toUInt()));
								msgDate.setTime(setTimeFromString(msg.attribute("time")));
								if ( msg.attribute("in").toShort())
									w->setMessage(msg.attribute("nick"), msg.text(), msgDate, true);
								else
									w->setMessage(msg.attribute("nick"), msg.text(), msgDate, false);
								}
								msg = msg.nextSiblingElement("msg");
							}
							
						historyFile.close();
												
					}
					
					
			}
			
		}
	}
}

void historyXML::saveUnreadedMessages(QHash<QString, treeBuddyItem*> *messageList)
{
	QFile historyFile(historyPath + "/unread.xml");
	QDir historyDir(historyPath);
	if ( !historyDir.exists() )
		historyDir.mkpath(historyPath);	
	
	
	if ( historyFile.open(QIODevice::WriteOnly) )
			{
				QDomDocument doc("unreadmessages");
				QDomProcessingInstruction process = doc.createProcessingInstruction(
							        "xml", "version=\"1.0\" encoding=\"utf-8\"");
				doc.appendChild(process);
				QDomElement rootElement = doc.createElement("unreadmessages");
				doc.appendChild(rootElement);
				
				
				foreach(treeBuddyItem *buddy, *messageList)
				{
				
					foreach(messageFormat *msg, buddy->messageList)
					{
						QDomElement messageXml = doc.createElement("unrmsg");
		
						QDomAttr nickAttr = doc.createAttribute("uin");
						nickAttr.setValue(buddy->getUin());
						messageXml.setAttributeNode(nickAttr);
						
						QDomAttr dateAttr = doc.createAttribute("date");
						dateAttr.setValue(msg->date.date().toString());
						messageXml.setAttributeNode(dateAttr);
						
						QDomAttr timeAttr = doc.createAttribute("time");
						timeAttr.setValue(msg->date.time().toString());
						messageXml.setAttributeNode(timeAttr);
						
						QDomText messageText = doc.createTextNode(msg->message);
						messageXml.appendChild(messageText);
						
						rootElement.appendChild(messageXml);
				
					}
				
				}
				QTextStream historyStream(&historyFile);
				historyStream.setCodec("UTF-8");
				historyStream<<doc.toString();
				historyFile.close();
			}
	
	
}

void historyXML::loadUnreaded( QList< messageFormat > *messageList )
{
	QFile historyFile(historyPath + "/unread.xml");
	if ( historyFile.open(QIODevice::ReadOnly))
	{
		QDomDocument doc;
		if ( doc.setContent(&historyFile) )
			{
				QDomElement rootElement = doc.documentElement();
				int msgCount = rootElement.elementsByTagName("unrmsg").count();
				
				QDomElement msg = rootElement.firstChildElement("unrmsg");
				
				for( int i = 0; i < msgCount; i++)
				{
					messageFormat unreadedMessage;
					unreadedMessage.fromUin = msg.attribute("uin");
					unreadedMessage.message = msg.text();
					unreadedMessage.date = QDateTime(QDate::fromString(msg.attribute("date")), (setTimeFromString(msg.attribute("time"))));
					messageList->append(unreadedMessage);
					msg = msg.nextSiblingElement("unrmsg");
				}
					
				historyFile.close();
				historyFile.remove();
										
			}
	}
	
}

void historyXML::addHistoryToList(QListWidget *list, const QString &uin, const QString &sSearch)
{
	QDir historyDir(historyPath);
	if ( historyDir.exists() )
	{
		QStringList fileList = historyDir.entryList().filter(uin);
		QStringList dates;
		foreach( QString file, fileList)
		{
			
			QFile historyFile(historyPath + "/" + file);
			if (historyFile.open(QIODevice::ReadOnly) )
			{
				QDomDocument doc;
				if ( doc.setContent(&historyFile) )
				{
					QDomElement rootElement = doc.documentElement();
					int msgCount = rootElement.elementsByTagName("msg").count();
					QDomElement dateElement = rootElement.firstChildElement("date");
					int year = dateElement.attribute("year").toUInt();
					int month = dateElement.attribute("month").toUInt();
					QDomElement msg = rootElement.firstChildElement("msg");
					for ( int i = 0; i < msgCount ; i++ )
					{
						
						
						int day = msg.attribute("day").toUInt();
						QDate dateItem(year, month ,day);
						QString dateStringItem = dateItem.toString("yyyy-MM-dd"); 
						if ( !dates.contains(dateStringItem))
							if( msg.text().indexOf(sSearch, Qt::CaseInsensitive) != -1 )
								dates<<dateStringItem;
						msg = msg.nextSiblingElement("msg");
							
					}
				}
			}
			
		}
		list->addItems(dates);
	}
}

void historyXML::addHistoryMessages(icqTextBrowser *chat, const QDate &date, const QString &uin, const QString &sSearch)
{
	chat->clear();
	QString filename = uin + date.toString(".yyyyMM") + ".xml";
	QFile historyFile(historyPath + "/" + filename);
	QDir historyDir(historyPath);
	if ( historyDir.exists() )
	{
		QFile historyFile(historyPath + "/" + filename);
		if (historyFile.open(QIODevice::ReadOnly) )
		{
			QDomDocument doc;
			if ( doc.setContent(&historyFile) )
			{
				QDomElement rootElement = doc.documentElement();
				int msgCount = rootElement.elementsByTagName("msg").count();
				QDomElement msg = rootElement.firstChildElement("msg");
				for ( int i = 0; i < msgCount ; i++ )
				{
					if ( msg.attribute("day").toInt() == date.day())
					{
						QTime time = setTimeFromString(msg.attribute("time"));
						QString from(msg.attribute("nick"));
						bool in = msg.attribute("in").toShort();
						QString fromText;
						
						if ( in )
						{
							fromText.append("<font color='red'>" + Qt::escape(from) +": ("
									+ time.toString("hh:mm:ss") + ")</font>" );
							
						} else {
							fromText.append("<font color='blue'>" + Qt::escape(from) +": ("
									+ time.toString("hh:mm:ss") + ")</font>" );
						}
						
						chat->append(fromText);
						

						
						if ( msg.text().startsWith("<qimage>") && msg.text().endsWith("</qimage>"))
						{
							QString tmpData = msg.text();
							tmpData.chop(9);
							
							QByteArray rawData = QByteArray::fromHex(tmpData.right(tmpData.length() - 8).toLocal8Bit());
							
							QPixmap pic;
							
							if ( pic.loadFromData(rawData))
							{
							
							chat->document()->addResource(QTextDocument::ImageResource,
								QUrl(Qt::escape(msg.text())), pic);
							chat->setLineWrapColumnOrWidth(chat->lineWrapColumnOrWidth());
							chat->append("<img src='" + Qt::escape(msg.text()) +"'>");
							} else 
								chat->append(Qt::escape(msg.text()));
						}
						else						
							if(!sSearch.isEmpty())
								chat->append("<font style=background-color:white>"+Qt::escape(msg.text()).replace(sSearch, "<font style=background-color:yellow>"+sSearch+"</font>", Qt::CaseInsensitive)+"</font>");
							else
								chat->append("<font color=black>" + findUrls(Qt::escape(msg.text())) + "</font>");
						chat->moveCursor(QTextCursor::End);
						chat->ensureCursorVisible();
						
					}
					msg = msg.nextSiblingElement("msg");
						
				}
				
			}
		}
	}
}

void historyXML::addServiceHistoryToList(QListWidget *list, const QString &sSearch)
{
	QDir historyDir(historyPath);
	if ( historyDir.exists() )
	{
		QStringList fileList = historyDir.entryList().filter("log");
		QStringList dates;
		foreach( QString file, fileList)
		{
			
			QFile historyFile(historyPath + "/" + file);
			if (historyFile.open(QIODevice::ReadOnly) )
			{
				QDomDocument doc;
				if ( doc.setContent(&historyFile) )
				{
					QDomElement rootElement = doc.documentElement();
					int msgCount = rootElement.childNodes().count();
					QDomElement dateElement = rootElement.firstChildElement("date");
					int year = dateElement.attribute("year").toUInt();
					int month = dateElement.attribute("month").toUInt();
					
					
					QDomElement msg = dateElement.nextSibling().toElement();
					for ( int i = 0; i < msgCount - 1 ; i++ )
					{
						
						
						int day = msg.attribute("day").toUInt();
						QDate dateItem(year, month ,day);
						QString dateStringItem = dateItem.toString("yyyy-MM-dd"); 
						if ( !dates.contains(dateStringItem))
							if( msg.attribute("nick").indexOf(sSearch, Qt::CaseInsensitive) != -1 )
								dates<<dateStringItem;
						msg = msg.nextSibling().toElement();
							
					}
				}
			}
			
		}
		
		list->addItems(dates);
	}
}


void historyXML::addServiceHistoryMessages(icqTextBrowser *chat, const QDate &date)
{
	chat->clear();
	QString filename = "log" + date.toString(".yyyyMM") + ".xml";
	QFile historyFile(historyPath + "/" + filename);
	QDir historyDir(historyPath);
	if ( historyDir.exists() )
	{
		QFile historyFile(historyPath + "/" + filename);
		if (historyFile.open(QIODevice::ReadOnly) )
		{
			QDomDocument doc;
			if ( doc.setContent(&historyFile) )
			{
				QDomElement rootElement = doc.documentElement();
				int msgCount = rootElement.childNodes().count();
				
				QDomElement msg = rootElement.firstChild().toElement();
				
				for ( int i = 0; i < msgCount ; i++ )
				{
					
					if ( msg.tagName() != "date")
					{
						if ( msg.attribute("day").toInt() == date.day())
						{
							QTime time = setTimeFromString(msg.attribute("time"));
							QString nick(msg.attribute("nick"));
							QDateTime dateTime(date, time);
							
							if ( msg.tagName() == "onl")
							{
								chat->append( "<font color = 'red'> [" + dateTime.toString() + QObject::tr("] : %1 is online").arg(Qt::escape(nick)) + "</font>");
							} else if ( msg.tagName() == "off")
							{
								chat->append( "<font color = 'blue'>[" + dateTime.toString() + QObject::tr("] : %1 is offline").arg(Qt::escape(nick)) + "</font>");
							} else if (msg.tagName() == "readaway")
							{
								chat->append( "<font color = 'green'>[" + dateTime.toString() + QObject::tr("] : %1 is reading your away message").arg(Qt::escape(nick)) + "</font>");
							} else if (msg.tagName() == "block")
							{
								chat->append( "<font color = 'grey'>[" + dateTime.toString() + QObject::tr("] : %1 (BLOCKED MESSAGE) : %2").arg(Qt::escape(nick)).arg(msg.text()) + "</font>");
							}
							chat->moveCursor(QTextCursor::End);
							chat->ensureCursorVisible();
							
						}
					}
					msg = msg.nextSibling().toElement();
						
				}
				
			}
		}
	}
}

QTime historyXML::setTimeFromString(const QString &timeString)
{
	QStringList timeList = timeString.split(":");
	return QTime(timeList.at(0).toUInt(),
			timeList.at(1).toUInt(),
			timeList.at(2).toUInt());
}

QString historyXML::findUrls(const QString &sourceHTML)
{
    QString html = sourceHTML;
     QRegExp linkRegExp("((https?|ftp://|www\\.)[^\\s]+)");
     int pos = 0;
     while(((pos=linkRegExp.indexIn(html, pos)) != -1))
     {
             QString link = linkRegExp.cap(0);
             int linkCount = link.count();
             link = "<a href=\"" + link + "\" target=\"_blank\">" + link + "</a>";
             html.replace(pos, linkCount, link);
             pos += link.count();
     }
     return html;
}

void historyXML::deleteHistory(const QString &uin)
{
	QDir historyDir(historyPath);
	if ( historyDir.exists() )
	{
		QStringList fileList = historyDir.entryList().filter(uin);
		
		foreach(QString file, fileList)
			QFile::remove(historyPath + "/" + file);
	}

}
