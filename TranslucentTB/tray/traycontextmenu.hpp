#pragma once
#include "contextmenu.hpp"
#include "../dynamicloader.hpp"
#include "trayicon.hpp"

class TrayContextMenu : public TrayIcon, public ContextMenu {
protected:
	inline TrayContextMenu(const GUID &iconId, Util::null_terminated_wstring_view className,
		Util::null_terminated_wstring_view windowName, const wchar_t *whiteIconResource, const wchar_t *darkIconResource,
		const wchar_t *menuResource, HINSTANCE hInstance, DynamicLoader &loader) :
		TrayIcon(iconId, className, windowName, whiteIconResource, darkIconResource, hInstance, loader.ShouldSystemUseDarkMode()),
		ContextMenu(menuResource, hInstance)
	{
		if (const auto admfm = loader.AllowDarkModeForWindow())
		{
			admfm(m_WindowHandle, true);
		}
	}

	inline LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_INITMENU:
			RefreshMenu();
			return 0;

		default:
			if (uMsg == TRAY_CALLBACK)
			{
				const UINT message = LOWORD(lParam);
				if (message == WM_CONTEXTMENU || message == WM_LBUTTONDOWN)
				{
					if (!SetForegroundWindow(m_WindowHandle))
					{
						MessagePrint(spdlog::level::info, L"Failed to set window as foreground window.");
					}

					ShowAtCursor(m_WindowHandle);
					post_message(WM_NULL);
					ReturnFocus();
				}

				return 0;
			}
			else
			{
				return TrayIcon::MessageHandler(uMsg, wParam, lParam);
			}
		}
	}
};
