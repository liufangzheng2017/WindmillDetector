#include "Main.h"

WindmillDetector::WindmillDetector() {
	try {
		m_stgRead.open("Settings.yaml", cv::FileStorage::READ);
	}
	catch (...) {
		std::cerr << "[Error] E0004 An exception occurred while loading Settings.yaml." << std::endl;
		std::cerr << "[Trace] Thrown at WindmillDetector::WindmillDetector()." << std::endl;
		exit(4);
	}
	if (m_stgRead.isOpened() == false) {
		std::cerr << "[Error] E0002 Settings.yaml is inaccessible." << std::endl;
		std::cerr << "[Trace] Thrown at WindmillDetector::WindmillDetector()." << std::endl;
		exit(2);
	}

	int tmpStgRead = m_stgRead["ColourIsRed"];
	if (tmpStgRead == 0) {
		m_clrRed = false;
	}
	else if (tmpStgRead == 1) {
		m_clrRed = true;
	}
	else {
		std::cerr << "[Warning] W0002 Field ColourIsRed has an unexpected value, using 0 instead." << std::endl;
		std::cerr << "[  Trace] Raised at WindmillDetector::WindmillDetector()." << std::endl;
		m_clrRed = false;
	}

	std::cout << "[Output] Initializing. Users are resposible for validity of Settings.yaml." << std::endl;
	f_SetMembers();
	m_stgRead.release();
}

int WindmillDetector::Detect(void) {
	std::chrono::steady_clock::time_point clkPast, clkNow;
	std::chrono::microseconds clkSpan;
	clkPast = std::chrono::steady_clock::now();

	std::vector<cv::Vec4i> ctrHierarchy;
	std::vector<std::vector<cv::Point2i>> ctrSet;
	cv::Mat imgSource, imgProcessed;

	m_vidRead >> imgSource;
	if (imgSource.empty()) {
		std::cerr << "[Warning] W0001 Source image is empty." << std::endl;
		std::cerr << "[  Trace] Raised at WindmillDetector::Detect." << std::endl;
		return 1;
	}

	cv::resize(imgSource, imgSource, cv::Size((int)(0.5 * imgSource.cols), (int)(0.5 * imgSource.rows)));
	cv::cvtColor(imgSource, imgProcessed, cv::COLOR_BGR2HSV);
	cv::inRange(imgProcessed, cv::Scalar(m_clrThresL[H], m_clrThresL[S], m_clrThresL[V]),
		cv::Scalar(m_clrThresH[H], m_clrThresH[S], m_clrThresH[V]), imgProcessed);
	cv::dilate(imgProcessed, imgProcessed, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2)), cv::Point(-1, -1), 1);
	cv::findContours(imgProcessed, ctrSet, ctrHierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_NONE);

	for (int i = 0, iBackup = 0; i < ctrSet.size(); i++) {
		if (ctrHierarchy[i][Parent] >= 0) continue;
		if (ctrHierarchy[i][Child] < 0) continue;
		if (cv::contourArea(ctrSet[i]) < m_tgtAreaMin || cv::contourArea(ctrSet[i]) > m_tgtAreaMax) continue;
		iBackup = i;
		i = ctrHierarchy[i][Child];
		while (i >= 0) {
			cv::RotatedRect tgtBounding = cv::minAreaRect(ctrSet[i]);
			cv::Point2f tgtVertices[4];
			if (cv::contourArea(ctrSet[i]) < m_tgtAreaMin || cv::contourArea(ctrSet[i]) > m_tgtAreaMax) {
				i = ctrHierarchy[i][Next];
				continue;
			}

			float tgtHeight = tgtBounding.size.height, tgtWidth = tgtBounding.size.width;
			if (tgtHeight > tgtWidth) {
				float t = tgtHeight;
				tgtHeight = tgtWidth;
				tgtWidth = t;
			}

			float tgtSize = tgtHeight * tgtWidth;
			float tgtHwRatio = tgtWidth / tgtHeight;
			if (m_tgtAreaMin < tgtSize && tgtSize < m_tgtAreaMax && tgtHwRatio < m_tgtHwRatioMax) {
				if (fabs(m_tgtPast.x - tgtBounding.center.x) > m_tgtBiasXMax
					|| fabs(m_tgtPast.y - tgtBounding.center.y) > m_tgtBiasYMax) {
					m_Delayer++;
					if (m_Delayer < m_DelayerMax) break;
				}
				m_Delayer = 0;
				m_tgtPast = tgtBounding.center;
				tgtBounding.points(tgtVertices);
				cv::line(imgSource, tgtVertices[0], tgtVertices[1], m_clrRendering, 2);
				cv::line(imgSource, tgtVertices[1], tgtVertices[2], m_clrRendering, 2);
				cv::line(imgSource, tgtVertices[2], tgtVertices[3], m_clrRendering, 2);
				cv::line(imgSource, tgtVertices[3], tgtVertices[0], m_clrRendering, 2);
				i = iBackup;
				break;
			}
			i = ctrHierarchy[i][Next];
		}
		i = iBackup;
	}
	clkNow = std::chrono::steady_clock::now();
	clkSpan = std::chrono::duration_cast<std::chrono::microseconds>(clkNow - clkPast);
	std::cout << "[Output] It took " << clkSpan.count() / 1000 << " millisecond(s) to finish detection." << std::endl;

	cv::imshow("imgSource", imgSource);
	cv::imshow("imgProcessed", imgProcessed);
	cv::waitKey();
	return 0;
}

