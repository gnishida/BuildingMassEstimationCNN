/********************************************************************************
** Form generated from reading UI file 'ParameterDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PARAMETERDIALOG_H
#define UI_PARAMETERDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>

QT_BEGIN_NAMESPACE

class Ui_ParameterDialog
{
public:
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *label_6;
    QLabel *label_7;
    QLabel *label_8;
    QLabel *label_9;
    QDoubleSpinBox *doubleSpinBoxXrot;
    QDoubleSpinBox *doubleSpinBoxYrot;
    QDoubleSpinBox *doubleSpinBoxZrot;
    QDoubleSpinBox *doubleSpinBoxFOV;
    QDoubleSpinBox *doubleSpinBoxDepth1;
    QDoubleSpinBox *doubleSpinBoxDepth2;
    QDoubleSpinBox *doubleSpinBoxHeight;
    QDoubleSpinBox *doubleSpinBoxWidth1;
    QDoubleSpinBox *doubleSpinBoxWidth2;

    void setupUi(QDialog *ParameterDialog)
    {
        if (ParameterDialog->objectName().isEmpty())
            ParameterDialog->setObjectName(QStringLiteral("ParameterDialog"));
        ParameterDialog->resize(182, 304);
        label = new QLabel(ParameterDialog);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(20, 20, 41, 21));
        label_2 = new QLabel(ParameterDialog);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(20, 50, 41, 21));
        label_3 = new QLabel(ParameterDialog);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(20, 80, 41, 21));
        label_4 = new QLabel(ParameterDialog);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(20, 110, 41, 21));
        label_5 = new QLabel(ParameterDialog);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setGeometry(QRect(20, 140, 41, 21));
        label_6 = new QLabel(ParameterDialog);
        label_6->setObjectName(QStringLiteral("label_6"));
        label_6->setGeometry(QRect(20, 170, 41, 21));
        label_7 = new QLabel(ParameterDialog);
        label_7->setObjectName(QStringLiteral("label_7"));
        label_7->setGeometry(QRect(20, 200, 41, 21));
        label_8 = new QLabel(ParameterDialog);
        label_8->setObjectName(QStringLiteral("label_8"));
        label_8->setGeometry(QRect(20, 230, 41, 21));
        label_9 = new QLabel(ParameterDialog);
        label_9->setObjectName(QStringLiteral("label_9"));
        label_9->setGeometry(QRect(20, 260, 41, 21));
        doubleSpinBoxXrot = new QDoubleSpinBox(ParameterDialog);
        doubleSpinBoxXrot->setObjectName(QStringLiteral("doubleSpinBoxXrot"));
        doubleSpinBoxXrot->setGeometry(QRect(70, 20, 81, 22));
        doubleSpinBoxYrot = new QDoubleSpinBox(ParameterDialog);
        doubleSpinBoxYrot->setObjectName(QStringLiteral("doubleSpinBoxYrot"));
        doubleSpinBoxYrot->setGeometry(QRect(70, 50, 81, 22));
        doubleSpinBoxZrot = new QDoubleSpinBox(ParameterDialog);
        doubleSpinBoxZrot->setObjectName(QStringLiteral("doubleSpinBoxZrot"));
        doubleSpinBoxZrot->setGeometry(QRect(70, 80, 81, 22));
        doubleSpinBoxFOV = new QDoubleSpinBox(ParameterDialog);
        doubleSpinBoxFOV->setObjectName(QStringLiteral("doubleSpinBoxFOV"));
        doubleSpinBoxFOV->setGeometry(QRect(70, 110, 81, 22));
        doubleSpinBoxDepth1 = new QDoubleSpinBox(ParameterDialog);
        doubleSpinBoxDepth1->setObjectName(QStringLiteral("doubleSpinBoxDepth1"));
        doubleSpinBoxDepth1->setGeometry(QRect(70, 140, 81, 22));
        doubleSpinBoxDepth2 = new QDoubleSpinBox(ParameterDialog);
        doubleSpinBoxDepth2->setObjectName(QStringLiteral("doubleSpinBoxDepth2"));
        doubleSpinBoxDepth2->setGeometry(QRect(70, 170, 81, 22));
        doubleSpinBoxHeight = new QDoubleSpinBox(ParameterDialog);
        doubleSpinBoxHeight->setObjectName(QStringLiteral("doubleSpinBoxHeight"));
        doubleSpinBoxHeight->setGeometry(QRect(70, 200, 81, 22));
        doubleSpinBoxWidth1 = new QDoubleSpinBox(ParameterDialog);
        doubleSpinBoxWidth1->setObjectName(QStringLiteral("doubleSpinBoxWidth1"));
        doubleSpinBoxWidth1->setGeometry(QRect(70, 230, 81, 22));
        doubleSpinBoxWidth2 = new QDoubleSpinBox(ParameterDialog);
        doubleSpinBoxWidth2->setObjectName(QStringLiteral("doubleSpinBoxWidth2"));
        doubleSpinBoxWidth2->setGeometry(QRect(70, 260, 81, 22));

        retranslateUi(ParameterDialog);

        QMetaObject::connectSlotsByName(ParameterDialog);
    } // setupUi

    void retranslateUi(QDialog *ParameterDialog)
    {
        ParameterDialog->setWindowTitle(QApplication::translate("ParameterDialog", "ParameterDialog", 0));
        label->setText(QApplication::translate("ParameterDialog", "X rot:", 0));
        label_2->setText(QApplication::translate("ParameterDialog", "Y rot:", 0));
        label_3->setText(QApplication::translate("ParameterDialog", "Z rot:", 0));
        label_4->setText(QApplication::translate("ParameterDialog", "FOV:", 0));
        label_5->setText(QApplication::translate("ParameterDialog", "Depth1:", 0));
        label_6->setText(QApplication::translate("ParameterDialog", "Depth2:", 0));
        label_7->setText(QApplication::translate("ParameterDialog", "Height:", 0));
        label_8->setText(QApplication::translate("ParameterDialog", "Width1:", 0));
        label_9->setText(QApplication::translate("ParameterDialog", "Width2:", 0));
    } // retranslateUi

};

namespace Ui {
    class ParameterDialog: public Ui_ParameterDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PARAMETERDIALOG_H
