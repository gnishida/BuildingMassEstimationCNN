#pragma once

#include <opencv/cv.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

namespace fe {

	void adjustFacadeImage(cv::Mat& img);
	double getDominantOrientation(const cv::Mat& img);

}