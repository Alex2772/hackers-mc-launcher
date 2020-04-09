#pragma once
#include <qvalidator.h>

class DefaultValidator: public QRegExpValidator
{
public:
	DefaultValidator(QObject* parent);
	virtual ~DefaultValidator() = default;
};
