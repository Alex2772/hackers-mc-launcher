#include "GameArgsListModel.h"
#include <QApplication>
#include <QFont>

int GameArgsListModel::columnCount(const QModelIndex& parent) const
{
	return 3;
}

GameArgsListModel::GameArgsListModel(Profile* profile, QObject* parent): QAbstractListModel(parent),
                                                                         mProfile(profile)
{
}

QVariant GameArgsListModel::data(const QModelIndex& index, int role) const
{
	switch (role)
	{
	case Qt::EditRole:
	case Qt::DisplayRole:
		switch (index.column())
		{
		case 0:
			return mProfile->mGameArgs[index.row()].mName;
		case 1:
			return mProfile->mGameArgs[index.row()].mValue;
		case 2:
			{
				QString res;
				for (auto& c : mProfile->mGameArgs[index.row()].mConditions)
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

bool GameArgsListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	switch (role)
	{
	case Qt::EditRole:
	case Qt::DisplayRole:
		switch (index.column())
		{
		case 0:
			mProfile->mGameArgs[index.row()].mName = value.toString();
			break;
		case 1:
			mProfile->mGameArgs[index.row()].mValue = value.toString();
			break;
		case 2:
			{
				mProfile->mGameArgs[index.row()].mConditions.clear();
				// holy parse hell
				QStringList args = value.toString().split(";");
				for (auto& a : args)
				{
					QStringList pair = a.split("=");

					// this is essentially contains logic
					if (pair.size() == 2)
					{
						if (!pair[0].isEmpty() && !pair[1].isEmpty())
							mProfile->mGameArgs[index.row()].mConditions
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

QVariant GameArgsListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal)
	{
		switch (role)
		{
		case Qt::DisplayRole:
			switch (section)
			{
			case 0:
				return "Key";
			case 1:
				return "Value";
			case 2:
				return "Condition";
			}
			break;
		case Qt::FontRole:
			return QFont(QApplication::font().family(), QApplication::font().pointSize(), QFont::Bold);
		}
	}
	return {};
}

int GameArgsListModel::rowCount(const QModelIndex& parent) const
{
	return mProfile->mGameArgs.size();
}

Qt::ItemFlags GameArgsListModel::flags(const QModelIndex& index) const
{
	return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool GameArgsListModel::removeRows(int row, int count, const QModelIndex& parent)
{
	if (row < 0)
		return false;

	beginRemoveRows(parent, row, row + count - 1);
	for (unsigned i = 0; i < count; ++i)
	{
		mProfile->mGameArgs.removeAt(row);
	}
	endRemoveRows();
	
	return true;
}

QModelIndex GameArgsListModel::insertRow()
{
	beginInsertRows({}, mProfile->mGameArgs.size(), mProfile->mGameArgs.size());
	mProfile->mGameArgs.push_back({});
	endInsertRows();
	return index(mProfile->mGameArgs.size() - 1);
}
