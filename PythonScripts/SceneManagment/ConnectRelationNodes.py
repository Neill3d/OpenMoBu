
############################################################
#
#  Example on how to work with relation constraint nodes
#
# Author Sergey Solohin (Neill3d)
# homepage: http://neill3d.com
# e-mail to: s@neill3d.com
# 2012
############################################################


from pyfbsdk import*

# Define a utility function to look up a named animation node
# under a given parent. Will not recurse in the hierarchy.
def FindAnimationNode( pParent, pName ):
    lResult = None
    for lNode in pParent.Nodes:
        if lNode.Name == pName:
            lResult = lNode
            break
    return lResult

def FindRelationBox( pCon, pName ):
    lResult = None
    for lBox in pCon.Boxes:
        if lBox.Name == pName:
            lResult = lBox
            break
    return lResult


for lCon in FBSystem().Scene.Constraints:
    print lCon.Name
    if lCon.Name == 'Relation':
        
        cubeBox = FindRelationBox(lCon, 'Cube')
        vectorToNumberBox = FindRelationBox(lCon, 'Vector to Number')
        
        if cubeBox and vectorToNumberBox:
            outNode = FindAnimationNode( cubeBox.AnimationNodeOutGet(), 'Lcl Rotation')
            inNode = FindAnimationNode( vectorToNumberBox.AnimationNodeInGet(), 'V' )
        
            if outNode and inNode:
                FBConnect(outNode, inNode)
        
