﻿#include <iostream>
#include "GLWidget3D.h"
#include "MainWindow.h"
#include <GL/GLU.h>
#include <QDir>
#include <QFileInfoList>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "GrammarParser.h"
#include "Rectangle.h"
#include "GLUtils.h"
#include <opencv2/calib3d.hpp>
#include "CameraCalibration.h"
#include <map>
#include "CVUtils.h"
#include "Utils.h"
#include <QFile>
#include <QTextStream>

#ifndef M_PI
#define	M_PI	3.141592653
#endif

#ifndef SQR
#define SQR(x)		((x) * (x))
#endif

GLWidget3D::GLWidget3D(QWidget *parent) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent) {
	mainWin = (MainWindow*)parent;
	dragging = false;
	ctrlPressed = false;
	shiftPressed = false;

	opacityOfBackground = 0.5f;
	estimation_error = 0;

	// This is necessary to prevent the screen overdrawn by OpenGL
	setAutoFillBackground(false);

	// 光源位置をセット
	// ShadowMappingは平行光源を使っている。この位置から原点方向を平行光源の方向とする。
	light_dir = glm::normalize(glm::vec3(-4, -5, -8));

	// シャドウマップ用のmodel/view/projection行列を作成
	glm::mat4 light_pMatrix = glm::ortho<float>(-50, 50, -50, 50, 0.1, 200);
	glm::mat4 light_mvMatrix = glm::lookAt(-light_dir * 50.0f, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	light_mvpMatrix = light_pMatrix * light_mvMatrix;

	//classifier = boost::shared_ptr<Classifier>(new Classifier("../models/deploy.prototxt", "../models/contour_iter_20000.caffemodel", "../models/contour_mean.binaryproto"));

	// caffe modelを読み込む
	{
		regressions.push_back(boost::shared_ptr<Regression>(new Regression("..\\models\\deploy_01.prototxt", "..\\models\\train_01_iter_240000.caffemodel")));
		regressions.push_back(boost::shared_ptr<Regression>(new Regression("..\\models\\deploy_02.prototxt", "..\\models\\train_02_iter_240000.caffemodel")));
		regressions.push_back(boost::shared_ptr<Regression>(new Regression("..\\models\\deploy_03.prototxt", "..\\models\\train_03_iter_240000.caffemodel")));
		regressions.push_back(boost::shared_ptr<Regression>(new Regression("..\\models\\deploy_04.prototxt", "..\\models\\train_04_iter_240000.caffemodel")));
		//regressions.push_back(boost::shared_ptr<Regression>(new Regression("..\\models\\deploy_05.prototxt", "..\\models\\train_05_iter_240000.caffemodel")));
	}

	// Grammarを読み込む
	{
		grammars.resize(4);
		cga::parseGrammar("..\\cga\\contour_01b.xml", grammars[0]);
		cga::parseGrammar("..\\cga\\contour_02b.xml", grammars[1]);
		cga::parseGrammar("..\\cga\\contour_03b.xml", grammars[2]);
		cga::parseGrammar("..\\cga\\contour_04b.xml", grammars[3]);
		//cga::parseGrammar("..\\cga\\contour_05.xml", grammars[4]);
	}
}

/**
 * Clear the canvas.
 */
void GLWidget3D::clearSilhouette() {
	silhouette.clear();

	update();
}

void GLWidget3D::clearBackground() {
	bgImage = QImage(width(), height(), QImage::Format_RGB32);
	bgImageOrig = bgImage;
	mainWin->setWindowTitle("Photo to 3D");

	update();
}

void GLWidget3D::clearGeometry() {
	renderManager.removeObjects();
	renderManager.updateShadowMap(this, light_dir, light_mvpMatrix);
	update();
}

void GLWidget3D::loadContour(const std::string& filename) {
	silhouette.clear();

	std::ifstream in(filename);
	
	while (!in.eof()) {
		Stroke stroke;
		if (!(in >> stroke.start.x >> stroke.start.y >> stroke.end.x >> stroke.end.y >> stroke.type)) break;
		
		silhouette.push_back(stroke);
	}

	update();
}

void GLWidget3D::saveContour(const std::string& filename) {
	std::ofstream out(filename);
	for (auto stroke : silhouette) {
		out << stroke.start.x << "\t" << stroke.start.y << "\t" << stroke.end.x << "\t" << stroke.end.y << "\n";
	}
	out.close();
}

/**
* Load an image from a file, and display options order by their probabilities.
* This is for test usage.
*/
void GLWidget3D::loadImage(const std::string& filename) {
	bgImageOrig.load(filename.c_str());

	float scale = std::min((float)width() / bgImageOrig.width(), (float)height() / bgImageOrig.height());
	QImage newImage = bgImageOrig.scaled(bgImageOrig.width() * scale, bgImageOrig.height() * scale);

	bgImage = QImage(width(), height(), QImage::Format_RGB32);
	QPainter painter(&bgImage);
	painter.drawImage((width() - newImage.width()) / 2, (height() - newImage.height()) / 2, newImage);
	
	opacityOfBackground = 0.5f;

	mainWin->setWindowTitle(QString("Photo to 3D - ") + filename.c_str());

	update();
}

/**
 * Load a grammar from a file and generate a 3d geometry.
 * This is only for test usage.
 */
void GLWidget3D::loadCGA(const std::string& filename) {
	cga::Grammar grammar;
	cga::parseGrammar(filename.c_str(), grammar);

	cga::CGA cga;
	cga.modelMat = glm::rotate(glm::mat4(), -3.1415926f * 0.5f, glm::vec3(1, 0, 0));

	renderManager.removeObjects();

	// set axiom
	boost::shared_ptr<cga::Shape> start = boost::shared_ptr<cga::Shape>(new cga::Rectangle("Start", "", glm::translate(glm::rotate(glm::mat4(), -3.141592f * 0.5f, glm::vec3(1, 0, 0)), glm::vec3(0, 0, 0)), glm::mat4(), 0, 0, glm::vec3(1, 1, 1)));
	cga.stack.push_back(start);

	// generate 3d model
	cga.derive(grammar, true);
	std::vector<boost::shared_ptr<glutils::Face> > faces;
	cga.generateGeometry(faces, true);
	renderManager.addFaces(faces, true);

	renderManager.updateShadowMap(this, light_dir, light_mvpMatrix);

	// render 2d image
	render();
}

void GLWidget3D::undo() {
	if (silhouette.size() > 0) {
		silhouette.pop_back();
		update();
	}
}

/**
 * Use the silhouette as an input to the pretrained network, and obtain the probabilities as output.
 * Then, display the options ordered by the probabilities.
 */
void GLWidget3D::parameterEstimation(bool automaticRecognition, int grammarSnippetId, bool centering3D, bool meanSubtraction, int cameraType, float cameraDistanceBase, float cameraHeight, bool rotateContour, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int zrotMin, int zrotMax, int fovMin, int fovMax, bool tryMultiples, int numMultipleTries, float maxNoise, bool refinement, bool refineFromBest, int maxIters, bool applyTexture) {
	// adjust the original background image such that the ratio of width to height is equal to the ratio of the window
	float bgImageScale = std::min((float)width() / bgImageOrig.width(), (float)height() / bgImageOrig.height());
	resizeImageCanvasSize(bgImageOrig, width() / bgImageScale, height() / bgImageScale);

	////////////////////////////////////////////////////////////////////////////////
	// rotate everything such that the silhouette stands upright
	if (rotateContour) {
		// compute the rotation
		float rot = getRotation(silhouette);

		// compute the center
		glm::vec2 center = getCenter(silhouette);

		rotateImage(-rot, center, bgImage);
		rotateImage(-rot, center / bgImageScale, bgImageOrig);
		rotateSilhouette(-rot, center, silhouette);
	}

	////////////////////////////////////////////////////////////////////////////////
	// shift everything such that their center is aligned
	// compute the offset
	glm::vec2 offset = getOffset(silhouette);

	// shift the image and silhouette
	shiftImageAndSilhouette(-offset.x, -offset.y, bgImage, silhouette);
	shiftImage(-offset.x / bgImageScale, -offset.y / bgImageScale, bgImageOrig);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// classification
	if (automaticRecognition) {
		// scale the contour to 128x128 size
		glm::vec2 scale(256.0 / width(), 256.0 / height());

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

		grammarSnippetId = prediction[0].first;
	}

	grammar_id = grammarSnippetId;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// regression

	// scale the contour to 128x128 size
	glm::vec2 scale(128.0 / width(), 128.0 / height());

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
		//cv::imwrite("input.png", input);

		if (meanSubtraction) {
			cv::Mat meanImg = cv::imread("../models/mean.png");
			cv::cvtColor(meanImg, meanImg, cv::COLOR_BGR2GRAY);
			input -= meanImg;
		}

		// estimate parameters
		std::vector<float> params = regressions[grammarSnippetId]->Predict(input);

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
		double dist = distance(grammarSnippetId, centering3D, cameraType, cameraDistanceBase, cameraHeight, xrotMin, xrotMax, yrotMin, yrotMax, zrotMin, zrotMax, fovMin, fovMax, params, faces);

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

	// Refine from Best なら、ベストの初期パラメータ値のみを、リストにいれる
	// このリストの各パラメータ値について、local refinementを実施する
	if (refineFromBest) {
		params_list.clear();
		params_list.push_back(best_params);
	}

#if 0
	if (refinement) {
		printf("Refinement: ");

		best_dist = std::numeric_limits<double>::max();
		for (int initial_params_id = 0; initial_params_id < params_list.size(); ++initial_params_id) {
			float delta = 0.1f;
			bool updated = false;
			double dist;
			for (int refinement_iter = 0;; ++refinement_iter) {
				std::vector<boost::shared_ptr<glutils::Face> > faces;
				dist = distance(grammarSnippetId, centering3D, cameraType, cameraDistanceBase, cameraHeight, xrotMin, xrotMax, yrotMin, yrotMax, zrotMin, zrotMax, fovMin, fovMax, params_list[initial_params_id], faces, false);

				// show the progress
				printf("\rRefinement: %d (delta: %lf, dist: %lf)        ", refinement_iter, delta, dist);
				fflush(stdout);

				// index for coordinate descent
				int param_id = refinement_iter % params_list[initial_params_id].size();
				if (param_id == 0) {
					updated = false;
				}

				// compute the score of -Delta
				std::vector<float> new_params1 = params_list[initial_params_id];
				new_params1[param_id] -= delta;
				double new_dist1 = distance(grammarSnippetId, centering3D, cameraType, cameraDistanceBase, cameraHeight, xrotMin, xrotMax, yrotMin, yrotMax, zrotMin, zrotMax, fovMin, fovMax, new_params1, faces, false);

				// compute the score of Delta
				std::vector<float> new_params2 = params_list[initial_params_id];
				new_params2[param_id] += delta;
				double new_dist2 = distance(grammarSnippetId, centering3D, cameraType, cameraDistanceBase, cameraHeight, xrotMin, xrotMax, yrotMin, yrotMax, zrotMin, zrotMax,  fovMin, fovMax, new_params2, faces, false);

				// pick the best one
				if (new_dist1 < dist && new_dist1 <= new_dist2) {
					params_list[initial_params_id] = new_params1;
					dist = new_dist1;
					updated = true;
				}
				else if (new_dist2 < dist) {
					params_list[initial_params_id] = new_params2;
					dist = new_dist2;
					updated = true;
				}

				// 一周しても更新がないなら、deltaを半減する
				if (param_id == params_list[initial_params_id].size() - 1) {
					if (!updated) {
						delta /= 2.0;
					}
				}

				// 十分収束したら、終了する
				if (delta < 0.01) {
					dist = distance(grammarSnippetId, centering3D, cameraType, cameraDistanceBase, cameraHeight, xrotMin, xrotMax, yrotMin, yrotMax, zrotMin, zrotMax, fovMin, fovMax, params_list[initial_params_id], faces, false, initial_params_id);
					break;
				}
			}
			printf("\n");

			std::cout << "Refined params:" << std::endl;
			for (int i = 0; i < params_list[initial_params_id].size(); ++i) {
				if (i > 0) std::cout << ", ";
				std::cout << params_list[initial_params_id][i];
			}
			std::cout << std::endl;

			// best paramsを更新
			if (dist < best_dist) {
				best_dist = dist;
				best_params = params_list[initial_params_id];
			}
		}
	}
#endif

	if (refinement) {
		QFile file("refinement.txt");
		file.open(QIODevice::WriteOnly);
		QTextStream out(&file);

		printf("Refinement: ");

		std::vector<float> params = params_list[0];
		best_params = params_list[0];
		std::vector<boost::shared_ptr<glutils::Face> > faces;

		double dist = distance(grammarSnippetId, centering3D, cameraType, cameraDistanceBase, cameraHeight, xrotMin, xrotMax, yrotMin, yrotMax, zrotMin, zrotMax, fovMin, fovMax, params, faces);
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
			double next_dist = distance(grammarSnippetId, centering3D, cameraType, cameraDistanceBase, cameraHeight, xrotMin, xrotMax, yrotMin, yrotMax, zrotMin, zrotMax, fovMin, fovMax, next_params, faces);
	
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
						double next_dist1 = distance(grammarSnippetId, centering3D, cameraType, cameraDistanceBase, cameraHeight, xrotMin, xrotMax, yrotMin, yrotMax, zrotMin, zrotMax, fovMin, fovMax, next_params1, faces);

						// option 2
						std::vector<float> next_params2 = next_params;
						next_params2[k] -= sqrt(params_var[k]) * delta;
						double next_dist2 = distance(grammarSnippetId, centering3D, cameraType, cameraDistanceBase, cameraHeight, xrotMin, xrotMax, yrotMin, yrotMax, zrotMin, zrotMax, fovMin, fovMax, next_params2, faces);

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

	estimated_pm_params.clear();
	for (int i = 4; i < best_params.size(); ++i) {
		estimated_pm_params.push_back(best_params[i]);
	}
	updateStatusBar();

	// update camera matrix
	camera.updatePMatrix(width(), height());

	// render the image
	cv::Mat renderedImage;
	std::vector<boost::shared_ptr<glutils::Face> > faces;
	renderManager.renderingMode = RenderManager::RENDERING_MODE_CONTOUR;
	render(grammarSnippetId, centering3D, cameraType, cameraDistanceBase, cameraHeight, xrotMin, xrotMax, yrotMin, yrotMax, zrotMin, zrotMax, fovMin, fovMax, best_params, faces, renderedImage);
	render(grammarSnippetId, centering3D, cameraType, cameraDistanceBase, cameraHeight, xrotMin, xrotMax, yrotMin, yrotMax, zrotMin, zrotMax, fovMin, fovMax, best_params, estimated_faces, renderedImage);

	// compute the offset of the rendered image
	glm::vec2 rendered_offset = getOffset(renderedImage);

	// compute the offset of the silhouette
	offset = getOffset(silhouette);

	// translate the background image and contour lines
	shiftImageAndSilhouette(rendered_offset.x - offset.x, rendered_offset.y - offset.y, bgImage, silhouette);
	shiftImage((rendered_offset.x - offset.x) / bgImageScale, (rendered_offset.y - offset.y) / bgImageScale, bgImageOrig);

	std::cout << "dist: " << distance(silhouette, renderedImage) << std::endl;

	// line modeで描画
	renderManager.renderingMode = RenderManager::RENDERING_MODE_LINE;

	if (applyTexture) {
		// create texture folder
		if (!QDir("textures").exists()) QDir().mkdir("textures");

		// convert bgImage to cv::Mat object
		cv::Mat bgImageMat = cv::Mat(bgImageOrig.height(), bgImageOrig.width(), CV_8UC4, const_cast<uchar*>(bgImageOrig.bits()), bgImageOrig.bytesPerLine()).clone();

		// rectify the facade image
		for (int i = 0; i < faces.size(); ++i) {
			std::vector<cv::Point2f> pts;
			std::vector<glm::vec3> pts3d;
			for (int j = 0; j < faces[i]->vertices.size(); ++j) {
				if (j == 3 || j == 4) continue;

				pts3d.push_back(faces[i]->vertices[j].position);

				glm::vec4 result = camera.mvpMatrix * glm::vec4(faces[i]->vertices[j].position, 1);
				glm::vec2 pp((result.x / result.w + 1) * 0.5 * width(), (1 - result.y / result.w) * 0.5 * height());
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

		renderManager.removeObjects();
		renderManager.addFaces(faces, true);
		renderManager.renderingMode = RenderManager::RENDERING_MODE_BASIC;

		opacityOfBackground = 0.1f;

		// remove the texture folder
		//if (QDir("textures").exists()) QDir("textures").removeRecursively();
	}

	updateStatusBar();
	update();
}

/**
* Use the silhouette as an input to the pretrained network, and obtain the probabilities as output.
* Then, display the options ordered by the probabilities.
*/
void GLWidget3D::parameterEstimationWithCameraCalibration(bool automaticRecognition, int grammarSnippetId, bool centering3D, bool meanSubtraction, int cameraType, float cameraDistanceBase, float cameraHeight, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int zrotMin, int zrotMax, int fovMin, int fovMax) {
	// adjust the original background image such that the ratio of width to height is equal to the ratio of the window
	float bgImageScale = std::min((float)width() / bgImageOrig.width(), (float)height() / bgImageOrig.height());
	resizeImageCanvasSize(bgImageOrig, width() / bgImageScale, height() / bgImageScale);

	// compute the bbox
	glutils::BoundingBox bbox;
	for (auto stroke : silhouette) {
		bbox.addPoint(stroke.start);
		bbox.addPoint(stroke.end);
	}

	// compute the offset
	glm::vec2 offset;
	offset.x = width() * 0.5f - bbox.center().x;
	offset.y = height() * 0.5f - bbox.center().y;

	// shift the image and silhouette
	shiftImageAndSilhouette(offset.x, offset.y, bgImage, silhouette);
	shiftImage(offset.x / bgImageScale, offset.y / bgImageScale, bgImageOrig);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// classification
	if (automaticRecognition) {



	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// regression

	// scale the contour to 128x128 size
	glm::vec2 scale(128.0 / width(), 128.0 / height());

	std::vector<Stroke> scaledSilhouette;
	for (auto stroke : silhouette) {
		Stroke s;
		s.start = glm::vec2(stroke.start.x * scale.x, stroke.start.y * scale.y);
		s.end = glm::vec2(stroke.end.x * scale.x, stroke.end.y * scale.y);
		scaledSilhouette.push_back(s);
	}

	// create input image
	cv::Mat input(128, 128, CV_8U, cv::Scalar(255));
	for (auto stroke : scaledSilhouette) {
		cv::line(input, cv::Point(stroke.start.x, stroke.start.y), cv::Point(stroke.end.x, stroke.end.y), cv::Scalar(0), 1, cv::LINE_AA);
	}

	if (meanSubtraction) {
		cv::Mat meanImg = cv::imread("../models/mean.png");
		cv::cvtColor(meanImg, meanImg, cv::COLOR_BGR2GRAY);
		input -= meanImg;
	}

	// estimate parameters
	std::vector<float> params = regressions[grammarSnippetId]->Predict(input);

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

	std::cout << "Params: ";
	for (int i = 0; i < params.size(); ++i) {
		if (i > 0) std::cout << ",";
		std::cout << params[i];
	}
	std::cout << std::endl;

	// Geometry faces
	std::vector<boost::shared_ptr<glutils::Face> > faces;
	distance(grammarSnippetId, centering3D, cameraType, cameraDistanceBase, cameraHeight, xrotMin, xrotMax, yrotMin, yrotMax, zrotMin, zrotMax, fovMin, fovMax, params, faces);
	/*
	std::cout << "----------------------------------------------" << std::endl;
	std::cout << "faces:" << std::endl;
	for (int i = 0; i < faces.size(); ++i) {
		std::cout << "face " << i + 1 << std::endl;
		for (int j = 0; j < faces[i]->vertices.size(); ++j) {
			std::cout << "(" << faces[i]->vertices[j].position.x << "," << faces[i]->vertices[j].position.y << "," << faces[i]->vertices[j].position.z << ")" << std::endl;
		}
		std::cout << std::endl;
	}
	std::cout << "----------------------------------------------" << std::endl;
	*/

	// update camera matrix
	camera.updatePMatrix(width(), height());

	///////////////////////////////////////////////////////////////////////////////////
	// contourの頂点の座標を取得
	std::vector<std::vector<cv::Point2f>> imagePoints(1);
	for (int i = 0; i < scaledSilhouette.size(); ++i) {
		cv::Point2f p1(scaledSilhouette[i].start.x / 64.0 - 1.0, 1 - scaledSilhouette[i].start.y / 64.0);
		bool registered = false;
		for (int j = 0; j < imagePoints[0].size(); ++j) {
			// 既に登録済みの頂点はスキップ
			if (sqrtf(SQR(imagePoints[0][j].x - p1.x) + SQR(imagePoints[0][j].y - p1.y)) < 0.07f) {
				registered = true;
				break;
			}
		}

		if (!registered) {
			imagePoints[0].push_back(p1);
		}

		cv::Point2f p2(scaledSilhouette[i].end.x / 64.0 - 1.0, 1 - scaledSilhouette[i].end.y / 64.0);
		registered = false;
		for (int j = 0; j < imagePoints[0].size(); ++j) {
			// 既に登録済みの頂点はスキップ
			if (sqrtf(SQR(imagePoints[0][j].x - p2.x) + SQR(imagePoints[0][j].y - p2.y)) < 0.07f) {
				registered = true;
				break;
			}
		}

		if (!registered) {
			imagePoints[0].push_back(p2);
		}
	}
	///////////////////////////////////////////////////////////////////////////////////


	// 3Dの頂点の座標を取得
	std::vector<glm::vec3> objectPointsTemp;
	for (int i = 0; i < faces.size(); ++i) {
		for (int j = 0; j < faces[i]->vertices.size(); ++j) {
			// 既にその座標が登録済みかチェック
			bool registered = false;
			for (int k = 0; k < objectPointsTemp.size(); ++k) {
				float dist = glm::length(objectPointsTemp[k] - faces[i]->vertices[j].position);
				if (dist < 0.1f) {
					registered = true;
					break;
				}
			}

			if (!registered) {
				objectPointsTemp.push_back(faces[i]->vertices[j].position);
			}
		}
	}

	// 3Dの頂点を、2Dの頂点に合わせて並べる
	std::vector<std::vector<cv::Point3f>> objectPoints(1);
	std::map<int, bool> used;
	for (int i = 0; i < imagePoints[0].size(); ++i) {
		// 最も近い3Dの頂点を探す
		float min_dist = std::numeric_limits<float>::max();
		int min_idx;
		for (int j = 0; j < objectPointsTemp.size(); ++j) {
			if (used.find(j) != used.end() && used[j]) continue;

			glm::vec4 result = camera.mvpMatrix * glm::vec4(objectPointsTemp[j], 1);
			glm::vec2 pp(result.x / result.w, result.y / result.w);

			float dist = sqrtf(SQR(imagePoints[0][i].x - pp.x) + SQR(imagePoints[0][i].y - pp.y));
			if (dist < min_dist) {
				min_dist = dist;
				min_idx = j;
			}
		}

		used[min_idx] = true;
		objectPoints[0].push_back(cv::Point3f(objectPointsTemp[min_idx].x, objectPointsTemp[min_idx].y, objectPointsTemp[min_idx].z));

	}
	///////////////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////////////
	std::cout << "--------------------------------------------------------------------------" << std::endl;
	std::cout << "Points correspondence (2D point by sketch --> 3D point --> 2D projection from 3D)" << std::endl;
	double dist = 0.0;
	for (int i = 0; i < objectPoints[0].size(); ++i) {
		cv::Mat cameraMat = cv::Mat::eye(3, 3, CV_64F);
		cameraMat.at<double>(0, 0) = camera.pMatrix[0][0];
		cameraMat.at<double>(1, 1) = camera.pMatrix[1][1];
		cameraMat.at<double>(2, 0) = camera.pMatrix[2][0];
		cameraMat.at<double>(2, 1) = camera.pMatrix[2][1];

		glm::vec4 p2(objectPoints[0][i].x, objectPoints[0][i].y, objectPoints[0][i].z, 1);
		glm::vec4 pp2 = camera.mvpMatrix * p2;
		std::cout << "(" << imagePoints[0][i].x << "," << imagePoints[0][i].y << ") --> (" << objectPoints[0][i].x << "," << objectPoints[0][i].y << "," << objectPoints[0][i].z << ") --> (" << pp2.x / pp2.w << "," << pp2.y / pp2.w << ")" << std::endl;

		dist += SQR(imagePoints[0][i].x - pp2.x / pp2.w) + SQR(imagePoints[0][i].y - pp2.y / pp2.w);
	}
	dist = sqrt(dist);
	std::cout << "Dist: " << dist << std::endl;

	// Camera calibration
	cv::Mat cameraMat = cv::Mat::eye(3, 3, CV_64F);
	cameraMat.at<double>(0, 0) = camera.pMatrix[0][0];
	cameraMat.at<double>(1, 1) = camera.pMatrix[1][1];
	cameraMat.at<double>(2, 0) = camera.pMatrix[2][0];
	cameraMat.at<double>(2, 1) = camera.pMatrix[2][1];
	std::cout << "--------------------------------------------" << std::endl;
	std::cout << "Camera matrix: " << std::endl;
	std::cout << cameraMat << std::endl;
	cv::Mat distortion = cv::Mat::zeros(1, 8, CV_64F);
	std::vector<cv::Mat> rvecs, tvecs;
	cv::calibrateCamera(objectPoints, imagePoints, cv::Size(128, 128), cameraMat, distortion, rvecs, tvecs, cv::CALIB_USE_INTRINSIC_GUESS | cv::CALIB_FIX_ASPECT_RATIO | cv::CALIB_FIX_PRINCIPAL_POINT | cv::CALIB_ZERO_TANGENT_DIST | cv::CALIB_FIX_K2 | cv::CALIB_FIX_K3 | cv::CALIB_FIX_K4 | cv::CALIB_FIX_K5 | cv::CALIB_FIX_K6);
	std::cout << "--------------------------------------------" << std::endl;
	std::cout << "Updated camera matrix:" << std::endl << cameraMat << std::endl;
	for (int i = 0; i < rvecs.size(); ++i) {
		std::cout << "R:" << std::endl << rvecs[i] << std::endl;
		std::cout << "T:" << std::endl << tvecs[i] << std::endl;
	}

	float fov = atan2(1.0, cameraMat.at<double>(0, 0)) * 2.0 / M_PI * 180.0;
	std::cout << "FOV: " << camera.fovy << " --> " << fov << std::endl;
	camera.fovy = atan2(1.0f, cameraMat.at<double>(0, 0)) * 2.0 / M_PI * 180.0;
	camera.pos.z = cameraDistanceBase / tanf(camera.fovy * 0.5 / 180.0f * M_PI);

	cv::Mat R;
	cv::Rodrigues(rvecs[0], R);
	float xrot = atan2(R.at<double>(1, 2), R.at<double>(2, 2)) / M_PI * 180.0;
	float yrot = atan2(-R.at<double>(0, 2), sqrt(SQR(R.at<double>(1, 2)) + SQR(R.at<double>(2, 2)))) / M_PI * 180.0;	
	std::cout << "Xrot: " << camera.xrot << " --> " << xrot << std::endl;
	std::cout << "Yrot: " << camera.yrot << " --> " << yrot << std::endl;
	camera.xrot = xrot;
	camera.yrot = yrot;

	camera.updatePMatrix(width(), height());
	/*
	cv::Mat R;
	cv::rodrigues(rvecs, R);

	cv::Mat P(3, 4, CV_64F);
	for (int r = 0; r < 3; ++r) {
	for (int c = 0; c < 3; ++c) {}
	}
	*/
	///////////////////////////////////////////////////////////////////////////////////

	cv::Mat M(3, 4, CV_64F);
	cv::Mat roi = M(cv::Rect(0, 0, 3, 3));
	R.copyTo(roi);
	roi = M(cv::Rect(3, 0, 1, 3));
	tvecs[0].copyTo(roi);
	std::cout << "-------------------------" << std::endl;
	std::cout << "Updated model view matrix:" << std::endl;
	std::cout << M << std::endl;

	std::cout << "-------------------------------------------------" << std::endl;
	std::cout << "Reprojected points by updated camera parameters" << std::endl;
	dist = 0;
	for (int i = 0; i < objectPoints[0].size(); ++i) {
		cv::Mat_<double> p(4, 1);
		p(0, 0) = objectPoints[0][i].x;
		p(1, 0) = objectPoints[0][i].y;
		p(2, 0) = objectPoints[0][i].z;
		p(3, 0) = 1;
		cv::Mat result = cameraMat * M * p;
		std::cout << "(" << result.at<double>(0, 0) / result.at<double>(2, 0) << "," << result.at<double>(1, 0) / result.at<double>(2, 0) << ")" << std::endl;

		dist += SQR(imagePoints[0][i].x - result.at<double>(0, 0) / result.at<double>(2, 0)) + SQR(imagePoints[0][i].y - result.at<double>(1, 0) / result.at<double>(2, 0));
	}
	dist = sqrt(dist);
	std::cout << "Updated dist: " << dist << std::endl;




	updateStatusBar();
	update();
}

void GLWidget3D::parameterEstimationWithCameraCalibration2(int grammarSnippetId, bool centering3D, bool meanSubtraction, int cameraType, float cameraDistanceBase, float cameraHeight, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int fovMin, int fovMax) {



	///////////////////////////////////////////////////////////////////////////////////
	// contourの頂点の座標を取得
	std::vector<std::vector<cv::Point2f>> imagePoints(7);
	imagePoints[0].push_back(cv::Point2f(-0.15875, 0.1375));
	imagePoints[0].push_back(cv::Point2f(-0.14375, -0.28));
	imagePoints[0].push_back(cv::Point2f(-0.03625, -0.345));
	imagePoints[0].push_back(cv::Point2f(0.15625, -0.16));
	imagePoints[0].push_back(cv::Point2f(0.15875, 0.295));
	imagePoints[0].push_back(cv::Point2f(0.05, 0.34375));

	imagePoints[1].push_back(cv::Point2f(-0.00749999, 0.245));
	imagePoints[1].push_back(cv::Point2f(-0.2375, 0.0825));
	imagePoints[1].push_back(cv::Point2f(-0.23, -0.1775));
	imagePoints[1].push_back(cv::Point2f(0.2375, 0.18));
	imagePoints[1].push_back(cv::Point2f(0.2325, -0.0575));
	imagePoints[1].push_back(cv::Point2f(0.02, -0.25));

	imagePoints[2].push_back(cv::Point2f(-0.26125, 0.115));
	imagePoints[2].push_back(cv::Point2f(-0.24625, -0.1175));
	imagePoints[2].push_back(cv::Point2f(-0.08625, -0.225));
	imagePoints[2].push_back(cv::Point2f(0.24875, -0.1025));
	imagePoints[2].push_back(cv::Point2f(0.26125, 0.1175));
	imagePoints[2].push_back(cv::Point2f(0.05625, 0.2225));

	imagePoints[3].push_back(cv::Point2f(-0.295, 0.20125));
	imagePoints[3].push_back(cv::Point2f(0.0599999, 0.29125));
	imagePoints[3].push_back(cv::Point2f(0.2975, -0.02625));
	imagePoints[3].push_back(cv::Point2f(0.2825, -0.18125));
	imagePoints[3].push_back(cv::Point2f(-0.0875, -0.29125));
	imagePoints[3].push_back(cv::Point2f(-0.29, 0.04875));

	imagePoints[4].push_back(cv::Point2f(-0.1975, 0.1725));
	imagePoints[4].push_back(cv::Point2f(-0.19, -0.1025));
	imagePoints[4].push_back(cv::Point2f(0.0749999, -0.2525));
	imagePoints[4].push_back(cv::Point2f(0.19, -0.1675));
	imagePoints[4].push_back(cv::Point2f(0.2, 0.11));
	imagePoints[4].push_back(cv::Point2f(-0.085, 0.25));

	imagePoints[5].push_back(cv::Point2f(-0.1175, 0.25));
	imagePoints[5].push_back(cv::Point2f(-0.02, 0.31));
	imagePoints[5].push_back(cv::Point2f(0.1175, 0.2375));
	imagePoints[5].push_back(cv::Point2f(0.11, -0.2475));
	imagePoints[5].push_back(cv::Point2f(0.0325, -0.31));
	imagePoints[5].push_back(cv::Point2f(-0.0975, -0.2225));

	imagePoints[6].push_back(cv::Point2f(-0.0225, 0.235));
	imagePoints[6].push_back(cv::Point2f(-0.2025, 0.09));
	imagePoints[6].push_back(cv::Point2f(-0.1975, -0.155));
	imagePoints[6].push_back(cv::Point2f(0.0224999, -0.2375));
	imagePoints[6].push_back(cv::Point2f(0.2, 0.1725));
	imagePoints[6].push_back(cv::Point2f(0.19, -0.0799999));


	///////////////////////////////////////////////////////////////////////////////////



	// 3Dの頂点を、2Dの頂点に合わせて並べる
	std::vector<std::vector<cv::Point3f>> objectPoints(7);
	objectPoints[0].push_back(cv::Point3f(-4.14134, 8.81758, -2));
	objectPoints[0].push_back(cv::Point3f(-4.14134, -8.81758, -2));
	objectPoints[0].push_back(cv::Point3f(-4.14134, -8.81758, 2));
	objectPoints[0].push_back(cv::Point3f(4.14134, -8.81758, 2));
	objectPoints[0].push_back(cv::Point3f(4.14134, 8.81758, 2));
	objectPoints[0].push_back(cv::Point3f(4.14134, 8.81758, -2));

	objectPoints[1].push_back(cv::Point3f(5.16899, 3.76056, -3.62279));
	objectPoints[1].push_back(cv::Point3f(-5.16899, 3.76056, -3.62279));
	objectPoints[1].push_back(cv::Point3f(-5.16899, -3.76056, -3.62279));
	objectPoints[1].push_back(cv::Point3f(5.16899, 3.76056, 3.62279));
	objectPoints[1].push_back(cv::Point3f(5.16899, -3.76056, 3.62279));
	objectPoints[1].push_back(cv::Point3f(-5.16899, -3.76056, 3.62279));

	objectPoints[2].push_back(cv::Point3f(-4.96565, 3.1291, -3.61298));
	objectPoints[2].push_back(cv::Point3f(-4.96565, -3.1291, -3.61298));
	objectPoints[2].push_back(cv::Point3f(-4.96565, -3.1291, 3.61298));
	objectPoints[2].push_back(cv::Point3f(4.96565, -3.1291, 3.61298));
	objectPoints[2].push_back(cv::Point3f(4.96565, 3.1291, 3.61298));
	objectPoints[2].push_back(cv::Point3f(4.96565, 3.1291, -3.61298));

	objectPoints[3].push_back(cv::Point3f(-5.37314, 2.90477, -6.32432));
	objectPoints[3].push_back(cv::Point3f(5.37314, 2.90477, -6.32432));
	objectPoints[3].push_back(cv::Point3f(5.37314, 2.90477, 6.32433));
	objectPoints[3].push_back(cv::Point3f(5.37314, -2.90477, 6.32433));
	objectPoints[3].push_back(cv::Point3f(-5.37314, -2.90477, 6.32433));
	objectPoints[3].push_back(cv::Point3f(-5.37314, -2.90477, -6.32433));

	objectPoints[4].push_back(cv::Point3f(-2.46817, 4.50724, -4.61624));
	objectPoints[4].push_back(cv::Point3f(-2.46817, -4.50723, -4.61625));
	objectPoints[4].push_back(cv::Point3f(-2.46817, -4.50724, 4.61624));
	objectPoints[4].push_back(cv::Point3f(2.46817, -4.50724, 4.61624));
	objectPoints[4].push_back(cv::Point3f(2.46817, 4.50723, 4.61625));
	objectPoints[4].push_back(cv::Point3f(2.46817, 4.50724, -4.61624));

	objectPoints[5].push_back(cv::Point3f(-2.04061, 7.62239, -2.79208));
	objectPoints[5].push_back(cv::Point3f(2.04061, 7.62239, -2.79208));
	objectPoints[5].push_back(cv::Point3f(2.04061, 7.62239, 2.79209));
	objectPoints[5].push_back(cv::Point3f(2.04061, -7.62239, 2.79209));
	objectPoints[5].push_back(cv::Point3f(-2.04061, -7.62239, 2.79209));
	objectPoints[5].push_back(cv::Point3f(-2.04061, -7.62239, -2.79209));

	objectPoints[6].push_back(cv::Point3f(4.02926, 4.04294, -3.25238));
	objectPoints[6].push_back(cv::Point3f(-4.02926, 4.04294, -3.25238));
	objectPoints[6].push_back(cv::Point3f(-4.02926, -4.04294, -3.25238));
	objectPoints[6].push_back(cv::Point3f(-4.02926, -4.04294, 3.25238));
	objectPoints[6].push_back(cv::Point3f(4.02926, 4.04294, 3.25238));
	objectPoints[6].push_back(cv::Point3f(4.02926, -4.04294, 3.25238));

	///////////////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////////////
	// Camera calibration
	cv::Mat cameraMat = cv::Mat::eye(3, 3, CV_64F);
	cameraMat.at<double>(0, 0) = 4.3;// camera.pMatrix[0][0];
	cameraMat.at<double>(1, 1) = 4.3;// camera.pMatrix[1][1];
	cameraMat.at<double>(2, 0) = 0;// camera.pMatrix[2][0];
	cameraMat.at<double>(2, 1) = 0;// camera.pMatrix[2][1];
	std::cout << "Camera Matrix: " << std::endl;
	std::cout << cameraMat << std::endl;
	cv::Mat distortion = cv::Mat::zeros(1, 8, CV_64F);
	std::vector<cv::Mat> rvecs, tvecs;
	cv::calibrateCamera(objectPoints, imagePoints, cv::Size(128, 128), cameraMat, distortion, rvecs, tvecs, cv::CALIB_USE_INTRINSIC_GUESS | cv::CALIB_FIX_ASPECT_RATIO | cv::CALIB_FIX_PRINCIPAL_POINT | cv::CALIB_ZERO_TANGENT_DIST | cv::CALIB_FIX_K2 | cv::CALIB_FIX_K3 | cv::CALIB_FIX_K4 | cv::CALIB_FIX_K5 | cv::CALIB_FIX_K6);
	std::cout << "<<< OpenCV results >>>" << std::endl;
	std::cout << "Camera Matrix:" << std::endl << cameraMat << std::endl;
	for (int i = 0; i < rvecs.size(); ++i) {
		std::cout << "R:" << std::endl << rvecs[i] << std::endl;
		std::cout << "T:" << std::endl << tvecs[i] << std::endl;
	}

	float fov = atan2(1.0, cameraMat.at<double>(0, 0)) * 2.0 / M_PI * 180.0;
	std::cout << "FOV: " << fov << std::endl;
	camera.fovy = atan2(1.0f, cameraMat.at<double>(0, 0)) * 2.0 / M_PI * 180.0;
	camera.pos.z = cameraDistanceBase / tanf(camera.fovy * 0.5 / 180.0f * M_PI);

	cv::Mat R;
	for (int i = 0; i < 7; ++i) {
		cv::Rodrigues(rvecs[i], R);
		float xrot = atan2(R.at<double>(1, 2), R.at<double>(2, 2)) / M_PI * 180.0;
		float yrot = atan2(-R.at<double>(0, 2), sqrt(SQR(R.at<double>(1, 2)) + SQR(R.at<double>(2, 2)))) / M_PI * 180.0;
		std::cout << "Xrot[" << i << "]: " << xrot << std::endl;
		std::cout << "Yrot[" << i << "]: " << yrot << std::endl;
	}
	/*
	cv::Mat R;
	cv::rodrigues(rvecs, R);

	cv::Mat P(3, 4, CV_64F);
	for (int r = 0; r < 3; ++r) {
	for (int c = 0; c < 3; ++c) {}
	}
	*/
	///////////////////////////////////////////////////////////////////////////////////





	updateStatusBar();
	update();
}

/**
 * 指定されたカメラパラメータ、PMパラメータでrenderした結果と、ユーザ指定のsilhouetteの距離を返却する。
 * renderした画像の中心とユーザ指定のsilhouetteの中心がずれている場合は、自動で調節する。
 */
double GLWidget3D::distance(int grammarSnippetId, bool centering3D, int cameraType, float cameraDistanceBase, float cameraHeight, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int zrotMin, int zrotMax, int fovMin, int fovMax, const std::vector<float>& params, std::vector<boost::shared_ptr<glutils::Face>>& faces) {
	float xrot = (xrotMax - xrotMin) * params[0] + xrotMin;
	float yrot = (yrotMax - yrotMin) * params[1] + yrotMin;
	float zrot = (zrotMax - zrotMin) * params[2] + zrotMin;
	float fov = (fovMax - fovMin) * params[3] + fovMin;

	std::vector<float> pm_params;
	for (int i = 4; i < params.size(); ++i) {
		pm_params.push_back(params[i]);
	}

	setupGeometry(grammarSnippetId, centering3D, cameraType, cameraDistanceBase, cameraHeight, xrot, yrot, zrot, fov, pm_params, faces);

	glutils::BoundingBox bbox;
	std::vector<glm::vec2> points;
	for (auto face : faces) {
		// check if the face is visible
		//std::cout << "normal: " << face->vertices[0].normal.x << "," << face->vertices[0].normal.y << "," << face->vertices[0].normal.z << std::endl;
		glm::vec3 normal = glm::vec3(camera.mvMatrix * glm::vec4(face->vertices[0].normal, 0));
		//std::cout << "normal 2: " << normal.x << "," << normal.y << "," << normal.z << std::endl;
		if (glm::dot(normal, glm::vec3(0, 0, 1)) < 0) continue;

		for (auto vert : face->vertices) {
			glm::vec2 pp = utils::projectPoint(width(), height(), vert.position, camera.mvpMatrix);
			bbox.addPoint(pp);
			points.push_back(pp);
		}
	}
	glm::vec2 rendered_offset(bbox.center().x - width() * 0.5, bbox.center().y - height() * 0.5);

	glm::vec2 offset = rendered_offset - getOffset(silhouette);

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

	return dist;
}

void GLWidget3D::setupGeometry(int grammarSnippetId, bool centering3D, int cameraType, float cameraDistanceBase, float cameraHeight, float xrot, float yrot, float zrot, float fov, const std::vector<float>& params, std::vector<boost::shared_ptr<glutils::Face>>& faces) {
	estimated_pm_params = params;

	// setup the camera
	camera.xrot = xrot;
	camera.yrot = yrot;
	camera.zrot = zrot;
	camera.fovy = fov;
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
	camera.updatePMatrix(width(), height());

	// setup CGA
	cga::CGA cga;
	cga.modelMat = glm::rotate(glm::mat4(), -(float)M_PI * 0.5f, glm::vec3(1, 0, 0));

	cga.setParamValues(grammars[grammarSnippetId], params);

	// set axiom
	boost::shared_ptr<cga::Shape> start = boost::shared_ptr<cga::Shape>(new cga::Rectangle("Start", "", glm::translate(glm::rotate(glm::mat4(), -3.141592f * 0.5f, glm::vec3(1, 0, 0)), glm::vec3(0, 0, 0)), glm::mat4(), 0, 0, glm::vec3(1, 1, 1)));
	cga.stack.push_back(start);

	// generate 3d model
	renderManager.removeObjects();
	cga.derive(grammars[grammarSnippetId], true);
	faces.clear();
	cga.generateGeometry(faces, centering3D);
	renderManager.addFaces(faces, true);
}

void GLWidget3D::render(int grammarSnippetId, bool centering3D, int cameraType, float cameraDistanceBase, float cameraHeight, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int zrotMin, int zrotMax, int fovMin, int fovMax, const std::vector<float>& params, std::vector<boost::shared_ptr<glutils::Face>>& faces, cv::Mat& renderedImage) {
	float xrot = xrotMin + (xrotMax - xrotMin) * params[0];
	float yrot = yrotMin + (yrotMax - yrotMin) * params[1];
	float zrot = zrotMin + (zrotMax - zrotMin) * params[2];
	float fov = fovMin + (fovMax - fovMin) * params[3];

	std::vector<float> pm_params;
	for (int i = 4; i < params.size(); ++i) {
		pm_params.push_back(params[i]);
	}

	setupGeometry(grammarSnippetId, centering3D, cameraType, cameraDistanceBase, cameraHeight, xrot, yrot, zrot, fov, pm_params, faces);

	// obtain the rendered image
	render();
	QImage img = grabFrameBuffer();
	renderedImage = cv::Mat(img.height(), img.width(), CV_8UC4, img.bits(), img.bytesPerLine()).clone();
}





void GLWidget3D::keyPressEvent(QKeyEvent *e) {
	ctrlPressed = false;

	switch (e->key()) {
	case Qt::Key_Control:
		ctrlPressed = true;
		break;
	case Qt::Key_Shift:
		shiftPressed = true;
		break;
	case Qt::Key_Delete:
		clearGeometry();
		break;
	default:
		break;
	}
}

void GLWidget3D::keyReleaseEvent(QKeyEvent* e) {
	ctrlPressed = false;
	shiftPressed = false;
}

/**
 * This event handler is called when the mouse press events occur.
 */
void GLWidget3D::mousePressEvent(QMouseEvent *e) {
	if (e->buttons() & Qt::RightButton) {
		camera.mousePress(e->x(), e->y());
	}
	else if (e->button() & Qt::LeftButton) {
		currentStroke = Stroke(glm::vec2(e->x(), e->y()), glm::vec2(e->x(), e->y()));
		dragging = true;
	}
}

/**
 * This event handler is called when the mouse release events occur.
 */
void GLWidget3D::mouseReleaseEvent(QMouseEvent *e) {
	if (e->button() == Qt::LeftButton) {
		silhouette.push_back(currentStroke);
		currentStroke = Stroke();

		dragging = false;
	}
	else if (e->button() == Qt::RightButton) {
		// compute the distance
		/*
		if (estimated_pm_params.size() > 0) {
			std::vector<float> params;
			std::vector<boost::shared_ptr<glutils::Face>> faces;

			setupGeometry(grammar_id, true, 1, 25, 0, camera.xrot, camera.yrot, camera.zrot, camera.fovy, estimated_pm_params, faces);

			glm::vec2 offset = getOffset(silhouette);
			
			renderManager.renderingMode = RenderManager::RENDERING_MODE_CONTOUR;
			render();
			QImage img = grabFrameBuffer();
			cv::Mat renderedImg = cv::Mat(img.height(), img.width(), CV_8UC4, img.bits(), img.bytesPerLine()).clone();
			glm::vec2 rendered_offset = getOffset(renderedImg);
			std::cout << "rendered offset: " << rendered_offset.x << "," << rendered_offset.y << std::endl;
			std::cout << "silhouette offset: " << offset.x << "," << offset.y << std::endl;
			shiftImageAndSilhouette(rendered_offset.x - offset.x, rendered_offset.y - offset.y, bgImage, silhouette);
			double estimation_error = distance(silhouette, renderedImg);
			std::cout << "estimation_error: " << estimation_error << std::endl;
			renderManager.renderingMode = RenderManager::RENDERING_MODE_LINE;
			render();
		}
		*/
	}

	updateStatusBar();
	update();
}

/**
 * This event handler is called when the mouse move events occur.
 */
void GLWidget3D::mouseMoveEvent(QMouseEvent *e) {
	if (e->buttons() & Qt::RightButton) { // Rotate
		if (shiftPressed) { // Move
			camera.move(e->x(), e->y());
		}
		else {
			camera.rotate(e->x(), e->y());
		}
	}
	else if (e->buttons() & Qt::MidButton) { // Rotate around viewing direction
		camera.rotateAroundZ(e->x(), e->y());
	}
	else if (e->buttons() & Qt::LeftButton) {
		currentStroke.end = glm::vec2(e->x(), e->y());
		//curPos = glm::vec2(e->x(), e->y());
	}
	
	updateStatusBar();
	update();
}

void GLWidget3D::wheelEvent(QWheelEvent* e) {
	camera.zoom(e->delta() * 0.01);
	updateStatusBar();
	update();
}

/**
 * This function is called once before the first call to paintGL() or resizeGL().
 */
void GLWidget3D::initializeGL() {
	// init glew
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
	}

	if (glewIsSupported("GL_VERSION_4_2"))
		printf("Ready for OpenGL 4.2\n");
	else {
		printf("OpenGL 4.2 not supported\n");
		exit(1);
	}
	const GLubyte* text = glGetString(GL_VERSION);
	printf("VERSION: %s\n", text);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_TEXTURE_2D);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glTexGenf(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenf(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glDisable(GL_TEXTURE_2D);

	glEnable(GL_TEXTURE_3D);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glDisable(GL_TEXTURE_3D);

	glEnable(GL_TEXTURE_2D_ARRAY);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glDisable(GL_TEXTURE_2D_ARRAY);

	////////////////////////////////
	renderManager.init("", "", "", true, 8192);
	renderManager.resize(this->width(), this->height());
	renderManager.renderingMode = RenderManager::RENDERING_MODE_LINE;

	glUniform1i(glGetUniformLocation(renderManager.programs["ssao"], "tex0"), 0);//tex0: 0

	//fixCamera();

	std::vector<Vertex> vertices;
	glutils::drawQuad(0, 0, glm::vec4(1, 1, 1, 1), glm::mat4(), vertices);
	renderManager.addObject("dummy", "", vertices, true);
}

/**
 * This function is called whenever the widget has been resized.
 */
void GLWidget3D::resizeGL(int width, int height) {
	height = height ? height : 1;
	glViewport(0, 0, width, height);
	camera.updatePMatrix(width, height);

	renderManager.resize(width, height);
}

void GLWidget3D::paintEvent(QPaintEvent *event) {
	// OpenGLで描画
	makeCurrent();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	render();

	// REMOVE
	glActiveTexture(GL_TEXTURE0);

	// OpenGLの設定を元に戻す
	glShadeModel(GL_FLAT);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	// draw background image
	QPainter painter(this);
	painter.setOpacity(opacityOfBackground);
	painter.drawImage(0, 0, bgImage);

	// draw silhouette
	painter.setOpacity(1.0f);
	painter.setPen(QPen(QColor(0, 0, 255), 3));
	for (auto stroke : silhouette) {
		painter.drawLine(stroke.start.x, stroke.start.y, stroke.end.x, stroke.end.y);
	}
	if (dragging) {
		painter.drawLine(currentStroke.start.x, currentStroke.start.y, currentStroke.end.x, currentStroke.end.y);
	}
	painter.end();

	glEnable(GL_DEPTH_TEST);

}

/**
* Draw the scene.
*/
void GLWidget3D::drawScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(true);

	renderManager.renderAll();
}

void GLWidget3D::render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// PASS 1: Render to texture
	glUseProgram(renderManager.programs["pass1"]);

	glBindFramebuffer(GL_FRAMEBUFFER, renderManager.fragDataFB);
	glClearColor(0.95, 0.95, 0.95, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderManager.fragDataTex[0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, renderManager.fragDataTex[1], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, renderManager.fragDataTex[2], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, renderManager.fragDataTex[3], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, renderManager.fragDepthTex, 0);

	// Set the list of draw buffers.
	GLenum DrawBuffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, DrawBuffers); // "4" is the size of DrawBuffers
	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("+ERROR: GL_FRAMEBUFFER_COMPLETE false\n");
		exit(0);
	}

	glUniformMatrix4fv(glGetUniformLocation(renderManager.programs["pass1"], "mvpMatrix"), 1, false, &camera.mvpMatrix[0][0]);
	glUniform3f(glGetUniformLocation(renderManager.programs["pass1"], "lightDir"), light_dir.x, light_dir.y, light_dir.z);
	glUniformMatrix4fv(glGetUniformLocation(renderManager.programs["pass1"], "light_mvpMatrix"), 1, false, &light_mvpMatrix[0][0]);

	glUniform1i(glGetUniformLocation(renderManager.programs["pass1"], "shadowMap"), 6);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, renderManager.shadow.textureDepth);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	drawScene();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// PASS 2: Create AO
	if (renderManager.renderingMode == RenderManager::RENDERING_MODE_SSAO) {
		glUseProgram(renderManager.programs["ssao"]);
		glBindFramebuffer(GL_FRAMEBUFFER, renderManager.fragDataFB_AO);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderManager.fragAOTex, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, renderManager.fragDepthTex_AO, 0);
		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Always check that our framebuffer is ok
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			printf("++ERROR: GL_FRAMEBUFFER_COMPLETE false\n");
			exit(0);
		}

		glDisable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);

		glUniform2f(glGetUniformLocation(renderManager.programs["ssao"], "pixelSize"), 2.0f / this->width(), 2.0f / this->height());

		glUniform1i(glGetUniformLocation(renderManager.programs["ssao"], "tex0"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDataTex[0]);

		glUniform1i(glGetUniformLocation(renderManager.programs["ssao"], "tex1"), 2);
		glActiveTexture(GL_TEXTURE2);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDataTex[1]);

		glUniform1i(glGetUniformLocation(renderManager.programs["ssao"], "tex2"), 3);
		glActiveTexture(GL_TEXTURE3);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDataTex[2]);

		glUniform1i(glGetUniformLocation(renderManager.programs["ssao"], "depthTex"), 8);
		glActiveTexture(GL_TEXTURE8);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDepthTex);

		glUniform1i(glGetUniformLocation(renderManager.programs["ssao"], "noiseTex"), 7);
		glActiveTexture(GL_TEXTURE7);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragNoiseTex);

		{
			glUniformMatrix4fv(glGetUniformLocation(renderManager.programs["ssao"], "mvpMatrix"), 1, false, &camera.mvpMatrix[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(renderManager.programs["ssao"], "pMatrix"), 1, false, &camera.pMatrix[0][0]);
		}

		glUniform1i(glGetUniformLocation(renderManager.programs["ssao"], "uKernelSize"), renderManager.uKernelSize);
		glUniform3fv(glGetUniformLocation(renderManager.programs["ssao"], "uKernelOffsets"), renderManager.uKernelOffsets.size(), (const GLfloat*)renderManager.uKernelOffsets.data());

		glUniform1f(glGetUniformLocation(renderManager.programs["ssao"], "uPower"), renderManager.uPower);
		glUniform1f(glGetUniformLocation(renderManager.programs["ssao"], "uRadius"), renderManager.uRadius);

		glBindVertexArray(renderManager.secondPassVAO);

		glDrawArrays(GL_QUADS, 0, 4);
		glBindVertexArray(0);
		glDepthFunc(GL_LEQUAL);
	}
	else if (renderManager.renderingMode == RenderManager::RENDERING_MODE_LINE || renderManager.renderingMode == RenderManager::RENDERING_MODE_HATCHING) {
		glUseProgram(renderManager.programs["line"]);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDisable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);

		glUniform2f(glGetUniformLocation(renderManager.programs["line"], "pixelSize"), 1.0f / this->width(), 1.0f / this->height());
		glUniformMatrix4fv(glGetUniformLocation(renderManager.programs["line"], "pMatrix"), 1, false, &camera.pMatrix[0][0]);
		if (renderManager.renderingMode == RenderManager::RENDERING_MODE_LINE) {
			glUniform1i(glGetUniformLocation(renderManager.programs["line"], "useHatching"), 0);
		}
		else {
			glUniform1i(glGetUniformLocation(renderManager.programs["line"], "useHatching"), 1);
		}

		glUniform1i(glGetUniformLocation(renderManager.programs["line"], "tex0"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDataTex[0]);

		glUniform1i(glGetUniformLocation(renderManager.programs["line"], "tex1"), 2);
		glActiveTexture(GL_TEXTURE2);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDataTex[1]);

		glUniform1i(glGetUniformLocation(renderManager.programs["line"], "tex2"), 3);
		glActiveTexture(GL_TEXTURE3);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDataTex[2]);

		glUniform1i(glGetUniformLocation(renderManager.programs["line"], "tex3"), 4);
		glActiveTexture(GL_TEXTURE4);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDataTex[3]);

		glUniform1i(glGetUniformLocation(renderManager.programs["line"], "depthTex"), 8);
		glActiveTexture(GL_TEXTURE8);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDepthTex);

		glUniform1i(glGetUniformLocation(renderManager.programs["line"], "hatchingTexture"), 5);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_3D, renderManager.hatchingTextures);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glBindVertexArray(renderManager.secondPassVAO);

		glDrawArrays(GL_QUADS, 0, 4);
		glBindVertexArray(0);
		glDepthFunc(GL_LEQUAL);
	}
	else if (renderManager.renderingMode == RenderManager::RENDERING_MODE_CONTOUR) {
		glUseProgram(renderManager.programs["contour"]);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDisable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);

		glUniform2f(glGetUniformLocation(renderManager.programs["contour"], "pixelSize"), 1.0f / this->width(), 1.0f / this->height());

		glUniform1i(glGetUniformLocation(renderManager.programs["contour"], "depthTex"), 8);
		glActiveTexture(GL_TEXTURE8);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDepthTex);

		glBindVertexArray(renderManager.secondPassVAO);

		glDrawArrays(GL_QUADS, 0, 4);
		glBindVertexArray(0);
		glDepthFunc(GL_LEQUAL);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Blur

	if (renderManager.renderingMode == RenderManager::RENDERING_MODE_BASIC || renderManager.renderingMode == RenderManager::RENDERING_MODE_SSAO) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		qglClearColor(QColor(0xFF, 0xFF, 0xFF));
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDisable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);

		glUseProgram(renderManager.programs["blur"]);
		glUniform2f(glGetUniformLocation(renderManager.programs["blur"], "pixelSize"), 2.0f / this->width(), 2.0f / this->height());
		//printf("pixelSize loc %d\n", glGetUniformLocation(vboRenderManager.programs["blur"], "pixelSize"));

		glUniform1i(glGetUniformLocation(renderManager.programs["blur"], "tex0"), 1);//COLOR
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDataTex[0]);

		glUniform1i(glGetUniformLocation(renderManager.programs["blur"], "tex1"), 2);//NORMAL
		glActiveTexture(GL_TEXTURE2);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDataTex[1]);

		/*glUniform1i(glGetUniformLocation(renderManager.programs["blur"], "tex2"), 3);
		glActiveTexture(GL_TEXTURE3);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDataTex[2]);*/

		glUniform1i(glGetUniformLocation(renderManager.programs["blur"], "depthTex"), 8);
		glActiveTexture(GL_TEXTURE8);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDepthTex);

		glUniform1i(glGetUniformLocation(renderManager.programs["blur"], "tex3"), 4);//AO
		glActiveTexture(GL_TEXTURE4);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragAOTex);

		if (renderManager.renderingMode == RenderManager::RENDERING_MODE_SSAO) {
			glUniform1i(glGetUniformLocation(renderManager.programs["blur"], "ssao_used"), 1); // ssao used
		}
		else {
			glUniform1i(glGetUniformLocation(renderManager.programs["blur"], "ssao_used"), 0); // no ssao
		}

		glBindVertexArray(renderManager.secondPassVAO);

		glDrawArrays(GL_QUADS, 0, 4);
		glBindVertexArray(0);
		glDepthFunc(GL_LEQUAL);

	}

	// REMOVE
	glActiveTexture(GL_TEXTURE0);
}

