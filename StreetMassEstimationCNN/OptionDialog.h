#ifndef OPTIONDIALOG_H
#define OPTIONDIALOG_H

#include <QDialog>
#include "ui_OptionDialog.h"

class OptionDialog : public QDialog
{
	Q_OBJECT

public:
	OptionDialog(QWidget *parent = 0);
	~OptionDialog();

	void setContourLineWidth(int contourLineWidth);
	int getContourLineWidth();
	void setHorizontalLeftColor(const QColor& horizontalLeftColor);
	QColor getHorizontalLeftColor();
	void setHorizontalRightColor(const QColor& horizontalRightColor);
	QColor getHorizontalRightColor();
	void setVerticalColor(const QColor& verticalColor);
	QColor getVerticalColor();
	void setSilhouetteWidth(int silhouetteWidth);
	int getSilhouetteWidth();
	void setSilhouetteColor(const QColor& silhouetteColor);
	QColor getSilhouetteColor();

	public slots:
	void onOK();
	void onCancel();

private:
	Ui::OptionDialog ui;
};

#endif // OPTIONDIALOG_H
