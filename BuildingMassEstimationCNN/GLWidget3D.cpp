#include <iostream>
#include "GLWidget3D.h"
#include "MainWindow.h"
#include <GL/GLU.h>
//#include "Classifier.h"
#include <QDir>
#include <QFileInfoList>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "GrammarParser.h"
#include "Rectangle.h"
#include "GLUtils.h"
#include <opencv2/calib3d.hpp>
#include <map>

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

	// This is necessary to prevent the screen overdrawn by OpenGL
	setAutoFillBackground(false);

	// 光源位置をセット
	// ShadowMappingは平行光源を使っている。この位置から原点方向を平行光源の方向とする。
	light_dir = glm::normalize(glm::vec3(-4, -5, -8));

	// シャドウマップ用のmodel/view/projection行列を作成
	glm::mat4 light_pMatrix = glm::ortho<float>(-50, 50, -50, 50, 0.1, 200);
	glm::mat4 light_mvMatrix = glm::lookAt(-light_dir * 50.0f, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	light_mvpMatrix = light_pMatrix * light_mvMatrix;

	/*classifier = boost::shared_ptr<Classifer>(new Classifier("../models/deploy.prototxt",
		"../models/train.caffemodel",
		"../models/mean.binaryproto",
		"../models/words.txt"));*/


	regression = boost::shared_ptr<Regression>(new Regression("../models/deploy_1.prototxt", "../models/contour1_iter_240000.caffemodel"));


}

/**
 * Clear the canvas.
 */
void GLWidget3D::clearSketch() {
	sketch.clear();

	update();
}

void GLWidget3D::clearGeometry() {
	renderManager.removeObjects();
	renderManager.updateShadowMap(this, light_dir, light_mvpMatrix);
	update();
}

void GLWidget3D::loadContour(const std::string& filename) {
	sketch.clear();

	std::ifstream in(filename);
	
	while (!in.eof()) {
		Stroke stroke;
		if (!(in >> stroke.start.x >> stroke.start.y >> stroke.end.x >> stroke.end.y)) break;
		
		sketch.push_back(stroke);
	}

	update();
}

void GLWidget3D::saveContour(const std::string& filename) {
	std::ofstream out(filename);
	for (auto stroke : sketch) {
		out << stroke.start.x << "\t" << stroke.start.y << "\t" << stroke.end.x << "\t" << stroke.end.y << "\n";
	}
	out.close();
}

/**
* Load a sketch image from a file, and display options order by their probabilities.
* This is for test usage.
*/
void GLWidget3D::loadImage(const std::string& filename) {
	QImage newImage;
	newImage.load(filename.c_str());

	float scale = std::min((float)width() / newImage.width(), (float)height() / newImage.height());
	newImage = newImage.scaled(newImage.width() * scale, newImage.height() * scale);

	bgImage = QImage(width(), height(), QImage::Format_RGB32);
	QPainter painter(&bgImage);
	painter.drawImage((width() - newImage.width()) / 2, (height() - newImage.height()) / 2, newImage);
	
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
	if (sketch.size() > 0) {
		sketch.pop_back();
		update();
	}
}

/**
 * Use the sketch as an input to the pretrained network, and obtain the probabilities as output.
 * Then, display the options ordered by the probabilities.
 */
