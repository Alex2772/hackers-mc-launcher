#include "Profile.h"
#include <QJsonObject>
#include <QJsonArray>
#include "Util/VariableHelper.h"
#include "HackersMCLauncher.h"
#include "Settings.h"
#include <QJsonDocument>
#include "Util/Crypto.h"
#include "Util/Util.h"

Download downloadFromJson(const QString& path, const QJsonObject& v)
{
	return Download{
		path, v["url"].toString(),
		quint64(v["size"].toInt()), false, v["sha1"].toString()
	};
}

/**
 * \brief Converts maven library name notation to path
 * 
 *        for example:
 *					optifine:OptiFine:1.14.4_HD_U_F4
 *							 ->
 *        optifine/OptiFine/1.14.4_HD_U_F4/OptiFine-1.14.4_HD_U_F4.jar
 *
 *        Java coders so Java coders.
 *        
 * \param name maven notation
 * \return meaningful path
 */
QString Profile::javaLibNameToPath(const QString& name)
{
	auto colonSplitted = name.split(':');
	if (colonSplitted.size() == 3)
	{
		colonSplitted[0].replace('.', '/');
		return colonSplitted[0] + '/' + colonSplitted[1] + '/' + colonSplitted[2]
			+ '/' + colonSplitted[1] + '-' + colonSplitted[2] + ".jar";
	}
	return "INVALID:" + name;
}

/**
 * \brief Cleans up garbage from the Profile (duplicate and corrupted
 *		  entries, order and etc)
 */
void Profile::makeClean()
{
	mClasspath.erase(std::remove_if(mClasspath.begin(), mClasspath.end(), [](const ClasspathEntry& e)
	{
		return
			e.mPath.endsWith("/") // a path ended with '/' is probably a dir, not a jar file.
			|| e.mPath.startsWith("versions/"); // the only one jar from versions/ folder can exists in classpath.
	}), mClasspath.end());

	// return profile main jar file back.
	mClasspath.push_back({"versions/" + mName + "/" + mName + ".jar"});

	Util::cleanup(mDownloads);
	Util::cleanup(mClasspath);

	QSet<QString> existingKeys;
	mJavaArgs.erase(std::remove_if(mJavaArgs.begin(), mJavaArgs.end(), [&](const JavaArg& e) -> bool
	{
		if (existingKeys.contains(e.mName))
		{
			return true;
		}
		existingKeys << e.mName;
		return false;
	}), mJavaArgs.end());

	existingKeys.clear();

	mGameArgs.erase(std::remove_if(mGameArgs.begin(), mGameArgs.end(), [&](const GameArg& e) -> bool
	{
		if (existingKeys.contains(e.mName))
		{
			return true;
		}
		existingKeys << e.mName;
		return false;
	}), mGameArgs.end());
}

