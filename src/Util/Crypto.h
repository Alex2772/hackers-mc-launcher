#pragma once
#include <QString>

class QFile;

namespace  Crypto
{
	QString sha1(QFile& file);
	QString sha1(const QByteArray& data);
	
	QString md5(const QString& from);
};
