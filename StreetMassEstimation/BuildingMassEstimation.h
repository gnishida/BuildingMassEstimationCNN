#pragma once

#include <vector>
#include "Stroke.h"
#include "GLUtils.h"
#include <QImage>
#include "Utils.h"
#include "Classifier.h"
#include "Regression.h"
#include <boost/shared_ptr.hpp>
#include "CGA.h"
#include "Grammar.h"
#include "Camera.h"

namespace bme {

	std::vector<float> estimate(int screen_width, int screen_height, std::vector<Stroke> silhouette, boost::shared_ptr<Classifier> classifier, std::vector<boost::shared_ptr<Regression>> regressions, std::vector<cga::Grammar>& grammars, bool automaticRecognition, int& grammarSnippetId, int image_size, bool grayscale, bool centering3D, bool meanSubtraction, float cameraDistanceBase, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int zrotMin, int zrotMax, int fovMin, int fovMax, int xMin, int xMax, int yMin, int yMax, bool tryMultiples, int numMultipleTries, float maxNoise, bool refinement, int maxIters, bool applyTexture);
	int classify(int screen_width, int screen_height, std::vector<Stroke>& silhouette, boost::shared_ptr<Classifier> classifier);
	std::vector<float> regress(int screen_width, int screen_height, std::vector<Stroke>& silhouette, boost::shared_ptr<Regression> regression, cga::Grammar& grammar, int image_size, bool grayscale, bool centering3D, bool meanSubtraction, const std::string& mean_file, float cameraDistanceBase, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int zrotMin, int zrotMax, int fovMin, int fovMax, int xMin, int xMax, int yMin, int yMax, bool tryMultiples, int numMultipleTries, float maxNoise, bool refinement, int maxIters, bool applyTexture);

	void coordinateDescent(std::vector<float>& params, double& dist, std::vector<float>& params_var, int screen_width, int screen_height, std::vector<Stroke>& silhouette, boost::shared_ptr<Regression> regression, cga::Grammar& grammar, bool centering3D, bool meanSubtraction, float cameraDistanceBase, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int zrotMin, int zrotMax, int fovMin, int fovMax, int xMin, int xMax, int yMin, int yMax);

	void generateTextures(const Camera& camera, const QImage& bgImage, int screen_width, int screen_height, std::vector<boost::shared_ptr<glutils::Face>>& faces);

	double distance(const std::vector<Stroke>& silhouette, const cv::Mat& renderedImg, int screen_width, int screen_height);
	double distance(int screen_width, int screen_height, const std::vector<Stroke>& silhouette, cga::Grammar& grammar, bool centering3D, float cameraDistanceBase, int xrotMin, int xrotMax, int yrotMin, int yrotMax, int zrotMin, int zrotMax, int fovMin, int fovMax, int xMin, int xMax, int yMin, int yMax, const std::vector<float>& params);
	void setupGeometry(cga::Grammar& grammar, bool centering3D, const std::vector<float>& pm_params, std::vector<boost::shared_ptr<glutils::Face>>& faces);

	void shiftImage(int shift_x, int shift_y, QImage& image);
	void shiftSilhouette(int shift_x, int shift_y, std::vector<Stroke>& silhouette);
	void rotateImage(float theta, const glm::vec2& center, QImage& image);
	void rotateSilhouette(float theta, const glm::vec2& center, std::vector<Stroke>& silhouette);
	glm::vec2 getOffset(const cv::Mat& img);
	glm::vec2 getOffset(const std::vector<Stroke>& silhouette, int screen_width, int screen_height);
	glm::vec2 getCenter(const std::vector<Stroke>& silhouette);
	float getRotation(const std::vector<Stroke>& silhouette);
	float silhouetteLength(const std::vector<Stroke>& silhouette);
}