#include "BuildingMassEstimation.h"
#include <iostream>
#include "Rectangle.h"
#include "Camera.h"
#include <QFile>
#include <QTextStream>
#include "CVUtils.h"
#include <QDir>

namespace bme {

	const double M_PI = 3.14159265358;

	std::vector<float> estimate(int screen_width, int screen_height, std::vector<Stroke> silhouette, boost::shared_ptr<Classifier> classifier, std::vector<boost::shared_ptr<Regression>> regressions, std::vector<cga::Grammar>& grammars, bool automaticRecognition, int& grammarSnippetId, bool centering3D, bool meanSubtraction, int cameraType, float cameraDistanceBase, float cameraHeight, bool rotateContour, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int zrotMin, int zrotMax, int fovMin, int fovMax, bool tryMultiples, int numMultipleTries, float maxNoise, bool refinement, int maxIters, bool applyTexture) {

		////////////////////////////////////////////////////////////////////////////////
		// shift everything such that their center is aligned
		// compute the offset
		glm::vec2 offset = getOffset(silhouette, screen_width, screen_height);

		// shift the image and silhouette
#if 0
		shiftImageAndSilhouette(-offset.x, -offset.y, bgImage, silhouette);
		shiftImage(-offset.x / bgImageScale, -offset.y / bgImageScale, bgImageOrig);
#endif
		shiftSilhouette(-offset.x, -offset.y, silhouette);

		if (automaticRecognition) {
			grammarSnippetId = classify(screen_width, screen_height, silhouette, classifier);
		}

		return regress(screen_width, screen_height, silhouette, regressions[grammarSnippetId], grammars[grammarSnippetId], centering3D, meanSubtraction, cameraType, cameraDistanceBase, cameraHeight, xrotMin, xrotMax, yrotMin, yrotMax, zrotMin, zrotMax, fovMin, fovMax, tryMultiples, numMultipleTries, maxNoise, refinement, maxIters, applyTexture);
	}

	/**
	 * 指定されたシルエットに対してClassifierによりgrammar idを特定し、返却する。
	 */
	int classify(int screen_width, int screen_height, std::vector<Stroke>& silhouette, boost::shared_ptr<Classifier> classifier) {
		// scale the contour to 256x256 size
		glm::vec2 scale(256.0 / screen_width, 256.0 / screen_height);

		std::vector<Stroke> scaledSilhouette;
		for (auto stroke : silhouette) {
			Stroke s;
			s.start = glm::vec2(stroke.start.x * scale.x, stroke.start.y * scale.y);
			s.end = glm::vec2(stroke.end.x * scale.x, stroke.end.y * scale.y);
			scaledSilhouette.push_back(s);
		}

		// create input image
		cv::Mat input(256, 256, CV_8UC3, cv::Scalar(255, 255, 255));
		for (auto stroke : scaledSilhouette) {
			cv::line(input, cv::Point(stroke.start.x, stroke.start.y), cv::Point(stroke.end.x, stroke.end.y), cv::Scalar(0), 1, cv::LINE_AA);
		}

		std::vector<Prediction> prediction = classifier->Classify(input, 5);
		std::cout << "/////////////////////////////////////////////////////////////" << std::endl;
		std::cout << "// Classification" << std::endl;
		for (int i = 0; i < prediction.size(); ++i) {
			std::cout << prediction[i].first + 1 << ": " << prediction[i].second << std::endl;
		}
		std::cout << "/////////////////////////////////////////////////////////////" << std::endl;

		return prediction[0].first;
	}

