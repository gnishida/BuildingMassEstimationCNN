#include <iostream>
#include "GLWidget3D.h"
#include "MainWindow.h"
#include <GL/GLU.h>
#include <QDir>
#include <QFileInfoList>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include "GrammarParser.h"
#include "Rectangle.h"
#include "GLUtils.h"
#include <opencv2/calib3d.hpp>
#include <map>
#include "CVUtils.h"
#include "Utils.h"
#include <QFile>
#include <QTextStream>

GLWidget3D::GLWidget3D(QWidget *parent) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent) {
	mainWin = (MainWindow*)parent;
	ctrlPressed = false;
	shiftPressed = false;
	altPressed = false;

	opacityOfBackground = 0.5f;
	pen_type = PEN_TYPE_VANISHING_LINE;
	lineWidth = 3;
	horizontalLeftColor = QColor(0, 0, 192);
	horizontalRightColor = QColor(64, 64, 255);
	verticalColor = QColor(140, 140, 255);
	silhouetteWidth = 3;
	silhouetteColor = QColor(255, 0, 0);

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
		regressions.push_back(boost::shared_ptr<Regression>(new Regression("models/deploy_01.prototxt", "models/train_01_iter_80000.caffemodel")));
		regressions.push_back(boost::shared_ptr<Regression>(new Regression("models/deploy_02.prototxt", "models/train_02_iter_80000.caffemodel")));
		regressions.push_back(boost::shared_ptr<Regression>(new Regression("models/deploy_03.prototxt", "models/train_03_iter_80000.caffemodel")));
		regressions.push_back(boost::shared_ptr<Regression>(new Regression("models/deploy_04.prototxt", "models/train_04_iter_80000.caffemodel")));
	}

	// Grammarを読み込む
	{
		grammars.resize(4);
		cga::parseGrammar("cga/contour_01.xml", grammars[0]);
		cga::parseGrammar("cga/contour_02.xml", grammars[1]);
		cga::parseGrammar("cga/contour_03.xml", grammars[2]);
		cga::parseGrammar("cga/contour_04.xml", grammars[3]);
	}

	// default grammar and pm values
	grammar_id = 0;
	pm_params.resize(grammars[0].attrs.size(), 0.5);
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

void GLWidget3D::clearBackground() {
	image_loaded = false;
	image = QImage(width(), height(), QImage::Format_RGB32);
	imageOrig = image;
	mainWin->setWindowTitle("Photo to 3D");

	update();
}

void GLWidget3D::loadImage(const QString& filename) {
	imageOrig.load(filename);
	image_loaded = true;

	float scale = std::min((float)width() / imageOrig.width(), (float)height() / imageOrig.height());
	QImage newImage = imageOrig.scaled(imageOrig.width() * scale, imageOrig.height() * scale);

	image = QImage(width(), height(), QImage::Format_RGB32);
	QPainter painter(&image);
	painter.drawImage((width() - newImage.width()) / 2, (height() - newImage.height()) / 2, newImage);

	opacityOfBackground = 0.5f;

	mainWin->setWindowTitle(QString("Photo to 3D - ") + filename);

	update();
}

void GLWidget3D::clearLines() {
	lines.clear();
	update();
}

void GLWidget3D::loadLines(const QString& filename) {
	lines.clear();

	QFile file(filename);
	if (file.open(QIODevice::ReadOnly)) {
		QTextStream in(&file);

		while (true) {
			QString line = in.readLine();
			if (line.isNull()) break;
			QStringList list = line.split(QRegExp("(\t| )"));

			if (list.size() == 2) {
				origin = glm::dvec2(list[0].toFloat(), list[1].toFloat());
			}
			else {
				glm::dvec2 start(list[0].toFloat(), list[1].toFloat());
				glm::dvec2 end(list[2].toFloat(), list[3].toFloat());
				if (start != end) {
					lines.push_back(vp::VanishingLine(start.x, start.y, end.x, end.y, list[4].toInt()));
				}
			}
		}

		file.close();
	}

	update();
}

void GLWidget3D::saveLines(const QString& filename) {
	QFile file(filename);
	if (file.open(QIODevice::WriteOnly)) {
		QTextStream out(&file);

		out << origin.x << " " << origin.y << "\n";
		for (auto line : lines) {
			out << line.start.x << " " << line.start.y << " " << line.end.x << " " << line.end.y << " " << line.type << "\n";
		}
	}
	file.close();
}

