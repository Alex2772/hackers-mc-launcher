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
			Profile p = Profile::fromJson(mLauncher, item->mName, QJsonDocument::fromJson(reply->readAll()).object());


			(new ProfileForm(mLauncher->getProfiles().add(std::move(p)), mLauncher))->show();
			close();
		});
	}
}

VersionChooserForm::~VersionChooserForm()
{
}
