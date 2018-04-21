/***************************************************************************************
 Autodesk(R) Open Reality(R) Samples
 
 (C) 2009 Autodesk, Inc. and/or its licensors
 All rights reserved.
 
 AUTODESK SOFTWARE LICENSE AGREEMENT
 Autodesk, Inc. licenses this Software to you only upon the condition that 
 you accept all of the terms contained in the Software License Agreement ("Agreement") 
 that is embedded in or that is delivered with this Software. By selecting 
 the "I ACCEPT" button at the end of the Agreement or by copying, installing, 
 uploading, accessing or using all or any portion of the Software you agree 
 to enter into the Agreement. A contract is then formed between Autodesk and 
 either you personally, if you acquire the Software for yourself, or the company 
 or other legal entity for which you are acquiring the software.
 
 AUTODESK, INC., MAKES NO WARRANTY, EITHER EXPRESS OR IMPLIED, INCLUDING BUT 
 NOT LIMITED TO ANY IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR 
 PURPOSE REGARDING THESE MATERIALS, AND MAKES SUCH MATERIALS AVAILABLE SOLELY ON AN 
 "AS-IS" BASIS.
 
 IN NO EVENT SHALL AUTODESK, INC., BE LIABLE TO ANYONE FOR SPECIAL, COLLATERAL, 
 INCIDENTAL, OR CONSEQUENTIAL DAMAGES IN CONNECTION WITH OR ARISING OUT OF PURCHASE 
 OR USE OF THESE MATERIALS. THE SOLE AND EXCLUSIVE LIABILITY TO AUTODESK, INC., 
 REGARDLESS OF THE FORM OF ACTION, SHALL NOT EXCEED THE PURCHASE PRICE OF THE 
 MATERIALS DESCRIBED HEREIN.
 
 Autodesk, Inc., reserves the right to revise and improve its products as it sees fit.
 
 Autodesk and Open Reality are registered trademarks or trademarks of Autodesk, Inc., 
 in the U.S.A. and/or other countries. All other brand names, product names, or 
 trademarks belong to their respective holders. 
 
 GOVERNMENT USE
 Use, duplication, or disclosure by the U.S. Government is subject to restrictions as 
 set forth in FAR 12.212 (Commercial Computer Software-Restricted Rights) and 
 DFAR 227.7202 (Rights in Technical Data and Computer Software), as applicable. 
 Manufacturer is Autodesk, Inc., 10 Duke Street, Montreal, Quebec, Canada, H3C 2L7.
***************************************************************************************/

/**	\file	ormodel_template_model.cxx
*/

//--- Class declaration
#include "ormodel_template_model.h"


FBClassImplementation( ORModelCustom );					//Register class
FBStorableClassImplementation( ORModelCustom, Model );	//Register to the store/retrieve system
FBElementClassImplementation( ORModelCustom, FB_DEFAULT_SDK_ICON );			//Register to the asset system

/************************************************
 *	Constructor.
 ************************************************/
