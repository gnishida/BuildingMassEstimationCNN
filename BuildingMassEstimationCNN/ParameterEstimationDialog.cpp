#include "ParameterEstimationDialog.h"

ParameterEstimationDialog::ParameterEstimationDialog(QWidget *parent) : QDialog(parent) {
	ui.setupUi(this);

	ui.checkBoxCentering3D->setChecked(true);
	ui.checkBoxMeanSubtraction->setChecked(false);
	ui.radioButtonCameraTypeStreetView->setChecked(false);
	ui.radioButtonCameraTypeAerialView->setChecked(true);
	ui.lineEditCameraDistance->setText("25");
	ui.lineEditXrotMin->setText("0");
	ui.lineEditXrotMax->setText("40");
	ui.lineEditYrotMin->setText("15");
	ui.lineEditYrotMax->setText("75");
	ui.lineEditFovMin->setText("10");
	ui.lineEditFovMax->setText("60");

	connect(ui.pushButtonOK, SIGNAL(clicked()), this, SLOT(onOK()));
	connect(ui.pushButtonCancel, SIGNAL(clicked()), this, SLOT(onCancel()));
}

ParameterEstimationDialog::~ParameterEstimationDialog() {
}

void ParameterEstimationDialog::onOK() {
	accept();
}

void ParameterEstimationDialog::onCancel() {
	reject();
}