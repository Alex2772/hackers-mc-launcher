#include "CommonUtils.h"
#include <QJsonObject>
#include <QObject>
#include <Qurl>

QString CommonUtils::determineName(const QJsonObject& object)
{
	auto manifest = object["manifest"];
	if (manifest.isObject())
	{
		if (manifest["name"].isString())
		{
			return manifest["name"].toString();
		}
	}
	if (object["url"].isString())
	{
		auto url = object["url"].toString();
		
		if (url.startsWith("https://launchermeta.mojang.com/"))
		{
			return QObject::tr("Official Minecraft");
		} else
		{
			return QUrl(url).host();
		}
	}
	
	return "unknown";
}
