#include "NeHe_Window.h"

void TerminateApplication (GL_Window *window)
{
	PostMessage (window->hWnd, WM_QUIT, 0, 0);
	g_isProgramLooping = FALSE;
}

void ToggleFullscreen (GL_Window *window)
{
	PostMessage (window->hWnd, WM_TOGGLEFULLSCREEN, 0, 0);
}

void ReshapeGL (int width, int height)
{
	glViewport (0, 0, (GLsizei)(width), (GLsizei)(height));
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (45.0f, (GLfloat)(width) / (GLfloat)(height), 0.1f, 100.0f);		
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
}

BOOL ChangeScreenResolution (int width, int height, int bitsPerPixel)
{
	DEVMODE dmScreenSettings;
	ZeroMemory (&dmScreenSettings, sizeof (DEVMODE));
	dmScreenSettings.dmSize				= sizeof (DEVMODE);
	dmScreenSettings.dmPelsWidth		= width;
	dmScreenSettings.dmPelsHeight		= height;
	dmScreenSettings.dmBitsPerPel		= bitsPerPixel;
	dmScreenSettings.dmFields			= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	if (ChangeDisplaySettings (&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CreateWindowGL (GL_Window *window)
{
	__int64	timer;
	DWORD windowStyle = WS_OVERLAPPEDWINDOW;
	DWORD windowExtendedStyle = WS_EX_APPWINDOW;

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof (PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		window->init.bitsPerPixel,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		16,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	RECT windowRect = {0, 0, window->init.width, window->init.height};

	GLuint PixelFormat;

	if (window->init.isFullScreen == TRUE)
	{
		if (ChangeScreenResolution (window->init.width, window->init.height, window->init.bitsPerPixel) == FALSE)
		{
			MessageBox (HWND_DESKTOP, "Mode Switch Failed.\nRunning In Windowed Mode.", "Error", MB_OK | MB_ICONEXCLAMATION);
			window->init.isFullScreen = FALSE;
		}
		else
		{
			ShowCursor (FALSE);
			windowStyle = WS_POPUP;
			windowExtendedStyle |= WS_EX_TOPMOST;
		}
	}
	else
		AdjustWindowRectEx (&windowRect, windowStyle, 0, windowExtendedStyle);

	window->hWnd = CreateWindowEx (windowExtendedStyle,
								   window->init.application->className,
								   window->init.title,
								   windowStyle,
								   0, 0,
								   windowRect.right - windowRect.left,
								   windowRect.bottom - windowRect.top,
								   HWND_DESKTOP,
								   0,
								   window->init.application->hInstance,
								   window);

	if (window->hWnd == 0)
	{
		return FALSE;
	}

	window->hDC = GetDC (window->hWnd);
	if (window->hDC == 0)
	{
		DestroyWindow (window->hWnd);
		window->hWnd = 0;
		return FALSE;
	}

	PixelFormat = ChoosePixelFormat (window->hDC, &pfd);
	if (PixelFormat == 0)
	{
		ReleaseDC (window->hWnd, window->hDC);
		window->hDC = 0;
		DestroyWindow (window->hWnd);
		window->hWnd = 0;
		return FALSE;
	}

	if (SetPixelFormat (window->hDC, PixelFormat, &pfd) == FALSE)
	{
		ReleaseDC (window->hWnd, window->hDC);
		window->hDC = 0;
		DestroyWindow (window->hWnd);
		window->hWnd = 0;
		return FALSE;
	}

	window->hRC = wglCreateContext (window->hDC);
	if (window->hRC == 0)
	{
		ReleaseDC (window->hWnd, window->hDC);
		window->hDC = 0;
		DestroyWindow (window->hWnd);
		window->hWnd = 0;
		return FALSE;
	}

	if (wglMakeCurrent (window->hDC, window->hRC) == FALSE)
	{
		wglDeleteContext (window->hRC);
		window->hRC = 0;
		ReleaseDC (window->hWnd, window->hDC);
		window->hDC = 0;
		DestroyWindow (window->hWnd);
		window->hWnd = 0;
		return FALSE;
	}

	ShowWindow (window->hWnd, SW_NORMAL);
	window->isVisible = TRUE;

	ReshapeGL (window->init.width, window->init.height);

	ZeroMemory (window->keys, sizeof (Keys));

	window->lastTickCount = GetTickCount ();

	return TRUE;
}

BOOL DestroyWindowGL (GL_Window* window)
{
	if (window->hWnd != 0)
	{	
		if (window->hDC != 0)
		{
			wglMakeCurrent (window->hDC, 0);
			if (window->hRC != 0)
			{
				wglDeleteContext (window->hRC);
				window->hRC = 0;
			}
			ReleaseDC (window->hWnd, window->hDC);
			window->hDC = 0;
		}
		DestroyWindow (window->hWnd);
		window->hWnd = 0;
	}

	if (window->init.isFullScreen)
	{
		ChangeDisplaySettings (NULL, 0);
		ShowCursor (TRUE);
	}	
	return TRUE;
}

BOOL RegisterWindowClass (Application* application, void *WindowProc)
{
	WNDCLASSEX windowClass;
	ZeroMemory (&windowClass, sizeof (WNDCLASSEX));
	windowClass.cbSize			= sizeof (WNDCLASSEX);
	windowClass.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc		= (WNDPROC)(WindowProc);
	windowClass.hInstance		= application->hInstance;
	windowClass.hbrBackground	= (HBRUSH)COLOR_WINDOW+1;
	windowClass.hCursor			= LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName	= application->className;
	if (RegisterClassEx (&windowClass) == 0)
	{
		MessageBox (HWND_DESKTOP, "RegisterClassEx Failed!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}
	return TRUE;
}
