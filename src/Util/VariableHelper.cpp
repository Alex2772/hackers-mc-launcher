#include "VariableHelper.h"
#include "HackersMCLauncher.h"
#include "Settings.h"
#include "launcher_config.h"
#include <QUuid>

QVariant VariableHelper::getVariableValue(HackersMCLauncher* launcher, const QString& name)
{
	static QMap<QString, std::function<QVariant()>> vars = {
		{
			"os.name",
			[]() -> QVariant
			{
				return QSysInfo::productType();
			}
		},
		{
			"os.version",
			[]() -> QVariant
			{
				return QSysInfo::productVersion();
			}
		},
		{
			"os.arch",
			[]() -> QVariant
			{
				return "x86";
			}
		},
		{
			"natives_directory",
			[launcher]() -> QVariant
			{
				return launcher->getSettings()->getGameDir().absolutePath();
			}
		},
		{
			"launcher_name",
			[]() -> QVariant
			{
				return "hackers-mc-launcher";
			}
		},
		{
			"launcher_version",
			[]() -> QVariant
			{
				return LAUNCHER_VERSION;
			}
		},
		{
			"classpath",
			[launcher]() -> QVariant
			{
				QString cp;
				Profile p;
				if (launcher->currentProfile(p)) {
					for (auto& c : p.mClasspath)
					{
						if (!cp.isEmpty())
						{
							// there's another divider for other OSes
							cp += ';';
						}
						cp += c.mPath;
					}
					return cp;
				}
				return {};
			}
		},
		{
			"auth_player_name",
			[launcher]() -> QVariant
			{
				User u;
				if (launcher->currentUser(u)) {
					return u.mUsername;
				}
				return {};
			}
		},
		{
			"auth_uuid",
			[launcher]() -> QVariant
			{
				User u;
				if (launcher->currentUser(u)) {
					// TODO
					return u.mUsername;
				}
				return {};
			}
		},
		{
			"auth_access_token",
			[]() -> QVariant
			{
				return "null";
			}
		},
		{
			"user_type",
			[]() -> QVariant
			{
				return "legacy";
			}
		},
		{
			"version_type",
			[]() -> QVariant
			{
				return "release";
			}
		},
		{
			"version_name",
			[launcher]() -> QVariant
			{
				Profile u;
				if (launcher->currentProfile(u)) {
					return u.mName;
				}
				return {};
			}
		},
		{
			"game_directory",
			[launcher]() -> QVariant
			{
				return launcher->getSettings()->getGameDir().absolutePath();
			}
		},
		{
			"assets_root",
			[launcher]() -> QVariant
			{
				return launcher->getSettings()->getGameDir().absoluteFilePath("assets");
			}
		},
		{
			"assets_index_name",
			[launcher]() -> QVariant
			{
				Profile u;
				if (launcher->currentProfile(u)) {
					return u.mAssetsIndex;
				}
				return {};
			}
		},
	};
	if (vars.contains(name))
		return vars[name]();
	return "NV:" + name;
}

/**
 * \return true, if some condition was not passed
 */
bool VariableHelper::checkConditions(HackersMCLauncher* launcher, const QList<QPair<QString, QVariant>>& conditions)
{

	for (auto& cond : conditions)
	{
		if (VariableHelper::getVariableValue(launcher, cond.first).toString() != cond.second.toString())
		{
			return true;
		}
	}
	return false;
}

QString VariableHelper::replaceVariablesInString(HackersMCLauncher* launcher, const QString& string)
{
	QString result;

	for (int i = 0; i < string.length(); ++i)
	{
		int begin = string.indexOf("${", i);
		if (begin < 0)
		{
			result += string.mid(i);
			break;
		}
		result += string.mid(i, begin - i);
		int end = string.indexOf("}", i);
		if (end < 0)
		{
			return "PARSE_ERROR";
		}
		i = end;

		auto variableName = string.mid(begin + 2, end - begin - 2);
		result += getVariableValue(launcher, variableName).toString();
	}
	
	return result;
}
