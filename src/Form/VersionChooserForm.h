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
	VersionChooserForm(QAbstractItemModel* profiles, const QModelIndex& index, HackersMCLauncher* launcher);
	~VersionChooserForm();

private:
	Ui::VersionChooserForm ui;

	unsigned mRequests = 0;
	
	QSet<QString> mTypes;
	QNetworkAccessManager mNet;
	VersionTreeModel* mTree;
	FilterProxyModel* mFilter;

	void decrementRequests();

};
