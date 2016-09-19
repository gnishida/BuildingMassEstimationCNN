﻿#include "FacadeSegmentation.h"
#include "CVUtils.h"
#include "Utils.h"
#include <iostream>
#include <fstream>

namespace fs {

	/**
	* 2つの領域の類似度を返却する。
	*
	* @param R1		領域1 (1-channel image)
	* @param R2		領域2 (1-channel image)
	* @return			類似度
	*/
	float MI(const cv::Mat& R1, const cv::Mat& R2) {
		return expf(-cvutils::msd(R1, R2) * 0.001f);
	}

	/**
	* Facade画像のS_max(y)、h_max(y)を計算する。
	*
	* @param img		Facade画像 (1-channel image)
	* @param SV_max		S_max(y)
	* @param h_max		h_max(y)
	* @param h_range	range of h
	*/
	void computeSV(const cv::Mat& img, cv::Mat_<float>& SV_max, cv::Mat_<float>& h_max, const std::pair<int, int>& h_range) {
		SV_max = cv::Mat_<float>(img.rows, 1, 0.0f);
		h_max = cv::Mat_<float>(img.rows, 1, 0.0f);

		printf("computing SV:");
		for (int r = 0; r < img.rows; ++r) {
			printf("\rcomputing SV: r = %d/%d  ", r, img.rows);

			cv::Mat_<float> SV(img.rows, 1, 0.0f);

			for (int h = h_range.first; h <= h_range.second; ++h) {
				if (r - h < 0 || r + h >= img.rows) continue;

				cv::Mat R1 = img(cv::Rect(0, r, img.cols, h));
				cv::Mat R2 = img(cv::Rect(0, r - h, img.cols, h));
				SV(h, 0) = MI(R1, R2);

				if (SV(h, 0) > SV_max(r, 0)) {
					SV_max(r, 0) = SV(h, 0);
					h_max(r, 0) = h;
				}
			}
		}
		printf("\n");
	}

	/**
	* Facade画像のS_max(x)、w_max(x)を計算する。
	*
	* @param img		Facade画像 (1-channel image)
	* @param SH_max	S_max(x)
	* @param w_max		w_max(x)
	* @param w_range	range of w
	*/
	void computeSH(const cv::Mat& img, cv::Mat_<float>& SH_max, cv::Mat_<float>& w_max, const std::pair<int, int>& w_range) {
		SH_max = cv::Mat_<float>(1, img.cols, 0.0f);
		w_max = cv::Mat_<float>(1, img.cols, 0.0f);

		printf("computing SH:");
		for (int c = 0; c < img.cols; ++c) {
			printf("\rcomputing SH: c = %d/%d  ", c, img.cols);

			cv::Mat_<float> SH(1, img.cols, 0.0f);

			for (int w = w_range.first; w <= w_range.second; ++w) {
				if (c - w < 0 || c + w >= img.cols) continue;

				cv::Mat R1 = img(cv::Rect(c, 0, w, img.rows));
				cv::Mat R2 = img(cv::Rect(c - w, 0, w, img.rows));

				SH(0, w) = MI(R1, R2);

				if (SH(0, w) > SH_max(0, c)) {
					SH_max(0, c) = SH(0, w);
					w_max(0, c) = w;
				}
			}
		}
		printf("\n");
	}

