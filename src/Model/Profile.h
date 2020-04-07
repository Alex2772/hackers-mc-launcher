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
	
	QString mName;

	QMap<QUrl, QList<JavaLib>> mJavaLibs;
	QList<GameArg> mGameArgs;
};
