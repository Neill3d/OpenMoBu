
# Script for a global scene retiming
#   by using exported xml media data form Sony Vegas montage
#
# Sergei <Neill3d> Solokhin 2018

import os
import math

from pyfbsdk import *
from xml.dom import minidom


#
gFrameRate = 24.0
gRenderFirstFrame = 0   # sync between media frames and rendered scene frames (IN)

gKeepPreFrames = 100
gKeepPostFrames = 100
gPlotCurves = True
gRenderInfoRoot = 'U:/UGA_Suits/XML/' # Dir from where search starts can be replaced with any path

gSystem = FBSystem()
gScene = gSystem.Scene
gKeyControl = FBKeyControl()
gPlayer = FBPlayerControl()
gApp = FBApplication()

# return frameRate, Active Take Name or media path name (Key)
def ImportHeader(xmlname):

    xmldoc = minidom.parse(xmlname)
    selAllElem = xmldoc.getElementsByTagName("Selection")
    
    fps = 1.0
    key = ''
    mediaPath = ''
    
    if len(selAllElem) > 0:
        selElem = selAllElem[0]
        frameRateStr = str(selElem.getAttribute('FrameRate'))
        frameRateStr = frameRateStr.replace(',', '.')
        fps = float(frameRateStr)
    
    allTakesElem = xmldoc.getElementsByTagName('ActiveTake')
    if len(allTakesElem) > 0:
        nameStr = str(allTakesElem[0].getAttribute('Name'))
        pathStr = str(allTakesElem[0].getAttribute('MediaPath'))
        
        if len(nameStr) > 0:
            key = nameStr
        else:
            key = os.path.basename(pathStr)
    
        mediaPath = pathStr
    
    return [fps, key, mediaPath]
    
#
def ImportInfo(xmlname):
    
    clipsInfo = []
    
    xmldoc = minidom.parse(xmlname)
    
    trackEventElem = xmldoc.getElementsByTagName('TrackEvent')
    
    if len(trackEventElem) > 0:
        for trackEvent in trackEventElem:
        
            strStart = str(trackEvent.getAttribute('Start'))
            strEnd = str(trackEvent.getAttribute('End'))
            strRate = str(trackEvent.getAttribute('PlaybackRate'))
        
            # convert to FBTime
            
            strStart = strStart.replace(',', ':')
            strEnd = strEnd.replace(',', ':')
            
            timeStartArr = strStart.split(':')
            timeEndArr = strEnd.split(':')
            
            timeStart = FBTime(int(timeStartArr[0]), int(timeStartArr[1]), int(timeStartArr[2]), int(timeStartArr[3]))
            timeEnd = FBTime(int(timeEndArr[0]), int(timeEndArr[1]), int(timeEndArr[2]), int(timeEndArr[3]))
            
            #print timeStart.GetFrame()
            #print timeEnd.GetFrame()
            
            # convert rate
            strRate = strRate.replace(',', '.')
            rate = float(strRate)
        
            # take a media offset
            mediaOffset = FBTime()
            
            allTakesElem = trackEvent.getElementsByTagName('ActiveTake')
            if len(allTakesElem) > 0:
                nameStr = str(allTakesElem[0].getAttribute('Name'))
                offsetStr = str(allTakesElem[0].getAttribute('Offset'))

                offsetStr = offsetStr.replace(',', ':')
                offsetArr = offsetStr.split(':')
                
                mediaOffset = FBTime(int(offsetArr[0]), int(offsetArr[1]), int(offsetArr[2]), int(offsetArr[3]))
                print mediaOffset.GetFrame()
            
            clipsInfo.append( [timeStart, timeEnd, rate, mediaOffset] )
            
    return clipsInfo

