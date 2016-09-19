#ifndef PARAMETERDIALOG_H
#define PARAMETERDIALOG_H

#include <QDialog>
#include "ui_ParameterDialog.h"

class MainWindow;

class ParameterDialog : public QDialog
{
	Q_OBJECT

public:
	ParameterDialog(QWidget *parent = 0);
	~ParameterDialog();

public slots:
	void onValueChanged();

public:
	Ui::ParameterDialog ui;
	MainWindow* mainWin;
	bool manual;
};

#endif // PARAMETERDIALOG_H
