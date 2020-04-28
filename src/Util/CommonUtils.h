#pragma once
#include <QString>

class QJsonObject;

namespace CommonUtils
{
	QString determineName(const QJsonObject& object);
};
