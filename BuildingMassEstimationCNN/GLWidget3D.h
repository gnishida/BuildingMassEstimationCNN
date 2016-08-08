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
	static enum { TYPE_LINE = 0, TYPE_VERTICAL };

public:
	glm::vec2 start;
	glm::vec2 end;
	int type;

public:
	Stroke() {}
	Stroke(const glm::vec2& start, const glm::vec2& end) : start(start), end(end) {}
};

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
	void parameterEstimation(bool automaticRecognition, int grammarSnippetId, bool centering3D, bool meanSubtraction, int cameraType, float cameraDistanceBase, float cameraHeight, bool rotateContour, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int zrotMin, int zrotMax, int fovMin, int fovMax, bool tryMultiples, int numMultipleTries, float maxNoise, bool refinement, int maxIters, bool applyTexture);
	void parameterEstimationWithCameraCalibration(bool automaticRecognition, int grammarSnippetId, bool centering3D, bool meanSubtraction, int cameraType, float cameraDistanceBase, float cameraHeight, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int zrotMin, int zrotMax, int fovMin, int fovMax);
	void parameterEstimationWithCameraCalibration2(int grammarSnippetId, bool centering3D, bool meanSubtraction, int cameraType, float cameraDistanceBase, float cameraHeight, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int fovMin, int fovMax);
	double distance(int grammarSnippetId, bool centering3D, int cameraType, float cameraDistanceBase, float cameraHeight, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int zrotMin, int zrotMax, int fovMin, int fovMax, const std::vector<float>& params, std::vector<boost::shared_ptr<glutils::Face>>& faces);
	void setupGeometry(int grammarSnippetId, bool centering3D, int cameraType, float cameraDistanceBase, float cameraHeight, float xrot, float yrot, float zrot, float fov, const std::vector<float>& params, std::vector<boost::shared_ptr<glutils::Face>>& faces);
	void render(int grammarSnippetId, bool centering3D, int cameraType, float cameraDistanceBase, float cameraHeight, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int zrotMin, int zrotMax, int fovMin, int fovMax, const std::vector<float>& params, std::vector<boost::shared_ptr<glutils::Face>>& faces, cv::Mat& renderedImage);

	void keyPressEvent(QKeyEvent* e);
	void keyReleaseEvent(QKeyEvent* e);
	void updateStatusBar();
	void shiftImage(int shift_x, int shift_y, QImage& image);
	void shiftImageAndSilhouette(int shift_x, int shift_y, QImage& image, std::vector<Stroke>& silhouette);
	void rotateImage(float theta, const glm::vec2& center, QImage& image);
	void rotateSilhouette(float theta, const glm::vec2& center, std::vector<Stroke>& silhouette);
	glm::vec2 getOffset(const cv::Mat& img);
	glm::vec2 getOffset(const std::vector<Stroke>& silhouette);
	glm::vec2 getCenter(const std::vector<Stroke>& silhouette);
	float getRotation(const std::vector<Stroke>& silhouette);
	double distance(const std::vector<Stroke>& silhouette, const cv::Mat& renderedImg);
	float silhouetteLength(const std::vector<Stroke>& silhouette);
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

