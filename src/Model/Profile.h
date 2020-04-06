#pragma once
#include <QUrl>
#include "JavaLibModel.h"

class Profile
{
public:
	QString mName;

	QMap<QUrl, QList<JavaLib>> mJavaLibs;
};
