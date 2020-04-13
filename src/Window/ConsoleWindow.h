#pragma once
#include <QFrame>

class QProcess;

class ConsoleWindow: public QFrame
{
	Q_OBJECT
public slots:
	void onClose();
public:
	ConsoleWindow(QProcess* process);
	virtual ~ConsoleWindow() = default;
};
