#include "SettingsForm.h"
#include <QDialogButtonBox>

SettingsForm::SettingsForm(QWidget *parent)
	: Form(parent)
{
	ui.setupUi(this);

	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &SettingsForm::close);

	ui.logo->setPixmap(ui.logo->pixmap()->scaled(64, 64,
		Qt::IgnoreAspectRatio, Qt::FastTransformation));
}

SettingsForm::~SettingsForm()
{
}
