#ifndef GL_FRAMEWORK__INCLUDED
#define GL_FRAMEWORK__INCLUDED

#ifndef CDS_FULLSCREEN
#define CDS_FULLSCREEN 4
#endif

#include <windows.h>

class Keys
{
public:
	BOOL keyDown [256];
};

class Application
{
public:
	HINSTANCE			hInstance;
	const char			*className;
};

class GL_WindowInit
{
public:
	Application			*application;
	char				*title;
	int					width;
	int					height;
	int					bitsPerPixel;
	BOOL				isFullScreen;
};

#endif
