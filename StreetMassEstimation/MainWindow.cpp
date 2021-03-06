#include "MainWindow.h"
#include <QFileDialog>
#include "ParameterEstimationDialog.h"
#include "OptionDialog.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
	ui.setupUi(this);

	QActionGroup* group = new QActionGroup(this);
	group->addAction(ui.actionPenVanishingLine);
	group->addAction(ui.actionPenSilhouette);

	ui.actionPenVanishingLine->setChecked(true);

	connect(ui.actionClearBackground, SIGNAL(triggered()), this, SLOT(onClearBackground()));
	connect(ui.actionOpenImage, SIGNAL(triggered()), this, SLOT(onOpenImage()));
	connect(ui.actionClearLines, SIGNAL(triggered()), this, SLOT(onClearLines()));
	connect(ui.actionLoadLines, SIGNAL(triggered()), this, SLOT(onLoadLines()));
	connect(ui.actionSaveLines, SIGNAL(triggered()), this, SLOT(onSaveLines()));
	connect(ui.actionClearSilhouette, SIGNAL(triggered()), this, SLOT(onClearSilhouette()));
	connect(ui.actionLoadSilhouette, SIGNAL(triggered()), this, SLOT(onLoadSilhouette()));
	connect(ui.actionSaveSilhouette, SIGNAL(triggered()), this, SLOT(onSaveSilhouette()));
	connect(ui.actionOpenCGA, SIGNAL(triggered()), this, SLOT(onOpenCGA()));
	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui.actionUndo, SIGNAL(triggered()), this, SLOT(onUndo()));
	connect(ui.actionParameterEstimation, SIGNAL(triggered()), this, SLOT(onParameterEstimation()));
	connect(ui.actionPenVanishingLine, SIGNAL(triggered()), this, SLOT(onPenChanged()));
	connect(ui.actionPenSilhouette, SIGNAL(triggered()), this, SLOT(onPenChanged()));
	connect(ui.actionOption, SIGNAL(triggered()), this, SLOT(onOption()));

	glWidget = new GLWidget3D(this);
	this->setCentralWidget(glWidget);
}

void MainWindow::keyPressEvent(QKeyEvent* e) {
	glWidget->keyPressEvent(e);
}

void MainWindow::keyReleaseEvent(QKeyEvent* e) {
	glWidget->keyReleaseEvent(e);
}

void MainWindow::onClearBackground() {
	glWidget->clearBackground();
}

void MainWindow::onOpenImage() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Image file..."), "", tr("Image Files (*.jpg *.png *.bmp)"));
	if (filename.isEmpty()) return;

	glWidget->clearLines();
	glWidget->clearGeometry();
	glWidget->loadImage(filename.toUtf8().constData());
}

void MainWindow::onClearLines() {
	glWidget->clearLines();
}

void MainWindow::onLoadLines() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open lines..."), "", tr("Line Files (*.txt)"));
	if (filename.isEmpty()) return;

	glWidget->loadLines(filename);
}

void MainWindow::onSaveLines() {
	QString filename = QFileDialog::getSaveFileName(this, tr("Save lines..."), "", tr("Line Files (*.txt)"));
	if (filename.isEmpty()) return;

	glWidget->saveLines(filename);
}

void MainWindow::onClearSilhouette() {
	glWidget->clearSilhouette();
}

void MainWindow::onLoadSilhouette() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open contour file..."), "", tr("Contour Files (*.ctr)"));
	if (filename.isEmpty()) return;

	glWidget->loadSilhouette(filename.toUtf8().constData());
}

void MainWindow::onSaveSilhouette() {
	QString filename = QFileDialog::getSaveFileName(this, tr("Save contour file..."), "", tr("Contour Files (*.ctr)"));
	if (filename.isEmpty()) return;

	glWidget->saveSilhouette(filename.toUtf8().constData());
}

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
		float cameraDistanceBase = dlg.ui.lineEditCameraDistance->text().toFloat();
		float xrotMin = dlg.ui.lineEditXrotMin->text().toFloat();
		float xrotMax = dlg.ui.lineEditXrotMax->text().toFloat();
		float yrotMin = dlg.ui.lineEditYrotMin->text().toFloat();
		float yrotMax = dlg.ui.lineEditYrotMax->text().toFloat();
		float zrotMin = dlg.ui.lineEditZrotMin->text().toFloat();
		float zrotMax = dlg.ui.lineEditZrotMax->text().toFloat();
		float fovMin = dlg.ui.lineEditFovMin->text().toFloat();
		float fovMax = dlg.ui.lineEditFovMax->text().toFloat();
		float xMin = dlg.ui.lineEditXMin->text().toFloat();
		float xMax = dlg.ui.lineEditXMax->text().toFloat();
		float yMin = dlg.ui.lineEditYMin->text().toFloat();
		float yMax = dlg.ui.lineEditYMax->text().toFloat();
		bool tryMultiples = dlg.ui.checkBoxTryMultiples->isChecked();
		int numMultipleTries = dlg.ui.lineEditNumMultipleTries->text().toInt();
		float maxNoise = dlg.ui.lineEditMaxNoise->text().toFloat();
		bool refinement = dlg.ui.checkBoxRefinement->isChecked();
		int maxIters = dlg.ui.lineEditIterations->text().toInt();
		bool applyTexture = dlg.ui.checkBoxApplyTexture->isChecked();

		glWidget->parameterEstimation(automaticRecognition, grammarSnippetId, image_size, grayscale, centering3D, meanSubtraction, cameraDistanceBase, xrotMin, xrotMax, yrotMin, yrotMax, zrotMin, zrotMax, fovMin, fovMax, xMin, xMax, yMin, yMax, tryMultiples, numMultipleTries, maxNoise, refinement, maxIters, applyTexture);
	}
}

void MainWindow::onPenChanged() {
	if (ui.actionPenVanishingLine->isChecked()) {
		glWidget->pen_type = GLWidget3D::PEN_TYPE_VANISHING_LINE;
	}
	else if (ui.actionPenSilhouette->isChecked())  {
		glWidget->pen_type = GLWidget3D::PEN_TYPE_SILHOUETTE;
	}
}

void MainWindow::onOption() {
	OptionDialog dlg;
	dlg.setContourLineWidth(glWidget->lineWidth);
	dlg.setHorizontalLeftColor(glWidget->horizontalLeftColor);
	dlg.setHorizontalRightColor(glWidget->horizontalRightColor);
	dlg.setVerticalColor(glWidget->verticalColor);
	dlg.setSilhouetteWidth(glWidget->silhouetteWidth);
	dlg.setSilhouetteColor(glWidget->silhouetteColor);

	if (dlg.exec()) {
		glWidget->lineWidth = dlg.getContourLineWidth();
		glWidget->horizontalLeftColor = dlg.getHorizontalLeftColor();
		glWidget->horizontalRightColor = dlg.getHorizontalRightColor();
		glWidget->verticalColor = dlg.getVerticalColor();
		glWidget->silhouetteWidth = dlg.getSilhouetteWidth();
		glWidget->silhouetteColor = dlg.getSilhouetteColor();
	}
}