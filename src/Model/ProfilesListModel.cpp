#include "ProfilesListModel.h"
#include <qfont.h>
#include <QApplication>
#include <qcolor.h>

int ProfilesListModel::rowCount(const QModelIndex& parent) const
{
	// there's the last one phantom profile used as placeholder for new profile
	return mProfiles.size() + 1;
}

QVariant ProfilesListModel::data(const QModelIndex& index, int role) const
{
	if (index.row() == mProfiles.size())
	{
		// the phantom profile
		switch (role)
		{
		case Qt::DisplayRole:
			return tr("<click here to add a new one>");
		case Qt::FontRole:
			return QFont(QApplication::font().family(), QApplication::font().pointSize(),
				QApplication::font().weight(), true);
		case Qt::ForegroundRole:
			return QColor(100, 100, 100);
		}
	}
	else {
		switch (role)
		{
		case Qt::EditRole:
		case Qt::DisplayRole:
			return mProfiles[index.row()].mName;
		}
	}
	return {};
}

bool ProfilesListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (role == Qt::EditRole || role == Qt::DisplayRole)
	{
		switch (index.column())
		{
		case 0:
			mProfiles[index.row()].mName = value.toString();
			break;
		default:
			return false;
		}
		emit dataChanged(index, index, { role });
		return true;
	}
	return false;
}

bool ProfilesListModel::insertRows(int row, int count, const QModelIndex& parent)
{
	for (int i = 0; i < count; ++i)
		mProfiles.insert(row, {});

	if (row > 0)
		row -= 1;

	emit dataChanged(index(row, 0), index(row + count, 1));

	return true;
}

bool ProfilesListModel::removeRows(int row, int count, const QModelIndex& parent)
{
	for (int i = 0; i < count; ++i)
		mProfiles.removeAt(row);

	emit dataChanged(index(row, 0), index(row + count, 1));

	return true;
}
