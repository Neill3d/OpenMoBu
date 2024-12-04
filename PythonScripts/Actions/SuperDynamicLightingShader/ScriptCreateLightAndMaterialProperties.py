
# ScriptCreateLightAndMaterialProperties.py
#
# Sergei <Neill3d> Solokhin 2024

from pyfbsdk import*

scene = FBSystem().Scene

for mat in scene.Materials:
    if mat.Selected:
        print (mat.Name)

        # 0.0 - Lambert, 1.0 - Oren-Nayar
        if not mat.PropertyList.Find('Diffuse Type'):
            mat.PropertyCreate('Diffuse Type', FBPropertyType.kFBPT_double, 'Number', True, True, None)
        
        # 0.0 - Phong, 1.0 - Anisotropic, 2.0 - Skin (Cook Torrance)
        if not mat.PropertyList.Find('Specular Type'):
            mat.PropertyCreate('Specular Type', FBPropertyType.kFBPT_double, 'Number', True, True, None)
        #mat.PropertyList.Find('Specular Type').SetString('Phong|Anisotropic|Skin')
        
        if not mat.PropertyList.Find('RoughnessX'):
            mat.PropertyCreate("RoughnessX", FBPropertyType.kFBPT_double, 'Number', True, True, None)        
        if not mat.PropertyList.Find('RoughnessY'):
            mat.PropertyCreate("RoughnessY", FBPropertyType.kFBPT_double, 'Number', True, True, None)        
            
            
for light in scene.Lights:
    if light.Selected:
        
        print (light.Name)       
        
        if not light.PropertyList.Find('Cast Specular On Object'):
            light.PropertyCreate('Cast Specular On Object', FBPropertyType.kFBPT_bool, 'Bool', True, True, None)     
            
        if not light.PropertyList.Find('Shadow Bounding Box'):
            light.PropertyCreate('Shadow Bounding Box', FBPropertyType.kFBPT_object, 'Object', False, True, None)