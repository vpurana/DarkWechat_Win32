#include "TrashHook.h"

#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")
#include <MinHook.h>
#include <gdiplustypes.h>
#include <gdiplus.h>
#include <gdiplusstringformat.h>
#include <gdiplusbrush.h>
#include <windows.h>
#include <objidl.h>
#include <gdiplusfont.h>

using namespace Gdiplus;

#pragma comment(lib,"libMinHook.lib")
#pragma comment(lib,"Msimg32.lib")

#define FOREGOUND_COLOR RGB(255, 0, 255)
#define BACKGOUND_COLOR RGB(0, 0, 0)

#define COLORREF2RGB(Color) (Color & 0xff00) | ((Color >> 16) & 0xff) \
                                 | ((Color << 16) & 0xff0000)

//-------------------------------------------------------------------------------
// ReplaceColor
//
// Author    : Dimitri Rochette drochette@coldcat.fr
// Specials Thanks to Joe Woodbury for his comments and code corrections
//
// Includes  : Only <windows.h>

//
// hBmp         : Source Bitmap
// cOldColor : Color to replace in hBmp
// cNewColor : Color used for replacement
// hBmpDC    : DC of hBmp ( default NULL ) could be NULL if hBmp is not selected
//
// Retcode   : HBITMAP of the modified bitmap or NULL for errors
//
//-------------------------------------------------------------------------------
HBITMAP ReplaceColor(HBITMAP hBmp, COLORREF cOldColor, COLORREF cNewColor, HDC hBmpDC, Rect* lpRect)
{
	HBITMAP RetBmp = NULL;
	if (hBmp)
	{
		HDC BufferDC = CreateCompatibleDC(NULL);    // DC for Source Bitmap
		if (BufferDC)
		{
			HBITMAP hTmpBitmap = (HBITMAP)NULL;
			if (hBmpDC)
				if (hBmp == (HBITMAP)GetCurrentObject(hBmpDC, OBJ_BITMAP))
				{
					hTmpBitmap = CreateBitmap(1, 1, 1, 1, NULL);
					SelectObject(hBmpDC, hTmpBitmap);
				}

			HGDIOBJ PreviousBufferObject = SelectObject(BufferDC, hBmp);
			// here BufferDC contains the bitmap

			HDC DirectDC = CreateCompatibleDC(NULL); // DC for working
			if (DirectDC)
			{
				// Get bitmap size
				BITMAP bm;
				GetObject(hBmp, sizeof(bm), &bm);

				// create a BITMAPINFO with minimal initilisation 
				// for the CreateDIBSection
				BITMAPINFO RGB32BitsBITMAPINFO;
				ZeroMemory(&RGB32BitsBITMAPINFO, sizeof(BITMAPINFO));
				RGB32BitsBITMAPINFO.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				RGB32BitsBITMAPINFO.bmiHeader.biWidth = bm.bmWidth;
				RGB32BitsBITMAPINFO.bmiHeader.biHeight = bm.bmHeight;
				RGB32BitsBITMAPINFO.bmiHeader.biPlanes = 1;
				RGB32BitsBITMAPINFO.bmiHeader.biBitCount = 32;

				// pointer used for direct Bitmap pixels access
				UINT* ptPixels;

				HBITMAP DirectBitmap = CreateDIBSection(DirectDC,
					(BITMAPINFO*)&RGB32BitsBITMAPINFO,
					DIB_RGB_COLORS,
					(void**)&ptPixels,
					NULL, 0);
				if (DirectBitmap)
				{
					// here DirectBitmap!=NULL so ptPixels!=NULL no need to test
					HGDIOBJ PreviousObject = SelectObject(DirectDC, DirectBitmap);
					BitBlt(DirectDC, 0, 0,
						bm.bmWidth, bm.bmHeight,
						BufferDC, lpRect->GetLeft(), lpRect->GetTop(), SRCCOPY);

					// here the DirectDC contains the bitmap

				 // Convert COLORREF to RGB (Invert RED and BLUE)
					cOldColor = COLORREF2RGB(cOldColor);
					cNewColor = COLORREF2RGB(cNewColor);

					// After all the inits we can do the job : Replace Color
					for (int i = ((bm.bmWidth * bm.bmHeight) - 1); i >= 0; i--)
					{
						if (ptPixels[i]) ptPixels[i] = cNewColor;
					}
					// little clean up
					// Don't delete the result of SelectObject because it's 
					// our modified bitmap (DirectBitmap)
					SelectObject(DirectDC, PreviousObject);

					// finish
					RetBmp = DirectBitmap;
				}
				// clean up
				DeleteDC(DirectDC);
			}
			if (hTmpBitmap)
			{
				SelectObject(hBmpDC, hBmp);
				DeleteObject(hTmpBitmap);
			}
			SelectObject(BufferDC, PreviousBufferObject);
			// BufferDC is now useless
			DeleteDC(BufferDC);
		}
	}
	return RetBmp;
}

