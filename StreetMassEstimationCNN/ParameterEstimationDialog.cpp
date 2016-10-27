#include "ParameterEstimationDialog.h"
#include <QFileDialog>

ParameterEstimationDialog::ParameterEstimationDialog(QWidget *parent) : QDialog(parent) {
	ui.setupUi(this);

	groupSilhouetteLine = new QButtonGroup(this);
	groupSilhouetteLine->addButton(ui.radioButtonSilhouetteLine8);
	groupSilhouetteLine->addButton(ui.radioButtonSilhouetteLineAA);

	ui.checkBoxAutomaticRecognition->setChecked(false);
	ui.lineEditGrammarSnippet->setText("1");
	ui.lineEditGrammarSnippet->setEnabled(!ui.checkBoxAutomaticRecognition->isChecked());
	ui.lineEditImageSize->setText("227");
	ui.lineEditCameraDistance->setText("25");
	// all
	ui.lineEditXrotMin->setText("-40");
	ui.lineEditXrotMax->setText("0");
	ui.lineEditYrotMin->setText("-70");
	ui.lineEditYrotMax->setText("-20");
	ui.lineEditZrotMin->setText("-10");
	ui.lineEditZrotMax->setText("10");
	ui.lineEditFovMin->setText("20");
	ui.lineEditFovMax->setText("90");
	ui.lineEditOXMin->setText("-0.8");
	ui.lineEditOXMax->setText("0.8");
	ui.lineEditOYMin->setText("-0.8");
	ui.lineEditOYMax->setText("0.8");
	ui.lineEditXMin->setText("-15");
	ui.lineEditXMax->setText("15");
	ui.lineEditYMin->setText("-15");
	ui.lineEditYMax->setText("15");
	// #3
	/*
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
	*/
	// #11 
	/*
	ui.lineEditXrotMin->setText("-11.7039");
	ui.lineEditXrotMax->setText("-11.7039");
	ui.lineEditYrotMin->setText("-44.5257");
	ui.lineEditYrotMax->setText("-44.5257");
	ui.lineEditZrotMin->setText("-1.6928");
	ui.lineEditZrotMax->setText("-1.6928");
	ui.lineEditFovMin->setText("81.7128");
	ui.lineEditFovMax->setText("81.7128");
	ui.lineEditOXMin->setText("-0.0603159");
	ui.lineEditOXMax->setText("-0.0603159");
	ui.lineEditOYMin->setText("-0.313111");
	ui.lineEditOYMax->setText("-0.313111");
	ui.lineEditXMin->setText("4.51085");
	ui.lineEditXMax->setText("4.51085");
	ui.lineEditYMin->setText("-1.22776");
	ui.lineEditYMax->setText("-1.22776");
	*/
	ui.radioButtonSilhouetteLine8->setChecked(false);
	ui.radioButtonSilhouetteLineAA->setChecked(true);
	ui.checkBoxImageBlur->setChecked(false);
	ui.lineEditImageBlurSize->setEnabled(false);
	ui.lineEditImageBlurSize->setText("5");
	ui.checkBoxTryMultiples->setChecked(false);
	ui.lineEditNumMultipleTries->setText("100");
	ui.lineEditNumMultipleTries->setEnabled(ui.checkBoxTryMultiples->isChecked());
	ui.lineEditMaxNoise->setText("0.5");
	ui.lineEditMaxNoise->setEnabled(ui.checkBoxTryMultiples->isChecked());
	ui.checkBoxRefinement->setChecked(false);
	ui.lineEditIterations->setText("30");
	ui.lineEditIterations->setEnabled(ui.checkBoxRefinement->isChecked());
	ui.checkBoxApplyTexture->setChecked(false);

	connect(ui.checkBoxAutomaticRecognition, SIGNAL(clicked()), this, SLOT(onAutomaticRecognition()));
	connect(ui.checkBoxImageBlur, SIGNAL(clicked()), this, SLOT(onImageBlur()));
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

void ParameterEstimationDialog::onImageBlur() {
	ui.lineEditImageBlurSize->setEnabled(ui.checkBoxImageBlur->isChecked());
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
