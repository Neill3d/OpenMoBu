from pyfbsdk import *
from pyfbsdk_additions import *

from PySide2 import QtCore, QtGui, QtWidgets
import NamespaceUpgradeDialog
import DialogSwapRefFile
import os

class NamespaceTableModel( QtCore.QAbstractTableModel ):
    namespaceRenamed = QtCore.Signal( str, str, name='namespaceRenamed' )
    namespaceFilePathSwapped = QtCore.Signal( object, str, str, bool, name='namespaceFilePathSwapped' )
    
    def __init__( self, pParentDialog ):
        super( NamespaceTableModel, self ).__init__()
        
        self.mParentDialog = pParentDialog
        self.mRefFilePath = {}
        self.mRefFileReload = {}
        
        self.mSys = FBSystem()
        self.mWatcher = QtCore.QFileSystemWatcher()
        
        self.mOnlyRef = True
        self.mNamespaces = []
        self.mNamespacesFlag = [] # is loaded and file exist
        self.Init()
        
    def UpdateRefNamespaces(self):
        
        self.mNamespaces = []
        self.mNamespacesFlag = []
        
        if True == self.mOnlyRef:
            for ns in self.mSys.Scene.Namespaces:
                if isinstance(ns, FBFileReference):
                    self.mNamespaces.append(ns)
                    flag = ns.IsLoaded and os.path.isfile(ns.ReferenceFilePath)
                    self.mNamespacesFlag.append(flag)
        else:
            for ns in self.mSys.Scene.Namespaces:
                self.mNamespaces.append(ns)
                self.mNamespacesFlag.append(False)
    
    def IsRefLoaded(self, ndx):
        return self.mNamespacesFlag[ndx]
    
    def GetNamespacesList(self):
        return self.mNamespaces
    
    def GetNamespace(self, ndx):
        return self.mNamespaces[ndx]
    
    def Init( self ):
        self.Connect()

    def Fini( self ):
        self.UpdateRefNamespaces()
        self.Disconnect()
        self.mWatcher.removePaths( self.mWatcher.files() )
        self.removeRows( 0, len(self.mNamespaces) )
        self.mRefFilePath = {}
        self.mRefFileReload = {}

    def Connect( self ):
        QtCore.QObject.connect(self.mWatcher, QtCore.SIGNAL("fileChanged(const QString&)"), self.OnFileChanged )

    def Disconnect( self ):
        QtCore.QObject.disconnect( self.mWatcher, QtCore.SIGNAL("fileChanged(const QString&)"), self.OnFileChanged )

    def removeRows( self, pRow, pCount, pParentIndex = QtCore.QModelIndex() ):
        if pCount <= 0 or pRow < 0 or pRow + pCount > self.rowCount( pParentIndex ):
            return False

        lList = FBStringList()
        for lNSIndex in range( pRow, pRow + pCount ):
            if self.mNamespaces[lNSIndex].Is( FBFileReference.TypeInfo ):
                lList.Add( self.mNamespaces[lNSIndex].LongName )  

        self.beginRemoveRows( pParentIndex, pRow, pRow + pCount - 1 )
        for lNSName in lList:
            self.mSys.Scene.NamespaceDelete( lNSName )
        self.endRemoveRows()

        return True

    def AddFileFromWatcher( self, pNSObj ):
        if not pNSObj.ReferenceFilePath in self.mRefFilePath:
            self.mRefFilePath[pNSObj.ReferenceFilePath] = []
            self.mRefFileReload[pNSObj.ReferenceFilePath] = False

        if not pNSObj.LongName in self.mRefFilePath[pNSObj.ReferenceFilePath]:
            self.mRefFilePath[pNSObj.ReferenceFilePath].append( pNSObj.LongName )

        self.UpdateFileWatcher()

    def RemoveFileFromWatcher( self, pNSObj ):
        if pNSObj.TypeInfo == FBNamespace.TypeInfo: return
        if not pNSObj.ReferenceFilePath in self.mRefFilePath: return

        if pNSObj.LongName in self.mRefFilePath[pNSObj.ReferenceFilePath]:
            self.mRefFilePath[pNSObj.ReferenceFilePath].remove( pNSObj.LongName )

        if len( self.mRefFilePath[pNSObj.ReferenceFilePath] ) == 0:
            del self.mRefFilePath[pNSObj.ReferenceFilePath]
            del self.mRefFileReload[pNSObj.ReferenceFilePath]

        self.UpdateFileWatcher()

    def UpdateFileWatcher( self ):
        self.mWatcher.removePaths( self.mWatcher.files() )

        if len( self.mRefFilePath.keys() ) == 0: return

        for lFilePath in self.mRefFilePath.keys():
            self.mWatcher.addPath( lFilePath )

    def OnFileChanged( self, pFile ):
        self.mRefFileReload[str(pFile)] = True
    
    def Refresh( self, pIndex = QtCore.QModelIndex() ):
        if pIndex.isValid():
            self.dataChanged.emit( pIndex, pIndex )
        else:
            self.reset()

    def rowCount( self, pIndex = QtCore.QModelIndex() ):
        if not pIndex.isValid():
            self.UpdateRefNamespaces()
            return len( self.mNamespaces )
        else:
            return 0

    def columnCount( self, pIndex = QtCore.QModelIndex() ):
        if not pIndex.isValid():
            return 4
        else:
            return 0

    def IsRefFileMonitored( self, pFileRefObj ):
        if pFileRefObj.TypeInfo == FBNamespace.TypeInfo: return False

        if not pFileRefObj.ReferenceFilePath in self.mRefFilePath:
            return False

        if not pFileRefObj.LongName in self.mRefFilePath[pFileRefObj.ReferenceFilePath]:
            return False

        return True

    def data( self, pIndex, pRole = QtCore.Qt.DisplayRole ):
        if not pIndex.isValid():
            return None

        self.UpdateRefNamespaces()

        if pIndex.row() >= len( self.mNamespaces ) or pIndex.row() < 0:
            return None

        lNSObj = self.mNamespaces[pIndex.row()]

        if pRole == QtCore.Qt.DisplayRole:
            if pIndex.column() == 0:
                if lNSObj.TypeInfo == FBFileReference.TypeInfo:
                    if lNSObj.IsLoaded and lNSObj.GetContentModified( FBPlugModificationFlag.kFBContentAllModifiedMask ):
                        return lNSObj.LongName + '*'
                return lNSObj.LongName
            elif pIndex.column() == 1:
                return lNSObj.TypeInfo == FBFileReference.TypeInfo
            elif pIndex.column() == 2:
                if self.IsRefFileMonitored(lNSObj):
                    return True
                else:
                    return False
            elif pIndex.column() == 3:
                if lNSObj.TypeInfo == FBFileReference.TypeInfo:
                    return lNSObj.ReferenceFilePath
                else:
                    return ''
        elif pRole == QtCore.Qt.ForegroundRole:
            if lNSObj.TypeInfo == FBFileReference.TypeInfo:
                if self.IsRefLoaded(pIndex.row()):
                    return QtGui.QBrush( QtCore.Qt.white )
                else:
                    return QtGui.QBrush( QtCore.Qt.darkGray )

        elif pRole == QtCore.Qt.CheckStateRole:
            '''
            if pIndex.column() == 1:
                if lNSObj.TypeInfo == FBFileReference.TypeInfo:
                    return QtCore.Qt.Checked
                else:
                    return QtCore.Qt.Unchecked
            elif
            '''
            if pIndex.column() == 2:
                if self.IsRefFileMonitored(lNSObj):
                    return QtCore.Qt.Checked
                else:
                    return QtCore.Qt.Unchecked

    def headerData( self, pSection, pOrientation, pRole = QtCore.Qt.DisplayRole ):
        if pRole <> QtCore.Qt.DisplayRole:
            return None

        if pOrientation == QtCore.Qt.Horizontal:
            if pSection == 0:
                return 'Namespace'
            elif pSection == 1:
                return 'Is Reference'
            elif pSection == 2:
                return 'File Monitored'
            elif pSection == 3:
                return 'Reference File Path'

        elif pOrientation == QtCore.Qt.Vertical:
            return pSection + 1

        return None

    def flags( self, pIndex ):
        
        if not pIndex.isValid(): return False
        
        if pIndex.row() >= len( self.mNamespaces ) or pIndex.row() < 0:
            return False
        
        lNSObj = self.mNamespaces[pIndex.row()]

        lFlag = QtCore.Qt.ItemIsEnabled | QtCore.Qt.ItemIsSelectable
        
        #if pIndex.column() == 1 or pIndex.column() == 2:
        if pIndex.column() == 2:
            lFlag = lFlag | QtCore.Qt.ItemIsUserCheckable
        
        if pIndex.column() == 0:
            lFlag = lFlag | QtCore.Qt.ItemIsEditable

        if pIndex.column() == 3:
            if lNSObj.Is( FBFileReference.TypeInfo ):
                lFlag = lFlag | QtCore.Qt.ItemIsEditable

        return lFlag

    def setData( self, pIndex, pValue, pRole = QtCore.Qt.EditRole ):
        
        if not pIndex.isValid(): return False

        if pIndex.row() >= len( self.mNamespaces ) or pIndex.row() < 0:
            return False

        if pRole == QtCore.Qt.EditRole:
            lNSObj = self.mNamespaces[pIndex.row()]
            if pIndex.column() == 0:
                if str(pValue) == '': return False
                lMsgBox = QtWidgets.QMessageBox( QtWidgets.QMessageBox.Information, 'Rename', 'Namespace %s will be renamed as %s. Do you want to proceed?' % ( lNSObj.LongName, str(pValue) ), QtGui.QMessageBox.Yes | QtGui.QMessageBox.No, self.mParentDialog )
                if lMsgBox.exec_() == QtWidgets.QMessageBox.Yes:
                    oldName = lNSObj.LongName
                    if True == self.mSys.Scene.NamespaceRename( lNSObj.LongName, str(pValue) ):
                        self.namespaceRenamed.emit( oldName, pValue )
                    #self.Refresh( pIndex, pIndex )
                    self.dataChanged.emit( pIndex, pIndex )
                    return True
            elif pIndex.column() == 3:
                lFileToLoad = pValue
                if lFileToLoad == '':
                    lFileToLoad, strFilter = QtWidgets.QFileDialog.getOpenFileName( self.mParentDialog, "Pick FBX to reference", self.mParentDialog.mDefaultPath, "*.fbx" )

                if lFileToLoad <> '':
                    lQFileInfo = QtCore.QFileInfo( unicode(lFileToLoad) )
                    
                    if not lQFileInfo.exists() or not lQFileInfo.suffix().lower() == 'fbx':
                        return False
                        
                    lSwapDlg = DialogSwapRefFile.DialogSwapRefFile( self.mParentDialog )
                    lSwapDlg.exec_()
                    oldPath = lNSObj.ReferenceFilePath
                    if True == lNSObj.SwapReferenceFilePath( str(lFileToLoad), lSwapDlg.uiCbApplyTargetEdit.checkState() == QtCore.Qt.Checked, lSwapDlg.uiCbMergeCurrentEdit.checkState() == QtCore.Qt.Checked ):
                        self.namespaceFilePathSwapped.emit( lNSObj, oldPath, str(lFileToLoad), lSwapDlg.uiCbMergeCurrentEdit.checkState() == QtCore.Qt.Checked )
                    self.dataChanged.emit( pIndex, pIndex )
                    return True

        elif pRole == QtCore.Qt.CheckStateRole:
            '''
            if pIndex.column() == 1:
                lNSObj = self.mSys.Scene.Namespaces[pIndex.row()]
                print lNSObj
                if lNSObj.TypeInfo == FBFileReference.TypeInfo:
                    if pValue == QtCore.Qt.Unchecked:
                        lMsgBox = QtGui.QMessageBox( QtGui.QMessageBox.Question, 'Namespace Downgrading', 'File Reference will be downgraded. Do you want to proceed?', QtGui.QMessageBox.Yes | QtGui.QMessageBox.No, self.mParentDialog )
                        if lMsgBox.exec_() == QtGui.QMessageBox.Yes:
                            self.RemoveFileFromWatcher( lNSObj )
                            self.mSys.Scene.NamespaceDowngradeFromFileReference( lNSObj.LongName )
                else:
                    if pValue == QtCore.Qt.Checked:
                        lDlg = NamespaceUpgradeDialog.NamespaceUpgradeDialog( self.mParentDialog )
                        if lNSObj.GetOwnerFileReference() is not None: 
                            lMsgBox = QtGui.QMessageBox( QtGui.QMessageBox.Warning, 'Namespace Upgrading', 'Referenced namespace is not allowed to be upgraded!', QtGui.QMessageBox.Ok, self.mParentDialog )
                            lMsgBox.exec_()
                        elif lDlg.exec_() == QtGui.QMessageBox.Ok:
                            if lDlg.uiCbSaveAsText.checkState() == QtCore.Qt.Checked:
                                lRet = self.mSys.Scene.NamespaceUpgradeToFileReference( lNSObj.LongName, str(lDlg.uiEditFilePath.text()), True )
                            else:
                                lRet = self.mSys.Scene.NamespaceUpgradeToFileReference( lNSObj.LongName, str(lDlg.uiEditFilePath.text()), False )
                    
                    self.dataChanged.emit( pIndex, pIndex )
                    return True
            elif 
            '''
            if pIndex.column() == 2:
                lNSObj = self.mNamespaces[pIndex.row()]
                if lNSObj.TypeInfo == FBFileReference.TypeInfo:
                    if pValue == QtCore.Qt.Checked:
                        #lMsgBox = QtGui.QMessageBox( QtGui.QMessageBox.Information, 'File Monitoring', 'File Reference is added to monitor.', QtGui.QMessageBox.Ok, self.mParentDialog )
                        #lMsgBox.exec_()

                        self.AddFileFromWatcher( lNSObj )

                    elif pValue == QtCore.Qt.Unchecked:
                        #lMsgBox = QtGui.QMessageBox( QtGui.QMessageBox.Information, 'File Monitoring', 'File Reference is removed from monitor.', QtGui.QMessageBox.Ok, self.mParentDialog )
                        #lMsgBox.exec_()

                        self.RemoveFileFromWatcher( lNSObj )
                    
                    self.dataChanged.emit( pIndex, pIndex )
                    return True
        return False
