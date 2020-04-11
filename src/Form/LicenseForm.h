#pragma once

#include <QWidget>
#include "ui_LicenseForm.h"

class LicenseForm : public QWidget
{
	Q_OBJECT

public:
	LicenseForm(QWidget *parent = Q_NULLPTR);
	~LicenseForm();

private:
	Ui::LicenseForm ui;
};
