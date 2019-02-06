import os
from PySide2 import QtCore, QtGui, QtWidgets
from DialogSwapRefFileUI import Ui_DialogSwapRefFile

class DialogSwapRefFile( QtWidgets.QDialog, Ui_DialogSwapRefFile ):
    def __init__( self, pParent = 0, pFlags = QtCore.Qt.Widget ):
        super( DialogSwapRefFile, self ).__init__( pParent, pFlags )
        self.setupUi( self )
    
    def OnBtnOKClicked( self ):
        self.done( QtWidgets.QMessageBox.Ok )