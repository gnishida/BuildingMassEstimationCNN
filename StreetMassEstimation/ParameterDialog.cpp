#include "ParameterDialog.h"
#include "MainWindow.h"
#include "GLWidget3D.h"
#include "BuildingMassEstimation.h"

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
	
	mainWin->glWidget->camera.xrot = ui.doubleSpinBoxXrot->value();
	mainWin->glWidget->camera.yrot = ui.doubleSpinBoxYrot->value();
	mainWin->glWidget->camera.zrot = ui.doubleSpinBoxZrot->value();
	mainWin->glWidget->camera.fovy = ui.doubleSpinBoxFOV->value();
	mainWin->glWidget->camera.updatePMatrix(mainWin->glWidget->width(), mainWin->glWidget->height());

	std::vector<float> pm_params;
	pm_params.push_back(ui.doubleSpinBoxDepth1->value());
	pm_params.push_back(ui.doubleSpinBoxDepth2->value());
	pm_params.push_back(ui.doubleSpinBoxHeight->value());
	pm_params.push_back(ui.doubleSpinBoxWidth1->value());
	pm_params.push_back(ui.doubleSpinBoxWidth2->value());
	mainWin->glWidget->estimated_pm_params = pm_params;
	//std::vector<boost::shared_ptr<glutils::Face>> faces;
	bme::setupGeometry(mainWin->glWidget->grammars[mainWin->glWidget->grammar_id], true, pm_params, mainWin->glWidget->estimated_faces);
	mainWin->glWidget->renderManager.removeObjects();
	mainWin->glWidget->renderManager.addFaces(mainWin->glWidget->estimated_faces, true);

	mainWin->glWidget->update();
}