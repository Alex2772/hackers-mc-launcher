#pragma once

#include "Model/DownloadsModel.h"
#include "Settings.h"
#include "Model/Profile.h"
#include "Model/User.h"

class QDir;
class HackersMCLauncher;

class DownloadHelper
{
private:
	typedef QPair<QString, QUrl> D; // just for convenience

	QList<D> mDownloadList;
	quint64 mTotalDownloadSize = 0;

	HackersMCLauncher* mLauncher;
	QDir mGameDir;


public:
	explicit DownloadHelper(HackersMCLauncher* launcher);

	void addDownloadList(const QList<Download>& downloads, bool withHashCheck);
	void performDownload();
	void gameDownload(const ::Profile& profile, const User& user, bool withUpdate);
	void setStatusUI(const QString& status);
};
