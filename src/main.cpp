#include "HackersMCLauncher.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	HackersMCLauncher w;
	w.show();
	return a.exec();
}
