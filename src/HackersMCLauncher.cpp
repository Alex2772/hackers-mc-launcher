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
#include <QProcess>
#include "Util/VariableHelper.h"
#include <QJsonDocument>
#include <QJsonObject>
#include "unzip.h"


HackersMCLauncher::HackersMCLauncher(QWidget* parent)
	: QMainWindow(parent),
	  mSettings(new Settings("hackers-mc-launcher", "hackers-mc-launcher", this))
{
	ui.setupUi(this);

	setWindowIcon(QIcon(":/hck/logo_256.png"));
	
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

	loadProfiles();

	connect(&mUsers, &QAbstractItemModel::dataChanged, this, &HackersMCLauncher::saveProfiles);
	connect(&mProfiles, &QAbstractItemModel::dataChanged, this, &HackersMCLauncher::saveProfiles);
}

void HackersMCLauncher::closeEvent(QCloseEvent* event)
{
	saveProfiles();
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

bool HackersMCLauncher::currentProfile(Profile& p)
{

	int r = ui.profilesList->selectionModel()->currentIndex().row();
	if (r < 0 || r >= mProfiles.profiles().size())
	{
		return false;
	}
	p = mProfiles.profiles().at(r);
	return true;
}

bool HackersMCLauncher::currentUser(User& p)
{

	int r = ui.usersList->selectionModel()->currentIndex().row();
	if (r < 0 || r >= mUsers.users().size())
	{
		return false;
	}
	p = mUsers.users().at(r);
	return true;
}

void HackersMCLauncher::resetDownloadIndicators()
{
	ui.progressBar->setMaximum(0);
	ui.downloaded->setText("-");
	ui.total->setText("-");
	ui.speed->setText("-");
	ui.eta->setText("-");
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
		resetDownloadIndicators();
		setDownloadMode(true);

		Profile profile;
		User user;

		if (!currentProfile(profile) || !currentUser(user))
			return;

		QThreadPool::globalInstance()->start(lambda([&, profile, user, withUpdate]()
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

			typedef QPair<QString, QUrl> D; // just for convenience
			for (int i = 0; i < 2; ++i) {
				QList<D> downloads;
				quint64 totalDownloadSize = 0;
				auto assetJsonPath = gameDir.absoluteFilePath("assets/indexes/" + profile.mAssetsIndex + ".json");
				bool assetIndexExists = QFile(assetJsonPath).exists();

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
				if (assetIndexExists)
				{
					QFile f(assetJsonPath);
					f.open(QIODevice::ReadOnly);
					auto objects = QJsonDocument::fromJson(f.readAll()).object()["objects"].toObject();
					f.close();
					
					QDir objectsDir = gameDir.absoluteFilePath("assets/objects");

					for (QJsonValue object : objects)
					{
						auto hash = object["hash"].toString();
						QString path = QString(hash.at(0)) + hash.at(1) + '/' + hash;
						QFile local = objectsDir.absoluteFilePath(path);

						if (local.exists())
						{
							if (!withUpdate
								|| Crypto::sha1(local) == hash // check file's sha1
								)
							{
								continue;
							}
						}

						totalDownloadSize += object["size"].toInt();
						downloads << D{ objectsDir.absoluteFilePath(path), QUrl("http://resources.download.minecraft.net/" + path) };
					}
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
						ui.progressBar->setMaximum(1000);
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
								file->deleteLater();
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

						if (QThread::currentThread()->isInterruptionRequested())
						{
							zaloop.exit();
							return;
						}
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
							ui.progressBar->setValue(d * 1000 / totalDownloadSize);
						});
					});
					t.setInterval(100);
					t.start();
					interruptionCheck();
					zaloop.exec();
				}
				if (assetIndexExists)
					break;
			}
			UIThread::run([&]()
			{
				resetDownloadIndicators();
			});
			setStatus(tr("Preparing to launch..."));

			// unpack necessary libraries
			for (auto& lib : profile.mDownloads)
			{
				if (lib.mExtract)
				{
					std::string absoluteFilePath = gameDir.absoluteFilePath(lib.mLocalPath).toStdString();
					QDir extractTo = gameDir.absoluteFilePath("bin/" + profile.id());
					if (!extractTo.exists())
						extractTo.mkpath(extractTo.absolutePath());

					unzFile unz = unzOpen(absoluteFilePath.c_str());
					if (unz)
					{
						unz_global_info info;
						if (unzGetGlobalInfo(unz, &info) != UNZ_OK)
							continue;
						for (size_t i = 0; i < info.number_entry; ++i)
						{
							unz_file_info fileInfo;
							char cFileName[512];
							if (unzGetCurrentFileInfo(unz, &fileInfo, cFileName, sizeof(cFileName), nullptr, 0, nullptr, 0) != UNZ_OK)
								break;

							QString fileName = cFileName;
							
							// meta-inf folder is not needed
							if (!fileName.startsWith("META-INF/"))
							{
								if (fileName.endsWith('/'))
								{
									// folder
									extractTo.mkpath(extractTo.absoluteFilePath(fileName));
								}
								else {
									// file
									if (unzOpenCurrentFile(unz) != UNZ_OK)
										break;


									QFile dstFile = extractTo.absoluteFilePath(fileName);
									auto containingDir = QFileInfo(dstFile).absoluteDir();
									if (!containingDir.exists())
										extractTo.mkpath(containingDir.absolutePath());

									dstFile.open(QIODevice::WriteOnly);

									char buf[0x1000];

									for (int read; (read = unzReadCurrentFile(unz, buf, sizeof(buf))) > 0;)
									{
										dstFile.write(buf, read);
									}

									dstFile.close();
									unzCloseCurrentFile(unz);
								}
							}
							
							
							if ((i + 1) < info.number_entry)
							{
								if (unzGoToNextFile(unz) != UNZ_OK)
									break;
							}
						}
						
						unzClose(unz);
					}					
				}
			}
			UIThread::run([&]()
			{
				setDownloadMode(false);
			});

			// JVM args
			QStringList args;
			for (auto& a : profile.mJavaArgs)
			{
				if (VariableHelper::checkConditions(this, a.mConditions))
					continue;
				args << VariableHelper::replaceVariablesInString(this, a.mName);
			}

			// main class
			args << profile.mMainClass;

			// game args
			for (auto& a : profile.mGameArgs)
			{
				if (VariableHelper::checkConditions(this, a.mConditions))
					continue;
				args << VariableHelper::replaceVariablesInString(this, a.mName);
				if (!a.mValue.isEmpty())
					args << VariableHelper::replaceVariablesInString(this, a.mValue);
			}

			UIThread::run([&, args]()
			{
				auto p = new QProcess(this);
				connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [p](int status, QProcess::ExitStatus e)
				{
					p->deleteLater();
				});
				connect(p, &QProcess::readyReadStandardOutput, this, [p]()
				{
					qInfo() << p->readAllStandardOutput();
					qInfo() << p->readAllStandardError();
				});
				p->setWorkingDirectory(getSettings()->getGameDir().absolutePath());
				p->setProgram("java");
				p->setArguments(args);
				p->startDetached();
			});

		}));
	}
}

