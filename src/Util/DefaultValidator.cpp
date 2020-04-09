#include "DefaultValidator.h"

DefaultValidator::DefaultValidator(QObject* parent):
	QRegExpValidator(QRegExp("[a-zA-Z0-9\\._-]+"), parent)
{
}
