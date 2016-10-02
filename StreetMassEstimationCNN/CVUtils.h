#pragma once

#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>

namespace cvutils {
	static enum { LOCAL_MINIMUM = 1, LOCAL_MAXIMUM = 2 };	cv::Mat rectify_image(cv::Mat& image, const std::vector<cv::Point2f>& pts, cv::Mat& transformMat);	cv::Mat adjust_contrast(const cv::Mat& image);
	cv::Mat adjust_contrast2(const cv::Mat& image, float threshold = 0.01);

	double max(const cv::Mat& mat);
	double min(const cv::Mat& mat);
	std::vector<double> mean(const cv::Mat& mat);
	double msd(const cv::Mat& img1, const cv::Mat& img2);
	double corr(const cv::Mat& img1, const cv::Mat& img2);
	double get(const cv::Mat& mat, int r, int c);
	bool isLocalMinimum(const cv::Mat& mat, int index, int num);
	bool isLocalMaximum(const cv::Mat& mat, int index, int num);
	std::vector<int> getPeak(const cv::Mat& mat, bool smooth, int sigma, int flag = 1, int width = 1);
	float getMostPopularValue(const cv::Mat& h_max, float sigma, float min_value);

}


