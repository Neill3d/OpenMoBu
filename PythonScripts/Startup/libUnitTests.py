
#
# LIBRARY OF FUNCTIONS TO DO A UNIT TESTING INSIDE THE MOBU
#
# Sergey Solokhin (Neill3d) 2016-2017
#   e-mail to: s@neill3d.com
#       www.neill3d.com
#

# PLAN FOR THE FEATURES LIST:

# measure average frametime
# measure average framerate (from FBApplication method)
# measure render time
# try to do display and offline rendering, with and without AA
#
# output screenshot, render video
# output log file with executing information and stats

from pyfbsdk import *

import os
import re

gApp = FBApplication()
gSystem = FBSystem()
gProfiler = FBProfiler()
gPlayer = FBPlayerControl()

gMainTestFolder = "D:\\Work_Google_Disk\\UNIT_TESTS\\"
gUnitTestSubFolder = ""
# Full dir = main dir + sub dir
gFullTestFolder = ""

gLogFilename = "\\unitTestsLog.txt"
gLogFullFilename = ""

############# LOG

def InitLOG(filepath):
    global gLogFullFilename
    if filepath == "":
        filepath = gSystem.ApplicationPath 
        
    gLogFullFilename = filepath + gLogFilename
    print gLogFullFilename

def StartLOG():
    global gLogFullFilename
    if gLogFullFilename != "":
        f = open(gLogFullFilename, 'wt')
        f.write( "Start logging...\n\n" )
        f.close()
    
def InfoLOG(text):
    global gLogFullFilename
    if gLogFullFilename != "":
        f = open(gLogFullFilename, 'a')
        f.write(text + "\n")
        f.close()

############# PATH

def CheckMainUnitTestFolder():
    
    global gMainTestFolder
    
    if not os.path.isdir(gMainTestFolder):
        lDialog = FBFolderPopup()
        lDialog.Caption = "Please choose a unit tests root folder"
        if lDialog.Execute():
            gMainTestFolder = lDialog.Path
        else:
            return False
    return True

def SetSubFolder(subpath):
    global gUnitTestSubFolder
    global gFullTestFolder
    
    gUnitTestSubFolder = subpath
    gFullTestFolder = gMainTestFolder + gUnitTestSubFolder
    
    if not os.path.isdir(gFullTestFolder):
        FBMessageBox("Unit Tests", "Unit Test subfolder doesn't exist", "Ok")
        return False
    return True

def GetCurrentPath():
    global gUnitTestSubFolder
    global gFullTestFolder
    
    gFullTestFolder = gMainTestFolder + gUnitTestSubFolder
    
    return gFullTestFolder

############# SCENE

def NewScene():
    gApp.FileNew()

def OpenScene(filename):
    
    if gFullTestFolder != "":
        gApp.FileOpen( gFullTestFolder + "\\" + filename )

def DisplayLightedMode():
    
    lrenderer = gSystem.Renderer
    loptions = lrenderer.GetViewingOptions()

    loptions.DisplayMode = FBModelShadingMode.kFBModelShadingLight
    lrenderer.SetViewingOptions(loptions)

def DisplayShaderMode():
    
    lrenderer = gSystem.Renderer
    loptions = lrenderer.GetViewingOptions()

    loptions.DisplayMode = FBModelShadingMode.kFBModelShadingAll
    lrenderer.SetViewingOptions(loptions)

def MakeAScreenshot(filename):

    InfoLOG( "make a screenshot" )
    
    lGrabber = FBVideoGrabber()
    
    image = lGrabber.RenderSnapshot( 640, 480 )
    if image:
        lfilename = filename
        if lfilename == None or lfilename == "":
            lfilename = "screenshot.tif"
            
        if gFullTestFolder != "":
            lfilename = gFullTestFolder + "\\" + lfilename            
            image.WriteToTif( lfilename, "Unit Test Screenshot", True )

    InfoLOG( "\t ... finished" )

