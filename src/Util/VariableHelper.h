#pragma once

#include <QVariant>

class HackersMCLauncher;

namespace  VariableHelper
{
	QVariant getVariableValue(HackersMCLauncher* launcher, const QString& name);
};
