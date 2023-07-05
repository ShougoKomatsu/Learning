#include "stdafx.h"
#include "CVImage.h"
#pragma once

#define DELETE_OBJECT(arg) if(arg!= NULL){DeleteObject(arg); arg=NULL;}
#define SAFE_DELETE(arg) if(arg!= NULL){delete [] (arg); arg=NULL;}

BOOL CalcAsp1(int iR0, int iR1, int iC1, int iWinW, int iWinH, int* iNeR0, int* iNeC0, int* iNewR1, int* iNewC1);
#define CHANNEL_UNDEFINED (0)
#define CHANNEL_1_8 (1)
#define CHANNEL_3_8 (3)
struct ImgRGB
{
	int iWidth;
	int iHeight;
	int iChannel;
	BYTE* byImgR;
	BYTE* byImgG;
	BYTE* byImgB;
	BOOL Set(int iWidth, int iHeight);
	BOOL Init();
	ImgRGB(){byImgR=NULL;byImgG=NULL;byImgB=NULL;Init();}
	~ImgRGB(){Init();}
BOOL Assign(CVImage* cvImg);
};

BOOL ImposeImage(BYTE* byData, int iR0, int iC0, int iR1, int iC1, int iWinW, int iWinH, ImgRGB* imgrgb);
#define DISPLAY_VISIBLE (1)
#define DISPLAY_DUMPABLE (2)
#define DISPLAY_DRAWABLE (4)


class ImageWindowDlg
{
public:
	HDC m_hDC;
	BITMAPINFO m_bmi;
	BITMAPINFOHEADER m_bmih;
	HBITMAP m_hBitMap;
	HWND m_hDlg;
	BYTE* m_v;
	BYTE* m_vDump;
	BYTE* m_vOrg;

	int m_iR0;
	int m_iC0;
	int m_iR1;
	int m_iC1;
	int m_iWidth;
	int m_iHeight;

	int m_iDisplayMode;

	BOOL Init()
	{
		m_iDisplayMode = (DISPLAY_VISIBLE | DISPLAY_DUMPABLE);
		m_hDC = NULL;
		m_v = NULL;
		m_vDump = NULL;
		m_vOrg = NULL;
		m_hDlg = NULL;
		m_hBitMap = NULL;
		return TRUE;
	}
	BOOL Exit()
	{
		if(m_hDC != NULL){DeleteDC(m_hDC); m_hDC=NULL;}
		if(m_hBitMap != NULL){DELETE_OBJECT(m_hBitMap); m_v=NULL;}
		SAFE_DELETE(m_vDump);
		SAFE_DELETE(m_vOrg);
		return TRUE;
	}
	ImageWindowDlg(){Init();}
	~ImageWindowDlg(){Exit();}

	BITMAPFILEHEADER m_bmfh;

	BOOL OpenWindow(HWND dlg, int iX0, int iY0, int iWidth, int iHeight,  int iDisplayMode);
	BOOL CloseWindow();
	BOOL SetPart(int iR0, int iC0, int iR1, int iC1);
	BOOL DispImage(ImgRGB* imgRGB);

};