	/**
	* imgから、Ver(y)とHor(x)を計算する。
	*
	* @param img		image (3-channel color image)
	* @param Ver		Ver(y)
	* @param Hor		Hor(x)
	*/
	void computeVerAndHor(const cv::Mat& img, cv::Mat_<float>& Ver, cv::Mat_<float>& Hor, float sigma) {
		cv::Mat grayImg;
		cv::cvtColor(img, grayImg, cv::COLOR_BGR2GRAY);

		float alpha = 0.9f;

		// compute hor(x,y) and ver(x,y) according to Equation (3)
		cv::Mat_<float> hor(grayImg.rows, grayImg.cols);
		cv::Mat_<float> ver(grayImg.rows, grayImg.cols);
		for (int r = 0; r < grayImg.rows; ++r) {
			for (int c = 0; c < grayImg.cols; ++c) {
				float dIdx;
				float dIdy;

				if (c == 0) {
					dIdx = grayImg.at<unsigned char>(r, c + 1) - grayImg.at<unsigned char>(r, c);
				}
				else if (c == grayImg.cols - 1) {
					dIdx = grayImg.at<unsigned char>(r, c) - grayImg.at<unsigned char>(r, c - 1);
				}
				else {
					dIdx = (grayImg.at<unsigned char>(r, c + 1) - grayImg.at<unsigned char>(r, c - 1)) / 2.0f;
				}

				if (r == 0) {
					dIdy = grayImg.at<unsigned char>(r + 1, c) - grayImg.at<unsigned char>(r, c);
				}
				else if (r == grayImg.rows - 1) {
					dIdy = grayImg.at<unsigned char>(r, c) - grayImg.at<unsigned char>(r - 1, c);
				}
				else {
					dIdy = (float)(grayImg.at<unsigned char>(r + 1, c) - grayImg.at<unsigned char>(r - 1, c)) / 2.0f;
				}

				hor(r, c) = std::max(dIdy * dIdy * (1.0f - alpha) - dIdx * dIdx * alpha, 0.0f);
				ver(r, c) = std::max(dIdx * dIdx * (1.0f - alpha) - dIdy * dIdy * alpha, 0.0f);
			}
		}

		// sum up the ver(x, y) horizontally and vertically, respectively
		cv::Mat ver_xtotal;
		cv::Mat ver_ytotal;
		cv::reduce(ver, ver_xtotal, 1, cv::REDUCE_SUM);
		cv::reduce(ver, ver_ytotal, 0, cv::REDUCE_SUM);

		// sum up the hor(x, y) horizontally and vertically, respectively
		cv::Mat hor_xtotal;
		cv::Mat hor_ytotal;
		cv::reduce(hor, hor_xtotal, 1, cv::REDUCE_SUM);
		cv::reduce(hor, hor_ytotal, 0, cv::REDUCE_SUM);

		// compute Ver(y) and Hor(x) according to Equation (4)
		Ver = cv::Mat_<float>(grayImg.rows, 1, 0.0f);
		Hor = cv::Mat_<float>(1, grayImg.cols, 0.0f);
		float beta = 0.1f;
		for (int r = 0; r < grayImg.rows; ++r) {
			for (int rr = 0; rr < grayImg.rows; ++rr) {
				Ver(r, 0) += (ver_xtotal.at<float>(rr, 0) - beta * hor_xtotal.at<float>(rr, 0)) * utils::gause(rr - r, sigma);
			}
		}
		for (int c = 0; c < grayImg.cols; ++c) {
			for (int cc = 0; cc < grayImg.cols; ++cc) {
				Hor(0, c) += (hor_ytotal.at<float>(0, cc) - beta * ver_ytotal.at<float>(0, cc)) * utils::gause(cc - c, sigma);
			}
		}
	}