void HackersMCLauncher::loadProfiles()
{
	QFile c = getSettings()->getGameDir().absoluteFilePath("launcher_profiles.json");
	if (c.exists())
	{
		c.open(QIODevice::ReadOnly);
		QJsonObject config = QJsonDocument::fromJson(c.readAll()).object();
		c.close();

		for (auto& key : config["authenticationDatabase"].toObject().keys())
		{
			auto user = config["authenticationDatabase"].toObject()[key].toObject();
			
			mUsers.add(User{ user["username"].toString() });
		}
		for (auto& key : config["profiles"].toObject().keys())
		{
			auto tryLoad = [&](Profile& dst, const QString& name) -> bool
			{
				for (auto extension : { ".hck.json", ".json" }) {
					QFile file = mSettings->getGameDir()
					.absoluteFilePath("versions/" + name + '/' + name + extension);
					if (file.exists()) {
						file.open(QIODevice::ReadOnly);
						dst = Profile::fromJson(this, name, 
							QJsonDocument::fromJson(file.readAll()).object());
						file.close();
						return true;
					}
				}
				return false;
			};
			auto profile = config["profiles"].toObject()[key].toObject();
			auto name = profile["name"].toString();
			Profile dst;
			if (name.isEmpty() || !tryLoad(dst, name))
			{
				name = profile["lastVersionId"].toString();
				if (!tryLoad(dst, name))
					continue;
			}
			
			mProfiles.add(std::move(dst));
		}

		auto selectedUser = config["selectedUser"].toObject();
		if (selectedUser["account"].isString())
		{
			unsigned counter = 0;
			for (auto& u : mUsers.users())
			{
				if (u.id() == selectedUser["account"].toString())
				{
					ui.usersList->selectionModel()->setCurrentIndex(mUsers.index(counter, 0, {}), QItemSelectionModel::Select);
					break;
				}
				++counter;
			}
		}
		if (selectedUser["profile"].isString())
		{
			unsigned counter = 0;
			for (auto& u : mProfiles.profiles())
			{
				if (u.id() == selectedUser["profile"].toString())
				{
					ui.profilesList->selectionModel()->setCurrentIndex(mProfiles.index(counter, 0, {}), QItemSelectionModel::Select);
					break;
				}
				++counter;
			}
		}
	}
}

