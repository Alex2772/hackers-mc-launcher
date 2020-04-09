#include "DownloadsModel.h"
#include <QUrl>


DownloadsModel::DownloadsModel(QList<Download>& downloads, QObject* parent):
	QAbstractListModel(parent), mDownloads(downloads)
{
}

DownloadsModel::~DownloadsModel()
{
}

int DownloadsModel::rowCount(const QModelIndex& parent) const
{
	return mDownloads.size();
}

QVariant DownloadsModel::data(const QModelIndex& index, int role) const
{
	switch (role)
	{
	case Qt::EditRole:
	case Qt::DisplayRole:
		switch (index.column())
		{
		case 0:
			return mDownloads[index.row()].mLocalPath;
		case 1:
			return mDownloads[index.row()].mUrl;
		case 2:
			return mDownloads[index.row()].mSize;
		case 3:
			return mDownloads[index.row()].mHash;
		}
	}
	return {};
}

bool DownloadsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	switch (role)
	{
	case Qt::EditRole:
	case Qt::DisplayRole:
		switch (index.column())
		{
		case 0:
			mDownloads[index.row()].mLocalPath = value.toString();
			break;
		case 1:
			mDownloads[index.row()].mUrl = value.toString();
			break;
		case 2:
			mDownloads[index.row()].mSize = value.toUInt();
			break;
		case 3:
			mDownloads[index.row()].mHash = value.toString();
			break;
		default:
			return false;
		}
		return true;
	}
	return false;
}

QVariant DownloadsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal) {
		switch (role)
		{
		case Qt::DisplayRole:
			switch (section)
			{
			case 0:
				return tr("Local path");
			case 1:
				return tr("URL");
			case 2:
				return tr("Size (bytes)");
			case 3:
				return tr("SHA1");
			}
		}
	}
	return {};
}

Qt::ItemFlags DownloadsModel::flags(const QModelIndex& index) const
{
	return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

QModelIndex DownloadsModel::insertRow()
{
	int row = mDownloads.size();
	insertRows(row, 1, {});

	return index(row, 0, {});
}

bool DownloadsModel::insertRows(int row, int count, const QModelIndex& parent)
{
	if (row < 0)
		return false;
	beginInsertRows(parent, row, row + count - 1);

	for (int i = 0; i < count; ++i)
		mDownloads.insert(row, {});

	endInsertRows();
	return true;
}

bool DownloadsModel::removeRows(int row, int count, const QModelIndex& parent)
{
	if (row < 0)
		return false;
	beginRemoveRows(parent, row, row + count - 1);

	for (int i = 0; i < count; ++i)
		mDownloads.removeAt(row);

	endRemoveRows();
	return true;
}



int DownloadsModel::columnCount(const QModelIndex& parent) const
{
	return 4;
}
