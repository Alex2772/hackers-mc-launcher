#pragma once
#include <QString>
#include "JavaLibModel.h"

class Profile
{
public:
	QString mName;

	QMap<QString, QList<JavaLib>> mJavaLibs;
};
