#pragma once

#include <Form/Form.h>
#include "ui_LicenseForm.h"

class LicenseForm : public Form
{
	Q_OBJECT

public:
	LicenseForm(const QString& text, QWidget *parent = Q_NULLPTR);
	virtual ~LicenseForm();
	
private:
	Ui::LicenseForm ui;
};
