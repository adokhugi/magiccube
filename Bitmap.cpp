#include "Bitmap.h"

bool Bitmap::New (long lWidth, long lHeight)
{
	hdcTemp = CreateCompatibleDC (GetDC (0));
	if (!hdcTemp)
	{
		pPicture->Release ();
		return false;
	}

	actualWidthPixels = lWidth;
	actualHeightPixels = lHeight;
	lWidthPixels = lWidth;
	lHeightPixels = lHeight;

	ResizeImageToClosestPowerOfTwo ();

	BITMAPINFO bi = { 0 };

	bi.bmiHeader.biSize			= sizeof (BITMAPINFOHEADER);
	bi.bmiHeader.biBitCount		= 32;
	bi.bmiHeader.biWidth		= lWidthPixels;
	bi.bmiHeader.biHeight		= lHeightPixels;
	bi.bmiHeader.biCompression	= BI_RGB;
	bi.bmiHeader.biPlanes		= 1;

	hbmpTemp = CreateDIBSection (hdcTemp, &bi, DIB_RGB_COLORS, (void**)&pBits, 0, 0);
	
	if (!hbmpTemp)
	{
		DeleteDC (hdcTemp);
		pPicture->Release ();
		return false;
	}

	transparencyMask = new bool [lWidthPixels * lHeightPixels];

	for (long i = 0; i < lWidthPixels * lHeightPixels; i++)
	{
		BYTE *pPixel	= (BYTE*)(&pBits [i]);
		pPixel [3]		= 0;
		transparencyMask [i]	= true;
	}

	loaded = false;

	return true;
}

bool Bitmap::LoadFileFromMemory (int res, char *name, bool transparency, GLuint *transparentCol)
{
	HRSRC rec;
	HGLOBAL	handle;
	unsigned char *data;
	int length;
	FILE *tempFile;
	
	rec = FindResource (NULL, MAKEINTRESOURCE (res), name);
	handle = LoadResource (NULL, rec);
	
	data = (unsigned char *)LockResource (handle);
	length = SizeofResource (NULL, rec);

	tempFile = fopen ("temp", "wb");

	for (int i = 0; i < length; i++)
		fputc (data [i], tempFile);

	fclose (tempFile);

	return LoadFile ("temp", transparency, transparentCol);
}

