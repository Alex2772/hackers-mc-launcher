#pragma once
#include <QLayout>

class StackedLayout: public QLayout
{
	Q_OBJECT
public:
	explicit StackedLayout(QWidget* parent);
	StackedLayout();
	StackedLayout(QLayoutPrivate& d, QLayout* layout, QWidget* widget);
	virtual ~StackedLayout();

	QSize sizeHint() const override;
	void addItem(QLayoutItem*) override;
	QLayoutItem* itemAt(int index) const override;
	QLayoutItem* takeAt(int index) override;
	int count() const override;
	void setGeometry(const QRect& r) override;


	QSize minimumSize() const override;
private:
	QList<QLayoutItem*> mItems;
};
