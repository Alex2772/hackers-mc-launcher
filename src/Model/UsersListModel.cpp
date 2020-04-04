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
			return mUsers.at(index.row()).mUsername;
		}
	}
	return {};
}
