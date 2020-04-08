#include "StringHelper.h"
#include <QObject>

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


/**
 * \brief ALMOST EVERY MY PROJECT HAS THIS FUNCTION WHICH
 *        CREATES PRETTY FORMATTED STRING FROM BYTES OR BYTES/SEC
 *
 *        HOLY SHIT STROUSTRUP INCLUDE THIS FUNCTION INTO STD FINALLY
 *
 * \param size size in bytes or speed in bytes/sec
 * \param isSpeed selects regular bytes or speed
 * \return pretty formatted string
 */
QString StringHelper::prettySize(quint64 size, bool isSpeed)
{
	long double fsize = size;
	long double power = log(fsize) / log(2);
	long double index = std::max(floor(power / 10 - 0.01), long double(0));

	QVector<QString> strs;
	if (isSpeed)
	{
		strs = {
			QObject::tr("B/sec"),
			QObject::tr("Kb/sec"),
			QObject::tr("Mb/sec"),
			QObject::tr("Gb/sec"),
			QObject::tr("Tb/sec"),
			QObject::tr("Pb/sec")
		};
	}
	else
	{
		strs = {
			QObject::tr("B"),
			QObject::tr("Kb"),
			QObject::tr("Mb"),
			QObject::tr("Gb"),
			QObject::tr("Tb"),
			QObject::tr("Pb")
		};
	}

	uint32_t i = index;

	return QString("%1 %2").arg(double(size / pow(1024, index)), 0, 'f', 1).arg(i < strs.size() ? strs[i] : "??");
}
