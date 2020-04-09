#include "ProfileForm.h"
#include <HackersMCLauncher.h>
#include <QDataWidgetMapper>

#include "Model/GameArgsListModel.h"
#include "Model/JavaArgsListModel.h"
#include "Model/ClasspathListModel.h"
#include <QFileDialog>
#include <QDesktopServices>
#include "Settings.h"


ProfileForm::ProfileForm(const QModelIndex& index, HackersMCLauncher* parent)
	: Form(parent), mItem(&parent->getProfiles().profiles()[index.row()])
{
	ui.setupUi(this);
	connect(ui.buttonBox, &QDialogButtonBox::clicked, this, &ProfileForm::close);
	connect(&parent->getProfiles(), &QAbstractItemModel::dataChanged, this, &ProfileForm::updateCaption);
	connect(ui.openGameDir, &QAbstractButton::clicked, this, [parent]()
	{
		QDesktopServices::openUrl(QUrl::fromLocalFile(parent->getSettings()->getGameDir().absolutePath()));
	});

	// Common
	
	auto mapper = new QDataWidgetMapper(this);
	mapper->setModel(&parent->getProfiles());
	mapper->addMapping(ui.profilename, 0);
	mapper->setCurrentIndex(index.row());

	updateCaption();

	// Downloads
	auto downloads = new DownloadsModel(mItem->mDownloads, this);
	ui.dl_tree->setModel(downloads);
	ui.dl_tree->setColumnWidth(0, 200);
	ui.dl_tree->setColumnWidth(1, 150);
	ui.dl_tree->setColumnWidth(2, 70);
	ui.dl_tree->expandAll();

	connect(ui.dl_add, &QAbstractButton::clicked, this, [&, downloads]()
	{
		ui.dl_tree->setFocus();
		ui.dl_tree->setCurrentIndex(downloads->insertRow());
	});
	connect(ui.dl_delete, &QAbstractButton::clicked, this, [&, downloads]()
	{

		auto index = ui.dl_tree->selectionModel()->currentIndex();
		downloads->removeRow(index.row(), index.parent());
	});
	
	// Classpath
	auto classpath = new ClasspathListModel(mItem->mClasspath, this);
	ui.cp_list->setModel(classpath);

	connect(ui.cp_add, &QAbstractButton::clicked, this, [&, classpath]()
	{
		ui.cp_list->setFocus();
		ui.cp_list->setCurrentIndex(classpath->insertRow({}));
	});
	connect(ui.cp_choose, &QAbstractButton::clicked, this, [&, classpath, parent]()
	{
		auto start = parent->getSettings()->getGameDir().absolutePath() + '/';
		auto f = QFileDialog::getOpenFileName(this,
			tr("Add jar to classpath"), start, tr("Jar Files (*.jar)"));
		if (!f.isEmpty()) {
			if (f.startsWith(start))
				f = f.mid(start.length());
			ui.cp_list->setFocus();
			ui.cp_list->setCurrentIndex(classpath->insertRow({f}));
		}
	});
	connect(ui.cp_delete, &QAbstractButton::clicked, this, [&, classpath]()
	{

		auto index = ui.cp_list->selectionModel()->currentIndex();
		classpath->removeRow(index.row(), index.parent());
	});

	// Game args
	auto gameArgs = new GameArgsListModel(mItem, this);
	ui.ga_tree->setModel(gameArgs);
	ui.ga_tree->setColumnWidth(0, 110);
	ui.ga_tree->setColumnWidth(1, 200);
	ui.ga_tree->setColumnWidth(2, 200);

	connect(ui.ga_add, &QAbstractButton::clicked, this, [&, gameArgs]()
	{
		ui.ga_tree->setFocus();
		ui.ga_tree->setCurrentIndex(gameArgs->insertRow());
	});
	connect(ui.ga_delete, &QAbstractButton::clicked, this, [&, gameArgs]()
	{

		auto index = ui.ga_tree->selectionModel()->currentIndex();
		gameArgs->removeRow(index.row(), index.parent());
	});
	// Java args
	auto javaArgs = new JavaArgsListModel(mItem, this);
	ui.ja_tree->setModel(javaArgs);
	ui.ja_tree->setColumnWidth(0, 300);
	ui.ja_tree->setColumnWidth(1, 220);

	connect(ui.ja_add, &QAbstractButton::clicked, this, [&, javaArgs]()
	{
		ui.ja_tree->setFocus();
		ui.ja_tree->setCurrentIndex(javaArgs->insertRow());
	});
	connect(ui.ja_delete, &QAbstractButton::clicked, this, [&, javaArgs]()
	{

		auto index = ui.ja_tree->selectionModel()->currentIndex();
		javaArgs->removeRow(index.row(), index.parent());
	});


	connect(ui.run, &QAbstractButton::clicked, this, [&, parent]()
	{
		parent->play();
		close();
	});
}

void ProfileForm::updateCaption()
{
	setWindowTitle(mItem->mName);
}
ProfileForm::~ProfileForm()
{
}
