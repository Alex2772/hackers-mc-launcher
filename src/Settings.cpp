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
	bindDefault("show_console", false);
	bindDefault("check_launcher_updates", true);
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
	auto path = value("game_dir").toString();
	QDir d(path);
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
			return Settings::value(key);
		}
	}
	return {};
}

QVariant Settings::value(const QString& key) const
{
	return QSettings::value(key, mDefaults[key]);
}

void Settings::setValue(const QString& key, const QVariant& value)
{
	QSettings::setValue(key, value);
	int index = mMappings.indexOf(key);
	if (index >= 0)
	{
		emit dataChanged(Settings::index(0, index, {}), Settings::index(0, index, {}));
	}
}
