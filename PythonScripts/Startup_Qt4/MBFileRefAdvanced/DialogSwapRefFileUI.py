# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file '\bin\config\Scripts\Samples\Referencing\MBFileRefDemo\DialogSwapRefFileUI.ui'
#
# Created: Wed Nov 07 14:09:33 2012
#      by: pyside-uic 0.2.14 running on PySide 1.1.2
#
# WARNING! All changes made in this file will be lost!

from PySide import QtCore, QtGui

class Ui_DialogSwapRefFile(object):
    def setupUi(self, DialogSwapRefFile):
        DialogSwapRefFile.setObjectName("DialogSwapRefFile")
        DialogSwapRefFile.resize(400, 155)
        self.verticalLayout_2 = QtGui.QVBoxLayout(DialogSwapRefFile)
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.groupBox = QtGui.QGroupBox(DialogSwapRefFile)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Expanding)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.groupBox.sizePolicy().hasHeightForWidth())
        self.groupBox.setSizePolicy(sizePolicy)
        self.groupBox.setTitle("")
        self.groupBox.setObjectName("groupBox")
        self.verticalLayout = QtGui.QVBoxLayout(self.groupBox)
        self.verticalLayout.setObjectName("verticalLayout")
        self.uiCbApplyTargetEdit = QtGui.QCheckBox(self.groupBox)
        self.uiCbApplyTargetEdit.setChecked(True)
        self.uiCbApplyTargetEdit.setObjectName("uiCbApplyTargetEdit")
        self.verticalLayout.addWidget(self.uiCbApplyTargetEdit)
        self.uiCbMergeCurrentEdit = QtGui.QCheckBox(self.groupBox)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.uiCbMergeCurrentEdit.sizePolicy().hasHeightForWidth())
        self.uiCbMergeCurrentEdit.setSizePolicy(sizePolicy)
        self.uiCbMergeCurrentEdit.setChecked(True)
        self.uiCbMergeCurrentEdit.setObjectName("uiCbMergeCurrentEdit")
        self.verticalLayout.addWidget(self.uiCbMergeCurrentEdit)
        self.label = QtGui.QLabel(self.groupBox)
        self.label.setObjectName("label")
        self.verticalLayout.addWidget(self.label)
        self.verticalLayout_2.addWidget(self.groupBox)
        self.groupBox_2 = QtGui.QGroupBox(DialogSwapRefFile)
        self.groupBox_2.setTitle("")
        self.groupBox_2.setObjectName("groupBox_2")
        self.horizontalLayout = QtGui.QHBoxLayout(self.groupBox_2)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.uiBtnOK = QtGui.QPushButton(self.groupBox_2)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.uiBtnOK.sizePolicy().hasHeightForWidth())
        self.uiBtnOK.setSizePolicy(sizePolicy)
        self.uiBtnOK.setDefault(True)
        self.uiBtnOK.setFlat(False)
        self.uiBtnOK.setObjectName("uiBtnOK")
        self.horizontalLayout.addWidget(self.uiBtnOK)
        self.verticalLayout_2.addWidget(self.groupBox_2)

        self.retranslateUi(DialogSwapRefFile)
        QtCore.QObject.connect(self.uiBtnOK, QtCore.SIGNAL("clicked()"), DialogSwapRefFile.OnBtnOKClicked)
        QtCore.QMetaObject.connectSlotsByName(DialogSwapRefFile)

    def retranslateUi(self, DialogSwapRefFile):
        DialogSwapRefFile.setWindowTitle(QtGui.QApplication.translate("DialogSwapRefFile", "Swap Reference File", None, QtGui.QApplication.UnicodeUTF8))
        self.uiCbApplyTargetEdit.setText(QtGui.QApplication.translate("DialogSwapRefFile", "Use edits from incoming file", None, QtGui.QApplication.UnicodeUTF8))
        self.uiCbMergeCurrentEdit.setText(QtGui.QApplication.translate("DialogSwapRefFile", "Use already loaded edits", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("DialogSwapRefFile", "( Checking both will merge all edits together )", None, QtGui.QApplication.UnicodeUTF8))
        self.uiBtnOK.setText(QtGui.QApplication.translate("DialogSwapRefFile", "OK", None, QtGui.QApplication.UnicodeUTF8))

