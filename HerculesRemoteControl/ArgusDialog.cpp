// ArgusDialog.cpp

#include <iostream>
#include "ArgusDialog.h"
#include "AppUtility.h"

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"
#include "wx/dynlib.h"
#include <wx/stdpaths.h>
#include <wx/msw/icon.h>
#include <wx/msw/rcdefs.h>
#include <wx/mousestate.h>

#include <process.h> // for _beginthreadex()

using namespace std;

namespace
{

	// Critical section that guards everything related to wxWidgets "main" thread
	// startup or shutdown.
	wxCriticalSection gs_wxStartupCS;
	// Handle of wx "main" thread if running, NULL otherwise
	HANDLE gs_wxMainThread = NULL;


	//  wx application startup code -- runs from its own thread
	unsigned wxSTDCALL MyAppLauncher(void* event)
	{
		// Note: The thread that called run_wx_gui_from_dll() holds gs_wxStartupCS
		//       at this point and won't release it until we signal it.

		// We need to pass correct HINSTANCE to wxEntry() and the right value is
		// HINSTANCE of this DLL, not of the main .exe, use this MSW-specific wx
		// function to get it. Notice that under Windows XP and later the name is
		// not needed/used as we retrieve the DLL handle from an address inside it
		// but you do need to use the correct name for this code to work with older
		// systems as well.
		const HINSTANCE
			hInstance = wxDynamicLibrary::MSWGetModuleHandle("ArgusDialog",
			&gs_wxMainThread);
		if (!hInstance)
			return 0; // failed to get DLL's handle

		// IMPLEMENT_WXWIN_MAIN does this as the first thing
		wxDISABLE_DEBUG_SUPPORT();

		// We do this before wxEntry() explicitly, even though wxEntry() would
		// do it too, so that we know when wx is initialized and can signal
		// run_wx_gui_from_dll() about it *before* starting the event loop.
		wxInitializer wxinit;
		if (!wxinit.IsOk())
			return 0; // failed to init wx

		// Signal run_wx_gui_from_dll() that it can continue
		HANDLE hEvent = *(static_cast<HANDLE*>(event));
		if (!SetEvent(hEvent))
			return 0; // failed setting up the mutex

		// Run the app:
		wxEntry(hInstance);

		return 1;
	}

} // anonymous namespace

namespace Argus
{
	ARGUSDIALOG_API void ArgusDialog::Create()
	{
		char *title = "";
		m_isOpen = true;

		// In order to prevent conflicts with hosting app's event loop, we
		// launch wx app from the DLL in its own thread.
		//
		// We can't even use wxInitializer: it initializes wxModules and one of
		// the modules it handles is wxThread's private module that remembers
		// ID of the main thread. But we need to fool wxWidgets into thinking that
		// the thread we are about to create now is the main thread, not the one
		// from which this function is called.
		//
		// Note that we cannot use wxThread here, because the wx library wasn't
		// initialized yet. wxCriticalSection is safe to use, though.

		wxCriticalSectionLocker lock(gs_wxStartupCS);

		if (!gs_wxMainThread)
		{
			HANDLE hEvent = CreateEvent
				(
				NULL,  // default security attributes
				FALSE, // auto-reset
				FALSE, // initially non-signaled
				NULL   // anonymous
				);
			if (!hEvent)
				return; // error

			// NB: If your compiler doesn't have _beginthreadex(), use CreateThread()
			gs_wxMainThread = (HANDLE)_beginthreadex
				(
				NULL,           // default security
				0,              // default stack size
				&MyAppLauncher,
				&hEvent,        // arguments
				0,              // create running
				NULL
				);

			if (!gs_wxMainThread)
			{
				CloseHandle(hEvent);
				return; // error
			}

			// Wait until MyAppLauncher signals us that wx was initialized. This
			// is because we use wxMessageQueue<> and wxString later and so must
			// be sure that they are in working state.
			WaitForSingleObject(hEvent, INFINITE);
			CloseHandle(hEvent);
		}

		// Send a message to wx thread to show a new frame:
		wxThreadEvent *event =
			new wxThreadEvent(wxEVT_THREAD, CMD_SHOW_WINDOW);
		event->SetString(title);
		event->SetPayload<Argus::ArgusDialog *>(this);
		wxQueueEvent(wxApp::GetInstance(), event);
	}

	ARGUSDIALOG_API void ArgusDialog::Delete()
	{
		wxCriticalSectionLocker lock(gs_wxStartupCS);

		if (!gs_wxMainThread)
			return;

		// If wx main thread is running, we need to stop it. To accomplish this,
		// send a message telling it to terminate the app.
		wxThreadEvent *event =
			new wxThreadEvent(wxEVT_THREAD, CMD_TERMINATE);
		wxQueueEvent(wxApp::GetInstance(), event);
		
		// We must then wait for the thread to actually terminate.
		WaitForSingleObject(gs_wxMainThread, INFINITE);
		CloseHandle(gs_wxMainThread);
		gs_wxMainThread = NULL;
	}

