#include "JavaArgsListModel.h"
#include <QApplication>
#include <QFont>

int JavaArgsListModel::columnCount(const QModelIndex& parent) const
{
	return 2;
}

JavaArgsListModel::JavaArgsListModel(Profile* profile, QObject* parent) : QAbstractListModel(parent),
mProfile(profile)
{
}

QVariant JavaArgsListModel::data(const QModelIndex& index, int role) const
{
	switch (role)
	{
	case Qt::EditRole:
	case Qt::DisplayRole:
		switch (index.column())
		{
		case 0:
			return mProfile->mJavaArgs[index.row()].mName;
		case 1:
		{
			QString res;
			for (auto& c : mProfile->mJavaArgs[index.row()].mConditions)
			{
				if (!res.isEmpty())
					res += ';';
				res += c.first + '=' + c.second.toString();
			}
			return res;
		}
		}
	}
	return {};
}

bool JavaArgsListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	switch (role)
	{
	case Qt::EditRole:
	case Qt::DisplayRole:
		switch (index.column())
		{
		case 0:
			mProfile->mJavaArgs[index.row()].mName = value.toString();
			break;
		case 1:
		{
			mProfile->mJavaArgs[index.row()].mConditions.clear();
			// holy parse hell
			QStringList args = value.toString().split(";");
			for (auto& a : args)
			{
				QStringList pair = a.split("=");

				// this is essentially contains logic
				if (pair.size() == 2)
				{
					if (!pair[0].isEmpty() && !pair[1].isEmpty())
						mProfile->mJavaArgs[index.row()].mConditions
						<< QPair<QString, QVariant>{pair[0], pair[1]};
				}
			}
			break;
		}
		default:
			return false;
		}
		return true;
	}
	return false;
}

QVariant JavaArgsListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal)
	{
		switch (role)
		{
		case Qt::DisplayRole:
			switch (section)
			{
			case 0:
				return "Arg";
			case 1:
				return "Condition";
			}
			break;
		case Qt::FontRole:
			return QFont(QApplication::font().family(), QApplication::font().pointSize(), QFont::Bold);
		}
	}
	return {};
}

int JavaArgsListModel::rowCount(const QModelIndex& parent) const
{
	return mProfile->mJavaArgs.size();
}

Qt::ItemFlags JavaArgsListModel::flags(const QModelIndex& index) const
{
	return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool JavaArgsListModel::removeRows(int row, int count, const QModelIndex& parent)
{
	if (row < 0)
		return false;

	beginRemoveRows(parent, row, row + count - 1);
	for (unsigned i = 0; i < count; ++i)
	{
		mProfile->mJavaArgs.removeAt(row);
	}
	endRemoveRows();

	return true;
}

QModelIndex JavaArgsListModel::insertRow()
{
	beginInsertRows({}, mProfile->mJavaArgs.size(), mProfile->mJavaArgs.size());
	mProfile->mJavaArgs.push_back({});
	endInsertRows();
	return index(mProfile->mJavaArgs.size() - 1);
}
