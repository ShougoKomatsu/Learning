#include "stdafx.h"
#include "CVImage.h"


BOOL CVImage::Assign(cv::Mat* cvMat)
{
	return TRUE;
}

BOOL CVImage::Assign(TCHAR* tchFilePath)
{
	m_Image.release();
	m_Image = cv::imread(tchFilePath, cv::IMREAD_COLOR );
	return TRUE;
}


BOOL CVImage::Init()
{
	if(m_pbyImage1!=NULL){delete [] m_pbyImage1; m_pbyImage1=NULL;}
	if(m_pbyImage2!=NULL){delete [] m_pbyImage2; m_pbyImage2=NULL;}
	if(m_pbyImage3!=NULL){delete [] m_pbyImage3; m_pbyImage3=NULL;}

	m_Image.release();

	return TRUE;
}