HBRUSH hFgBrush = NULL;
HBRUSH hBkBrush = NULL;
//WINGDIAPI BOOL  WINAPI RoundRect(_In_ HDC hdc, _In_ int left, _In_ int top, _In_ int right, _In_ int bottom, _In_ int width, _In_ int height);
//typedef int(__stdcall* pfnRoundRect)(HDC hdc, int left, int top, int right, int bottom, int width, int height);
//pfnRoundRect fpRoundRect = NULL;
//int __stdcall MyRoundRect(HDC hdc, int left, int top, int right, int bottom, int width, int height)
//{
//	::SelectObject(hdc, hBkBrush);
//	int rtn = fpRoundRect(hdc, left, top, right, bottom, width, height);
//	return rtn;
//}
//WINGDIAPI HBRUSH  WINAPI CreateSolidBrush( _In_ COLORREF color);
typedef HBRUSH(__stdcall* pfnCreateSolidBrush)(COLORREF color);
pfnCreateSolidBrush fpCreateSolidBrush = NULL;
HBRUSH __stdcall MyCreateSolidBrush(COLORREF color)
{
	
	HBRUSH rtn = fpCreateSolidBrush(BACKGOUND_COLOR);
	return rtn;
}
//WINGDIAPI HPEN    WINAPI CreatePen(int iStyle, int cWidth, COLORREF color);
typedef HPEN(__stdcall* pfnCreatePen)(int iStyle, int cWidth, COLORREF color);
pfnCreatePen fpCreatePen = NULL;
HPEN __stdcall MyCreatePen(int iStyle, int cWidth, COLORREF color)
{
	HPEN rtn = fpCreatePen(iStyle, cWidth, FOREGOUND_COLOR);
	return rtn;
}
//WINUSERAPI int WINAPI FillRect( _In_ HDC hDC, _In_ CONST RECT* lprc, _In_ HBRUSH hbr);

typedef int(__stdcall* pfnFillRect)(HDC hdc, RECT* lprc, HBRUSH hbr);
pfnFillRect fpFillRect = NULL;
int __stdcall MyFillRect(HDC hdc, RECT* lprc, HBRUSH hbr)
{
	::SelectObject(hdc, hBkBrush);
	int rtn = fpFillRect(hdc, lprc, hbr);
	return rtn;
}
typedef BOOL(WINAPI* pfnFillRgn)(HDC hdc, HRGN hrgn, HBRUSH hbr);
pfnFillRgn fpFillRgn = NULL;
BOOL __stdcall MyFillRgn(HDC hdc, HRGN hrgn, HBRUSH hbr)
{
	::SelectObject(hdc, hBkBrush);
	BOOL blRtn = fpFillRgn(hdc, hrgn, hbr);
	return blRtn;
}

