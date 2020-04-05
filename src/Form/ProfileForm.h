#pragma once

#include "Form.h"
#include "ui_ProfileForm.h"

class QAbstractItemModel;

class ProfileForm : public Form
{
	Q_OBJECT

public:
	ProfileForm(QAbstractItemModel* model, const QModelIndex& index, QWidget* parent = Q_NULLPTR);
	~ProfileForm();

private:
	Ui::ProfileForm ui;
};
