#pragma once
#include <QAbstractListModel>
#include "Repository.h"

class RepositoriesModel: public QAbstractListModel
{
	Q_OBJECT
private:
	QList<Repository> mList;
public:
	int rowCount(const ::QModelIndex& parent) const override;
	::QVariant data(const ::QModelIndex& index, int role) const override;
	void setToDefault();

	const QList<Repository>& getItems();
};
