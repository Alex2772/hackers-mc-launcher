﻿#pragma once
#include <QUrl>
#include "DownloadsModel.h"
#include "ClasspathListModel.h"

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
	QString mMainClass;
	QString mAssetsIndex;

	QList<Download> mDownloads;
	QList<GameArg> mGameArgs;
	QList<JavaArg> mJavaArgs;
	QList<ClasspathEntry> mClasspath;
};
