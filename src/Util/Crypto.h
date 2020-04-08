#pragma once
#include <QString>

class QFile;

namespace  Crypto
{
	QString sha1(QFile& file);
};
