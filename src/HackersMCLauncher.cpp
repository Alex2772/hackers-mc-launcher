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
#include "LicenseForm.h"
#include <QJsonArray>
#include "Util/DownloadHelper.h"
#include "Window/ConsoleWindow.h"
#include "launcher_config.h"
#include <QDesktopServices>


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
	updatePlayButton();

	connect(&mUsers, &QAbstractItemModel::dataChanged, this, &HackersMCLauncher::saveProfiles);
	connect(&mProfiles, &QAbstractItemModel::dataChanged, this, &HackersMCLauncher::saveProfiles);

	// Check for updates
	if (mSettings->value("check_launcher_updates").toBool())
	{
		checkForUpdates();
	}
}

void HackersMCLauncher::closeEvent(QCloseEvent* event)
{
	emit closeConsoleWindows();
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


void HackersMCLauncher::installPackage(const QUrl& url)
{
	auto reply = mNetwork.get(QNetworkRequest(url));
	setDownloadMode(true);
	ui.status->setText(tr("Downloading package manifest..."));

	connect(reply, &QNetworkReply::finished, this, [&, reply]()
	{
		reply->deleteLater();
		auto buffer = reply->readAll();
		auto hash = Crypto::sha1(buffer);
		QJsonObject manifest = QJsonDocument::fromJson(buffer).object();

		auto performDownload = [&, manifest, hash]() {
			QThreadPool::globalInstance()->start(lambda([&, manifest, hash]()
			{
				QDir installDir = getSettings()->getGameDir().absoluteFilePath("packages/" + hash);
				if (!installDir.exists())
				{
					installDir.mkpath(installDir.absolutePath());
				}
				// compose the download list
				QList<Download> downloads;
				for (auto& key : manifest["files"].toObject().keys())
				{
					auto file = manifest["files"].toObject()[key].toObject();
					// necessary directories will be created automatically
					if (file["type"].toString() == "file") {
						auto raw = file["downloads"].toObject()["raw"].toObject();
						downloads << Download{ installDir.absoluteFilePath(key), raw["url"].toString(),
							quint64(raw["size"].toInt()), false, raw["sha1"].toString() };
					}
				}
				DownloadHelper d(this);
				d.addDownloadList(downloads, true);
				d.performDownload();
				UIThread::run([&]()
				{
					ui.play->setEnabled(true); // play will work only if ui.play is enabled
					play(false);
				});
			}));
		};

		
		// check for the license file
		if (manifest["files"].toObject()["LICENSE"].isObject())
		{
			auto url =
				manifest["files"].toObject()["LICENSE"].toObject()["downloads"].toObject()["raw"].toObject()["url"].toString();
			auto licenseReply = mNetwork.get(QNetworkRequest(url));
			connect(licenseReply, &QNetworkReply::finished, this, [&, performDownload, licenseReply]()
			{
				licenseReply->deleteLater();

				LicenseForm f(QString::fromUtf8(licenseReply->readAll()), this);
				if (f.exec())
				{
					performDownload();
				} else
				{
					setDownloadMode(false);
				}
			});
		} else
		{
			performDownload();
		}
	});
}

void HackersMCLauncher::downloadJava()
{
	// cracked from the legacy minecraft launcher
	auto reply = mNetwork.get(QNetworkRequest(QUrl(
		"http://launchermeta.mojang.com/v1/products/launcher/d03cf0cf95cce259fa9ea3ab54b65bd28bb0ae82/windows-x86.json")));

	setDownloadMode(true);
	ui.status->setText(tr("Locating package manifest..."));
	
	connect(reply, &QNetworkReply::finished, this, [&, reply]()
	{
		reply->deleteLater();
		QJsonObject json = QJsonDocument::fromJson(reply->readAll()).object();
		QJsonObject manifest = json[QSysInfo::currentCpuArchitecture().contains(QLatin1String("64"))
			                            ? "jre-x64"
			                            : "jre-x86"].toArray()[0].toObject()["manifest"].toObject();

		QString url = manifest["url"].toString();
		if (url.isEmpty())
		{
			QMessageBox::critical(this, tr("Failed to install Java"), tr("Could not locate package manifest."));
			setDownloadMode(false);
		} else
		{
			installPackage(QUrl(url));
		}
	});
}


void HackersMCLauncher::updatePlayButton()
{
	if (mProcesses.isEmpty())
	{
		ui.play->setText(tr("Play"));
	} else
	{
		ui.play->setText(tr("New instance"));
	}
}

QProcess* HackersMCLauncher::createProcess()
{
	if (mProcesses.empty())
	{
		emit closeConsoleWindows();
	}
	auto p = new QProcess(this);
	mProcesses << p;
	if (getSettings()->value("hide_launcher").toBool())
	{
		hide();
	}
	else {
		updatePlayButton();
	}
	if (getSettings()->value("show_console").toBool())
	{
		auto console = new ConsoleWindow(p);
		connect(this, &HackersMCLauncher::closeConsoleWindows, console, &ConsoleWindow::onClose);
	}
	return p;
}

void HackersMCLauncher::removeProcess(QProcess* p, int status, QProcess::ExitStatus e)
{
	mProcesses.removeAll(p);
	p->deleteLater();
	if (isHidden())
	{
		show();
	}
	else {
		updatePlayButton();
	}
	if (mProcesses.isEmpty())
	{
		if (getSettings()->value("close_launcher").toBool())
		{
			close();
		}
	}
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
			DownloadHelper helper(this);
			helper.setStatusUI(tr("Searching files..."));
			helper.gameDownload(profile, user, withUpdate);
			helper.setStatusUI(tr("Preparing to launch..."));
			
			// unpack necessary libraries
			for (auto& lib : profile.mDownloads)
			{
				if (lib.mExtract)
				{
					std::string absoluteFilePath = mSettings->getGameDir().absoluteFilePath(lib.mLocalPath).toStdString();
					QDir extractTo = mSettings->getGameDir().absoluteFilePath("bin/" + profile.id());
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
							if (unzGetCurrentFileInfo(unz, &fileInfo, cFileName, sizeof(cFileName), nullptr, 0, nullptr,
							                          0) != UNZ_OK)
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
								else
								{
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
				auto p = createProcess();
				connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
				        [&, p](int status, QProcess::ExitStatus e)
				        {
							removeProcess(p, status, e);
				        });
				connect(p, &QProcess::readyReadStandardOutput, this, [p]()
				{
					printf("%s", p->readAllStandardOutput().data());
					printf("%s", p->readAllStandardError().data());
				});
				connect(p, &QProcess::errorOccurred, this, [&, p](QProcess::ProcessError e)
				{
					if (e == 0)
						switch (QMessageBox::question(this, tr("Could not start Java"), tr(
							"It seems like you have no proper Java installation."
							" Minecraft requires Java to launch.\n\nLauncher can download and install Java for you."),
							tr("Download and install Java for me"), tr("I'll install by myself")))
						{
						case 0: // install
							downloadJava();
							break;
						case 1: // install by user
							QMessageBox::information(this, tr("Java installation"),
								tr("Don't forget to restart the launcher."), tr("Got it"));
							break;
						}
					removeProcess(p, 0, QProcess::ExitStatus::NormalExit);
				});
				p->setWorkingDirectory(getSettings()->getGameDir().absolutePath());

				// set default java command.
				p->setProgram("java");

				// try to find java in packages dir
				QDir packageDir = getSettings()->getGameDir().absoluteFilePath("packages");
				for (auto& e : packageDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
				{
					auto executable = packageDir.absoluteFilePath(e + "/bin/java.exe");
					if (QFileInfo(executable).isFile())
					{
						p->setProgram(executable);
						break;
					}
				}
				p->setArguments(args);
				
				qInfo() << p->program();
				qInfo() << p->arguments().join(' ');
				
				p->start();
			});
		}));
	}
}

