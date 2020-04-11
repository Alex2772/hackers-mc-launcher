#include "LicenseForm.h"
#include <QEventLoop>

LicenseForm::LicenseForm(const QString& text, QWidget *parent)
	: Form(parent)
{
	ui.setupUi(this);
	ui.plainTextEdit->setPlainText(text);

	connect(ui.buttonBox, &QDialogButtonBox::rejected, this, [&]()
	{
		done(0);
	});

	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, [&]()
	{
		done(1);
	});
}

LicenseForm::~LicenseForm()
{
}