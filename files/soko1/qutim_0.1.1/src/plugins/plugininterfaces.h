//#ifndef PLUGININTERFACES_H_
//#define PLUGININTERFACES_H_

//#include <QObject>
//class QTreeWidget;
//class QStackedWidget;
//class QAction;
//
//class fileTransferSignals : public QObject
//{
//	Q_OBJECT
//public:
//	fileTransferSignals(QObject *parent = 0 ) : QObject(parent){}
//	~fileTransferSignals(){}
//	
//	void emitSendFile(const QString &t){
//		emit sendFile(t);
//	}
//signals:
//	void sendFile(const QString &);
//};
//
//
//
// class FileTransferInterface 
// {
//
// public:
//     virtual ~FileTransferInterface() {}
//     virtual void addSettings(QTreeWidget *, QStackedWidget *) = 0;
//     virtual void deleteSettings() = 0;
//     virtual QAction *getSendFileAction() = 0;
//     virtual void removeAll() = 0;
//     virtual fileTransferSignals *getSignalObject() = 0;
//     virtual QObject *instance()=0;
//
//
// };
//
// Q_DECLARE_INTERFACE(FileTransferInterface,
//                     "plugins.filetransfer/1.0")
//
//#endif /*PLUGININTERFACES_H_*/
