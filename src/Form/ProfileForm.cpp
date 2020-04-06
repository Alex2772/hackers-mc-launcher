#include "ProfileForm.h"
#include <HackersMCLauncher.h>
#include <QDataWidgetMapper>


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
	ui.jl_table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	ui.jl_table->verticalHeader()->setDefaultSectionSize(20);
	ui.jl_table->setColumnWidth(0, 200);
	ui.jl_table->setModel(new JavaLibListModel(mItem->mJavaLibs, this));
}

void ProfileForm::updateCaption()
{
	setWindowTitle(mItem->mName);
}
ProfileForm::~ProfileForm()
{
}