# import render info
def ImportRenderInfo(xmlname):
    
    xmldoc = minidom.parse(xmlname)

    inValue = 0
    outValue = 0
    ScenePath = ''
    MediaPath = ''
    CameraName = ''

    inElem = xmldoc.getElementsByTagName('IN')
    if len(inElem) > 0:
        inValue = int(str(inElem[0].firstChild.data))
        
    scenePathElem = xmldoc.getElementsByTagName('ScenePath')
    if len(scenePathElem) > 0:
        ScenePath = str(scenePathElem[0].firstChild.data)
        ScenePath = os.path.normpath(ScenePath)
        
    mediaPathElem = xmldoc.getElementsByTagName('VideoPath')
    if len(mediaPathElem) > 0:
        MediaPath = str(mediaPathElem[0].firstChild.data)
        MediaPath = os.path.normpath(MediaPath)

    mediaPathElem = xmldoc.getElementsByTagName('CameraName')
    if len(mediaPathElem) > 0:
        CameraName = str(mediaPathElem[0].firstChild.data)
        
    return [inValue, ScenePath, MediaPath, CameraName]

# scale keyframes

def scaleFCurve(fcurve, clipsInfo, offsetTime):
    
    if 0 == len(fcurve.Keys):
        return 0
    
    premode = fcurve.GetPreExtrapolationMode()
    postmode = fcurve.GetPostExtrapolationMode()
    
    #
    # replotting when use extrapolation
        
    useCurvePlotting = (FBExtrapolationMode.kFCurveExtrapolationConst != premode or FBExtrapolationMode.kFCurveExtrapolationConst != postmode)
    
    if True == gPlotCurves:
        useCurvePlotting = True
    
    tempCurve = FBFCurve()
    tempCurve.KeyReplaceBy(fcurve)
    
    fcurve.EditBegin() #len(newCurve.Keys))
    fcurve.EditClear()
    
    firstTime = FBTime.Infinity
    clipTimeShift = FBTime.Zero
    
    halfFrameTime = FBTime(0,0,0,1)
    secs = halfFrameTime.GetSecondDouble()
    secs *= 0.1
    halfFrameTime.SetSecondDouble(secs)
    
    preTime = FBTime(0,0,0, gKeepPreFrames)
    postTime = FBTime(0,0,0, gKeepPostFrames)
    
    for clipNdx, clipInfo in enumerate(clipsInfo):

        clipStart = FBTime(clipInfo[0]) # FBTime
        clipStop = FBTime(clipInfo[1])  # FBTime
        clipRate = float(clipInfo[2]) # double
        sourceStart = FBTime(clipInfo[3]) # FBTime

        if FBTime.Infinity == firstTime:
            firstTime = FBTime(clipStart)

        sourceStartInSecs = sourceStart.GetSecondDouble()        
        sourceStartInSecs *= clipRate
        sourceStart.SetSecondDouble(sourceStartInSecs)
        sourceStart = sourceStart + offsetTime
                
        sourceLen = (clipStop.GetSecondDouble() - clipStart.GetSecondDouble()) * clipRate
        sourceStop = FBTime()
        sourceStop.SetSecondDouble(sourceLen)
        sourceStop = sourceStart + sourceStop

        isKeyOnStart = False
        isKeyOnStop = False
        
        clipStartInSecs = clipStart.GetSecondDouble()
        sourceStartInSecs = sourceStart.GetSecondDouble()

        #
        if 0 == clipNdx:
            sourceStart = sourceStart - preTime
        elif len(clipsInfo) == (clipNdx+1):
            sourceStop = sourceStop + postTime
            
        #
        
        if True == useCurvePlotting:
            #print "TODO: FRAME BY FRAME REPLOTTING"
        
            lTime = FBTime(sourceStart)
            lTimeStep = FBTime(0,0,0,1)
            scaledTime = FBTime()
            
            while lTime.GetSecondDouble() <= sourceStop.GetSecondDouble():
            
                value = tempCurve.Evaluate(lTime)
                
                secs = lTime.GetSecondDouble()
                secs -= sourceStartInSecs
                secs /= clipRate
                secs += clipStartInSecs
                # DONE: this is not correct !
                scaledTime.SetSecondDouble(secs)
                newTime = scaledTime - firstTime + offsetTime
                ndx = fcurve.KeyAdd(newTime, value)
                #print ndx
                fcurve.Keys[ndx].TangentMode = FBTangentMode.kFBTangentModeClampProgressive
        
                lTime = lTime + lTimeStep
        
        else:
            #
            # key retiming
        
            for key in tempCurve.Keys:
                
                time = FBTime(key.Time)
                #print time.GetFrame()
                if time == sourceStart:
                    isKeyOnStart = True
                elif time == sourceStop:
                    isKeyOnStop = True
            
                if time.GetSecondDouble() > sourceStop.GetSecondDouble():
                    break
                
                elif time.GetSecondDouble() >= sourceStart.GetSecondDouble():
                    
                    if False == isKeyOnStart:
                        value = tempCurve.Evaluate(sourceStart)
                        newTime = clipStart - firstTime + offsetTime + clipTimeShift
                        
                        if FBTime.Zero != clipTimeShift:
                            lastKey = fcurve.Keys[len(fcurve.Keys)-1]
                            ndx1 = fcurve.KeyAdd(lastKey.Time + halfFrameTime, lastKey.Value)
                            ndx2 = fcurve.KeyAdd(newTime - halfFrameTime, value)
    
                            fcurve.Keys[ndx1].TangentMode = FBTangentMode.kFBTangentModeClampProgressive
                            fcurve.Keys[ndx2].TangentMode = FBTangentMode.kFBTangentModeClampProgressive
                                                    
                        fcurve.KeyAdd(newTime, value)
                    
                    value = key.Value
                    scaledTime = FBTime()
                    
                    secs = time.GetSecondDouble()
                    secs -= sourceStartInSecs
                    secs /= clipRate
                    secs += clipStartInSecs
                    # DONE: this is not correct !
                    scaledTime.SetSecondDouble(secs)
                    newTime = scaledTime - firstTime + offsetTime
                    ndx = fcurve.KeyAdd(newTime, value)
                    #print ndx
                    fcurve.Keys[ndx].TangentMode = key.TangentMode
                    fcurve.Keys[ndx].TangentClampMode = key.TangentClampMode
                    fcurve.Keys[ndx].Interpolation = key.Interpolation
            
            if False == isKeyOnStart:
                value = tempCurve.Evaluate(sourceStart)
                newTime = clipStart - firstTime + offsetTime + clipTimeShift
                
                if FBTime.Zero != clipTimeShift:
                    lastKey = fcurve.Keys[len(fcurve.Keys)-1]
                    ndx1 = fcurve.KeyAdd(lastKey.Time + halfFrameTime, lastKey.Value)
                    ndx2 = fcurve.KeyAdd(newTime - halfFrameTime, value)
    
                    fcurve.Keys[ndx1].TangentMode = FBTangentMode.kFBTangentModeClampProgressive
                    fcurve.Keys[ndx2].TangentMode = FBTangentMode.kFBTangentModeClampProgressive                
                   
                fcurve.KeyAdd(newTime, value)
            
            if False == isKeyOnStop:
                value = tempCurve.Evaluate(sourceStop)
                newTime = clipStop - firstTime + offsetTime
                ndx = fcurve.KeyAdd(newTime, value)
                
            #      
        
        clipTimeShift = FBTime(0,0,0,1)
        
    fcurve.EditEnd()
    
    del(tempCurve)
    
    return 1
    

