

/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////

//--- Class declarations
#include "orimpexpcsv_menu.h"

#ifdef KARCH_ENV_WIN
	#include <windows.h>
#endif

//--- Library Declaration
FBLibraryDeclare( orimpexptool )
{
	FBLibraryRegister( ORMenuItemCsvImport	);
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit()	{ return true; }
bool FBLibrary::LibOpen()	{ return true; }
bool FBLibrary::LibReady()
{
	ORMenuItemCsvImport*		lMenuImport = NULL;
	ORImportCsvEngine*			lEngine		= new ORImportCsvEngine;

	// Menu item activation
	FBMenuItemActivation( ORMenuItemCsvImport );
	FBMenuItemHandle	( ORMenuItemCsvImport, lMenuImport );

	// Both menu items share the same import/export engine.
	lMenuImport->mEngine = lEngine;

	return true;
}

bool FBLibrary::LibClose()	{ return true; }
bool FBLibrary::LibRelease()
{
	ORMenuItemCsvImport*		lMenuImport = NULL;

	// Get a handle onto the menu item.
	FBMenuItemHandle	( ORMenuItemCsvImport, lMenuImport );

    // Cleanup the engine.
	delete lMenuImport->mEngine;

    // Cleanup the menu entries.
    lMenuImport->FBDelete();

	return true;
}

/**
*	\mainpage	Import Export Tool
*	\section	intro	Introduction
*	This is a simple tool that will show the basic functions of
*	import/export using Open Reality. It shows the manipulation of
*	models, meshes, materials, textures and animation. It also
*	covers briefly the hierarchy that can be accessed by the FBSystem
*	class.
*	\section	walk	Walkthrough
*	\subsection func1	ImportScene()
*	<ol>
*	<li>	Creates the cube and the flag (see the CreateCube and CreateTexturedMesh functions);
*	<li>	Selects the flag;
*	<li>	Child the flag to the cube;
*	<li>	Set the TRS vectors for the cube and the flag;
*	<li>	Get the XYZ translation animation nodes for the cube;
*	<li>	Get the FCurves of the animation nodes;
*	<li>	Remove any animation that exists already (or create a new FCurve);
*	<li>	Add the keys for the translation of the cube;
*	<li>	Copy the FCurves into the animation nodes;
*	<li>	Add the cube (and its children) to the scene.
*	</ol>
*	\subsection	func2	CreateCube()
*	<ol>
*	<li>	Create a new model (no description);
*	<li>	Obtain the mesh from the model;
*	<li>	Set the model properties (shading mode);
*	<li>	Obtain a handle on a texture from the image path;
*	<li>	Create & configure two different materials;
*	<li>	Add the materials & textures to the mesh;
*	<li>	Create the vertices for the mesh;
*	<li>	Create the polygons by connecting the vertices;
*	<li>	Make the model visible;
*	<li>	Return the created model.
*	</ol>
*	\subsection	func3	CreateTexturedMesh()
*	<ol>
*	<li>	Create a new model (no description);
*	<li>	Obtain the mesh from the model;
*	<li>	Create the material & texture;
*	<li>	Add the material & texture to the model;
*	<li>	Configure the UV coordinates of the texture;
*	<li>	Make the model visible;
*	<li>	Return the model.
*	</ol>
*	\subsection	func4	ImportOpticalButtonEvent()
*	<ol>
*	<li>	Create a new optical model (see CreateOpticalModel function);
*	<li>	Get the animation nodes from the model;
*	<li>	Import the FCurves onto the animation node (see ImportFCurve function);
*	<li>	Import the animation onto the Optical model (see ImportOpticalModelAnimation function);
*	<li>	Add the model to the scene.
*	</ol>
*	\subsection	func5	ImportFCurve()
*	<ol>
*	<li>	Get the FCurve of the destination animation node (create if it does not exist);
*	<li>	Create a sinusoidal curve, for the number of samples specified, adding key frames every frame onto the FCurve in question;
*	<li>	Copy the FCurve onto the animation node.
*	</ol>
*	\subsection	func6	ImportOpticalModelAnimation()
*	<ol>
*	<li>	Set the sampling start & stop times, as well as the sampling period;
*	<li>	Call the import the setup function for the optical model, setting the internal values with the sampling properties that have been set;
*	<li>	For each child of the optical model: add the sample data for each sample, keying in the animation data.
*	</ol>
*	\subsection	func7	CreateOpticalModel()
*	<ol>
*	<li>	Create a new optical model (with no description);
*	<li>	Adjust the marker size;
*	<li>	Add the model to the scene;
*	<li>	Create the number of markers specified, adding them as children of the optical model;
*	<li>	Set the TRS vectors for the model;
*	<li>	Return the created optical model.
*	</ol>
*/
