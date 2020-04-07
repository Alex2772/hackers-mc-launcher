#pragma once
#include <QUrl>
#include "JavaLibModel.h"

class Profile
{
public:
	class GameArg
	{
	public:
		QString mName;
		QString mValue;
		QList<QPair<QString, QVariant>> mConditions;
	};

	/**
	 * \brief not all arguments have it's own value
	 */
	class JavaArg
	{
	public:
		QString mName;
		QList<QPair<QString, QVariant>> mConditions;
	};
	
	QString mName;

	QMap<QUrl, QList<JavaLib>> mJavaLibs;
	QList<GameArg> mGameArgs;
	QList<JavaArg> mJavaArgs;
};