def scaleAnimNode(animNode, clipsInfo, offsetTime):
    
    if animNode is None:
        return
    
    # skip all connected nodes
    #srcCount = animNode.GetSrcCount()
    #if srcCount > 0:
    #    return
    
    for node in animNode.Nodes:
        scaleAnimNode(node, clipsInfo, offsetTime)
        
    #
    
    fcurve = animNode.FCurve
    if fcurve is not None and len(fcurve.Keys) > 0:
        
        scaleFCurve(fcurve, clipsInfo, offsetTime)
        
def timeWrapVideoClip(videoClip, clipsInfo, offsetTime):
    
    startFrame = int(videoClip.StartFrame)
    stopFrame = int(videoClip.StopFrame)
    
    if stopFrame > 0 and startFrame != stopFrame:
        
        wrapProp = videoClip.PropertyList.Find('VideoTimeWrap')
        if wrapProp is None:
            wrapProp = videoClip.PropertyCreate('VideoTimeWrap', FBPropertyType.kFBPT_double, 'Double', True, True, None )
        
        lastFrameProp = videoClip.PropertyList.Find('VideoTimeWrapLastUpdate')
        if lastFrameProp is None:
            lastFrameProp = videoClip.PropertyCreate('VideoTimeWrapLastUpdate', FBPropertyType.kFBPT_int, 'int', False, True, None )
        
        stepProp = videoClip.PropertyList.Find('VideoTimeWrapStep')
        if stepProp is None:
            stepProp = videoClip.PropertyCreate('VideoTimeWrapStep', FBPropertyType.kFBPT_int, 'int', False, True, None )
            stepProp.Data = 7  
        
        
        if wrapProp is not None:
            wrapProp.SetAnimated(False)
            wrapProp.SetAnimated(True)
            
            animNode = wrapProp.GetAnimationNode()
            
            # TODO: build time wrap curve
            timeOffset = FBTime(videoClip.TimeOffset)
            playSpeed = float(videoClip.PlaySpeed)
            
            stopTime = FBTime(0,0,0, stopFrame)
            
            animNode.KeyAdd(timeOffset, float(startFrame))
            animNode.KeyAdd(timeOffset + stopTime, float(stopFrame))
            
            scaleFCurve(animNode.FCurve, clipsInfo, offsetTime)
            
            videoClip.PlaySpeed = 0.0