void GLWidget3D::updateStatusBar() {
	QString msg = QString("xrot: %1, yrot: %2, zrot: %3, pos: (%4, %5, %6), FOV: %7, Error: %8").arg(camera.xrot).arg(camera.yrot).arg(camera.zrot).arg(camera.pos.x).arg(camera.pos.y).arg(camera.pos.z).arg(camera.fovy).arg(estimation_error);
	mainWin->statusBar()->showMessage(msg);

	mainWin->parameterDialog->manual = true;
	mainWin->parameterDialog->ui.doubleSpinBoxXrot->setValue(camera.xrot);
	mainWin->parameterDialog->ui.doubleSpinBoxYrot->setValue(camera.yrot);
	mainWin->parameterDialog->ui.doubleSpinBoxZrot->setValue(camera.zrot);
	mainWin->parameterDialog->ui.doubleSpinBoxFOV->setValue(camera.fovy);
	if (estimated_pm_params.size() > 0) {
		mainWin->parameterDialog->ui.doubleSpinBoxDepth1->setValue(estimated_pm_params[0]);
		mainWin->parameterDialog->ui.doubleSpinBoxDepth2->setValue(estimated_pm_params[1]);
		mainWin->parameterDialog->ui.doubleSpinBoxHeight->setValue(estimated_pm_params[2]);
		mainWin->parameterDialog->ui.doubleSpinBoxWidth1->setValue(estimated_pm_params[3]);
		mainWin->parameterDialog->ui.doubleSpinBoxWidth2->setValue(estimated_pm_params[4]);
	}
	mainWin->parameterDialog->manual = false;
}

