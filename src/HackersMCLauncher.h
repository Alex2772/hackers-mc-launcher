#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_HackersMCLauncher.h"
#include "Model/UsersListModel.h"
#include "Model/RepositoriesModel.h"
#include "Model/ProfilesListModel.h"
#include <QNetworkAccessManager>
#include <qprocess.h>

class Settings;

class HackersMCLauncher : public QMainWindow
{
	Q_OBJECT

		friend class DownloadHelper;
signals:
	void closeConsoleWindows();
protected:
	void closeEvent(QCloseEvent* event) override;

public:
	HackersMCLauncher(QWidget *parent = Q_NULLPTR);
	bool nativeEvent(const QByteArray& eventType, void* message, long* result) override;
	void showEvent(QShowEvent* event) override;
	void changeEvent(QEvent* e) override;
	void mousePressEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
	
	void updateMiddleButton();


	RepositoriesModel* getRepositories();
	ProfilesListModel& getProfiles();
	
	Settings* getSettings() const;
	bool currentProfile(Profile& p);
	bool currentUser(User&);
	bool tryLoadProfile(Profile& dst, const QString& name);
	
public slots:
	void resetDownloadIndicators();
	void installPackage(const QUrl& url);
	void downloadJava();
	void play(bool withUpdate = false);
	void loadProfiles();
	void saveProfiles();
	
private:
	Ui::HackersMCLauncherClass ui;
	int mUiScale = 1;

	UsersListModel mUsers;
	ProfilesListModel mProfiles;
	RepositoriesModel mRepos;
	Settings* mSettings;
	QNetworkAccessManager mNetwork;

	QList<QProcess*> mProcesses;
	QProcess* createProcess();
	void removeProcess(QProcess* p, int status, QProcess::ExitStatus e);
private slots:
	void screenScaleChanged();
	void setDownloadMode(bool m);
	void updatePlayButton();
};