bool Bitmap::LoadFile (char *szPathName, bool transparency, GLuint *transparentCol)
{
	OLECHAR		wszPath [MAX_PATH + 1];
	char		szPath [MAX_PATH + 1];
	long		lWidth;
	long		lHeight;
	DWORD		*pBits_tmp;

	if (strstr (szPathName, "http://"))
		strcpy (szPath, szPathName);
	else
	{
		GetCurrentDirectory (MAX_PATH, szPath);
		strcat (szPath, "\\");
		strcat (szPath, szPathName);
	}

	MultiByteToWideChar (CP_ACP, 0, szPath, -1, wszPath, MAX_PATH);
	HRESULT hr = OleLoadPicturePath (wszPath, 0, 0, 0, IID_IPicture, (void**)&pPicture);

	if (FAILED (hr))
		return false;

	hdcTemp = CreateCompatibleDC (GetDC (0));
	if (!hdcTemp)
	{
		pPicture->Release ();
		return false;
	}
	
	pPicture->get_Width (&lWidth);
	lWidthPixels	= MulDiv (lWidth, GetDeviceCaps (hdcTemp, LOGPIXELSX), 2540);
	actualWidthPixels = lWidthPixels;
	pPicture->get_Height (&lHeight);
	lHeightPixels	= MulDiv (lHeight, GetDeviceCaps (hdcTemp, LOGPIXELSY), 2540);
	actualHeightPixels = lHeightPixels;

	ResizeImageToClosestPowerOfTwo ();

	BITMAPINFO bi = { 0 };

	bi.bmiHeader.biSize			= sizeof (BITMAPINFOHEADER);
	bi.bmiHeader.biBitCount		= 32;
	bi.bmiHeader.biWidth		= actualWidthPixels;
	bi.bmiHeader.biHeight		= actualHeightPixels;
	bi.bmiHeader.biCompression	= BI_RGB;
	bi.bmiHeader.biPlanes		= 1;

	hbmpTemp = CreateDIBSection (hdcTemp, &bi, DIB_RGB_COLORS, (void**)&pBits_tmp, 0, 0);
	
	if (!hbmpTemp)
	{
		DeleteDC (hdcTemp);
		pPicture->Release ();
		return false;
	}

	SelectObject (hdcTemp, hbmpTemp);

	pPicture->Render (hdcTemp, 0, 0, actualWidthPixels, actualHeightPixels, 0, lHeight, lWidth, -lHeight, 0);

	bi.bmiHeader.biSize			= sizeof (BITMAPINFOHEADER);
	bi.bmiHeader.biBitCount		= 32;
	bi.bmiHeader.biWidth		= lWidthPixels;
	bi.bmiHeader.biHeight		= lHeightPixels;
	bi.bmiHeader.biCompression	= BI_RGB;
	bi.bmiHeader.biPlanes		= 1;

	hbmpTemp = CreateDIBSection (hdcTemp, &bi, DIB_RGB_COLORS, (void**)&pBits, 0, 0);
	
	if (!hbmpTemp)
	{
		DeleteDC (hdcTemp);
		pPicture->Release ();
		return false;
	}

	transparencyMask = new bool [lWidthPixels * lHeightPixels];

	long i;

	for (i = 0; i < lWidthPixels * lHeightPixels; i++)
	{
		BYTE *pPixel			= (BYTE*)(&pBits [i]);
		pPixel [3]				= 0;
		transparencyMask [i]	= true;
	}

	i = 0;
	long i_tmp = 0;
	for (long y = 0; y < actualHeightPixels; y++)
	{
		for (long x = 0; x < actualWidthPixels; x++)
		{
			BYTE *pPixel		= (BYTE*)(&pBits [i]);
			BYTE *pPixel_tmp	= (BYTE*)(&pBits_tmp [i_tmp]);

			pPixel [0]			= pPixel_tmp [2];
			pPixel [1]			= pPixel_tmp [1];
			pPixel [2]			= pPixel_tmp [0];

			if ((transparency && pPixel [0] == transparentCol [0] && pPixel [1] == transparentCol [1] && pPixel [2] == transparentCol [2]))
			{
				pPixel [3]				=   0;
				transparencyMask [i]	= true;
			}
			else
			{
				pPixel [3]				= 255;
				transparencyMask [i]	= false;
			}

			i++;
			i_tmp++;
		}
		i += lWidthPixels - actualWidthPixels;
	}

	loaded = true;

	return true;
}

bool Bitmap::LoadFile (char *szPathName)
{
	return LoadFile (szPathName, false, NULL);
}

void Bitmap::RenderText (TextObject *textObject, AlignmentTypes align)
{
	HBITMAP hbmpText;

	SelectObject (hdcTemp, hFont);

	BITMAPINFO	biText = {0};
	DWORD		*pBitsText = 0;

	biText.bmiHeader.biSize			= sizeof (BITMAPINFOHEADER);
	biText.bmiHeader.biBitCount		= 32;
	biText.bmiHeader.biWidth		= lWidthPixels;
	biText.bmiHeader.biHeight		= lHeightPixels;
	biText.bmiHeader.biCompression	= BI_RGB;
	biText.bmiHeader.biPlanes		= 1;

	hbmpText = CreateDIBSection (hdcTemp, &biText, DIB_RGB_COLORS, (void**)&pBitsText, 0, 0);

	SelectObject (hdcTemp, hbmpText);

	RECT rect;
	rect.top = 0;
	rect.left = 0;
	rect.bottom = lHeightPixels - 1;
	rect.right = lWidthPixels - 1;
	textObject->DrawFormatted (hdcTemp, &rect, align);

	for (long i = 0; i < lWidthPixels * lHeightPixels; i++)
	{
		BYTE *pPixelText = (BYTE*)(&pBitsText [i]);
		BYTE *pPixel	= (BYTE*)(&pBits [i]);

		if (!(pPixelText [0] == 255 && pPixelText [1] == 255 && pPixelText[2] == 255)
			&& !(pPixelText [0] == 0 && pPixelText [1] == 0 && pPixelText[2] == 0))
		{
			pPixel [0]				= pPixelText [0];
			pPixel [1]				= pPixelText [1];
			pPixel [2]				= pPixelText [2];
			pPixel [3]				= 255;
			transparencyMask [i]	= false;
		}
	}

	DeleteObject (hbmpText);
}