	std::vector<float> regress(int screen_width, int screen_height, std::vector<Stroke>& silhouette, boost::shared_ptr<Regression> regression, cga::Grammar& grammar, bool centering3D, bool meanSubtraction, int cameraType, float cameraDistanceBase, float cameraHeight, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int zrotMin, int zrotMax, int fovMin, int fovMax, bool tryMultiples, int numMultipleTries, float maxNoise, bool refinement, int maxIters, bool applyTexture) {	
		// scale the contour to 128x128 size
		glm::vec2 scale(128.0 / screen_width, 128.0 / screen_height);

		std::vector<Stroke> scaledSilhouette;
		for (auto stroke : silhouette) {
			Stroke s;
			s.start = glm::vec2(stroke.start.x * scale.x, stroke.start.y * scale.y);
			s.end = glm::vec2(stroke.end.x * scale.x, stroke.end.y * scale.y);
			scaledSilhouette.push_back(s);
		}

		std::vector<float> best_params;
		std::vector<std::vector<float>> params_list;
		double best_dist = std::numeric_limits<double>::max();
		for (int multipleTryIter = 0; multipleTryIter < (tryMultiples ? numMultipleTries : 1); ++multipleTryIter) {
			// create input image
			cv::Mat input(128, 128, CV_8U, cv::Scalar(255));
			for (auto stroke : scaledSilhouette) {
				if (multipleTryIter > 0) {
					stroke.start.x += round(utils::genRand(-input.cols * maxNoise * 0.01f, input.cols * maxNoise * 0.01f));
					stroke.start.y += round(utils::genRand(-input.rows * maxNoise * 0.01f, input.rows * maxNoise * 0.01f));
					stroke.end.x += round(utils::genRand(-input.cols * maxNoise * 0.01f, input.cols * maxNoise * 0.01f));
					stroke.end.y += round(utils::genRand(-input.rows * maxNoise * 0.01f, input.rows * maxNoise * 0.01f));
				}
				cv::line(input, cv::Point(stroke.start.x, stroke.start.y), cv::Point(stroke.end.x, stroke.end.y), cv::Scalar(0), 1, cv::LINE_AA);
			}

			if (meanSubtraction) {
				cv::Mat meanImg = cv::imread("../models/mean.png");
				cv::cvtColor(meanImg, meanImg, cv::COLOR_BGR2GRAY);
				input -= meanImg;
			}

			// estimate parameters
			std::vector<float> params = regression->Predict(input);

			// rotation固定の場合には、ダミーでパラメータを入れちゃう
			if (xrotMin == xrotMax) {
				params.insert(params.begin() + 0, 0.5);
			}
			if (yrotMin == yrotMax) {
				params.insert(params.begin() + 1, 0.5);
			}
			if (zrotMin == zrotMax) {
				params.insert(params.begin() + 2, 0.5);
			}

			// FOV固定の場合には、ダミーでパラメータを入れちゃう
			if (fovMin == fovMax) {
				params.insert(params.begin() + 3, 0.5);
			}

			params_list.push_back(params);

			// Geometry faces
			std::vector<boost::shared_ptr<glutils::Face> > faces;
			double dist = distance(screen_width, screen_height, silhouette, grammar, centering3D, cameraType, cameraDistanceBase, cameraHeight, xrotMin, xrotMax, yrotMin, yrotMax, zrotMin, zrotMax, fovMin, fovMax, params, faces);

			if (multipleTryIter == 0) {
				// display the initial parameters
				std::cout << "Initial params:" << std::endl;
				utils::output_vector(params);

				// display the score
				std::cout << "Initial dist: " << dist << std::endl;
			}

			if (!tryMultiples || dist < best_dist) {
				best_dist = dist;
				best_params = params;
			}
		}

		std::vector<float> params_var;
		std::vector<float> params_mean;
		if (tryMultiples) {
			utils::computeMean(params_list, params_mean);
			utils::computeVariance(params_list, params_mean, params_var);

			// display the mean and stddev
			std::cout << "Mean (stddev):" << std::endl;
			for (int i = 0; i < params_mean.size(); ++i) {
				if (i > 0) std::cout << ", ";
				std::cout << params_mean[i] << " (" << sqrtf(params_var[i]) << ")";
			}
			std::cout << std::endl;
		}

		// display the best parameters
		std::cout << "Best initial params:" << std::endl;
		utils::output_vector(best_params);

		// display the score
		std::cout << "Best initial dist: " << best_dist << std::endl;

		if (refinement) {
			QFile file("refinement.txt");
			file.open(QIODevice::WriteOnly);
			QTextStream out(&file);

			printf("Refinement: ");

			std::vector<float> params = params_list[0];
			best_params = params_list[0];
			std::vector<boost::shared_ptr<glutils::Face> > faces;

			double dist = distance(screen_width, screen_height, silhouette, grammar, centering3D, cameraType, cameraDistanceBase, cameraHeight, xrotMin, xrotMax, yrotMin, yrotMax, zrotMin, zrotMax, fovMin, fovMax, params, faces);
			best_dist = dist;

			// if variance is not computed, set a default value
			if (params_var.size() == 0) {
				for (int i = 0; i < params.size(); ++i) {
					params_var.push_back(0.1);
					params_mean.push_back(best_params[i]);
				}
			}

			for (int iter = 0; iter < maxIters; ++iter) {
				// propse the next state
				std::vector<float> next_params = params;
				for (int i = 0; i < params.size(); ++i) {
					next_params[i] = utils::genNormal(params_mean[i], sqrtf(params_var[i]));
				}
				double next_dist = distance(screen_width, screen_height, silhouette, grammar, centering3D, cameraType, cameraDistanceBase, cameraHeight, xrotMin, xrotMax, yrotMin, yrotMax, zrotMin, zrotMax, fovMin, fovMax, next_params, faces);

				// update the best
				if (next_dist < best_dist) {
					// coordinate descent
					double delta = 0.01;
					for (int iter2 = 0; iter2 < 10000; ++iter2) {
						bool updated = false;

						for (int k = 0; k < next_params.size(); ++k) {
							// option 1
							std::vector<float> next_params1 = next_params;
							next_params1[k] += sqrt(params_var[k]) * delta;
							double next_dist1 = distance(screen_width, screen_height, silhouette, grammar, centering3D, cameraType, cameraDistanceBase, cameraHeight, xrotMin, xrotMax, yrotMin, yrotMax, zrotMin, zrotMax, fovMin, fovMax, next_params1, faces);

							// option 2
							std::vector<float> next_params2 = next_params;
							next_params2[k] -= sqrt(params_var[k]) * delta;
							double next_dist2 = distance(screen_width, screen_height, silhouette, grammar, centering3D, cameraType, cameraDistanceBase, cameraHeight, xrotMin, xrotMax, yrotMin, yrotMax, zrotMin, zrotMax, fovMin, fovMax, next_params2, faces);

							if (next_dist1 < next_dist && next_dist1 < next_dist2) {
								next_dist = next_dist1;
								next_params = next_params1;
								updated = true;
							}
							else if (next_dist2 < next_dist && next_dist2 < next_dist1) {
								next_dist = next_dist2;
								next_params = next_params2;
								updated = true;
							}
						}

						// stop when converged
						if (!updated) break;
					}

					best_dist = next_dist;
					best_params = next_params;
				}

				// write the params to the file
				out << next_dist;
				for (auto value : next_params) {
					out << "," << value;
				}
				out << "\n";

				// show the progress
				printf("\rRefinement: %d (best dist: %lf)        ", iter, best_dist);
				fflush(stdout);
			}

			file.close();
		}
		printf("\n");

		if (refinement) {
			std::cout << "Refined parameter:" << std::endl;
			utils::output_vector(best_params);
		}

		return best_params;
	}

