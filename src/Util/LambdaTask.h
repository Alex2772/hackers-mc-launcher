#pragma once
#include <qrunnable.h>
#include <QThread>

class Interrupted {};

template <typename T>
class LambdaTask: public QRunnable
{
private:
	T mData;
	
public:
	explicit LambdaTask(const T& data)
		: mData(data)
	{
	}

	void run() override
	{
		try {
			mData();
		} catch (Interrupted) {}
	}
};


/**
 * \brief Helper factory for simple instancing
 */
template<typename T>
static LambdaTask<T>* lambda(const T& data)
{
	return new LambdaTask<T>(data);
}
static void interruptionCheck()
{
	if (QThread::currentThread()->isInterruptionRequested())
	{
		throw Interrupted();
	}
}