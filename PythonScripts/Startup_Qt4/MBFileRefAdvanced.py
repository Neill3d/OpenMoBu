# Copyright 2012 Autodesk, Inc.  All rights reserved.
# Use of this software is subject to the terms of the Autodesk license agreement 
# provided at the time of installation or download, or which otherwise accompanies
# this software in either electronic or hard copy form.
#
# Script updated by Sergei <Neill3d> Solokhin 2018
#
# Script description:
#   Tool to manage references in MoBu
#
# Topic: FBWidgetHolder, FBTool, FBFileReference
#

from pyfbsdk import *
from pyfbsdk_additions import *
from PySide import QtCore, QtGui, shiboken
import sys, inspect, os

# Add temp sys.path
lCurFilePath = inspect.currentframe().f_code.co_filename
sys.path.append( os.path.dirname(lCurFilePath) )
sys.path.append( os.path.join( os.path.dirname(lCurFilePath), 'MBFileRefAdvanced' ) )
import ReferencingSample
import ReferencingSampleUI2
import FbxShadersGraphImport
import FbxShadersGraphBake
import FbxShadersGraphMisc
import NamespaceTableModel

reload(ReferencingSample)
reload(ReferencingSampleUI2)
reload(FbxShadersGraphImport)
reload(FbxShadersGraphBake)
reload(FbxShadersGraphMisc)
reload(NamespaceTableModel)

#
# Subclass FBWidgetHolder and override its WidgetCreate function
#
class NativeWidgetHolder(FBWidgetHolder):
    #
    # Override WidgetCreate function to create native widget via PySide.
    # \param  parentWidget  Memory address of Parent QWidget.
    # \return               Memory address of the child native widget.
    #
    def WidgetCreate(self, pWidgetParent):
               
        #
        # IN parameter pWidgetparent is the memory address of the parent Qt widget. 
        #   here we should PySide.shiboken.wrapInstance() function to convert it to PySide.QtWidget object.
        #   and use it the as the parent for native Qt widgets created via Python. 
        #   Similiar approach is available in the sip python module for PyQt 
        #
        # Only a single widget is allowed to be the *direct* child of the IN parent widget. 
        #
        self.mNativeQtWidget = ReferencingSample.MainForm(shiboken.wrapInstance(pWidgetParent, QtGui.QWidget))
       
        #
        # return the memory address of the *single direct* child QWidget. 
        #
        return shiboken.getCppPointer(self.mNativeQtWidget)[0]
    
    def ShowEvent(self, shown):
        if True == shown:
            self.mNativeQtWidget.OnShow()
        else:
            self.mNativeQtWidget.OnHide()
    
class FileReferenceTool(FBTool):
    def BuildLayout(self):
        x = FBAddRegionParam(0,FBAttachType.kFBAttachLeft,"")
        y = FBAddRegionParam(0,FBAttachType.kFBAttachTop,"")
        w = FBAddRegionParam(0,FBAttachType.kFBAttachRight,"")
        h = FBAddRegionParam(0,FBAttachType.kFBAttachBottom,"")
        self.AddRegion("main","main", x, y, w, h)
        self.SetControl("main", self.mNativeWidgetHolder)
                
    def __init__(self, name):
        FBTool.__init__(self, name)
        self.mNativeWidgetHolder = NativeWidgetHolder();
        self.BuildLayout()
        self.StartSizeX = 650
        self.StartSizeY = 500
        self.MinSizeX = 650
        self.MinSizeY = 500
        self.OnShow.Add(self.ShowEvent)
        
    def ShowEvent(self, pControl, pEvent):
        
        self.mNativeWidgetHolder.ShowEvent(pEvent.Shown)

gToolName = "File Reference Advanced Tool"

#Development? - need to recreate each time!!
gDEVELOPMENT = False

if gDEVELOPMENT:
    FBDestroyToolByName(gToolName)

if gToolName in FBToolList:
    tool = FBToolList[gToolName]
    ShowTool(tool)
else:
    tool=FileReferenceTool(gToolName)
    FBAddTool(tool)
    if gDEVELOPMENT:
        ShowTool(tool)

# Remove temp sys.path
sys.path.remove( os.path.join( os.path.dirname(lCurFilePath), 'MBFileRefAdvanced' ) )
sys.path.remove( os.path.dirname(lCurFilePath) )
