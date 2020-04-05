#pragma once

#include "ui_UserForm.h"
#include "Form.h"


class QAbstractItemModel;

class UserForm : public Form
{
	Q_OBJECT

public:
	UserForm(QAbstractItemModel* model, const QModelIndex& index, QWidget *parent = Q_NULLPTR);
	~UserForm();

private:
	Ui::UserForm ui;
};