def scaleVideoClip(videoClip, clipsInfo, offsetTime):
    
    if len(clipsInfo) > 0:
        clipInfo = clipsInfo[0]
    
        clipStart = FBTime(clipInfo[0]) # FBTime
        clipStop = FBTime(clipInfo[1])  # FBTime
        clipRate = float(clipInfo[2]) # double
        sourceStart = FBTime(clipInfo[3]) # FBTime
        sourceStart = sourceStart + offsetTime

        timeOffset = FBTime(videoClip.TimeOffset)
        playSpeed = float(videoClip.PlaySpeed)
        
        timeOffset = timeOffset - clipInfo[3]
        playSpeed *= clipRate
        
        videoClip.TimeOffset = timeOffset
        videoClip.playSpeed = clipRate

def replaceAsStoryTrack(videoClip):

    # replace only if videoClip is not a story track and if in use in scene

    isInUse = False
    
    dstCount = videoClip.GetDstCount()
    for i in range(dstCount):
        dst = videoClip.GetDst(i)
        
        if isinstance(dst, FBTexture):
            
            texDstCount = dst.GetDstCount()

            for j in range(texDstCount):
                texDst = dst.GetDst(j)

                if isinstance(texDst, FBProperty):
                    owner = texDst.GetOwner()
                    if isinstance(owner, FBMaterial):
                        isInUse = True
                        break
                        
                elif isinstance(texDst, FBMaterial):
                    isInUse = True
                    break
                    
            if True == isInUse:
                break
            
    startFrame = int(videoClip.StartFrame)
    stopFrame = int(videoClip.StopFrame)
    
    if True == isInUse and stopFrame > 0 and startFrame != stopFrame:

        offsetTime = FBTime(videoClip.TimeOffset)

        newTrack = FBStoryTrack(FBStoryTrackType.kFBStoryTrackVideo)
        newClip = FBStoryClip(videoClip.Filename, newTrack, offsetTime)
        
        newClip.Speed = videoClip.PlaySpeed
        
        # replace connections
        conn = []
        dstCount = videoClip.GetDstCount()
        for i in range(dstCount):
            dst = videoClip.GetDst(i)
            if isinstance(dst, FBTexture):
                conn.append(dst)
        
        #gScene.CandidateEvaluationAndResolve()
        #gScene.Evaluate()
        '''
        for theConn in conn:
            
            print 'connection should be replaced with a story track output'
            #FBDisconnect(videoClip, theConn)

            # TODO: crash here!
            #trackVideo = newTrack.TrackVideo
            #if trackVideo is not None and isinstance(trackVideo, FBVideoClip):
            #    FBConnect(trackVideo, theConn)
            #if newTrack.TrackVideo is not None:
            #FBConnect(newTrack.TrackVideo, theConn)
        '''
        
