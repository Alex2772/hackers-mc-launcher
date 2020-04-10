#include "UsersListModel.h"
#include <qfont.h>
#include <QApplication>
#include <qcolor.h>

int UsersListModel::rowCount(const QModelIndex& parent) const
{
	// there's the last one phantom user used as placeholder for new user
	return mUsers.size() + 1;
}

QVariant UsersListModel::data(const QModelIndex& index, int role) const
{
	if (index.row() == mUsers.size())
	{
		// the phantom user
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
			return mUsers[index.row()].mUsername;
		}
	}
	return {};
}

bool UsersListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (role == Qt::EditRole || role == Qt::DisplayRole)
	{
		switch (index.column())
		{
		case 0:
			mUsers[index.row()].mUsername = value.toString();
			break;
		default:
			return false;
		}
		emit dataChanged(index, index, {role});
		return true;
	}
	return false;
}

bool UsersListModel::insertRows(int row, int count, const QModelIndex& parent)
{
	// TODO maybe should be rewritten for beginInsertRows/endInsertRows api
	for (int i = 0; i < count; ++i)
		mUsers.insert(row, {});

	if (row > 0)
		row -= 1;
	
	emit dataChanged(index(row, 0), index(row + count, 1));
	
	return true;
}

bool UsersListModel::removeRows(int row, int count, const QModelIndex& parent)
{
	// TODO maybe should be rewritten for beginRemoveRows/endRemoveRows api
	for (int i = 0; i < count; ++i)
		mUsers.removeAt(row);

	emit dataChanged(index(row, 0), index(row + count, 1));

	return true;
}


void UsersListModel::add(const User& u)
{
	mUsers << u;

	emit dataChanged(index(mUsers.size() - 2, 0), index(mUsers.size() - 1, 1));
}