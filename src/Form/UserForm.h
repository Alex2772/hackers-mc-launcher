#pragma once

#include "ui_UserForm.h"
#include "Form.h"
#include <QAbstractItemModel>


class QAbstractItemModel;

class UserForm : public Form
{
	Q_OBJECT

public:
	UserForm(QAbstractItemModel* model, const QModelIndex& index, bool newUser, QWidget *parent = Q_NULLPTR);
	~UserForm();

private:
	Ui::UserForm ui;

	QAbstractItemModel* mModel;
	QModelIndex mIndex;
	bool mPositiveResult;
};
