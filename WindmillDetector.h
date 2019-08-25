#pragma once

#include <string>

class WindmillDetector {
public:
	WindmillDetector();
	int Detect(void);

private:
	bool m_clrRed;
	float m_tgtHwRatioMax;
	int m_clrThresL[3], m_clrThresH[3];
	int m_Delayer, m_DelayerMax;
	int m_tgtAreaMin, m_tgtAreaMax;
	int m_tgtBiasXMax, m_tgtBiasYMax;
	std::string m_vidPath;
	cv::FileStorage m_stgRead;
	cv::Point2i m_tgtPast;
	cv::Scalar m_clrRendering;
	cv::VideoCapture m_vidRead;

	void f_SetMembers(void);
};