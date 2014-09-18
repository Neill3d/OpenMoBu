from pyfbsdk import *

list = FBModelList()
FBGetSelectedModels(list)

for obj in list:

    # Create a Key Extrapolate filter.
    lFilter = FBFilterManager().CreateFilter( 'Extrapolate' )

    if lFilter:
        # Set the filter's precision to 2.0, and apply it to
        # the object's translation animation.
        
        #for lProp in lFilter.PropertyList:
        #    print lProp.Name
        
        lFilter.PropertyList.Find( 'Pre Enabled' ).Data = True
        lFilter.Apply( obj.Translation.GetAnimationNode(), True )
