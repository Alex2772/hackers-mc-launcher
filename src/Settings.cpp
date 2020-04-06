#include "Settings.h"
#include <QStandardPaths>

Settings::Settings(const QString& organization, const QString& application, QObject* parent): QSettings(
	organization, application, parent)
{
	auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

	// i don't actually need the "hachers-mc-launcher" suffix. I want to use .minecraft instead
	QDir d(path);
	d.cdUp();

	bindDefault("game_dir", d.absoluteFilePath(".minecraft"));
	bindDefault("hide_launcher", true);
	bindDefault("close_launcher", true);
}

void Settings::bindDefault(const QString& name, const QVariant& value)
{
	mDefaults[name] = value;
}

int Settings::columnCount(const QModelIndex& parent) const
{
	return mMappings.size();
}

void Settings::resetToDefaults()
{
	clear();
}

QDir Settings::getGameDir()
{
	QDir d(value("game_dir", QStandardPaths::locate(QStandardPaths::AppDataLocation, "")).toString());
	if (!d.exists())
		d.mkdir(".");

	return d;
}

int Settings::sectionOf(const QString& name)
{
	int i = mMappings.indexOf(name);
	if (i >= 0)
		return i;
	i = mMappings.size();
	mMappings << name;

	return i;
}

bool Settings::setData(const QModelIndex& index, const QVariant& value, int role)
{
	switch (role)
	{
	case Qt::DisplayRole:
	case Qt::EditRole:
		setValue(mMappings[index.column()], value);
		return true;
	}
	return false;
}

int Settings::rowCount(const QModelIndex& parent) const
{
	return 1;
}

QVariant Settings::data(const QModelIndex& index, int role) const
{
	switch (role)
	{
	case Qt::DisplayRole:
	case Qt::EditRole:
		{
			auto key = mMappings[index.column()];
			if (contains(key))
				return value(key);
			return mDefaults[key];
		}
	}
	return {};
}
