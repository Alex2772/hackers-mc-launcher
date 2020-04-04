#include "UIHelper.h"
#ifdef Q_OS_WIN
#include <Windows.h>
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")

#endif
#ifdef Q_OS_WIN

#define GET_X_LPARAM(lp)    ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)    ((int)(short)HIWORD(lp))
#endif
void UIHelper::customWindowShowEvent(QWidget* window)
{
#ifdef Q_OS_WIN
	LONG_PTR d = GetWindowLongPtr((HWND)window->winId(), GWL_STYLE);
	SetWindowLongPtr((HWND)window->winId(), GWL_STYLE, d | WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX);
	const MARGINS shadow = { 1, 1, 1, 1 };
	DwmExtendFrameIntoClientArea((HWND)window->winId(), &shadow);
#endif
}

bool UIHelper::customFrameNativeEvent(QWidget* window, const QByteArray& eventType, void* message, long* result)
{
#ifdef Q_OS_WIN
	MSG* msg = (MSG*)message;
	switch (msg->message)
	{
	case WM_NCCALCSIZE:
	{
		//this kills the window frame and title bar we added with
		//WS_THICKFRAME and WS_CAPTION
		*result = 0;
		return true;
		break;
	}
	case WM_NCHITTEST:
	{
		*result = 0;
		const LONG border_width = 4; //in pixels
		RECT winrect;
		GetWindowRect((HWND)window->winId(), &winrect);

		long x = GET_X_LPARAM(msg->lParam);
		long y = GET_Y_LPARAM(msg->lParam);

		bool resizeWidth = window->minimumWidth() != window->maximumWidth();
		bool resizeHeight = window->minimumHeight() != window->maximumHeight();

		if (resizeWidth)
		{
			//left border
			if (x >= winrect.left && x < winrect.left + border_width)
			{
				*result = HTLEFT;
			}
			//right border
			if (x < winrect.right && x >= winrect.right - border_width)
			{
				*result = HTRIGHT;
			}
		}
		if (resizeHeight)
		{
			//bottom border
			if (y < winrect.bottom && y >= winrect.bottom - border_width)
			{
				*result = HTBOTTOM;
			}
			//top border
			if (y >= winrect.top && y < winrect.top + border_width)
			{
				*result = HTTOP;
			}
		}
		if (resizeWidth && resizeHeight)
		{
			//bottom left corner
			if (x >= winrect.left && x < winrect.left + border_width &&
				y < winrect.bottom && y >= winrect.bottom - border_width)
			{
				*result = HTBOTTOMLEFT;
			}
			//bottom right corner
			if (x < winrect.right && x >= winrect.right - 22 &&
				y < winrect.bottom && y >= winrect.bottom - 12)
			{
				*result = HTBOTTOMRIGHT;
			}
			//top left corner
			if (x >= winrect.left && x < winrect.left + border_width &&
				y >= winrect.top && y < winrect.top + border_width)
			{
				*result = HTTOPLEFT;
			}
			//top right corner
			if (x < winrect.right && x >= winrect.right - border_width &&
				y >= winrect.top && y < winrect.top + border_width)
			{
				*result = HTTOPRIGHT;
			}
		}

		//TODO: allow move?
		if (!*result && y - winrect.top <= 30) {
			QWidget* w = window->childAt(x - winrect.left, y - winrect.top);
			if (w != nullptr) {
				if (!(w->property("cap_override").toBool() || w->property("cap").toBool())) {
					*result = HTCAPTION;
				}
			}
		}
		if (*result)
			return true;
		break;
	} //end case WM_NCHITTEST
	}
#endif
	return false;
}

void UIHelper::redrawRecursive(QWidget* w)
{
	w->style()->unpolish(w);
	for (auto& c : w->findChildren<QWidget*>())
	{
		c->style()->unpolish(c);
	}
	w->style()->polish(w);
	for (auto& c : w->findChildren<QWidget*>())
	{
		c->style()->polish(c);
	}
}
void UIHelper::redraw(QWidget* widget)
{
	widget->style()->unpolish(widget);
	widget->style()->polish(widget);
}

QWidget* UIHelper::separator(Qt::Orientation orientation)
{
	auto f = new QFrame;
	f->setFrameShape(QFrame::NoFrame);
	f->setFrameShadow(QFrame::Plain);
	f->setStyleSheet("background: rgba(255, 255, 255, 100)");
	switch (orientation)
	{
	case Qt::Horizontal:
		f->setFixedSize(18, 1);
		break;
	case Qt::Vertical:
		f->setFixedSize(1, 18);
		break;
	}
	
	return f;
}