	/**
	 * 指定されたカメラ、画像、geometryに基づいて、各faceの画像をrectifyしてテクスチャ画像を生成する。
	 */
	void generateTextures(const Camera& camera, const QImage& bgImage, int screen_width, int screen_height, std::vector<boost::shared_ptr<glutils::Face>>& faces) {
		// create texture folder
		if (!QDir("textures").exists()) QDir().mkdir("textures");

		float bgImageScale = std::min((float)screen_width / bgImage.width(), (float)screen_height / bgImage.height());

		// convert bgImage to cv::Mat object
		cv::Mat bgImageMat = cv::Mat(bgImage.height(), bgImage.width(), CV_8UC4, const_cast<uchar*>(bgImage.bits()), bgImage.bytesPerLine()).clone();

		// rectify the facade image
		for (int i = 0; i < faces.size(); ++i) {
			std::vector<cv::Point2f> pts;
			std::vector<glm::vec3> pts3d;
			for (int j = 0; j < faces[i]->vertices.size(); ++j) {
				if (j == 3 || j == 4) continue;

				pts3d.push_back(faces[i]->vertices[j].position);

				glm::vec4 result = camera.mvpMatrix * glm::vec4(faces[i]->vertices[j].position, 1);
				glm::vec2 pp((result.x / result.w + 1) * 0.5 * screen_width, (1 - result.y / result.w) * 0.5 * screen_height);
				pts.push_back(cv::Point2f(pp.x, pp.y));
			}

			// check if the face is visible
			if (pts3d.size() < 3) continue;
			glm::vec3 normal = glm::cross(pts3d[1] - pts3d[0], pts3d[2] - pts3d[1]);
			glm::vec4 normalInCameraCoordinates = camera.mvMatrix * glm::vec4(normal, 0);
			if (glm::dot(glm::vec3(normalInCameraCoordinates), glm::vec3(0, 0, 1)) <= 0) continue;

			if (pts.size() == 4) {
				std::vector<cv::Point2f> pts_scaled;
				for (auto pt : pts) {
					pts_scaled.push_back(cv::Point2f(pt.x / bgImageScale, pt.y / bgImageScale));
				}

				cv::Mat rectifiedImage = cvutils::rectify_image(bgImageMat, pts_scaled);
				rectifiedImage = cvutils::adjust_contrast(rectifiedImage);

				time_t now = clock();
				QString name = QString("textures/rectified_%1_%2.png").arg(now).arg(i);
				cv::imwrite(name.toUtf8().constData(), rectifiedImage);

				faces[i]->texture = name.toUtf8().constData();
			}
		}


	}

