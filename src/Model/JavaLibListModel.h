#pragma once
#include <QAbstractListModel>

class JavaLib
{
public:
	QString mName;
	QString mVersion;
	QString mUrl;
	QString mHash;
};

class JavaLibListModel: public QAbstractListModel
{
	Q_OBJECT

public:
	JavaLibListModel(QList<JavaLib>& javaLibs, QObject* parent);

	int rowCount(const QModelIndex& parent) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role) override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	Qt::ItemFlags flags(const QModelIndex& index) const override;
private:
	QList<JavaLib>& mItems;

	int columnCount(const QModelIndex& parent) const override;
};