	/**
	* tileを分割し、分割方向、分割タイプ、ボーダーからの距離を返却する。
	* 分割しない場合はfalseを返却する。
	*
	* @param tile					タイル画像 (3-channel image)
	* @param min_size
	* @param horizontal_edge_max	水平分割線に対する、エッジの強さの最小値
	* @param rect
	* @return						分割する場合はtrue / false otherwise
	*/
	bool subdivideTile(const cv::Mat& tile, const cv::Mat& edges, int min_size, int tile_margin, WindowPos& winpos) {
		if (tile.cols < min_size || tile.rows < min_size) return false;

		//cv::imwrite("tile.png", tile);

		cv::Mat grayTile;
		cv::cvtColor(tile, grayTile, CV_BGR2GRAY);

		// sum horizontally and vertically
		cv::Mat vertical_edges;
		cv::Mat horizontal_edges;
		cv::reduce(edges, vertical_edges, 0, CV_REDUCE_SUM, CV_32F);
		cv::reduce(edges, horizontal_edges, 1, CV_REDUCE_SUM, CV_32F);

		//cvutils::outputImageWithHorizontalAndVerticalGraph(tile, horizontal_edges, vertical_edges, "tile_ver_hor.png");

		cv::Mat vertical_edges_max;
		cv::Mat horizonta_edges_max;
		cv::reduce(vertical_edges, vertical_edges_max, 1, CV_REDUCE_MAX, CV_32F);
		cv::reduce(horizontal_edges, horizonta_edges_max, 0, CV_REDUCE_MAX, CV_32F);

		float vertical_edge_threshold = tile.rows * 0.2f * 255;
		float horizontal_edge_threshold = tile.cols * 0.2f * 255;

		// find the split closest to the boundary
		int x1 = -1;
		float prev_x1;
		for (int c = tile_margin; c < vertical_edges.cols - tile_margin; ++c) {
			if (x1 == -1) {
				if (vertical_edges.at<float>(0, c) >= vertical_edge_threshold) {
					x1 = c;
					prev_x1 = vertical_edges.at<float>(0, c);
				}
			}
			else if (vertical_edges.at<float>(0, c) > prev_x1) {
				x1 = c;
				prev_x1 = vertical_edges.at<float>(0, c);
			}
			else {
				break;
			}

		}
		int x2 = -1;
		float prev_x2;
		for (int c = vertical_edges.cols - tile_margin - 1; c >= tile_margin; --c) {
			if (x2 == -1) {
				if (vertical_edges.at<float>(0, c) >= vertical_edge_threshold) {
					x2 = c;
					prev_x2 = vertical_edges.at<float>(0, c);
				}
			}
			else if (vertical_edges.at<float>(0, c) > prev_x2) {
				x2 = c;
				prev_x2 = vertical_edges.at<float>(0, c);
			}
			else {
				break;
			}
		}
		if (x1 == -1 || x2 == -1 || x1 == x2) return false;

		int y1 = -1;
		float prev_y1;
		for (int r = tile_margin; r < horizontal_edges.rows - tile_margin; ++r) {
			if (y1 == -1) {
				if (horizontal_edges.at<float>(r, 0) >= horizontal_edge_threshold) {
					y1 = r;
					prev_y1 = horizontal_edges.at<float>(r, 0);
				}
			}
			else if (horizontal_edges.at<float>(r, 0) > prev_y1) {
				y1 = r;
				prev_y1 = horizontal_edges.at<float>(r, 0);
			}
			else {
				break;
			}
		}
		int y2 = -1;
		float prev_y2;
		for (int r = horizontal_edges.rows - tile_margin - 1; r >= tile_margin; --r) {
			if (y2 == -1) {
				if (horizontal_edges.at<float>(r, 0) >= horizontal_edge_threshold) {
					y2 = r;
					prev_y2 = horizontal_edges.at<float>(r, 0);
				}
			}
			else if (horizontal_edges.at<float>(r, 0) > prev_y2) {
				y2 = r;
				prev_y2 = horizontal_edges.at<float>(r, 0);
			}
			else {
				break;
			}
		}
		if (y1 == -1 || y2 == -1 || y1 == y2) return false;


		/*cv::Mat result = tile.clone();
		cv::rectangle(result, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(255, 0, 0), 1);
		cv::imwrite("tile_window.png", result);*/

		winpos = WindowPos(x1, y1, tile.cols - 1 - x2, tile.rows - 1 - y2);

		return true;
	}

