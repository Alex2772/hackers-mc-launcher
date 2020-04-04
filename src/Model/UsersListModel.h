#pragma once
#include <qabstractitemmodel.h>
#include "User.h"

class UsersListModel: public QAbstractListModel
{
	Q_OBJECT
public:
	int rowCount(const QModelIndex& parent) const override;
	QVariant data(const QModelIndex& index, int role) const override;

private:
	QList<User> mUsers;
};
