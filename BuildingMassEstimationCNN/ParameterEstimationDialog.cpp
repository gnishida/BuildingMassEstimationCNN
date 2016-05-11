#include "ParameterEstimationDialog.h"
#include <QFileDialog>

ParameterEstimationDialog::ParameterEstimationDialog(QWidget *parent) : QDialog(parent) {
	ui.setupUi(this);

	ui.lineEditGrammarSnippet->setText("1");
	ui.checkBoxCentering3D->setChecked(true);
	ui.checkBoxMeanSubtraction->setChecked(false);
	ui.radioButtonCameraTypeStreetView->setChecked(false);
	ui.radioButtonCameraTypeAerialView->setChecked(true);
	ui.lineEditCameraDistance->setText("25");
	ui.lineEditXrotMin->setText("30");
	ui.lineEditXrotMax->setText("60");
	ui.lineEditYrotMin->setText("30");
	ui.lineEditYrotMax->setText("60");
	ui.lineEditFovMin->setText("20");
	ui.lineEditFovMax->setText("30");
	ui.checkBoxTryMultiples->setChecked(true);
	ui.lineEditNumMultipleTries->setText("100");
	ui.lineEditMaxNoise->setText("3");
	ui.checkBoxRefinement->setChecked(false);
	ui.checkBoxApplyTexture->setChecked(false);

	connect(ui.checkBoxTryMultiples, SIGNAL(clicked()), this, SLOT(onTryMultiples()));
	connect(ui.pushButtonOK, SIGNAL(clicked()), this, SLOT(onOK()));
	connect(ui.pushButtonCancel, SIGNAL(clicked()), this, SLOT(onCancel()));
}

ParameterEstimationDialog::~ParameterEstimationDialog() {
}

void ParameterEstimationDialog::onTryMultiples() {
	ui.lineEditNumMultipleTries->setEnabled(ui.checkBoxTryMultiples->isChecked());
	ui.lineEditMaxNoise->setEnabled(ui.checkBoxTryMultiples->isChecked());
}

void ParameterEstimationDialog::onOK() {
	accept();
}

void ParameterEstimationDialog::onCancel() {
	reject();
}
