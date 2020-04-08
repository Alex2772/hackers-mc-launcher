#pragma once

#include <QString>

namespace  StringHelper
{
	void normalizeUrl(QString& url);
	QString prettySize(quint64 size, bool isSpeed = false);
};
