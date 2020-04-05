#pragma once
#include <qabstractitemmodel.h>
#include "User.h"

class UsersListModel: public QAbstractListModel
{
	Q_OBJECT
public:
	int rowCount(const QModelIndex& parent) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role) override;
	bool insertRows(int row, int count, const QModelIndex& parent) override;
	bool removeRows(int row, int count, const QModelIndex& parent) override;
private:
	QList<User> mUsers;
};
