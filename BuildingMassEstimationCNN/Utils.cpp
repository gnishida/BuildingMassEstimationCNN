#include "Utils.h"
#include <regex>
#include <sstream>

namespace utils {

	const float FLOAT_TOL = 1e-6f;

	bool isNumber(const std::string& str) {
		std::regex e("^-?\\d*\\.?\\d+");
		if (std::regex_match(str, e)) return true;
		else return false;
	}

	float genRand() {
		return (float)(rand() % 1000) / 1000.0f;
	}

	float genRand(float v) {
		return genRand() * v;
	}

	float genRand(float a, float b) {
		return genRand(b - a) + a;
	}

	/**
	* Computes the intersection between two line segments on the XY plane.
	* Segments must intersect within their extents for the intersection to be valid. z coordinate is ignored.
	*
	* @param a one end of the first line
	* @param b another end of the first line
	* @param c one end of the second line
	* @param d another end of the second line
	* @param tab
	* @param tcd
	* @param segmentOnly
	* @param intPoint	the intersection
	* @return true if two lines intersect / false otherwise
	**/
	bool segmentSegmentIntersect(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, const glm::vec2& d, float *tab, float *tcd, bool segmentOnly, glm::vec2& intPoint) {
		glm::vec2 u = b - a;
		glm::vec2 v = d - c;

		if (glm::length(u) < FLOAT_TOL || glm::length(u) < FLOAT_TOL) {
			return false;
		}

		float numer = v.x*(c.y - a.y) + v.y*(a.x - c.x);
		float denom = u.y*v.x - u.x*v.y;

		if (denom == 0.0f)  {
			// they are parallel
			*tab = 0.0f;
			*tcd = 0.0f;
			return false;
		}

		float t0 = numer / denom;

		glm::vec2 ipt = a + t0*u;
		glm::vec2 tmp = ipt - c;
		float t1;
		if (glm::dot(tmp, v) > 0.0f) {
			t1 = tmp.length() / v.length();
		}
		else {
			t1 = -1.0f * tmp.length() / v.length();
		}

		//Check if intersection is within segments
		if (segmentOnly && !((t0 >= FLOAT_TOL) && (t0 <= 1.0f - FLOAT_TOL) && (t1 >= FLOAT_TOL) && (t1 <= 1.0f - FLOAT_TOL))){
			return false;
		}

		*tab = t0;
		*tcd = t1;
		glm::vec2 dirVec = b - a;

		intPoint = a + (*tab)*dirVec;

		return true;
	}

	void cleanEdges(std::vector<std::pair<glm::vec2, glm::vec2>>& edges, float maxLineGap, float theta) {
		float dotTol = cosf(theta);

		bool erased;
		while (true) {
			erased = false;
			for (int i = 0; i < edges.size() && !erased; ++i) {
				for (int j = i + 1; j < edges.size() && !erased; ++j) {
					if (glm::length(edges[i].first - edges[j].first) < 1 && glm::length(edges[i].second - edges[j].second) < 1) {
						edges.erase(edges.begin() + j);
						erased = true;
					}
					else if (glm::length(edges[i].first - edges[j].second) < 1 && glm::length(edges[i].second - edges[j].first) < 1) {
						edges.erase(edges.begin() + j);
						erased = true;
					}
					else {
						if (fabs(glm::dot(glm::normalize(edges[i].first - edges[i].second), glm::normalize(edges[j].first - edges[j].second))) > dotTol) {
							glm::vec2 norm1(-(edges[i].first - edges[i].second).y, (edges[i].first - edges[i].second).x);
							glm::vec2 norm2(-(edges[j].first - edges[j].second).y, (edges[j].first - edges[j].second).x);
							norm1 = glm::normalize(norm1);
							norm2 = glm::normalize(norm2);
							if (fabs(glm::dot(norm1, edges[j].first - edges[i].first)) < 3 || fabs(glm::dot(norm1, edges[j].second - edges[i].first)) < 3) {	// two lines are parallel and close!!
								if (fabs(edges[i].first.x - edges[i].second.x) > fabs(edges[i].first.y - edges[i].second.y)) {	// horizontal line
									// order each line
									if (edges[i].first.x > edges[i].second.x) {
										std::swap(edges[i].first.x, edges[i].second.x);
										std::swap(edges[i].first.y, edges[i].second.y);
									}
									if (edges[j].first.x > edges[j].second.x) {
										std::swap(edges[j].first.x, edges[j].second.x);
										std::swap(edges[j].first.y, edges[j].second.y);
									}

									// if two lines are apart, don't merge them.
									if (edges[i].first.x > edges[j].second.x) {
										if (glm::length(edges[i].first - edges[j].second) > maxLineGap) continue;
									}
									if (edges[j].first.x > edges[i].second.x) {
										if (glm::length(edges[i].second - edges[j].first) > maxLineGap) continue;
									}

									// merge two lines to the first line
									if (edges[j].first.x < edges[i].first.x) {
										edges[i].first = edges[j].first;
									}
									if (edges[j].second.x > edges[i].second.x) {
										edges[i].second = edges[j].second;
									}

									edges.erase(edges.begin() + j);
									erased = true;
								}
								else {	// vertical line
									// order each line
									if (edges[i].first.y > edges[i].second.y) {
										std::swap(edges[i].first.x, edges[i].second.x);
										std::swap(edges[i].first.y, edges[i].second.y);
									}
									if (edges[j].first.y > edges[j].second.y) {
										std::swap(edges[j].first.x, edges[j].second.x);
										std::swap(edges[j].first.y, edges[j].second.y);
									}

									// if two lines are apart, don't merge them.
									if (edges[i].first.y > edges[j].second.y) {
										if (glm::length(edges[i].first - edges[j].second) > maxLineGap) continue;
									}
									if (edges[j].first.y > edges[i].second.y) {
										if (glm::length(edges[i].second - edges[j].first) > maxLineGap) continue;
									}

									// merge two lines to the first line
									if (edges[j].first.y < edges[i].first.y) {
										edges[i].first = edges[j].first;
									}
									if (edges[j].second.y > edges[i].second.y) {
										edges[i].second = edges[j].second;
									}

									edges.erase(edges.begin() + j);
									erased = true;
								}
							}
						}
					}
				}
			}

			if (!erased) break;
		}
	}

