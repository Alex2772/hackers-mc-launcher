#pragma once

#include "Form.h"
#include "ui_VersionChooserForm.h"
#include <HackersMCLauncher.h>
#include <QNetworkAccessManager>
#include <QSet>
#include "Model/VersionTreeModel.h"
#include "Model/FilterProxyModel.h"

class VersionChooserForm : public Form
{
	Q_OBJECT

public:
	VersionChooserForm(HackersMCLauncher* launcher);
	~VersionChooserForm() override;

private:
	Ui::VersionChooserForm ui;

	unsigned mRequests = 0;
	
	QSet<QString> mTypes;
	QNetworkAccessManager mNet;
	VersionTreeModel* mTree;
	FilterProxyModel* mFilter;
	HackersMCLauncher* mLauncher;

	void decrementRequests();

private slots:
	void onVersionSelected(const QModelIndex& index);
};