bool HackersMCLauncher::tryLoadProfile(Profile& dst, const QString& name)
{
	for (auto extension : { ".hck.json", ".json" })
	{
		QFile file = mSettings->getGameDir()
			.absoluteFilePath("versions/" + name + '/' + name + extension);
		if (file.exists())
		{
			file.open(QIODevice::ReadOnly);
			dst = Profile::fromJson(this, name,
				QJsonDocument::fromJson(file.readAll()).object());
			file.close();
			return true;
		}
	}
	return false;
}

void HackersMCLauncher::checkForUpdates(bool ignoreErrors)
{
	auto reply = mNetwork.get(QNetworkRequest(QUrl("https://api.github.com/repos/alex2772/hackers-mc-launcher/releases/latest")));
	connect(reply, &QNetworkReply::finished, this, [&, reply, ignoreErrors]()
	{
		emit updateCheckFinished();
		auto replyBuffer = reply->readAll();
		if (!replyBuffer.isEmpty())
		{
			QJsonObject o = QJsonDocument::fromJson(replyBuffer).object();
			if (o.contains("tag_name"))
			{
				// Construct release info information.
				QString releaseInfo;
				if (o["assets"].isArray() && !o["assets"].toArray().isEmpty())
				{
					auto release = o["assets"].toArray().first().toObject();

					releaseInfo += "<b>" + tr("Release date") + ":</b> " + QDateTime::fromString(release["updated_at"].toString(), Qt::ISODate).toString() + "<br />";
					releaseInfo += "<b>" + tr("Size") + ":</b> " + StringHelper::prettySize(release["size"].toInt()) + "<br />";
					releaseInfo += "<b>" + tr("Downloads") + ":</b> " + QString::number(release["download_count"].toInt()) + "<br />";
					releaseInfo += "<br /><i>" + tr("Changelog") + ":</i>" + StringHelper::markdownToHtml(o["body"].toString());
				}
				
				if (o["tag_name"].toString() != LAUNCHER_VERSION)
				{
					QMessageBox b(this);
					b.setWindowTitle(tr("Update available"));
					b.setTextFormat(Qt::TextFormat::RichText);
					b.setText("<h2>" + tr("New version available") + ": " + o["tag_name"].toString() + "</h2>" + releaseInfo);
					b.setIcon(QMessageBox::Information);
					b.addButton(tr("Download"), QMessageBox::AcceptRole);
					b.addButton(tr("Remind me later"), QMessageBox::AcceptRole);

					if (mSettings->value("check_launcher_updates").toBool())
						b.addButton(tr("Disable update checking"), QMessageBox::AcceptRole);

					switch (b.exec())
					{
					case 0:
						QDesktopServices::openUrl(o["html_url"].toString());
					case 1:
						break;
					case 2:
						mSettings->setValue("check_launcher_updates", false);
						break;
					}
				} else if (!ignoreErrors)
				{
					QMessageBox b(this);
					b.setWindowTitle(tr("No updates available"));
					b.setTextFormat(Qt::TextFormat::RichText);
					b.setText("<h4>" + tr("You are using the latest version") + ": " + o["tag_name"].toString() + "</h4>" + releaseInfo);
					b.exec();
				}
			}
			else if (!ignoreErrors)
			{
				QMessageBox::information(this, tr("Could not check for updates"), tr("Invalid response."));
			}
		}
		else if (!ignoreErrors)
		{
			QMessageBox::information(this, tr("Could not check for updates"), tr("Service is unavailable."));
		}
	});
};
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

			mUsers.add(User{user["username"].toString()});
		}
		for (auto& key : config["profiles"].toObject().keys())
		{
			auto profile = config["profiles"].toObject()[key].toObject();
			auto name = profile["name"].toString();
			Profile dst;
			if (name.isEmpty() || !tryLoadProfile(dst, name))
			{
				name = profile["lastVersionId"].toString();
				if (!tryLoadProfile(dst, name))
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
					ui.usersList->selectionModel()->setCurrentIndex(mUsers.index(counter, 0, {}),
					                                                QItemSelectionModel::Select);
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
					ui.profilesList->selectionModel()->setCurrentIndex(mProfiles.index(counter, 0, {}),
					                                                   QItemSelectionModel::Select);
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
