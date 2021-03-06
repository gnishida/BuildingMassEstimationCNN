/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindowClass
{
public:
    QAction *actionOpenImage;
    QAction *actionExit;
    QAction *actionParameterEstimation;
    QAction *actionNew;
    QAction *actionOpenCGA;
    QAction *actionLoadSilhouette;
    QAction *actionSaveSilhouette;
    QAction *actionUndo;
    QAction *actionExtractCameraParameter;
    QAction *actionParameterEstimationWithCameraCalibration;
    QAction *actionParameterDialog;
    QAction *actionSaveScreen;
    QAction *actionOption;
    QAction *actionPenVanishingLine;
    QAction *actionPenSilhouette;
    QAction *actionClearLines;
    QAction *actionSaveLines;
    QAction *actionLoadLines;
    QAction *actionClearSilhouette;
    QAction *actionClearBackground;
    QWidget *centralWidget;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuOption;
    QMenu *menuPen;
    QMenu *menuEdit;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindowClass)
    {
        if (MainWindowClass->objectName().isEmpty())
            MainWindowClass->setObjectName(QStringLiteral("MainWindowClass"));
        MainWindowClass->resize(798, 853);
        actionOpenImage = new QAction(MainWindowClass);
        actionOpenImage->setObjectName(QStringLiteral("actionOpenImage"));
        actionExit = new QAction(MainWindowClass);
        actionExit->setObjectName(QStringLiteral("actionExit"));
        actionParameterEstimation = new QAction(MainWindowClass);
        actionParameterEstimation->setObjectName(QStringLiteral("actionParameterEstimation"));
        actionNew = new QAction(MainWindowClass);
        actionNew->setObjectName(QStringLiteral("actionNew"));
        actionOpenCGA = new QAction(MainWindowClass);
        actionOpenCGA->setObjectName(QStringLiteral("actionOpenCGA"));
        actionLoadSilhouette = new QAction(MainWindowClass);
        actionLoadSilhouette->setObjectName(QStringLiteral("actionLoadSilhouette"));
        actionSaveSilhouette = new QAction(MainWindowClass);
        actionSaveSilhouette->setObjectName(QStringLiteral("actionSaveSilhouette"));
        actionUndo = new QAction(MainWindowClass);
        actionUndo->setObjectName(QStringLiteral("actionUndo"));
        actionExtractCameraParameter = new QAction(MainWindowClass);
        actionExtractCameraParameter->setObjectName(QStringLiteral("actionExtractCameraParameter"));
        actionParameterEstimationWithCameraCalibration = new QAction(MainWindowClass);
        actionParameterEstimationWithCameraCalibration->setObjectName(QStringLiteral("actionParameterEstimationWithCameraCalibration"));
        actionParameterDialog = new QAction(MainWindowClass);
        actionParameterDialog->setObjectName(QStringLiteral("actionParameterDialog"));
        actionSaveScreen = new QAction(MainWindowClass);
        actionSaveScreen->setObjectName(QStringLiteral("actionSaveScreen"));
        actionOption = new QAction(MainWindowClass);
        actionOption->setObjectName(QStringLiteral("actionOption"));
        actionPenVanishingLine = new QAction(MainWindowClass);
        actionPenVanishingLine->setObjectName(QStringLiteral("actionPenVanishingLine"));
        actionPenVanishingLine->setCheckable(true);
        actionPenSilhouette = new QAction(MainWindowClass);
        actionPenSilhouette->setObjectName(QStringLiteral("actionPenSilhouette"));
        actionPenSilhouette->setCheckable(true);
        actionClearLines = new QAction(MainWindowClass);
        actionClearLines->setObjectName(QStringLiteral("actionClearLines"));
        actionSaveLines = new QAction(MainWindowClass);
        actionSaveLines->setObjectName(QStringLiteral("actionSaveLines"));
        actionLoadLines = new QAction(MainWindowClass);
        actionLoadLines->setObjectName(QStringLiteral("actionLoadLines"));
        actionClearSilhouette = new QAction(MainWindowClass);
        actionClearSilhouette->setObjectName(QStringLiteral("actionClearSilhouette"));
        actionClearBackground = new QAction(MainWindowClass);
        actionClearBackground->setObjectName(QStringLiteral("actionClearBackground"));
        centralWidget = new QWidget(MainWindowClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        MainWindowClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindowClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 798, 21));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        menuOption = new QMenu(menuBar);
        menuOption->setObjectName(QStringLiteral("menuOption"));
        menuPen = new QMenu(menuOption);
        menuPen->setObjectName(QStringLiteral("menuPen"));
        menuEdit = new QMenu(menuBar);
        menuEdit->setObjectName(QStringLiteral("menuEdit"));
        MainWindowClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindowClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindowClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindowClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindowClass->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuEdit->menuAction());
        menuBar->addAction(menuOption->menuAction());
        menuFile->addAction(actionClearBackground);
        menuFile->addAction(actionOpenImage);
        menuFile->addSeparator();
        menuFile->addAction(actionClearLines);
        menuFile->addAction(actionLoadLines);
        menuFile->addAction(actionSaveLines);
        menuFile->addSeparator();
        menuFile->addAction(actionClearSilhouette);
        menuFile->addAction(actionLoadSilhouette);
        menuFile->addAction(actionSaveSilhouette);
        menuFile->addSeparator();
        menuFile->addAction(actionOpenCGA);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);
        menuOption->addAction(actionParameterEstimation);
        menuOption->addSeparator();
        menuOption->addAction(menuPen->menuAction());
        menuOption->addAction(actionOption);
        menuPen->addAction(actionPenVanishingLine);
        menuPen->addAction(actionPenSilhouette);
        menuEdit->addAction(actionUndo);

        retranslateUi(MainWindowClass);

        QMetaObject::connectSlotsByName(MainWindowClass);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindowClass)
    {
        MainWindowClass->setWindowTitle(QApplication::translate("MainWindowClass", "Photo to 3D", 0));
        actionOpenImage->setText(QApplication::translate("MainWindowClass", "Open Image", 0));
        actionExit->setText(QApplication::translate("MainWindowClass", "Exit", 0));
        actionParameterEstimation->setText(QApplication::translate("MainWindowClass", "Parameter Estimation", 0));
        actionParameterEstimation->setShortcut(QApplication::translate("MainWindowClass", "Ctrl+P", 0));
        actionNew->setText(QApplication::translate("MainWindowClass", "New", 0));
        actionNew->setShortcut(QApplication::translate("MainWindowClass", "Ctrl+N", 0));
        actionOpenCGA->setText(QApplication::translate("MainWindowClass", "Open CGA", 0));
        actionLoadSilhouette->setText(QApplication::translate("MainWindowClass", "Load Silhouette", 0));
        actionLoadSilhouette->setShortcut(QApplication::translate("MainWindowClass", "Ctrl+O", 0));
        actionSaveSilhouette->setText(QApplication::translate("MainWindowClass", "Save Silhouette", 0));
        actionSaveSilhouette->setShortcut(QApplication::translate("MainWindowClass", "Ctrl+S", 0));
        actionUndo->setText(QApplication::translate("MainWindowClass", "Undo", 0));
        actionUndo->setShortcut(QApplication::translate("MainWindowClass", "Ctrl+Z", 0));
        actionExtractCameraParameter->setText(QApplication::translate("MainWindowClass", "Extract Camera Parameter", 0));
        actionParameterEstimationWithCameraCalibration->setText(QApplication::translate("MainWindowClass", "Parameter Estimation with Camera Calibration", 0));
        actionParameterDialog->setText(QApplication::translate("MainWindowClass", "Parameter Dialog", 0));
        actionSaveScreen->setText(QApplication::translate("MainWindowClass", "Save Screen", 0));
        actionOption->setText(QApplication::translate("MainWindowClass", "Option", 0));
        actionPenVanishingLine->setText(QApplication::translate("MainWindowClass", "Vanishing Line", 0));
        actionPenSilhouette->setText(QApplication::translate("MainWindowClass", "Silhouette", 0));
        actionClearLines->setText(QApplication::translate("MainWindowClass", "Clear Lines", 0));
        actionSaveLines->setText(QApplication::translate("MainWindowClass", "Save Lines", 0));
        actionLoadLines->setText(QApplication::translate("MainWindowClass", "Load Lines", 0));
        actionClearSilhouette->setText(QApplication::translate("MainWindowClass", "Clear Silhouette", 0));
        actionClearBackground->setText(QApplication::translate("MainWindowClass", "Clear Background", 0));
        menuFile->setTitle(QApplication::translate("MainWindowClass", "File", 0));
        menuOption->setTitle(QApplication::translate("MainWindowClass", "Tool", 0));
        menuPen->setTitle(QApplication::translate("MainWindowClass", "Pen", 0));
        menuEdit->setTitle(QApplication::translate("MainWindowClass", "Edit", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindowClass: public Ui_MainWindowClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
