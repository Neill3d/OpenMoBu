
############################################################
#
#  Script plotting on skeleton and removing control rig
# then create a new control rig and back plot animation from skeleton
#
# Author Sergey Solohin (Neill3d)
# homepage: http://neill3d.com
# e-mail to: s@neill3d.com
# 2012
############################################################


from pyfbsdk import *

def GetParent(model):
    parentModel = model.Parent
    if parentModel: 
        parentModel = GetParent(parentModel)
    else: 
        parentModel = model

    
    return parentModel

def SelectHierarchy(model):
    model.Selected = True
    for child in model.Children:
        SelectHierarchy(child)

def Replot():
    
    currChar = FBApplication().CurrentCharacter
    if currChar:
        hipsModel = currChar.GetModel(FBBodyNodeId.kFBHipsNodeId)
        if hipsModel:
            print hipsModel.Name
            parentModel = GetParent(hipsModel)
            if parentModel:
                SelectHierarchy(parentModel) 
            
        # set some important plotting options
        plotOptions = FBPlotOptions()
        plotOptions.ConstantKeyReducerKeepOneKey = False
        plotOptions.UseConstantKeyReducer = False
        plotOptions.RotationFilterToApply = FBRotationFilter.kFBRotationFilterGimbleKiller

        # plot selected
        take = FBSystem().CurrentTake
        lPeriod = FBTime(0,0,0,1)
        if take: take.PlotTakeOnSelected(lPeriod)

        # disable and delete control rig
        currChar.ActiveInput = False
        ctrlRig = currChar.GetCurrentControlSet()
        if ctrlRig: ctrlRig.FBDelete()

        # create new control rig
        if currChar.CreateControlRig(True):
            currChar.PlotAnimation(FBCharacterPlotWhere.kFBCharacterPlotOnControlRig, plotOptions)

Replot()

