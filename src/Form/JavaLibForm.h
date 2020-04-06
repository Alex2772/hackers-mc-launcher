#pragma once

#include <QWidget>
#include "ui_JavaLibForm.h"
#include <ui_VersionChooserForm.h>

class JavaLibModel;

class JavaLibForm : public Form
{
	Q_OBJECT

public:
	JavaLibForm(JavaLibModel* model, QWidget *parent = Q_NULLPTR);
	~JavaLibForm();

private:
	Ui::JavaLibForm ui;
};
