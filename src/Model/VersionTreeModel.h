#pragma once

#include <QAbstractItemModel>
#include "RepositoriesModel.h"
#include <qjsonvalue.h>
#include <QDateTime>

class VersionTreeModel: public QAbstractItemModel
{
Q_OBJECT
	
public:
	class Item
	{
	private:
		QList<Item*> mChildren;
		Item* mParent = nullptr;

	public:
		QString mName;
		QString mType;
		QDateTime mReleaseDate;
		bool mIsErrored = false;
		bool mIsLatest = false;

		explicit Item(const QString& chars)
			: mName(chars)
		{
		}

		Item() = default;
		~Item();
		
		void addChild(Item* item);
		QList<Item*> children() const;
		Item* parent() const;
	};
	
	VersionTreeModel(QObject* parent);
	virtual ~VersionTreeModel();
	
	QModelIndex index(int row, int column, const QModelIndex& parent) const override;
	QModelIndex parent(const QModelIndex& child) const override;
	int rowCount(const QModelIndex& parent) const override;
	int columnCount(const QModelIndex& parent) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	void notifyUpdated(Item* item);

	Item& root();

private:
	mutable Item mRoot;
};
