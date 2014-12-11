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

lSystem = FBSystem()
lScene = lSystem.Scene

for handle in lScene.Handles:
    if handle.Name == 'quickHandle':
        for item in handle.Manipulate:
            handle.Manipulate.remove( item )

            del( item )

        handle.FBDelete()

    del( handle )

del( lSystem, lScene )
