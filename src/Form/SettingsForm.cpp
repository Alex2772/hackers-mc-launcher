#include "SettingsForm.h"
#include <QDialogButtonBox>
#include "../HackersMCLauncher.h"

SettingsForm::SettingsForm(HackersMCLauncher* launcher)
	: Form(launcher)
{
	ui.setupUi(this);

	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &SettingsForm::close);

	ui.repoList->setModel(launcher->getRepositories());
	
	
	ui.logo->setPixmap(ui.logo->pixmap()->scaled(64, 64,
		Qt::IgnoreAspectRatio, Qt::FastTransformation));
}

SettingsForm::~SettingsForm()
{
}
