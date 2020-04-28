#pragma once

#include "Form.h"
#include "ui_RepositoryForm.h"

class RepositoryForm : public Form
{
	Q_OBJECT

public:
	RepositoryForm(QWidget *parent = Q_NULLPTR);
	~RepositoryForm();

private:
	Ui::RepositoryForm ui;
};
