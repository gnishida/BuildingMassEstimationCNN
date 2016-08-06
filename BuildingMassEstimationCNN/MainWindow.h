#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"
#include "GLWidget3D.h"
#include "ParameterDialog.h"

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	Ui::MainWindowClass ui;
	GLWidget3D* glWidget;

public:
	ParameterDialog* parameterDialog;

public:
	MainWindow(QWidget *parent = 0);

protected:
	void keyPressEvent(QKeyEvent* e);
	void keyReleaseEvent(QKeyEvent* e);

public slots:
	void onNew();
	void onOpenContour();
	void onSaveContour();
	void onOpenImage();
	void onOpenCGA();
	void onUndo();
	void onParameterEstimation();
	void onParameterEstimationWithCameraCalibration();
	void onParameterDialog();
};

#endif // MAINWINDOW_H
