#include "RepositoriesModel.h"

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

void RepositoriesModel::setToDefault()
{
	mList.clear();
	mList << Repository{tr("Official Minecraft"), "https://launchermeta.mojang.com/mc/game/version_manifest.json"};
}

const QList<Repository>& RepositoriesModel::getItems()
{
	return mList;
}
