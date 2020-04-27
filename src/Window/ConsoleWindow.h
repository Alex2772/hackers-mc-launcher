#pragma once
#include <QFrame>

class QProcess;

class ConsoleWindow: public QFrame
{
	Q_OBJECT
	int mCurrentColor = -1;
	
public slots:
	void onClose();
public:
	ConsoleWindow(QProcess* process);
	virtual ~ConsoleWindow() = default;
};
