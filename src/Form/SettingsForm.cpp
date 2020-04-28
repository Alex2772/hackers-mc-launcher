#include "SettingsForm.h"
#include <QDialogButtonBox>
#include "../HackersMCLauncher.h"
#include <QDataWidgetMapper>
#include <QFileDialog>
#include <Settings.h>
#include "launcher_config.h"
#include <QMessageBox>
#include "RepositoryForm.h"


SettingsForm::SettingsForm(HackersMCLauncher* launcher)
	: Form(launcher)
{
	ui.setupUi(this);

	ui.version->setText(tr("Version") + ": " + LAUNCHER_VERSION);

	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &SettingsForm::close);

	// Common
	
	auto mapper = new QDataWidgetMapper(this);
	mapper->setModel(launcher->getSettings());
	mapper->addMapping(ui.gameDir, launcher->getSettings()->sectionOf("game_dir"));
	mapper->addMapping(ui.hideLauncher, launcher->getSettings()->sectionOf("hide_launcher"));
	mapper->addMapping(ui.closeLauncher, launcher->getSettings()->sectionOf("close_launcher"));
	mapper->addMapping(ui.showConsole, launcher->getSettings()->sectionOf("show_console"));
	mapper->addMapping(ui.updateLauncher, launcher->getSettings()->sectionOf("check_launcher_updates"));

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
	
	ui.repos->setModel(launcher->getRepositories());
	ui.repos->setColumnWidth(0, 200);
	ui.logo->setPixmap(ui.logo->pixmap()->scaled(64, 64,
		Qt::IgnoreAspectRatio, Qt::FastTransformation));


	connect(ui.forceCheckUpdates, &QAbstractButton::clicked, launcher, &HackersMCLauncher::checkForUpdates);
	connect(ui.forceCheckUpdates, &QAbstractButton::clicked, ui.forceCheckUpdates, &QAbstractButton::setEnabled);
	connect(launcher, &HackersMCLauncher::updateCheckFinished, ui.forceCheckUpdates, [&]() {ui.forceCheckUpdates->setDisabled(false); });


	// Repositories
	connect(ui.repositoryReset, &QAbstractButton::clicked, this, [&, launcher]()
	{
		if (QMessageBox::information(this, tr("Reset to default"), 
			tr("The repositories will be removed from your list. Continue?"), 
			QMessageBox::Yes, QMessageBox::Cancel) == QMessageBox::Yes)
		{
			launcher->getRepositories()->setToDefault();			
		}
	});
	connect(ui.repositoryAdd, &QAbstractButton::clicked, this, [&, launcher]()
	{
		auto r = new RepositoryForm({}, this);
		connect(r, &RepositoryForm::result, this, [launcher](const Repository& r)
		{
			launcher->getRepositories()->add(r);
		});
		r->show();
	});

	auto updateRow = [&, launcher](int row)
	{
		auto r = new RepositoryForm(launcher->getRepositories()->getItems().at(row), this);
		connect(r, &RepositoryForm::result, this, [launcher, row](const Repository& r)
		{
			launcher->getRepositories()->update(row, r);
		});
		r->show();
	};
	
	connect(ui.repositoryModify, &QAbstractButton::clicked, this, [&, updateRow]()
	{
		int row = ui.repos->selectionModel()->currentIndex().row();
		if (row >= 0) {
			updateRow(row);
		}
	});
	connect(ui.repos, &QTreeView::doubleClicked, this, [updateRow](const QModelIndex& i)
	{
		updateRow(i.row());
	});

	connect(ui.repositoryDelete, &QAbstractButton::clicked, this, [&, launcher]()
	{
		int row = ui.repos->selectionModel()->currentIndex().row();
		if (row >= 0) {
			launcher->getRepositories()->removeRow(row);
		}
	});

}

SettingsForm::~SettingsForm()
{
}
