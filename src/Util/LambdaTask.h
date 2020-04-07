#pragma once
#include <qrunnable.h>

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
		mData();
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