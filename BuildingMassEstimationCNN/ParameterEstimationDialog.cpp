#include "ParameterEstimationDialog.h"
#include <QFileDialog>

ParameterEstimationDialog::ParameterEstimationDialog(QWidget *parent) : QDialog(parent) {
	ui.setupUi(this);

	QButtonGroup* group1 = new QButtonGroup();
	group1->addButton(ui.radioButtonCameraTypeStreetView);
	group1->addButton(ui.radioButtonCameraTypeAerialView);

/*	QButtonGroup* group2 = new QButtonGroup();
	group2->addButton(ui.radioButtonRefineFromBest);
	group2->addButton(ui.radioButtonRefineFromAll);*/

	ui.checkBoxAutomaticRecognition->setChecked(false);
	ui.lineEditGrammarSnippet->setText("1");
	ui.lineEditGrammarSnippet->setEnabled(!ui.checkBoxAutomaticRecognition->isChecked());
	ui.lineEditImageSize->setText("227");
	ui.checkBoxGrayscale->setChecked(false);
	ui.checkBoxCentering3D->setChecked(true);
	ui.checkBoxMeanSubtraction->setChecked(false);
	ui.radioButtonCameraTypeStreetView->setChecked(true);
	ui.radioButtonCameraTypeAerialView->setChecked(false);
	ui.lineEditCameraDistance->setText("25");
	ui.checkBoxRotateContour->setChecked(true);
	ui.lineEditXrotMin->setText("-14");
	ui.lineEditXrotMax->setText("0");
	ui.lineEditYrotMin->setText("20");
	ui.lineEditYrotMax->setText("70");
	ui.lineEditZrotMin->setText("0");
	ui.lineEditZrotMax->setText("0");
	ui.lineEditFovMin->setText("30");
	ui.lineEditFovMax->setText("80");
	ui.checkBoxTryMultiples->setChecked(false);
	ui.lineEditNumMultipleTries->setText("100");
	ui.lineEditNumMultipleTries->setEnabled(ui.checkBoxTryMultiples->isChecked());
	ui.lineEditMaxNoise->setText("1");
	ui.lineEditMaxNoise->setEnabled(ui.checkBoxTryMultiples->isChecked());
	ui.checkBoxRefinement->setChecked(false);
	//ui.radioButtonRefineFromBest->setChecked(true);
	//ui.radioButtonRefineFromBest->setEnabled(ui.checkBoxRefinement->isChecked());
	//ui.radioButtonRefineFromAll->setChecked(false);
	//ui.radioButtonRefineFromAll->setEnabled(ui.checkBoxRefinement->isChecked());
	ui.lineEditIterations->setText("1000000");
	ui.lineEditIterations->setEnabled(ui.checkBoxRefinement->isChecked());
	ui.checkBoxApplyTexture->setChecked(false);

	connect(ui.checkBoxAutomaticRecognition, SIGNAL(clicked()), this, SLOT(onAutomaticRecognition()));
	connect(ui.checkBoxTryMultiples, SIGNAL(clicked()), this, SLOT(onTryMultiples()));
	connect(ui.checkBoxRefinement, SIGNAL(clicked()), this, SLOT(onRefinement()));
	connect(ui.pushButtonOK, SIGNAL(clicked()), this, SLOT(onOK()));
	connect(ui.pushButtonCancel, SIGNAL(clicked()), this, SLOT(onCancel()));
}

ParameterEstimationDialog::~ParameterEstimationDialog() {
}

void ParameterEstimationDialog::onAutomaticRecognition() {
	ui.lineEditGrammarSnippet->setEnabled(ui.checkBoxAutomaticRecognition->isChecked());
}

void ParameterEstimationDialog::onTryMultiples() {
	ui.lineEditNumMultipleTries->setEnabled(ui.checkBoxTryMultiples->isChecked());
	ui.lineEditMaxNoise->setEnabled(ui.checkBoxTryMultiples->isChecked());
}

void ParameterEstimationDialog::onRefinement() {
	ui.lineEditIterations->setEnabled(ui.checkBoxRefinement->isChecked());
}

void ParameterEstimationDialog::onOK() {
	accept();
}

void ParameterEstimationDialog::onCancel() {
	reject();
}
