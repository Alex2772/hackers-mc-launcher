#include "UIThread.h"

void UIThread::execute(void* function)
{
	auto f = reinterpret_cast<std::function<void()>*>(function);
	(*f)();
	delete f;
}

void UIThread::run(const std::function<void()>& function)
{
	static UIThread t;
	QMetaObject::invokeMethod(&t, "execute", Qt::AutoConnection, Q_ARG(void*, new std::function<void()>(function)));
}