def scaleAudioClip(audioClip, clipsInfo, offsetTime):
    pass

def scaleKeyframes(clipsInfo, offsetTime):
    
    numberOfLayer = gSystem.CurrentTake.GetLayerCount()
    
    for idx in range(numberOfLayer):
        print "Moving keys for layer : " + gSystem.CurrentTake.GetLayer(idx).Name
        gSystem.CurrentTake.GetLayerByName(gSystem.CurrentTake.GetLayer(idx).Name).SelectLayer(True, True)
        gSystem.CurrentTake.SetCurrentLayer(idx)
        gScene.Evaluate()
    
        for comp in gScene.Components:
        
            if isinstance(comp, FBModel):
                #if comp.LongName == 'Brutiful:Brutiful_Layout_Ctrl:Spine3':
                
                animNode = comp.AnimationNode        
                scaleAnimNode(animNode, clipsInfo, offsetTime)
                #print "empty"
            
            elif False == comp.HasObjectFlags(FBObjectFlag.kFBFlagSystem) and False == comp.HasObjectFlags(FBObjectFlag.kFBFlagStory):
                
                for prop in comp.PropertyList:
                    if prop is not None and prop.IsAnimatable() and prop.IsAnimated():
                        scaleAnimNode(prop.GetAnimationNode(), clipsInfo, offsetTime)
            
            
    # Back to Base Animation Layer       
    gSystem.CurrentTake.GetLayerByName(gSystem.CurrentTake.GetLayer(0).Name).SelectLayer(True, True)
    gSystem.CurrentTake.SetCurrentLayer(0)
    
    # TODO: audio and video clips, process in story
    
    #for videoClip in gScene.VideoClips:
    #    replaceAsStoryTrack(videoClip, clipsInfo, offsetTime)
     
    # TODO: replace as story audio track
       
    #for audioClip in gScene.AudioClips:
    #    scaleAudioClip(audioClip, clipsInfo, offsetTime)    

def scaleStoryTrack(track, clipsInfo, offsetTime):
    
    clips = []
    
    for trackClip in track.Clips:
        clips.append(trackClip)
    
    if 0 == len(clips):
        return 0
    
    newTrack = FBStoryTrack(track.Type, track.ParentFolder)
    charIndex = track.CharacterIndex
    
    if charIndex >= 0:    
        track.CharacterIndex = -1
        newTrack.CharacterIndex = charIndex

    firstTime = FBTime.Infinity
    
    for clipInfo in clipsInfo:

        clipStart = FBTime(clipInfo[0]) # FBTime
        clipStop = FBTime(clipInfo[1])  # FBTime
        clipRate = float(clipInfo[2]) # double
        sourceStart = FBTime(clipInfo[3]) # FBTime

        if FBTime.Infinity == firstTime:
            firstTime = FBTime(clipStart)

        sourceStartInSecs = sourceStart.GetSecondDouble()        
        sourceStartInSecs *= clipRate
        sourceStart.SetSecondDouble(sourceStartInSecs)
        sourceStart = sourceStart + offsetTime
                
        sourceLen = (clipStop.GetSecondDouble() - clipStart.GetSecondDouble()) * clipRate
        sourceStop = FBTime()
        sourceStop.SetSecondDouble(sourceLen)
        sourceStop = sourceStart + sourceStop

        isKeyOnStart = False
        isKeyOnStop = False
        
        clipStartInSecs = clipStart.GetSecondDouble()
        sourceStartInSecs = sourceStart.GetSecondDouble()
        
        #    
        for clip in clips:
            #print clip.Name
        
            storyClipStart = FBTime(clip.Start)
            storyClipStop = FBTime(clip.Stop)

            storyClipMarkIn = FBTime(clip.MarkIn)
            storyClipMarkOut = FBTime(clip.MarkOut)

        
            if storyClipStart < sourceStop and storyClipStop > sourceStart:
                
                newClip = clip.Clone()
                newClip.LockPitchToSpeed = False
                
                newTrack.AddClip(newClip, clipStart)
                
                newSpeed = clip.Speed * clipRate

                newSourceStart = FBTime(clip.MarkIn)
                newSourceStop = FBTime(clip.MarkOut)

                newClipStart = clipStart - firstTime + offsetTime
                newClipStop = clipStop - firstTime + offsetTime

                # convert ot clip local time
                f = (sourceStart.GetSecondDouble() - storyClipStart.GetSecondDouble()) / (storyClipStop.GetSecondDouble() - storyClipStart.GetSecondDouble())
                newSourceStart.SetSecondDouble( storyClipMarkIn.GetSecondDouble() + f * (storyClipMarkOut.GetSecondDouble() - storyClipMarkIn.GetSecondDouble()) )
                
                newClip.SetTime(newSourceStart, newSourceStop, newClipStart, newClipStop, True)
                newClip.Speed = newSpeed
                
                newClip.PropertyList.Find('ExtractStart').Data = newSourceStart
                newClip.PropertyList.Find('ExtractStop').Data = newSourceStop
                
                newClip.PropertyList.Find('FirstLoopMarkIn').Data = newSourceStart
                newClip.PropertyList.Find('LastLoopMarkOut').Data= newSourceStop
                
                '''
                newClip.Stop = clipStop
                newClip.Start = clipStart
                
                newClip.MarkOut = newSourceStop
                newClip.MarkIn = newSourceStart
                
                print newSourceStop.GetSecondDouble()
                print newSourceStart.GetSecondDouble()

                newClip.MarkIn = newSourceStart
                newClip.MarkOut = newSourceStop
                '''

    newCount = len(newTrack.Clips)
    if 0 == newCount:
        newTrack.FBDelete()

    return newCount

