#pragma once
#include <QAbstractListModel>

class JavaLib
{
public:
	QString mGroup;
	QString mName;
	QString mVersion;
	QString mUrl;
	QString mHash;
};

class JavaLibModel: public QAbstractItemModel
{
	Q_OBJECT

public:
	JavaLibModel(QMap<QString, QList<JavaLib>>& javaLibs, QObject* parent);

	int rowCount(const QModelIndex& parent) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role) override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	Qt::ItemFlags flags(const QModelIndex& index) const override;


	QModelIndex index(int row, int column, const QModelIndex& parent) const override;
	QModelIndex parent(const QModelIndex& child) const override;
private:
	QMap<QString, QList<JavaLib>>& mItems;

	int columnCount(const QModelIndex& parent) const override;
};