ORModelCustom::ORModelCustom( char* pName, HIObject pObject ) : FBModel( pName, pObject )
{
	FBClassInit;
}

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ORModelCustom::FBCreate()
{
	CreateGeometry	();

	ShadingMode = kFBModelShadingTexture;
	return true;
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void ORModelCustom::FBDestroy()
{
}

/************************************************
 *	Create the geometry.
 ************************************************/
void ORModelCustom::CreateGeometry()
{
	FBMesh		lMesh( this );

	double len = 10.0;//unit length

	// Object creation
	int P0a,P0e,P0d;
	int P1a,P1b,P1e;
	int P2a,P2b,P2c;
	int P3a,P3c,P3d;
	int P4d,P4e,P4f;
	int P5b,P5e,P5f;
	int P6b,P6c,P6f;
	int P7c,P7d,P7f;
	lMesh.GeometryBegin();

	// Step 1: Create vertices

	// Because FiLMBOX doesn't yet support multiple UV or multiple Normals per vertex,
	// the cube's vertices need to be duplicated
	// Surface A: Y=0 N=(0,-1,0)
	lMesh.VertexNormalSet(0.0,-1.0,0.0);

	lMesh.VertexUVSet(0.0,0.0);
	P0a = lMesh.VertexAdd(	0.0,	0.0,	0.0	);

	lMesh.VertexUVSet(0.0,1.0);
	P1a = lMesh.VertexAdd(	0.0,	0.0,	len	);

	lMesh.VertexUVSet(1.0,0.0);
	P2a = lMesh.VertexAdd(	len,	0.0,	len	);

	lMesh.VertexUVSet(1.0,1.0);
	P3a = lMesh.VertexAdd(	len,	0.0,	0.0	);

	// Surface B: Z=1, N=(0,0,1)
	lMesh.VertexNormalSet(0,0,1);
	lMesh.VertexUVSet(0.0,0.0);
	P1b = lMesh.VertexAdd(	0.0,	0.0,	len	);
	lMesh.VertexUVSet(1.0,0.0);
 	P2b = lMesh.VertexAdd(	len,	0.0,	len	);
	lMesh.VertexUVSet(0.0,1.0);
 	P5b = lMesh.VertexAdd(	0.0,	len,	len	);
	lMesh.VertexUVSet(1.0,0.5);
 	P6b = lMesh.VertexAdd(	len,	len,	len	);
	// Surface C: X=1 N=(1,0,0)
	lMesh.VertexNormalSet(1,0,0);
	P2c = lMesh.VertexAdd(	len,	0.0,	len	);
	P3c = lMesh.VertexAdd(	len,	0.0,	0.0	);
 	P6c = lMesh.VertexAdd(	len,	len,	len	);
 	P7c = lMesh.VertexAdd(	len,	len,	0.0	);

	// Surface D: Z=0 N=(0,0,-1)
	lMesh.VertexNormalSet(0,0,-1);
	P0d = lMesh.VertexAdd(	0.0,	0.0,	0.0	);
	P3d = lMesh.VertexAdd(	len,	0.0,	0.0	);
 	P7d = lMesh.VertexAdd(	len,	len,	0.0	);
 	P4d = lMesh.VertexAdd(	0.0,	len,	0.0	);

	// Surface E: X=0 N=(-1,0,0)
	lMesh.VertexNormalSet(-1,0,0);
	P0e = lMesh.VertexAdd(	0.0,	0.0,	0.0	);
	P1e = lMesh.VertexAdd(	0.0,	0.0,	len	);
 	P4e = lMesh.VertexAdd(	0.0,	len,	0.0	);
 	P5e = lMesh.VertexAdd(	0.0,	len,	len	);

	// Surface F: Y=1 N=(0,1,0)
	lMesh.VertexNormalSet(0,1,0);
 	P4f = lMesh.VertexAdd(	0.0,	len,	0.0	);
 	P5f = lMesh.VertexAdd(	0.0,	len,	len	);
 	P6f = lMesh.VertexAdd(	len,	len,	len	);
 	P7f = lMesh.VertexAdd(	len,	len,	0.0	);


	// Step 2: Create polygons
	lMesh.PolygonBegin();
		lMesh.PolygonVertexAdd(P0a);
		lMesh.PolygonVertexAdd(P3a);
		lMesh.PolygonVertexAdd(P2a);
		lMesh.PolygonVertexAdd(P1a);
	lMesh.PolygonEnd();

	lMesh.PolygonBegin();
		lMesh.PolygonVertexAdd(P1b);
		lMesh.PolygonVertexAdd(P2b);
		lMesh.PolygonVertexAdd(P6b);
		lMesh.PolygonVertexAdd(P5b);
	lMesh.PolygonEnd();

	lMesh.PolygonBegin();
		lMesh.PolygonVertexAdd(P2c);
		lMesh.PolygonVertexAdd(P3c);
		lMesh.PolygonVertexAdd(P7c);
		lMesh.PolygonVertexAdd(P6c);
	lMesh.PolygonEnd();

	lMesh.PolygonBegin();
		lMesh.PolygonVertexAdd(P0d);
		lMesh.PolygonVertexAdd(P4d);
		lMesh.PolygonVertexAdd(P7d);
		lMesh.PolygonVertexAdd(P3d);
	lMesh.PolygonEnd();

	lMesh.PolygonBegin();
		lMesh.PolygonVertexAdd(P1e);
		lMesh.PolygonVertexAdd(P5e);
		lMesh.PolygonVertexAdd(P4e);
		lMesh.PolygonVertexAdd(P0e);
	lMesh.PolygonEnd();

	lMesh.PolygonBegin();
		lMesh.PolygonVertexAdd(P4f);
		lMesh.PolygonVertexAdd(P5f);
		lMesh.PolygonVertexAdd(P6f);
		lMesh.PolygonVertexAdd(P7f);
	lMesh.PolygonEnd();

	lMesh.GeometryEnd();
}

bool ORModelCustom::FbxStore(HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}

bool ORModelCustom::FbxRetrieve(HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}