void GLWidget3D::shiftImage(int shift_x, int shift_y, QImage& image) {
	QImage newImage = image;
	QPainter painter(&image);
	painter.drawImage(shift_x, shift_y, newImage);
}

void GLWidget3D::shiftImageAndSilhouette(int shift_x, int shift_y, QImage& image, std::vector<Stroke>& silhouette) {
	// shift the silhouette
	for (int i = 0; i < silhouette.size(); ++i) {
		silhouette[i].start.x += shift_x;
		silhouette[i].start.y += shift_y;
		silhouette[i].end.x += shift_x;
		silhouette[i].end.y += shift_y;
	}

	float scale = std::min((float)width() / image.width(), (float)height() / image.height());
	int scaled_shift_x = (float)shift_x / scale;
	int scaled_shift_y = (float)shift_y / scale;

	// shift the image
	QImage newImage = image;
	QPainter painter(&image);
	painter.drawImage(scaled_shift_x, scaled_shift_y, newImage);
}

void GLWidget3D::rotateImage(float theta, const glm::vec2& center, QImage& image) {
	cv::Mat img = cv::Mat(image.height(), image.width(), CV_8UC4, const_cast<uchar*>(image.bits()), image.bytesPerLine()).clone();

	cv::Mat rot_mat = cv::getRotationMatrix2D(cv::Point(center.x, center.y), theta / M_PI * 180.0, 1);
	cv::warpAffine(img, img, rot_mat, img.size(), cv::INTER_LINEAR,	cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255, 255));

	image = QImage(img.data, img.cols, img.rows, static_cast<int>(img.step), QImage::Format_RGB32).copy();
}