	/**
	* renderした画像とユーザ指定のシルエットとの差（距離）を返却する。
	* renderした画像がシルエットの中心からずれている場合は、シルエットを適宜ずらして距離を計算する。
	*/
	double distance(const std::vector<Stroke>& silhouette, const cv::Mat& renderedImg, int screen_width, int screen_height) {
		// compute the distance transform of the rendered image
		cv::Mat renderedImgGray;
		cv::cvtColor(renderedImg, renderedImgGray, cv::COLOR_BGRA2GRAY);
		cv::threshold(renderedImgGray, renderedImgGray, 250, 255, CV_THRESH_BINARY);
		cv::Mat renderedDistMap;
		cv::distanceTransform(renderedImgGray, renderedDistMap, CV_DIST_L2, 3);
		renderedDistMap.convertTo(renderedDistMap, CV_32F);

		// compute the offset of the rendered image
		glm::vec2 rendered_offset = bme::getOffset(renderedImg);

		// シルエットをずらす距離を計算
		glm::vec2 offset = rendered_offset - bme::getOffset(silhouette, screen_width, screen_height);

		// create an image of the target silhouette
		cv::Mat targetImg(screen_width, screen_height, CV_8U, cv::Scalar(255));
		for (auto line : silhouette) {
			cv::line(targetImg, cv::Point(line.start.x + offset.x, line.start.y + offset.y), cv::Point(line.end.x + offset.x, line.end.y + offset.y), cv::Scalar(0), 1);
		}

		cv::Mat targetDistMap;
		cv::distanceTransform(targetImg, targetDistMap, CV_DIST_L2, 3);
		//cv::imwrite("distance_targetDist.png", targetDistMap);

		// コストを計算
		double dist = 0.0;
		for (int r = 0; r < renderedDistMap.rows; ++r) {
			for (int c = 0; c < renderedDistMap.cols; ++c) {
				if (renderedDistMap.at<float>(r, c) == 0) {
					dist += targetDistMap.at<float>(r, c);
				}
				if (targetDistMap.at<float>(r, c) == 0) {
					dist += renderedDistMap.at<float>(r, c);
				}
			}
		}

		// シルエット側とrendered image側で2回距離を計算しているので、2で割る
		dist *= 0.5f;

		// normalize the distance
		dist /= silhouetteLength(silhouette);

		return dist;
	}

