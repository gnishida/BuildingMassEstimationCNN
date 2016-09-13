#include "MainWindow.h"
#include <QFileDialog>
#include "ParameterEstimationDialog.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
	ui.setupUi(this);

	connect(ui.actionNew, SIGNAL(triggered()), this, SLOT(onNew()));
	connect(ui.actionOpenContour, SIGNAL(triggered()), this, SLOT(onOpenContour()));
	connect(ui.actionSaveContour, SIGNAL(triggered()), this, SLOT(onSaveContour()));
	connect(ui.actionOpenImage, SIGNAL(triggered()), this, SLOT(onOpenImage()));
	connect(ui.actionOpenCGA, SIGNAL(triggered()), this, SLOT(onOpenCGA()));
	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui.actionUndo, SIGNAL(triggered()), this, SLOT(onUndo()));
	connect(ui.actionParameterEstimation, SIGNAL(triggered()), this, SLOT(onParameterEstimation()));
	connect(ui.actionParameterEstimationWithCameraCalibration, SIGNAL(triggered()), this, SLOT(onParameterEstimationWithCameraCalibration()));
	connect(ui.actionParameterDialog, SIGNAL(triggered()), this, SLOT(onParameterDialog()));

	glWidget = new GLWidget3D(this);
	this->setCentralWidget(glWidget);

	parameterDialog = new ParameterDialog(this);
}

void MainWindow::keyPressEvent(QKeyEvent* e) {
	glWidget->keyPressEvent(e);
}

void MainWindow::keyReleaseEvent(QKeyEvent* e) {
	glWidget->keyReleaseEvent(e);
}

/**
 * This is called when the user clicks [File] -> [New].
 */
void MainWindow::onNew() {
	glWidget->clearSilhouette();
	glWidget->clearBackground();
}

void MainWindow::onOpenContour() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open contour file..."), "", tr("Contour Files (*.ctr)"));
	if (filename.isEmpty()) return;

	glWidget->loadContour(filename.toUtf8().constData());
}

void MainWindow::onSaveContour() {
	QString filename = QFileDialog::getSaveFileName(this, tr("Save contour file..."), "", tr("Contour Files (*.ctr)"));
	if (filename.isEmpty()) return;

	glWidget->saveContour(filename.toUtf8().constData());
}

/**
* This is called when the user clickes [File] -> [Open]
*/
void MainWindow::onOpenImage() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Image file..."), "", tr("Image Files (*.jpg *.png *.bmp)"));
	if (filename.isEmpty()) return;

	glWidget->clearSilhouette();
	glWidget->clearGeometry();
	glWidget->loadImage(filename.toUtf8().constData());
}

/**
* This is called when the user clickes [File] -> [Open CGA]
*/
void MainWindow::onOpenCGA() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open CGA file..."), "", tr("CGA Files (*.xml)"));
	if (filename.isEmpty()) return;

	glWidget->loadCGA(filename.toUtf8().data());
}

void MainWindow::onUndo() {
	glWidget->undo();
}

/**
* This is called when the user clickes [Tool] -> [Predict]
*/
void MainWindow::onParameterEstimation() {
	ParameterEstimationDialog dlg;
	if (dlg.exec()) {
		bool automaticRecognition = dlg.ui.checkBoxAutomaticRecognition->isChecked();
		int grammarSnippetId = dlg.ui.lineEditGrammarSnippet->text().toInt() - 1;
		int image_size = dlg.ui.lineEditImageSize->text().toInt();
		bool grayscale = dlg.ui.checkBoxGrayscale->isChecked();
		bool centering3D = dlg.ui.checkBoxCentering3D->isChecked();
		bool meanSubtraction = dlg.ui.checkBoxMeanSubtraction->isChecked();
		int cameraType = 0;
		if (dlg.ui.radioButtonCameraTypeAerialView->isChecked()) {
			cameraType = 1;
		}
		float cameraDistanceBase = dlg.ui.lineEditCameraDistance->text().toFloat();
		bool rotateContour = dlg.ui.checkBoxRotateContour->isChecked();
		int xrotMin = dlg.ui.lineEditXrotMin->text().toInt();
		int xrotMax = dlg.ui.lineEditXrotMax->text().toInt();
		int yrotMin = dlg.ui.lineEditYrotMin->text().toInt();
		int yrotMax = dlg.ui.lineEditYrotMax->text().toInt();
		int zrotMin = dlg.ui.lineEditZrotMin->text().toInt();
		int zrotMax = dlg.ui.lineEditZrotMax->text().toInt();
		int fovMin = dlg.ui.lineEditFovMin->text().toInt();
		int fovMax = dlg.ui.lineEditFovMax->text().toInt();
		bool tryMultiples = dlg.ui.checkBoxTryMultiples->isChecked();
		int numMultipleTries = dlg.ui.lineEditNumMultipleTries->text().toInt();
		float maxNoise = dlg.ui.lineEditMaxNoise->text().toFloat();
		bool refinement = dlg.ui.checkBoxRefinement->isChecked();
		//bool refineFromBest = dlg.ui.radioButtonRefineFromBest->isChecked();
		int maxIters = dlg.ui.lineEditIterations->text().toInt();
		bool applyTexture = dlg.ui.checkBoxApplyTexture->isChecked();

		glWidget->parameterEstimation(automaticRecognition, grammarSnippetId, image_size, grayscale, centering3D, meanSubtraction, cameraType, cameraDistanceBase, 0.0f, rotateContour, xrotMin, xrotMax, yrotMin, yrotMax, zrotMin, zrotMax, fovMin, fovMax, tryMultiples, numMultipleTries, maxNoise, refinement, maxIters, applyTexture);
	}
}

void MainWindow::onParameterEstimationWithCameraCalibration() {
	ParameterEstimationDialog dlg;
	if (dlg.exec()) {
		bool automaticRecognition = dlg.ui.checkBoxAutomaticRecognition->isChecked();
		int grammarSnippetId = dlg.ui.lineEditGrammarSnippet->text().toInt() - 1;
		bool centering3D = dlg.ui.checkBoxCentering3D->isChecked();
		bool meanSubtraction = dlg.ui.checkBoxMeanSubtraction->isChecked();
		int cameraType = 0;
		if (dlg.ui.radioButtonCameraTypeAerialView->isChecked()) {
			cameraType = 1;
		}
		float cameraDistanceBase = dlg.ui.lineEditCameraDistance->text().toFloat();
		int xrotMin = dlg.ui.lineEditXrotMin->text().toInt();
		int xrotMax = dlg.ui.lineEditXrotMax->text().toInt();
		int yrotMin = dlg.ui.lineEditYrotMin->text().toInt();
		int yrotMax = dlg.ui.lineEditYrotMax->text().toInt();
		int zrotMin = dlg.ui.lineEditZrotMin->text().toInt();
		int zrotMax = dlg.ui.lineEditZrotMax->text().toInt();
		int fovMin = dlg.ui.lineEditFovMin->text().toInt();
		int fovMax = dlg.ui.lineEditFovMax->text().toInt();

		//glWidget->parameterEstimationWithCameraCalibration(automaticRecognition, grammarSnippetId, centering3D, meanSubtraction, cameraType, cameraDistanceBase, 0.0f, xrotMin, xrotMax, yrotMin, yrotMax, zrotMin, zrotMax, fovMin, fovMax);
	}
}

void MainWindow::onParameterDialog() {
	parameterDialog->show();
}