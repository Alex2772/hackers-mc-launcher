#include "StackedLayout.h"
#include <QWidget>

StackedLayout::StackedLayout(QWidget* parent): QLayout(parent)
{
}

StackedLayout::StackedLayout()
{
}

StackedLayout::StackedLayout(QLayoutPrivate& d, QLayout* layout, QWidget* widget): QLayout(d, layout, widget)
{
}

StackedLayout::~StackedLayout()
{
	qDeleteAll(mItems);
}

QSize StackedLayout::sizeHint() const
{
	return parentWidget()->size();
}

void StackedLayout::addItem(QLayoutItem* item)
{
	mItems << item;	
}

QLayoutItem* StackedLayout::itemAt(int index) const
{
	return mItems.value(index);
}

QLayoutItem* StackedLayout::takeAt(int index)
{
	return index < 0 ? 0 : mItems.takeAt(index);
}

int StackedLayout::count() const
{
	return mItems.size();
}

void StackedLayout::setGeometry(const QRect& r)
{
	for (auto& i : mItems)
		i->setGeometry(r);
}

QSize StackedLayout::minimumSize() const
{
	QSize s(0, 0);
	int n = mItems.count();
	int i = 0;
	while (i < n) {
		QLayoutItem* o = mItems.at(i);
		s = s.expandedTo(o->minimumSize());
		++i;
	}
	return s;
}