	// проверяет открыт ли диалог
	ARGUSDIALOG_API bool ArgusDialog::IsOpen()
	{
		return m_isOpen;
	}

	// установка изображений
	ARGUSDIALOG_API void ArgusDialog::SetImages(unsigned char* image0, unsigned char* image1, unsigned char* image2, unsigned char* image3, unsigned char* image4, int width, int height)
	{
		AppUtility *app = dynamic_cast<AppUtility*>(wxApp::GetInstance());
		AppUtilityFrame *frame = app->frame();

		if (frame == 0)
		{
			return;
		}

		if (frame->isClosing())
		{
			return;
		}

		ArgusImages images;
		images.image0 = image0;
		images.image1 = image1;
		images.image2 = image2;
		images.image3 = image3;
		images.image4 = image4;
		images.width = width;
		images.height = height;

		// Send a message to wx thread to show a new frame:
		wxThreadEvent *event =
			new wxThreadEvent(wxEVT_THREAD, CMD_SET_IMAGES);
		event->SetPayload<ArgusImages>(images);
		wxQueueEvent(wxApp::GetInstance(), event);

		/*if (frame != NULL)
		{
			AppGLCanvas *glCanvas = frame->glCanvas();
			glCanvas->setImages(image0, image1, image2, image3, image4, width, height);
		}*/		
	}

	// установка кнопок
	ARGUSDIALOG_API void ArgusDialog::SetButtons(std::vector<ArgusButton>& buttons0, std::vector<ArgusButton>& buttons1, std::vector<ArgusButton>& buttons2, std::vector<ArgusButton>& buttons3, std::vector<ArgusButton>& buttons4)
	{
		AppUtility *app = dynamic_cast<AppUtility*>(wxApp::GetInstance());
		AppUtilityFrame *frame = app->frame();

		ArgusButtons buttons;
		buttons.buttons0 = buttons0;
		buttons.buttons1 = buttons1;
		buttons.buttons2 = buttons2;
		buttons.buttons3 = buttons3;
		buttons.buttons4 = buttons4;

		// Send a message to wx thread to show a new frame:
		wxThreadEvent *event =
			new wxThreadEvent(wxEVT_THREAD, CMD_SET_BUTTONS);
		event->SetPayload<ArgusButtons>(buttons);
		wxQueueEvent(wxApp::GetInstance(), event);
	}

	// получение значений кнопок
	ARGUSDIALOG_API void ArgusDialog::GetButtons(std::vector<ArgusButton>& buttons0, std::vector<ArgusButton>& buttons1, std::vector<ArgusButton>& buttons2, std::vector<ArgusButton>& buttons3, std::vector<ArgusButton>& buttons4)
	{
		AppUtility *app = dynamic_cast<AppUtility*>(wxApp::GetInstance());
		AppUtilityFrame *frame = app->frame();

		if (frame != 0 && !frame->isClosing())
		{
			frame->GetButtons(buttons0, buttons1, buttons2, buttons3, buttons4);
		}		
	}

	// установка ползунка
	ARGUSDIALOG_API void ArgusDialog::SetSlider(ArgusSlider& slider)
	{
		// Send a message to wx thread to show a new frame:
		wxThreadEvent *event =
			new wxThreadEvent(wxEVT_THREAD, CMD_SET_SLIDER);
		event->SetPayload<ArgusSlider>(slider);
		wxQueueEvent(wxApp::GetInstance(), event);
	}

	// получение значений ползунка
	ARGUSDIALOG_API void ArgusDialog::GetSlider(ArgusSlider& slider)
	{
		AppUtility *app = dynamic_cast<AppUtility*>(wxApp::GetInstance());
		AppUtilityFrame *frame = app->frame();

		if (frame != 0 && !frame->isClosing())
		{
			frame->GetSlider(slider);
		}
	}

	// получение текущих координат курсора
	ARGUSDIALOG_API void ArgusDialog::GetMouseState(ArgusMouseState& mouseState)
	{
		wxPoint pnt = wxGetMousePosition();
		mouseState.active.x = pnt.x;
		mouseState.active.y = pnt.y;

		AppUtility *app = dynamic_cast<AppUtility*>(wxApp::GetInstance());
		AppUtilityFrame *frame = app->frame();

		if (frame != 0)
		{
			frame->glCanvas()->mouseState(mouseState);
		}
	}

	// установка номеров отображаемых подобластей
	ARGUSDIALOG_API void ArgusDialog::SetSubviews(std::vector<int> indices)
	{
		AppUtility *app = dynamic_cast<AppUtility*>(wxApp::GetInstance());
		AppUtilityFrame *frame = app->frame();

		// Send a message to wx thread to show a new frame:
		wxThreadEvent *event =
			new wxThreadEvent(wxEVT_THREAD, CMD_SET_SUBVIEWS);
		event->SetPayload< std::vector<int> >(indices);
		wxQueueEvent(wxApp::GetInstance(), event);
	}
}
