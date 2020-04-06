#pragma once

template <typename T>
class SimpleTreeItem
{
	SimpleTreeItem<T>* mParent = nullptr;
	QList<SimpleTreeItem<T>*> mChildren;
public:
	T mData;

	explicit SimpleTreeItem(const T& data);
	SimpleTreeItem();
	~SimpleTreeItem();

	void addChild(SimpleTreeItem<T>* child);

	SimpleTreeItem<T>* parent() const;
	const QList<SimpleTreeItem<T>*>& children() const;
};

template <typename T>
SimpleTreeItem<T>::SimpleTreeItem(const T& data): mData(data)
{
}

template <typename T>
SimpleTreeItem<T>::SimpleTreeItem()
{
	if (std::is_pointer_v<T>)
	{
		mData = nullptr;
	}
}

template <typename T>
SimpleTreeItem<T>::~SimpleTreeItem()
{
	if (std::is_pointer_v<T>)
	{
		delete mData;
	}
	qDeleteAll(mChildren);
}

template <typename T>
void SimpleTreeItem<T>::addChild(SimpleTreeItem<T>* child)
{
	mChildren << child;
	child->mParent = this;
}

template <typename T>
SimpleTreeItem<T>* SimpleTreeItem<T>::parent() const
{
	return mParent;
}

template <typename T>
const QList<SimpleTreeItem<T>*>& SimpleTreeItem<T>::children() const
{
	return mChildren;
}
