#include "HackersMCLauncher.h"
#include <QtWidgets/QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QTranslator translator;
	
	translator.load(":/hck/translations/hackersmclauncher_" + QLocale::system().name());
	QApplication::installTranslator(&translator);
	HackersMCLauncher w;
	w.show();
	return a.exec();
}
