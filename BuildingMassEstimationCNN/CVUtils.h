#pragma once

#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>

namespace cvutils {
	cv::Mat rectify_image(cv::Mat& image, const std::vector<cv::Point2f>& pts);	cv::Mat adjust_contrast(const cv::Mat& image);
	cv::Mat adjust_contrast2(const cv::Mat& image, float threshold = 0.01);
}


