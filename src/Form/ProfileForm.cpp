#include "ProfileForm.h"
#include <HackersMCLauncher.h>
#include <QDataWidgetMapper>
#include "JavaLibForm.h"


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
}

void ProfileForm::updateCaption()
{
	setWindowTitle(mItem->mName);
}
ProfileForm::~ProfileForm()
{
}