//  WINGDIAPI HGDIOBJ WINAPI SelectObject(_In_ HDC hdc, _In_ HGDIOBJ h);
typedef HGDIOBJ(__stdcall* pfnSelectObject)(HDC hdc, HGDIOBJ h);
pfnSelectObject fpSelectObject = NULL;
HGDIOBJ __stdcall MySelectObject(HDC hdc, HGDIOBJ h)
{
	return fpSelectObject(hdc, h);
}
typedef COLORREF (__stdcall* pfnSetBkColor)(HDC hdc, COLORREF color);
pfnSetBkColor fpSetBkColor = NULL;
COLORREF __stdcall MySetBkColor(HDC hdc, COLORREF color)
{
	return fpSetBkColor(hdc, BACKGOUND_COLOR);
}
typedef COLORREF (__stdcall* pfnSetBkMode)(HDC hdc, int mode);
pfnSetBkMode fpSetBkMode = NULL;
COLORREF __stdcall MySetBkMode(HDC hdc, int mode)
{
	return fpSetBkMode(hdc, mode);
}
typedef COLORREF (__stdcall* pfnSetTextColor)(HDC hdc, COLORREF color);
pfnSetTextColor fpSetTextColor = NULL;
COLORREF __stdcall MySetTextColor(HDC hdc, COLORREF color)
{
	return fpSetTextColor(hdc, FOREGOUND_COLOR);
}
typedef BOOL(__stdcall* pfnAlphaBlend)(HDC hdcDest, int xoriginDest, int yoriginDest, int wDest, int hDest,
	HDC hdcSrc, int xoriginSrc, int yoriginSrc, int wSrc, int hSrc, BLENDFUNCTION ftn);
