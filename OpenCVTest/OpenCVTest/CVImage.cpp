#include "stdafx.h"
#include "CVImage.h"


BOOL CVImage::Assign(cv::Mat cvMat)
{
	Init();
	if(cvMat.channels()==1)
	{
		cvMat.copyTo(m_Image1);
		m_pbyImage1 = m_Image1.ptr();
	}

	if(cvMat.channels()==3)
	{
		cv::extractChannel(cvMat,m_Image1,2);
		cv::extractChannel(cvMat,m_Image2,1);
		cv::extractChannel(cvMat,m_Image3,0);

		m_pbyImage1 = m_Image1.ptr();
		m_pbyImage2 = m_Image2.ptr();
		m_pbyImage3 = m_Image3.ptr();
	}

	CvSize size;
	size = m_Image1.size();
	iWidth = size.width;
	iHeight = size.height;
	return TRUE;
}


BOOL CVImage::Assign(cv::Mat* cvMat)
{
	return Assign(*cvMat);
}

BOOL CVImage::Assign(TCHAR* tchFilePath)
{
	return Assign(cv::imread(tchFilePath, cv::IMREAD_COLOR ));
}


BOOL CVImage::Assign(CVImage* cvImage)
{
	return Assign(&(cvImage->m_Image1));
}

BOOL CVImage::Init()
{
//	if(m_pbyImage1!=NULL){delete [] m_pbyImage1; m_pbyImage1=NULL;}
//	if(m_pbyImage2!=NULL){delete [] m_pbyImage2; m_pbyImage2=NULL;}
//	if(m_pbyImage3!=NULL){delete [] m_pbyImage3; m_pbyImage3=NULL;}

	m_Image1.release();
	m_Image2.release();
	m_Image3.release();
	m_pbyImage1=NULL;
	m_pbyImage2=NULL;
	m_pbyImage3=NULL; 
	return TRUE;
}

