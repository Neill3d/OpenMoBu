
'''
    libAdditiveAnimation

    Sergey Solokhin (Neill3d) 2017
        s@neill3d.com   
        www.neill3d.com
'''

from pyfbsdk import *

class CAdditiveAnimation():

    mVersion = 101

    mSystem = FBSystem()
    mApp = FBApplication()
    mPlayer = FBPlayerControl()
    
    mModels = []
    #mTargetTM = []
    
    #
    mAnimTakeIndex = 0
    mBaseTakeIndex = 1
    mBaseFromFirstFrame = True
    mOutputTakeIndex = 2
    
    #
    mComputedAdditiveTakeName = "Take With Computed Additive"
    mAddtiveLayerName = "Additive Layer"
    mExtractedAdditiveTakeName = "Extracted Additive Animation"
    
    def __init__(self):

        pass

    ######################################################
    # Options
    
    def GetVersion(self):
        return self.mVersion
    
    def SetAnimationTakeIndex(self, index):
        self.mAnimTakeIndex = index

    def SetBaseTakeIndex(self, index):
        self.mBaseTakeIndex = index
        
    def SetBaseFromAnimationFirstFrame(self, value):
        self.mBaseFromFirstFrame = value

    ###########################################################################
    #
    
    def ClearLists(self):
        self.mModels[:] = []
        #self.mTargetTM[:] = []
    
    def appendModelList( self, pModel ):
        
        self.mModels.append(pModel)
        #self.mTargetTM.append( FBMatrix() )
        
        for child in pModel.Children:
            self.appendModelList( child )
    
    
    def GrabTM(self):
    
        newList = []
    
        for model in self.mModels:
            m = FBMatrix()
            model.GetMatrix(m, FBModelTransformationType.kModelTransformation, True)
            
            newList.append(m)
            
        return newList
    
    def SetResult(self, tmList):
    
        pTime = FBTime()
        pTime = self.mSystem.LocalTime
        
        for model, theMatrix in zip(self.mModels, tmList):
     
            model.SetMatrix(theMatrix, FBModelTransformationType.kModelTransformation, True)
            '''
            m = FBMatrix()
            model.GetMatrix(m, FBModelTransformationType.kModelTransformation, False)
                        
            t = FBVector4d()
            r = FBVector3d()
            s = FBSVector()
        
            FBMatrixToTRS(t, r, s, m)
            '''
            # rotation part
            lAnimProp = model.Rotation
            lRotNode = lAnimProp.GetAnimationNode()
                    
            if lRotNode is not None:
                lRotNode.KeyCandidate()
                #for i in range(3):
                #    lRotNode.Nodes[i].KeyAdd( pTime, r[i] )
        
            # translation part
            lAnimProp = model.Translation
            lMoveNode = lAnimProp.GetAnimationNode()
        
            if lMoveNode is not None:
                lMoveNode.KeyCandidate()
                #for i in range(3):
                #    lMoveNode.Nodes[i].KeyAdd( pTime, t[i] )
    
    def EvalOneFrame(self, lTime):
        
        self.mPlayer.Goto(lTime)
        
        # get target pose
        currTake = self.mSystem.Scene.Takes[self.mAnimTakeIndex]
        self.mSystem.CurrentTake = currTake
        currTake.SetCurrentLayer(0)
        self.mSystem.Scene.Evaluate()
    
        goalTransforms = []
        goalTransforms = self.GrabTM()
        
        # go back to additive take and additive layer
        currTake = self.mSystem.Scene.Takes[self.mOutputTakeIndex]
        self.mSystem.CurrentTake = currTake
        currTake.SetCurrentLayer(1)
        self.mSystem.Scene.Evaluate()                
        
        self.SetResult(goalTransforms)
        
        # our models are already selected 
        self.mSystem.Scene.Evaluate()
        self.mPlayer.Key()
        
    def CheckTakeIndex(self):
        
        count = len(self.mSystem.Scene.Takes)
        
        if self.mAnimTakeIndex < 0:
            self.mAnimTakeIndex = 0
        if self.mBaseTakeIndex < 0:
            self.mBaseTakeIndex = 0
        
        if self.mAnimTakeIndex >= count:
            self.mAnimTakeIndex = count-1
        if self.mBaseTakeIndex >= count:
            self.mBaseTakeIndex = count-1
        
        
    def ComputeCharacterAdditive(self):
        
        lCharacter = self.mApp.CurrentCharacter
        
        if lCharacter is None:
            return False
            
        startIndex = FBBodyNodeId.kFBHipsNodeId
        rootModel = lCharacter.GetModel(FBBodyNodeId(startIndex))
    
        # allocate storage for character models
        self.ClearLists()
        self.appendModelList(rootModel)
    
        # select our models to key them each frame  
        modelList = FBModelList()
        FBGetSelectedModels(modelList)
        for model in modelList:
            model.Selected = False
        
        for model in self.mModels:
            model.Selected = True
        
        #
        # prepare output take
        
        self.CheckTakeIndex()
        
        animTake = self.mSystem.Scene.Takes[self.mAnimTakeIndex] 
        poseTake = self.mSystem.Scene.Takes[self.mBaseTakeIndex]
        
        print "prepare output take"
        
        outputTake = None
        
        for i, take in enumerate(self.mSystem.Scene.Takes):
            if take.Name == self.mComputedAdditiveTakeName:
                userId = FBMessageBox("Compute Additive",
                    "Compute Additive Take already exist.",
                    "Override", "Create A New" )
    
                if 1 == userId:
                    take.FBDelete()
                    
                break
        
        if outputTake is None:
            if True == self.mBaseFromFirstFrame:
                outputTake = animTake.CopyTake(self.mComputedAdditiveTakeName)
            else:
                outputTake = poseTake.CopyTake(self.mComputedAdditiveTakeName)
                
            self.mOutputTakeIndex = len(self.mSystem.Scene.Takes)-1
            
        outputTake.SetCurrentLayer(0)
        if True == self.mBaseFromFirstFrame:
            outputTake.ClearAllProperties(False)
        
        for i in range(outputTake.GetLayerCount()-1, 0, -1):
            outputTake.RemoveLayer(i)
        
        outputTake.CreateNewLayer()
        lLayer = outputTake.GetLayer(1)
        lLayer.Name = self.mAddtiveLayerName
        outputTake.SetCurrentLayer(1)
        
        lTime = FBTime()
        lStartFrame = self.mPlayer.LoopStart.GetFrame()
        lStopFrame = self.mPlayer.LoopStop.GetFrame()
    
        lProgress = FBProgress()
        lProgress.ProgressBegin()
        lProgress.Caption = 'Computing an additive layer'
        lProgress.Percent = 0
        
        self.mApp.UpdateAllWidgets()
        self.mApp.FlushEventQueue()
        
        for lFrame in range(lStartFrame, lStopFrame+1):
    
            print lFrame
    
            lTime.SetTime( 0,0,0, lFrame)
            self.EvalOneFrame(lTime)
    
            if True == lProgress.UserRequestCancell():
                break
            
            p = 100.0 * lFrame / (lStopFrame-lStartFrame)
            lProgress.Percent = int(p)
            
        lProgress.ProgressDone()
        '''
        # deselect models
        FBGetSelectedModels(modelList)
        for model in modelList:
            model.Selected = False
        '''
        
        return True
        
    def ExtractAdditiveLayerIntoANewTake(self):
    
    
        srcTake = self.mSystem.CurrentTake
        if srcTake.GetLayerCount() < 2:
            return False
    
        lCharacter = self.mApp.CurrentCharacter    
        if lCharacter is None:
            return False
    
        for take in self.mSystem.Scene.Takes:
            if take.Name == self.mExtractedAdditiveTakeName:
                userId = FBMessageBox("Extract Layer into A New Take",
                    "Extracted Additive Take already exist.",
                    "Override", "Create A New" )
    
                if 1 == userId:
                    take.FBDelete()
                    
                break
                
        newTake = srcTake.CopyTake(self.mExtractedAdditiveTakeName)
        newTake.SetCurrentLayer(0)
        newTake.ClearAllPropertiesOnCurrentLayer()
    
        startIndex = FBBodyNodeId.kFBHipsNodeId
        rootModel = lCharacter.GetModel(FBBodyNodeId(startIndex))
    
        # allocate storage for character models
        self.ClearLists()
        self.appendModelList(rootModel)
    
        #
        self.mPlayer.GotoStart()
        
        lTime = FBTime()
        lTime = self.mSystem.LocalTime
        lTime = lTime - FBTime(0,0,0,1)
        t = FBVector3d(0.0, 0.0, 0.0)
        r = FBVector3d(0.0, 0.0, 0.0)
        
        selModels = FBModelList()
        FBGetSelectedModels(selModels)
        for model in selModels:
            model.Selected = False
        
        # put a zero key
        for model in self.mModels:
            
            # rotation part
            lAnimProp = model.Rotation
            lRotNode = lAnimProp.GetAnimationNode()
                    
            if lRotNode is not None:
                for i in range(3):
                    lRotNode.Nodes[i].KeyAdd( lTime, r[i] )
        
            # translation part
            lAnimProp = model.Translation
            lMoveNode = lAnimProp.GetAnimationNode()
        
            if lMoveNode is not None:
                for i in range(3):
                    lMoveNode.Nodes[i].KeyAdd( lTime, t[i] )
                    
            model.Selected = True
        
        stepTime = FBTime(0,0,0,1)
        newTake.PlotTakeOnSelected(stepTime)
        
        #newTake.MergeLayers(FBAnimationLayerMergeOptions.kFBAnimLayerMerge_AllLayers_CompleteScene, True, FBMergeLayerMode.kFBMergeLayerModeAutomatic )

        return True
        