#include "arch.h"
#include <errhandlingapi.h>
#include <synchapi.h>
#include <wil/resource.h>
#include "winrt.hpp"

#include "application.hpp"
#include "constants.hpp"
#include "mainappwindow.hpp"
#include "../ProgramLog/error/winrt.hpp"
#include "../ProgramLog/log.hpp"
#include "uwp/uwp.hpp"

_Use_decl_annotations_ int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, wchar_t *, int)
{
	const bool hasPackageIdentity = UWP::HasPackageIdentity();
	Log::Initialize(hasPackageIdentity);

	win32::HardenProcess();

	wil::unique_mutex mutex(MUTEX_GUID.c_str());
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		// If there already is another instance running, tell it to exit
		MainAppWindow::CloseRemote();
	}

	try
	{
		winrt::init_apartment(winrt::apartment_type::single_threaded);
	}
	HresultErrorCatch(spdlog::level::critical, L"Initialization of Windows Runtime failed.");

	// Run the main program loop. When this method exits, TranslucentTB itself is about to exit.
	return Application(hInstance, hasPackageIdentity).Run();

	// let the apartment get cleaned up by the system so that our static
	// COM pointers don't crash when they try to call Release().
}

#pragma endregion
