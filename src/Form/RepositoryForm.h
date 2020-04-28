#pragma once

#include "Form.h"
#include "ui_RepositoryForm.h"
#include "Model/Repository.h"
#include <QNetworkAccessManager>

class RepositoryForm : public Form
{
	Q_OBJECT

signals:
	void result(const Repository& r);
public:
	RepositoryForm(const Repository& r, QWidget *parent = Q_NULLPTR);
	~RepositoryForm();
	

private:
	Ui::RepositoryForm ui;
	QNetworkAccessManager mNet;

	void setTestingState(bool testing);
};
