import os
from PySide import QtCore, QtGui
from DialogSwapRefFileUI import Ui_DialogSwapRefFile

class DialogSwapRefFile( QtGui.QDialog, Ui_DialogSwapRefFile ):
    def __init__( self, pParent = 0, pFlags = QtCore.Qt.Widget ):
        super( DialogSwapRefFile, self ).__init__( pParent, pFlags )
        self.setupUi( self )
    
    def OnBtnOKClicked( self ):
        self.done( QtGui.QMessageBox.Ok )