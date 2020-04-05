#pragma once
#include <QDialog>

class Form: public QDialog
{
	Q_OBJECT
public:
	explicit Form(QWidget* parent);
	~Form() override = default;

protected:
	void closeEvent(QCloseEvent*) override;
};
