#include "JavaLibModel.h"
#include <QUrl>

QString JavaLib::jarName() const
{
	return mName + '-' + mVersion + ".jar";
}

QString JavaLib::path() const
{
	auto group = mGroup;
	group.replace('.', '/');

	return group + '/' + mName + '/' + mVersion + '/' + jarName();
}

JavaLibModel::JavaLibModel(QMap<QUrl, QList<JavaLib>>& javaLibs, QObject* parent):
	QAbstractItemModel(parent), mItems(javaLibs)
{
	for (auto& j : javaLibs.keys())
	{
		auto d = new Item(new JavaLib({ j.toString() }));
		for (auto& i : javaLibs[j])
			d->addChild(new Item(&i));
		mRoot.addChild(d);
	}
}

JavaLibModel::~JavaLibModel()
{
}

int JavaLibModel::rowCount(const QModelIndex& parent) const
{
	const Item* item;
	if (parent.isValid())
	{
		item = static_cast<Item*>(parent.internalPointer());
	}
	else
	{
		item = &mRoot;
	}

	return item->children().size();
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
			return static_cast<Item*>(index.internalPointer())->mData->mGroup;
		case 1:
			return static_cast<Item*>(index.internalPointer())->mData->mName;
		case 2:
			return static_cast<Item*>(index.internalPointer())->mData->mVersion;
		case 3:
			return static_cast<Item*>(index.internalPointer())->mData->mHash;
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
			static_cast<Item*>(index.internalPointer())->mData->mGroup = value.toString();
			break;
		case 1:
			static_cast<Item*>(index.internalPointer())->mData->mName = value.toString();
			break;
		case 2:
			static_cast<Item*>(index.internalPointer())->mData->mVersion = value.toString();
			break;
		case 3:
			static_cast<Item*>(index.internalPointer())->mData->mHash = value.toString();
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
				return tr("SHA1");
			}
		}
	}
	return {};
}

Qt::ItemFlags JavaLibModel::flags(const QModelIndex& index) const
{
	if (static_cast<Item*>(index.internalPointer())->parent() != &mRoot || index.column() == 0)
		return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
	return QAbstractItemModel::flags(index);
}

bool JavaLibModel::removeRows(int row, int count, const QModelIndex& parent)
{
	if (row < 0)
		return false;
	auto item = static_cast<Item*>(parent.internalPointer());
	beginRemoveRows(parent, row, row + count - 1);
	for (int i = row; i < row + count; ++i)
	{
		auto lib = item->children()[i]->mData;
		for (auto& k : mItems)
		{
			bool found = false;
			for (auto it = k.begin(); it != k.end(); ++it)
			{
				if (&(*it) == lib)
				{
					found = true;
					item->removeChild(i);
					k.erase(it);
					break;
				}
			}
			if (found)
				break;
		}
	}

	endRemoveRows();
	return true;
}

QModelIndex JavaLibModel::index(int row, int column, const QModelIndex& parent) const
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

QModelIndex JavaLibModel::parent(const QModelIndex& child) const
{
	if (child.isValid())
	{
		auto item = static_cast<Item*>(child.internalPointer());
		auto parent = item->parent();
		if (parent != &mRoot) {
			return createIndex(
				parent->parent() ? parent->parent()->children().indexOf(parent) : 0,
				0, parent);
		}
	}
	return {};
}
QModelIndex JavaLibModel::indexOfItem(Item* item)
{
	if (item->parent() == nullptr)
		return {};
	return index(item->parent()->children().indexOf(item), 0, indexOfItem(item->parent()));
}
void JavaLibModel::add(QUrl url, const JavaLib& l)
{
	if (mItems.contains(url)) {
		mItems[url] << l;
		for (auto& c : mRoot.children())
		{
			if (c->mData->mGroup == url.toString())
			{
				beginInsertRows(indexOfItem(c), c->children().size(), c->children().size() + 1);
				c->addChild(new Item(&(mItems[url].back())));
				endInsertRows();
				break;
			}
		}
	} else
	{
		mItems[url] << l;
		auto c = new Item(new JavaLib{url.toString()});
		beginInsertRows({}, mRoot.children().size(), mRoot.children().size() + 1);
		mRoot.addChild(c);
		c->addChild(new Item(&(mItems[url].back())));
		endInsertRows();
	}
}

int JavaLibModel::columnCount(const QModelIndex& parent) const
{
	return 4;
}
