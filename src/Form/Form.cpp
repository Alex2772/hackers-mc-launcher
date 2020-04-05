#include "Form.h"

Form::Form(QWidget* parent):
	QDialog(parent, Qt::CustomizeWindowHint
		| Qt::WindowTitleHint
		| Qt::WindowCloseButtonHint
		| Qt::MSWindowsFixedSizeDialogHint)
{
	setModal(true);
}