/**
 * シルエットをcenterを中心にtheta [rad]反時計方向に回転させる。
 * ただし、シルエットのY座標の方向が逆なので、実際には-theta [rad]だけ反時計方向に回転させればよい。
 */
void GLWidget3D::rotateSilhouette(float theta, const glm::vec2& center, std::vector<Stroke>& silhouette) {
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
glm::vec2 GLWidget3D::getOffset(const cv::Mat& img) {
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
glm::vec2 GLWidget3D::getOffset(const std::vector<Stroke>& silhouette) {
	// compute the bbox
	glutils::BoundingBox bbox;
	for (auto stroke : silhouette) {
		bbox.addPoint(stroke.start);
		bbox.addPoint(stroke.end);
	}

	// compute the offset
	glm::vec2 offset;
	offset.x = bbox.center().x - width() * 0.5f;
	offset.y = bbox.center().y - height() * 0.5f;

	return offset;
}

glm::vec2 GLWidget3D::getCenter(const std::vector<Stroke>& silhouette) {
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
float GLWidget3D::getRotation(const std::vector<Stroke>& silhouette) {
	float min_theta = std::numeric_limits<float>::max();
	float max_theta = -std::numeric_limits<float>::max();
	int count = 0;
	for (auto line : silhouette) {
		if (line.type == Stroke::TYPE_VERTICAL) {
			std::cout << "Vertical!" << std::endl;
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
		std::cout << "Max theta: " << max_theta << std::endl;
		std::cout << "Min theta: " << min_theta << std::endl;
		return (max_theta + min_theta) * 0.5f - M_PI * 0.5f;
	}
}

/**
 * renderした画像とユーザ指定のシルエットとの差（距離）を返却する。
 * renderした画像がシルエットの中心からずれている場合は、シルエットを適宜ずらして距離を計算する。
 */
double GLWidget3D::distance(const std::vector<Stroke>& silhouette, const cv::Mat& renderedImg) {
	// compute the distance transform of the rendered image
	cv::Mat renderedImgGray;
	cv::cvtColor(renderedImg, renderedImgGray, cv::COLOR_BGRA2GRAY);
	cv::threshold(renderedImgGray, renderedImgGray, 250, 255, CV_THRESH_BINARY);
	cv::Mat renderedDistMap;
	cv::distanceTransform(renderedImgGray, renderedDistMap, CV_DIST_L2, 3);
	renderedDistMap.convertTo(renderedDistMap, CV_32F);

	// compute the offset of the rendered image
	glm::vec2 rendered_offset = getOffset(renderedImg);

	// compute the offset of the target silhouette
	glm::vec2 target_offset = getOffset(silhouette);

	// shift the silhouette
	std::vector<Stroke> shiftedSilhouette = silhouette;
	for (int i = 0; i < shiftedSilhouette.size(); ++i) {
		shiftedSilhouette[i].start.x += rendered_offset.x - target_offset.x;
		shiftedSilhouette[i].start.y += rendered_offset.y - target_offset.y;
		shiftedSilhouette[i].end.x += rendered_offset.x - target_offset.x;
		shiftedSilhouette[i].end.y += rendered_offset.y - target_offset.y;
	}


	// compute the distance transform of the target silhouette
	cv::Mat targetImg(height(), width(), CV_8U, cv::Scalar(255));
	for (auto line : shiftedSilhouette) {
		cv::line(targetImg, cv::Point(line.start.x, line.start.y), cv::Point(line.end.x, line.end.y), cv::Scalar(0), 1);
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

	return dist;
}

/**
 * Resize the canvas size while keeping the image size. 
 */
void GLWidget3D::resizeImageCanvasSize(QImage& image, int width, int height) {
	QImage tmp = image;
	image = QImage(width, height, tmp.format());
	QPainter painter(&image);
	painter.drawImage((image.width() - tmp.width()) / 2, (image.height() - tmp.height()) / 2, tmp);
}