#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_HackersMCLauncher.h"
#include "Model/UsersListModel.h"
#include "Model/RepositoriesModel.h"
#include "Model/ProfilesListModel.h"

class Settings;

class HackersMCLauncher : public QMainWindow
{
	Q_OBJECT
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
	
public slots:
	void resetDownloadIndicators();
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


private slots:
	void screenScaleChanged();
	void setDownloadMode(bool m);
};
