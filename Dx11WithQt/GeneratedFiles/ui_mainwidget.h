/********************************************************************************
** Form generated from reading UI file 'mainwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.4.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWIDGET_H
#define UI_MAINWIDGET_H

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

class Ui_MainWidgetClass
{
public:
    QWidget *centralWidget;
    QStatusBar *statusBar;
    QMenuBar *menuBar;
    QMenu *menu;
    QToolBar *toolBar;

    void setupUi(QMainWindow *MainWidgetClass)
    {
        if (MainWidgetClass->objectName().isEmpty())
            MainWidgetClass->setObjectName(QStringLiteral("MainWidgetClass"));
        MainWidgetClass->resize(600, 400);
        centralWidget = new QWidget(MainWidgetClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        MainWidgetClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(MainWidgetClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWidgetClass->setStatusBar(statusBar);
        menuBar = new QMenuBar(MainWidgetClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 600, 23));
        menu = new QMenu(menuBar);
        menu->setObjectName(QStringLiteral("menu"));
        MainWidgetClass->setMenuBar(menuBar);
        toolBar = new QToolBar(MainWidgetClass);
        toolBar->setObjectName(QStringLiteral("toolBar"));
        MainWidgetClass->addToolBar(Qt::TopToolBarArea, toolBar);

        menuBar->addAction(menu->menuAction());

        retranslateUi(MainWidgetClass);

        QMetaObject::connectSlotsByName(MainWidgetClass);
    } // setupUi

    void retranslateUi(QMainWindow *MainWidgetClass)
    {
        MainWidgetClass->setWindowTitle(QApplication::translate("MainWidgetClass", "MainWidget", 0));
        menu->setTitle(QApplication::translate("MainWidgetClass", "\350\217\234\345\215\225", 0));
        toolBar->setWindowTitle(QApplication::translate("MainWidgetClass", "toolBar", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWidgetClass: public Ui_MainWidgetClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWIDGET_H
