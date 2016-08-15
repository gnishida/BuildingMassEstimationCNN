#include "CVUtils.h"

namespace cvutils {

	/**
	 * 画像と4つの頂点を与えられた時に、rectifyして四角形の画像を作成する。
	 * ただし、4つの頂点は半時計周りに並んでいること。
	 */
	cv::Mat rectify_image(cv::Mat& image, const std::vector<cv::Point2f>& pts) {
		cv::Point2f tl = pts[3];
		cv::Point2f tr = pts[2];
		cv::Point2f br = pts[1];
		cv::Point2f bl = pts[0];

		// compute the width of the new image, which will be the
		// maximum distance between bottom - right and bottom - left
		// x - coordiates or the top - right and top - left x - coordinates
		float widthA = cv::norm(br - bl);
		float widthB = cv::norm(tr - tl);
		int maxWidth = std::max(int(widthA), int(widthB));

		// compute the height of the new image, which will be the
		// maximum distance between the top - right and bottom - right
		// y - coordinates or the top - left and bottom - left y - coordinates
		float heightA = cv::norm(tr - br);
		float heightB = cv::norm(tl - bl);
		int maxHeight = std::max(int(heightA), int(heightB));

		// now that we have the dimensions of the new image, construct
		// the set of destination points to obtain a "birds eye view",
		// (i.e. top-down view) of the image, again specifying points
		// in the top - left, top - right, bottom - right, and bottom - left
		// order
		std::vector<cv::Point2f> dst(4);
		dst[0] = cv::Point2f(0, maxHeight - 1);
		dst[1] = cv::Point2f(maxWidth - 1, maxHeight - 1);
		dst[2] = cv::Point2f(maxWidth - 1, 0);
		dst[3] = cv::Point2f(0, 0);

		// compute the perspective transform matrix and then apply it
		cv::Mat M = cv::getPerspectiveTransform(pts, dst);
		cv::Mat warped;
		cv::warpPerspective(image, warped, M, cv::Size(maxWidth, maxHeight));

		// return the warped image
		return warped;
	}

	cv::Mat adjust_contrast(const cv::Mat& image) {
		// convert the image to HSV format
		cv::Mat src;
		if (image.channels() == 1) {
			cv::cvtColor(image, src, cv::COLOR_GRAY2BGR);
			cv::cvtColor(src, src, cv::COLOR_BGR2HSV);
		}
		else if (image.channels() == 3) {
			cv::cvtColor(image, src, cv::COLOR_BGR2HSV);
		}
		else if (image.channels() == 4) {
			cv::cvtColor(image, src, cv::COLOR_BGRA2BGR);
			cv::cvtColor(src, src, cv::COLOR_BGR2HSV);
		}

		// split the image into 3 channels
		std::vector<cv::Mat> splittedChannels(3);
		cv::split(src, splittedChannels);
		
		// adjusst the contrast and brightness
		/*
		// equalize the histogram
		cv::equalizeHist(splittedChannels[2], splittedChannels[2]);
		*/

		cv::Mat adjusted;
		splittedChannels[2].convertTo(adjusted, CV_64F, 1.0 / 255.0);
		cv::Scalar mean, stddev;
		cv::meanStdDev(adjusted, mean, stddev);
		adjusted = adjusted - (mean - stddev * 2);// *stddev;// / 4.0;
		cv::Mat diviser(adjusted.size(), adjusted.type(), stddev * 4);
		cv::divide(adjusted, diviser, adjusted);
		adjusted.convertTo(splittedChannels[2], CV_8U, 255.0);

		// merge the channels back to a single image
		cv::merge(splittedChannels, src);

		// convert the HSV to BGR format
		cv::Mat result;
		cv::cvtColor(src, result, cv::COLOR_HSV2BGR);

		return result;
	}

	cv::Mat adjust_contrast2(const cv::Mat& image, float threshold) {
		cv::Mat hsvImg;
		if (image.channels() == 1) {
			cv::cvtColor(image, hsvImg, cv::COLOR_GRAY2BGR);
			cv::cvtColor(hsvImg, hsvImg, cv::COLOR_BGR2HSV);
		}
		else if (image.channels() == 3) {
			cv::cvtColor(image, hsvImg, cv::COLOR_BGR2HSV);
		}
		else if (image.channels() == 4) {
			cv::cvtColor(image, hsvImg, cv::COLOR_BGRA2BGR);
			cv::cvtColor(hsvImg, hsvImg, cv::COLOR_BGR2HSV);
		}

		std::vector<float> histogram(256, 0.0f);
		for (int r = 0; r < hsvImg.rows; ++r) {
			for (int c = 0; c < hsvImg.cols; ++c) {
				int intensity = hsvImg.at<cv::Vec3b>(r, c)[2];
				histogram[intensity]++;
			}
		}

		int min_val;
		float total = 0.0f;
		for (int i = 0; i < histogram.size(); ++i) {
			total += histogram[i];
			if (total > hsvImg.rows * hsvImg.cols * threshold) {
				min_val = i;
				break;
			}
		}

		int max_val;
		total = 0.0f;
		for (int i = histogram.size() - 1; i >= 0; --i) {
			total += histogram[i];
			if (total > hsvImg.rows * hsvImg.cols * threshold) {
				max_val = i;
				break;
			}
		}

		//std::cout << "Max val: " << max_val << ", Min val: " << min_val << std::endl;

		for (int r = 0; r < hsvImg.rows; ++r) {
			for (int c = 0; c < hsvImg.cols; ++c) {
				int intensity = hsvImg.at<cv::Vec3b>(r, c)[2];

				int new_intensity = std::max(0, std::min(255, (int)((float)(intensity - min_val) / (max_val - min_val) * 255.0)));
				cv::Vec3b new_val(hsvImg.at<cv::Vec3b>(r, c)[0], hsvImg.at<cv::Vec3b>(r, c)[1], new_intensity);
				hsvImg.at<cv::Vec3b>(r, c) = new_val;
			}
		}

		cv::Mat result;
		cv::cvtColor(hsvImg, result, cv::COLOR_HSV2BGR);

		return result;
	}
}