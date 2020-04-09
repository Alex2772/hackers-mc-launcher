#pragma once

#include <QVariant>

class HackersMCLauncher;

namespace  VariableHelper
{
	QVariant getVariableValue(HackersMCLauncher* launcher, const QString& name);
	bool checkConditions(HackersMCLauncher* launcher, const QList<QPair<QString, QVariant>>& conditions);
	QString replaceVariablesInString(HackersMCLauncher* launcher, const QString& string);
};
