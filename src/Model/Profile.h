#pragma once
#include <QString>
#include "JavaLibListModel.h"

class Profile
{
public:
	QString mName;

	QList<JavaLib> mJavaLibs;
};
