
#
#   libReStructuring
#
#       functions to reparent element preserving the animation trajectory
#
#   
#    mList = FBModelList()   
#    FBGetSelectedModels(mList)
#    CResturing.Hold(self, modelList)    
#
#

from pyfbsdk import *



class CRestructuring():

    lMgr = FBConstraintManager()
    mSystem = FBSystem()
    mApp = FBApplication()
    
    source = []
    nulls = []
    constraints = []

    # prepare constraint type
    lPosIdx = -1
    
    def __init__(self):

        self.lPosIdx = self.prepConstraint()
        print(self.lPosIdx)

    @staticmethod
    def prepConstraint():
        lMgr = FBConstraintManager()
        for lIdx in range( lMgr.TypeGetCount() ):
            if lMgr.TypeGetName( lIdx ) == 'Parent/Child':
                return lIdx
        return -1

    
    ###########################################################################
    #
    def Hold(self, modelList):
    
        self.source[:] = []
        self.nulls[:] = []
        self.constraints[:] = []
    
        for obj in modelList:
            newObj = FBModelNull( obj.LongName + '_Null' )
            newObj.Show = True
            newObj.Selected = True
            obj.Selected = False
            
            self.source.append(obj.LongName)
            self.nulls.append(newObj)
            
            if self.lPosIdx >= 0:
                lConstraint = self.lMgr.TypeCreateConstraint(self.lPosIdx)
                lConstraint.ReferenceAdd(0, newObj)
                lConstraint.ReferenceAdd(1, obj)    
                lConstraint.Lock = True
                lConstraint.Active = True

                self.constraints.append(lConstraint)
                
            
        # plot all new nulls    
        take = self.mSystem.CurrentTake
        lPeriod = FBTime(0,0,0,1)
        if take: take.PlotTakeOnSelected(lPeriod)
                
        # switch off the constraints            
        for lConstraint in self.constraints:
            lConstraint.Active = False
            
            objA = lConstraint.ReferenceGet(0, 0)
            objB = lConstraint.ReferenceGet(1, 0)
            
            lConstraint.ReferenceRemove(0, objA)
            lConstraint.ReferenceRemove(1, objB)
                    
            lConstraint.ReferenceAdd(0, objB)
            lConstraint.ReferenceAdd(1, objA)
        
            lConstraint.Active = True
        
        # restore selection
        for newObj in self.nulls:
            newObj.Selected = False
        for obj in modelList:
            obj.Selected = True
    
    def SaveToFile(self, filename):
        
        # we should store temp null models and constraints
        
        modelList = FBModelList()
        FBGetSelectedModels(modelList)
        
        for model in modelList:
            model.Selected = False
            
        for newObj in self.nulls:
            newObj.Selected = True
        for lConstraint in self.constraints:
            lConstraint.Selected = True
        
        options = FBFbxOptions(False)
        options.SaveSelectedModelsOnly = True
        
        self.mApp.FileSave(filename, options)
        
    def LoadFromFile(self, filename):
        
        options = FBFbxOptions(True)
        self.mApp.FileMerge(filename, False, options)
    
        self.nulls[:] = []
        self.constraints[:] = []
    
        for lSourceName in self.source:
            
            tempName = lSourceName + '_Null'
            objA = FBFindModelByLabelName(tempName)

            self.nulls.append(objA)
            
        for lConstraint in self.mSystem.Scene.Constraints:
            
            for i in range(0, lConstraint.GetSrcCount() ):
                src = lConstraint.GetSrc(i)
                
                for null in self.nulls:
                    if null == src:
                        
                        self.constraints.append(lConstraint)
                        break
        
        print(self.constraints)
        
    def Fetch(self):
        print("fetch")
        print(self.constraints)
        if len(self.constraints) == 0:
            return
    
        # activate constraints

        for lConstraint, lSourceName in zip(self.constraints, self.source):
            lConstraint.Active = True
            
            objA = lConstraint.ReferenceGet(0,0)
            if objA == None:
                objA = FBFindModelByLabelName(lSourceName)

                if objA !=None:
                    lConstraint.ReferenceAdd(0,objA)
                    objA.Selected = True
                else:
                    lConstraint.Active = False
                    
            else: 
                objA.Selected = True
                    
        self.mSystem.Scene.Evaluate()
        
        # plot animations
        take = self.mSystem.CurrentTake
        lPeriod = FBTime(0,0,0,1)
        if take: 
            take.PlotTakeOnSelected(lPeriod)

        self.mSystem.Scene.Evaluate()
        
        # remove unneeded objects and constraints
        for lConstraint in self.constraints:
            lConstraint.Lock = False
            lConstraint.Active = False
            
            objB = lConstraint.ReferenceGet(1,0)
            
            if objB != None:
                objB.FBDelete()
                
            lConstraint.FBDelete()
        