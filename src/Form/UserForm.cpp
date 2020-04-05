#include "UserForm.h"
#include <QDataWidgetMapper>
#include <qabstractitemmodel.h>

UserForm::UserForm(QAbstractItemModel* model, const QModelIndex& index, QWidget *parent)
	: Form(parent)
{
	ui.setupUi(this);

	auto mapper = new QDataWidgetMapper(this);
	
	mapper->setModel(model);

	mapper->addMapping(ui.username, 0);
	
	connect(ui.buttonBox, &QDialogButtonBox::rejected, this, [&, model]()
	{
		model->removeRow(index.row());
;		close();
	});
	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, [&]()
	{
		close();
	});
	mapper->setCurrentIndex(index.row());
}

UserForm::~UserForm()
{
}
