//#include "pluginsystem.h"
//
//pluginSystem::pluginSystem(const QString &uin, QObject *parent) : QObject(parent), accountUin(uin)
//{
//	fileTransferPluginExist = false;
//	loadPlugins();
//}
//
//pluginSystem::~pluginSystem()
//{
//	if ( fileTransferPluginExist )
//		fileTransferPlugin->removeAll();
//}
//
//void pluginSystem::loadPlugins()
//{
//	const QDir dir("");
//
//	foreach (QObject *plugin, QPluginLoader::staticInstances())
//	{
//    	FileTransferInterface *iFile = qobject_cast<FileTransferInterface *>(plugin);
//		if (iFile)
//		{
//			fileTransferPluginExist = true;
//			fileTransferPlugin = iFile;
//		}
//	}
//	
//    foreach (QString fileName, dir.entryList(QDir::Files)) {
//        QPluginLoader loader(dir.absoluteFilePath(fileName));
//        QObject *plugin = loader.instance();
//        if (plugin) {
//        	FileTransferInterface *iFile = qobject_cast<FileTransferInterface *>(plugin);
//    		if (iFile)
//    		{
//    			fileTransferPluginExist = true;
//    			fileTransferPlugin = iFile;
//    			connect( fileTransferPlugin->instance(), SIGNAL(sendFile(const QString &)),
//    					this, SLOT(debugMessage(const QString &)));
//    		}
//        }
//    }
//}
//
//void pluginSystem::addSettings(QTreeWidget* settingsTree, QStackedWidget *stack)
//{
//	if ( fileTransferPluginExist )
//	{
//		fileTransferPlugin->addSettings(settingsTree, stack);
//	}
//}
//
//void pluginSystem::deleteSettings()
//{
//	if ( fileTransferPluginExist )
//	{
//		fileTransferPlugin->deleteSettings();
//	}
//}
//
//QAction* pluginSystem::getFileTransferAction()
//{
//	if ( fileTransferPluginExist )
//		return fileTransferPlugin->getSendFileAction();
//	else
//		return 0;
//}
