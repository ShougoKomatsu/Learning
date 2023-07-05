#include "stdafx.h"
#include "ImageWindow.h"

double frac(double in)
{
	return in-int(in);
}

BOOL ImposeImage(BYTE* byData, int iR0, int iC0, int iR1, int iC1, int iWinW, int iWinH, ImgRGB* Img)
{
	if(byData == NULL){return FALSE;}
	if(iWinW<=0){return FALSE;}
	if(iWinH<=0){return FALSE;}
	
	double a, b;
	int ii;
	int i1, i2, i3, i4;
	i1=0;
	i2=0;
	i3=0;
	i4=0;
	ii=0;
	int i5;
	i5=0;
	CString sss;
	sss.Format(_T("%d %d %d %d | %d %d / %f"), iR0, iC0, iR1, iC1, iWinW, iWinH,(iR1-iR0+1)/(1.0*iWinH));
//	AfxMessageBox(sss);
	if(Img->iChannel==CHANNEL_3_8)
	{
		for(int r=0; r<iWinH; r++)
		{
			for(int c=0; c<iWinW; c++)
			{
				a=frac(iR0+(iR1-iR0+1)/(1.0*iWinH)*(r));
				b=frac(iC0+(iC1-iC0+1)/(1.0*iWinW)*(c));

				int i00 = int(iR0+(iR1-iR0+1)/(1.0*iWinH)*(r))*Img->iWidth + int(iC0+(iC1-iC0+1)/(1.0*iWinW)*(c));
				int i01 = int(iR0+(iR1-iR0+1)/(1.0*iWinH)*(r))*Img->iWidth + int(iC0+(iC1-iC0+1)/(1.0*iWinW)*(c+1));
				int i10 = int(iR0+(iR1-iR0+1)/(1.0*iWinH)*(r+1))*Img->iWidth + int(iC0+(iC1-iC0+1)/(1.0*iWinW)*(c));
				int i11 = int(iR0+(iR1-iR0+1)/(1.0*iWinH)*(r+1))*Img->iWidth + int(iC0+(iC1-iC0+1)/(1.0*iWinW)*(c+1));

				if(int(iR0 +(iR1-iR0+1)/(1.0*iWinH)*(r)) >= Img->iHeight){byData[3*(r*iWinW+c)+0]=128;byData[3*(r*iWinW+c)+1]=0;byData[3*(r*iWinW+c)+2]=0; i1++;continue;}
				if(int(iC0 +(iC1-iC0+1)/(1.0*iWinW)*(c)) >= Img->iWidth){byData[3*(r*iWinW+c)+0]=0;byData[3*(r*iWinW+c)+1]=0;byData[3*(r*iWinW+c)+2]=0; i2++;continue;}
				if(int(iR0 +(iR1-iR0+1)/(1.0*iWinH)*(r)) <0) {byData[3*(r*iWinW+c)+0]=0;byData[3*(r*iWinW+c)+1]=0;byData[3*(r*iWinW+c)+2]=0; i3++;continue;}
				if(int(iC0 +(iC1-iC0+1)/(1.0*iWinW)*(c)) <0) {byData[3*(r*iWinW+c)+0]=0;byData[3*(r*iWinW+c)+1]=0;byData[3*(r*iWinW+c)+2]=0; i3++;continue;}
				if((i00 <0) || (i01 <0)|| (i10 <0)||(i11 <0)){byData[3*(r*iWinW+c)+0]=0;byData[3*(r*iWinW+c)+1]=0;byData[3*(r*iWinW+c)+2]=0; i3++;continue;}

				if((i00>=Img->iHeight*Img->iWidth) || (i01>=Img->iHeight*Img->iWidth) || (i10>=Img->iHeight*Img->iWidth) || (i11>=Img->iHeight*Img->iWidth))
				{byData[3*(r*iWinW+c)+0]=0;byData[3*(r*iWinW+c)+1]=0;byData[3*(r*iWinW+c)+2]=0; i4++;continue;}
				ii++;
				byData[3*(r*iWinW+c)+0] = BYTE(b*(a*Img->byImgB[i11] +(1-a)*Img->byImgB[i01]) + (1-b)*(a*Img->byImgB[i10] +(1-a)*Img->byImgB[i11]));
				byData[3*(r*iWinW+c)+1] = BYTE(b*(a*Img->byImgG[i11] +(1-a)*Img->byImgG[i01]) + (1-b)*(a*Img->byImgG[i10] +(1-a)*Img->byImgG[i11]));
				byData[3*(r*iWinW+c)+2] = BYTE(b*(a*Img->byImgR[i11] +(1-a)*Img->byImgR[i01]) + (1-b)*(a*Img->byImgR[i10] +(1-a)*Img->byImgR[i11]));
				if((byData[3*(r*iWinW+c)+0]!=0)||(byData[3*(r*iWinW+c)+1]!=0)||(byData[3*(r*iWinW+c)+2]!=0)){i5++;}
			}
		}
	}
	sss.Format(_T("ii = %d %d %d %d | %d %d %d %d %d"), ii, iWinH, iWinW, iWinH*iWinW, i1, i2, i3, i4, i5);
//	AfxMessageBox(sss);
	if(Img->iChannel==CHANNEL_1_8)
	{
		for(int r=0; r<iWinH; r++)
		{
			for(int c=0; c<iWinW; c++)
			{
				a=frac(iR0+(iR1-iR0+1)/(1.0*iWinH)*(r));
				b=frac(iC0+(iC1-iC0+1)/(1.0*iWinW)*(c));

				int i00 = int(iR0+(iR1-iR0+1)/(1.0*iWinH)*(r))*Img->iWidth + int(iC0+(iC1-iC0+1)/(1.0*iWinW)*(c));
				int i01 = int(iR0+(iR1-iR0+1)/(1.0*iWinH)*(r))*Img->iWidth + int(iC0+(iC1-iC0+1)/(1.0*iWinW)*(c+1));
				int i10 = int(iR0+(iR1-iR0+1)/(1.0*iWinH)*(r+1))*Img->iWidth + int(iC0+(iC1-iC0+1)/(1.0*iWinW)*(c));
				int i11 = int(iR0+(iR1-iR0+1)/(1.0*iWinH)*(r+1))*Img->iWidth + int(iC0+(iC1-iC0+1)/(1.0*iWinW)*(c+1));
				
				if(int(iR0 +(iR1-iR0+1)/(1.0*iWinH)*(r)) >= Img->iHeight){byData[3*(r*iWinW+c)+0]=0;byData[3*(r*iWinW+c)+1]=0;byData[3*(r*iWinW+c)+2]=0; continue;}
				if(int(iC0 +(iC1-iC0+1)/(1.0*iWinW)*(c)) >= Img->iWidth){byData[3*(r*iWinW+c)+0]=0;byData[3*(r*iWinW+c)+1]=0;byData[3*(r*iWinW+c)+2]=0; continue;}
				if(int(iR0 +(iR1-iR0+1)/(1.0*iWinH)*(r)) <0) {byData[3*(r*iWinW+c)+0]=0;byData[3*(r*iWinW+c)+1]=0;byData[3*(r*iWinW+c)+2]=0; continue;}
				if(int(iC0 +(iC1-iC0+1)/(1.0*iWinW)*(c)) <0) {byData[3*(r*iWinW+c)+0]=0;byData[3*(r*iWinW+c)+1]=0;byData[3*(r*iWinW+c)+2]=0; continue;}
				if((i00 <0) || (i01 <0)|| (i10 <0)||(i11 <0)){byData[3*(r*iWinW+c)+0]=0;byData[3*(r*iWinW+c)+1]=0;byData[3*(r*iWinW+c)+2]=0; continue;}

				if((i00>=Img->iHeight*Img->iWidth) || (i01>=Img->iHeight*Img->iWidth) || (i10>=Img->iHeight*Img->iWidth) || (i11>=Img->iHeight*Img->iWidth))
				{byData[3*(r*iWinW+c)+0]=0;byData[3*(r*iWinW+c)+1]=0;byData[3*(r*iWinW+c)+2]=0; continue;}

				byData[3*(r+iWinW+c)+0] = BYTE(b*(a*Img->byImgR[i11] +(1-a)*Img->byImgR[i01]) + (1-b)*(a*Img->byImgR[i10] +(1-a)*Img->byImgR[i11]));
				byData[3*(r+iWinW+c)+1] = BYTE(b*(a*Img->byImgR[i11] +(1-a)*Img->byImgR[i01]) + (1-b)*(a*Img->byImgR[i10] +(1-a)*Img->byImgR[i11]));
				byData[3*(r+iWinW+c)+2] = BYTE(b*(a*Img->byImgR[i11] +(1-a)*Img->byImgR[i01]) + (1-b)*(a*Img->byImgR[i10] +(1-a)*Img->byImgR[i11]));

			}
		}

	}

	return TRUE;
}
BOOL ImgRGB::Init()
{
	if(byImgR!= NULL){delete [] byImgR; byImgR=NULL;}
	if(byImgG!= NULL){delete [] byImgG; byImgG=NULL;}
	if(byImgB!= NULL){delete [] byImgB; byImgB=NULL;}
	iChannel = CHANNEL_UNDEFINED;
	iWidth=0;
	iHeight=0;
	return TRUE;
}
BOOL ImgRGB::Set(int iWidthIn, int iHeightIn)
{

	Init();
	this->iWidth=iWidthIn;
	this->iHeight=iHeightIn;

	byImgR=new BYTE[iWidth*iHeight];
	byImgG=new BYTE[iWidth*iHeight];
	byImgB=new BYTE[iWidth*iHeight];

	if(byImgR==NULL){return FALSE;}
	if(byImgG==NULL){return FALSE;}
	if(byImgB==NULL){return FALSE;}
	for(int r=0; r<iHeight; r++)
	{
		for(int c=0; c<iWidth; c++)
		{
			byImgR[iWidth*r+c]=100;
			byImgG[iWidth*r+c]=0;
			byImgB[iWidth*r+c]=0;
		}
	}
	iChannel = CHANNEL_1_8;
	return TRUE;
}
BOOL ImageWindowDlg::OpenWindow(HWND dlg, int iX0, int iY0, int iWidth, int iHeight, int iDisplayMode)
{
	if(iWidth<0){return FALSE;}
	if(iHeight<0){return FALSE;}

	m_iWidth = iWidth;
	m_iHeight = iHeight;
	m_hDlg = dlg;

	CWnd* wnd_bmp;

	wnd_bmp=NULL;
	m_v = NULL;
	if((m_iDisplayMode & DISPLAY_VISIBLE) == DISPLAY_VISIBLE)
	{

		wnd_bmp = wnd_bmp->FromHandle(dlg);
		if(wnd_bmp == NULL){return FALSE;}

		wnd_bmp->MoveWindow(iX0, iY0, iWidth-1,iHeight-1);
		
		m_hDC = (wnd_bmp->GetDC())->GetSafeHdc();


		m_bmih.biSize=sizeof(m_bmih);
		m_bmih.biWidth=iWidth;
		m_bmih.biHeight=iHeight;

		if(m_bmih.biHeight>0){m_bmih.biHeight*=-1;}


	}

	m_bmih.biPlanes=1;
	m_bmih.biBitCount=24;
	m_bmih.biXPelsPerMeter=0;
	m_bmih.biYPelsPerMeter=0;
	
	m_bmih.biClrUsed=0;
	m_bmih.biClrImportant=0;

	m_bmih.biCompression=BI_RGB;
	m_bmi.bmiHeader=m_bmih;
	m_bmih.biSizeImage=iWidth*iHeight*3;
	m_bmi.bmiHeader=m_bmih;

	m_hBitMap = NULL;



	m_hBitMap = CreateDIBSection(m_hDC, &m_bmi, DIB_RGB_COLORS, (void**)&m_v, NULL, 0);
	return TRUE;
}