pfnAlphaBlend fpAlphaBlend = NULL;
BOOL __stdcall MyAlphaBlend(HDC hdcDest, int xoriginDest, int yoriginDest, int wDest, int hDest,
	HDC hdcSrc, int xoriginSrc, int yoriginSrc, int wSrc, int hSrc, BLENDFUNCTION ftn)
{
	//::SelectObject(hdcSrc, hBkBrush);
	//::SelectObject(hdcDest, hBkBrush);
	
	Rect srcRect{ xoriginSrc, yoriginSrc, xoriginSrc + wSrc, yoriginSrc + hSrc };
	HBITMAP hBmp = reinterpret_cast<HBITMAP>(GetCurrentObject(hdcSrc, OBJ_BITMAP));
	HBITMAP nBitmap = ReplaceColor(hBmp, 0, RGB(0, 0, 255), hdcSrc, &srcRect);
	//for (int ecx = xoriginSrc; ecx < wSrc + xoriginSrc; ++ecx)
	//{
	//	for (int ecy = yoriginSrc; ecy < hSrc + yoriginSrc; ++ecy)
	//	{
	//		COLORREF pixCol = GetPixel(hdcSrc, ecx, ecy);
	//		if (pixCol)
	//		{
	//			pixCol = SetPixel(hdcSrc, ecx, ecy, RGB(0, 0, 255));
	//			//
	//		}
	//	}
	//}
	SelectObject(hdcSrc, nBitmap);

	BLENDFUNCTION bf = { 0 };
	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;
	bf.AlphaFormat = 0;
	bf.SourceConstantAlpha = 192;
	BOOL blRtn = fpAlphaBlend(hdcDest, xoriginDest, yoriginDest, wDest, hDest, hdcSrc, xoriginSrc, yoriginSrc, wSrc, hSrc, bf);
	DeleteObject(nBitmap);
	//RECT targetRect = { xoriginDest, yoriginDest, xoriginDest + wDest, yoriginDest + hDest };
	//FillRect(hdcDest, &targetRect, hBkBrush);
	return blRtn;
}
typedef HPEN(__stdcall* pfnCreatePenIndirect)(CONST LOGPEN* plpen);
pfnCreatePenIndirect fpCreatePenIndirect = NULL;
HPEN __stdcall MyCreatePenIndirect(LOGPEN* plpen)
{
	plpen->lopnColor = FOREGOUND_COLOR;
	return fpCreatePenIndirect(plpen);
}
//BOOL  WINAPI BitBlt( _In_ HDC hdc, _In_ int x, _In_ int y, _In_ int cx, _In_ int cy, _In_opt_ HDC hdcSrc, _In_ int x1, _In_ int y1, _In_ DWORD rop);
typedef BOOL(__stdcall* pfnBitBlt)(HDC hdc, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1, DWORD rop);
pfnBitBlt fpBitBlt = NULL;
BOOL __stdcall MyBitBlt(HDC hdc, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1, DWORD rop)
{
	//::SelectObject(hdc, hBkBrush);
	BOOL blRtn = fpBitBlt(hdc, x, y, cx, cy, hdcSrc, x1, y1, rop);
	// RECT targetRect = { x, y, x + cx, y + cy };
	// FillRect(hdc, &targetRect, hBkBrush);
	return blRtn;
}
// WINGDIAPI BOOL  WINAPI StretchBlt(_In_ HDC hdcDest, _In_ int xDest, _In_ int yDest, _In_ int wDest, _In_ int hDest, _In_opt_ HDC hdcSrc, _In_ int xSrc, _In_ int ySrc, _In_ int wSrc, _In_ int hSrc, _In_ DWORD rop);
typedef BOOL(__stdcall* pfnStretchBlt)(HDC hdcDest, int xDest, int yDest, int wDest, int hDest, HDC hdcSrc, int xSrc, int ySrc, int wSrc, int hSrc, DWORD rop);
pfnStretchBlt fpStretchBlt = NULL;
BOOL __stdcall MyStretchBlt(HDC hdcDest, int xDest, int yDest, int wDest, int hDest, HDC hdcSrc, int xSrc, int ySrc, int wSrc, int hSrc, DWORD rop)
{
	BOOL blRtn = fpStretchBlt(hdcDest, xDest, yDest, wDest, hDest, hdcSrc, xSrc, ySrc, wSrc, hSrc, rop);
	RECT targetRect = { xDest, yDest, xDest + wDest, yDest + hDest };
	// fpFillRect(hdcDest, &targetRect, hBkBrush);
	return blRtn;
}
// WINGDIAPI BOOL  WINAPI ExtTextOutW(_In_ HDC hdc, _In_ int x, _In_ int y, _In_ UINT options, _In_opt_ CONST RECT* lprect, _In_reads_opt_(c) LPCWSTR lpString, _In_ UINT c, _In_reads_opt_(c) CONST INT* lpDx);
typedef BOOL(__stdcall* pfnExtTextOutW)(HDC hdc, int x, int y, UINT options, RECT* lprect, LPCWSTR lpString, UINT c, INT* lpDx);
pfnExtTextOutW fpExtTextOutW = NULL;
BOOL __stdcall MyExtTextOutW(HDC hdc, int x, int y, UINT options, RECT* lprect, LPCWSTR lpString, UINT c, INT* lpDx)
{
	::SelectObject(hdc, hFgBrush);
	BOOL blRtn = fpExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);
	return blRtn;
}
// WINGDIAPI BOOL WINAPI Rectangle(_In_ HDC hdc, _In_ int left, _In_ int top, _In_ int right, _In_ int bottom);
typedef BOOL(__stdcall* pfnRectangle)(HDC hdc, int left, int top, int right, int bottom);
pfnRectangle fpRectangle = NULL;
BOOL __stdcall MyRectangle(HDC hdc, int left, int top, int right, int bottom)
{
	BOOL blRtn = fpRectangle(hdc, left, top, right, bottom);
	RECT targetRect = { left, top, right, bottom };
	fpFillRect(hdc, &targetRect, hBkBrush);
	return blRtn;
}
// WINGDIAPI BOOL  WINAPI RoundRect(_In_ HDC hdc, _In_ int left, _In_ int top, _In_ int right, _In_ int bottom, _In_ int width, _In_ int height);
typedef BOOL(__stdcall* pfnRoundRect)(HDC hdc, int left, int top, int right, int bottom, int width, int height);
pfnRoundRect fpRoundRect = NULL;
BOOL __stdcall MyRoundRect(HDC hdc, int left, int top, int right, int bottom, int width, int height)
{
	::SelectObject(hdc, hBkBrush);
	BOOL blRtn = fpRoundRect(hdc, left, top, right, bottom, width, height);
	RECT targetRect = { left, top, right, bottom };
	fpFillRect(hdc, &targetRect, hBkBrush);
	return blRtn;
}
typedef Gdiplus::Status(__stdcall* pfnDrawString)(Gdiplus::Graphics* graphic, WCHAR* string, INT length, Gdiplus::Font* font, Gdiplus::RectF& layoutRect, Gdiplus::StringFormat* stringFormat, Gdiplus::Brush* brush);
pfnDrawString fpDrawString = NULL;
Gdiplus::Status __stdcall MyDrawString(Gdiplus::Graphics* graphic, WCHAR* string, INT length, Gdiplus::Font* font, Gdiplus::RectF& layoutRect, Gdiplus::StringFormat* stringFormat, Gdiplus::Brush* brush)
{
	SolidBrush sbbrush(Color(255, 0, 0, 255));
	Status blRtn = fpDrawString(graphic, string, length, font, layoutRect, stringFormat, &sbbrush);
	return blRtn;
}
typedef int(__stdcall* pfnDrawTextW)(_In_ HDC hdc, _When_((format & DT_MODIFYSTRING), _At_((LPWSTR)lpchText, _Inout_grows_updates_bypassable_or_z_(cchText, 4)))
	_When_((!(format & DT_MODIFYSTRING)), _In_bypassable_reads_or_z_(cchText)) LPCWSTR lpchText, int cchText, LPRECT lprc, UINT format);
