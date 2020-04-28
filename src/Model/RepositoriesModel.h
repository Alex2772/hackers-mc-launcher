#pragma once
#include <QAbstractListModel>
#include "Repository.h"

class RepositoriesModel: public QAbstractListModel
{
	Q_OBJECT
private:
	QList<Repository> mList;

	int columnCount(const QModelIndex& parent) const override;
	
public:
	int rowCount(const ::QModelIndex& parent) const override;
	::QVariant data(const ::QModelIndex& index, int role) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role) override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	
	void setToDefault();
	QModelIndex add(const Repository& profile);

	bool removeRows(int row, int count, const QModelIndex& parent) override;
	void update(int row, const Repository& profile);

	const QList<Repository>& getItems();
};
