#include "ParameterDialog.h"
#include "MainWindow.h"
#include "GLWidget3D.h"

ParameterDialog::ParameterDialog(QWidget *parent) : QDialog(parent) {
	mainWin = (MainWindow*)parent;
	ui.setupUi(this);

	manual = false;

	connect(ui.doubleSpinBoxXrot, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged()));
	connect(ui.doubleSpinBoxYrot, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged()));
	connect(ui.doubleSpinBoxZrot, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged()));
	connect(ui.doubleSpinBoxFOV, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged()));
	connect(ui.doubleSpinBoxDepth1, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged()));
	connect(ui.doubleSpinBoxDepth2, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged()));
	connect(ui.doubleSpinBoxHeight, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged()));
	connect(ui.doubleSpinBoxWidth1, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged()));
	connect(ui.doubleSpinBoxWidth2, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged()));
}

ParameterDialog::~ParameterDialog()
{

}

void ParameterDialog::onValueChanged() {
	if (manual) return;
	
	std::vector<float> params;
	params.push_back(ui.doubleSpinBoxDepth1->value());
	params.push_back(ui.doubleSpinBoxDepth2->value());
	params.push_back(ui.doubleSpinBoxHeight->value());
	params.push_back(ui.doubleSpinBoxWidth1->value());
	params.push_back(ui.doubleSpinBoxWidth2->value());
	std::vector<boost::shared_ptr<glutils::Face>> faces;
	mainWin->glWidget->setupGeometry(1, true, 1, 25, 0, ui.doubleSpinBoxXrot->value(), ui.doubleSpinBoxYrot->value(), ui.doubleSpinBoxZrot->value(), ui.doubleSpinBoxFOV->value(), params, faces);
	mainWin->glWidget->update();
}