void Bitmap::GenerateTexture (GLuint &texid)
{
	glGenTextures (1, &texid);
	glBindTexture (GL_TEXTURE_2D, texid);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	UpdateTexture (texid);
}

void Bitmap::UpdateTexture (GLuint &texid)
{
	glBindTexture (GL_TEXTURE_2D, texid);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, lWidthPixels, lHeightPixels, 0, GL_RGBA, GL_UNSIGNED_BYTE, pBits);
}

void Bitmap::Delete ()
{
	DeleteObject (hbmpTemp);
	DeleteDC (hdcTemp);

	if (loaded)
		pPicture->Release();
}

HDC Bitmap::GetHdcTemp ()
{
	return hdcTemp;
}

void Bitmap::ChangeOpacity (int newOpacity)
{
	for (long i = 0; i < lWidthPixels * lHeightPixels; i++)
	{
		BYTE *pPixel = (BYTE*)(&pBits [i]);

		if (!transparencyMask [i])
			pPixel [3] = newOpacity;
	}
}

void Bitmap::PutPixel (int x, int y, int *color)
{
	BYTE *pPixel = (BYTE *)(&pBits [y * lWidthPixels + x]);
	for (int i = 0; i < 4; i++)
		pPixel [i] = color [i];
	transparencyMask [y * lWidthPixels + x] = false;
}

void Bitmap::SetPixel (int x, int y, int *color)
{
	PutPixel (x, y, color);
}

int *Bitmap::GetPixel (int x, int y)
{
	BYTE *pPixel = (BYTE *)(&pBits [y * lWidthPixels + x]);
	int *color = new int [4];
	for (int i = 0; i < 4; i++)
		color [i] = pPixel [i];
	return color;
}

void Bitmap::SetRenderTextColor (COLORREF color)
{
	SetTextColor (hdcTemp, color);
}

void Bitmap::SetRenderTextFont (char *face, int height, bool italic, bool underline)
{
	hFont = CreateFontA (height, 0, 0, 0, 0, italic, underline, 0, 'w', 
		    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
			DEFAULT_PITCH | FF_DONTCARE, face);
}

void Bitmap::SetRenderTextFont (char *face, int height)
{
	SetRenderTextFont (face, height, false, false);
}

DWORD *Bitmap::GetPBits ()
{
	return pBits;
}

long Bitmap::GetLHeightPixels ()
{
	return lHeightPixels;
}

long Bitmap::GetLWidthPixels ()
{
	return lWidthPixels;
}

long Bitmap::GetActualHeightPixels ()
{
	return actualHeightPixels;
}

long Bitmap::GetActualWidthPixels ()
{
	return actualWidthPixels;
}