	/**
	* 指定されたカメラパラメータ、PMパラメータによる生成されたgeometryの結果と、ユーザ指定のsilhouetteの距離を返却する。
	* silhouetteの各頂点について、直近のgeometryのprojectされた頂点を探し、その距離にsilhouetteのエッジの長さを掛けたものの和を計算し、
	* silhouetteの全長で割ってnormalizeする。
	* なお、geometryをprojectした画像の中心とユーザ指定のsilhouetteの中心がずれている場合は、自動で調節する。
	*/
	double distance(int screen_width, int screen_height, const std::vector<Stroke>& silhouette, cga::Grammar& grammar, bool centering3D, int cameraType, float cameraDistanceBase, float cameraHeight, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int zrotMin, int zrotMax, int fovMin, int fovMax, const std::vector<float>& params, std::vector<boost::shared_ptr<glutils::Face>>& faces) {
		// setup the camera
		Camera camera;
		camera.xrot = (xrotMax - xrotMin) * params[0] + xrotMin;
		camera.yrot = (yrotMax - yrotMin) * params[1] + yrotMin;
		camera.zrot = (zrotMax - zrotMin) * params[2] + zrotMin;
		camera.fovy = (fovMax - fovMin) * params[3] + fovMin;
		float cameraDistance = cameraDistanceBase / tanf(camera.fovy * 0.5 / 180.0f * M_PI);

		if (cameraType == 0) { // street view
			camera.pos.x = 0;
			camera.pos.y = -cameraDistance * sinf(camera.xrot / 180.0f * M_PI) + cameraHeight * cosf(camera.xrot / 180.0f * M_PI);
			camera.pos.z = cameraDistance * cosf(camera.xrot / 180.0f * M_PI) + cameraHeight * sinf(camera.xrot / 180.0f * M_PI);
		}
		else { // aerial view
			camera.pos.x = 0;
			camera.pos.y = cameraHeight;
			camera.pos.z = cameraDistance;
		}
		camera.updatePMatrix(screen_width, screen_height);
		
		// geometryを生成する。
		std::vector<float> pm_params;
		for (int i = 4; i < params.size(); ++i) {
			pm_params.push_back(params[i]);
		}
		setupGeometry(grammar, centering3D, pm_params, faces);

		// geometryのprojectした画像のズレを計算する
		glutils::BoundingBox bbox;
		std::vector<glm::vec2> points;
		for (auto face : faces) {
			// check if the face is visible
			glm::vec3 normal = glm::vec3(camera.mvMatrix * glm::vec4(face->vertices[0].normal, 0));
			if (glm::dot(normal, glm::vec3(0, 0, 1)) < 0) continue;

			for (auto vert : face->vertices) {
				glm::vec2 pp = utils::projectPoint(screen_width, screen_height, vert.position, camera.mvpMatrix);
				bbox.addPoint(pp);
				points.push_back(pp);
			}
		}
		glm::vec2 rendered_offset(bbox.center().x - screen_width * 0.5, bbox.center().y - screen_height * 0.5);

		// silhouetteをずらすための距離を計算する
		glm::vec2 offset = rendered_offset - getOffset(silhouette, screen_width, screen_height);

		// compute the distance
		float dist = 0.0f;
		for (auto line : silhouette) {
			glm::vec2 p1 = line.start + offset;
			float min_dist1 = std::numeric_limits<float>::max();
			for (int i = 0; i < points.size(); ++i) {
				float dist = glm::length(points[i] - p1);
				if (dist < min_dist1) {
					min_dist1 = dist;
				}
			}

			glm::vec2 p2 = line.end + offset;
			float min_dist2 = std::numeric_limits<float>::max();
			for (int i = 0; i < points.size(); ++i) {
				float dist = glm::length(points[i] - p2);
				if (dist < min_dist2) {
					min_dist2 = dist;
				}
			}

			dist += (min_dist1 + min_dist2) * glm::length(line.end - line.start);
		}

		// normalize the dist by dividing it by the total length of silhouette
		dist /= silhouetteLength(silhouette);

		return dist;
	}

