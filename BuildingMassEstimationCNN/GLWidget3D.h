#pragma once

#include <glew.h>
#include "Shader.h"
#include "Vertex.h"
#include <QGLWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include <vector>
#include <QImage>
#include "Camera.h"
#include "ShadowMapping.h"
#include "RenderManager.h"
#include "CGA.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "Classifier.h"
#include "Regression.h"

class MainWindow;

class Stroke {
public:
	glm::vec2 start;
	glm::vec2 end;

public:
	Stroke() {}
	Stroke(const glm::vec2& start, const glm::vec2& end) : start(start), end(end) {}
};

class GLWidget3D : public QGLWidget {
private:
	static enum{ VERTEX, NORMAL, COLOR, TOTAL_VBO_ID };

	MainWindow* mainWin;
	bool dragging;
	bool ctrlPressed;
	bool shiftPressed;
	boost::shared_ptr<Classifier> classifier;
	std::vector<boost::shared_ptr<Regression>> regressions;
	std::vector<cga::Grammar> grammars;

	QImage bgImage;
	//std::vector<Stroke> sketch;
	std::vector<Stroke> silhouette;
	Stroke currentStroke;
	float opacityOfBackground;

	Camera camera;
	glm::vec3 light_dir;
	glm::mat4 light_mvpMatrix;
	RenderManager renderManager;
	cga::CGA system;

public:
	GLWidget3D(QWidget *parent);
	void clearSilhouette();
	void clearBackground();
	void clearGeometry();
	void drawScene();
	void render();
	void loadContour(const std::string& filename);
	void saveContour(const std::string& filename);
	void loadImage(const std::string& filename);
	void loadCGA(const std::string& filename);
	void undo();
	void parameterEstimation(bool automaticRecognition, int grammarSnippetId, bool centering3D, bool meanSubtraction, int cameraType, float cameraDistanceBase, float cameraHeight, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int zrotMin, int zrotMax, int fovMin, int fovMax, bool tryMultiples, int numMultipleTries, float maxNoise, bool refinement, bool refineFromBest, bool applyTexture);
	void parameterEstimationWithCameraCalibration(int grammarSnippetId, bool centering3D, bool meanSubtraction, int cameraType, float cameraDistanceBase, float cameraHeight, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int fovMin, int fovMax);
	void parameterEstimationWithCameraCalibration2(int grammarSnippetId, bool centering3D, bool meanSubtraction, int cameraType, float cameraDistanceBase, float cameraHeight, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int fovMin, int fovMax);
	double computeDistance(int grammarSnippetId, bool centering3D, int cameraType, float cameraDistanceBase, float cameraHeight, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int zrotMin, int zrotMax, int fovMin, int fovMax, const std::vector<float>& params, std::vector<boost::shared_ptr<glutils::Face>>& faces, bool saveFile, int fileId = 0);
	double computeDistance2(int grammarSnippetId, bool centering3D, int cameraType, float cameraDistanceBase, float cameraHeight, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int zrotMin, int zrotMax, int fovMin, int fovMax, const std::vector<float>& params, std::vector<boost::shared_ptr<glutils::Face>>& faces, bool saveFile, int fileId = 0);
	void render(int grammarSnippetId, bool centering3D, int cameraType, float cameraDistanceBase, float cameraHeight, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int zrotMin, int zrotMax, int fovMin, int fovMax, const std::vector<float>& params, std::vector<boost::shared_ptr<glutils::Face>>& faces, cv::Mat& renderedImage);

	void keyPressEvent(QKeyEvent* e);
	void keyReleaseEvent(QKeyEvent* e);
	void updateStatusBar();
	void shiftImageAndSilhouette(int shift_x, int shift_y, QImage& image, std::vector<Stroke>& silhouette);
	glm::vec2 getOffsetImage(cv::Mat& img);

protected:
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void wheelEvent(QWheelEvent* e);
	void initializeGL();
	void resizeGL(int width, int height);
	void paintEvent(QPaintEvent *event);

};

