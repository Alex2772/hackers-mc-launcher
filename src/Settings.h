#pragma once
#include <QSettings>
#include <QDir>
#include <QAbstractListModel>

class Settings: public QSettings, public QAbstractListModel
{
private:
	QList<QString> mMappings;
	QMap<QString, QVariant> mDefaults;

	
	void bindDefault(const QString& name, const QVariant& value);
private:
	int columnCount(const QModelIndex& parent) const override;

public:
	Settings(const QString& organization, const QString& application, QObject* parent);
	QDir getGameDir();

	int sectionOf(const QString& name);

	bool setData(const QModelIndex& index, const QVariant& value, int role) override;
	int rowCount(const QModelIndex& parent) const override;
	QVariant data(const QModelIndex& index, int role) const override;

	QVariant value(const QString& key) const;
	void setValue(const QString& key, const QVariant& value);
public slots:
	void resetToDefaults();
};

