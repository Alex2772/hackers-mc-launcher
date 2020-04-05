#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_HackersMCLauncher.h"
#include "Model/UsersListModel.h"

class HackersMCLauncher : public QMainWindow
{
	Q_OBJECT

public:
	HackersMCLauncher(QWidget *parent = Q_NULLPTR);
	bool nativeEvent(const QByteArray& eventType, void* message, long* result) override;
	void showEvent(QShowEvent* event) override;
	void changeEvent(QEvent* e) override;
	void mousePressEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
	
	void updateMiddleButton();

private:
	Ui::HackersMCLauncherClass ui;
	int mUiScale = 1;

	UsersListModel mUsers;

private slots:
	void screenScaleChanged();
};