	void setupGeometry(cga::Grammar& grammar, bool centering3D, const std::vector<float>& pm_params, std::vector<boost::shared_ptr<glutils::Face>>& faces) {
		// setup CGA
		cga::CGA cga;
		cga.modelMat = glm::rotate(glm::mat4(), -(float)M_PI * 0.5f, glm::vec3(1, 0, 0));

		cga.setParamValues(grammar, pm_params);

		// set axiom
		boost::shared_ptr<cga::Shape> start = boost::shared_ptr<cga::Shape>(new cga::Rectangle("Start", "", glm::translate(glm::rotate(glm::mat4(), -3.141592f * 0.5f, glm::vec3(1, 0, 0)), glm::vec3(0, 0, 0)), glm::mat4(), 0, 0, glm::vec3(1, 1, 1)));
		cga.stack.push_back(start);

		// generate 3d model
		cga.derive(grammar, true);
		faces.clear();
		cga.generateGeometry(faces, centering3D);
	}

	/**
	 * 画像を(shift_x, shift_y)だけ移動させる。
	 */
	void shiftImage(int shift_x, int shift_y, QImage& image) {
		QImage newImage = image;
		QPainter painter(&image);
		painter.drawImage(shift_x, shift_y, newImage);
	}

	/**
	* シルエットを(shift_x, shift_y)だけ移動させる。
	*/
	void shiftSilhouette(int shift_x, int shift_y, std::vector<Stroke>& silhouette) {
		for (int i = 0; i < silhouette.size(); ++i) {
			silhouette[i].start.x += shift_x;
			silhouette[i].start.y += shift_y;
			silhouette[i].end.x += shift_x;
			silhouette[i].end.y += shift_y;
		}
	}

	void rotateImage(float theta, const glm::vec2& center, QImage& image) {
		cv::Mat img = cv::Mat(image.height(), image.width(), CV_8UC4, const_cast<uchar*>(image.bits()), image.bytesPerLine()).clone();

		cv::Mat rot_mat = cv::getRotationMatrix2D(cv::Point(center.x, center.y), theta / M_PI * 180.0, 1);
		cv::warpAffine(img, img, rot_mat, img.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255, 255));

