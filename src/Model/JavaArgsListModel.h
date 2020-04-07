#pragma once
#include <QAbstractListModel>
#include "Profile.h"

class JavaArgsListModel : public QAbstractListModel
{
	Q_OBJECT
		int columnCount(const QModelIndex& parent) const override;
public:
	JavaArgsListModel(Profile* profile, QObject* parent);
	virtual ~JavaArgsListModel() = default;

	QVariant data(const QModelIndex& index, int role) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role) override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	int rowCount(const QModelIndex& parent) const override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	bool removeRows(int row, int count, const QModelIndex& parent) override;

	QModelIndex insertRow();

private:
	Profile* mProfile;
};
