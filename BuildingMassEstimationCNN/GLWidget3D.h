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
#include "Stroke.h"

class MainWindow;

class GLWidget3D : public QGLWidget {
public:
	static enum{ VERTEX, NORMAL, COLOR, TOTAL_VBO_ID };

	MainWindow* mainWin;
	bool dragging;
	bool ctrlPressed;
	bool shiftPressed;
	boost::shared_ptr<Classifier> classifier;
	std::vector<boost::shared_ptr<Regression>> regressions;
	std::vector<cga::Grammar> grammars;

	QImage bgImage;
	QImage bgImageOrig;
	std::vector<Stroke> silhouette;
	Stroke currentStroke;
	float opacityOfBackground;

	Camera camera;
	glm::vec3 light_dir;
	glm::mat4 light_mvpMatrix;
	RenderManager renderManager;
	cga::CGA system;
	
	std::vector<boost::shared_ptr<glutils::Face>> estimated_faces;
	std::vector<float> estimated_pm_params;
	int grammar_id;
	float estimation_error;

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
	void parameterEstimation(bool automaticRecognition, int grammarSnippetId, int image_size, bool grayscale, bool centering3D, bool meanSubtraction, int cameraType, float cameraDistanceBase, float cameraHeight, bool rotateContour, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int zrotMin, int zrotMax, int fovMin, int fovMax, bool tryMultiples, int numMultipleTries, float maxNoise, bool refinement, int maxIters, bool applyTexture);
	//void parameterEstimationWithCameraCalibration(bool automaticRecognition, int grammarSnippetId, bool centering3D, bool meanSubtraction, int cameraType, float cameraDistanceBase, float cameraHeight, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int zrotMin, int zrotMax, int fovMin, int fovMax);
	void render(int grammarSnippetId, bool centering3D, int cameraType, float cameraDistanceBase, float cameraHeight, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int zrotMin, int zrotMax, int fovMin, int fovMax, const std::vector<float>& params, std::vector<boost::shared_ptr<glutils::Face>>& faces, cv::Mat& renderedImage);
	void setupCameraFromParams(int xrotMin, int xrotMax, int yrotMin, int yrotMax, int zrotMin, int zrotMax, int fovMin, int fovMax, int cameraType, float cameraDistanceBase, float cameraHeight, const std::vector<float>& params);
	void keyPressEvent(QKeyEvent* e);
	void keyReleaseEvent(QKeyEvent* e);
	void updateStatusBar();	
	void resizeImageCanvasSize(QImage& image, int width, int height);

protected:
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void wheelEvent(QWheelEvent* e);
	void initializeGL();
	void resizeGL(int width, int height);
	void paintEvent(QPaintEvent *event);

};

