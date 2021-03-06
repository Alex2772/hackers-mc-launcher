﻿#pragma once
#include <QAbstractListModel>
#include "Util/SimpleTreeItem.h"

class Download
{
public:
	QString mLocalPath;
	QString mUrl;
	quint64 mSize = 0;
	bool mExtract = false;
	QString mHash;
};

inline bool operator==(const Download& l, const Download& r)
{
	return l.mLocalPath == r.mLocalPath;
}
inline uint qHash(const Download& t)
{
	return qHash(t.mLocalPath);
}

class DownloadsModel: public QAbstractListModel
{
	Q_OBJECT

public:
	DownloadsModel(QList<Download>& downloads, QObject* parent);
	virtual ~DownloadsModel();

	int rowCount(const QModelIndex& parent) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role) override;
	
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	
	QModelIndex insertRow();
	
	bool insertRows(int row, int count, const QModelIndex& parent) override;
	bool removeRows(int row, int count, const QModelIndex& parent) override;
private:
	QList<Download>& mDownloads;

	int columnCount(const QModelIndex& parent) const override;
};

