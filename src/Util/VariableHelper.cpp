#include "VariableHelper.h"

QVariant VariableHelper::getVariableValue(HackersMCLauncher* launcher, const QString& name)
{
	static QMap<QString, std::function<QVariant()>> vars = {
		{
			"os.name",
			[]() -> QVariant
			{
				return QSysInfo::productType();
			}
		}
	};

	return vars[name]();
}
