# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file '\bin\config\Scripts\Samples\Referencing\MBFileRefDemo\NamespaceUpgradeDialogUI.ui'
#
# Created: Wed Nov 07 14:10:37 2012
#      by: pyside-uic 0.2.14 running on PySide 1.1.2
#
# WARNING! All changes made in this file will be lost!

try:
    from PySide2 import QtCore, QtGui, QtWidgets
except ImportError:
    from PySide6 import QtCore, QtGui, QtWidgets

class Ui_NamespaceUpgradeDialog(object):
    def setupUi(self, NamespaceUpgradeDialog):
        NamespaceUpgradeDialog.setObjectName("NamespaceUpgradeDialog")
        NamespaceUpgradeDialog.resize(368, 146)
        self.verticalLayout = QtWidgets.QVBoxLayout(NamespaceUpgradeDialog)
        self.verticalLayout.setObjectName("verticalLayout")
        self.groupBox = QtWidgets.QGroupBox(NamespaceUpgradeDialog)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.groupBox.sizePolicy().hasHeightForWidth())
        self.groupBox.setSizePolicy(sizePolicy)
        self.groupBox.setObjectName("groupBox")
        self.horizontalLayout = QtWidgets.QHBoxLayout(self.groupBox)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.uiEditFilePath = QtWidgets.QLineEdit(self.groupBox)
        self.uiEditFilePath.setObjectName("uiEditFilePath")
        self.horizontalLayout.addWidget(self.uiEditFilePath)
        self.uiBtnBrowsePath = QtWidgets.QPushButton(self.groupBox)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.uiBtnBrowsePath.sizePolicy().hasHeightForWidth())
        self.uiBtnBrowsePath.setSizePolicy(sizePolicy)
        self.uiBtnBrowsePath.setMaximumSize(QtCore.QSize(23, 16777215))
        self.uiBtnBrowsePath.setAutoDefault(False)
        self.uiBtnBrowsePath.setObjectName("uiBtnBrowsePath")
        self.horizontalLayout.addWidget(self.uiBtnBrowsePath)
        self.verticalLayout.addWidget(self.groupBox)
        self.uiCbSaveAsText = QtWidgets.QCheckBox(NamespaceUpgradeDialog)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.uiCbSaveAsText.sizePolicy().hasHeightForWidth())
        self.uiCbSaveAsText.setSizePolicy(sizePolicy)
        self.uiCbSaveAsText.setChecked(True)
        self.uiCbSaveAsText.setObjectName("uiCbSaveAsText")
        self.verticalLayout.addWidget(self.uiCbSaveAsText)
        self.groupBox_2 = QtWidgets.QGroupBox(NamespaceUpgradeDialog)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.groupBox_2.sizePolicy().hasHeightForWidth())
        self.groupBox_2.setSizePolicy(sizePolicy)
        self.groupBox_2.setTitle("")
        self.groupBox_2.setObjectName("groupBox_2")
        self.horizontalLayout_2 = QtWidgets.QHBoxLayout(self.groupBox_2)
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        self.uiBtnOK = QtWidgets.QPushButton(self.groupBox_2)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.uiBtnOK.sizePolicy().hasHeightForWidth())
        self.uiBtnOK.setSizePolicy(sizePolicy)
        self.uiBtnOK.setDefault(True)
        self.uiBtnOK.setFlat(False)
        self.uiBtnOK.setObjectName("uiBtnOK")
        self.horizontalLayout_2.addWidget(self.uiBtnOK)
        self.uiBtnCancel = QtWidgets.QPushButton(self.groupBox_2)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.uiBtnCancel.sizePolicy().hasHeightForWidth())
        self.uiBtnCancel.setSizePolicy(sizePolicy)
        self.uiBtnCancel.setObjectName("uiBtnCancel")
        self.horizontalLayout_2.addWidget(self.uiBtnCancel)
        self.verticalLayout.addWidget(self.groupBox_2)

        self.retranslateUi(NamespaceUpgradeDialog)
        self.uiBtnBrowsePath.clicked.connect(NamespaceUpgradeDialog.OnBtnBrowsePathClicked)
        self.uiBtnOK.clicked.connect(NamespaceUpgradeDialog.OnBtnOKClicked)
        self.uiBtnCancel.clicked.connect(NamespaceUpgradeDialog.OnBtnOKClicked)
        QtCore.QMetaObject.connectSlotsByName(NamespaceUpgradeDialog)

    def retranslateUi(self, NamespaceUpgradeDialog):
        NamespaceUpgradeDialog.setWindowTitle(QtWidgets.QApplication.translate("NamespaceUpgradeDialog", "Namespace Upgrading"))
        self.groupBox.setTitle(QtWidgets.QApplication.translate("NamespaceUpgradeDialog", "Choose a reference file"))
        self.uiBtnBrowsePath.setText(QtWidgets.QApplication.translate("NamespaceUpgradeDialog", "..."))
        self.uiCbSaveAsText.setText(QtWidgets.QApplication.translate("NamespaceUpgradeDialog", "Save as ASCII format"))
        self.uiBtnOK.setText(QtWidgets.QApplication.translate("NamespaceUpgradeDialog", "OK"))
        self.uiBtnCancel.setText(QtWidgets.QApplication.translate("NamespaceUpgradeDialog", "Cancel"))