void GLWidget3D::clearSilhouette() {
	silhouette.clear();
	update();
}

void GLWidget3D::loadSilhouette(const QString& filename) {
	silhouette.clear();

	QFile file(filename);
	if (file.open(QIODevice::ReadOnly)) {
		QTextStream in(&file);

		while (true) {
			QString line = in.readLine();
			if (line.isNull()) break;
			QStringList list = line.split(QRegExp("(\t| )"));

			glm::dvec2 start(list[0].toFloat(), list[1].toFloat());
			glm::dvec2 end(list[2].toFloat(), list[3].toFloat());
			if (start != end) {
				silhouette.push_back(vp::VanishingLine(start.x, start.y, end.x, end.y, list[4].toInt()));
			}
		}

		file.close();
	}
	update();
}

void GLWidget3D::saveSilhouette(const QString& filename) {
	QFile file(filename);
	if (file.open(QIODevice::WriteOnly)) {
		QTextStream out(&file);

		for (auto line : silhouette) {
			out << line.start.x << " " << line.start.y << " " << line.end.x << " " << line.end.y << " " << line.type << "\n";
		}
	}
	file.close();
}

void GLWidget3D::clearGeometry() {
	renderManager.removeObjects();
	renderManager.updateShadowMap(this, light_dir, light_mvpMatrix);
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
	if (pen_type == PEN_TYPE_VANISHING_LINE) {
		if (lines.size() > 0) {
			lines.pop_back();
			update();
		}
	}
	else if (pen_type == PEN_TYPE_SILHOUETTE) {
		if (silhouette.size() > 0) {
			silhouette.pop_back();
			update();
		}
	}
}

/**
 * Use the silhouette as an input to the pretrained network, and obtain the probabilities as output.
 * Then, display the options ordered by the probabilities.
 */
