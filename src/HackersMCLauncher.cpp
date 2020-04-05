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

HackersMCLauncher::HackersMCLauncher(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	{
		QFile f(":/hck/style.css");
		f.open(QIODevice::ReadOnly);
		setStyleSheet(f.readAll());
	}

	mRepos.setToDefault();
	
	screenScaleChanged();
	connect(QGuiApplication::primaryScreen(), &QScreen::logicalDotsPerInchChanged, this, &HackersMCLauncher::screenScaleChanged);

	// Users
	connect(ui.usersList, &QListView::clicked, this, [&](const QModelIndex& index)
	{
		if (index.row() == index.model()->rowCount() - 1)
		{
			mUsers.insertRow(mUsers.rowCount({}) - 1);
			(new UserForm(&mUsers, index, this))->show();
		}
	});
	connect(ui.usersList, &QListView::doubleClicked, this, [&](const QModelIndex& index)
	{
		if (index.row() < index.model()->rowCount() - 1)
		{
			(new UserForm(&mUsers, index, this))->show();
		}
	});

	// Profiles
	connect(ui.profilesList, &QListView::clicked, this, [&](const QModelIndex& index)
	{
		if (index.row() == index.model()->rowCount() - 1)
		{
			mProfiles.insertRow(mProfiles.rowCount({}) - 1);
			auto p = new VersionChooserForm(&mProfiles, index, this);
			p->show();
		}
	});
	connect(ui.profilesList, &QListView::doubleClicked, this, [&](const QModelIndex& index)
	{
		if (index.row() < index.model()->rowCount() - 1)
		{
			(new ProfileForm(&mProfiles, index, this))->show();
		}
	});

	connect(ui.settings, &QAbstractButton::clicked, this, [&]()
	{
		(new SettingsForm(this))->show();
	});
	
	ui.usersList->setModel(&mUsers);
	ui.profilesList->setModel(&mProfiles);
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