#include "HackersMCLauncher.h"
#include <QtWidgets/QApplication>
#include <QTranslator>
#include "Util/UIThread.h"
#include <QtPlugin>

#ifndef _DEBUG
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
Q_IMPORT_PLUGIN(QSvgPlugin);
#endif

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	// running empty task to make sure it's internal object will be in main qt thread
	UIThread::run([]() {});
	
	QApplication::setApplicationDisplayName("Hacker's MC Launcher");
	QApplication::setApplicationName("Hackers-mc-launcher");
	QApplication::setApplicationVersion("0.1-alpha");

	QTranslator translator;
	translator.load(":/hck/translations/hackersmclauncher_" + QLocale::system().name());
	QApplication::installTranslator(&translator);
	
	HackersMCLauncher w;
	w.show();
	return a.exec();
}
