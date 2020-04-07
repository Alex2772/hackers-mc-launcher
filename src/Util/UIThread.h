#pragma once
#include <QObject>
#include <functional>



class UIThread : public QObject
{
	Q_OBJECT
private slots:
	void execute(void* function);
private:
	UIThread() = default;
	
public:
	static void run(const std::function<void()>& function);
};
