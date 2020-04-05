#pragma once
#include <qabstractitemmodel.h>
#include "User.h"
#include "Profile.h"

class ProfilesListModel : public QAbstractListModel
{
	Q_OBJECT
public:
	int rowCount(const QModelIndex& parent) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role) override;
	bool insertRows(int row, int count, const QModelIndex& parent) override;
	bool removeRows(int row, int count, const QModelIndex& parent) override;

	QModelIndex push_back(Profile profile);
	
private:
	QList<Profile> mProfiles;
};
