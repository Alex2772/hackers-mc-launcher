#include "ClasspathListModel.h"
#include <QUrl>


ClasspathListModel::ClasspathListModel(QList<ClasspathEntry>& classpath, QObject* parent) :
	QAbstractListModel(parent), mClasspath(classpath)
{
}

ClasspathListModel::~ClasspathListModel()
{
}

int ClasspathListModel::rowCount(const QModelIndex& parent) const
{
	return mClasspath.size();
}

QVariant ClasspathListModel::data(const QModelIndex& index, int role) const
{
	switch (role)
	{
	case Qt::EditRole:
	case Qt::DisplayRole:
		switch (index.column())
		{
		case 0:
			return mClasspath[index.row()].mPath;
		}
	}
	return {};
}

bool ClasspathListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	switch (role)
	{
	case Qt::EditRole:
	case Qt::DisplayRole:
		switch (index.column())
		{
		case 0:
			mClasspath[index.row()].mPath = value.toString();
		default:
			return false;
		}
		return true;
	}
	return false;
}

QVariant ClasspathListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal) {
		switch (role)
		{
		case Qt::DisplayRole:
			switch (section)
			{
			case 0:
				return tr("Local path");
			}
		}
	}
	return {};
}

Qt::ItemFlags ClasspathListModel::flags(const QModelIndex& index) const
{
	return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

QModelIndex ClasspathListModel::insertRow(const ClasspathEntry& cp)
{
	int row = mClasspath.size();
	beginInsertRows({}, row, row);

	mClasspath.insert(row, cp);

	endInsertRows();
	
	return index(row, 0, {});
}

bool ClasspathListModel::insertRows(int row, int count, const QModelIndex& parent)
{
	if (row < 0)
		return false;
	beginInsertRows(parent, row, row + count - 1);

	for (int i = 0; i < count; ++i)
		mClasspath.insert(row, {});

	endInsertRows();
	return true;
}

bool ClasspathListModel::removeRows(int row, int count, const QModelIndex& parent)
{
	if (row < 0)
		return false;
	beginRemoveRows(parent, row, row + count - 1);

	for (int i = 0; i < count; ++i)
		mClasspath.removeAt(row);

	endRemoveRows();
	return true;
}



int ClasspathListModel::columnCount(const QModelIndex& parent) const
{
	return 1;
}