void GLWidget3D::parameterEstimation(bool automaticRecognition, int grammarSnippetId, int image_size, bool grayscale, bool centering3D, bool meanSubtraction, float cameraDistanceBase, float xrotMin, float xrotMax, float yrotMin, float yrotMax, float zrotMin, float zrotMax, float fovMin, float fovMax, float oxMin, float oxMax, float oyMin, float oyMax, float xMin, float xMax, float yMin, float yMax, bool tryMultiples, int numMultipleTries, float maxNoise, bool refinement, int maxIters, bool applyTexture) {
	time_t start = clock();

	std::cout << "-----------------------------------------------------" << std::endl;

	camera.distanceBase = cameraDistanceBase;

	// adjust the original background image such that the ratio of width to height is equal to the ratio of the window
	float imageScale = std::min((float)width() / imageOrig.width(), (float)height() / imageOrig.height());
	resizeImageCanvasSize(imageOrig, width() / imageScale, height() / imageScale);

	// grammar id
	grammar_id = grammarSnippetId;




	// create image of silhouette
	cv::Mat silhouette_image(height(), width(), CV_8UC1, cv::Scalar(255));
	for (auto line : silhouette) {
		cv::line(silhouette_image, cv::Point(line.start.x, line.start.y), cv::Point(line.end.x, line.end.y), cv::Scalar(0), 1);
	}

	// create distance map of silhouette
	cv::Mat silhouette_dist_map;
	cv::threshold(silhouette_image, silhouette_image, 254, 255, CV_THRESH_BINARY);
	cv::distanceTransform(silhouette_image, silhouette_dist_map, CV_DIST_L2, 3);
	silhouette_dist_map.convertTo(silhouette_dist_map, CV_64F);







	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// parameter estimation by CNN

	// create input image to CNN
	glm::vec2 scale((float)image_size / width(), (float)image_size / height());
	cv::Mat input = cv::Mat(image_size, image_size, CV_8UC3, cv::Scalar(255, 255, 255));
	std::vector<vp::VanishingLine> scaledSilhouette;
	for (auto stroke : silhouette) {
		cv::line(input, cv::Point(stroke.start.x * scale.x, stroke.start.y * scale.y), cv::Point(stroke.end.x * scale.x, stroke.end.y * scale.y), cv::Scalar(0, 0, 0), 1, cv::LINE_AA);
	}
	cv::imwrite("input.png", input);

	// estimate paramter values by CNN
	std::vector<float> params = regressions[grammar_id]->Predict(input);
	utils::output_vector(params);


	// 固定の場合には、ダミーでパラメータを入れちゃう
	if (xrotMin == xrotMax) {
		params.insert(params.begin() + 0, 0.5);
	}
	if (yrotMin == yrotMax) {
		params.insert(params.begin() + 1, 0.5);
	}
	if (zrotMin == zrotMax) {
		params.insert(params.begin() + 2, 0.5);
	}
	if (fovMin == fovMax) {
		params.insert(params.begin() + 3, 0.5);
	}
	if (oxMin == oxMax) {
		params.insert(params.begin() + 4, 0.5);
	}
	if (oyMin == oyMax) {
		params.insert(params.begin() + 5, 0.5);
	}
	if (xMin == xMax) {
		params.insert(params.begin() + 6, 0.5);
	}
	if (yMin == yMax) {
		params.insert(params.begin() + 7, 0.5);
	}




	/////////////////////////////////////////////////////////////////////////
	// compute camera parameters
	if (lines.size() > 0) { // use the vanishing points to estimate camera parameters
		std::vector<glm::dvec2> vps;
		vp::computeVanishingPoints(lines, vps);

		// convert the coordinates of vp to [-1, 1]
		for (int i = 0; i < vps.size(); ++i) {
			vps[i].x = vps[i].x / width() * 2.0 - 1;
			vps[i].y = 1 - vps[i].y / height() * 2.0;
		}

		double f, xrot, yrot, zrot;
		vp::extractCameraParameters(vps, f, xrot, yrot, zrot, camera.center);
		camera.fovy = vp::rad2deg(atan2(1.0, f) * 2);
		camera.xrot = vp::rad2deg(xrot);
		camera.yrot = vp::rad2deg(yrot);
		camera.zrot = vp::rad2deg(zrot);

		// update camera
		camera.updatePMatrix(width(), height());
	}
	else { // use the CNN to estimate camera parameters
		camera.xrot = (xrotMax - xrotMin) * params[0] + xrotMin;
		camera.yrot = (yrotMax - yrotMin) * params[1] + yrotMin;
		camera.zrot = (zrotMax - zrotMin) * params[2] + zrotMin;
		camera.fovy = (fovMax - fovMin) * params[3] + fovMin;
		camera.center.x = (oxMax - oxMin) * params[4] + oxMin;
		camera.center.y = (oyMax - oyMin) * params[5] + oyMin;
	}



	/*
	// Since the x/y pos are the relative to the z pos, recover the actual coordinates.
	double camera_distance = camera.distanceBase / tan(vp::deg2rad(camera.fovy * 0.5));
	params[4] = (params[4] * (xMax - xMin) + xMin) * camera_distance;
	params[5] = (params[5] * (yMax - yMin) + yMin) * camera_distance;
	//std::cout << params[4] << ", " << params[5] << std::endl;

	// Convert the x/y pos to the origin position on the image plane [0, 1]
	// where 0 means the left or bottom and 1 means the right or top edge.
	params[4] = (-params[4] / camera_distance * camera.f() + camera.center.x + 1) * 0.5;
	params[5] = (-params[5] / camera_distance * camera.f() + camera.center.y + 1) * 0.5;
	//std::cout << params[4] << ", " << params[5] << std::endl;
	*/


	// Put the param values to init params but discard the first 6 parameters.
	// We refine only the camera pos and PM parameters
	std::vector<float> init_params;
	for (int k = 6; k < params.size(); ++k) {
		init_params.push_back(params[k]);
	}

	std::vector<float> best_params = init_params;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// refine the parameter estimation
	if (refinement) {
		renderManager.renderingMode = RenderManager::RENDERING_MODE_CONTOUR;

		double diff_min = std::numeric_limits<double>::max();
		printf("find good values by random sampling: ");
		for (int iter = 0; iter < maxIters; ++iter) {
			printf("\rfind good initial values by random sampling: %d", iter + 1);

			// randomly pick the initial values
			std::vector<float> cur_params(init_params.size());
			for (int k = 0; k < cur_params.size(); ++k) {
				//cur_params[k] = utils::genRand(0.0, 1.0);
				cur_params[k] = utils::genRand(init_params[k] - 0.2, init_params[k] + 0.2);
			}

			// compute the camera pos
			camera.pos.x = cur_params[0] * (xMax - xMin) + xMin;
			camera.pos.y = cur_params[1] * (xMax - xMin) + xMin;

			// update camera
			camera.updatePMatrix(width(), height());

			cv::Mat rendered_image;
			renderImage(grammars[grammar_id], std::vector<float>(cur_params.begin() + 2, cur_params.end()), rendered_image);

			// compute the difference
			double diff = distanceMap(rendered_image, silhouette_dist_map);

			// coordinate descent
			float delta = 0.1;
			for (int iter2 = 0; iter2 < 20; ++iter2) {
				for (int k = 0; k < cur_params.size(); ++k) {
					// option 1
					std::vector<float> next_params1 = cur_params;
					next_params1[k] -= delta;
					if (k < 2) {
						glm::dvec3 T;
						double camera_distance = camera.distanceBase / tan(vp::deg2rad(camera.fovy * 0.5));
						vp::extractCameraMatrixT(camera.f(), glm::vec2(next_params1[0] * 2 - 1, next_params1[1] * 2 - 1) - camera.center, camera_distance, T);
						camera.pos = glm::vec3(-T.x, -T.y, -T.z);

						// update camera
						camera.updatePMatrix(width(), height());
					}
					cv::Mat rendered_image1;
					renderImage(grammars[grammar_id], std::vector<float>(next_params1.begin() + 2, next_params1.end()), rendered_image1);
					double diff1 = distanceMap(rendered_image1, silhouette_dist_map);

					// option 2
					std::vector<float> next_params2 = cur_params;
					next_params2[k] += delta;
					if (k < 2) {
						glm::dvec3 T;
						double camera_distance = camera.distanceBase / tan(vp::deg2rad(camera.fovy * 0.5));
						vp::extractCameraMatrixT(camera.f(), glm::vec2(next_params2[0] * 2 - 1, next_params2[1] * 2 - 1) - camera.center, camera_distance, T);
						camera.pos = glm::vec3(-T.x, -T.y, -T.z);

						// update camera
						camera.updatePMatrix(width(), height());
					}
					cv::Mat rendered_image2;
					renderImage(grammars[grammar_id], std::vector<float>(next_params2.begin() + 2, next_params2.end()), rendered_image2);
					double diff2 = distanceMap(rendered_image2, silhouette_dist_map);

					if (diff1 < diff2 && diff1 < diff) {
						diff = diff1;
						cur_params = next_params1;
					}
					else if (diff2 < diff1 && diff2 < diff) {
						diff = diff2;
						cur_params = next_params2;
					}
				}

				delta *= 0.8;
			}

			if (diff < diff_min) {
				diff_min = diff;
				best_params = cur_params;
			}
		}
		printf("\n");


		renderManager.renderingMode = RenderManager::RENDERING_MODE_LINE;
	}


	// Since the x/y pos are the relative to the z pos, recover the actual coordinates.
	double camera_distance = camera.distanceBase / tan(vp::deg2rad(camera.fovy * 0.5));
	camera.pos.x = best_params[0] * (xMax - xMin) + xMin;
	camera.pos.y = best_params[1] * (yMax - yMin) + yMin;
	camera.pos.z = camera.distanceBase / tan(vp::deg2rad(camera.fovy * 0.5));
	//std::cout << best_params[0] << ", " << best_params[1] << std::endl;

	// set the camera pos
	/*
	glm::dvec3 T;
	camera_distance = camera.distanceBase / tan(vp::deg2rad(camera.fovy * 0.5));
	vp::extractCameraMatrixT(camera.f(), glm::vec2(best_params[0] * 2 - 1, best_params[1] * 2 - 1) - camera.center, camera_distance, T);
	camera.pos = glm::vec3(-T.x, -T.y, -T.z);
	*/


	/*
	camera.pos.x = best_params[0] * (xMax - xMin) + xMin;
	camera.pos.y = best_params[1] * (yMax - yMin) + yMin;
	camera.pos.z = camera.distanceBase / tan(vp::deg2rad(camera.fovy * 0.5));
	*/
	camera.updatePMatrix(width(), height());

	// set PM parameter values
	pm_params.clear();
	for (int k = 2; k < best_params.size(); ++k) {
		pm_params.push_back(best_params[k]);
	}

	updateGeometry(grammars[grammar_id], pm_params);

	updateStatusBar();
	update();

	time_t end = clock();
	std::cout << (double)(end - start) / CLOCKS_PER_SEC << "sec." << std::endl;

#if 0	

	std::cout << "Final dist: " << bme::distance(silhouette, renderedImage, width(), height()) << std::endl;

	// line modeで描画
	renderManager.renderingMode = RenderManager::RENDERING_MODE_LINE;

	if (applyTexture) {
		bme::generateTextures(camera, bgImageOrig, width(), height(), faces);

		renderManager.removeObjects();
		renderManager.addFaces(faces, true);
		renderManager.renderingMode = RenderManager::RENDERING_MODE_BASIC;

		opacityOfBackground = 0.1f;

		// remove the texture folder
		//if (QDir("textures").exists()) QDir("textures").removeRecursively();
	}

	updateStatusBar();
	update();
#endif

}

