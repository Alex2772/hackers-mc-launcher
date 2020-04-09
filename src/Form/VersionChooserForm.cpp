#include "VersionChooserForm.h"
#include <QNetworkReply>
#include "Model/VersionTreeModel.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include "ProfileForm.h"
#include "Util/StringHelper.h"
#include "Util/VariableHelper.h"


VersionChooserForm::VersionChooserForm(HackersMCLauncher* launcher)
	: Form(launcher),
	  mLauncher(launcher)
{
	ui.setupUi(this);

	mTree = new VersionTreeModel(this);
	mFilter = new FilterProxyModel(1, this);
	mFilter->setSourceModel(mTree);
	ui.treeView->setModel(mFilter);
	ui.treeView->setColumnWidth(0, 220);
	ui.treeView->expandAll();

	ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

	connect(ui.treeView, &QTreeView::clicked, this, [&](const QModelIndex& index)
	{
		VersionTreeModel::Item* item = static_cast<VersionTreeModel::Item*>(mFilter
		                                                                    ->mapToSource(index).internalPointer());
		ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(
			item->parent() != nullptr && item->parent()->parent() != nullptr);
	});
	connect(ui.treeView, &QTreeView::doubleClicked, this, &VersionChooserForm::onVersionSelected);
	connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &VersionChooserForm::close);
	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, [&]()
	{
		onVersionSelected(ui.treeView->selectionModel()->currentIndex());
	});

	for (auto& repo : launcher->getRepositories()->getItems())
	{
		auto i = new VersionTreeModel::Item(repo.mName);
		mTree->root().addChild(i);
		auto reply = mNet.get(QNetworkRequest(QUrl(repo.mUrl)));
		connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, [&, i]()
		{
			i->mIsErrored = true;
			decrementRequests();
		});
		connect(reply, &QNetworkReply::finished, this, [&, reply, i]()
		{
			auto root = QJsonDocument::fromJson(reply->readAll()).object();

			auto latest = root["latest"].toObject();

			for (QJsonValue version : root["versions"].toArray())
			{
				auto item = new VersionTreeModel::Item(version["id"].toString());
				item->mType = version["type"].toString();
				item->mUrl = version["url"].toString();
				mTypes << item->mType;
				item->mIsLatest = latest[item->mType].toString() == item->mName;
				// 2019-08-22T12:06:21+00:00
				auto releaseTime = version["releaseTime"].toString();
				item->mReleaseDate = QDateTime::fromString(releaseTime, "yyyy-MM-dd'T'hh:mm:ss'+00:00'");
				i->addChild(item);
			}
			mTree->notifyUpdated(i);
			decrementRequests();
		});
		mRequests += 1;
	}
}

void VersionChooserForm::decrementRequests()
{
	mRequests -= 1;
	if (mRequests == 0)
	{
		ui.progressBar->setVisible(false);
		QList<QString> types(mTypes.begin(), mTypes.end());
		types.sort();
		for (auto& type : types)
		{
			auto cb = new QCheckBox(tr(type.toStdString().c_str()));
			cb->setChecked(true);

			connect(cb, &QCheckBox::clicked, this, [&, type](bool checked)
			{
				mFilter->filters()[type] = checked;
				mFilter->invalidate();
			});
			ui.filters->layout()->addWidget(cb);
		}
		mFilter->invalidate();
		ui.filters->layout()->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
	}
}

Download downloadFromJson(const QString& path, const QJsonObject& v)
{
	return Download{ path, v["url"].toString(),
											quint64(v["size"].toInt()), v["sha1"].toString() };
}

void VersionChooserForm::onVersionSelected(const QModelIndex& index)
{
	if (ui.buttonBox->button(QDialogButtonBox::Ok)->isEnabled())
	{
		ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
		ui.progressBar->setVisible(true);
		VersionTreeModel::Item* item = static_cast<VersionTreeModel::Item*>(mFilter
		                                                                    ->mapToSource(index).internalPointer());

		auto reply = mNet.get(QNetworkRequest(QUrl(item->mUrl)));

		connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, [&]()
		{
			QMessageBox::critical(this, tr("Could not download file"), tr("Unknown error"));
			ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
			ui.progressBar->setVisible(false);
		});
		connect(reply, &QNetworkReply::finished, this, [&, item, reply]()
		{
			Profile p;
			p.mName = item->mName;
			QJsonObject o = QJsonDocument::fromJson(reply->readAll()).object();

			p.mMainClass = o["mainClass"].toString();
			p.mAssetsIndex = o["assets"].toString();

			p.mDownloads << Download{ "assets/indexes/" + p.mAssetsIndex + ".json",
				o["assetIndex"].toObject()["url"].toString(), quint64(o["assetIndex"].toObject()["totalSize"].toInt()), o["assetIndex"].toObject()["sha1"].toString() };
			
			// client jar
			{
				auto path = "versions/" + o["id"].toString() + '/' + o["id"].toString() + ".jar";
				p.mDownloads << downloadFromJson(path, o["downloads"].toObject()["client"].toObject());
				p.mClasspath << ClasspathEntry{ path };
			}
			// Java libraries
			for (QJsonValue v : o["libraries"].toArray())
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
										if (VariableHelper::getVariableValue(mLauncher, k + '.' + v).toString() != x[v].toVariant().toString())
										{
											rulePassed = false;
											break;
										}
									}
								} else
								{
									if (VariableHelper::getVariableValue(mLauncher, k).toString() != rule[k].toVariant().toString())
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

				p.mDownloads << downloadFromJson("libraries/" + v["downloads"]["artifact"]["path"].toString(), v["downloads"]["artifact"].toObject());
				p.mClasspath << ClasspathEntry{ "libraries/" + v["downloads"]["artifact"]["path"].toString() };

				if (v["downloads"]["classifiers"].isObject())
				{
					auto k = v["downloads"]["classifiers"]["natives-windows"];
					if (k.isObject())
					{
						p.mDownloads << downloadFromJson("libraries/" + k["path"].toString(), k.toObject());
						p.mClasspath << ClasspathEntry{ "libraries/" + k["path"].toString() };
					}
				}
			}
			

			auto args = o["arguments"].toObject();

			unsigned counter = 0;

			auto parseConditions = [](QList<QPair<QString, QVariant>>& dst, const QJsonArray& in)
			{
				// find positive conditions

				for (auto& r : in)
				{
					auto rule = r.toObject();
					if (rule["action"].toString() == "allow")
					{
						for (auto& key : rule.keys()) {
							if (key == "features") {
								auto features = rule[key].toObject();
								for (auto& featureName : features.keys())
								{
									dst << QPair<QString, QVariant>{featureName, features[featureName].toVariant()};
								}
							} else if (key != "action")
							{
								if (rule[key].isObject())
								{
									auto sub = rule[key].toObject();
									for (auto& subKey : sub.keys())
									{
										dst << QPair<QString, QVariant>{key + '.' + subKey, sub[subKey].toVariant()};
									}
								} else
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
						p.mJavaArgs << Profile::JavaArg{ a["value"].toString(), conditions };
					} else if (a["value"].isArray())
					{
						for (auto& i : a["value"].toArray())
						{
							p.mJavaArgs << Profile::JavaArg{ i.toString(), conditions };
						}
					}
				}
			}

			(new ProfileForm(mLauncher->getProfiles().add(std::move(p)), mLauncher))->show();
			close();
		});
	}
}

VersionChooserForm::~VersionChooserForm()
{
}
