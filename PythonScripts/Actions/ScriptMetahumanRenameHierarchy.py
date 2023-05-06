#
# ScriptMetahumanRenameHierarchy.py
#
# script to name two identical hierarchies the same way but with adding a namespace
#  namespace is needed as in motionbuilder the model must have unique name
#  this is made for metahuman skeleton, exported from the sequencer 
# in order to merge body part skeletons into one model skeleton
#
# How to use
#  Select a root of body part which hierarchy you want to use as a naming reference
# Select a root of body part which hierarchy you want to rename with a namespace
# Run a script
#
# Sergei <Neill3d> Solokhin 2023, email to: neill3d@gmail.com
#

from pyfbsdk import *

def StoreChild(model, storage):
    storage[model.Name] = [x.Name for x in model.Children]

    for x in model.Children:
        StoreChild(x, storage)

def Store(root):
    
    storage = dict()
    StoreChild(root, storage)

    return (storage, root.Name)
    
    
def RestoreChild(model, storage, rootName=None, prefix="new_"):

    storageKey = model.Name if rootName is None else rootName
    if not storage.has_key(storageKey):
        return
    
    childrenNames = storage[storageKey]
    hasDifferentChildren = True # len(childrenNames) != len(model.Children)
    
    for (j, child) in enumerate(model.Children):
        # find a best candidate
        key = child.Name

        bestMatch = 0
        bestName = childrenNames[j] if j < len(childrenNames) else key
        
        if hasDifferentChildren:
            for theName in childrenNames:
                
                count = len(key) if len(key) < len(theName) else len(theName)
                counter = 0
                
                for i in xrange(count):
                    if (theName[i] == key[i]):
                        counter += 1
                    else: break
                        
                if (counter > bestMatch and counter > 3):
                    bestMatch = counter
                    bestName = theName
                
        child.LongName = prefix + bestName
        RestoreChild(child, storage, bestName, prefix)
        
    
def Restore(root, storage, rootName, prefix):    
    RestoreChild(root, storage, rootName, prefix)
    
#
# Main
models = FBModelList()
FBGetSelectedModels(models, None, True, True) # sort by select order, we need first src, second dst
if len(models) == 2:
    
    root = models[0]
    (storage, rootName) = Store(root)
    Restore(models[1], storage, rootName, prefix=models[1].Name+":")