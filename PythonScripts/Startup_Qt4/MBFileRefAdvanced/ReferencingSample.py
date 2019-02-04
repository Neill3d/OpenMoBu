import os

from pyfbsdk import *
from pyfbsdk_additions import *

from PySide import QtCore, QtGui
from ReferencingSampleUI2 import Ui_ReferencingSample

import NamespaceTableModel
import FbxShadersGraphImport as imp
import FbxShadersGraphBake as bake
import FbxShadersGraphMisc as misc

######################################################
# Functions
######################################################
def PickRefName():
    lFileRefName = ''
    lBtnClicked, lFileRefName = FBMessageBoxGetUserValue( 'File Reference', 'Enter your file reference name', '', FBPopupInputType.kFBPopupString, 'OK', 'Cancel', None, 1, True )
    if lBtnClicked == 2: return lBtnClicked, lFileRefName

    lFileRefName = lFileRefName.replace(' ','')
    while lFileRefName is '':
        FBMessageBox('File Reference', 'Error, please enter a file reference name', "OK")
        lBtnClicked, lFileRefName = PickRefName()
        if lBtnClicked == 2: break
    
    return lBtnClicked, lFileRefName

class MainForm( QtGui.QWidget, Ui_ReferencingSample ):
    def __init__( self, parent ):
        QtGui.QWidget.__init__( self, parent )
        
        self.setupUi( self )
        self.uiTreeNamespace.header().resizeSection(0, 300)
        
        import inspect
        self.mDefaultPath = inspect.currentframe().f_code.co_filename
        self.mSingleSelection = False
        
        self.mSys = FBSystem()
        self.mApp = FBApplication()
        
        self.mApp.OnFileNewCompleted.Add( self.OnFileNew )
        self.mApp.OnFileOpenCompleted.Add( self.OnFileOpen )
        
        self.mNSModel = NamespaceTableModel.NamespaceTableModel( self )
        self.uiTableNamespace.setModel( self.mNSModel )
        
        self.mTimer = QtCore.QTimer( self )
        
        self.mIsInitialized = False
        self.Init()
        
        self.DialogTitle = 'Reference Tool'
        
    def Init( self ):
        self.mNSModel.namespaceRenamed.connect( self.OnNamespaceRenamed )
        self.mNSModel.namespaceFilePathSwapped.connect( self.OnNamespaceFilePathSwapped )
        
        lSelectionModel = self.uiTableNamespace.selectionModel()
        QtCore.QObject.connect( lSelectionModel, QtCore.SIGNAL( 'selectionChanged(const QItemSelection&, const QItemSelection&)' ), self.OnTableNamespaceSelectionChanged )
        
        QtCore.QObject.connect( self.mTimer, QtCore.SIGNAL( 'timeout()' ), self.OnTimer )
        self.mTimer.start( 2000 )
        self.UpdateUI()

        self.mIsInitialized = True
    
    def Fini( self ):
        # disconnecting the update timer
        self.mTimer.stop()
        QtCore.QObject.disconnect( self.mTimer, QtCore.SIGNAL( 'timeout()' ), self.OnTimer )
        
        # disconnecting the selectionChanged signal
        lSelectionModel = self.uiTableNamespace.selectionModel()
        QtCore.QObject.disconnect( lSelectionModel, QtCore.SIGNAL( 'selectionChanged(const QItemSelection&, const QItemSelection&)' ), self.OnTableNamespaceSelectionChanged )
        
        self.mNSModel.namespaceRenamed.disconnect( self.OnNamespaceRenamed )
        self.mNSModel.namespaceFilePathSwapped.disconnect( self.OnNamespaceFilePathSwapped )
                
        self.mIsInitialized = False

    def OnShow( self ):
        if not self.mIsInitialized:
            self.Init()
            self.mNSModel.Connect()
            
            
        
    def OnHide( self ):
        if self.mIsInitialized:
            self.Fini()
            self.mNSModel.Disconnect()
            
            #self.mApp.OnFileNewCompleted.Remove( self.OnFileNew )
            #self.mApp.OnFileOpenCompleted.Remove( self.OnFileOpen )
    
    def OnFileNew( self, pControl, pEvent ):
        self.mNSModel.Fini()
        self.mNSModel.Init()

    def OnFileOpen( self, pControl, pEvent ):
        self.mNSModel.Refresh()
        self.UpdateUI()
        self.UpdateTreeNamespace()

    def OnNamespaceRenamed( self, pOldName, pNewName ):
        '''
        # change name of shaders namespace and change name of description holder
        lScene = self.mSys.Scene
        if True == lScene.NamespaceExist( str(pOldName) + 'S' ):
            lScene.NamespaceRename( str(pOldName) + 'S', str(pNewName) + 'S' )
        
        # holder
        objNS = lScene.NamespaceGet(str(pNewName))
        if objNS is not None:
            misc.DescriptionRename(objNS, str(pNewName))
        '''
        self.UpdateTreeNamespace()

    # applyEdits defines if we keep or remove shaders and import from scratch
    def OnNamespaceFilePathSwapped(self, objNS, pOldPath, pNewPath, applyEdits ):
        
        lScene = self.mSys.Scene
        
        if (False == applyEdits):
            
            # do we have any other instances with old path
            anyOtherInstance = False
            for ns in self.mSys.Scene.Namespaces:
                if isinstance(ns, FBFileReference):
                    refPath = ns.ReferenceFilePath
                    if pOldPath == refPath and ns != objNS:
                        anyOtherInstance = True
                        break
            
            if True == anyOtherInstance:
                misc.DisconnectAllShaders(objNS)
                misc.DescriptionDisconnect(objNS)
            else:
                misc.DeleteShadersGraph(objNS, pOldPath)
            
            modelsMask = []
            objNS.RevertRefEdit()
            imp.DoFbxShadersGraphImport(True, 2, objNS, modelsMask)
            
            # holder
            base = os.path.splitext(pNewPath)[0]
            newXml = base + '.xml'
            misc.DescriptionStore(objNS, str(newXml))
            
        else:
            misc.SwapShadersGraph(objNS, str(pOldPath), str(pNewPath))
    #
        
    def OnBtnBrowsePathClicked( self ):
        lFileToLoad = QtGui.QFileDialog.getOpenFileName(self, "Pick FBX to reference", self.mDefaultPath, "*.fbx" )[0]      # Returns a Tuple. Get the filename only
        lQFileInfo = QtCore.QFileInfo( lFileToLoad )
        
        lSuffix = str(lQFileInfo.suffix())
        
        if lQFileInfo.exists() and lSuffix.lower() == 'fbx':
            self.uiEditFilePath.setText( lFileToLoad )
            self.mDefaultPath = lQFileInfo.filePath()
        self.UpdateUI()

    def OnBtnLoadClicked( self ):
        
        lLoadCount = self.uiSpinLoadTimes.value()
        lBtnClicked, lName = PickRefName()
        if lBtnClicked == 2: return

        lNameList = FBStringList()
        if lLoadCount == 1:
            lNameList.Add( lName )
        else:
            for lCount in range( 0, lLoadCount ):
                if self.mSys.Scene.NamespaceGet( lName + str(lCount+1) ) <> None:
                    lMsgBox = QtGui.QMessageBox( QtGui.QMessageBox.Information, 'Loading', 'Creation of file reference %s will be skipped as it is already there.' % ( lName + str(lCount+1) ), QtGui.QMessageBox.Ok, self )
                    lMsgBox.exec_()
                else:
                    lNameList.Add( lName + str(lCount+1) )
        
        lStatus = self.mSys.Scene.NamespaceImportToMultiple( lNameList, str(self.uiEditFilePath.text()), True )
        # import shaders graph
        if True == lStatus:
            
            # create a description holder
            descHolder = FBCreateObject(misc.gDescAssetPath, misc.gDescTitle, lName + '_DescHolder')
            descProp = None
            
            if descHolder is not None:
                descProp = descHolder.PropertyList.Find('Reference Model')
                    
            
            # we do shaders graph copies
            for name in lNameList:
                objNS = self.mSys.Scene.NamespaceGet( name  )

                if descProp is not None:
                    FBConnect(objNS, descProp)
                
                if objNS.IsLoaded:
                    modelsMask = []
                    imp.DoFbxShadersGraphImport(True, 1, objNS, modelsMask)
                    if descHolder is not None:
                        prop = descHolder.PropertyList.Find('Store')
                        if prop is not None:
                            prop()
                else:
                    prop = objNS.PropertyCreate( 'FirstTime',  FBPropertyType.kFBPT_bool, 'Bool', False, True, None )
                    if prop is not None:
                        prop.Data = True
                    
        self.mNSModel.Refresh()
        self.UpdateUI()

    def OnBtnUnloadClicked( self ):
        lIndexes = self.uiTableNamespace.selectedIndexes()
        for lIndex in lIndexes:
            if lIndex.column() == 0:
                lNSObj = self.mNSModel.GetNamespace(lIndex.row())
                self.mNSModel.RemoveFileFromWatcher( lNSObj )
                lNSObj.IsLoaded = False

                #
                refPath = lNSObj.ReferenceFilePath
                if (0 == len(refPath)) or (False == os.path.isfile(refPath)):
                    FBMessageBox(self.DialogTitle, 'Reference file is not found!\n Please enter a new file path', 'Ok')

            self.mNSModel.Refresh( lIndex )
        self.UpdateUI()
        self.UpdateTreeNamespace()

    def OnBtnReloadClicked( self ):
        lIndexes = self.uiTableNamespace.selectedIndexes()
        for lIndex in lIndexes:
            if lIndex.column() == 0:
                lNSObj = self.mNSModel.GetNamespace(lIndex.row())
                '''
                filename = lNSObj.ReferenceFilePath
                if False == os.path.isfile(filename):
                    FBMessageBox(self.DialogTitle, 'Reference file is not found!\n Please enter a new file path', 'Ok')
                else:
                    
                    isUpdated = imp.HasReferenceNewUpdate(lNSObj)
                    lOption = 3
                    
                    if isUpdated:
                        # ask user to reload a shaders graph
                        lOption = FBMessageBox( "External File Changed", "Please choose the following action for Reference: %s!" % ( filename ), "Load", "Merge", "Ignore" )                        
                    
                    if lOption <> 3:
                        lUndo = FBUndoManager()
                        lUndo.Clear()
                        
                        diff = []
                        
                        edits = ''
                        misc.DisconnectAllShaders(lNSObj)
                        if lNSObj.GetContentModified( FBPlugModificationFlag.kFBContentAllModifiedMask ):
                            edits = lNSObj.GetRefEdit()
                        
                        lNSObj.IsLoaded = False
                        lNSObj.IsLoaded = True
                        
                        filename = lNSObj.ReferenceFilePath
                    
                        base = os.path.splitext(filename)[0]
                        xmlname = base + '.xml'
                        
                        if lOption == 2:
                            #if lNSObj.GetContentModified( FBPlugModificationFlag.kFBContentAllModifiedMask ):
                            #edits = lNSObj.GetRefEdit()
                            imp.DoFbxShadersGraphImport(True, 2, lNSObj, diff)
                            #imp.LoadShaderGraphResources(xmlname, xmlname, lNSObj, True, True)
                            if len(edits) > 0:
                                lNSObj.RevertRefEdit()
                                lNSObj.ApplyRefEditPyScriptFromString( edits )
                                
                            
                            imp.LoadShaderGraphConnections(xmlname, xmlname, lNSObj, True)
                                
                            # TODO: process shader graph for the new added models
                            # diff
                            
                        else:
                            modelsMask = []
                            lNSObj.RevertRefEdit()
                            imp.DoFbxShadersGraphImport(True, 2, lNSObj, modelsMask)
                        
                        # store new xml data
                        misc.DescriptionStore(lNSObj)

                    else:
                        lNSObj.IsLoaded = True
                    
                        if True == lNSObj.IsLoaded:
                            prop = lNSObj.PropertyList.Find('FirstTime')
                            if prop is not None:
                                modelsMask = []
                                imp.DoFbxShadersGraphImport(True, 2, lNSObj, modelsMask)
                                lNSObj.PropertyRemove(prop)
                        else:
                            FBMessageBox(self.DialogTitle, 'Failed to reload a reference', 'Ok')
                '''    
                self.mNSModel.Refresh( lIndex )
        self.UpdateUI()
        self.UpdateTreeNamespace()

    def OnBtnDeleteClicked( self ):
        lIndexes = self.uiTableNamespace.selectedIndexes()
        lNameList = []
        for lIndex in lIndexes:
            if lIndex.column() == 0:
                lNSObj = self.mNSModel.GetNamespace(lIndex.row())
                lNameList.append( lNSObj.LongName )
                self.mNSModel.RemoveFileFromWatcher( lNSObj )
                '''
                # check if we have any shaders graph connected to delete
                anyInstances = False
                for namespace in self.mSys.Scene.Namespaces:
                    if namespace != lNSObj:
                        if isinstance(namespace, FBFileReference):
                            if namespace.ReferenceFilePath == lNSObj.ReferenceFilePath:
                                anyInstances = True
                
                if False == anyInstances:
                    misc.DeleteShadersGraph(lNSObj)
                    # let's find any Description Holder connection
                    misc.DescriptionDelete(lNSObj)
                '''
        for lName in lNameList:
            self.mSys.Scene.NamespaceDelete( lName )
        
        self.mNSModel.Refresh()
        self.UpdateUI()
        self.UpdateTreeNamespace()
        
    def OnBtnInstanceClicked( self ):
        lIndexes = self.uiTableNamespace.selectedIndexes()
        for lIndex in lIndexes:
            if lIndex.column() == 0:
                lNSObj = self.mNSModel.GetNamespace(lIndex.row())

        lInstanceCount = self.uiSpinInstanceTimes.value()
        lBtnClicked, lName = PickRefName()
        if lBtnClicked == 2: return

        lNameList = FBStringList()
        if lInstanceCount == 1:
            lNameList.Add( lName )
        else:
            for lCount in range( 0, lInstanceCount ):
                lNameList.Add( lName + str(lCount+1) )

        lApplyRefEdits = False
        if lNSObj.GetContentModified( FBPlugModificationFlag.kFBContentAllModifiedMask ):
            lMsgBox = QtGui.QMessageBox( QtGui.QMessageBox.Question, 'Instancing', 'Do you want to apply reference edits after instancing?', QtGui.QMessageBox.Yes | QtGui.QMessageBox.No, self )
            if lMsgBox.exec_() == QtGui.QMessageBox.Yes:
                lApplyRefEdits = True

        lStatus = lNSObj.DuplicateFileRef( lNameList, lApplyRefEdits )

        if True == lStatus:
            misc.DescriptionConnectModels(lNSObj, lNameList)
                    
        self.mNSModel.Refresh()
        self.UpdateUI()
        self.UpdateTreeNamespace()
        
    def OnBtnRestoreClicked( self ):
        lIndexes = self.uiTableNamespace.selectedIndexes()
        for lIndex in lIndexes:
            if lIndex.column() == 0:
                lNSObj = self.mNSModel.GetNamespace(lIndex.row())
                lNSObj.RevertRefEdit()
                # if we have shaders graph, let's assign it from scratch
                imp.RestoreShadersGraph(lNSObj)
                
                self.mNSModel.Refresh( lIndex )
        self.UpdateUI()

    def OnBtnShowEditsClicked( self ):
        lIndexes = self.uiTableNamespace.selectedIndexes()
        for lIndex in lIndexes:
            if lIndex.column() == 0:
                lNSObj = self.mNSModel.GetNamespace(lIndex.row())
                if lNSObj.GetContentModified( FBPlugModificationFlag.kFBContentAllModifiedMask ):
                    
                    text = lNSObj.GetRefEdit()
                    
                    lDialog = FBFilePopup()
                    lDialog.Filter = '*.txt'
                    lDialog.Style = FBFilePopupStyle.kFBFilePopupSave
                    
                    if (lDialog.Execute()):
                        fname = lDialog.FullFilename
                        
                        with open(fname, "w") as text_file:
                            text_file.write(text)
                    
                    
                    #lMsgBox = QtGui.QMessageBox( self )
                    #lMsgBox.setText( lNSObj.GetRefEdit() )
                    #lMsgBox.exec_()
    
    def OnBtnShaderRestoreClicked(self):
        lIndexes = self.uiTableNamespace.selectedIndexes()
        for lIndex in lIndexes:
            if lIndex.column() == 0:
                lNSObj = self.mNSModel.GetNamespace(lIndex.row())
                lNSObj.RevertRefEdit()
                # if we have shaders graph, let's assign it from scratch
                imp.RestoreShadersGraph(lNSObj)
                
                self.mNSModel.Refresh( lIndex )
        self.UpdateUI()
        
    def OnBtnShaderBakeClicked(self):
        #lMsgBox = QtGui.QMessageBox( self )
        #lMsgBox.setText( 'Shader Edits Bake Here!' )
        #lMsgBox.exec_()
        
        lIndexes = self.uiTableNamespace.selectedIndexes()
        for lIndex in lIndexes:
            if lIndex.column() == 0:
                lNSObj = self.mNSModel.GetNamespace(lIndex.row())        
                
                filename = lNSObj.ReferenceFilePath
            
                base = os.path.splitext(filename)[0]
                xmlname = base + '.xml'                
                
                bake.BakeShadersGraphEdits(lNSObj, xmlname)
    
    def OnBtnShowShaderEditsClicked(self):
        
        if self.mSingleSelection:
            lIndexes = self.uiTableNamespace.selectedIndexes()
            lNSObj = self.mNSModel.GetNamespace(lIndexes[0].row())
        
            lShaders = []
            lList = []
            
            misc.CollectReferenceContent(lNSObj, lList)
            
            for lPlug in lList:
                
                if isinstance(lPlug, FBModel):
                    for lShader in lPlug.Shaders:
                        if not (lShader in lShaders):
                            lShaders.append(lShader)

            # compare shaders with shaders Graph description
            text = imp.CompareShadersGraph(lNSObj, lShaders)
            
            lMsgBox = QtGui.QMessageBox( self )
            lMsgBox.setText( text )
            lMsgBox.exec_()
    
    def OnTableNamespaceSelectionChanged( self, pSelected, pUnselected ):
        self.UpdateUI()
        self.UpdateTreeNamespace()
    
    def UpdateTreeNamespace( self ):
        self.uiTreeNamespace.clear()

        if self.mSingleSelection:
            lIndexes = self.uiTableNamespace.selectedIndexes()
            lNSObj = self.mNSModel.GetNamespace(lIndexes[0].row())
            
            lStrList = [lNSObj.LongName]
            if lNSObj.ClassGroupName <> '':
                lStrList.append( lNSObj.ClassGroupName )

            lItem = QtGui.QTreeWidgetItem( lStrList )
            self.uiTreeNamespace.addTopLevelItem( lItem )
            self.UpdateTreeNamespaceRecursively( lItem, lNSObj )
            self.uiTreeNamespace.expandItem( lItem )
            
            if isinstance(lNSObj, FBFileReference):
                lShadersItem = QtGui.QTreeWidgetItem( ['Shaders Graph'] )
                lItem.addChild( lShadersItem )
                self.PopulateShadersGraph( lShadersItem, lNSObj )

    def UpdateTreeNamespaceRecursively( self, pParentItem, pNSObj ):
        lList = FBComponentList()
        pNSObj.GetContentList( lList, FBPlugModificationFlag.kFBPlugAllContent, False )
        for lPlug in lList:
            lStrList = [lPlug.LongName]
            if lPlug.ClassGroupName <> '':
                lStrList.append( lPlug.ClassGroupName )
            else:
                lStrList.append( lPlug.ClassName() )

            lItem = QtGui.QTreeWidgetItem( lStrList )
            pParentItem.addChild( lItem )
            self.uiTreeNamespace.expandItem( lItem )
            if lPlug.TypeInfo == FBNamespace.TypeInfo:
                self.UpdateTreeNamespaceRecursively( lItem, lPlug )

    def PopulateShadersGraph(self, pParentItem, pNSObj):

        lList = []
        misc.CollectReferenceContent(pNSObj, lList)
        lShaders = []
        lShaderModels = []
        
        for lPlug in lList:
            
            if isinstance(lPlug, FBModel):
                for lShader in lPlug.Shaders:
                    if lShader in lShaders:
                        ndx = lShaders.index(lShader)
                        lShaderModels[ndx].append(lPlug)
                    else:
                        lShaders.append(lShader)
                        lShaderModels.append([lPlug])
                    
        
        for shader, models in zip(lShaders, lShaderModels):
        
            lStrList = [shader.LongName]
            lStrModels = misc.MBGetShaderTypeName(shader)
            '''
            for model in models:
                lStrModels = lStrModels + model.Name
                lStrModels = lStrModels + '; '
            '''
            lStrList.append(lStrModels)
            
            lItem = QtGui.QTreeWidgetItem( lStrList )
            pParentItem.addChild( lItem )
            self.uiTreeNamespace.expandItem( lItem )

    #def OnSceneChanged( self, pComp, pEvent ):
    #    self.UpdateRefEditStatus()

    #def UpdateRefEditStatus( self ):
    #    lRowSize = self.mNSModel.rowCount()
    #    for lRow in range( 0, lRowSize ):
    #        lIndex = self.mNSModel.index( lRow, 0 )
    #        self.mNSModel.dataChanged.emit( lIndex, lIndex )

    def UpdateUI( self ):
        luiBtnBrowsePath = True
        luiBtnLoad = False
        luiSpinLoadTimes = False
        luiBtnUnload = False
        luiBtnReload = False
        luiBtnDelete = False
        luiBtnInstance = False
        luiBtnRestore = False
        luiBtnShowEdits = False
        lShaderEdits = False
        luiSpinInstanceTimes = False

        if os.path.exists( self.uiEditFilePath.text() ):
            luiBtnLoad = True
            luiSpinLoadTimes = True

        #
        # Update file reference related UI
        #
        lIndexes = self.uiTableNamespace.selectedIndexes()
        
        lRows = []
        lHasSelection = False
        if len( lIndexes ) > 0:
            lHasSelection = True

        lAllSelectionFileReference = True
        lAllSelectionLoaded = True
        lAllSelectionUnLoaded = True
        lSelectionHasRefEdits = False
        for lIndex in lIndexes:
            if lIndex.column() == 0:
                lRows.append( lIndex.row() )
                lNSObj = self.mNSModel.GetNamespace(lIndex.row())
                if lNSObj.TypeInfo == FBNamespace.TypeInfo:
                    lAllSelectionFileReference = False
                else:
                    if lNSObj.IsLoaded:
                        if lNSObj.GetContentModified( FBPlugModificationFlag.kFBContentAllModifiedMask ):
                            lSelectionHasRefEdits = True
                        lAllSelectionUnLoaded = False
                        lShaderEdits = True
                    else:
                        lAllSelectionLoaded = False

        self.mSingleSelection = False
        if len( lRows ) == 1:
            self.mSingleSelection = True

        if lHasSelection:
            luiBtnDelete = True

            if lAllSelectionFileReference:
                if lSelectionHasRefEdits:
                    luiBtnRestore = True

                if self.mSingleSelection:
                    if lSelectionHasRefEdits:
                        luiBtnShowEdits = True
                    luiBtnInstance = True
                    luiSpinInstanceTimes = True

                luiBtnUnload = True
                luiBtnReload = True
                if lAllSelectionLoaded:
                    luiBtnReload = False
                if lAllSelectionUnLoaded:
                    luiBtnUnload = False

        self.uiBtnBrowsePath.setEnabled( luiBtnBrowsePath )
        self.uiBtnLoad.setEnabled( luiBtnLoad )
        self.uiSpinLoadTimes.setEnabled( luiSpinLoadTimes )
        self.uiBtnUnload.setEnabled( luiBtnUnload )
        self.uiBtnReload.setEnabled( luiBtnReload )
        self.uiBtnDelete.setEnabled( luiBtnDelete )
        self.uiBtnInstance.setEnabled( luiBtnInstance )
        self.uiBtnRestore.setEnabled( luiBtnRestore )
        self.uiBtnShowEdits.setEnabled( luiBtnShowEdits )
        self.uiSpinInstanceTimes.setEnabled( luiSpinInstanceTimes )

        # shader edis
        self.uiBtnRestoreShaders.setEnabled( lShaderEdits )
        self.uiBtnBakeShaders.setEnabled( lShaderEdits )
        self.uiBtnShowShadersEdits.setEnabled( lShaderEdits )

    def OnTimer( self ):
        for lFilePath, lReload in self.mNSModel.mRefFileReload.items():
            
            if False == os.path.isfile(lFilePath) and True == lReload:
                FBMessageBox( "External File Not Found", "The referenced file '%s' is not found!\nReference goes offline" % ( lFilePath ), "Ok" )
                
                if lFilePath in self.mNSModel.mRefFilePath:
                    for lFileRefName in self.mNSModel.mRefFilePath[lFilePath]:
                        lFileRefObj = FBFindObjectByFullName( 'FileReference::' + lFileRefName )
                        
                        if lFileRefObj is not None:
                            self.mNSModel.RemoveFileFromWatcher( lFileRefObj )
                            lFileRefObj.IsLoaded = False
                                
                    self.mNSModel.mRefFileReload[lFilePath] = False
                
                self.UpdateUI()
                self.UpdateTreeNamespace()
                
            elif lReload:
                FBMessageBox( "External File Changed", "The referenced file '%s' has been changed externally!" % ( lFilePath ), "OK" )
                if lFilePath in self.mNSModel.mRefFilePath:
                    for lFileRefName in self.mNSModel.mRefFilePath[lFilePath]:
                        lNSObj = FBFindObjectByFullName( 'FileReference::' + lFileRefName )
                        lOption = FBMessageBox( "External File Changed", "Please choose the following action for Reference: %s!" % ( lFileRefName ), "Load", "Merge", "Ignore" )
                        
                        if lOption <> 3:
                            lUndo = FBUndoManager()
                            lUndo.Clear()
                            
                            diff = []
                            
                            edits = ''
                            misc.DisconnectAllShaders(lNSObj)
                            if lNSObj.GetContentModified( FBPlugModificationFlag.kFBContentAllModifiedMask ):
                                edits = lNSObj.GetRefEdit()
                            
                            lNSObj.IsLoaded = False
                            lNSObj.IsLoaded = True
                            
                            filename = lNSObj.ReferenceFilePath
                            
                            base = os.path.splitext(filename)[0]
                            xmlname = base + '.xml'
                            
                            if lOption == 2:
                                
                                imp.DoFbxShadersGraphImport(True, 2, lNSObj, diff)
                                if len(edits) > 0:
                                    lNSObj.RevertRefEdit()
                                    lNSObj.ApplyRefEditPyScriptFromString( edits )
                                 
                                imp.LoadShaderGraphConnections(xmlname, xmlname, lNSObj, True)
                                
                            else:
                                modelsMask = []
                                lNSObj.RevertRefEdit()
                                imp.DoFbxShadersGraphImport(True, 2, lNSObj, modelsMask)
                            
                            # store new xml data
                            misc.DescriptionStore(lNSObj)
                            
                    self.mNSModel.mRefFileReload[lFilePath] = False

        self.mNSModel.UpdateFileWatcher()