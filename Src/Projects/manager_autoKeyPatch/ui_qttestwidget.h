/********************************************************************************
** Form generated from reading UI file 'qttest.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTTESTWIDGET_H
#define UI_QTTESTWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDial>
#include <QtGui/QFrame>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QtTestWidget
{
public:
    QGroupBox *groupBox;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QPushButton *createCubeBtn;
    QLabel *label;
    QDial *rotateCubeDial;
    QLabel *label_2;
    QSlider *scaleCubeSlider;

    void setupUi(QFrame *QtTestWidget)
    {
        if (QtTestWidget->objectName().isEmpty())
            QtTestWidget->setObjectName(QString::fromUtf8("QtTestWidget"));
        QtTestWidget->resize(183, 312);
        QtTestWidget->setFrameShape(QFrame::NoFrame);
        QtTestWidget->setFrameShadow(QFrame::Raised);
        groupBox = new QGroupBox(QtTestWidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(0, 0, 181, 311));
        verticalLayoutWidget = new QWidget(groupBox);
        verticalLayoutWidget->setObjectName(QString::fromUtf8("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(10, 20, 160, 281));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        createCubeBtn = new QPushButton(verticalLayoutWidget);
        createCubeBtn->setObjectName(QString::fromUtf8("createCubeBtn"));

        verticalLayout->addWidget(createCubeBtn);

        label = new QLabel(verticalLayoutWidget);
        label->setObjectName(QString::fromUtf8("label"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy);
        label->setAlignment(Qt::AlignHCenter|Qt::AlignTop);

        verticalLayout->addWidget(label);

        rotateCubeDial = new QDial(verticalLayoutWidget);
        rotateCubeDial->setObjectName(QString::fromUtf8("rotateCubeDial"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(1);
        sizePolicy1.setHeightForWidth(rotateCubeDial->sizePolicy().hasHeightForWidth());
        rotateCubeDial->setSizePolicy(sizePolicy1);

        verticalLayout->addWidget(rotateCubeDial);

        label_2 = new QLabel(verticalLayoutWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label_2);

        scaleCubeSlider = new QSlider(verticalLayoutWidget);
        scaleCubeSlider->setObjectName(QString::fromUtf8("scaleCubeSlider"));
        scaleCubeSlider->setOrientation(Qt::Horizontal);

        verticalLayout->addWidget(scaleCubeSlider);


        retranslateUi(QtTestWidget);

        QMetaObject::connectSlotsByName(QtTestWidget);
    } // setupUi

    void retranslateUi(QFrame *QtTestWidget)
    {
        QtTestWidget->setWindowTitle(QApplication::translate("QtTestWidget", "Frame", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("QtTestWidget", "This is a native Qt Widget", 0, QApplication::UnicodeUTF8));
        createCubeBtn->setText(QApplication::translate("QtTestWidget", "Create Ultimate Cube", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("QtTestWidget", "Rotation", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("QtTestWidget", "Uniform scaling", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class QtTestWidget: public Ui_QtTestWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTTESTWIDGET_H