BOOL ImageWindowDlg::SetPart(int iR0, int iC0, int iR1, int iC1)
{
	m_iC0 = iC0;
	m_iR0 = iR0;
	m_iC1 = iC1;
	m_iR1 = iR1;
	return TRUE;
}

BOOL ImageWindowDlg::DispImage(ImgRGB* imgRGB)
{
	HDC hDCCompt;
	HBITMAP hBmpOrg;
	hDCCompt = CreateCompatibleDC(m_hDC);
	BOOL bRet;
	hBmpOrg = (HBITMAP)::SelectObject(hDCCompt, m_hBitMap);


	bRet = ImposeImage(m_v,m_iR0,m_iC0,m_iR1,m_iC1, m_iWidth, m_iHeight,imgRGB); 
	CString sss;
	sss.Format(_T("bRet = %d"), bRet);
//	AfxMessageBox(sss);
	BitBlt(m_hDC, 0, 0, m_iWidth, m_iHeight, hDCCompt, 0, 0, SRCCOPY);
	SelectObject(hDCCompt, hBmpOrg);
	DeleteDC(hDCCompt);
	DeleteObject(hBmpOrg);
	return TRUE;
}
#include "CVImage.h"

BOOL ImgRGB::Assign(CVImage* cvImg)
{
this->Set(cvImg->iWidth, cvImg->iHeight);
this->iChannel=CHANNEL_3_8;
for(int r=0; r<iHeight; r++)
{
	for(int c=0; c<iWidth; c++)
	{
		this->byImgR[r*iWidth+c]=cvImg->m_pbyImage1[r*iWidth+c];
		this->byImgG[r*iWidth+c]=cvImg->m_pbyImage2[r*iWidth+c];
		this->byImgB[r*iWidth+c]=cvImg->m_pbyImage3[r*iWidth+c];
	}
}
return TRUE;
}