#include "HackersMCLauncher.h"
#include "Util/UIHelper.h"
#include <QScreen>
#include <QTimer>
#include <QMouseEvent>
#include "Model/UsersListModel.h"
#include "Form/VersionChooserForm.h"
#include "Form/UserForm.h"
#include "Form/ProfileForm.h"
#include "Form/SettingsForm.h"
#include "Settings.h"
#include <QMessageBox>
#include <QThreadPool>
#include "Layout/StackedLayout.h"
#include "Util/LambdaTask.h"
#include "Util/UIThread.h"
#include "Util/Crypto.h"
#include "Util/StringHelper.h"
#include <QNetworkReply>

HackersMCLauncher::HackersMCLauncher(QWidget* parent)
	: QMainWindow(parent),
	  mSettings(new Settings("hackers-mc-launcher", "hackers-mc-launcher", this))
{
	ui.setupUi(this);

	ui.content->setLayout(new StackedLayout(ui.content));
	ui.content->layout()->addWidget(ui.label);
	ui.content->layout()->addWidget(ui.frame);

	mRepos.setToDefault();

	screenScaleChanged();
	connect(QGuiApplication::primaryScreen(), &QScreen::logicalDotsPerInchChanged, this,
	        &HackersMCLauncher::screenScaleChanged);

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
		ui.middle->setPixmap(QIcon{":/hck/cap_restore.svg"}.pixmap(QSize(10, 10) * mUiScale));
		ui.caption->setContentsMargins(4, 5, 9, 0);
		ui.mfix->setContentsMargins(7, 0, 7, 7);
	}
	else
	{
		ui.caption->setContentsMargins(0, 0, 0, 0);
		ui.middle->setPixmap(QIcon{":/hck/cap_maximize.svg"}.pixmap(QSize(10, 10) * mUiScale));
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


void HackersMCLauncher::play(bool withUpdate)
{
	const QString errorTitle = tr("Could not launch");
	if (ui.play->isEnabled())
	{
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
		ui.progressBar->setMaximum(0);
		ui.downloaded->setText("-");
		ui.total->setText("-");
		ui.speed->setText("-");
		ui.eta->setText("-");
		setDownloadMode(true);

		auto& profile = mProfiles.profiles().at(ui.profilesList->selectionModel()->currentIndex().row());
		auto& user = mUsers.users().at(ui.usersList->selectionModel()->currentIndex().row());

		QThreadPool::globalInstance()->start(lambda([&, withUpdate]()
		{
			auto setStatus = [&](const QString& s)
			{
				UIThread::run([&, s]()
				{
					ui.status->setText(s);
				});
			};

			setStatus(tr("Searching files..."));

			QDir gameDir = mSettings->getGameDir();

			// Determine download list and count total download size

			quint64 totalDownloadSize = 0;
			typedef QPair<QString, QUrl> D; // just for convenience
			QList<D> downloads;

			for (auto d : profile.mDownloads)
			{
				auto absPath = gameDir.absoluteFilePath(d.mLocalPath);
				QFile local = absPath;
				if (local.exists())
				{
					if (!withUpdate
						|| d.mHash.isEmpty()
						|| Crypto::sha1(local) == d.mHash // check file's sha1
						)
					{
						continue;
					}
				}

				totalDownloadSize += d.mSize;
				downloads << D{ absPath, QUrl(d.mUrl) };
			}
			if (!downloads.isEmpty())
			{
				setStatus(tr("Downloading..."));

				UIThread::run([&, totalDownloadSize]()
				{
					ui.total->setText(StringHelper::prettySize(totalDownloadSize));
					ui.downloaded->setText(StringHelper::prettySize(0));
					ui.eta->setText(tr("Calculating..."));
					ui.speed->setText(StringHelper::prettySize(0, true));
					ui.progressBar->setMaximum(totalDownloadSize);
					ui.progressBar->setValue(0);
				});
				QNetworkAccessManager network;
				QEventLoop zaloop;

				quint64 downloaded = 0;
				unsigned downloadTasks = 0;

				auto dummy = new QObject(&network);

				for (auto& d : downloads)
				{
					auto reply = network.get(QNetworkRequest(d.second));
					auto file = new QFile(d.first, &network);

					connect(reply, &QNetworkReply::readyRead, dummy, [&, reply, file]()
					{
						if (!file->isOpen())
						{
							auto absDir = QFileInfo(*file).absoluteDir();
							if (!absDir.exists())
							{
								absDir.mkpath(absDir.absolutePath());
							}
							if (!file->open(QIODevice::WriteOnly))
							{
								// try to delete existing file.
								file->remove();

								if (!file->open(QIODevice::WriteOnly))
								{
									qWarning("Could not open file: %s", file->fileName().toStdString().c_str());
									reply->close();
									return;
								}
							}
						}
						auto buf = reply->readAll();
						downloaded += file->write(buf);
					});
					connect(reply, &QNetworkReply::finished, dummy, [&, reply, file]()
					{
						if (file->isOpen())
						{
							file->close();
						}
						downloadTasks -= 1;

						if (downloadTasks == 0)
						{
							zaloop.exit();
						}
					});
					downloadTasks += 1;
				}

				QTimer t;
				unsigned counter = 0;
				quint64 lastPeriodDownloaded = 0;
				float averangeDelta = 0.f;
				connect(&t, &QTimer::timeout, dummy, [&]()
				{
					unsigned delta = downloaded - lastPeriodDownloaded;
					if (++counter % 10 == 0)
					{
						averangeDelta += (delta - averangeDelta) * 0.2f;
						if (counter >= 50)
						{
							// it's time to show up the speed
							float time = (totalDownloadSize - downloaded) / averangeDelta;
							UIThread::run([&, time]()
							{
								auto k = QDateTime::fromMSecsSinceEpoch(time * 1000).toUTC().toString("HH:mm:ss");
								ui.eta->setText(k);
							});
						}

						lastPeriodDownloaded = downloaded;
					}

					// copy this value to prevent data racing
					quint64 d = downloaded;
					UIThread::run([&, d]()
					{
						ui.downloaded->setText(StringHelper::prettySize(d));
						ui.speed->setText(StringHelper::prettySize(averangeDelta, true));
						ui.progressBar->setValue(d);;
					});
				});
				t.setInterval(100);
				t.start();
				zaloop.exec();
			}
		}));
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
	ui.status->setVisible(m);
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
