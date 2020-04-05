#include "FilterProxyModel.h"

FilterProxyModel::FilterProxyModel(int column, QObject* o):
	QSortFilterProxyModel(o),
	mColumn(column)
{
}

bool FilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
	auto it = mFilters.find(sourceModel()->data(sourceModel()->index(source_row, mColumn, source_parent)).toString());
	if (it != mFilters.end())
		return it.value();

	return true;
}

QMap<QString, bool>& FilterProxyModel::filters()
{
	return mFilters;
}
