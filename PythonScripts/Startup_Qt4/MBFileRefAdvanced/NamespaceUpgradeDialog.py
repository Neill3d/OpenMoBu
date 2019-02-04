import os
from PySide import QtCore, QtGui, QtUiTools
from NamespaceUpgradeDialogUI import Ui_NamespaceUpgradeDialog

class NamespaceUpgradeDialog( QtGui.QDialog, Ui_NamespaceUpgradeDialog ):
    def __init__( self, pParent = 0, pFlags = QtCore.Qt.Widget ):
        super( NamespaceUpgradeDialog, self ).__init__( pParent, pFlags )
        self.setupUi( self )
        
        import inspect
        self.mDefaultPath = os.path.dirname(inspect.currentframe().f_code.co_filename)

    def OnBtnBrowsePathClicked( self ):
        self.mFileToSave = QtGui.QFileDialog.getSaveFileName( self, "Save to FBX", self.mDefaultPath, "*.fbx" )
        if self.mFileToSave <> '':
            self.uiEditFilePath.setText( self.mFileToSave )
            self.mDefaultPath = self.mFileToSave

    def OnBtnOKClicked( self ):
        self.done( QtGui.QMessageBox.Ok )

    def OnBtnCancelClicked( self ):
        self.done( QtGui.QMessageBox.Cancel )