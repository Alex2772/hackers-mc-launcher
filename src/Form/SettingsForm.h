#pragma once

#include "ui_SettingsForm.h"
#include "Form.h"

class SettingsForm : public Form
{
	Q_OBJECT

public:
	SettingsForm(QWidget *parent = Q_NULLPTR);
	~SettingsForm();

private:
	Ui::SettingsForm ui;
};