def scaleStory(clipsInfo, offsetTime):
    
    # collect tracks
    
    tracks = []
    #filters = [FBStoryTrackType.kFBStoryTrackAnimation, FBStoryTrackType.kFBStoryTrackCharacter]
    for comp in gScene.Components:
        if isinstance(comp, FBStoryTrack):
            
            #if comp.Type in filters:
            tracks.append(comp)
    
    # scale track clips    

    for track in tracks:
        scaleStoryTrack(track, clipsInfo, offsetTime)
        track.FBDelete()

#
def setTimeRangeFromCuts(info, offsetTime):
    
    loopStart = FBTime.Infinity
    loopStop = FBTime.Infinity
    firstTime = FBTime.Infinity
    
    for clipInfo in info:
        
        clipStart = FBTime(clipInfo[0]) # FBTime
        clipStop = FBTime(clipInfo[1])  # FBTime
        clipRate = float(clipInfo[2]) # double
        sourceStart = FBTime(clipInfo[3]) # FBTime
        sourceStart = sourceStart + offsetTime
    
        if FBTime.Infinity == firstTime:
            firstTime = clipStart
        if FBTime.Infinity == loopStart:
            loopStart = FBTime(offsetTime)
            sourceStartInSecs = sourceStart.GetSecondDouble()        
        sourceStartInSecs *= clipRate
        sourceStart.SetSecondDouble(sourceStartInSecs)
    
        sourceLen = (clipStop.GetSecondDouble() - clipStart.GetSecondDouble()) * clipRate
        sourceStop = FBTime()
        sourceStop.SetSecondDouble(sourceLen)
        sourceStop = sourceStart + sourceStop
    
        sourceStop = clipStop - firstTime + offsetTime
    
        if FBTime.Infinity == loopStop:
            loopStop = sourceStop
        elif loopStop.GetSecondDouble() < sourceStop.GetSecondDouble():
            loopStop = sourceStop
        
    
    if FBTime.Infinity != loopStart:
        gPlayer.LoopStart = loopStart
    if FBTime.Infinity != loopStop:
        gPlayer.LoopStop = loopStop
#
lDialog = FBFilePopup()
lDialog.Style = FBFilePopupStyle.kFBFilePopupOpen
lDialog.Filter = '*.xml'