void Bitmap::Copy (Bitmap source)
{
	int x0 = (actualWidthPixels - source.GetActualWidthPixels ()) >> 1;
	int y0 = lHeightPixels - actualHeightPixels + ((actualHeightPixels - source.GetActualHeightPixels ()) >> 1);
	DWORD *pBits_src = source.GetPBits () + (source.GetLHeightPixels () - source.GetActualHeightPixels ()) * source.GetLWidthPixels ();
	DWORD *pBits_dst = pBits + y0 * lWidthPixels + x0;
	bool *transparencyMask_dst = transparencyMask + y0 * lWidthPixels + x0;

	for (int y = 0; y < source.GetActualHeightPixels (); y++)
	{
		for (int x = 0; x < source.GetActualWidthPixels (); x++)
		{
			*pBits_dst = *pBits_src;
			*transparencyMask_dst = false;
			pBits_src++;
			pBits_dst++;
			transparencyMask_dst++;
		}
		pBits_src += source.GetLWidthPixels () - source.GetActualWidthPixels ();
		pBits_dst += lWidthPixels - source.GetActualWidthPixels ();
		transparencyMask_dst += lWidthPixels - source.GetActualWidthPixels ();
	}
}

void Bitmap::Clear (int *color)
{
	if (color == NULL)
		for (int i = 0; i < lWidthPixels * lHeightPixels; i++)
		{
			pBits [i] = 0;
			transparencyMask [i] = true;
		}
	else
		for (int i = 0; i < lWidthPixels * lHeightPixels; i++)
		{
			pBits [i] = (((((color [3] << 8) + color [2]) << 8) + color [1]) << 8) + color [0];
			transparencyMask [i] = true;
		}
}

void Bitmap::SplitVertical (int splitY, Bitmap *bitmap1, Bitmap *bitmap2)
{
	DWORD *pBits_src = pBits;
	DWORD *pBits_dst;
	int y;
	bool *transparencyMask_dst;
	int lWidthPixels_dst;

	bitmap1->New (actualWidthPixels, splitY);
	bitmap2->New (actualWidthPixels, actualHeightPixels - splitY);
	bitmap1->Clear ();
	bitmap2->Clear ();

	pBits_dst = bitmap1->GetPBits ();
	transparencyMask_dst = bitmap1->GetTransparencyMask ();
	lWidthPixels_dst = bitmap1->GetLWidthPixels ();

	for (y = 0; y < actualHeightPixels; y++)
	{
		if (y == splitY)
		{
			pBits_dst = bitmap2->GetPBits ();
			transparencyMask_dst = bitmap2->GetTransparencyMask ();
			lWidthPixels_dst = bitmap2->GetLWidthPixels ();
		}

		for (int x = 0; x < actualWidthPixels; x++)
		{
			*pBits_dst = *pBits_src;
			*transparencyMask_dst = false;
			pBits_src++;
			pBits_dst++;
			transparencyMask_dst++;
		}
		pBits_src += lWidthPixels - actualWidthPixels;
		pBits_dst += lWidthPixels_dst - actualWidthPixels;
		transparencyMask_dst += lWidthPixels_dst - actualWidthPixels;
	}
}

void Bitmap::ResizeImageToClosestPowerOfTwo ()
{
	GLint glMaxTexDim;

	glGetIntegerv (GL_MAX_TEXTURE_SIZE, &glMaxTexDim);

	if (lWidthPixels <= glMaxTexDim)
		lWidthPixels = ClosestPowerOfTwo (lWidthPixels);
	else
		lWidthPixels = glMaxTexDim;
 
	if (lHeightPixels <= glMaxTexDim)
		lHeightPixels = ClosestPowerOfTwo (lHeightPixels);
	else
		lHeightPixels = glMaxTexDim;
}

long Bitmap::ClosestPowerOfTwo (long v)
{
	bool breakcond = false;

	for (unsigned long i = 1 << ((sizeof (long) << 3) - 1); i && !breakcond; i >>= 1)
	{
		if (v & i)
		{
			if (v != i)
				v = i << 1;
			breakcond = true;
		}
	}

	return v;
}

bool *Bitmap::GetTransparencyMask ()
{
	return transparencyMask;
}