void Profile::fromJson(HackersMCLauncher* launcher, Profile& p, const QString& name, const QJsonObject& object)
{
	bool cleanupNeeded = false;

	if (object["hackers-mc"].isBool())
	{
		// hackers-mc format

		// Downloads
		for (auto& d : object["downloads"].toArray())
		{
			auto o = d.toObject();
			p.mDownloads << downloadFromJson(o["local"].toString(), o);
			p.mDownloads.last().mExtract = o["extract"].toBool();
		}

		// game args
		for (QJsonValue d : object["game_args"].toArray())
		{
			GameArg entry;
			entry.mName = d["name"].toString();
			entry.mValue = d["value"].toString();

			QJsonArray conditions;

			for (auto& c : d["conditions"].toArray())
			{
				entry.mConditions << QPair<QString, QVariant>{c.toArray()[0].toString(), c.toArray()[1].toVariant()};
			}

			p.mGameArgs << entry;
		}
		// java args
		for (QJsonValue d : object["java_args"].toArray())
		{
			JavaArg entry;
			entry.mName = d["name"].toString();

			QJsonArray conditions;

			for (auto& c : d["conditions"].toArray())
			{
				entry.mConditions << QPair<QString, QVariant>{c.toArray()[0].toString(), c.toArray()[1].toVariant()};
			}

			p.mJavaArgs << entry;
		}

		// classpath
		for (auto& e : object["classpath"].toArray())
		{
			p.mClasspath << ClasspathEntry{e.toString()};
		}

		auto settings = object["settings"].toObject();
		p.mWindowWidth = settings["window_width"].toInt();
		p.mWindowHeight = settings["window_height"].toInt();
		p.mIsFullscreen = settings["fullscreen"].toBool();
	}
	else
	{
		// legacy minecraft launcher format

		// Java libraries
		for (QJsonValue v : object["libraries"].toArray())
		{
			if (v["rules"].isArray())
			{
				bool allowed = false;
				for (auto& r : v["rules"].toArray())
				{
					auto rule = r.toObject();
					bool rulePassed = true;
					for (auto& k : rule.keys())
					{
						if (k != "action")
						{
							if (rule[k].isObject())
							{
								auto x = rule[k].toObject();
								for (auto& v : x.keys())
								{
									if (VariableHelper::getVariableValue(launcher, k + '.' + v).toString() != x[v]
									                                                                          .
									                                                                          toVariant()
									                                                                          .
									                                                                          toString()
									)
									{
										rulePassed = false;
										break;
									}
								}
							}
							else
							{
								if (VariableHelper::getVariableValue(launcher, k).toString() != rule[k]
								                                                                .toVariant().toString())
								{
									rulePassed = false;
								}
							}
							if (!rulePassed)
								break;
						}
					}
					if (rulePassed)
						allowed = rule["action"] == "allow";
				}
				if (!allowed)
					continue;
			}

			QString name = v["name"].toString();

			if (v["downloads"].isObject())
			{
				p.mDownloads << downloadFromJson("libraries/" + v["downloads"]["artifact"]["path"].toString(),
				                                 v["downloads"]["artifact"].toObject());

				bool extract = v["extract"].isObject();
				p.mDownloads.last().mExtract = extract;

				if (v["downloads"]["classifiers"].isObject())
				{
					auto k = v["downloads"]["classifiers"]["natives-windows"];
					if (k.isObject())
					{
						p.mDownloads.last().mExtract = false;
						p.mDownloads << downloadFromJson("libraries/" + k["path"].toString(), k.toObject());
						p.mDownloads.last().mExtract = extract;
						p.mClasspath << ClasspathEntry{"libraries/" + k["path"].toString()};
					}
				}
			}
			else
			{
				// Minecraft Forge-style entry
				QString url = "https://libraries.minecraft.net/";
				if (v["url"].isString())
				{
					url = v["url"].toString();
					if (!url.endsWith("/"))
					{
						url += "/";
					}
				}
				p.mDownloads << Download{
					"libraries/" + javaLibNameToPath(name), url + javaLibNameToPath(name), 0, false, ""
				};
			}

			if (v["downloads"].isObject())
				p.mClasspath << ClasspathEntry{"libraries/" + v["downloads"]["artifact"]["path"].toString()};
			else
				p.mClasspath << ClasspathEntry{"libraries/" + javaLibNameToPath(name)};
		}


		auto args = object["arguments"].toObject();

		unsigned counter = 0;

		auto parseConditions = [](QList<QPair<QString, QVariant>>& dst, const QJsonArray& in)
		{
			// find positive conditions

			for (auto& r : in)
			{
				auto rule = r.toObject();
				if (rule["action"].toString() == "allow")
				{
					for (auto& key : rule.keys())
					{
						if (key == "features")
						{
							auto features = rule[key].toObject();
							for (auto& featureName : features.keys())
							{
								dst << QPair<QString, QVariant>{featureName, features[featureName].toVariant()};
							}
						}
						else if (key != "action")
						{
							if (rule[key].isObject())
							{
								auto sub = rule[key].toObject();
								for (auto& subKey : sub.keys())
								{
									dst << QPair<QString, QVariant>{key + '.' + subKey, sub[subKey].toVariant()};
								}
							}
							else
							{
								dst << QPair<QString, QVariant>{key, rule[key].toVariant()};
							}
						}
					}
				}
			}
		};

		// Game args
		{
			Profile::GameArg arg;

			auto processStringObject = [&](const QString& value)
			{
				if (counter % 2 == 0)
				{
					arg.mName = value;
				}
				else
				{
					arg.mValue = value;
					for (auto it = p.mGameArgs.begin(); it != p.mGameArgs.end();)
					{
						if (it->mName == arg.mName)
						{
							it = p.mGameArgs.erase(it);
						}
						else
						{
							++it;
						}
					}
					p.mGameArgs << arg;
					arg = {};
				}

				counter += 1;
			};

			if (object["minecraftArguments"].isString())
			{
				// old-style args
				QStringList args = object["minecraftArguments"].toString().split(' ');
				for (auto& s : args)
					if (!s.isEmpty())
						processStringObject(s);
			}
			else
			{
				for (QJsonValue a : args["game"].toArray())
				{
					if (a.isString())
					{
						processStringObject(a.toString());
					}
					else if (a.isObject())
					{
						// flush last arg
						if (counter % 2 == 1)
						{
							p.mGameArgs << arg;
							arg = {};
							counter += 1;
						}

						parseConditions(arg.mConditions, a["rules"].toArray());

						// Add arguments
						if (a["value"].isArray())
						{
							for (auto value : a["value"].toArray())
							{
								if (counter % 2 == 0)
								{
									arg.mName = value.toString();
								}
								else
								{
									arg.mValue = value.toString();
									p.mGameArgs << arg;
									arg.mName.clear();
									arg.mValue.clear();
								}
								counter += 1;
							}
							if (counter % 2 == 1)
							{
								p.mGameArgs << arg;
							}

							arg = {};
						}
						else if (a["value"].isString())
						{
							arg.mName = a["value"].toString();
							p.mGameArgs << arg;
							arg = {};
						}
					}
				}
			}
		}

		// JVM args
		for (QJsonValue a : args["jvm"].toArray())
		{
			if (a.isString())
			{
				p.mJavaArgs << Profile::JavaArg{a.toString()};
			}
			else if (a.isObject())
			{
				QList<QPair<QString, QVariant>> conditions;
				parseConditions(conditions, a["rules"].toArray());

				if (a["value"].isString())
				{
					p.mJavaArgs << Profile::JavaArg{a["value"].toString(), conditions};
				}
				else if (a["value"].isArray())
				{
					for (auto& i : a["value"].toArray())
					{
						p.mJavaArgs << Profile::JavaArg{i.toString(), conditions};
					}
				}
			}
		}

		if (p.mJavaArgs.isEmpty())
		{
			// add some important Java args
			p.mJavaArgs << JavaArg{"-Djava.library.path=${natives_directory}"};
			p.mJavaArgs << JavaArg{"-cp"} << JavaArg{"${classpath}"};
		}
		cleanupNeeded = true;
	}

	// classpath order fix for Optifine 1.15.2
	if (object["inheritsFrom"].isString())
	{
		auto tmp1 = p.mJavaArgs;
		auto tmp2 = p.mGameArgs;
		launcher->tryLoadProfile(p, object["inheritsFrom"].toString());
		p.mJavaArgs << tmp1;
		p.mGameArgs << tmp2;

		// if the profile does not have it's own main jar file, we can copy it from
		// the inherited profile.
		// in theory, it would work recursively too.
		auto mainJarAbsPath = launcher->getSettings()->getGameDir().absoluteFilePath(
			"versions/" + name + "/" + name + ".jar");
		if (!QFile(mainJarAbsPath).exists())
		{
			QFile::copy(
				launcher->getSettings()->getGameDir().absoluteFilePath("versions/" + p.mName + "/" + p.mName + ".jar"),
				mainJarAbsPath);
		}
	}

	if (cleanupNeeded)
		p.makeClean();

	p.mName = name;

	if (object["mainClass"].isString())
		p.mMainClass = object["mainClass"].toString();
	if (object["assets"].isString())
		p.mAssetsIndex = object["assets"].toString();


	if (!object["hackers-mc"].isBool())
	{
		// again, due to Optifine 1.15.2 load order main game jar should be in the end of classpath load order.

		// client jar
		{
			auto path = "versions/" + object["id"].toString() + '/' + object["id"].toString() + ".jar";
			p.mDownloads << downloadFromJson(path, object["downloads"].toObject()["client"].toObject());
			p.mClasspath << ClasspathEntry{path};
		}

		// Asset index
		p.mDownloads << Download{
			"assets/indexes/" + p.mAssetsIndex + ".json",
			object["assetIndex"].toObject()["url"].toString(),
			quint64(object["assetIndex"].toObject()["size"].toInt()),
			false,
			object["assetIndex"].toObject()["sha1"].toString()
		};
	}
}