if lDialog.Execute():
    
    header = ImportHeader(lDialog.FullFilename)
    print header
    
    key = header[1]
    
    #
    # search for xml description and fbx scene file
    file_name = key + '.xml'
    file_name = file_name.lower()
    cur_dir = gRenderInfoRoot # Dir from where search starts can be replaced with any path
    isFileFound = False
        
    for root, dirs, files in os.walk(cur_dir):
        for file in files:
            if file.endswith(".xml"):
                if file.lower() == file_name:
                    isFileFound = True
                    file_name = os.path.join(root,file)
                    break

    offsetTime = FBTime.Zero
    renderInfoFbx = ''
    renderInfoCamera = ''
    
    if False == isFileFound:
        FBMessageBox('Import Info', 'Media Clip xml description on disk U is not found!', 'Ok')  
    else:
        
        renderInfo = ImportRenderInfo(file_name)
        print renderInfo
        
        offsetTime = FBTime(0,0,0, renderInfo[0])
        renderInfoFbx = renderInfo[1]
        renderInfoCamera = renderInfo[3]
    
    lDoWeProceed = False
    
    if 0 == len(renderInfoFbx):
        lOption = FBMessageBox('Import Info', 'Render Info Fbx Is empty, Do you want to continue with current scene ?', 'Ok', 'Cancel')  
        lDoWeProceed = (1 == lOption)
        
    else:
        
        # DONE: open media correspondent fbx
        currFbx = gApp.FBXFileName
        currFbx = os.path.normpath(currFbx)
        
        if currFbx != renderInfo[1]:
            lOption = FBMessageBox('Import Info', renderInfo[1] + ' is not opened. \nDo you want to open it now ?', 'Open', 'Use Current', 'Cancel')
        
            if 1 == lOption:
                gApp.FileOpen(renderInfo[1])

            lDoWeProceed = (lOption < 3)
        
    if True == lDoWeProceed:
        
        sceneFrameRate = gPlayer.GetTransportFpsValue()
        frameRate = float(header[0])
        
        isSameFps = (abs(frameRate - sceneFrameRate) < 0.01)
        
        if False == isSameFps:
            FBMessageBox('Import Info', 'Scene Frame Rate is not equal to the Info Frame Rate', 'Ok')

        if len(renderInfoCamera) > 0:
            lRenderer = gSystem.Renderer
            selPaneId = lRenderer.GetSelectedPaneIndex()
            currentCamera = lRenderer.GetCameraInPane(selPaneId)
            if currentCamera is not None:
                currentCameraName = currentCamera.LongName
                
                if currentCameraName != renderInfoCamera:
                    lOption = FBMessageBox('Import Info', 'Render Info has a different Camera Name\n Do you want to switch to that camera?', 'Yes', 'Ignore')
                    if 1 == lOption:
                        renderInfoCam = FBFindModelByLabelName(renderInfoCamera)
                        
                        if renderInfoCam is not None:
                            lRenderer.SetCameraInPane(renderInfoCam, selPaneId)
        

        info = ImportInfo(lDialog.FullFilename)
        print info
        
        gApp.FlushEventQueue()
        
        lProgress = FBProgress()
        lProgress.Caption = 'Process Scene'
        lProgress.Text = 'Keyframes'
        lProgress.Percent = 10
        lProgress.ProgressBegin()

        gApp.FlushEventQueue()
        
        #
        # MAIN PROCESSING START POINT
        
        scaleKeyframes(info, offsetTime)
        
        lProgress.Text = 'Replace Seq As Story Tracks'
        lProgress.Percent = 50

        gApp.FlushEventQueue()
        
        #for videoClip in gScene.VideoClips:
        #    replaceAsStoryTrack(videoClip)
        
        lProgress.Text = 'Story'
        lProgress.Percent = 70
        
        gApp.FlushEventQueue()
        
        #scaleStory(info, offsetTime)
        
        # set time range
        
        lProgress.Text = 'SetUp Time Range'
        lProgress.Percent = 90

        gApp.FlushEventQueue()
        
        setTimeRangeFromCuts(info, offsetTime)

        # TODO: output a work result (number of processed fcurves, clips, etc.)
        lProgress.ProgressDone()
        
        FBMessageBox('Import Info', 'Job is Done!', 'Ok')
