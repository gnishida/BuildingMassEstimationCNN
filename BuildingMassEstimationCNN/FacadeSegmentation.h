#pragma once

#include <vector>
#include <opencv2/opencv.hpp>

namespace fs {

	class WindowPos {
	public:
		int left;
		int right;
		int top;
		int bottom;
		bool valid;

	public:
		WindowPos() : valid(false) {}
		WindowPos(int left, int top, int right, int bottom) : left(left), top(top), right(right), bottom(bottom), valid(true) {}
	};

	float MI(const cv::Mat& R1, const cv::Mat& R2);
	void computeSV(const cv::Mat& img, cv::Mat_<float>& SV_max, cv::Mat_<float>& h_max, const std::pair<int, int>& h_range);
	void computeSH(const cv::Mat& img, cv::Mat_<float>& SH_max, cv::Mat_<float>& w_max, const std::pair<int, int>& w_range);
	void computeVerAndHor(const cv::Mat& img, cv::Mat_<float>& Ver, cv::Mat_<float>& Hor, float sigma);
	bool subdivideTile(const cv::Mat& tile, const cv::Mat& edges, int min_size, int tile_margin, WindowPos& winpos);
	void findBestHorizontalSplitLines(const cv::Mat& img, const cv::Mat_<float>& Ver, float min_interval, float max_interval, std::vector<int>& y_split);
	void findBestVerticalSplitLines(const cv::Mat& img, const cv::Mat_<float>& Hor, float min_interval, float max_interval, std::vector<int>& x_split);
	void getSplitLines(const cv::Mat_<float>& mat, std::vector<float>& split_positions);
	void refineSplitLines(std::vector<float>& split_positions);
	void refine(std::vector<float>& y_split, std::vector<float>& x_split, std::vector<std::vector<WindowPos>>& winpos);
	void align(const std::vector<float>& y_split, const std::vector<float>& x_split, std::vector<std::vector<WindowPos>>& winpos);


	// visualization
	void outputFacadeStructure(const cv::Mat& img, const std::vector<float>& y_set, const std::vector<float>& x_set, const std::string& filename, int lineWidth);
	void outputFacadeAndWindows(const cv::Mat& img, const std::vector<float>& y_split, const std::vector<float>& x_split, const std::vector<std::vector<WindowPos>>& winpos, const std::string& filename);

}