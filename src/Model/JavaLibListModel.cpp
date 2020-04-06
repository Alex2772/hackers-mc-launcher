#include "JavaLibListModel.h"

JavaLibListModel::JavaLibListModel(QList<JavaLib>& javaLibs, QObject* parent): QAbstractListModel(parent),
                                                                               mItems(javaLibs)
{
}

int JavaLibListModel::rowCount(const QModelIndex& parent) const
{
	return mItems.size();
}

QVariant JavaLibListModel::data(const QModelIndex& index, int role) const
{
	switch (role)
	{
	case Qt::EditRole:
	case Qt::DisplayRole:
		switch (index.column())
		{
		case 0:
			return mItems[index.row()].mName;
		case 1:
			return mItems[index.row()].mVersion;
		case 2:
			return mItems[index.row()].mUrl;
		case 3:
			return mItems[index.row()].mHash;
		}
	}
	return {};
}

bool JavaLibListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	switch (role)
	{
	case Qt::EditRole:
	case Qt::DisplayRole:
		switch (index.column())
		{
		case 0:
			mItems[index.row()].mName = value.toString();
			break;
		case 1:
			mItems[index.row()].mVersion = value.toString();
			break;
		case 2:
			mItems[index.row()].mUrl = value.toString();
			break;
		case 3:
			mItems[index.row()].mHash = value.toString();
			break;
		default:
			return false;
		}
		return true;
	}
	return false;
}

QVariant JavaLibListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal) {
		switch (role)
		{
		case Qt::DisplayRole:
			switch (section)
			{
			case 0:
				return tr("Name");
			case 1:
				return tr("Version");
			case 2:
				return tr("Url");
			case 3:
				return tr("SHA1");
			}
		}
	}
	return {};
}

Qt::ItemFlags JavaLibListModel::flags(const QModelIndex& index) const
{
	if (index.column() != 4)
		return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
	return QAbstractItemModel::flags(index);
}

int JavaLibListModel::columnCount(const QModelIndex& parent) const
{
	return 4;
}
