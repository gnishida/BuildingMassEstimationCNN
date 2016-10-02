#include "OptionDialog.h"

OptionDialog::OptionDialog(QWidget *parent) : QDialog(parent) {
	ui.setupUi(this);

	connect(ui.pushButtonOK, SIGNAL(clicked()), this, SLOT(onOK()));
	connect(ui.pushButtonCancel, SIGNAL(clicked()), this, SLOT(onCancel()));
}

OptionDialog::~OptionDialog() {
}

void OptionDialog::setContourLineWidth(int contourLineWidth) {
	ui.lineEditContourLineWidth->setText(QString::number(contourLineWidth));
}
int OptionDialog::getContourLineWidth() {
	return ui.lineEditContourLineWidth->text().toInt();
}

void OptionDialog::setHorizontalLeftColor(const QColor& horizontalLeftColor) {
	ui.lineEditHorizontalLeftRed->setText(QString::number(horizontalLeftColor.red()));
	ui.lineEditHorizontalLeftGreen->setText(QString::number(horizontalLeftColor.green()));
	ui.lineEditHorizontalLeftBlue->setText(QString::number(horizontalLeftColor.blue()));
}

QColor OptionDialog::getHorizontalLeftColor() {
	return QColor(ui.lineEditHorizontalLeftRed->text().toInt(), ui.lineEditHorizontalLeftGreen->text().toInt(), ui.lineEditHorizontalLeftBlue->text().toInt());
}

void OptionDialog::setHorizontalRightColor(const QColor& horizontalRightColor) {
	ui.lineEditHorizontalRightRed->setText(QString::number(horizontalRightColor.red()));
	ui.lineEditHorizontalRightGreen->setText(QString::number(horizontalRightColor.green()));
	ui.lineEditHorizontalRightBlue->setText(QString::number(horizontalRightColor.blue()));
}

QColor OptionDialog::getHorizontalRightColor() {
	return QColor(ui.lineEditHorizontalRightRed->text().toInt(), ui.lineEditHorizontalRightGreen->text().toInt(), ui.lineEditHorizontalRightBlue->text().toInt());
}

void OptionDialog::setVerticalColor(const QColor& verticalColor) {
	ui.lineEditVerticalRed->setText(QString::number(verticalColor.red()));
	ui.lineEditVerticalGreen->setText(QString::number(verticalColor.green()));
	ui.lineEditVerticalBlue->setText(QString::number(verticalColor.blue()));
}

QColor OptionDialog::getVerticalColor() {
	return QColor(ui.lineEditVerticalRed->text().toInt(), ui.lineEditVerticalGreen->text().toInt(), ui.lineEditVerticalBlue->text().toInt());
}

void OptionDialog::setSilhouetteWidth(int silhouetteWidth) {
	ui.lineEditSilhouetteWidth->setText(QString::number(silhouetteWidth));
}

int OptionDialog::getSilhouetteWidth() {
	return ui.lineEditSilhouetteWidth->text().toInt();
}

void OptionDialog::setSilhouetteColor(const QColor& silhouetteColor) {
	ui.lineEditSilhouetteRed->setText(QString::number(silhouetteColor.red()));
	ui.lineEditSilhouetteGreen->setText(QString::number(silhouetteColor.green()));
	ui.lineEditSilhouetteBlue->setText(QString::number(silhouetteColor.blue()));
}

QColor OptionDialog::getSilhouetteColor() {
	return QColor(ui.lineEditSilhouetteRed->text().toInt(), ui.lineEditSilhouetteGreen->text().toInt(), ui.lineEditSilhouetteBlue->text().toInt());
}

void OptionDialog::onOK() {
	accept();
}

void OptionDialog::onCancel() {
	reject();
}
