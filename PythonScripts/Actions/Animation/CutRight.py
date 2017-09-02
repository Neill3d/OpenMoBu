# CutRight.py
#
# script for cutting TRS keys 
#  from current time to the end of the time range
#   for all selected objects in scene
#
# Author Sergey Solohin (Neill3d), mail to: neill.solow@gmail.com
# homepage: http://neill3d.com

from pyfbsdk import *

list = FBModelList()
FBGetSelectedModels(list)

def curveCutRight(node, time):
    curve = node.FCurve
    if curve:
        count = len(curve.Keys)
        while count > 0:
            key = curve.Keys[count-1]
    
            if (key.Time.Get() >= time.Get()):
                curve.KeyRemove(count-1)
            
            count -= 1

ltime = FBTime()
ltime = FBSystem().LocalTime

undoman = FBUndoManager()
undoman.TransactionBegin("CutLeft")

for obj in list:
    undoman.TransactionAddModelTRS(obj)
    tnode = obj.Translation.GetAnimationNode()
    if tnode:
        curveCutRight(tnode.Nodes[0], ltime)
        curveCutRight(tnode.Nodes[1], ltime)
        curveCutRight(tnode.Nodes[2], ltime)        
    
    tnode = obj.Rotation.GetAnimationNode()
    if tnode:
        curveCutRight(tnode.Nodes[0], ltime)
        curveCutRight(tnode.Nodes[1], ltime)
        curveCutRight(tnode.Nodes[2], ltime)   
        
    # try to find quality property and cut it
    prop = obj.PropertyList.Find("Quality")
    if prop and prop.IsAnimatable():
        undoman.TransactionAddProperty(prop)
        tnode = prop.GetAnimationNode()
        if tnode: curveCutRight(tnode, ltime)   
        
undoman.TransactionEnd()