// TODO downloads storage
QJsonObject Profile::toJson()
{
	QJsonObject object;

	object["mainClass"] = mMainClass;
	object["assets"] = mAssetsIndex;

	// export to minecraft legacy launcher format is not supported
	object["hackers-mc"] = true;

	// Downloads
	QJsonArray downloads;
	for (auto& d : mDownloads)
	{
		QJsonObject entry;
		entry["local"] = d.mLocalPath;
		entry["url"] = d.mUrl;
		entry["size"] = int(d.mSize);
		entry["sha1"] = d.mHash;
		entry["extract"] = d.mExtract;

		downloads << entry;
	}
	object["downloads"] = downloads;

	// game args
	QJsonArray gameArgs;
	for (auto& d : mGameArgs)
	{
		QJsonObject entry;
		entry["name"] = d.mName;
		entry["value"] = d.mValue;

		QJsonArray conditions;

		for (auto& c : d.mConditions)
		{
			QJsonArray e;
			e << c.first;
			e << c.second.toJsonValue();
			conditions << e;
		}
		if (!conditions.isEmpty())
			entry["conditions"] = conditions;

		gameArgs << entry;
	}
	object["game_args"] = gameArgs;

	// java args
	QJsonArray javaArgs;
	for (auto& d : mJavaArgs)
	{
		QJsonObject entry;
		entry["name"] = d.mName;

		QJsonArray conditions;

		for (auto& c : d.mConditions)
		{
			QJsonArray e;
			e << c.first;
			e << c.second.toJsonValue();
			conditions << e;
		}
		if (!conditions.isEmpty())
			entry["conditions"] = conditions;

		javaArgs << entry;
	}
	object["java_args"] = javaArgs;


	// Classpath
	QJsonArray classpath;
	for (auto& lib : mClasspath)
	{
		classpath << lib.mPath;
	}
	object["classpath"] = classpath;

	// Settings
	QJsonObject settings;

	settings["window_width"] = mWindowWidth;
	settings["window_height"] = mWindowHeight;
	settings["fullscreen"] = mIsFullscreen;

	object["settings"] = settings;

	return object;
}

void Profile::save(HackersMCLauncher* launcher)
{
	auto dir = launcher->getSettings()->getGameDir().absoluteFilePath("versions/" + mName);
	{
		QDir d = dir;
		if (!d.exists())
			d.mkpath(dir);
	}
	QFile f = dir + '/' + mName + ".hck.json";
	f.open(QIODevice::WriteOnly);
	f.write(QJsonDocument(toJson()).toJson());
	f.close();
}

QString Profile::id() const
{
	return Crypto::md5(mName).mid(0, 32);
}