	/**
	* Ver(y)の極小値をsplit lineの候補とし、S_max(y)に基づいて最適なsplit lineの組み合わせを探す。
	*
	* @param Ver			Ver(y)
	* @param min_interval	minimum floor height
	* @param max_interval	maximum floor height
	* @param y_split		最適なsplit lineの組み合わせ
	*/
	void findBestHorizontalSplitLines(const cv::Mat& img, const cv::Mat_<float>& Ver, float min_interval, float max_interval, std::vector<int>& y_split) {
		y_split.clear();

		std::vector<int> y_candidates = cvutils::getPeak(Ver, false, 1, cvutils::LOCAL_MINIMUM, 1);
		y_candidates.insert(y_candidates.begin(), 0);
		y_candidates.push_back(img.rows - 1);

		std::vector<std::vector<float>> costs;
		std::vector<std::vector<int>> indices;
		std::vector<std::vector<int>> nums;

		// 最初の行のコストを初期化
		{
			costs.push_back(std::vector<float>(y_candidates.size(), std::numeric_limits<float>::max()));
			indices.push_back(std::vector<int>(y_candidates.size(), -1));
			nums.push_back(std::vector<int>(y_candidates.size(), 0));
			costs[0][0] = 0;
			indices[0][0] = 0;
			nums[0][0] = 0;
		}

		// 2行目以降について、Dynamic Programmingで最小コストを計算
		for (int i = 1;; ++i) {
			costs.push_back(std::vector<float>(y_candidates.size(), std::numeric_limits<float>::max()));
			indices.push_back(std::vector<int>(y_candidates.size(), -1));
			nums.push_back(std::vector<int>(y_candidates.size(), 0));

			for (int k = 0; k < y_candidates.size() - 1; ++k) {
				bool found = false;

				for (int j = k + 1; j < y_candidates.size() - 1; ++j) {
					if (indices[i - 1][k] == -1) continue;

					if (y_candidates[j] - y_candidates[k] < min_interval) continue;
					if (found && y_candidates[j] - y_candidates[k] > max_interval) continue;

					found = true;
					float new_cost = costs[i - 1][k] + Ver(y_candidates[j], 0);
					if (new_cost / (nums[i - 1][k] + 1) < costs[i][j] / nums[i][j]) {
						costs[i][j] = new_cost;
						indices[i][j] = k;
						nums[i][j] = nums[i - 1][k] + 1;
					}
				}
			}

			for (int k = 0; k < y_candidates.size(); ++k) {
				if (indices[i - 1][k] == -1) continue;

				if (y_candidates.back() - y_candidates[k] > max_interval) continue;

				if (costs[i - 1][k] / nums[i - 1][k] < costs[i].back() / nums[i].back()) {
					costs[i].back() = costs[i - 1][k];
					indices[i].back() = k;
					nums[i].back() = nums[i - 1][k];
				}
			}

			// 最後のy以外がすべて-1なら、終了
			bool updated = false;
			for (int j = 0; j < indices[i].size() - 1; ++j) {
				if (indices[i][j] != -1) updated = true;
			}
			if (!updated) break;
		}

		// y_splitに、最適解を格納する
		y_split.push_back(y_candidates.back());
		int prev_index = y_candidates.size() - 1;
		for (int i = indices.size() - 1; i >= 1; --i) {
			int index = indices[i][prev_index];
			if (index == prev_index) continue;

			y_split.insert(y_split.begin(), y_candidates[index]);
			prev_index = index;
		}
	}