/**
 * \brief Saves users and profiles lists to launcher_profiles.json
 *
 *		  Partically compatible with legacy Minecraft launcher format, which
 *		  is also compatible with mods installers (Forge, Optifine etc...)
 */
void HackersMCLauncher::saveProfiles()
{
	/*
	 * {
  "authenticationDatabase" : {
    "e681e8fcde71f860219102687b158192" : {
      "username" : "alex2772sc@gmail.com"
    }
  },
  "clientToken" : "",
  "launcherVersion" : {
    "format" : 21,
    "name" : "",
    "profilesFormat" : 2
  },
  "profiles" : {
    "20bad5e4ac74fef693ba34e5e840a2d2" : {
      "created" : "2020-04-06T01:55:00.204Z",
      "icon" : "Furnace",
      "lastUsed" : "2020-04-06T01:55:04.528Z",
      "lastVersionId" : "rd-132211",
      "name" : "",
      "type" : "custom"
    },
    ....
  },
  "selectedUser" : {
    "account" : "e681e8fcde71f860219102687b158192",
    "profile" : ""
  },
  "settings" : {
	...
  }
}
	 */

	QJsonObject out;

	// users
	QJsonObject authDatabase;
	for (auto& user : mUsers.users())
	{
		QJsonObject u;
		u["username"] = user.mUsername;

		authDatabase[user.id()] = u;
	}
	out["authenticationDatabase"] = authDatabase;

	// unsupported.
	out["clientToken"] = "";

	// version
	QJsonObject launcherVersion;
	launcherVersion["format"] = 21;
	launcherVersion["name"] = "";
	launcherVersion["profilesFormat"] = 2;
	out["launcherVersion"] = launcherVersion;

	// profiles
	QJsonObject profiles;
	for (auto& profile : mProfiles.profiles())
	{
		QJsonObject p;

		p["name"] = profile.mName;
		p["lastVersionId"] = profile.mName;
		
		profiles[profile.id()] = p;
	}
	out["profiles"] = profiles;

	// selected user
	QJsonObject selectedUser;
	User u;
	if (currentUser(u))
		selectedUser["account"] = u.id();

	Profile p;
	if (currentProfile(p))
		selectedUser["profile"] = p.id();
	out["selectedUser"] = selectedUser;
	
	// unused.
	out["settings"] = QJsonObject();
	
	QFile f = mSettings->getGameDir().absoluteFilePath("launcher_profiles.json");
	f.open(QIODevice::WriteOnly);
	f.write(QJsonDocument(out).toJson());
	f.close();
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
