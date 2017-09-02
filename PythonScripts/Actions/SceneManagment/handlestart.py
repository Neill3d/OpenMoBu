#***************************************************************************
#
#   Copyright   (c) 2007 CREOTEAM
#
#                   Handle start/finish
#   Algoritm: put in scene null object with name 'handle' which will be handle
#               pivot point. The select objects and run this one.
#               When finish u work, run 'HandleFinish' script to erase handle
#
#   Author: Solohin Sergey (Neill), e-mail to: neill@bk.ru
#
#   Date: 10.05.07
#
#***************************************************************************


from pyfbsdk import *

models = FBModelList()

# -- models for group with handler
FBGetSelectedModels( models )

# handler pivot
handleModel = FBFindModelByName( "handle" )

if handleModel:
    followModel = models[0]

    handle = FBHandle( "quickHandle" )
    handle.Follow.append( followModel )

    for model in models:
        handle.Manipulate.append( model )

        del( model )

    # setUp flag for using Pivot point
    lProp = handle.PropertyList.Find( 'Manipulate Around Handle' )
    if lProp: 
        lProp.Data = True
        del( lProp )

    # translation mode set to rigid
    lProp = handle.PropertyList.Find( 'Translation Mode' )
    if lProp:
        lProp.Data = 1
        del( lProp )

    # setUp Pivot position
    lclTranslation = FBVector3d()
    
    handleModel.Parent = followModel
    handleModel.GetVector( lclTranslation, FBModelTransformationMatrix.kModelTranslation, False )
    handleModel.Parent = None
    
    lProp = handle.PropertyList.Find( 'Translation Offset' )
    if lProp:
        lProp.Data = lclTranslation
        del( lProp )

    handle.Select()

    del( lclTranslation )
    
# cleanUp
del( models, handleModel )