void GLWidget3D::parameterEstimation(bool centering3D, bool meanSubtraction, int cameraType, float cameraDistanceBase, float cameraHeight, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int fovMin, int fovMax) {
	// compute the bbox
	glutils::BoundingBox bbox;
	for (auto stroke : sketch) {
		bbox.addPoint(stroke.start);
		bbox.addPoint(stroke.end);
	}

	// compute the offset
	glm::vec2 offset;
	offset.x = width() * 0.5f - bbox.center().x;
	offset.y = height() * 0.5f - bbox.center().y;

	// shift the sketch
	for (int i = 0; i < sketch.size(); ++i) {
		sketch[i].start.x += offset.x;
		sketch[i].start.y += offset.y;
		sketch[i].end.x += offset.x;
		sketch[i].end.y += offset.y;
	}

	// shift the image
	QImage newImage = bgImage;
	QPainter painter(&bgImage);
	painter.drawImage(offset.x, offset.y, newImage);

	// scale the contour to 128x128 size
	glm::vec2 scale(128.0 / width(), 128.0 / height());

	std::vector<Stroke> scaledSketch;
	for (auto stroke : sketch) {
		Stroke s;
		s.start = glm::vec2(stroke.start.x * scale.x, stroke.start.y * scale.y);
		s.end = glm::vec2(stroke.end.x * scale.x, stroke.end.y * scale.y);
		scaledSketch.push_back(s);
	}

	// create input image
	cv::Mat input(128, 128, CV_8U, cv::Scalar(255));
	for (auto stroke : scaledSketch) {
		cv::line(input, cv::Point(stroke.start.x, stroke.start.y), cv::Point(stroke.end.x, stroke.end.y), cv::Scalar(0), 1, cv::LINE_AA);
	}
	//cv::imwrite("input.png", input);

	if (meanSubtraction) {
		cv::Mat meanImg = cv::imread("../models/mean.png");
		cv::cvtColor(meanImg, meanImg, cv::COLOR_BGR2GRAY);
		input -= meanImg;
	}


	//input = cv::imread("..\\photos\\image_001550.png");
	//std::cout << "ch: " << input.channels() << std::endl;


	// estimate parameters
	std::vector<float> params = regression->Predict(input);
	for (int i = 0; i < params.size(); ++i) {
		if (i > 0) std::cout << ",";
		std::cout << params[i];
	}
	std::cout << std::endl;

	// setup the camera
	if (xrotMin != xrotMax && yrotMin != yrotMax) {
		camera.xrot = xrotMin + (xrotMax - xrotMin) * params[0];
		camera.yrot = yrotMin + (yrotMax - yrotMin) * params[1];
		params.erase(params.begin());
		params.erase(params.begin());
	}
	else {
		camera.xrot = xrotMin;
		camera.yrot = yrotMin;
	}
	camera.zrot = 0;
	if (fovMin != fovMax) {
		camera.fovy = fovMin + params[0] * (fovMax - fovMin);
		params.erase(params.begin());

	}
	else {
		camera.fovy = fovMin;
	}
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
	QString cga_file = QString("..\\cga\\contour_01.xml");
	cga::CGA cga;
	cga::Grammar grammar;
	cga.modelMat = glm::rotate(glm::mat4(), -(float)M_PI * 0.5f, glm::vec3(1, 0, 0));
	cga::parseGrammar(cga_file.toUtf8().constData(), grammar);

	// set parameters
	cga.setParamValues(grammar, params);

	// set axiom
	cga::Rectangle* start = new cga::Rectangle("Start", "", glm::translate(glm::rotate(glm::mat4(), -3.141592f * 0.5f, glm::vec3(1, 0, 0)), glm::vec3(0, 0, 0)), glm::mat4(), 0, 0, glm::vec3(1, 1, 1));
	cga.stack.push_back(boost::shared_ptr<cga::Shape>(start));

	// generate 3d model
	renderManager.removeObjects();
	cga.derive(grammar, true);
	std::vector<boost::shared_ptr<glutils::Face> > faces;
	cga.generateGeometry(faces, centering3D);
	renderManager.addFaces(faces, true);

	updateStatusBar();
	update();
}