/**
 * Generate an image using the selected grammar and the PM parameter values.
 *
 * @param grammar			grammar
 * @param pm_params			PM parameter values
 * @param rendered_image	rendered image [OUT]
 */
void GLWidget3D::renderImage(cga::Grammar& grammar, const std::vector<float>& pm_params, cv::Mat& rendered_image) {
	updateGeometry(grammar, pm_params);

	render();
	QImage img = grabFrameBuffer();
	rendered_image = cv::Mat(img.height(), img.width(), CV_8UC4, img.bits(), img.bytesPerLine()).clone();
	cv::cvtColor(rendered_image, rendered_image, cv::COLOR_BGRA2BGR);
}

/**
 * Compute the distance between the rendered image and the target.
 *
 * @param rendered_image		rendered image
 * @param reference_dist_map	distance map of the target
 * @return						distance
 */
double GLWidget3D::distanceMap(cv::Mat rendered_image, const cv::Mat& reference_dist_map) {
	cv::cvtColor(rendered_image, rendered_image, CV_BGR2GRAY);

	// compute the distance map
	cv::Mat rendered_dist_map;
	cv::threshold(rendered_image, rendered_image, 254, 255, CV_THRESH_BINARY);
	cv::distanceTransform(rendered_image, rendered_dist_map, CV_DIST_L2, 3);
	rendered_dist_map.convertTo(rendered_dist_map, CV_64F);

	// compute the squared difference
	cv::Mat diff_mat;
	cv::reduce((reference_dist_map - rendered_dist_map).mul(reference_dist_map - rendered_dist_map), diff_mat, 0, CV_REDUCE_SUM);
	cv::reduce(diff_mat, diff_mat, 1, CV_REDUCE_SUM);

	return diff_mat.at<double>(0, 0);
}

