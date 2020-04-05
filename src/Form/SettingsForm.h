#pragma once

#include "ui_SettingsForm.h"
#include "Form.h"

class HackersMCLauncher;

class SettingsForm : public Form
{
	Q_OBJECT

public:
	SettingsForm(HackersMCLauncher* launcher);
	~SettingsForm();

private:
	Ui::SettingsForm ui;

};
