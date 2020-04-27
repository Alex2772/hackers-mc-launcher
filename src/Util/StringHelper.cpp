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

QString StringHelper::markdownToHtml(const QString& markdown)
{
	auto lines = markdown.split("\n");
	QString result;

	unsigned openedTagsStack = 0;
	QString sectionTag;
	
	for (auto line : lines)
	{
		if (line.isEmpty())
		{
			if (!sectionTag.isEmpty())
			{
				result += "</" + sectionTag + ">";
				sectionTag.clear();
			}
			continue;
		}
		
		bool listItem = false;
		QString resultLine;

		bool wasEmpty = sectionTag.isEmpty();
		if (line.startsWith("* "))
		{
			// list item
			listItem = true;
			line = line.mid(2);
			resultLine += "<li>";

			if (sectionTag.isEmpty())
			{
				sectionTag = "ul";
			}
		} else
		{
			sectionTag = "p";
		}
		if (wasEmpty)
		{
			result += "<" + sectionTag + ">";
		}
		
		for (auto it = line.begin(); it != line.end(); ++it)
		{
			if (*it == '*')
			{
				QChar nextChar = *std::next(it);
				
				char tag = 'i';
				if (nextChar == '*')
				{
					// bold
					tag = 'b';
				}
				
				result += '<';
				if (openedTagsStack)
				{
					result += '/';
				}
				openedTagsStack = !openedTagsStack;
				result += tag;
				result += '>';
			} else
			{
				resultLine += *it;
			}
		}
		if (listItem)
		{
			resultLine += "</li>";
		}
		result += resultLine;
	}

	if (!sectionTag.isEmpty())
	{
		result += "</" + sectionTag + ">";
		sectionTag.clear();
	}
	
	return result;
}
