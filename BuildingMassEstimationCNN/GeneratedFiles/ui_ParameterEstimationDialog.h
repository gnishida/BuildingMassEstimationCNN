/********************************************************************************
** Form generated from reading UI file 'ParameterEstimationDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PARAMETERESTIMATIONDIALOG_H
#define UI_PARAMETERESTIMATIONDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>

QT_BEGIN_NAMESPACE

class Ui_ParameterEstimationDialog
{
public:
    QLabel *label_11;
    QLineEdit *lineEditFovMax;
    QLineEdit *lineEditYrotMin;
    QLineEdit *lineEditXrotMin;
    QLabel *label_13;
    QLineEdit *lineEditYrotMax;
    QLineEdit *lineEditXrotMax;
    QLabel *label_14;
    QLabel *label_10;
    QLineEdit *lineEditCameraDistance;
    QLabel *label_7;
    QLabel *label_6;
    QLabel *label_8;
    QLineEdit *lineEditFovMin;
    QPushButton *pushButtonCancel;
    QPushButton *pushButtonOK;
    QLabel *label_12;
    QRadioButton *radioButtonCameraTypeStreetView;
    QRadioButton *radioButtonCameraTypeAerialView;
    QCheckBox *checkBoxCentering3D;
    QCheckBox *checkBoxMeanSubtraction;
    QLabel *label_15;
    QLineEdit *lineEditGrammarSnippet;
    QLabel *label_16;
    QCheckBox *checkBoxApplyTexture;
    QCheckBox *checkBoxRefinement;
    QCheckBox *checkBoxTryMultiples;
    QLabel *label_17;
    QLineEdit *lineEditNumMultipleTries;
    QLabel *label_18;
    QLineEdit *lineEditMaxNoise;
    QLabel *label_19;
    QCheckBox *checkBoxAutomaticRecognition;
    QLineEdit *lineEditZrotMin;
    QLabel *label_9;
    QLineEdit *lineEditZrotMax;
    QLabel *label_20;
    QLineEdit *lineEditIterations;
    QLabel *label;
    QCheckBox *checkBoxRotateContour;
    QLabel *label_21;
    QLineEdit *lineEditImageSize;
    QCheckBox *checkBoxGrayscale;

    void setupUi(QDialog *ParameterEstimationDialog)
    {
        if (ParameterEstimationDialog->objectName().isEmpty())
            ParameterEstimationDialog->setObjectName(QStringLiteral("ParameterEstimationDialog"));
        ParameterEstimationDialog->resize(585, 401);
        label_11 = new QLabel(ParameterEstimationDialog);
        label_11->setObjectName(QStringLiteral("label_11"));
        label_11->setGeometry(QRect(173, 240, 16, 20));
        label_11->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        lineEditFovMax = new QLineEdit(ParameterEstimationDialog);
        lineEditFovMax->setObjectName(QStringLiteral("lineEditFovMax"));
        lineEditFovMax->setGeometry(QRect(200, 240, 61, 20));
        lineEditYrotMin = new QLineEdit(ParameterEstimationDialog);
        lineEditYrotMin->setObjectName(QStringLiteral("lineEditYrotMin"));
        lineEditYrotMin->setGeometry(QRect(110, 180, 61, 20));
        lineEditXrotMin = new QLineEdit(ParameterEstimationDialog);
        lineEditXrotMin->setObjectName(QStringLiteral("lineEditXrotMin"));
        lineEditXrotMin->setGeometry(QRect(110, 150, 61, 20));
        label_13 = new QLabel(ParameterEstimationDialog);
        label_13->setObjectName(QStringLiteral("label_13"));
        label_13->setGeometry(QRect(173, 150, 16, 20));
        label_13->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        lineEditYrotMax = new QLineEdit(ParameterEstimationDialog);
        lineEditYrotMax->setObjectName(QStringLiteral("lineEditYrotMax"));
        lineEditYrotMax->setGeometry(QRect(200, 180, 61, 20));
        lineEditXrotMax = new QLineEdit(ParameterEstimationDialog);
        lineEditXrotMax->setObjectName(QStringLiteral("lineEditXrotMax"));
        lineEditXrotMax->setGeometry(QRect(200, 150, 61, 20));
        label_14 = new QLabel(ParameterEstimationDialog);
        label_14->setObjectName(QStringLiteral("label_14"));
        label_14->setGeometry(QRect(173, 180, 16, 20));
        label_14->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        label_10 = new QLabel(ParameterEstimationDialog);
        label_10->setObjectName(QStringLiteral("label_10"));
        label_10->setGeometry(QRect(30, 240, 71, 16));
        label_10->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        lineEditCameraDistance = new QLineEdit(ParameterEstimationDialog);
        lineEditCameraDistance->setObjectName(QStringLiteral("lineEditCameraDistance"));
        lineEditCameraDistance->setGeometry(QRect(110, 120, 61, 20));
        label_7 = new QLabel(ParameterEstimationDialog);
        label_7->setObjectName(QStringLiteral("label_7"));
        label_7->setGeometry(QRect(10, 120, 91, 20));
        label_7->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        label_6 = new QLabel(ParameterEstimationDialog);
        label_6->setObjectName(QStringLiteral("label_6"));
        label_6->setGeometry(QRect(30, 150, 71, 16));
        label_6->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        label_8 = new QLabel(ParameterEstimationDialog);
        label_8->setObjectName(QStringLiteral("label_8"));
        label_8->setGeometry(QRect(30, 180, 71, 16));
        label_8->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        lineEditFovMin = new QLineEdit(ParameterEstimationDialog);
        lineEditFovMin->setObjectName(QStringLiteral("lineEditFovMin"));
        lineEditFovMin->setGeometry(QRect(110, 240, 61, 20));
        pushButtonCancel = new QPushButton(ParameterEstimationDialog);
        pushButtonCancel->setObjectName(QStringLiteral("pushButtonCancel"));
        pushButtonCancel->setGeometry(QRect(470, 360, 91, 31));
        pushButtonOK = new QPushButton(ParameterEstimationDialog);
        pushButtonOK->setObjectName(QStringLiteral("pushButtonOK"));
        pushButtonOK->setGeometry(QRect(370, 360, 91, 31));
        label_12 = new QLabel(ParameterEstimationDialog);
        label_12->setObjectName(QStringLiteral("label_12"));
        label_12->setGeometry(QRect(30, 90, 71, 16));
        label_12->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        radioButtonCameraTypeStreetView = new QRadioButton(ParameterEstimationDialog);
        radioButtonCameraTypeStreetView->setObjectName(QStringLiteral("radioButtonCameraTypeStreetView"));
        radioButtonCameraTypeStreetView->setGeometry(QRect(110, 90, 82, 17));
        radioButtonCameraTypeAerialView = new QRadioButton(ParameterEstimationDialog);
        radioButtonCameraTypeAerialView->setObjectName(QStringLiteral("radioButtonCameraTypeAerialView"));
        radioButtonCameraTypeAerialView->setGeometry(QRect(210, 90, 82, 17));
        checkBoxCentering3D = new QCheckBox(ParameterEstimationDialog);
        checkBoxCentering3D->setObjectName(QStringLiteral("checkBoxCentering3D"));
        checkBoxCentering3D->setGeometry(QRect(300, 50, 91, 17));
        checkBoxMeanSubtraction = new QCheckBox(ParameterEstimationDialog);
        checkBoxMeanSubtraction->setObjectName(QStringLiteral("checkBoxMeanSubtraction"));
        checkBoxMeanSubtraction->setGeometry(QRect(410, 50, 121, 17));
        label_15 = new QLabel(ParameterEstimationDialog);
        label_15->setObjectName(QStringLiteral("label_15"));
        label_15->setGeometry(QRect(10, 10, 91, 20));
        label_15->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        lineEditGrammarSnippet = new QLineEdit(ParameterEstimationDialog);
        lineEditGrammarSnippet->setObjectName(QStringLiteral("lineEditGrammarSnippet"));
        lineEditGrammarSnippet->setGeometry(QRect(250, 10, 31, 20));
        label_16 = new QLabel(ParameterEstimationDialog);
        label_16->setObjectName(QStringLiteral("label_16"));
        label_16->setGeometry(QRect(290, 10, 41, 20));
        label_16->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        checkBoxApplyTexture = new QCheckBox(ParameterEstimationDialog);
        checkBoxApplyTexture->setObjectName(QStringLiteral("checkBoxApplyTexture"));
        checkBoxApplyTexture->setGeometry(QRect(110, 330, 91, 17));
        checkBoxRefinement = new QCheckBox(ParameterEstimationDialog);
        checkBoxRefinement->setObjectName(QStringLiteral("checkBoxRefinement"));
        checkBoxRefinement->setGeometry(QRect(110, 300, 91, 17));
        checkBoxTryMultiples = new QCheckBox(ParameterEstimationDialog);
        checkBoxTryMultiples->setObjectName(QStringLiteral("checkBoxTryMultiples"));
        checkBoxTryMultiples->setGeometry(QRect(110, 270, 91, 17));
        label_17 = new QLabel(ParameterEstimationDialog);
        label_17->setObjectName(QStringLiteral("label_17"));
        label_17->setGeometry(QRect(210, 270, 31, 16));
        label_17->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        lineEditNumMultipleTries = new QLineEdit(ParameterEstimationDialog);
        lineEditNumMultipleTries->setObjectName(QStringLiteral("lineEditNumMultipleTries"));
        lineEditNumMultipleTries->setGeometry(QRect(250, 270, 61, 20));
        label_18 = new QLabel(ParameterEstimationDialog);
        label_18->setObjectName(QStringLiteral("label_18"));
        label_18->setGeometry(QRect(330, 270, 61, 16));
        label_18->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        lineEditMaxNoise = new QLineEdit(ParameterEstimationDialog);
        lineEditMaxNoise->setObjectName(QStringLiteral("lineEditMaxNoise"));
        lineEditMaxNoise->setGeometry(QRect(400, 270, 41, 20));
        label_19 = new QLabel(ParameterEstimationDialog);
        label_19->setObjectName(QStringLiteral("label_19"));
        label_19->setGeometry(QRect(446, 270, 21, 20));
        label_19->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        checkBoxAutomaticRecognition = new QCheckBox(ParameterEstimationDialog);
        checkBoxAutomaticRecognition->setObjectName(QStringLiteral("checkBoxAutomaticRecognition"));
        checkBoxAutomaticRecognition->setGeometry(QRect(110, 10, 131, 17));
        lineEditZrotMin = new QLineEdit(ParameterEstimationDialog);
        lineEditZrotMin->setObjectName(QStringLiteral("lineEditZrotMin"));
        lineEditZrotMin->setGeometry(QRect(110, 210, 61, 20));
        label_9 = new QLabel(ParameterEstimationDialog);
        label_9->setObjectName(QStringLiteral("label_9"));
        label_9->setGeometry(QRect(30, 210, 71, 16));
        label_9->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        lineEditZrotMax = new QLineEdit(ParameterEstimationDialog);
        lineEditZrotMax->setObjectName(QStringLiteral("lineEditZrotMax"));
        lineEditZrotMax->setGeometry(QRect(200, 210, 61, 20));
        label_20 = new QLabel(ParameterEstimationDialog);
        label_20->setObjectName(QStringLiteral("label_20"));
        label_20->setGeometry(QRect(173, 210, 16, 20));
        label_20->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        lineEditIterations = new QLineEdit(ParameterEstimationDialog);
        lineEditIterations->setObjectName(QStringLiteral("lineEditIterations"));
        lineEditIterations->setGeometry(QRect(300, 300, 91, 20));
        label = new QLabel(ParameterEstimationDialog);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(230, 300, 71, 20));
        checkBoxRotateContour = new QCheckBox(ParameterEstimationDialog);
        checkBoxRotateContour->setObjectName(QStringLiteral("checkBoxRotateContour"));
        checkBoxRotateContour->setGeometry(QRect(350, 120, 101, 17));
        label_21 = new QLabel(ParameterEstimationDialog);
        label_21->setObjectName(QStringLiteral("label_21"));
        label_21->setGeometry(QRect(20, 50, 81, 16));
        label_21->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        lineEditImageSize = new QLineEdit(ParameterEstimationDialog);
        lineEditImageSize->setObjectName(QStringLiteral("lineEditImageSize"));
        lineEditImageSize->setGeometry(QRect(110, 50, 61, 20));
        checkBoxGrayscale = new QCheckBox(ParameterEstimationDialog);
        checkBoxGrayscale->setObjectName(QStringLiteral("checkBoxGrayscale"));
        checkBoxGrayscale->setGeometry(QRect(200, 50, 101, 17));
        QWidget::setTabOrder(checkBoxAutomaticRecognition, lineEditGrammarSnippet);
        QWidget::setTabOrder(lineEditGrammarSnippet, checkBoxCentering3D);
        QWidget::setTabOrder(checkBoxCentering3D, checkBoxMeanSubtraction);
        QWidget::setTabOrder(checkBoxMeanSubtraction, radioButtonCameraTypeStreetView);
        QWidget::setTabOrder(radioButtonCameraTypeStreetView, radioButtonCameraTypeAerialView);
        QWidget::setTabOrder(radioButtonCameraTypeAerialView, lineEditCameraDistance);
        QWidget::setTabOrder(lineEditCameraDistance, checkBoxRotateContour);
        QWidget::setTabOrder(checkBoxRotateContour, lineEditXrotMin);
        QWidget::setTabOrder(lineEditXrotMin, lineEditXrotMax);
        QWidget::setTabOrder(lineEditXrotMax, lineEditYrotMin);
        QWidget::setTabOrder(lineEditYrotMin, lineEditYrotMax);
        QWidget::setTabOrder(lineEditYrotMax, lineEditZrotMin);
        QWidget::setTabOrder(lineEditZrotMin, lineEditZrotMax);
        QWidget::setTabOrder(lineEditZrotMax, lineEditFovMin);
        QWidget::setTabOrder(lineEditFovMin, lineEditFovMax);
        QWidget::setTabOrder(lineEditFovMax, checkBoxTryMultiples);
        QWidget::setTabOrder(checkBoxTryMultiples, lineEditNumMultipleTries);
        QWidget::setTabOrder(lineEditNumMultipleTries, lineEditMaxNoise);
        QWidget::setTabOrder(lineEditMaxNoise, checkBoxRefinement);
        QWidget::setTabOrder(checkBoxRefinement, lineEditIterations);
        QWidget::setTabOrder(lineEditIterations, checkBoxApplyTexture);
        QWidget::setTabOrder(checkBoxApplyTexture, pushButtonOK);
        QWidget::setTabOrder(pushButtonOK, pushButtonCancel);

        retranslateUi(ParameterEstimationDialog);

        QMetaObject::connectSlotsByName(ParameterEstimationDialog);
    } // setupUi

    void retranslateUi(QDialog *ParameterEstimationDialog)
    {
        ParameterEstimationDialog->setWindowTitle(QApplication::translate("ParameterEstimationDialog", "ParameterEstimationDialog", 0));
        label_11->setText(QApplication::translate("ParameterEstimationDialog", "~", 0));
        label_13->setText(QApplication::translate("ParameterEstimationDialog", "~", 0));
        label_14->setText(QApplication::translate("ParameterEstimationDialog", "~", 0));
        label_10->setText(QApplication::translate("ParameterEstimationDialog", "Field of view:", 0));
        label_7->setText(QApplication::translate("ParameterEstimationDialog", "Camera distance:", 0));
        label_6->setText(QApplication::translate("ParameterEstimationDialog", "Xrot:", 0));
        label_8->setText(QApplication::translate("ParameterEstimationDialog", "Yrot:", 0));
        pushButtonCancel->setText(QApplication::translate("ParameterEstimationDialog", "Cancel", 0));
        pushButtonOK->setText(QApplication::translate("ParameterEstimationDialog", "OK", 0));
        label_12->setText(QApplication::translate("ParameterEstimationDialog", "Camera type:", 0));
        radioButtonCameraTypeStreetView->setText(QApplication::translate("ParameterEstimationDialog", "Street view", 0));
        radioButtonCameraTypeAerialView->setText(QApplication::translate("ParameterEstimationDialog", "Aerial view", 0));
        checkBoxCentering3D->setText(QApplication::translate("ParameterEstimationDialog", "3D Centering", 0));
        checkBoxMeanSubtraction->setText(QApplication::translate("ParameterEstimationDialog", "Mean subtraction", 0));
        label_15->setText(QApplication::translate("ParameterEstimationDialog", "Grammar snippet:", 0));
        label_16->setText(QApplication::translate("ParameterEstimationDialog", "(1 - 14)", 0));
        checkBoxApplyTexture->setText(QApplication::translate("ParameterEstimationDialog", "Apply Texture", 0));
        checkBoxRefinement->setText(QApplication::translate("ParameterEstimationDialog", "Refinement", 0));
        checkBoxTryMultiples->setText(QApplication::translate("ParameterEstimationDialog", "Try Multiples", 0));
        label_17->setText(QApplication::translate("ParameterEstimationDialog", "# Tries:", 0));
        label_18->setText(QApplication::translate("ParameterEstimationDialog", "Max Noise:", 0));
        label_19->setText(QApplication::translate("ParameterEstimationDialog", "%", 0));
        checkBoxAutomaticRecognition->setText(QApplication::translate("ParameterEstimationDialog", "Automatic recognition", 0));
        label_9->setText(QApplication::translate("ParameterEstimationDialog", "Zrot:", 0));
        label_20->setText(QApplication::translate("ParameterEstimationDialog", "~", 0));
        label->setText(QApplication::translate("ParameterEstimationDialog", "#Iterations:", 0));
        checkBoxRotateContour->setText(QApplication::translate("ParameterEstimationDialog", "Rotate Contour", 0));
        label_21->setText(QApplication::translate("ParameterEstimationDialog", "Image size:", 0));
        checkBoxGrayscale->setText(QApplication::translate("ParameterEstimationDialog", "Grayscale", 0));
    } // retranslateUi

};

namespace Ui {
    class ParameterEstimationDialog: public Ui_ParameterEstimationDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PARAMETERESTIMATIONDIALOG_H