pfnDrawTextW fpDrawTextW = NULL;
int __stdcall MyDrawTextW(_In_ HDC hdc, _When_((format& DT_MODIFYSTRING), _At_((LPWSTR)lpchText, _Inout_grows_updates_bypassable_or_z_(cchText, 4)))
	_When_((!(format& DT_MODIFYSTRING)), _In_bypassable_reads_or_z_(cchText)) LPCWSTR lpchText, int cchText, LPRECT lprc, UINT format)
{
	int blRtn = fpDrawTextW(hdc, lpchText, cchText, lprc, format);
	return blRtn;
}
// WINGDIAPI BOOL  WINAPI ExtTextOutA( _In_ HDC hdc, _In_ int x, _In_ int y, _In_ UINT options, _In_opt_ CONST RECT * lprect, _In_reads_opt_(c) LPCSTR lpString, _In_ UINT c, _In_reads_opt_(c) CONST INT * lpDx);
// 
// 
// //
//typedef HPEN (__stdcall* pfnCreateDIBSection)(HDC hdc, BITMAPINFO* pbmi, UINT usage, void** ppvBits, HANDLE hSection, DWORD offset);
//pfnCreateDIBSection fpCreateDIBSection = NULL;
//HPEN __stdcall MyCreateDIBSectiont(HDC hdc, BITMAPINFO* pbmi, UINT usage, void** ppvBits, HANDLE hSection, DWORD offset)
//{
//	return fpCreateDIBSection(plpen);
//}

//typedef int (WINAPI* OldMessageBox)(HWND, LPCSTR, LPCSTR, UINT);
//
//OldMessageBox fpMessageBoxA = NULL;
//
//int WINAPI MyMessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
//{
//	int ret = fpMessageBoxA(hWnd, "Hook Inject", lpCaption, uType);
//	return ret;
//}

