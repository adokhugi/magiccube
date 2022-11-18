#ifndef _NEHE_WINDOW_H_
#define _NEHE_WINDOW_H_

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "NeHeGL.h"

class GL_Window
{
public:
	Keys				*keys;
	HWND				hWnd;
	HDC					hDC;
	HGLRC				hRC;
	GL_WindowInit		init;
	BOOL				isVisible;
	DWORD				lastTickCount;
};

void TerminateApplication (GL_Window *window);
void ToggleFullscreen (GL_Window *window);
void ReshapeGL (int width, int height);
BOOL ChangeScreenResolution (int width, int height, int bitsPerPixel);
BOOL CreateWindowGL (GL_Window *window);
BOOL DestroyWindowGL (GL_Window *window);
BOOL RegisterWindowClass (Application *application, void *WindowProc);

#define WM_TOGGLEFULLSCREEN (WM_USER + 1)

extern bool g_isProgramLooping;
extern bool g_createFullScreen;

#endif