#pragma once
#include <QSet>

namespace Util
{
	template<typename T>
	void cleanup(QList<T>& c)
	{
		QSet<T> set = c.toSet();
		c = set.values();
	}
}