/**
* Use the sketch as an input to the pretrained network, and obtain the probabilities as output.
* Then, display the options ordered by the probabilities.
*/
void GLWidget3D::parameterEstimation2(bool centering3D, bool meanSubtraction, int cameraType, float cameraDistanceBase, float cameraHeight, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int fovMin, int fovMax) {
	// compute the bbox
	glutils::BoundingBox bbox;
	for (auto stroke : sketch) {
		bbox.addPoint(stroke.start);
		bbox.addPoint(stroke.end);
	}

	// compute the offset
	glm::vec2 offset;
	offset.x = width() * 0.5f - bbox.center().x;
	offset.y = height() * 0.5f - bbox.center().y;

	// shift the sketch
	for (int i = 0; i < sketch.size(); ++i) {
		sketch[i].start.x += offset.x;
		sketch[i].start.y += offset.y;
		sketch[i].end.x += offset.x;
		sketch[i].end.y += offset.y;
	}

	// shift the image
	QImage newImage = bgImage;
	QPainter painter(&bgImage);
	painter.drawImage(offset.x, offset.y, newImage);

	// scale the contour to 128x128 size
	glm::vec2 scale(128.0 / width(), 128.0 / height());

	std::vector<Stroke> scaledSketch;
	for (auto stroke : sketch) {
		Stroke s;
		s.start = glm::vec2(stroke.start.x * scale.x, stroke.start.y * scale.y);
		s.end = glm::vec2(stroke.end.x * scale.x, stroke.end.y * scale.y);
		scaledSketch.push_back(s);
	}

	// create input image
	cv::Mat input(128, 128, CV_8U, cv::Scalar(255));
	for (auto stroke : scaledSketch) {
		cv::line(input, cv::Point(stroke.start.x, stroke.start.y), cv::Point(stroke.end.x, stroke.end.y), cv::Scalar(0), 1, cv::LINE_AA);
	}
	//cv::imwrite("input.png", input);

	if (meanSubtraction) {
		cv::Mat meanImg = cv::imread("../models/mean.png");
		cv::cvtColor(meanImg, meanImg, cv::COLOR_BGR2GRAY);
		input -= meanImg;
	}



	///////////////////////////////////////////////////////////////////////////////////
	// contourの頂点の座標を取得
	std::vector<std::vector<cv::Point2f>> imagePoints(1);
	for (int i = 0; i < scaledSketch.size(); ++i) {
		cv::Point2f p1(scaledSketch[i].start.x / 64.0 - 1.0, 1 - scaledSketch[i].start.y / 64.0);
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

		cv::Point2f p2(scaledSketch[i].end.x / 64.0 - 1.0, 1 - scaledSketch[i].end.y / 64.0);
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




	//input = cv::imread("..\\photos\\image_001550.png");
	//std::cout << "ch: " << input.channels() << std::endl;


	// estimate parameters
	std::vector<float> params = regression->Predict(input);
	for (int i = 0; i < params.size(); ++i) {
		if (i > 0) std::cout << ",";
		std::cout << params[i];
	}
	std::cout << std::endl;

	// setup the camera
	if (xrotMin != xrotMax && yrotMin != yrotMax) {
		camera.xrot = xrotMin + (xrotMax - xrotMin) * params[0];
		camera.yrot = yrotMin + (yrotMax - yrotMin) * params[1];
		params.erase(params.begin());
		params.erase(params.begin());
	}
	else {
		camera.xrot = xrotMin;
		camera.yrot = yrotMin;
	}
	camera.zrot = 0;
	if (fovMin != fovMax) {
		camera.fovy = fovMin + params[0] * (fovMax - fovMin);
		params.erase(params.begin());

	}
	else {
		camera.fovy = fovMin;
	}
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
	QString cga_file = QString("..\\cga\\contour_01.xml");
	cga::CGA cga;
	cga::Grammar grammar;
	cga.modelMat = glm::rotate(glm::mat4(), -(float)M_PI * 0.5f, glm::vec3(1, 0, 0));
	cga::parseGrammar(cga_file.toUtf8().constData(), grammar);

	// set parameters
	cga.setParamValues(grammar, params);

	// set axiom
	cga::Rectangle* start = new cga::Rectangle("Start", "", glm::translate(glm::rotate(glm::mat4(), -3.141592f * 0.5f, glm::vec3(1, 0, 0)), glm::vec3(0, 0, 0)), glm::mat4(), 0, 0, glm::vec3(1, 1, 1));
	cga.stack.push_back(boost::shared_ptr<cga::Shape>(start));

	// generate 3d model
	renderManager.removeObjects();
	cga.derive(grammar, true);
	std::vector<boost::shared_ptr<glutils::Face> > faces;
	cga.generateGeometry(faces, centering3D);
	renderManager.addFaces(faces, true);


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
	for (int i = 0; i < objectPoints[0].size(); ++i) {
		std::cout << "(" << imagePoints[0][i].x << "," << imagePoints[0][i].y << ") --> (" << objectPoints[0][i].x << "," << objectPoints[0][i].y << "," << objectPoints[0][i].z << ")" << std::endl;
	}

	// Camera calibration
	cv::Mat cameraMat = cv::Mat::eye(3, 3, CV_64F);
	cameraMat.at<double>(0, 0) = camera.pMatrix[0][0];
	cameraMat.at<double>(1, 1) = camera.pMatrix[1][1];
	cameraMat.at<double>(2, 0) = camera.pMatrix[2][0];
	cameraMat.at<double>(2, 1) = camera.pMatrix[2][1];
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





	updateStatusBar();
	update();
}

void GLWidget3D::parameterEstimation3(bool centering3D, bool meanSubtraction, int cameraType, float cameraDistanceBase, float cameraHeight, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int fovMin, int fovMax) {



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
		sketch.push_back(currentStroke);
		currentStroke = Stroke();

		dragging = false;
	}
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


	//sketch = QImage(this->width(), this->height(), QImage::Format_RGB888);
	//sketch.fill(qRgba(255, 255, 255, 255));
}

/**
 * This function is called whenever the widget has been resized.
 */
void GLWidget3D::resizeGL(int width, int height) {
	height = height ? height : 1;
	glViewport(0, 0, width, height);
	camera.updatePMatrix(width, height);

	renderManager.resize(width, height);
	
	/*
	QImage newImage(width, height, QImage::Format_RGB888);
	newImage.fill(qRgba(255, 255, 255, 255));
	QPainter painter(&newImage);

	painter.drawImage(0, 0, sketch);
	sketch = newImage;
	*/
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

	// draw sketch
	QPainter painter(this);
	painter.setOpacity(0.5);
	painter.drawImage(0, 0, bgImage);

	//painter.setPen(QPen(Qt::black, 3));
	painter.setPen(QPen(QColor(0, 0, 255), 3));
	for (auto stroke : sketch) {
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
	QString msg = QString("xrot: %1, yrot: %2, zrot: %3, pos: (%4, %5, %6), FOV: %7").arg(camera.xrot).arg(camera.yrot).arg(camera.zrot).arg(camera.pos.x).arg(camera.pos.y).arg(camera.pos.z).arg(camera.fovy);
	mainWin->statusBar()->showMessage(msg);
}
