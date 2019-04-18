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



#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QtGui/QWidget>


#include "ui_chatwindow.h"

class messageFormat;
class emoticonMenu;

class chatWindow : public QWidget
{
    Q_OBJECT

public:
    chatWindow(QWidget *parent = 0);
    ~chatWindow();
    void setMessage(const QString &, const QString &, const QDateTime &, bool in = true);
    void setUin(const QString u) { uin = u; };
    void setContactUin(const QString u) { ui.uinLabel->setText(u); chatWith = u; };
    inline QString getUin() const { return uin; };
    QString contactName;
    void setOnline(bool);
    bool showNames;
    quint8 timestamp;
    void setOnEnter(bool);
    void focusEditMessage();
    bool closeOnSend;  
    bool sendTyping;
    void restoreState();
    void typingIcon(bool);
    QString accountNickName;
    QString chatWith;
    void setAvatars(const QString &);
    void setOwnerAvatar(const QString &);
    void setServiceMessage(const QString &, const QString &, const QDateTime &);
    void setEmoticonPath(const QString &path);
    bool lightVersion;
    void setContactName(const QString &n){ui.nameLabel->setText(n);}
    void setContactClient(const QString &n){contactClient = n; ui.clientLabel->setText(n);}
    void setContactNote(QString n){ui.noteEdit->setText(n.replace("\n", " "));}
    void setWindowFont(const QFont &, const QColor &, const QColor &);
    void setTyping(bool f){sendTyping = f; ui.sendTypingButton->setChecked(f);}
    void messageApply(quint64);
private slots:
	void on_messageEdit_textChanged();
	void on_sendButton_clicked();
	void tst() { emit windowFocused(chatWith);}
	void typingNow();
	void on_historyButton_clicked();
	void on_infoButton_clicked();
	void insertSmile(const QString &);
	void on_imageButton_clicked();
	void on_translateButton_clicked() {translateMessage();}
	void on_sendFileButton_clicked() {emit sendFile(chatWith);}
	void on_quoteButton_clicked();
	void on_clearButton_clicked();
	void fontChangeActionTriggered();
	void fontChangeColorActionTriggered();
	void on_backgroundButton_clicked();
	void on_sendOnEnterButton_clicked();
	void on_sendTypingButton_clicked();
signals:
	void sendMessage(const messageFormat &);
	void windowFocused(const QString &);
	void sendTypingNotification(const QString &, quint16);
	void showHistory(const QString &);
	void openInfoWindow(const QString &);
	void sendImage(const QString &, const QString &);
	void sendFile(const QString &);
	void nextTab(int);
	void sendFontSignal(const QFont&, const QColor &, const QColor &);
protected:
	void focusInEvent( QFocusEvent * event );

	void keyPressEvent ( QKeyEvent * event);
	void showEvent ( QShowEvent * event );
private:
	QString invertMessage(QString &);
	bool typingChanged;
    Ui::chatWindowClass ui;
    QString uin;
    
    bool online;//		if ( movie->frameCount())
    //		{
    QString generateTime();
    bool sendOnEnter;
    QShortcut *sendShortCut;
    icqTextEdit *messageEdit;
    QByteArray saveState;
    bool textChanged;
    QString contactAvatarPath;
    QString findUrls(const QString &);
    QString ownerAvatarPath;
    
    QHash<QString, QString> emoticonList;
    
    QMenu *emoticMenu;
    QWidgetAction *emoticonAction;
    emoticonMenu *emoticonWidget;
    
    QHash<QString, QStringList> regExpList;
    QHash<QString, QString> urls;
    
    QString checkForEmoticons(const QString &);
    void translateMessage();
    
    QString contactClient;
    
    QAction *changeFont;
    QAction *changeFontColor;
    QMenu *fontMenu;
    QFont textFont;
    QColor textColor;
    QColor backroundColor;
    
    QMovie *contactAv;
    QMovie *mineAv;
};

#endif // CHATWINDOW_H