/**
 * Update the geometry on GPU using the selected grammar and the PM parameter values.
 *
 * @param grammar		grammar
 * @param pm_params		PM parameter values
 */
void GLWidget3D::updateGeometry(cga::Grammar& grammar, const std::vector<float>& pm_params) {
	std::vector<boost::shared_ptr<glutils::Face>> faces;

	// setup CGA
	cga::CGA cga;
	cga.modelMat = glm::rotate(glm::mat4(), -(float)vp::M_PI * 0.5f, glm::vec3(1, 0, 0));
	cga.setParamValues(grammar, pm_params);

	// set axiom
	boost::shared_ptr<cga::Shape> start = boost::shared_ptr<cga::Shape>(new cga::Rectangle("Start", "", glm::translate(glm::rotate(glm::mat4(), -(float)vp::M_PI * 0.5f, glm::vec3(1, 0, 0)), glm::vec3(0, 0, 0)), glm::mat4(), 0, 0, glm::vec3(1, 1, 1)));
	cga.stack.push_back(start);

	// generate 3d model
	cga.derive(grammar, true);
	cga.generateGeometry(faces, true);
	renderManager.removeObjects();
	renderManager.addFaces(faces, true);
}

void GLWidget3D::updateStatusBar() {
	QString format("rot=(%1, %2, %3), pos=(%4, %5, %6), fov=%7, O=(%8, %9), PM=(");
	for (int i = 0; i < pm_params.size(); ++i) {
		if (i > 0) format += ", ";
		format += "%" + QString::number(10 + i);
	}
	format += ")";

	QString msg = format.arg(camera.xrot).arg(camera.yrot).arg(camera.zrot).arg(camera.pos.x).arg(camera.pos.y).arg(camera.pos.z).arg(camera.fovy).arg(camera.center.x).arg(camera.center.y);

	// add PM parameter values (instead of normalized ones!)
	int k = 0;
	for (auto it = grammars[grammar_id].attrs.begin(); it != grammars[grammar_id].attrs.end(); ++it, ++k) {
		msg = msg.arg(pm_params[k] * (it->second.range_end - it->second.range_start) + it->second.range_start);
	}

	mainWin->statusBar()->showMessage(msg);
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

void GLWidget3D::keyPressEvent(QKeyEvent *e) {
	ctrlPressed = false;

	if (e->modifiers() == Qt::ControlModifier) {
		ctrlPressed = true;
	}

	switch (e->key()) {
	case Qt::Key_Shift:
		shiftPressed = true;
		break;
	case Qt::Key_Alt:
		altPressed = true;
		break;
	case Qt::Key_Delete:
		clearGeometry();
		break;
	case Qt::Key_1:
		if (pm_params.size() > 0) {
			pm_params[0] += 0.01 * (ctrlPressed ? 0.1 : 1) * (altPressed ? -1 : 1);
			updateGeometry(grammars[grammar_id], pm_params);
			updateStatusBar();
			update();
		}
		break;
	case Qt::Key_2:
		if (pm_params.size() > 1) {
			pm_params[1] += 0.01 * (ctrlPressed ? 0.1 : 1) * (altPressed ? -1 : 1);
			updateGeometry(grammars[grammar_id], pm_params);
			updateStatusBar();
			update();
		}
		break;
	case Qt::Key_3:
		if (pm_params.size() > 2) {
			pm_params[2] += 0.01 * (ctrlPressed ? 0.1 : 1) * (altPressed ? -1 : 1);
			updateGeometry(grammars[grammar_id], pm_params);
			updateStatusBar();
			update();
		}
		break;
	case Qt::Key_4:
		if (pm_params.size() > 3) {
			pm_params[3] += 0.01 * (ctrlPressed ? 0.1 : 1) * (altPressed ? -1 : 1);
			updateGeometry(grammars[grammar_id], pm_params);
			updateStatusBar();
		}
		update();
		break;
	case Qt::Key_5:
		if (pm_params.size() > 4) {
			pm_params[4] += 0.01 * (ctrlPressed ? 0.1 : 1) * (altPressed ? -1 : 1);
			updateGeometry(grammars[grammar_id], pm_params);
			updateStatusBar();
		}
		update();
		break;
	case Qt::Key_Left:
		camera.pos.x += 0.5 * (ctrlPressed ? 0.1 : 1);
		camera.updateMVPMatrix();
		updateStatusBar();
		update();
		break;
	case Qt::Key_Right:
		camera.pos.x -= 0.5 * (ctrlPressed ? 0.1 : 1);
		camera.updateMVPMatrix();
		updateStatusBar();
		update();
		break;
	case Qt::Key_Up:
		camera.pos.y -= 0.5 * (ctrlPressed ? 0.1 : 1);
		camera.updateMVPMatrix();
		updateStatusBar();
		update();
		break;
	case Qt::Key_Down:
		camera.pos.y += 0.5 * (ctrlPressed ? 0.1 : 1);
		camera.updateMVPMatrix();
		updateStatusBar();
		update();
		break;
	default:
		break;
	}
}

void GLWidget3D::keyReleaseEvent(QKeyEvent* e) {
	switch (e->key()) {
	case Qt::Key_Shift:
		shiftPressed = false;
		break;
	case Qt::Key_Control:
		ctrlPressed = false;
		break;
	case Qt::Key_Alt:
		altPressed = false;
		break;
	default:
		break;
	}
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

	QPainter painter(this);

	// draw background image
	if (image_loaded) {
		painter.setOpacity(opacityOfBackground);
		painter.drawImage(0, 0, image);
	}

	// draw lines
	painter.setOpacity(1.0f);
	for (auto line : lines) {
		if (line.type == vp::VanishingLine::TYPE_HORIZONTAL_LEFT) {
			painter.setPen(QPen(horizontalLeftColor, lineWidth));
		}
		else if (line.type == vp::VanishingLine::TYPE_HORIZONTAL_RIGHT) {
			painter.setPen(QPen(horizontalRightColor, lineWidth));
		}
		else {
			painter.setPen(QPen(verticalColor, lineWidth));
		}
		painter.drawLine(line.start.x, line.start.y, line.end.x, line.end.y);
	}

	// draw silhouette
	painter.setPen(QPen(silhouetteColor, silhouetteWidth));
	for (auto line : silhouette) {
		painter.drawLine(line.start.x, line.start.y, line.end.x, line.end.y);
	}

	// draw origin
	painter.setPen(QPen(QColor(255, 255, 0), 1, Qt::SolidLine));
	painter.setBrush(QBrush(QColor(255, 255, 0)));
	painter.drawEllipse(origin.x - 3, origin.y - 3, 7, 7);

	// draw the center of the building
	glm::vec2 pp = vp::projectPoint(camera.mvpMatrix, glm::dvec3(0, 0, 0));
	painter.setPen(QPen(QColor(255, 0, 0), 1, Qt::SolidLine));
	painter.setBrush(QBrush(QColor(255, 0, 0)));
	painter.drawEllipse((pp.x + 1) * 0.5 * width() - 3, (1 - pp.y) * 0.5 * height() - 3, 7, 7);

	painter.end();

	glEnable(GL_DEPTH_TEST);

}

/**
 * This event handler is called when the mouse press events occur.
 */
void GLWidget3D::mousePressEvent(QMouseEvent *e) {
	if (e->buttons() & Qt::RightButton) {
		camera.mousePress(e->x(), e->y());
	}
	else if (e->button() & Qt::LeftButton) {
		if (shiftPressed) {
			if (pen_type == PEN_TYPE_VANISHING_LINE) {
				lines.push_back(vp::VanishingLine(e->x(), e->y(), e->x(), e->y(), vp::VanishingLine::TYPE_HORIZONTAL_RIGHT));
			}
			else if (pen_type == PEN_TYPE_SILHOUETTE) {
				silhouette.push_back(vp::VanishingLine(e->x(), e->y(), e->x(), e->y(), vp::VanishingLine::TYPE_HORIZONTAL_RIGHT));
			}
		}
		else if (altPressed) {
			if (pen_type == PEN_TYPE_VANISHING_LINE) {
				lines.push_back(vp::VanishingLine(e->x(), e->y(), e->x(), e->y(), vp::VanishingLine::TYPE_VERTICAL));
			}
			else if (pen_type == PEN_TYPE_SILHOUETTE) {
				silhouette.push_back(vp::VanishingLine(e->x(), e->y(), e->x(), e->y(), vp::VanishingLine::TYPE_VERTICAL));
			}
		}
		else {
			if (pen_type == PEN_TYPE_VANISHING_LINE) {
				lines.push_back(vp::VanishingLine(e->x(), e->y(), e->x(), e->y(), vp::VanishingLine::TYPE_HORIZONTAL_LEFT));
			}
			else if (pen_type == PEN_TYPE_SILHOUETTE) {
				silhouette.push_back(vp::VanishingLine(e->x(), e->y(), e->x(), e->y(), vp::VanishingLine::TYPE_HORIZONTAL_LEFT));
			}
		}
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
			camera.rotate(e->x(), e->y(), (ctrlPressed ? 0.1 : 1));
		}
	}
	else if (e->buttons() & Qt::MidButton) { // Rotate around Z axis
		camera.rotateAroundZ(e->x(), e->y(), (ctrlPressed ? 0.1 : 1));
	}
	else if (e->buttons() & Qt::LeftButton) {
		if (pen_type == PEN_TYPE_VANISHING_LINE) {
			if (lines.size() > 0) {
				lines.back().end = glm::vec2(e->x(), e->y());
			}
		}
		else if (pen_type == PEN_TYPE_SILHOUETTE) {
			if (silhouette.size() > 0) {
				silhouette.back().end = glm::vec2(e->x(), e->y());
			}
		}
	}
	
	updateStatusBar();
	update();
}

/**
 * This event handler is called when the mouse release events occur.
 */
void GLWidget3D::mouseReleaseEvent(QMouseEvent *e) {
	if (e->button() == Qt::LeftButton) {
		if (pen_type == PEN_TYPE_VANISHING_LINE) {
			if (lines.size() > 0 && lines.back().start == lines.back().end) {
				lines.pop_back();
			}
		}
		else if (pen_type == PEN_TYPE_SILHOUETTE) {
			if (silhouette.size() > 0 && silhouette.back().start == silhouette.back().end) {
				silhouette.pop_back();
			}
		}
	}
}

void GLWidget3D::wheelEvent(QWheelEvent* e) {
	camera.changeFov(e->delta() * 0.05, (ctrlPressed ? 0.1 : 1), width(), height());
	updateStatusBar();
	update();
}

