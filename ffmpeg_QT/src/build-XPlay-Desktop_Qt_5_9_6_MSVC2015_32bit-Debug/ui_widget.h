/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 5.9.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>
#include <XVideoWidget.h>
#include "XSlider.h"

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    XVideoWidget *vedio;
    QPushButton *pushButton;
    XSlider *playPos;
    QPushButton *isplay;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QStringLiteral("Widget"));
        Widget->resize(1280, 720);
        vedio = new XVideoWidget(Widget);
        vedio->setObjectName(QStringLiteral("vedio"));
        vedio->setGeometry(QRect(-20, -20, 1280, 720));
        pushButton = new QPushButton(Widget);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(90, 580, 101, 51));
        playPos = new XSlider(Widget);
        playPos->setObjectName(QStringLiteral("playPos"));
        playPos->setGeometry(QRect(30, 650, 1201, 21));
        playPos->setStyleSheet(QStringLiteral("background:rgb(255, 255, 0)"));
        playPos->setMaximum(999);
        playPos->setOrientation(Qt::Horizontal);
        isplay = new QPushButton(Widget);
        isplay->setObjectName(QStringLiteral("isplay"));
        isplay->setGeometry(QRect(220, 580, 101, 51));

        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QApplication::translate("Widget", "Widget", Q_NULLPTR));
        pushButton->setText(QApplication::translate("Widget", "Open", Q_NULLPTR));
        isplay->setText(QApplication::translate("Widget", "Play", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
