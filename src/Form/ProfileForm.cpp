#include "ProfileForm.h"

ProfileForm::ProfileForm(QAbstractItemModel* model, const QModelIndex& index, QWidget* parent)
	: Form(parent)
{
	ui.setupUi(this);
	
}

ProfileForm::~ProfileForm()
{
}