		image = QImage(img.data, img.cols, img.rows, static_cast<int>(img.step), QImage::Format_RGB32).copy();
	}

	/**
	* シルエットをcenterを中心にtheta [rad]反時計方向に回転させる。
	* ただし、シルエットのY座標の方向が逆なので、実際には-theta [rad]だけ反時計方向に回転させればよい。
	*/
	void rotateSilhouette(float theta, const glm::vec2& center, std::vector<Stroke>& silhouette) {
		for (int i = 0; i < silhouette.size(); ++i) {
			glm::vec2 start = silhouette[i].start - center;
			glm::vec2 end = silhouette[i].end - center;

			silhouette[i].start.x = start.x * cosf(-theta) - start.y * sinf(-theta) + center.x;
			silhouette[i].start.y = start.x * sinf(-theta) + start.y * cosf(-theta) + center.y;
			silhouette[i].end.x = end.x * cosf(-theta) - end.y * sinf(-theta) + center.x;
			silhouette[i].end.y = end.x * sinf(-theta) + end.y * cosf(-theta) + center.y;
		}
	}

	/**
	* 画像が、キャンバスの中心からどの程度ずれているかを返却する。
	* 例えば、(100, 0)を返すということは、右に100pixずれているということ。
	* ただし、暗い部分を画像、白い部分を背景と見なす。
	*
	* @param img		画像（4チャンネル）
	* @return			ずれ
	*/
	glm::vec2 getOffset(const cv::Mat& img) {
		bool scan_r = false;
		int min_r = -1;
		int max_r = -1;
		for (int r = 0; r < img.rows; ++r) {
			for (int c = 0; c < img.cols; ++c) {
				cv::Vec4b color = img.at<cv::Vec4b>(r, c);

				if (color[0] < 100 && color[1] < 100 && color[2] < 100) {
					if (!scan_r) {
						min_r = r;
						max_r = r;
						scan_r = true;
					}
					else {
						max_r = r;
					}
				}
			}
		}

		bool scan_c = false;
		int min_c = -1;
		int max_c = -1;
		for (int c = 0; c < img.rows; ++c) {
			for (int r = 0; r < img.cols; ++r) {
				cv::Vec4b color = img.at<cv::Vec4b>(r, c);

				if (color[0] < 100 && color[1] < 100 && color[2] < 100) {
					if (!scan_c) {
						min_c = c;
						max_c = c;
						scan_c = true;
					}
					else {
						max_c = c;
					}
				}
			}
		}

		// if there is no image, cancel the translation.
		if (min_r == -1 || min_c == -1) return glm::vec2();

		// if the image is not strictly inside the canvas, cancel the translation.
		if (min_r == 0 || min_c == 0 || max_r == img.rows - 1 || max_c == img.cols - 1) return glm::vec2();

		int offset_c = (min_c + max_c) * 0.5 - img.cols * 0.5;
		int offset_r = (min_r + max_r) * 0.5 - img.rows * 0.5;

		return glm::vec2(offset_c, offset_r);
	}

	/**
	* シルエットが、キャンバスの中心からどの程度ずれているかを返却する。
	* 例えば、(100, 0)を返すということは、右に100pixずれているということ。
	* ただし、暗い部分を画像、白い部分を背景と見なす。
	*
	* @param silhouette	線分のリスト
	* @return			ずれ
	*/
	glm::vec2 getOffset(const std::vector<Stroke>& silhouette, int screen_width, int screen_height) {
		// compute the bbox
		glutils::BoundingBox bbox;
		for (auto stroke : silhouette) {
			bbox.addPoint(stroke.start);
			bbox.addPoint(stroke.end);
		}

		// compute the offset
		glm::vec2 offset;
		offset.x = bbox.center().x - screen_width * 0.5f;
		offset.y = bbox.center().y - screen_height * 0.5f;

		return offset;
	}

	/**
	 * シルエットのbounding boxを計算し、その中心を返却する。
	 */
	glm::vec2 getCenter(const std::vector<Stroke>& silhouette) {
		// compute the bbox
		glutils::BoundingBox bbox;
		for (auto stroke : silhouette) {
			bbox.addPoint(stroke.start);
			bbox.addPoint(stroke.end);
		}

		return glm::vec2(bbox.center());
	}

	/**
	* ユーザ指定のシルエットの傾き角度[rad]を返却する。
	* type=1のedgeのみについて、傾きの最小と最大のものを取得し、その平均を計算する。
	*/
	float getRotation(const std::vector<Stroke>& silhouette) {
		float min_theta = std::numeric_limits<float>::max();
		float max_theta = -std::numeric_limits<float>::max();
		int count = 0;
		for (auto line : silhouette) {
			if (line.type == Stroke::TYPE_VERTICAL) {
				// 下向きが正なので、y座標を反転させる
				float theta = atan2(-line.end.y + line.start.y, line.end.x - line.start.x);

				if (theta < 0) {
					theta += M_PI;
				}

				if (theta < min_theta) {
					min_theta = theta;
				}
				if (theta > max_theta) {
					max_theta = theta;
				}

				count++;
			}
		}

		if (count == 0) return 0;
		else {
			return (max_theta + min_theta) * 0.5f - M_PI * 0.5f;
		}
	}

	/**
	* シルエットの全長を返却する。
	*/
	float silhouetteLength(const std::vector<Stroke>& silhouette) {
		float silhouette_length = 0.0f;
		for (auto line : silhouette) {
			silhouette_length += glm::length(line.end - line.start);
		}

		return silhouette_length;
	}
}