void WindmillDetector::f_SetMembers(void) {
	m_vidPath = (std::string)m_stgRead["VideoPath"];
	if (m_vidPath.empty()) {
		std::cerr << "[Error] E0001 VideoPath in Settings.yaml is empty." << std::endl;
		std::cerr << "[Trace] Thrown at WindmillDetector::WindmillDetector()." << std::endl;
		exit(1);
	}
	m_vidRead.open(m_vidPath);
	if (m_vidRead.isOpened() == false) {
		std::cerr << "[Error] E0003 Cannot open video specified in Settings.yaml." << std::endl;
		std::cerr << "[Trace] Thrown at WindmillDetector::WindmillDetector()." << std::endl;
		exit(3);
	}

	if (m_clrRed == true) {
		m_clrThresL[H] = m_stgRead["ColourThresLRedH"];
		m_clrThresL[S] = m_stgRead["ColourThresLRedS"];
		m_clrThresL[V] = m_stgRead["ColourThresLRedV"];
		m_clrThresH[H] = m_stgRead["ColourThresHRedH"];
		m_clrThresH[S] = m_stgRead["ColourThresHRedS"];
		m_clrThresH[V] = m_stgRead["ColourThresHRedV"];
		m_clrRendering = cv::Scalar(255, 255, 0);
	}
	else {
		m_clrThresL[H] = m_stgRead["ColourThresLBlueH"];
		m_clrThresL[S] = m_stgRead["ColourThresLBlueS"];
		m_clrThresL[V] = m_stgRead["ColourThresLBlueV"];
		m_clrThresH[H] = m_stgRead["ColourThresHBlueH"];
		m_clrThresH[S] = m_stgRead["ColourThresHBlueS"];
		m_clrThresH[V] = m_stgRead["ColourThresHBlueV"];
		m_clrRendering = cv::Scalar(0, 0, 255);
	}

	m_tgtAreaMin = m_stgRead["BoundingAreaMin"];
	m_tgtAreaMax = m_stgRead["BoundingAreaMax"];
	m_tgtBiasXMax = m_stgRead["BiasXMax"];
	m_tgtBiasYMax = m_stgRead["BiasYMax"];
	m_tgtHwRatioMax = m_stgRead["HwRatioMax"];
	m_DelayerMax = m_stgRead["Delayer"];

	m_Delayer = 0;
	m_tgtPast = cv::Point2i(0, 0);
}