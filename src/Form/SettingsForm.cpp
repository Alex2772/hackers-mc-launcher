#include "SettingsForm.h"
#include <QDialogButtonBox>
#include "../HackersMCLauncher.h"
#include <QDataWidgetMapper>
#include <QFileDialog>
#include <Settings.h>
#include "launcher_config.h"

SettingsForm::SettingsForm(HackersMCLauncher* launcher)
	: Form(launcher)
{
	ui.setupUi(this);

	ui.version->setText(tr("Version") + ": " + LAUNCHER_VERSION);

	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &SettingsForm::close);

	auto mapper = new QDataWidgetMapper(this);
	mapper->setModel(launcher->getSettings());
	mapper->addMapping(ui.gameDir, launcher->getSettings()->sectionOf("game_dir"));
	mapper->addMapping(ui.hideLauncher, launcher->getSettings()->sectionOf("hide_launcher"));
	mapper->addMapping(ui.closeLauncher, launcher->getSettings()->sectionOf("close_launcher"));
	mapper->addMapping(ui.showConsole, launcher->getSettings()->sectionOf("show_console"));

	mapper->toFirst();

	connect(ui.gameDirTT, &QAbstractButton::clicked, this, [&]()
	{
		QFileDialog d;
		d.setFileMode(QFileDialog::DirectoryOnly);
		d.setOption(QFileDialog::ShowDirsOnly, false);
		d.exec();

		if (!d.directory().absolutePath().isEmpty())
		{
			ui.gameDir->setText(d.directory().absolutePath());
			ui.gameDir->setFocus();
		}
	});
	
	ui.repoList->setModel(launcher->getRepositories());	
	ui.logo->setPixmap(ui.logo->pixmap()->scaled(64, 64,
		Qt::IgnoreAspectRatio, Qt::FastTransformation));
}

SettingsForm::~SettingsForm()
{
}
