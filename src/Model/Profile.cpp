#include "Profile.h"
#include <QJsonObject>
#include <QJsonArray>
#include "Util/VariableHelper.h"
#include "HackersMCLauncher.h"
#include "Settings.h"
#include <QJsonDocument>

Download downloadFromJson(const QString& path, const QJsonObject& v)
{
	return Download{
		path, v["url"].toString(),
		quint64(v["size"].toInt()), v["sha1"].toString()
	};
}

Profile Profile::fromJson(HackersMCLauncher* launcher, const QString& name, const QJsonObject& object)
{
	Profile p;
	p.mName = name;

	p.mMainClass = object["mainClass"].toString();
	p.mAssetsIndex = object["assets"].toString();

	p.mDownloads << Download{
		"assets/indexes/" + p.mAssetsIndex + ".json",
		object["assetIndex"].toObject()["url"].toString(),
		quint64(object["assetIndex"].toObject()["totalSize"].toInt()),
		object["assetIndex"].toObject()["sha1"].toString()
	};

	// client jar
	{
		auto path = "versions/" + object["id"].toString() + '/' + object["id"].toString() + ".jar";
		p.mDownloads << downloadFromJson(path, object["downloads"].toObject()["client"].toObject());
		p.mClasspath << ClasspathEntry{path};
	}
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
								                                                                          .toVariant().
								                                                                          toString())
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

		p.mDownloads << downloadFromJson("libraries/" + v["downloads"]["artifact"]["path"].toString(),
		                                 v["downloads"]["artifact"].toObject());
		p.mClasspath << ClasspathEntry{"libraries/" + v["downloads"]["artifact"]["path"].toString()};

		if (v["downloads"]["classifiers"].isObject())
		{
			auto k = v["downloads"]["classifiers"]["natives-windows"];
			if (k.isObject())
			{
				p.mDownloads << downloadFromJson("libraries/" + k["path"].toString(), k.toObject());
				p.mClasspath << ClasspathEntry{"libraries/" + k["path"].toString()};
			}
		}
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
		for (QJsonValue a : args["game"].toArray())
		{
			if (a.isString())
			{
				if (counter % 2 == 0)
				{
					arg.mName = a.toString();
				}
				else
				{
					arg.mValue = a.toString();
					p.mGameArgs << arg;
					arg = {};
				}

				counter += 1;
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
	return p;
}

// TODO downloads storage
QJsonObject Profile::toJson()
{
	QJsonObject object;

	object["mainClass"] = mMainClass;
	object["assets"] = mAssetsIndex;

	// libraries

	QJsonArray libraries;

	for (auto& lib : mClasspath)
	{
		
	}

	object["libraries"] = libraries;

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
