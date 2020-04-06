#pragma once

#include "Form.h"
#include "ui_ProfileForm.h"
#include "Model/Profile.h"
#include <QList>

class HackersMCLauncher;
class QAbstractItemModel;

class ProfileForm : public Form
{
	Q_OBJECT
private slots:
	void updateCaption();
	
public:
	ProfileForm(const QModelIndex& index, HackersMCLauncher* parent);
	~ProfileForm();

private:
	Ui::ProfileForm ui;
	Profile* mItem;
};
