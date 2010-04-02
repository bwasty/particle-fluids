/********************************************************************************
** Form generated from reading UI file 'particlefluids.ui'
**
** Created: Fri 2. Apr 17:34:20 2010
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PARTICLEFLUIDS_H
#define UI_PARTICLEFLUIDS_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ParticleFluidsClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *ParticleFluidsClass)
    {
        if (ParticleFluidsClass->objectName().isEmpty())
            ParticleFluidsClass->setObjectName(QString::fromUtf8("ParticleFluidsClass"));
        ParticleFluidsClass->resize(600, 400);
        menuBar = new QMenuBar(ParticleFluidsClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        ParticleFluidsClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(ParticleFluidsClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        ParticleFluidsClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(ParticleFluidsClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        ParticleFluidsClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(ParticleFluidsClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        ParticleFluidsClass->setStatusBar(statusBar);

        retranslateUi(ParticleFluidsClass);

        QMetaObject::connectSlotsByName(ParticleFluidsClass);
    } // setupUi

    void retranslateUi(QMainWindow *ParticleFluidsClass)
    {
        ParticleFluidsClass->setWindowTitle(QApplication::translate("ParticleFluidsClass", "ParticleFluids", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ParticleFluidsClass: public Ui_ParticleFluidsClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PARTICLEFLUIDS_H
