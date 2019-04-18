/*
    chatWindow

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

#include <QtXml>
#include "icqmessage.h"
#include "icqtextedit.h"
#include "emoticonmenu.h"

#include "chatwindow.h"

chatWindow::chatWindow(QWidget *parent)
    : QWidget(parent)
{
	online = false;
	showNames = false;
	timestamp = 1;
	typingChanged = false;
	textChanged = false;
	lightVersion = false;
	contactAvatarPath.clear();
	ui.setupUi(this);
	ui.countLabel->setVisible(false);
//	ui.messageChat->setForegroundRole(QPalette::Text);
//	ui.messageEdit->setForegroundRole(QPalette::Text);
	connect(ui.messageEdit, SIGNAL(sendMessage()), ui.sendButton, SLOT(click()));
	connect(ui.messageEdit, SIGNAL(focusWindow()), this, SLOT(tst()));
	connect(ui.messageChat, SIGNAL(focusWindow()), this, SLOT(tst()));
	connect(ui.messageEdit, SIGNAL(closeWindow()), this, SLOT(close()));
	setFocus(Qt::ActiveWindowFocusReason);
//	ui.messageChat->setFocusProxy(this);
//	ui.messageEdit->setFocusProxy(this);
	
	ui.sendButton->setFocusProxy(this);
	ui.messageEdit->setFocus(Qt::ActiveWindowFocusReason);
	setMinimumWidth(250);
//	resize(600,600);
//	installEventFilter(this);
	emoticMenu = new QMenu(ui.emoticonButton);
	emoticonAction = new QWidgetAction(emoticMenu);
	emoticonWidget = new emoticonMenu(emoticMenu);
	emoticonAction->setDefaultWidget(emoticonWidget);
	emoticMenu->addAction(emoticonAction);
	ui.emoticonButton->setMenu(emoticMenu);
	connect(emoticonWidget, SIGNAL(insertSmile(const QString &)), this,
			SLOT(insertSmile(const QString &)));
			
#if defined(Q_OS_MAC)
	// Mac OS X layout fixup
	ui.splitter->setHandleWidth(3);
	ui.gridLayout->setSpacing(3);
	ui.vboxLayout->setSpacing(3);
#endif
	
	changeFont = new QAction(QIcon(":/icons/crystal_project/fonts.png"), tr("Change font"), this);
	connect(changeFont, SIGNAL(triggered()), this, SLOT(fontChangeActionTriggered()));
	changeFontColor = new QAction(QIcon(":/icons/crystal_project/fontcolor.png"), tr("Change text color"), this);
	connect(changeFontColor, SIGNAL(triggered()), this, SLOT(fontChangeColorActionTriggered()));
	fontMenu = new QMenu(ui.fontButton);
	fontMenu->addAction(changeFont);
	fontMenu->addAction(changeFontColor);
	ui.fontButton->setMenu(fontMenu);
	textColor.setRgb(0,0,0);
	backroundColor.setRgb(255,255,255);
	contactAv = new QMovie;
	mineAv = new QMovie;
	
	ui.messageEdit->setFocus();
}

chatWindow::~chatWindow()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+uin, "account");
	settings.setValue("chatwindow/state", ui.splitter->saveState());
	delete emoticonWidget;
	delete emoticMenu;
	delete contactAv;
	delete mineAv;
}

void chatWindow::setMessage(const QString &from, const QString &message, const QDateTime &date, bool in)
{
	
	ui.messageChat->moveCursor(QTextCursor::End);
	QString fromText;
	if ( !lightVersion )
	{
		
		fromText.append("<table width = '100%' border='0' cellspacing='0' cellpadding='0'><tbody><tr><td style='background-image : url(front.png);'><font color='grey' size='3'>");
		QString color;
		if ( in )
			color = "red";
		else
			color = "blue";
		if ( showNames )
			fromText.append(Qt::escape(from));
		
		fromText.append("</font></td><td align='right' style='background-image : url(front.png);'><font color='grey'>");
		if ( date.isNull() )
		{
			fromText.append(generateTime());
		}
		else
		{
			QString dateString;
			switch(timestamp)
				{
				case 0:
					dateString =  QString(date.toString("hh:mm:ss dd/MM/yyyy"));
					break;
				case 1:
					dateString =  QString(date.toString("hh:mm:ss"));
					break;
				case 2:
					dateString =  QString(date.toString("hh:mm ddd/MMM/yyyy"));
					break;
					
				default:
					dateString =  QString( date.toString("hh:mm:ss") );
				}
			fromText.append(dateString);
		}
		
		fromText.append("</font></td></tr></tbody></table>");
		
		if (  in )
		{
		if ( !contactAvatarPath.isEmpty() )
		{
			fromText.append("<img height='50' width='50' align='left' src='"+ contactAvatarPath +"' hspace='10'>");
		}
		else
			fromText.append("<br>");
		} else {
			
			if ( !ownerAvatarPath.isEmpty() )
			{
				fromText.append("<img height='50' width='50' align='right' src='"+ ownerAvatarPath +"' hspace='10'>");
			}
			else
				fromText.append("<br>");
		}
		
		ui.messageChat->append(fromText);
		
		if ( message.startsWith("<qimage>") && message.endsWith("</qimage>"))
		{
			ui.messageChat->insertImage(message);
		}
		else
			ui.messageChat->append("<font color='#525252' >" + checkForEmoticons(Qt::escape(message)) + "</font><br clear=all>");
	} else {
		
		QString str;

			QString color;
			QString dateString;
			if ( in )
				color = "red";
			else
				color = "blue";

			if ( date.isNull() )
			{
				dateString = generateTime();
			}
			else
			{
				switch(timestamp)
					{
					case 0:
						dateString =  QString(date.toString("hh:mm:ss dd/MM/yyyy"));
						break;
					case 1:
						dateString =  QString(date.toString("hh:mm:ss"));
						break;
					case 2:
						dateString =  QString(date.toString("hh:mm ddd/MMM/yyyy"));
						break;
						
					default:
						dateString =  QString( date.toString("hh:mm:ss") );
					}
			}



			if(showNames) {
				str = QString("<font color='%1' size='3'><b>%2: ").arg(color).arg(Qt::escape(from));
				fromText.append(str);
			}
			
			str = QString("(%2)</b> ").arg(dateString);
			fromText.append(str);
			
			ui.messageChat->insertHtml("<br>");
			ui.messageChat->textCursor().insertImage(":/icons/crystal_project/message.png");
			ui.messageChat->insertHtml("&nbsp;");
			ui.messageChat->insertHtml(fromText);
			
			if ( message.startsWith("<qimage>") && message.endsWith("</qimage>"))
				{
//					ui.messageChat->append(fromText);
					ui.messageChat->insertImage(message);
				}
				else {
//					fromText.append("<br><font color='"+ textColor.name() +"' >" + checkForEmoticons(Qt::escape(message)) + "</font><clear=all>");
//					ui.messageChat->append(fromText);
					ui.messageChat->append("<font color='"+ textColor.name() +"' >" + checkForEmoticons(Qt::escape(message)) + "</font><clear=all>");
				}
		
	}
	
//	fromText.append("<font color='#525252' >" + Qt::escape(message) + "</div></font>");
	
//	ui.messageChat->append(fromText);
	
//	ui.messageChat->insertPlainText("\n" + message);
//	ui.messageChat->append("<font color='black' size='3'>" + Qt::escape(message) + "</font>");
	
//	int vPos = ui.messageChat->verticalScrollBar()->value();
	
	typingIcon(false);
	ui.messageChat->setLineWrapColumnOrWidth(ui.messageChat->lineWrapColumnOrWidth());
//	ui.messageChat->moveCursor(QTextCursor::End);
//	ui.messageChat->ensureCursorVisible();
	ui.messageChat->verticalScrollBar()->setValue( ui.messageChat->verticalScrollBar()->maximum() );
}

void chatWindow::on_messageEdit_textChanged()
{
	
//	ui.countLabel->setText(QString::number(ui.messageEdit->toPlainText().count()));
//	qDebug()<<QString::number(ui.messageEdit->toPlainText().count());
//	ui.countLabel->setNum(ui.messageEdit->toPlainText().count());
	textChanged = true;
	if ( sendTyping )
	{
		if ( ! typingChanged )
		{
			typingChanged = true;
			
			emit sendTypingNotification(chatWith, 0x0002);
			QTimer::singleShot(5000, this, SLOT(typingNow()));
		}
		
		
		if ( ui.messageEdit->toPlainText().isEmpty() )
		{
			typingChanged = false;
			emit sendTypingNotification(chatWith, 0x0000);
		}
	}
	
	if ( online && !ui.messageEdit->toPlainText().isEmpty() )
		ui.sendButton->setEnabled(true);
	else 
		ui.sendButton->setEnabled(false);
}

void chatWindow::on_sendButton_clicked()
{
	ui.messageChat->moveCursor(QTextCursor::End);
	quint64 cursorPosition = 0;
	if ( !lightVersion)
	{
		QString from;
		
		from.append("<table width = '100%' border='0' cellspacing='0' cellpadding='0'><tbody><tr><td style='background-image : url(front.png);'><font color='grey' size='3'>");
		if ( showNames )
	//		from.append("<font color='blue'>" + accountNickName +": </font>" );
			from.append(Qt::escape(accountNickName));
	//	from.append("<font color='blue'>" + generateTime() +"</font>");
		from.append("</font></td><td align='right' style='background-image : url(front.png);'><font color='grey'>");
		from.append(generateTime());
		from.append("</font></td></tr></tbody></table>");
		
		if ( !ownerAvatarPath.isEmpty())
		{
			from.append("<img height='50' width='50' align='right' src='"+ ownerAvatarPath +"' hspace='10'>");
		}
		else
			from.append("<br>");
		
		
		
		
		ui.messageChat->append(from);
	//	ui.messageChat->insertPlainText("\n" + ui.messageEdit->toPlainText());
		cursorPosition = ui.messageChat->textCursor().position();
		ui.messageChat->textCursor().insertImage(":/icons/crystal_project/message.png");
		ui.messageChat->insertHtml("<font color='#525252' size='3'> " + checkForEmoticons(Qt::escape(ui.messageEdit->toPlainText())) + "</font>");
	
	} else {

		QString str;
		QString fromText;
		QString dateString;
		ui.messageChat->insertHtml("<br>");
		cursorPosition = ui.messageChat->textCursor().position();
		ui.messageChat->textCursor().insertImage(":/icons/crystal_project/message.png");
		ui.messageChat->insertHtml("&nbsp;");
		dateString = generateTime();

		if(showNames) {
			str = QString(" <font color='blue' size='3'><b> %1: ").arg(Qt::escape(accountNickName));
			fromText.append(str);
		}
		str = QString("(%1)</b> ").arg(dateString);
		fromText.append(str);
		
		ui.messageChat->insertHtml(fromText);
		ui.messageChat->append("<font color='" + textColor.name() + "' >" + checkForEmoticons(Qt::escape(ui.messageEdit->toPlainText())) + "</font><clear=all>");
//			ui.messageChat->append(fromText);
		
	}
	
	messageFormat msg;
	msg.fromUin = chatWith;
	msg.message = ui.messageEdit->toPlainText();
	msg.position = cursorPosition;
	ui.messageChat->moveCursor(QTextCursor::End);
	ui.messageChat->ensureCursorVisible();
	ui.messageChat->setLineWrapColumnOrWidth(ui.messageChat->lineWrapColumnOrWidth());
	ui.messageChat->verticalScrollBar()->setValue( ui.messageChat->verticalScrollBar()->maximum() );
//	emit sendMessage(msg);
	
	QString tmp = msg.message;
	if (msg.message.length() > 1272)
	{
		while (tmp.length() > 1272)
		{
			QString nmsg = tmp;
			nmsg.remove(1272, nmsg.length() - 1272);
			msg.message = nmsg;
			emit sendMessage(msg);
			tmp.remove(1, 1272);
		}
	}
	if (tmp.length() > 0)
	{
		msg.message = tmp;
		emit sendMessage(msg);
	}
	
	ui.messageEdit->clear();
	if ( closeOnSend )
		close();
	if ( sendTyping )
	{
		typingChanged = false;
		emit sendTypingNotification(chatWith, 0x0000);
	}
	
	ui.sendButton->setEnabled(false);
	
}

void chatWindow::setOnline(bool o)
{
	online = o;
	
	if (o)
	{
		ui.imageButton->setEnabled(true);
		ui.sendFileButton->setEnabled(true);
	}
	else
	{
		ui.imageButton->setEnabled(false);
		ui.sendFileButton->setEnabled(false);	
	}
	
	if (online && !ui.messageEdit->toPlainText().isEmpty() )
		ui.sendButton->setEnabled(true);
	else
		ui.sendButton->setEnabled(false);
}

QString chatWindow::generateTime()
{
	QDateTime today = QDateTime::currentDateTime();
	switch(timestamp)
	{
	case 0:
		return QString(today.toString("hh:mm:ss dd/MM/yyyy"));
		break;
	case 1:
		return QString(today.toString("hh:mm:ss"));
		break;
	case 2:
		return QString(today.toString("hh:mm ddd/MMM/yyyy") );
		break;
		
	default:
		return QString(today.toString("hh:mm:ss"));
	}
	return QString ();
}

void chatWindow::setOnEnter(bool f)
{
	sendOnEnter = f;
	ui.sendOnEnterButton->setChecked(f);
	if ( sendOnEnter )
	{
		ui.sendButton->setShortcut(QKeySequence(Qt::Key_Return));
		ui.messageEdit->onEnter = true;
	} else {
		ui.sendButton->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Return));
		ui.messageEdit->onEnter = false;
	}
}

void chatWindow::focusEditMessage()
{
	ui.messageEdit->setFocus();
}

void chatWindow::focusInEvent(QFocusEvent *e)
{
	
	emit windowFocused(chatWith);
	QWidget::focusInEvent(e);
}

void chatWindow::typingNow()
{
	if ( sendTyping )
	{
		if ( textChanged )
		{
			textChanged = false;
			emit sendTypingNotification(chatWith, 0x0001);
			QTimer::singleShot(5000, this, SLOT(typingNow()));
		} else {
			typingChanged = false;
			emit sendTypingNotification(chatWith, 0x0000);
		}
	}
}

void chatWindow::restoreState()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+uin, "account");
	ui.splitter->restoreState(settings.value("chatwindow/state").toByteArray());
}

void chatWindow::typingIcon(bool f)
{
	if ( f )
	{
//		ui.typingLabel->setText("<img src=':/icons/crystal_project/typing.png'>");
		ui.clientLabel->setText(tr("<font color='red'>Typing...</font>"));
	} else
	{
//		ui.typingLabel->clear();
		ui.clientLabel->setText(contactClient);
	}
}

void chatWindow::on_historyButton_clicked()
{
	emit showHistory(chatWith);
}

void chatWindow::setServiceMessage(const QString &from, const QString &msg, const QDateTime &date)
{
	ui.messageChat->moveCursor(QTextCursor::End);
	QString fromText;
//	fromText.append("<font color='grey' size = '2'>");
	fromText.append("<table width='100%'><tbody><tr><td style='background-image:url(front.png);' ><font color='grey' size='2'>");

		QString dateString;
		switch(timestamp)
			{
			case 0:
				dateString =  QString("(" + date.toString("hh:mm:ss dd/MM/yyyy") + ")" );
				break;
			case 1:
				dateString =  QString("(" + date.toString("hh:mm:ss") + ")" );
				break;
			case 2:
				dateString =  QString("(" + date.toString("hh:mm ddd/MMM/yyyy") + ")" );
				break;
				
			default:
				dateString =  QString("(" + date.toString("hh:mm:ss") + ")" );
			}
		if ( showNames )
//			fromText.append(" " + from +": " );
			fromText.append(" " + from );
		fromText.append(dateString);
	fromText.append(" " + msg + "</font></td></tr> </tbody></table>");
	ui.messageChat->append(fromText);
	ui.messageChat->moveCursor(QTextCursor::End);
	ui.messageChat->ensureCursorVisible();	
	ui.messageChat->setLineWrapColumnOrWidth(ui.messageChat->lineWrapColumnOrWidth());
}

void chatWindow::keyPressEvent(QKeyEvent *event)
{
	if ( event->key() == Qt::Key_Escape || (event->key() == Qt::Key_W && event->modifiers() == Qt::CTRL))
		close();
	if ( (event->key() == Qt::Key_N && event->modifiers() == Qt::CTRL) || (event->key() == Qt::Key_Up && event->modifiers() == Qt::CTRL))
		emit nextTab(1);
	if ( (event->key() == Qt::Key_P && event->modifiers() == Qt::CTRL) || (event->key() == Qt::Key_Down && event->modifiers() == Qt::CTRL))
		emit nextTab(-1);
	if ((event->key() == Qt::Key_R && event->modifiers() == Qt::CTRL))
	    translateMessage();
}

void chatWindow::on_infoButton_clicked()
{
	emit openInfoWindow(chatWith);
}

void chatWindow::setAvatars(const QString &userAvatar)
{
	if ( QFile::exists(userAvatar) )
	{
		contactAvatarPath = userAvatar;
//		contactAv->setFileName(contactAvatarPath);
//		contactAv->setScaledSize(QSize(48,48));
		ui.avatarLabel->setText("<img src='" + contactAvatarPath + "' width = 48 height = 48>");
//		ui.avatarLabel->setMovie(contactAv);
//		contactAv->setCacheMode(QMovie::CacheAll);
//		contactAv->start();
	}
	else
		contactAvatarPath.clear();
}

void chatWindow::showEvent ( QShowEvent * event )
{
	ui.messageChat->setLineWrapColumnOrWidth(ui.messageChat->lineWrapColumnOrWidth());
	ui.messageChat->verticalScrollBar()->setValue( ui.messageChat->verticalScrollBar()->maximum() );
//	ui.messageChat->moveCursor(QTextCursor::End);
//	ui.messageChat->ensureCursorVisible();
	ui.messageEdit->setFocus();
	QWidget::showEvent(event);

}

QString chatWindow::findUrls(const QString &sourceHTML)
{
    QString html = sourceHTML;
     QRegExp linkRegExp("((https?|ftp://|www\\.)[^\\s]+)");
     linkRegExp.setCaseSensitivity(Qt::CaseInsensitive);
     int pos = 0;
     while(((pos = linkRegExp.indexIn(html, pos)) != -1))
     {
    	 QString link = linkRegExp.cap(0);
    	 int linkCount = link.count();
         QString tmplink = link;
         if (tmplink.toLower().startsWith("www."))
         {
             tmplink = "http://" + tmplink;
         }
         link = "<a href=\"" + tmplink + "\" target=\"_blank\">" + link + "</a>";
         html.replace(pos, linkCount, link);
         pos += link.count();
    }
     QRegExp mailRegExp("([a-zA-Z0-9\\-\\_\\.]+@([a-zA-Z0-9\\-\\_]+\\.)+[a-zA-Z]+)");
     pos = 0;
     while(((pos = mailRegExp.indexIn(html, pos)) != -1))
     {
        QString mailAddr = mailRegExp.cap(0);
        int mailAddrCount = mailAddr.count();
        mailAddr = "<a href=\"mailto:" + mailAddr + "\">" + mailAddr + "</a>";
        if ((pos == 0) || ((pos != 0) && (html[pos - 1] != ':')))
        {
           html.replace(pos, mailAddrCount, mailAddr);
        }      
        pos += mailAddr.count();
     }
     return html;
}

void chatWindow::setOwnerAvatar(const QString &userAvatar)
{
	if ( QFile::exists(userAvatar) )
	{
		ownerAvatarPath = userAvatar;
		ui.ownAvatarLabel->setText("<img src='" + ownerAvatarPath + "' width = 40 height = 40>");
//		mineAv->setFileName(ownerAvatarPath);
//		mineAv->setScaledSize(QSize(40,40));
//		ui.ownAvatarLabel->setMovie(mineAv);
//		mineAv->setCacheMode(QMovie::CacheAll);
//		mineAv->start();
	}
	else
	{
		ownerAvatarPath.clear();
		ui.ownAvatarLabel->clear();
	}
}

void chatWindow::setEmoticonPath(const QString &path)
{
	emoticonList.clear();
	ui.messageChat->clearMovieList();
	QFile file(path);
	
	QString dirPath = path.section('/', 0, -2);
	QDir dir ( dirPath );
	
	QStringList fileList = dir.entryList(QDir::Files);
	
	if (file.exists() && file.open(QIODevice::ReadOnly) )
	{
		QDomDocument doc;
		if ( doc.setContent(&file) )
		{
			QDomElement rootElement = doc.documentElement();
			int emoticonCount = rootElement.childNodes().count();
			
			QDomElement emoticon = rootElement.firstChild().toElement();
			
			for ( int i = 0; i < emoticonCount ; i++ )
			{
				
				if ( emoticon.tagName() == "emoticon")
				{
					
					QStringList fileName = fileList.filter(emoticon.attribute("file"));
					if ( !fileName.isEmpty())
					{
					
//						qDebug()<<fileName.at(0);
						emoticonList.insert(fileName.at(0),emoticon.firstChild().toElement().text());
						

						
						int stringCount = emoticon.childNodes().count();
						QDomElement emoticonString = emoticon.firstChild().toElement();
						QStringList regExp;
						for(int j = 0; j < stringCount; j++)
						{
							if ( emoticonString.tagName() == "string")
							{
//								regExp.append("[" + emoticonString.text() + "]");
//								regExp.append(emoticonString.text());
								regExp<<emoticonString.text();

//								if ( j < (stringCount - 1))
//									regExp.append("|");
//								else
//									regExp.append(")");
							}
							
							emoticonString = emoticonString.nextSibling().toElement();
						}
						if ( regExp.count() )
						{
							regExpList.insert(fileName.at(0) + "-emoticon", regExp);

							urls.insert(fileName.at(0) + "-emoticon", dirPath + "/" + fileName.at(0));
						}
					}
					
				}
				emoticon = emoticon.nextSibling().toElement();
					
			}
			
		}
		
		
		emoticonWidget->setEmoticons(emoticonList, dirPath);
	}
}
void chatWindow::insertSmile(const QString &smile)
{
	ui.messageEdit->insertPlainText(" " + smile + " ");
}

QString chatWindow::checkForEmoticons(const QString &_source)
{
	QString source = _source;
	foreach(QStringList reg, regExpList)
	{
		foreach(QString regExp,reg )
		{
			QRegExp linkRegExp("((https?://|ftps?://|www\\.)((([\\S]+(\\:[\\S]@+))?([\\w-\\.]+)\\.[a-zA-Z]{2,5})|(([0-9]{1,3}\\.){3}([0-9]{1,3})))(\\:[0-9]{1,5})?\\/?(([\\S])?)+)");
			linkRegExp.setCaseSensitivity(Qt::CaseInsensitive);

			int posSmile = 0;	
			while ((posSmile = source.indexOf(Qt::escape(regExp), posSmile)) != -1)
			{
				int pos = 0;
				bool ok = true;
				while((pos = linkRegExp.indexIn(source, pos)) != -1)
				{
					QString link = linkRegExp.cap(0);
					int linkCount = link.count();
					if ((posSmile >= pos) && (posSmile <= (pos + linkCount))){
						ok = false;
						posSmile += linkCount;
						break;
					}
					pos += linkCount;
				}
				
				if (ok){
					source.replace(posSmile, Qt::escape(regExp).length(), "<img src='" + regExpList.key(reg) + "'>");
					QString str = "<img src='" +  regExpList.key(reg) + "'>";
					posSmile += str.length();
					ui.messageChat->addSmile(regExpList.key(reg), urls.value(regExpList.key(reg)));
				}
			}

		}
	}
	return source = findUrls(source).replace("\n", "<br>");
}

void chatWindow::on_imageButton_clicked()
{
	QString fileName =   QFileDialog::getOpenFileName(this, tr("Open File"),
			 "",
	                                                 tr("Images (*.gif *.bmp *.jpg *.jpeg *.png)"));
	
	if ( !fileName.isEmpty())
		{
			QFile iconFile(fileName);

				if ( iconFile.size() > (7 * 1024 + 600))
				{
					QMessageBox::warning(this, tr("Open error"),
					                   tr("Image size is too big"));
				}
				else
				{
					emit sendImage(chatWith,fileName);
					
					if ( QFile::exists(fileName))
					{
					
					ui.messageChat->moveCursor(QTextCursor::End);
					
					QString from;
					
					from.append("<table width = '100%' border='0' cellspacing='0' cellpadding='0'><tbody><tr><td style='background-image : url(front.png);'><font color='grey' size='3'>");
					if ( showNames )
						from.append(accountNickName);
					from.append("</font></td><td align='right' style='background-image : url(front.png);'><font color='grey'>");
					from.append(generateTime());
					from.append("</font></td></tr></tbody></table>");
					
					if ( !ownerAvatarPath.isEmpty())
					{
						from.append("<img height='50' width='50' align='right' src='"+ ownerAvatarPath +"' hspace='10'>");
					}
					else
						from.append("<br>");
					
					
					
					
					ui.messageChat->append(from);
					
					QFile file(fileName);
					if (file.open(QIODevice::ReadOnly))
					{
					
					ui.messageChat->insertImage("<qimage>" + file.readAll().toHex() + "</qimage>");
					file.close();
					}
					}
					
				}
		}
}

void chatWindow::translateMessage()
{
	   QString txt = ui.messageEdit->toPlainText();      
	   if (ui.messageEdit->textCursor().hasSelection()){
	      QString SelText = ui.messageEdit->textCursor().selectedText();
	      SelText = invertMessage(SelText);
	      txt.replace(ui.messageEdit->textCursor().selectionStart(), SelText.length(), SelText);
	   } else {
	      txt = invertMessage(txt);
	   }
	   
	   ui.messageEdit->clear();
	   ui.messageEdit->insertPlainText(txt);
}

void chatWindow::on_quoteButton_clicked()
{
	QString selectedText = ui.messageChat->textCursor().selectedText();
	// Prepearing the string for quoting
	if ( !selectedText.isEmpty() )
	{
		// Replace paragraph separators
		selectedText.replace(QChar::ParagraphSeparator, "\n>  ");
		// Qt internally uses U+FDD0 and U+FDD1 to mark the beginning and the end of frames.
		// They should be seen as non-printable characters, as trying to display them leads
		// to a crash caused by a Qt "noBlockInString" assertion.
		selectedText.replace(QChar(0xFDD0), " ");
		selectedText.replace(QChar(0xFDD1), " ");
		// Prepend text with ">" tag
		QStringList list = selectedText.split(QChar(0x2028));
		foreach (QString mes, list)
			ui.messageEdit->insertPlainText(mes.prepend("> ") + "\n");
		ui.messageEdit->moveCursor(QTextCursor::End);
		ui.messageEdit->ensureCursorVisible();	
		ui.messageEdit->setFocus();
	}
}

QString chatWindow::invertMessage(QString &text)
{
   QString tableR=tr("qwertyuiop[]asdfghjkl;'zxcvbnm,./QWERTYUIOP{}ASDFGHJKL:\"ZXCVBNM<>?");
   QString tableE="qwertyuiop[]asdfghjkl;'zxcvbnm,./QWERTYUIOP{}ASDFGHJKL:\"ZXCVBNM<>?";
   QString txt = text;
   for(int i = 0; i < txt.length(); i++)
   {
      if(txt.at(i) <= QChar('z'))
      {
         int j = 0; bool b=true;
         while((j < tableE.length()) && b)
         {
            if(txt[i] == tableE[j]) 
            {
               b = false;
               txt[i] = tableR[j];
            }
            j++;
         }
      }else{
         int j = 0; bool b = true;
         while((j < tableR.length()) && b)
         {
            if(txt[i] == tableR[j])
            {
               b = false;
               txt[i] = tableE[j];
            }
            j++;
         }
      }
   }
   return txt;
}
void chatWindow::on_clearButton_clicked()
{
	ui.messageChat->setText("<html>");
}

void chatWindow::fontChangeActionTriggered()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, textFont, this);
	if ( ok && font != textFont )
	{
		textFont = font;
		emit sendFontSignal(textFont, textColor, backroundColor);
	}
}

void chatWindow::fontChangeColorActionTriggered()
{
	QColor color = 	QColorDialog::getColor(textColor, this); 
	if ( color.isValid() && color != textColor )
	{
		textColor.setRgb(color.rgb());
		emit sendFontSignal(textFont, textColor, backroundColor);
	}
}

void chatWindow::setWindowFont(const QFont &f, const QColor &fc, const QColor &c)
{
	textFont = f;
	textColor = fc;
	backroundColor = c;
//	ui.messageEdit->setTextColor(fc);
	
	ui.messageChat->setFont(f);
	ui.messageEdit->setFont(f);
	ui.messageChat->setTextColor(fc);

//	ui.messageChat->setTextBackgroundColor(c);
//	ui.messageEdit->setTextBackgroundColor(c);
	ui.messageChat->setStyleSheet("background-color:" + c.name());
	ui.messageEdit->setStyleSheet("background-color:" + c.name() + ";color :" + fc.name());
}

void chatWindow::on_backgroundButton_clicked()
{
	QColor color = 	QColorDialog::getColor(backroundColor, this); 
	if ( color.isValid() && color != backroundColor )
	{
		backroundColor.setRgb(color.rgb());
		emit sendFontSignal(textFont, textColor, backroundColor);
	}
}

void chatWindow::on_sendOnEnterButton_clicked()
{
	sendOnEnter = ui.sendOnEnterButton->isChecked();
	setOnEnter(sendOnEnter);
}

void chatWindow::on_sendTypingButton_clicked()
{
	sendTyping = ui.sendTypingButton->isChecked();
	
}

void chatWindow::messageApply(quint64 position)
{
	QTextCursor cursor = ui.messageChat->textCursor();
    cursor.setPosition(position, QTextCursor::MoveAnchor);
    cursor.deleteChar();
    ui.messageChat->setTextCursor(cursor);
	ui.messageChat->textCursor().insertImage(":/icons/crystal_project/message_accept.png");
	ui.messageChat->moveCursor(QTextCursor::End);
	ui.messageChat->ensureCursorVisible();
	ui.messageChat->setLineWrapColumnOrWidth(ui.messageChat->lineWrapColumnOrWidth());
	ui.messageChat->verticalScrollBar()->setValue( ui.messageChat->verticalScrollBar()->maximum() );
}
