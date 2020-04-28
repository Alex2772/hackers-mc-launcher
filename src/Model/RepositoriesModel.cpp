#include "RepositoriesModel.h"

int RepositoriesModel::columnCount(const QModelIndex& parent) const
{
	return 2;
}

int RepositoriesModel::rowCount(const ::QModelIndex& parent) const
{
	return mList.size();
}

::QVariant RepositoriesModel::data(const ::QModelIndex& index, int role) const
{
	switch (role)
	{
	case Qt::EditRole:
	case Qt::DisplayRole:
		switch(index.column())
		{
		case 0:
			return mList[index.row()].mName;
		case 1:
			return mList[index.row()].mUrl;
		}
;	}
	return {};
}

bool RepositoriesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{

	switch (role)
	{
	case Qt::EditRole:
	case Qt::DisplayRole:
		switch (index.column())
		{
		case 0:
			mList[index.row()].mName = value.toString();
			break;
		case 1:
			mList[index.row()].mUrl = value.toString();
			break;
		}
		;
	}
	return {};
}

QVariant RepositoriesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	switch (role)
	{
	case Qt::DisplayRole:
		switch (section)
		{
		case 0:
			return tr("Name");
		case 1:
			return tr("URL");
		}
		;
	}
	return {};
}

void RepositoriesModel::setToDefault()
{
	removeRows(0, mList.size(), {});
	add(Repository{tr("Official Minecraft"), "https://launchermeta.mojang.com/mc/game/version_manifest.json"});
}

QModelIndex RepositoriesModel::add(const Repository& profile)
{
	for (auto& i : mList)
	{
		if (i.mUrl == profile.mUrl)
		{
			return {};
		}
	}
	
	beginInsertRows({}, mList.size(), mList.size());
	mList << profile;
	endInsertRows();

	auto i = index(mList.size() - 1, 0);
	emit dataChanged(i, i);
	return i;
}

bool RepositoriesModel::removeRows(int row, int count, const QModelIndex& parent)
{
	if (count) {
		beginRemoveRows(parent, row, row + count - 1);
		mList.erase(mList.begin() + row, mList.begin() + row + count);
		endRemoveRows();

		emit dataChanged({}, {});
		return true;
	}
	return false;
}

void RepositoriesModel::update(int row, const Repository& profile)
{
	mList[row] = profile;
	emit dataChanged(index(row), index(row, columnCount({})));
}

const QList<Repository>& RepositoriesModel::getItems()
{
	return mList;
}
