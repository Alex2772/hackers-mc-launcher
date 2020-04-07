#include "ProfileForm.h"
#include <HackersMCLauncher.h>
#include <QDataWidgetMapper>
#include "JavaLibForm.h"
#include "Model/GameArgsListModel.h"
#include "Model/JavaArgsListModel.h"


ProfileForm::ProfileForm(const QModelIndex& index, HackersMCLauncher* parent)
	: Form(parent), mItem(&parent->getProfiles().profiles()[index.row()])
{
	ui.setupUi(this);
	connect(ui.buttonBox, &QDialogButtonBox::clicked, this, &ProfileForm::close);
	connect(&parent->getProfiles(), &QAbstractItemModel::dataChanged, this, &ProfileForm::updateCaption);

	// Common
	
	auto mapper = new QDataWidgetMapper(this);
	mapper->setModel(&parent->getProfiles());
	mapper->addMapping(ui.profilename, 0);
	mapper->setCurrentIndex(index.row());

	updateCaption();

	// Java libs
	auto javaLibs = new JavaLibModel(mItem->mJavaLibs, this);
	ui.jl_tree->setModel(javaLibs);
	ui.jl_tree->setColumnWidth(0, 200);
	ui.jl_tree->setColumnWidth(1, 150);
	ui.jl_tree->setColumnWidth(2, 70);
	ui.jl_tree->expandAll();

	connect(ui.jl_add, &QAbstractButton::clicked, this, [&, javaLibs]()
	{
		(new JavaLibForm(javaLibs, this))->show();
	});
	connect(ui.jl_delete, &QAbstractButton::clicked, this, [&, javaLibs]()
	{

		auto index = ui.jl_tree->selectionModel()->currentIndex();
		javaLibs->removeRow(index.row(), index.parent());
	});

	// Game args
	auto gameArgs = new GameArgsListModel(mItem, this);
	ui.ga_table->setModel(gameArgs);
	ui.ga_table->setColumnWidth(0, 110);
	ui.ga_table->setColumnWidth(1, 200);
	ui.ga_table->setColumnWidth(2, 200);

	connect(ui.ga_add, &QAbstractButton::clicked, this, [&, gameArgs]()
	{
		ui.ga_table->setFocus();
		ui.ga_table->setCurrentIndex(gameArgs->insertRow());
	});
	connect(ui.ga_delete, &QAbstractButton::clicked, this, [&, gameArgs]()
	{

		auto index = ui.ga_table->selectionModel()->currentIndex();
		gameArgs->removeRow(index.row(), index.parent());
	});
	// Java args
	auto javaArgs = new JavaArgsListModel(mItem, this);
	ui.ja_table->setModel(javaArgs);
	ui.ja_table->setColumnWidth(0, 300);
	ui.ja_table->setColumnWidth(1, 220);

	connect(ui.ja_add, &QAbstractButton::clicked, this, [&, javaArgs]()
	{
		ui.ja_table->setFocus();
		ui.ja_table->setCurrentIndex(javaArgs->insertRow());
	});
	connect(ui.ja_delete, &QAbstractButton::clicked, this, [&, javaArgs]()
	{

		auto index = ui.ja_table->selectionModel()->currentIndex();
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
