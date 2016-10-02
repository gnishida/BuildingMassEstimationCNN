#include "ParameterEstimationDialog.h"
#include <QFileDialog>

ParameterEstimationDialog::ParameterEstimationDialog(QWidget *parent) : QDialog(parent) {
	ui.setupUi(this);

	ui.checkBoxAutomaticRecognition->setChecked(false);
	ui.lineEditGrammarSnippet->setText("1");
	ui.lineEditGrammarSnippet->setEnabled(!ui.checkBoxAutomaticRecognition->isChecked());
	ui.lineEditImageSize->setText("227");
	ui.checkBoxGrayscale->setChecked(false);
	ui.checkBoxCentering3D->setChecked(true);
	ui.checkBoxMeanSubtraction->setChecked(false);
	ui.lineEditCameraDistance->setText("25");
	ui.lineEditXrotMin->setText("-24.6008");
	ui.lineEditXrotMax->setText("-24.6008");
	ui.lineEditYrotMin->setText("-39.9673");
	ui.lineEditYrotMax->setText("-39.9673");
	ui.lineEditZrotMin->setText("1.55795");
	ui.lineEditZrotMax->setText("1.55795");
	ui.lineEditFovMin->setText("64.9414");
	ui.lineEditFovMax->setText("64.9414");
	ui.lineEditOXMin->setText("0.0272728");
	ui.lineEditOXMax->setText("0.0272728");
	ui.lineEditOYMin->setText("-0.000758028");
	ui.lineEditOYMax->setText("-0.000758028");
	ui.lineEditXMin->setText("-1.94318");
	ui.lineEditXMax->setText("-1.94318");
	ui.lineEditYMin->setText("0.531049");
	ui.lineEditYMax->setText("0.531049");
	ui.checkBoxTryMultiples->setChecked(false);
	ui.lineEditNumMultipleTries->setText("100");
	ui.lineEditNumMultipleTries->setEnabled(ui.checkBoxTryMultiples->isChecked());
	ui.lineEditMaxNoise->setText("1");
	ui.lineEditMaxNoise->setEnabled(ui.checkBoxTryMultiples->isChecked());
	ui.checkBoxRefinement->setChecked(false);
	ui.lineEditIterations->setText("50");
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
