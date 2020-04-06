#include "JavaLibModel.h"

JavaLibModel::JavaLibModel(QMap<QString, QList<JavaLib>>& javaLibs, QObject* parent): QAbstractItemModel(parent),
                                                                               mItems(javaLibs)
{
}

int JavaLibModel::rowCount(const QModelIndex& parent) const
{
	return mItems.size();
}

QVariant JavaLibModel::data(const QModelIndex& index, int role) const
{
	switch (role)
	{
	case Qt::EditRole:
	case Qt::DisplayRole:
		switch (index.column())
		{
		case 0:
			return mItems[index.row()].mGroup;
		case 1:
			return mItems[index.row()].mName;
		case 2:
			return mItems[index.row()].mVersion;
		case 3:
			return mItems[index.row()].mUrl;
		case 4:
			return mItems[index.row()].mHash;
		}
	}
	return {};
}

bool JavaLibModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	switch (role)
	{
	case Qt::EditRole:
	case Qt::DisplayRole:
		switch (index.column())
		{
		case 0:
			mItems[index.row()].mGroup = value.toString();
			break;
		case 1:
			mItems[index.row()].mName = value.toString();
			break;
		case 2:
			mItems[index.row()].mVersion = value.toString();
			break;
		case 3:
			mItems[index.row()].mUrl = value.toString();
			break;
		case 4:
			mItems[index.row()].mHash = value.toString();
			break;
		default:
			return false;
		}
		return true;
	}
	return false;
}

QVariant JavaLibModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal) {
		switch (role)
		{
		case Qt::DisplayRole:
			switch (section)
			{
			case 0:
				return tr("Group");
			case 1:
				return tr("Name");
			case 2:
				return tr("Version");
			case 3:
				return tr("Url");
			case 4:
				return tr("SHA1");
			}
		}
	}
	return {};
}

Qt::ItemFlags JavaLibModel::flags(const QModelIndex& index) const
{
	if (index.column() != 4)
		return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
	return QAbstractItemModel::flags(index);
}

QModelIndex JavaLibModel::index(int row, int column, const QModelIndex& parent) const
{
}

QModelIndex JavaLibModel::parent(const QModelIndex& child) const
{
	if (child.isValid())
	{
		if (child.parent().isValid())
		{
			
		}
	}
	return {};
}

int JavaLibModel::columnCount(const QModelIndex& parent) const
{
	return 4;
}
