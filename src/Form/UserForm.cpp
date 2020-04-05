#include "UserForm.h"
#include <QDataWidgetMapper>
#include <qabstractitemmodel.h>

UserForm::UserForm(QAbstractItemModel* model, const QModelIndex& index, bool newUser, QWidget* parent)
	: Form(parent), mModel(model), mIndex(index), mPositiveResult(!newUser)
{
	ui.setupUi(this);

	auto mapper = new QDataWidgetMapper(this);
	
	mapper->setModel(model);

	mapper->addMapping(ui.username, 0);
	
	connect(ui.buttonBox, &QDialogButtonBox::rejected, this, [&]()
	{
;		close();
	});
	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, [&]()
	{
		mPositiveResult = true;
		close();
	});
	mapper->setCurrentIndex(index.row());
}

UserForm::~UserForm()
{
	if (!mPositiveResult)
	{
		mModel->removeRow(mIndex.row());
	}
}
