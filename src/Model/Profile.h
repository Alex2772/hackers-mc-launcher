#pragma once
#include <QUrl>
#include "DownloadsModel.h"
#include "ClasspathListModel.h"

class HackersMCLauncher;
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

	static QString javaLibNameToPath(const QString& name);
	void cleanup();
	static void fromJson(HackersMCLauncher* launcher, Profile& p, const QString& name, const QJsonObject& object);
	QJsonObject toJson();
	void save(HackersMCLauncher* launcher);
	QString id() const;

	bool mIsFullscreen = false;
	unsigned short mWindowWidth = 854;
	unsigned short mWindowHeight = 500;
};