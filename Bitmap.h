#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <stdio.h>
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <olectl.h>
#include <math.h>
#include "TextObject.h"
#include "AlignmentTypes.h"

class Bitmap
{
public:
	bool New (long lWidth, long lHeight);
	bool LoadFileFromMemory (int res, char *name, bool transparency, GLuint *transparentCol);
	bool LoadFile (char *szPathName, bool transparency, GLuint *transparentCol);
	bool LoadFile (char *szPathName);
	void RenderText (TextObject *textObject, AlignmentTypes align = Align_Left);
	void GenerateTexture (GLuint &texid);
	void UpdateTexture (GLuint &texid);
	void Delete ();
	HDC GetHdcTemp ();
	void ChangeOpacity (int newOpacity);
	void PutPixel (int x, int y, int *color);
	void SetPixel (int x, int y, int *color);
	int *GetPixel (int x, int y);
	void SetRenderTextColor (COLORREF color);
	void SetRenderTextFont (char *face, int height, bool italic, bool underline);
	void SetRenderTextFont (char *face, int height);
	DWORD *GetPBits ();
	void Copy (Bitmap source);
	long GetLHeightPixels ();
	long GetLWidthPixels ();
	long GetActualHeightPixels ();
	long GetActualWidthPixels ();
	void Clear (int *color = NULL);
	void SplitVertical (int splitY, Bitmap *bitmap1, Bitmap *bitmap2);
	void ResizeImageToClosestPowerOfTwo ();
	long ClosestPowerOfTwo (long v);
	bool *GetTransparencyMask ();

private:
	HDC hdcTemp;
	HBITMAP hbmpTemp;
	long lWidthPixels;
	long lHeightPixels;
	long actualWidthPixels;
	long actualHeightPixels;
	DWORD *pBits;
	bool *transparencyMask;
	IPicture *pPicture;
	bool loaded;
	HFONT hFont;
};

#endif