#include "stdafx.h"

#include <opencv2/imgcodecs.hpp>

class CVImage
{
	cv::Mat m_Image;
	BYTE* m_pbyImage1;
	BYTE* m_pbyImage2;
	BYTE* m_pbyImage3;

	BOOL Assign(cv::Mat* cvMat);
	BOOL Assign(TCHAR* tchFilePath);
	CVImage(){m_pbyImage1=NULL;m_pbyImage2=NULL;m_pbyImage3=NULL; Init();}
	~CVImage(){Init();}
	BOOL Init();
};