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
#include "VanishingPoint.h"

class MainWindow;

class GLWidget3D : public QGLWidget {
public:
	static enum { PEN_TYPE_VANISHING_LINE = 0, PEN_TYPE_SILHOUETTE };
	static enum { GRAMMAR_TYPE_MASS = 0, GRAMMAR_TYPE_FACADE };

	MainWindow* mainWin;

	// camera
	Camera camera;
	glm::vec3 light_dir;
	glm::mat4 light_mvpMatrix;

	// rendering engine
	RenderManager renderManager;

	// key status
	bool ctrlPressed;
	bool shiftPressed;
	bool altPressed;

	// CNN and grammars
	boost::shared_ptr<Classifier> classifier;
	std::vector<boost::shared_ptr<Regression>> regressions;
	std::map<std::string, std::vector<cga::Grammar>> grammars;
	std::map<std::string, int> grammar_ids;
	std::map<std::string, std::vector<float>> pm_params;
	std::vector<boost::shared_ptr<glutils::Face>> faces;
	
	// GUI options
	float opacityOfBackground;
	bool image_loaded;
	QImage image;
	QImage imageOrig;
	int pen_type;
	int grammar_type;
	std::vector<vp::VanishingLine> lines;
	glm::dvec2 origin;
	int lineWidth;
	QColor horizontalLeftColor;
	QColor horizontalRightColor;
	QColor verticalColor;
	std::vector<vp::VanishingLine> silhouette;
	int silhouetteWidth;
	QColor silhouetteColor;

public:
	GLWidget3D(QWidget *parent);

	void drawScene();
	void render();
	void clearBackground();
	void loadImage(const QString& filename);
	void clearLines();
	void loadLines(const QString& filename);
	void saveLines(const QString& filename);
	void clearSilhouette();
	void loadSilhouette(const QString& filename);
	void saveSilhouette(const QString& filename);
	void clearGeometry();
	void loadCGA(const std::string& filename);
	void undo();
	void massReconstruction(bool automaticRecognition, int grammarSnippetId, int image_size, float cameraDistanceBase, float xrotMin, float xrotMax, float yrotMin, float yrotMax, float zrotMin, float zrotMax, float fovMin, float fovMax, float oxMin, float oxMax, float oyMin, float oyMax, float xMin, float xMax, float yMin, float yMax, int silhouette_line_type, bool imageBlur, int imageBlurSize, bool tryMultiples, int numMultipleTries, float maxNoise, bool refinement, int maxIters, int refinement_method);
	void autoTest(int grammar_id, int image_size, const QString& param_filename, float xrotMin, float xrotMax, float yrotMin, float yrotMax, float zrotMin, float zrotMax, float fovMin, float fovMax, float oxMin, float oxMax, float oyMin, float oyMax, float xMin, float xMax, float yMin, float yMax, int silhouette_line_type, bool imageBlur, int imageBlurSize, bool refinement);
	bool renderImage(cga::Grammar* grammar, std::vector<float>* pm_params, cv::Mat& rendered_image, bool discardIfTopFaceIsVisible = false, bool discardIfBottomFaceIsVisible = false);
	double distanceMap(cv::Mat rendered_image, const cv::Mat& reference_dist_map);
	std::vector<boost::shared_ptr<glutils::Face>> updateGeometry(int grammar_type, cga::Grammar* mass_grammar, std::vector<float>* mass_params, cga::Grammar* facade_grammar = NULL, std::vector<float>* facade_params = NULL, cga::Grammar* window_grammar = NULL, std::vector<float>* window_params = NULL);
	void updateGeometry();
	void setupCamera(const std::vector<float>& params, float xrotMax, float xrotMin, float yrotMax, float yrotMin, float zrotMax, float zrotMin, float fovMax, float fovMin, float oxMax, float oxMin, float oyMax, float oyMin, float xMax, float xMin, float yMax, float yMin);
	void textureMapping();
	void updateStatusBar();
	void resizeImageCanvasSize(QImage& image, int width, int height);

	void keyPressEvent(QKeyEvent* e);
	void keyReleaseEvent(QKeyEvent* e);


protected:
	void initializeGL();
	void resizeGL(int width, int height);
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void wheelEvent(QWheelEvent* e);

};

