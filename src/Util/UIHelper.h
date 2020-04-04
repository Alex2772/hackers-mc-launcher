#pragma once
#include <QComboBox>
#include <QLayout>

namespace UIHelper
{
	void customWindowShowEvent(QWidget* window);
	bool customFrameNativeEvent(QWidget* window, const QByteArray& eventType, void* message, long* result);
	void redraw(QWidget* widget);

	/**
	 * It's obsolete utter garbage 
	 * Use UIHelper::redraw instead
	 */
	[[deprecated]] void redrawRecursive(QWidget* widget);
	QWidget* separator(Qt::Orientation orientation);
};