	/**
	* Hor(x)の極小値をsplit lineの候補とし、S_max(x)に基づいて最適なsplit lineの組み合わせを探す。
	*
	* @param Hor			Hor(x)
	* @param min_interval	minimum tile width
	* @param max_interval	maximum tile width
	* @param x_split		最適なsplit lineの組み合わせ
	*/
	void findBestVerticalSplitLines(const cv::Mat& img, const cv::Mat_<float>& Hor, float min_interval, float max_interval, std::vector<int>& x_split) {
		x_split.clear();

		std::vector<int> x_candidates = cvutils::getPeak(Hor, false, 1, cvutils::LOCAL_MINIMUM, 1);
		x_candidates.insert(x_candidates.begin(), 0);
		x_candidates.push_back(img.cols - 1);

		std::vector<std::vector<float>> costs;
		std::vector<std::vector<int>> indices;
		std::vector<std::vector<int>> nums;

		// 最初の列のコストを初期化
		{
			costs.push_back(std::vector<float>(x_candidates.size(), std::numeric_limits<float>::max()));
			indices.push_back(std::vector<int>(x_candidates.size(), -1));
			nums.push_back(std::vector<int>(x_candidates.size(), 0));
			costs[0][0] = 0;
			indices[0][0] = 0;
			nums[0][0] = 0;
		}

		// 2列目以降について、Dynamic Programmingで最小コストを計算
		for (int i = 1;; ++i) {
			costs.push_back(std::vector<float>(x_candidates.size(), std::numeric_limits<float>::max()));
			indices.push_back(std::vector<int>(x_candidates.size(), -1));
			nums.push_back(std::vector<int>(x_candidates.size(), 0));

			for (int k = 0; k < x_candidates.size() - 1; ++k) {
				bool found = false;

				for (int j = k + 1; j < x_candidates.size() - 1; ++j) {
					if (indices[i - 1][k] == -1) continue;

					if (x_candidates[j] - x_candidates[k] < min_interval) continue;
					if (found && x_candidates[j] - x_candidates[k] > max_interval) continue;

					found = true;
					float new_cost = costs[i - 1][k] + Hor(0, x_candidates[j]);
					if (new_cost / (nums[i - 1][k] + 1) < costs[i][j] / nums[i][j]) {
						costs[i][j] = new_cost;
						indices[i][j] = k;
						nums[i][j] = nums[i - 1][k] + 1;
					}
				}
			}

			for (int k = 0; k < x_candidates.size(); ++k) {
				if (indices[i - 1][k] == -1) continue;

				if (x_candidates.back() - x_candidates[k] > max_interval) continue;

				if (costs[i - 1][k] / nums[i - 1][k] < costs[i].back() / nums[i].back()) {
					costs[i].back() = costs[i - 1][k];
					indices[i].back() = k;
					nums[i].back() = nums[i - 1][k];
				}
			}

			// 最後のx以外がすべて-1なら、終了
			bool updated = false;
			for (int j = 0; j < indices[i].size() - 1; ++j) {
				if (indices[i][j] != -1) updated = true;
			}
			if (!updated) break;
		}

		// x_splitに、最適解を格納する
		x_split.push_back(x_candidates.back());
		int prev_index = x_candidates.size() - 1;
		for (int i = indices.size() - 1; i >= 1; --i) {
			int index = indices[i][prev_index];
			if (index == prev_index) continue;

			x_split.insert(x_split.begin(), x_candidates[index]);
			prev_index = index;
		}
	}

	/**
	* 与えられた関数の極小値を使ってsplit lineを決定する。
	*/
	void getSplitLines(const cv::Mat_<float>& val, std::vector<float>& split_positions) {
		cv::Mat_<float> mat = val.clone();
		if (mat.rows == 1) {
			mat = mat.t();
		}

		for (int r = 0; r < mat.rows; ++r) {
			if (cvutils::isLocalMinimum(mat, r, 1)) {
				split_positions.push_back(r);
			}
		}

		// 両端処理
		if (split_positions.size() == 0) {
			split_positions.insert(split_positions.begin(), 0);
		}
		else if (split_positions[0] > 0) {
			if (split_positions[0] < 5) {
				split_positions[0] = 0;
			}
			else {
				split_positions.insert(split_positions.begin(), 0);
			}
		}

		if (split_positions.back() < mat.rows) {
			if (split_positions.back() >= mat.rows - 5) {
				split_positions.back() = mat.rows;
			}
			else {
				split_positions.push_back(mat.rows);
			}
		}
	}

	void refineSplitLines(std::vector<float>& split_positions) {
		// 間隔が狭すぎる場合は、分割して隣接領域にマージする
		while (true) {
			// 領域の幅を計算する
			cv::Mat intervals(split_positions.size() - 1, 1, CV_32F);
			for (int i = 0; i < split_positions.size() - 1; ++i) {
				intervals.at<float>(i, 0) = split_positions[i + 1] - split_positions[i];
			}
			float avg_interval = cvutils::getMostPopularValue(intervals, 3, 3);

			bool updated = false;
			for (int i = 0; i < split_positions.size() - 1;) {
				if (split_positions[i + 1] - split_positions[i] < avg_interval * 0.5) {
					if (i == 0) {
						split_positions.erase(split_positions.begin() + 1);
					}
					else if (i == split_positions.size() - 2) {
						split_positions.erase(split_positions.begin() + split_positions.size() - 2);
					}
					else {
						split_positions[i] = (split_positions[i + 1] + split_positions[i]) * 0.5;
						split_positions.erase(split_positions.begin() + i + 1);
					}
					updated = true;
				}
				else {
					i++;
				}
			}

			if (!updated) break;
		}
	}

