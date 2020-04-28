#include "RepositoryForm.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include "Util/CommonUtils.h"
#include <QNetworkReply>

RepositoryForm::RepositoryForm(const Repository& r, QWidget *parent)
	: Form(parent)
{
	ui.setupUi(this);

	setTestingState(false);
	ui.url->setText(r.mUrl);
	
	connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &RepositoryForm::close);
	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, [&]()
	{
		setTestingState(true);
		auto url = ui.url->text();
		
		auto reply = mNet.get(QNetworkRequest(QUrl(url)));
		connect(reply, &QNetworkReply::finished, this, [&, reply, url]()
		{
			QJsonObject o = QJsonDocument::fromJson(reply->readAll()).object();

			if (o["versions"].isArray())
			{
				// Result ok
				o["url"] = url;
				auto name = CommonUtils::determineName(o);
				Repository r{ name, url };
				emit result(r);
				close();
			} else
			{
				QMessageBox::critical(this, tr("The repository is not found"), tr("The URL you typed cannot be opened or URL does not point at repository description file."));
				setTestingState(false);
			}
		});
	});
}

RepositoryForm::~RepositoryForm()
{
}

void RepositoryForm::setTestingState(bool testing)
{
	ui.progressBar->setVisible(testing);
	ui.buttonBox->setEnabled(!testing);
}
