#include "stdafx.h"

#include <opencv2/imgcodecs.hpp>
#pragma once
class CVImage
{
public:
	cv::Mat m_Image1;
	cv::Mat m_Image2;
	cv::Mat m_Image3;

	BYTE* m_pbyImage1;
	BYTE* m_pbyImage2;
	BYTE* m_pbyImage3;


	int iWidth;
	int iHeight;

	BOOL Assign(CVImage* cvImage);
	BOOL Assign(cv::Mat* cvMat);
	BOOL Assign(cv::Mat cvMat);
	BOOL Assign(TCHAR* tchFilePath);
	CVImage(){Init();}
	~CVImage(){Init();}
	BOOL Init();
};