	void refine(std::vector<float>& y_split, std::vector<float>& x_split, std::vector<std::vector<WindowPos>>& winpos) {
		// 各フロアの窓の数をカウントする
		std::vector<int> win_per_row(y_split.size() - 1, 0);
		int max_win_per_row = 0;
		for (int i = 0; i < y_split.size() - 1; ++i) {
			for (int j = 0; j < x_split.size() - 1; ++j) {
				if (winpos[i][j].valid) {
					win_per_row[i]++;
				}
			}
			if (win_per_row[i] > max_win_per_row) {
				max_win_per_row = win_per_row[i];
			}
		}

		// 各カラムの窓の数をカウントする
		std::vector<int> win_per_col(x_split.size() - 1, 0);
		int max_win_per_col = 0;
		for (int j = 0; j < x_split.size() - 1; ++j) {
			for (int i = 0; i < y_split.size() - 1; ++i) {
				if (winpos[i][j].valid) {
					win_per_col[j]++;
				}
			}
			if (win_per_col[j] > max_win_per_col) {
				max_win_per_col = win_per_col[j];
			}
		}

		// 壁のフロアかどうかチェックする
		std::vector<bool> is_wall_row(y_split.size() - 1, false);
		for (int i = 0; i < y_split.size() - 1; ++i) {
			if (win_per_row[i] < max_win_per_row * 0.2) {
				is_wall_row[i] = true;

				for (int j = 0; j < x_split.size() - 1; ++j) {
					winpos[i][j].valid = false;
				}
			}
		}

		// 壁のカラムかどうかチェックする
		std::vector<bool> is_wall_col(x_split.size() - 1, false);
		for (int j = 0; j < x_split.size() - 1; ++j) {
			if (win_per_col[j] < max_win_per_col * 0.2) {
				is_wall_col[j] = true;

				for (int i = 0; i < y_split.size() - 1; ++i) {
					winpos[i][j].valid = false;
				}
			}
		}

		// 窓のないフロアが連続している場合は、連結する
		for (int i = 0; i < is_wall_row.size() - 1;) {
			if (is_wall_row[i] && is_wall_row[i + 1]) {
				is_wall_row.erase(is_wall_row.begin() + i + 1);
				y_split.erase(y_split.begin() + i + 1);
				winpos.erase(winpos.begin() + i + 1);
			}
			else {
				i++;
			}
		}

		// 窓のないカラムが連続している場合は、連結する
		for (int j = 0; j < is_wall_col.size() - 1;) {
			if (is_wall_col[j] && is_wall_col[j + 1]) {
				is_wall_col.erase(is_wall_col.begin() + j + 1);
				x_split.erase(x_split.begin() + j + 1);
				for (int i = 0; i < y_split.size() - 1; ++i) {
					winpos[i].erase(winpos[i].begin() + j + 1);
				}
			}
			else {
				j++;
			}
		}
	}

