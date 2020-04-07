#pragma once
#include <QAbstractListModel>
#include "Util/SimpleTreeItem.h"

class JavaLib
{
public:
	QString mGroup;
	QString mName;
	QString mVersion;
	QString mHash;
};

class JavaLibModel: public QAbstractItemModel
{
	Q_OBJECT

public:
	JavaLibModel(QMap<QUrl, QList<JavaLib>>& javaLibs, QObject* parent);
	virtual ~JavaLibModel();

	int rowCount(const QModelIndex& parent) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role) override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	Qt::ItemFlags flags(const QModelIndex& index) const override;


	bool removeRows(int row, int count, const QModelIndex& parent) override;
	QModelIndex index(int row, int column, const QModelIndex& parent) const override;
	QModelIndex parent(const QModelIndex& child) const override;

	void add(QUrl url, const JavaLib& l);
	
private:
	QMap<QUrl, QList<JavaLib>>& mItems;
	typedef SimpleTreeItem<JavaLib*> Item;
	mutable Item mRoot;

	int columnCount(const QModelIndex& parent) const override;
};