	void cleanContours(std::vector<std::pair<glm::vec2, glm::vec2>>& edges, float maxLineGap, float theta) {
		float dotTol = cosf(theta);

		std::vector<std::pair<glm::vec2, glm::vec2>> contour;
		std::vector<bool> used;
		for (int i = 0; i < edges.size(); ++i) {
			used.push_back(false);
		}

		contour.push_back(edges[0]);
		used[0] = true;
		int numRemainedEdges = edges.size() - 1;

		while (numRemainedEdges > 0) {
			float min_dist = std::numeric_limits<float>::max();
			int next = -1;

			// find the next edge
			for (int i = 0; i < edges.size(); ++i) {
				if (used[i]) continue;

				float dist1 = glm::length(contour.back().second - edges[i].first);
				float dist2 = glm::length(contour.back().second - edges[i].second);

				if (dist1 < min_dist && dist1 < dist2) {
					min_dist = dist1;
					next = i;
				}
				else if (dist2 < min_dist) {
					min_dist = dist2;
					next = i;
				}
			}

			if (glm::length(edges[next].first - contour.back().second) > glm::length(edges[next].second - contour.back().second)) {
				std::swap(edges[next].first.x, edges[next].second.x);
				std::swap(edges[next].first.y, edges[next].second.y);
			}

			if (glm::length(edges[next].first - contour.back().second) <= 3) {
				glm::vec2 midPt = (contour.back().second + edges[next].first) * 0.5f;
				contour.back().second = midPt;
				edges[next].first = midPt;
			}
			else if (glm::length(edges[next].first - contour.back().second) <= 10) {
				glm::vec2 dir = glm::normalize(edges[next].first - contour.back().second);
				if (fabs(glm::dot(glm::normalize(contour.back().second - contour.back().first), dir)) > 0.8) {
					contour.back().second = edges[next].first;
				}
				else if (fabs(glm::dot(glm::normalize(edges[next].second - edges[next].first), dir)) > 0.8) {
					edges[next].first = contour.back().second;
				}
				else {
					contour.push_back(std::make_pair(contour.back().second, edges[next].first));
				}
			}
			else if (glm::length(edges[next].first - contour.back().second) <= maxLineGap) {
				glm::vec2 dir = glm::normalize(edges[next].first - contour.back().second);
				if (fabs(glm::dot(glm::normalize(contour.back().second - contour.back().first), dir)) > dotTol) {
					contour.back().second = edges[next].first;
				}
				else if (fabs(glm::dot(glm::normalize(edges[next].second - edges[next].first), dir)) > dotTol) {
					edges[next].first = contour.back().second;
				}
				else {
					contour.push_back(std::make_pair(contour.back().second, edges[next].first));
				}
			}
			else {
				used[next] = true;
				numRemainedEdges--;
				continue;
			}

			contour.push_back(edges[next]);
			used[next] = true;
			numRemainedEdges--;
		}

		if (glm::length(contour.front().first - contour.back().second) <= 3) {
			glm::vec2 midPt = (contour.front().first + contour.back().second) * 0.5f;
			contour.front().first = midPt;
			contour.back().second = midPt;
		}
		else if (glm::length(contour.front().first - contour.back().second) <= 10) {
			glm::vec2 dir = glm::normalize(contour.front().first - contour.back().second);
			if (fabs(glm::dot(glm::normalize(contour.back().second - contour.back().first), dir)) > 0.8) {
				contour.back().second = contour.front().first;
			}
			else if (fabs(glm::dot(glm::normalize(contour.front().second - contour.front().first), dir)) > 0.8) {
				contour.front().first = contour.back().second;
			}
			else {
				contour.push_back(std::make_pair(contour.back().second, contour.front().first));
			}
		}
		else if (glm::length(contour.front().first - contour.back().second) <= maxLineGap) {
			glm::vec2 dir = glm::normalize(contour.front().first - contour.back().second);
			if (fabs(glm::dot(glm::normalize(contour.back().second - contour.back().first), dir)) > dotTol) {
				contour.back().second = contour.front().first;
			}
			else if (fabs(glm::dot(glm::normalize(contour.front().second - contour.front().first), dir)) > dotTol) {
				contour.front().first = contour.back().second;
			}
			else {
				contour.push_back(std::make_pair(contour.back().second, contour.front().first));
			}
		}

		edges = contour;
	}
}