	void align(const std::vector<float>& y_split, const std::vector<float>& x_split, std::vector<std::vector<WindowPos>>& winpos) {
		// 窓のX座標をvoteする
		for (int j = 0; j < x_split.size() - 1; ++j) {
			int max_left, max_right;

			// voteする
			std::vector<float> histogram1(x_split[j + 1] - x_split[j], 0);
			std::vector<float> histogram2(x_split[j + 1] - x_split[j], 0);
			int count = 0;
			for (int i = 0; i < y_split.size() - 1; ++i) {
				if (!winpos[i][j].valid) continue;

				count++;
				for (int c = 0; c < histogram1.size(); ++c) {
					histogram1[c] += utils::gause(winpos[i][j].left - c, 2);
					histogram2[c] += utils::gause(winpos[i][j].right - c, 2);
				}
			}

			if (count == 0) continue;

			// max voteを探す
			float max_val1 = 0.0f;
			float max_val2 = 0.0f;
			for (int c = 0; c < histogram1.size(); ++c) {
				if (histogram1[c] > max_val1) {
					max_val1 = histogram1[c];
					max_left = c;
				}
				if (histogram2[c] > max_val2) {
					max_val2 = histogram2[c];
					max_right = c;
				}
			}

			// 全てのフロアの窓のX座標をそろえる
			for (int r = 0; r < y_split.size() - 1; ++r) {
				if (!winpos[r][j].valid) continue;

				if (r == 0 || r == y_split.size() - 1) {
					if (abs(winpos[r][j].left - max_left) < 5) {
						winpos[r][j].left = max_left;
					}
					if (abs(winpos[r][j].right - max_right) < 5) {
						winpos[r][j].right = max_right;
					}
				}
				else {
					winpos[r][j].left = max_left;
					winpos[r][j].right = max_right;
				}
			}
		}

		// 窓のY座標をvoteする
		for (int i = 0; i < y_split.size() - 1; ++i) {
			int max_top, max_bottom;

			// voteする
			std::vector<float> histogram1(y_split[i + 1] - y_split[i], 0);
			std::vector<float> histogram2(y_split[i + 1] - y_split[i], 0);
			int count = 0;
			for (int j = 0; j < x_split.size() - 1; ++j) {
				if (!winpos[i][j].valid) continue;

				count++;
				for (int r = 0; r < histogram1.size(); ++r) {
					histogram1[r] += utils::gause(winpos[i][j].top - r, 2);
					histogram2[r] += utils::gause(winpos[i][j].bottom - r, 2);
				}
			}

			if (count == 0) continue;

			// max voteを探す
			float max_val1 = 0.0f;
			float max_val2 = 0.0f;
			for (int r = 0; r < histogram1.size(); ++r) {
				if (histogram1[r] > max_val1) {
					max_val1 = histogram1[r];
					max_top = r;
				}
				if (histogram2[r] > max_val2) {
					max_val2 = histogram2[r];
					max_bottom = r;
				}
			}

			// 全てのカラムの窓のY座標をそろえる
			for (int c = 0; c < x_split.size() - 1; ++c) {
				if (!winpos[i][c].valid) continue;

				winpos[i][c].top = max_top;
				winpos[i][c].bottom = max_bottom;
			}
		}

	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// visualization

	void outputFacadeStructure(const cv::Mat& img, const std::vector<float>& y_set, const std::vector<float>& x_set, const std::string& filename, int lineWidth) {
		cv::Mat result = img.clone();

		for (int i = 0; i < y_set.size(); ++i) {
			cv::line(result, cv::Point(0, y_set[i]), cv::Point(img.cols, y_set[i]), cv::Scalar(0, 0, 255), lineWidth);
		}
		for (int i = 0; i < x_set.size(); ++i) {
			cv::line(result, cv::Point(x_set[i], 0), cv::Point(x_set[i], img.rows), cv::Scalar(0, 0, 255), lineWidth);
		}
		cv::imwrite(filename, result);
	}

	void outputFacadeAndWindows(const cv::Mat& img, const std::vector<float>& y_split, const std::vector<float>& x_split, const std::vector<std::vector<WindowPos>>& winpos, const std::string& filename) {
		cv::Mat result = img.clone();
		for (int i = 0; i < y_split.size(); ++i) {
			cv::line(result, cv::Point(0, y_split[i]), cv::Point(result.cols - 1, y_split[i]), cv::Scalar(0, 0, 255), 1);
		}
		for (int i = 0; i < x_split.size(); ++i) {
			cv::line(result, cv::Point(x_split[i], 0), cv::Point(x_split[i], result.rows - 1), cv::Scalar(0, 0, 255), 1);
		}
		for (int i = 0; i < y_split.size() - 1; ++i) {
			for (int j = 0; j < x_split.size() - 1; ++j) {
				if (winpos[i][j].valid) {
					int x1 = x_split[j] + winpos[i][j].left;
					int y1 = y_split[i] + winpos[i][j].top;
					int x2 = x_split[j + 1] - 1 - winpos[i][j].right;
					int y2 = y_split[i + 1] - 1 - winpos[i][j].bottom;
					cv::rectangle(result, cv::Rect(x1, y1, x2 - x1 + 1, y2 - y1 + 1), cv::Scalar(255, 0, 0), 1);
				}
			}
		}
		cv::imwrite(filename, result);
	}

}