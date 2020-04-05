#pragma once
#include <QSortFilterProxyModel>

class FilterProxyModel: public QSortFilterProxyModel
{
	Q_OBJECT
public:
	FilterProxyModel(int column, QObject* o);

	QMap<QString, bool>& filters();
	
protected:
	bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
	int mColumn;
	QMap<QString, bool> mFilters;
	
};
