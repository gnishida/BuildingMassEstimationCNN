#include "FacadeEstimation.h"
#include <iostream>
#include <QString>
#include <time.h>

#ifndef SQR
#define SQR(x)	((x) * (x))
#endif

namespace fe {

	const double M_PI = 3.1415926535897932384626433832795;

	void adjustFacadeImage(cv::Mat& img) {
		// compute the dominant orientation of the rectified image
		double angle = fe::getDominantOrientation(img);

		cv::Mat rot = cv::getRotationMatrix2D(cv::Point2f(img.cols * 0.5, img.rows * 0.5), angle / M_PI * 180.0, 1.0);
		cv::warpAffine(img, img, rot, img.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255, 255));
	}

	double getDominantOrientation(const cv::Mat& img) {
		cv::Mat grayImg;
		if (img.channels() == 1) {
			img.copyTo(grayImg);
		}
		else if (img.channels() == 3) {
			cv::cvtColor(img, grayImg, cv::COLOR_BGR2GRAY);
		}
		else if (img.channels() == 4) {
			cv::cvtColor(img, grayImg, cv::COLOR_BGRA2GRAY);
		}

		// detect edges
		cv::Mat edgeImg;
		int lowThreshold = 50;
		int ratio = 3;
		int kernel_size = 3;
		cv::blur(grayImg, grayImg, cv::Size(3, 3));
		cv::Canny(grayImg, edgeImg, lowThreshold, lowThreshold * ratio, kernel_size);

		// detect lines
		std::vector<cv::Vec4f> lines;
		cv::HoughLinesP(edgeImg, lines, 1, CV_PI / 180, 80, 30, 20);
		
		// use histogram to find the dominant orientation {[-45, -35), [-35, -25), [-25, -15), [-15, -5), [-5, 5), ..., [35, 45)}
		std::vector<double> histogram(8, 0);
		for (size_t i = 0; i < lines.size(); ++i) {
			double theta = atan2(lines[i][3] - lines[i][1], lines[i][2] - lines[i][0]);
			if (theta < 0) {
				theta += M_PI;
			}

			// make the vertical line to a horizontal line
			if (theta >= M_PI * 0.25 && theta < M_PI * 0.5) {
				theta += M_PI * 0.5;
			}
			else if (theta >= M_PI * 0.5 && theta < M_PI * 0.75) {
				theta -= M_PI * 0.5;
			}

			// make the theta within the range [-PI/2, PI/2)
			if (theta > M_PI * 0.5) {
				theta -= M_PI;
			}

			int bin_id = (theta + M_PI * 0.25) / M_PI * 180 / 10;
			if (bin_id >= 0 && bin_id < histogram.size()) {
				histogram[bin_id] += sqrt(SQR(lines[i][2] - lines[i][0]) + SQR(lines[i][3] - lines[i][1]));
			}
		}

		int best_bin_id = std::distance(histogram.begin(), std::max_element(histogram.begin(), histogram.end()));

		// average the orientation of the dominant edges
		double theta_total = 0.0;
		int count = 0;
		for (size_t i = 0; i < lines.size(); ++i) {
			double theta = atan2(lines[i][3] - lines[i][1], lines[i][2] - lines[i][0]);
			if (theta < 0) {
				theta += M_PI;
			}

			// make the vertical line to a horizontal line
			if (theta >= M_PI * 0.25 && theta < M_PI * 0.5) {
				theta += M_PI * 0.5;
			}
			else if (theta >= M_PI * 0.5 && theta < M_PI * 0.75) {
				theta -= M_PI * 0.5;
			}

			// make the theta within the range [-PI/2, PI/2)
			if (theta > M_PI * 0.5) {
				theta -= M_PI;
			}

			int bin_id = (theta + M_PI * 0.25) / M_PI * 180 / 10;
			if (bin_id == best_bin_id) {
				theta_total += theta;
				count++;
			}
		}
		
		if (count > 0) {
			return theta_total / count;
		}
		else {
			return 0;
		}
	}

}
