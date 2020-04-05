#include "VersionChooserForm.h"
#include <QNetworkReply>
#include "Model/VersionTreeModel.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCheckBox>


VersionChooserForm::VersionChooserForm(QAbstractItemModel* profiles, const QModelIndex& index, HackersMCLauncher* launcher)
	: Form(launcher)
{
	ui.setupUi(this);

	mTree = new VersionTreeModel(this);
	mFilter = new FilterProxyModel(1, this);
	mFilter->setSourceModel(mTree);
	ui.treeView->setModel(mFilter);

	ui.treeView->setColumnWidth(0, 220);
	
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

VersionChooserForm::~VersionChooserForm()
{
}