def MakeAVideo(filename, antialiasing, numberOfFrames):
    print "make video"
    
    InfoLOG( "Make a video test" )
    
    if gFullTestFolder == "":
        return 0
    
    if antialiasing:
        InfoLOG( "antialiasing is on" )
    else:
        InfoLOG( "antialiasing is off" )
    
    # determine render number of frames
    gPlayer.GotoStart()
    startFrame = FBSystem().LocalTime.GetFrame()
    
    stopTime = gPlayer.LoopStop
    stopFrame = stopTime.GetFrame()
    
    limit = stopFrame - startFrame
    if limit > numberOfFrames:
        limit = int(numberOfFrames)
    stopFrame = limit
    
    #
    # Do the render
    #
    # Get the default rendering options, which are saved in the FBX file.
 
    lRenderFileFormat = '.avi'
    lOptions = FBVideoGrabber().GetOptions()
                    
    # Set VideoCodec Option:
    VideoManager = FBVideoCodecManager()
    VideoManager.VideoCodecMode = FBVideoCodecMode.FBVideoCodecStored
    
    codeclist = FBStringList()
    codellist = VideoManager.GetCodecIdList('AVI');
    for item in codeclist:
        if item.find('XVID') >= 0: 
            VideoManager.SetDefaultCodec('AVI', item)
            #FBMessageBox( "video code", "xvid is found", "Ok" )
    
    # Set the name of the rendered file.
    lDstFileName = gFullTestFolder + "\\" + filename
    lOptions.OutputFileName = lDstFileName
    lOptions.CameraResolution = FBCameraResolutionMode.kFBResolutionHD
    lOptions.AntiAliasing = antialiasing
    # for 60 fps lets white video in half frame rate
    lOptions.TimeSpan = FBTimeSpan(FBTime(0,0,0,0), FBTime(0,0,0,stopFrame))
    lOptions.TimeSteps = FBTime(0,0,0,1) 
    
    # On Mac OSX, QuickTime renders need to be in 32 bits.
    if lRenderFileFormat == '.mov' and platform == 'darwin':
        lOptions.BitsPerPixel = FBVideoRenderDepth.FBVideoRender32Bits
    
    # Do the render. This will always be done in uncompressed mode.
    gApp.FileRender( lOptions )
    
    InfoLOG( "> ... render finish" )
    return 1
    
def MakeABenchmark(filename, numberOfFrames):
    
    InfoLOG( "make a benchmark" )

    if gFullTestFolder == "":
        return 0
    
    # Activate collection of time events information
    gProfiler.ActiveSampling = True
    gProfiler.FrameReference = True
    # Play for 50 frames
    gPlayer.LoopActive = False
    gPlayer.GotoStart()
    #gPlayer.Goto( FBTime(0,0,0,0) )
    startFrame = FBSystem().LocalTime.GetFrame()
    
    stopTime = gPlayer.LoopStop
    stopFrame = stopTime.GetFrame()
    
    limit = stopFrame - startFrame
    if limit > numberOfFrames:
        limit = int(numberOfFrames)
    
    gPlayer.Play()
    while FBSystem().LocalTime.GetFrame()-startFrame < limit:
        gApp.FlushEventQueue()

    # t.editBenchmarkLength.Value

    # Wait for playing to stop
    gPlayer.Stop()
    while gPlayer.IsPlaying:
        gApp.FlushEventQueue()
                
    # Deactivate collection of time events information
    gProfiler.ActiveSampling = False
    
    lSamplesCount = gProfiler.GetEventSampleCount()
    print "Number of samples collected: %d\n" % lSamplesCount

    lMainTaskCycle = FBGetMainThreadTaskCycle()
    lRenderTaskCycle = FBGetRenderingTaskCycle()
    lEvalTaskCycle = FBGetEvaluationTaskCycle()
    
    if lSamplesCount > 0:
        #logPath = os.path.join( tempfile.gettempdir(), "EventsLog.txt" )
        f = open(gFullTestFolder + "\\" + filename, 'wt')
    
        [avgValue, minValue, maxValue] = lMainTaskCycle.GetAvgMinMaxUsage()
        lString = "Main Tasks - average %f, min %f, max %f\n" % (avgValue, minValue, maxValue)
        f.write(lString)
        
        [avgValue, minValue, maxValue] = lRenderTaskCycle.GetAvgMinMaxUsage()
        lString = "Render Tasks - average %f, min %f, max %f\n" % (avgValue, minValue, maxValue)
        f.write(lString)
        
        [avgValue, minValue, maxValue] = lEvalTaskCycle.GetAvgMinMaxUsage()
        lString = "Eval Tasks - average %f, min %f, max %f\n" % (avgValue, minValue, maxValue)
        f.write(lString)
        
        f.close()
    
        print "File successfully saved to %s" % filename
        
    InfoLOG( "\t ... finished" )
    return 1

################# START / STOP testing

def START( subpath ):
    
    if not CheckMainUnitTestFolder():
        return False
    
    if not SetSubFolder(subpath):
        return False
    
    InitLOG(gFullTestFolder)
    StartLOG()
    
    return True
    
def STOP():
    
    InfoLOG( "\n\nSTOP" )




    