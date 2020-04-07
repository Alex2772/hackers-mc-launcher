#include "HackersMCLauncher.h"
#include "Util/UIHelper.h"
#include <QFile>
#include <QScreen>
#include <QMouseEvent>
#include "Model/UsersListModel.h"
#include "Form/VersionChooserForm.h"
#include "Form/UserForm.h"
#include "Form/ProfileForm.h"
#include "Form/SettingsForm.h"
#include "Settings.h"
#include <QMessageBox>
#include "Layout/StackedLayout.h"

HackersMCLauncher::HackersMCLauncher(QWidget *parent)
	: QMainWindow(parent),
	mSettings(new Settings("hackers-mc-launcher", "hackers-mc-launcher", this))
{
	ui.setupUi(this);

	ui.content->setLayout(new StackedLayout(ui.content));
	ui.content->layout()->addWidget(ui.label);
	ui.content->layout()->addWidget(ui.frame);
	
	mRepos.setToDefault();
	
	screenScaleChanged();
	connect(QGuiApplication::primaryScreen(), &QScreen::logicalDotsPerInchChanged, this, &HackersMCLauncher::screenScaleChanged);

	// Users
	connect(ui.usersList, &QListView::clicked, this, [&](const QModelIndex& index)
	{
		if (index.row() == index.model()->rowCount() - 1)
		{
			mUsers.insertRow(mUsers.rowCount({}) - 1);
			(new UserForm(&mUsers, index, true, this))->show();
		}
	});
	connect(ui.usersList, &QListView::doubleClicked, this, [&](const QModelIndex& index)
	{
		if (index.row() < index.model()->rowCount() - 1)
		{
			(new UserForm(&mUsers, index, false, this))->show();
		}
	});

	// Profiles
	connect(ui.profilesList, &QListView::clicked, this, [&](const QModelIndex& index)
	{
		if (index.row() == index.model()->rowCount() - 1)
		{
			auto p = new VersionChooserForm(this);
			p->show();
		}
	});
	connect(ui.profilesList, &QListView::doubleClicked, this, [&](const QModelIndex& index)
	{
		if (index.row() < index.model()->rowCount() - 1)
		{
			(new ProfileForm(index, this))->show();
		}
	});

	connect(ui.settings, &QAbstractButton::clicked, this, [&]()
	{
		(new SettingsForm(this))->show();
	});
	
	ui.usersList->setModel(&mUsers);
	ui.profilesList->setModel(&mProfiles);

	// Play
	connect(ui.play, &QAbstractButton::clicked, this, &HackersMCLauncher::play);

	setDownloadMode(false);
}
bool HackersMCLauncher::nativeEvent(const QByteArray& eventType, void* message, long* result)
{
	if (UIHelper::customFrameNativeEvent(this, eventType, message, result))
	{
		return true;
	}
	return QMainWindow::nativeEvent(eventType, message, result);
}

void HackersMCLauncher::showEvent(QShowEvent* event)
{
	UIHelper::customWindowShowEvent(this);

	updateMiddleButton();
}

void HackersMCLauncher::changeEvent(QEvent* e)
{
	if (e->type() == QEvent::WindowStateChange)
	{
		updateMiddleButton();
	}
}

void HackersMCLauncher::updateMiddleButton()
{
	if (windowState() & Qt::WindowMaximized)
	{
		ui.middle->setPixmap(QIcon{ ":/hck/cap_restore.svg" }.pixmap(QSize(10, 10) * mUiScale));
		ui.caption->setContentsMargins(4, 5, 9, 0);
		ui.mfix->setContentsMargins(7, 0, 7, 7);
	}
	else
	{
		ui.caption->setContentsMargins(0, 0, 0, 0);
		ui.middle->setPixmap(QIcon{ ":/hck/cap_maximize.svg" }.pixmap(QSize(10, 10) * mUiScale));
		ui.mfix->setContentsMargins(0, 0, 0, 0);
	}
}

RepositoriesModel* HackersMCLauncher::getRepositories()
{
	return &mRepos;
}

ProfilesListModel& HackersMCLauncher::getProfiles()
{
	return mProfiles;
}

Settings* HackersMCLauncher::getSettings() const
{
	return mSettings;
}

void HackersMCLauncher::play()
{
	const QString errorTitle = tr("Could not launch");
	if (ui.play->isEnabled()) {
		if (!ui.usersList->selectionModel()->currentIndex().isValid())
		{
			QMessageBox::critical(this, errorTitle, tr("Please select the user to play for"));
			return;
		}
		if (!ui.profilesList->selectionModel()->currentIndex().isValid())
		{
			QMessageBox::critical(this, errorTitle, tr("Please select the profile to play with"));
			return;
		}

		setDownloadMode(true);
	}
}

void HackersMCLauncher::screenScaleChanged()
{
	static qreal prevScale = 96;
	qreal currentScale = QGuiApplication::primaryScreen()->logicalDotsPerInch();
	qreal factor = currentScale / prevScale;
	prevScale = currentScale;

	mUiScale = currentScale / 96;

	for (auto& w : findChildren<QWidget*>())
	{
		if (!w->property("fdpi").toBool())
		{
			w->setContentsMargins(w->contentsMargins() * factor);
			w->setMinimumSize(w->minimumSize() * factor);
			w->setMaximumSize(w->maximumSize() * factor);

			if (auto label = dynamic_cast<QLabel*>(w))
			{
				if (label->pixmap())
				{
					QString path = label->property("lbl_bg").toString();
					if (path.isEmpty())
						label->setPixmap(label->pixmap()->scaled(label->pixmap()->size() * factor));
					else
					{
						label->setPixmap(QIcon(path).pixmap(label->pixmap()->size() * factor));
					}
				}
			}
		}
	}
	for (auto& w : findChildren<QHBoxLayout*>())
	{
		if (!w->property("fdpi").toBool())
		{
			w->setContentsMargins(w->contentsMargins() * factor);
		}
	}
}

void HackersMCLauncher::setDownloadMode(bool m)
{
	ui.play->setEnabled(!m);
	ui.listsSplitter->setEnabled(!m);
	ui.downloadPanel->setVisible(m);
}

void HackersMCLauncher::mousePressEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
	{
		if (QWidget* w = childAt(QPoint(e->windowPos().x(), e->windowPos().y())))
		{
			if (w->property("cap").toBool())
			{
				w->setProperty("pressed", true);
				style()->unpolish(w);
				style()->polish(w);
			}
		}
	}
}

void HackersMCLauncher::mouseReleaseEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
	{
		if (QWidget* w = childAt(QPoint(e->windowPos().x(), e->windowPos().y())))
		{
			if (w->property("pressed").toBool())
			{
				if (w == ui.close)
				{
					close();
				}
				else if (w == ui.middle)
				{
					setWindowState(windowState() ^ Qt::WindowMaximized);
					updateMiddleButton();
				}
				else if (w == ui.minimize)
				{
					setWindowState(Qt::WindowMinimized);
				}
			}
		}
	}
	auto removePressedEffect = [&](QWidget* w)
	{
		if (w->property("pressed").toBool())
		{
			w->setProperty("pressed", false);
			style()->unpolish(w);
			style()->polish(w);
		}
	};
	removePressedEffect(ui.minimize);
	removePressedEffect(ui.middle);
	removePressedEffect(ui.close);
}