#include "JavaLibForm.h"
#include "Model/JavaLibModel.h"
#include <QUrl>
#include "Util/StringHelper.h"

JavaLibForm::JavaLibForm(JavaLibModel* model, QWidget *parent)
	: Form(parent)
{
	ui.setupUi(this);

	connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &JavaLibForm::close);
	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, [&, model]()
	{
		auto url = ui.repo->text();
		StringHelper::normalizeUrl(url);
		model->add(QUrl(url), {ui.group->text(), ui.name->text(), 
			ui.version->text(), ui.sha1->text()});
		close();
	});
}

JavaLibForm::~JavaLibForm()
{
}
