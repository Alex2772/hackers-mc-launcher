#pragma once
#include <QString>

class QFile;

namespace  Crypto
{
	QString sha1(QFile& file);
	QString md5(const QString& from);
};
