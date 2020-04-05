#pragma once

#include "Form.h"
#include "ui_ProfileForm.h"

class HackersMCLauncher;
class QAbstractItemModel;

class ProfileForm : public Form
{
	Q_OBJECT

public:
	ProfileForm(const QModelIndex& index, HackersMCLauncher* parent);
	~ProfileForm();

private:
	Ui::ProfileForm ui;
};
