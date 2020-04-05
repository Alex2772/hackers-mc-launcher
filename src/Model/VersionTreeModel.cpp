#include "VersionTreeModel.h"
#include <qcolor.h>
#include <QApplication>
#include <QFont>

QList<VersionTreeModel::Item*> VersionTreeModel::Item::children() const
{
	return mChildren;
}

VersionTreeModel::Item* VersionTreeModel::Item::parent() const
{
	return mParent;
}


VersionTreeModel::VersionTreeModel(QObject* parent):
	QAbstractItemModel(parent)
{
}


VersionTreeModel::Item::~Item()
{
	qDeleteAll(mChildren);
}

void VersionTreeModel::Item::addChild(Item* item)
{
	mChildren << item;
	item->mParent = this;
}


QModelIndex VersionTreeModel::index(int row, int column, const QModelIndex& parent) const
{
	if (hasIndex(row, column, parent))
	{
		Item* parentItem;
		if (parent.isValid())
			parentItem = static_cast<Item*>(parent.internalPointer());
		else
			parentItem = &mRoot;

		if (parentItem->children().size() > row)
			return createIndex(row, column, parentItem->children()[row]);
	}
	return {};
}

QModelIndex VersionTreeModel::parent(const QModelIndex& child) const
{
	if (child.isValid())
	{
		auto item = static_cast<Item*>(child.internalPointer());
		auto parent = item->parent();
		if (parent != &mRoot) {
			return createIndex(
				parent->parent() ? parent->parent()->children().indexOf(parent) : 0,
				child.column(), parent);
		}
	}
	return {};
}

int VersionTreeModel::rowCount(const QModelIndex& parent) const
{
	const Item* item;
	if (parent.isValid())
	{
		item = static_cast<Item*>(parent.internalPointer());
	} else
	{
		item = &mRoot;
	}

	return item->children().size();
}

int VersionTreeModel::columnCount(const QModelIndex& parent) const
{
	return 3;
}

QVariant VersionTreeModel::data(const QModelIndex& index, int role) const
{
	Item* item = static_cast<Item*>(index.internalPointer());
	switch (role) {
	case Qt::DisplayRole:
	case Qt::EditRole:

		switch (index.column())
		{
		case 0:
			return item->mName;
		case 1:
			return item->mType;
		case 2:
			if (!item->mType.isEmpty())
				return item->mReleaseDate;
		}
		break;
	case Qt::ForegroundRole:
		if (index.column() == 0 && item->mIsErrored)
		{
			return QColor(255, 0, 0);
		}
		break;
	case Qt::FontRole:
		if (index.column() == 0 && item->mIsLatest)
		{
			return QFont(QApplication::font().family(), QApplication::font().pointSize(), QFont::Bold);
		}
		break;
	}
	return {};
}

QVariant VersionTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		switch (section)
		{
		case 0:
			return tr("Name");
		case 1:
			return tr("Type");
		case 2:
			return tr("Release date");
		}
	}
	return {};
}

VersionTreeModel::Item& VersionTreeModel::root()
{
	return mRoot;
}


void VersionTreeModel::notifyUpdated(Item* item)
{
	auto index = createIndex(item->parent()->children().indexOf(item), 0, item);
	emit dataChanged(index, index);
}
VersionTreeModel::~VersionTreeModel()
{
}
