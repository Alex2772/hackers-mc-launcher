#include "StringHelper.h"

void StringHelper::normalizeUrl(QString& url)
{
	if (!url.startsWith("http"))
	{
		url = "http://" + url;
	}
	if (url.endsWith("/"))
	{
		url.resize(url.size() - 1);
	}
}
