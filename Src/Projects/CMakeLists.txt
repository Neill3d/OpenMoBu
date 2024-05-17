
# relation boxes

add_subdirectory( box_poseReader )
add_subdirectory( box_poseTransform )
add_subdirectory( box_rayIntersect )
add_subdirectory( box_RBF )
add_subdirectory( box_Spring )

# constraints

add_subdirectory( constraint_attachment )
add_subdirectory( constraint_driven )
add_subdirectory( constraint_twistextraction )

# devices

add_subdirectory( device_faceCap )
add_subdirectory( device_projectTango )

# misc (objects, tools)

add_subdirectory( FBXExt_ReferencesFix )
add_subdirectory( misc_VolumeCalculator)
add_subdirectory( tool_BlendShape )

# graphics

add_subdirectory( manip_lockCamera )
add_subdirectory( manager_PostProcessing )

add_subdirectory( shader_SuperDynamicLighting )
add_subdirectory( solver_calculateNormals )

add_subdirectory( renderer_colors )
add_subdirectory( texture_advanceBlend )

# references manager and fbx extraction console app

add_subdirectory(cmd_shadingGraph_exporter)
add_subdirectory( manager_References )
add_subdirectory(manager_CameraLinkVis)