void SetHook()
{
	hFgBrush = CreateSolidBrush(RGB(255, 255, 255));
	hBkBrush = CreateSolidBrush(RGB(0, 0, 0));
	//AlphaBlend
	//AlphaBitBlt
	MH_STATUS mhStatus = MH_Initialize();
	Status (Graphics:: * bbb) (IN const WCHAR * string, IN INT length, IN const Font * font, IN const RectF & layoutRect, 
		IN const Gdiplus:: StringFormat * stringFormat, IN const Brush * brush ) = &Graphics::DrawString;
	void* abbbg = reinterpret_cast<void*&>(bbb);
	mhStatus = MH_CreateHook(abbbg, &MyDrawString, reinterpret_cast<void**>(&fpDrawString));
	mhStatus = MH_EnableHook(abbbg);

	/*mhStatus = MH_CreateHook(&RoundRect, &MyRoundRect, reinterpret_cast<void**>(&fpRoundRect));
	mhStatus = MH_EnableHook(&RoundRect);*/

	mhStatus = MH_CreateHook(&FillRect, &MyFillRect, reinterpret_cast<void**>(&fpFillRect));
	mhStatus = MH_EnableHook(&FillRect);
	
	mhStatus = MH_CreateHook(&FillRgn, &MyFillRgn, reinterpret_cast<void**>(&fpFillRgn));
	mhStatus = MH_EnableHook(&FillRgn);

	mhStatus = MH_CreateHook(&SelectObject, &MySelectObject, reinterpret_cast<void**>(&fpSelectObject));
	mhStatus = MH_EnableHook(&SelectObject);

	mhStatus = MH_CreateHook(&DrawTextW, &MyDrawTextW, reinterpret_cast<void**>(&fpDrawTextW));
	mhStatus = MH_EnableHook(&DrawTextW);

	mhStatus = MH_CreateHook(&SetBkColor, &MySetBkColor, reinterpret_cast<void**>(&fpSetBkColor));
	mhStatus = MH_EnableHook(&SetBkColor);

	mhStatus = MH_CreateHook(&SetBkMode, &MySetBkMode, reinterpret_cast<void**>(&fpSetBkMode));
	mhStatus = MH_EnableHook(&SetBkMode);

	mhStatus = MH_CreateHook(&SetTextColor, &MySetTextColor, reinterpret_cast<void**>(&fpSetTextColor));
	mhStatus = MH_EnableHook(&SetTextColor);

	mhStatus = MH_CreateHook(&CreateSolidBrush, &MyCreateSolidBrush, reinterpret_cast<void**>(&fpCreateSolidBrush));
	mhStatus = MH_EnableHook(&CreateSolidBrush);
	

	mhStatus = MH_CreateHook(&CreatePen, &MyCreatePen, reinterpret_cast<void**>(&fpCreatePen));
	mhStatus = MH_EnableHook(&CreatePen);

	mhStatus = MH_CreateHook(&CreatePenIndirect, &MyCreatePenIndirect, reinterpret_cast<void**>(&fpCreatePenIndirect));
	mhStatus = MH_EnableHook(&CreatePenIndirect);

	mhStatus = MH_CreateHook(&AlphaBlend, &MyAlphaBlend, reinterpret_cast<void**>(&fpAlphaBlend));
	mhStatus = MH_EnableHook(&AlphaBlend);
	
	mhStatus = MH_CreateHook(&BitBlt, &MyBitBlt, reinterpret_cast<void**>(&fpBitBlt));
	mhStatus = MH_EnableHook(&BitBlt);
	
	mhStatus = MH_CreateHook(&StretchBlt, &MyStretchBlt, reinterpret_cast<void**>(&fpStretchBlt));
	mhStatus = MH_EnableHook(&StretchBlt);

	mhStatus = MH_CreateHook(&ExtTextOutW, &MyExtTextOutW, reinterpret_cast<void**>(&fpExtTextOutW));
	mhStatus = MH_EnableHook(&ExtTextOutW);
	
	mhStatus = MH_CreateHook(&Rectangle, &MyRectangle, reinterpret_cast<void**>(&fpRectangle));
	mhStatus = MH_EnableHook(&Rectangle);

	mhStatus = MH_CreateHook(&RoundRect, &MyRoundRect, reinterpret_cast<void**>(&fpRoundRect));
	mhStatus = MH_EnableHook(&RoundRect);
	
	
	/*if (MH_Initialize() == MB_OK)
	{
		MH_CreateHook(&MessageBoxA, &MyMessageBoxA, reinterpret_cast<void**>(&fpMessageBoxA));
		MH_EnableHook(&MessageBoxA);
	}*/
}

void UnHook()
{
	MH_Uninitialize();
	/*if (MH_DisableHook(&MessageBoxA) == MB_OK)
	{
		MH_Uninitialize();
	}*/
}


