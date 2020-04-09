#pragma once
#include <QAbstractListModel>
#include "Util/SimpleTreeItem.h"

class ClasspathEntry
{
public:
	QString mPath;
};

class ClasspathListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	ClasspathListModel(QList<ClasspathEntry>& classpath, QObject* parent);
	virtual ~ClasspathListModel();

	int rowCount(const QModelIndex& parent) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role) override;

	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	QModelIndex insertRow(const ClasspathEntry& cp);

	bool insertRows(int row, int count, const QModelIndex& parent) override;
	bool removeRows(int row, int count, const QModelIndex& parent) override;
private:
	QList<ClasspathEntry>& mClasspath;

	int columnCount(const QModelIndex& parent) const override;
};

