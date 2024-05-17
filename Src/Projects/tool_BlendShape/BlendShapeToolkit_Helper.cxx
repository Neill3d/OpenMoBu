
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BlendShapeToolkit_Helper.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "BlendShapeToolkit_Helper.h"
#include "math3d.h"
#include "BlendShapeToolkit_manip.h"
#include "BlendShapeToolkit_deformer_constraint.h"
#include "ResourceUtils.h"
#include "ClusterAdvance.h"
#include "GeometryUtils.h"

#include "tinyxml.h"
#include <string>

BlendShapeDeformerConstraint	*gTempConstraint = nullptr;		// this deform constraint is used for shaping


// all index and result position, normal arrays must be allocated with the length of numberOfVertices
//  if vertexIndex < 0 - no difference in that vertex
// return number of differences

const int CalculateDifference( const int numberOfVertices, const FBVertex *positions, const FBVertex *positionsBase, int *outOriginIndex, FBVertex *outPositions, const bool calcNormals, const FBNormal *normals, const FBNormal *normalsBase, FBNormal *outNormals )
{
	int difCount = 0;

	FBVertex *v1 = (FBVertex*) positions;
	FBVertex *v2 = (FBVertex*) positionsBase;
	FBNormal *n1 = (FBNormal*) normals;
	FBNormal *n2 = (FBNormal*) normalsBase;
	
	FBVertex pDif;

	for (int i=0; i<numberOfVertices; ++i)
	{
		outOriginIndex[i] = -1;
		
		pDif = FBVertex(v1->mValue[0] - v2->mValue[0], v1->mValue[1] - v2->mValue[1], v1->mValue[2] - v2->mValue[2]);
		double len = sqrt(pDif[0]*pDif[0] + pDif[1]*pDif[1] + pDif[2]*pDif[2]);
		if (len > 0.001) 
		{
			outPositions[i] = pDif;
			outOriginIndex[i] = i;

			if (calcNormals)
				outNormals[i] = FBNormal( n1->mValue[0] - n2->mValue[0], n1->mValue[1] - n2->mValue[1], n1->mValue[2] - n2->mValue[2] );
			
			difCount += 1;
		}

		v1++;
		v2++;
		if (calcNormals)
		{
			n1++;
			n2++;
		}
	}

	return difCount;
}

int AddBlendShape( FBModel *pBaseModel, const int pointCount, const int *difOriginIndex, const int difCount, const FBVertex *difPositions, const FBNormal *difNormals, const char *defaultShapeName )
{
	int shapeId = -1;
	if (pBaseModel == nullptr) return shapeId;

	FBGeometry *pBaseGeometry = pBaseModel->Geometry;
	if (pBaseGeometry == nullptr) return shapeId;

	if (difCount && difPositions)
	{
		pBaseGeometry->GeometryBegin();

		FBString shapeName(defaultShapeName);
		FindUniqueBlendShapeName( pBaseModel, defaultShapeName, shapeName );
		shapeId = pBaseGeometry->ShapeAdd( shapeName );
		
		if (shapeId >= 0)
		{
			pBaseGeometry->ShapeInit(shapeId, difCount);

			bool result = false;
			int difId = 0;

			for (int j=0; j<pointCount; ++j)
			{
				if (difOriginIndex[j] >= 0)
				{
				
					if (difNormals)
					{
						result = pBaseGeometry->ShapeSetDiffPoint(shapeId, 
																difId, 
																difOriginIndex[j], 
																difPositions[j],
																difNormals[j] );
					}
					else
					{
						result = pBaseGeometry->ShapeSetDiffPoint(shapeId, 
																difId, 
																difOriginIndex[j], 
																difPositions[j] );
					}

					if (result == false)
					{
						printf( "diff point failed\n" );
					}

					difId++;
				}
			}
		}

		pBaseGeometry->GeometryEnd();
		pBaseGeometry->ModifyNotify();
		pBaseModel->SetupPropertiesForShapes();
		pBaseModel->BlendShapeDeformable = true;
	}

	return shapeId;
}

// return shapeId or -1 if failed to add
int AddBlendShape( FBModel *pBaseModel, FBModel *pModel )
{
	int shapeId = -1;
	if (pBaseModel == nullptr || pModel == nullptr) return shapeId;

	FBGeometry *pBaseGeometry = pBaseModel->Geometry;
	FBGeometry *pGeometry = pModel->Geometry;

	if (pBaseGeometry == nullptr || pGeometry == nullptr) return shapeId;
	
	int baseCount = pBaseGeometry->VertexCount();
	int count = pGeometry->VertexCount();

	if (baseCount != count)
	{
		FBMessageBox( "Adding a blendshape", "Error: shape geometry is not equal to the base mesh", "Ok" );
		return shapeId;
	}
	
	FBArrayTemplate<int>	pOriginIndex;
	FBArrayTemplate<FBVertex> difPositions;
	FBArrayTemplate<FBNormal> normals, normalsBase, difNormals;

	pOriginIndex.SetCount(count);
	difPositions.SetCount(count);
	normals.SetCount(count);
	normalsBase.SetCount(count);
	difNormals.SetCount(count);

	for (int i=0; i<count; ++i)
	{
		normals[i] = pGeometry->VertexNormalGet(i);
		normalsBase[i] = pBaseGeometry->VertexNormalGet(i);
	}

	int difCount = CalculateDifference( count, pGeometry->GetVertexes(), pBaseGeometry->GetVertexes(), pOriginIndex.GetArray(), difPositions.GetArray(), true, normals.GetArray(), normalsBase.GetArray(), difNormals.GetArray() );
	
	char strValue[128];
	memset( strValue, 0, sizeof(char) * 128 );
	sprintf_s( strValue, 128, "%s\0", "newShape" );

	if (1 == FBMessageBoxGetUserValue( "Adding a blendshape", "Enter a new shape name:", strValue, kFBPopupString, "Ok", "Cancel" ) )
	{
		shapeId = AddBlendShape( pBaseModel, count, pOriginIndex.GetArray(), difCount, difPositions.GetArray(), difNormals.GetArray(), strValue );
	}

	return shapeId;
}

FBModel *CalculateDeformedMesh(FBModel *in)
{
	FBModel *pNewModel = MakeSnapshot(in, false);

	ClusterAdvance	clusterAdvance(in);
	
	FBGeometry *pGeometryOUT = pNewModel->Geometry;
	
	int vertCountOUT, normCountOUT;
	FBVertex *pPositionsOUT = pGeometryOUT->GetPositionsArray(vertCountOUT);
	FBNormal *pNormalsOUT = pGeometryOUT->GetNormalsDirectArray(normCountOUT);

	for (int i=0; i<vertCountOUT; ++i)
	{
		pPositionsOUT[i] = clusterAdvance.CalculateDeformedPosition(i);
		pNormalsOUT[i] = clusterAdvance.CalculateDeformedNormal(i);
	}

	pGeometryOUT->ModifyNotify();

	return pNewModel;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct Blendshape
{
	FBString					name;
	int							difCount;

	FBArrayTemplate<int>		oriIndex;
	FBArrayTemplate<FBVertex>	posDiff;
	FBArrayTemplate<FBNormal>	normalDiff;
};

void StoryGeometryBlendshapes(FBGeometry *pGeometry, std::vector<Blendshape*> &blendshapes)
{
	const int count = pGeometry->ShapeGetCount();
	if (count == 0) return;

	blendshapes.resize(count);
	//Blendshape *blendshapes = new Blendshape[count];

	for (int i=0; i<count; ++i)
	{
		Blendshape *newBlendshape = new Blendshape();
		newBlendshape->name = pGeometry->ShapeGetName(i);

		int difCount = pGeometry->ShapeGetDiffPointCount(i);

		newBlendshape->difCount = difCount;
		newBlendshape->oriIndex.SetCount(difCount);
		newBlendshape->posDiff.SetCount(difCount);
		newBlendshape->normalDiff.SetCount(difCount);

		for (int j=0; j<difCount; ++j)
		{
			int oriIndex=0;
			FBVertex posDiff;
			FBNormal normalDiff;

			pGeometry->ShapeGetDiffPoint(i, j, oriIndex, posDiff, normalDiff);

			newBlendshape->oriIndex[j] = oriIndex;
			newBlendshape->posDiff[j] = posDiff;
			newBlendshape->normalDiff[j] = normalDiff;
		}

		blendshapes[i] = newBlendshape;
	}
}

void FreeGeometryBlendshapes(std::vector<Blendshape*> &blendshapes)
{
	for (auto iter=blendshapes.begin(); iter!=blendshapes.end(); ++iter)
	{
		Blendshape *lblendshape = *iter;

		if (lblendshape)
		{
			delete lblendshape;
			lblendshape = nullptr;
		}
	}
}


void ModifyGeometryShapes(FBGeometry *pGeometry, bool *checkList, const FBShapeChangeReason reason)
{
	std::vector<Blendshape*>	blendshapes;

	//
	// store all blendshapes in temp struct
	StoryGeometryBlendshapes(pGeometry, blendshapes);
	
	//
	//
	pGeometry->ShapeClearAll();

	const int strSize = 128;
	char strValue[strSize];
	memset( strValue, 0, sizeof(char) * strSize );

	pGeometry->GeometryBegin();
	const int count = blendshapes.size();

	FBArrayTemplate<bool>		localCheckList;
	localCheckList.SetCount(count);
	for (int i=0; i<count; ++i)
		localCheckList[i] = checkList[i];
	bool *ptrCheckList = localCheckList.GetArray();

	for (int i=0; i<count; ++i)
	{
		Blendshape *lblendshape = blendshapes[i];

		if ( (reason == kFBShapeMoveUp) && (i < count-1) && (ptrCheckList[i+1] == true) )
		{
			blendshapes[i] = blendshapes[i+1];
			blendshapes[i+1] = lblendshape;
			lblendshape = blendshapes[i];
			
			checkList[i+1] = false;
			checkList[i] = true;
		}
		else if ((reason == kFBShapeMoveDown) && (ptrCheckList[i] == true) )
		{
			int nextitem = i;

			for (int j=i; j<count; ++j)
				if (ptrCheckList[j] == false)
				{
					nextitem = j;
					break;
				}

			while (nextitem > i)
			{
				lblendshape = blendshapes[nextitem];
				blendshapes[nextitem] = blendshapes[nextitem-1];
				blendshapes[nextitem-1] = lblendshape;

				bool lstate = checkList[nextitem-1];
				checkList[nextitem-1] = checkList[nextitem];
				checkList[nextitem] = lstate;

				ptrCheckList[nextitem] = false;

				nextitem--;
			}

			lblendshape = blendshapes[i];
		}

		if (ptrCheckList[i] == true)
		{
			switch(reason)
			{
			case kFBShapeMoveUp:
				{
					if (i > 0 && (i < count-1))
					{
						
					}
				}
				break;

			case kFBShapeDublicate:
				{
					// dublicate shape
					sprintf_s( strValue, strSize, "%s\0", static_cast<const char*>(lblendshape->name) );
					if (1 == FBMessageBoxGetUserValue( "BlendShape Toolkit", "Please enter a dublicated shape name", strValue, kFBPopupString, "Ok", "Cancel" ) )
					{
						int idx = pGeometry->ShapeAdd( strValue );
						int difCount = lblendshape->difCount;

						pGeometry->ShapeInit(idx, difCount, true);
						for (int j=0; j<difCount; ++j)
						{
							pGeometry->ShapeSetDiffPoint( idx, j, lblendshape->oriIndex[j], lblendshape->posDiff[j], lblendshape->normalDiff[j] );
						}
					}
				} break;

			case kFBShapeRename:
				
				sprintf_s( strValue, strSize, "%s\0", static_cast<const char*>(lblendshape->name) );
				if (1 == FBMessageBoxGetUserValue( "BlendShape Toolkit", "Please enter a new shape name", strValue, kFBPopupString, "Ok", "Cancel" ) )
				{
					int idx = pGeometry->ShapeAdd( strValue );
					int difCount = lblendshape->difCount;

					pGeometry->ShapeInit(idx, difCount, true);
					for (int j=0; j<difCount; ++j)
					{
						pGeometry->ShapeSetDiffPoint( idx, j, lblendshape->oriIndex[j], lblendshape->posDiff[j], lblendshape->normalDiff[j] );
					}
				}
				continue;

			case kFBShapeRemove:
				continue;
			}
		}
		
		int idx = pGeometry->ShapeAdd( lblendshape->name );
		int difCount = lblendshape->difCount;

		pGeometry->ShapeInit(idx, difCount, true);
		for (int j=0; j<difCount; ++j)
		{
			pGeometry->ShapeSetDiffPoint( idx, j, lblendshape->oriIndex[j], lblendshape->posDiff[j], lblendshape->normalDiff[j] );
		}
	}
	pGeometry->GeometryEnd();
	pGeometry->ModifyNotify();

	//
	// free memory
	//
	FreeGeometryBlendshapes(blendshapes);
}

////////////////////////////////////////////////////////////////////////////////////////////
//
bool	Blendshapes_SaveXML( FBModelList &modelList, const char *filename )
{
	TiXmlDocument doc;

	TiXmlElement	head("Header");
	head.SetAttribute( "numberOfModels", modelList.GetCount() );
	head.SetAttribute( "version", 1 );

	for (int nModel=0; nModel<modelList.GetCount(); ++nModel)
	{
		FBModel *pModel = modelList[nModel];
		FBGeometry *pGeometry = pModel->Geometry;

		TiXmlElement modelItem("Model");
		modelItem.SetAttribute( "name", pModel->LongName );
		
		std::vector<Blendshape*>	blendshapes;

		//
		// store all blendshapes in temp struct
		StoryGeometryBlendshapes(pGeometry, blendshapes);
	
		const int numberOfShapes = pGeometry->ShapeGetCount();
		modelItem.SetAttribute( "numberOfShapes", numberOfShapes );

		for (int nShape=0; nShape<numberOfShapes; ++nShape)
		{
			TiXmlElement	shapeItem("Shape");

			Blendshape *pShape = blendshapes[nShape];

			shapeItem.SetAttribute( "name", (char*) pShape->name );
			shapeItem.SetAttribute( "numberOfDiffs", pShape->difCount );

			const int difCount = blendshapes[nShape]->difCount;
			for (int i=0; i<difCount; ++i)
			{
				TiXmlElement difItem("Diff");

				difItem.SetAttribute( "OriIndex", pShape->oriIndex[i] );
				difItem.SetDoubleAttribute( "PosX", pShape->posDiff[i][0] );
				difItem.SetDoubleAttribute( "PosY", pShape->posDiff[i][1] );
				difItem.SetDoubleAttribute( "PosZ", pShape->posDiff[i][2] );
				difItem.SetDoubleAttribute( "NorX", pShape->normalDiff[i][0] );
				difItem.SetDoubleAttribute( "NorY", pShape->normalDiff[i][1] );
				difItem.SetDoubleAttribute( "NorZ", pShape->normalDiff[i][2] );

				shapeItem.InsertEndChild( difItem );
			}

			modelItem.InsertEndChild( shapeItem );
		}

		head.InsertEndChild( modelItem );

		//
		// free mem
		FreeGeometryBlendshapes( blendshapes );
	}

	doc.InsertEndChild( head );
	doc.SaveFile(filename);

	if (doc.Error() )
	{
		printf( doc.ErrorDesc() );
		return false;
	}
	return true;
}

int	MakeIndexUnique( const std::vector<std::string> &names, const std::string &str, const int index )
{
	const size_t text_size = 128;
	char szText[text_size];
	sprintf_s( szText, text_size, "%s%d", str.c_str(), index );

	for (auto iter=names.cbegin(); iter!=names.cend(); ++iter)
	{
		if ( strcmp(iter->c_str(), szText) == 0)
		{
			return MakeIndexUnique( names, str, index+1 );
		}
	}

	return index;
}

void MakeNameUnique( const std::vector<std::string> &names, std::string &str )
{
	bool result = false;

	for (size_t i=0; i<names.size(); ++i)
	{
		const std::string &iName = names[i];

		if ( strcmp(iName.c_str(), str.c_str() ) == 0 )
		{
			// change str to make it unique
			
			const char *szText = str.c_str();
			const size_t len = str.length();

			int index = len-1;
			while (index >= 0)
			{
				char ch = szText[index];
				if ( isdigit(ch) == 0 )
				{
					break;
				}
				index--;
			}

			if (index < len-1)
			{
				index++;
				const size_t text_size = 128;
				char indexText[text_size];
				memset( &indexText[0], 0, sizeof(char) * text_size );

				for (int j=0; j<(len-index); ++j)
					indexText[j] = szText[index+j];
				
				int number=0;
				sscanf_s( indexText, "%d", &number );

				str = std::string( szText, index );
				index = MakeIndexUnique( names, str, number );

				sprintf_s( indexText, text_size, "%s%d", str.c_str(), index );
				str = indexText;
				
				result = true;
				break;
			}
		}
	}

	if (result == false)
	{
		str += " 1";
	}
}

bool	Blendshapes_LoadXML( FBModelList &modelList, const char *filename, const FBBlendShapeLoadMode mode )
{

	TiXmlDocument	doc;

	if (doc.LoadFile( filename ) == false)
	{
		return false;
	}

	TiXmlNode *node = nullptr;
	TiXmlElement *headElement = nullptr;
	TiXmlElement *modelElement = nullptr;
	TiXmlElement *shapeElement = nullptr;
	TiXmlElement *difElement = nullptr;
	TiXmlAttribute  *attrib = nullptr;

	node = doc.FirstChild("Header");
	if (node == nullptr)
	{
		return false;
	}

	headElement = node->ToElement();
	if (headElement)
	{
		modelElement = node->FirstChildElement("Model");
	}

	while (modelElement)
	{
		std::string str;
		int numberOfShapes=0;

		// enumerate attribs
		for( attrib = modelElement->FirstAttribute();
			 attrib;
			 attrib = attrib->Next() )
		{
			const char * attribName = attrib->Name();
			if ( strcmp(attribName, "name") == 0 )
				str = attrib->ValueStr();
			else if ( strcmp(attribName, "numberOfShapes") == 0 )
				numberOfShapes = attrib->IntValue();
		}

		FBModel *pModel = FBFindModelByLabelName( str.c_str() );
		if (pModel)
		{
			FBGeometry *pGeometry = pModel->Geometry;
			shapeElement = modelElement->FirstChildElement( "Shape" );

			if (mode == kFBShapeLoad)
			{
				pGeometry->ShapeClearAll();
			}

			//pGeometry->GeometryBegin();

			while(shapeElement)
			{

				str = "";
				int numberOfDiffs = 0;

				// enumerate attribs
				for( attrib = shapeElement->FirstAttribute();
					 attrib;
					 attrib = attrib->Next() )
				{
					const char * attribName = attrib->Name();
					if ( strcmp(attribName, "name") == 0 )
						str = attrib->ValueStr();
					else if ( strcmp(attribName, "numberOfDiffs") == 0 )
						numberOfDiffs = attrib->IntValue();
				}

				if (str != "")
				{
					int idx = -1;
					if (mode == kFBShapeMerge)
					{
						for (int i=0; i<pGeometry->ShapeGetCount(); ++i)
						{
							FBString shapeName( pGeometry->ShapeGetName(i) );
							if ( strcmp(shapeName, str.c_str() ) == 0 )
							{
								idx = i;
								break;
							}
						}
					}
					else if (mode == kFBShapeAppend)
					{
						// check if the name is unique
						std::vector< std::string > names;
						names.resize(pGeometry->ShapeGetCount());

						for (int i=0; i<pGeometry->ShapeGetCount(); ++i)
							names[i] = pGeometry->ShapeGetName(i);

						MakeNameUnique( names, str );
					}

					if (idx < 0) idx = pGeometry->ShapeAdd( str.c_str() );
					
					difElement = shapeElement->FirstChildElement( "Diff" );

					int OriIndex=0;
					double posX=0.0;
					double posY=0.0;
					double posZ=0.0;
					double norX=0.0;
					double norY=0.0;
					double norZ=0.0;

					pGeometry->ShapeInit(idx, numberOfDiffs, true);
					for (int j=0; j<numberOfDiffs; ++j)
					{
						// enumerate attribs
						for( attrib = difElement->FirstAttribute();
							 attrib;
							 attrib = attrib->Next() )
						{
							const char * attribName = attrib->Name();
							if ( strcmp(attribName, "OriIndex") == 0 )
								OriIndex = attrib->IntValue();
							else if ( strcmp(attribName, "PosX") == 0 )
								posX = attrib->DoubleValue();
							else if ( strcmp(attribName, "PosY") == 0 )
								posY = attrib->DoubleValue();
							else if ( strcmp(attribName, "PosZ") == 0 )
								posZ = attrib->DoubleValue();
							else if ( strcmp(attribName, "NorX") == 0 )
								norX = attrib->DoubleValue();
							else if ( strcmp(attribName, "NorX") == 0 )
								norY = attrib->DoubleValue();
							else if ( strcmp(attribName, "NorX") == 0 )
								norZ = attrib->DoubleValue();
						}

						pGeometry->ShapeSetDiffPoint( idx, j, OriIndex, FBVertex(posX, posY, posZ), FBNormal(norX, norY, norZ) );

						difElement = difElement->NextSiblingElement();
					}
				}

				shapeElement = shapeElement->NextSiblingElement();
			}

			//pGeometry->GeometryEnd();
			pGeometry->ModifyNotify();

			pModel->SetupPropertiesForShapes();
		}

		modelElement = modelElement->NextSiblingElement();
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//

BlendShapeDeformerConstraint *FindAttachedBlendShapeDeformer(FBModel *pModel)
{
	FBSystem	mSystem;
	FBScene *pScene = mSystem.Scene;
	const int numberOfConstraints = pScene->Constraints.GetCount();

	for (int i=numberOfConstraints-1; i>=0; --i)
	{
		FBConstraint *pConstraint = pScene->Constraints[i];
		if (FBIS( pConstraint, BlendShapeDeformerConstraint ) && pConstraint->ReferenceGetCount(0) > 0 && pConstraint->ReferenceGet(0) == pModel)
		{
			return (BlendShapeDeformerConstraint*) pConstraint;
		}
	}	

	return nullptr;
}

void SculptMode( FBModel *pFocusedObject, bool enter )
{
	FBSystem	mSystem;

	int lCnt = mSystem.Manipulators.GetCount();
	for( int i=0; i<lCnt; i++ )
	{
		FBString manipName( mSystem.Manipulators[i]->Name );

		if (manipName == ORMANIPASSOCIATION__LABEL)
		{
			ORManip_Sculpt* lORManip_Association = (ORManip_Sculpt*)mSystem.Manipulators[i];
			if (enter)
			{
				lORManip_Association->Active = true;
				lORManip_Association->AlwaysActive = true;
				lORManip_Association->DefaultBehavior = false;
				
				// DONE: add a deformer constraint
				InitTempDeformer( pFocusedObject,  lORManip_Association );
				//}
				//else
				//{
				//	lORManip_Association->AssignDeformerConstraint(nullptr);
				//	lORManip_Association->SetModel(pFocusedObject, nullptr);
				//}
			} 
			else
			{
				//TODO: apply mesh deformations ?!

				lORManip_Association->ApplyGeometryChanges();

				//
				lORManip_Association->Active = false;
				lORManip_Association->AlwaysActive = false;
				lORManip_Association->DefaultBehavior = true;

				lORManip_Association->AssignDeformerConstraint(nullptr);
				lORManip_Association->SetModel(nullptr, nullptr);

				// DONE: remove a deformer constraint
				FreeTempDeformer();
				/*
				FBScene *pScene = mSystem.Scene;
				const int numberOfConstraints = pScene->Constraints.GetCount();

				for (int i=numberOfConstraints-1; i>=0; --i)
				{
					FBConstraint *pConstraint = pScene->Constraints[i];
					if (FBIS( pConstraint, BlendShapeDeformer ) && ((BlendShapeDeformer*) pConstraint)->Temp == true )
					{
						pConstraint->Active = false;
						pConstraint->FBDelete();
					}
				}
				*/
			}
		}
		else
		{

			if( (enter == false) && (manipName == "Transformation") )
			{
				mSystem.Manipulators[i]->Active = true;
				mSystem.Manipulators[i]->AlwaysActive = true;
			}
			else
			{
				mSystem.Manipulators[i]->Active = false;
				mSystem.Manipulators[i]->AlwaysActive = false;
			}
		}
	}
	
}

void InitTempDeformer(FBModel *pModel, FBManipulator* pManipulator)
{
	if (pModel == nullptr)
		return;

	BlendShapeDeformerConstraint *pDeformerConstraint = nullptr;
	//if ( lORManip_Association->UseConstraint || pFocusedObject->IsDeformable )
	//{
	FBString constraintName("BlendShape Deformer ", pModel->Name);
					
	FreeTempDeformer();
					
	pDeformerConstraint = new BlendShapeDeformerConstraint( constraintName );
	pDeformerConstraint->FBCreate();
	pDeformerConstraint->Temp = true;
	gTempConstraint = pDeformerConstraint;
					
	pDeformerConstraint->ReferenceAdd(0, pModel);
	pDeformerConstraint->Active = true;

	if (pManipulator)
		( (ORManip_Sculpt*) pManipulator)->AssignDeformerConstraint(pDeformerConstraint);
}

void FreeTempDeformer()
{
	if (gTempConstraint)
	{
		gTempConstraint->Active = false;
		gTempConstraint->ReferenceRemoveAll();
		gTempConstraint->FBDestroy();
		delete gTempConstraint;
		gTempConstraint = nullptr;
	}
}

FBConstraint *GetTempDeformer()
{
	return gTempConstraint;
}

/////////////////////////////////////////////////////////////////////////////////////////
//

void FindUniqueBlendShapeName(FBModel *pModel, const char *defaultName, FBString &outName)
{
	FBGeometry *pGeom = pModel->Geometry;

	const int numberOfShapes = pGeom->ShapeGetCount();

	char buffer[128];
	memset(buffer, 0, sizeof(char) * 128);
	sprintf_s( buffer, 128, "%s\0", defaultName );

	int index=1;

	for (int i=0; i<numberOfShapes; ++i)
	{
		FBString shapeName( pGeom->ShapeGetName(i) );


		while( strcmp( shapeName, buffer ) == 0 )
		{
			// increase postfix
			sprintf_s( buffer, 128, "%s%d\0", defaultName, index );
			index++;
		}
	}

	outName = buffer;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//

void UnTransformBlendShapes( FBVertex &vertex, const int index, FBModel *pModel )
{
	FBGeometry *pGeometry = pModel->Geometry;

	int shapesCount = pGeometry->ShapeGetCount();

	for (int i=0; i<shapesCount; ++i)
	{
		int difCount = pGeometry->ShapeGetDiffPointCount(i);
		FBString shapeName = pGeometry->ShapeGetName(i);

		// find a intensity of a shape
		double lValue = 1.0;

		FBProperty *pProperty = pModel->PropertyList.Find( shapeName );
		if (pProperty)
		{
			pProperty->GetData( &lValue, sizeof(double) );
			lValue *= 0.01;
		}

		for (int j=0; j<difCount; ++j)
		{
			int oriIndex=0;
			FBVertex posDiff;
			FBNormal normalDiff;

			pGeometry->ShapeGetDiffPoint( i, j, oriIndex, posDiff, normalDiff );

			if (index == oriIndex)
			{
				vertex[0] -= lValue * posDiff[0];
				vertex[1] -= lValue * posDiff[1];
				vertex[2] -= lValue * posDiff[2];
				break;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// untransform skeletal cluster and all connected blendshapes

// function take modified vertices and normals and calcualte untransform difference to outVertices, outNormals
//  this result can be lately applied as a new model blendshape

void ComputeAllBlendshapesDifference( FBModel *pModel, FBVertex *diffVertices, FBNormal *diffNormals )
{
	FBGeometry *pGeometry = pModel->Geometry;

	int shapesCount = pGeometry->ShapeGetCount();

	for (int i=0; i<shapesCount; ++i)
	{
		int difCount = pGeometry->ShapeGetDiffPointCount(i);
		FBString shapeName = pGeometry->ShapeGetName(i);

		// find a intensity of a shape
		double lValue = 1.0;

		FBProperty *pProperty = pModel->PropertyList.Find( shapeName );
		if (pProperty)
		{
			pProperty->GetData( &lValue, sizeof(double) );
			lValue *= 0.01;
		}

		for (int j=0; j<difCount; ++j)
		{
			int oriIndex=0;
			FBVertex posDiff;
			FBNormal normalDiff;

			pGeometry->ShapeGetDiffPoint( i, j, oriIndex, posDiff, normalDiff );

			diffVertices[oriIndex][0] += lValue * posDiff[0];
			diffVertices[oriIndex][1] += lValue * posDiff[1];
			diffVertices[oriIndex][2] += lValue * posDiff[2];

			diffNormals[oriIndex][0] += lValue * normalDiff[0];
			diffNormals[oriIndex][1] += lValue * normalDiff[1];
			diffNormals[oriIndex][2] += lValue * normalDiff[2];
		}
	}
}

void CalculateDeltaMesh( FBModel *pModelBASE, const int numberOfVertices, const double *length, const FBVertex *vertices, const FBNormal *normals, FBVertex *outVertices, FBNormal *outNormals)
{
	// snapshot matrix
	FBMatrix InvTM;
	InvTM.Identity();

	pModelBASE->GetMatrix(InvTM, kModelInverse_Transformation_Geometry);

	//
	ClusterAdvance	clusterAdvance( nullptr );
	bool UseCluster = clusterAdvance.Init(pModelBASE);

	//
	// compute accumulated difference for all model blendshapes

	std::vector<FBVertex>	allDiffPositions;
	std::vector<FBNormal>	allDiffNormals;

	allDiffPositions.resize(numberOfVertices);
	allDiffNormals.resize(numberOfVertices);

	ComputeAllBlendshapesDifference( pModelBASE, allDiffPositions.data(), allDiffNormals.data() );

	//
	FBVertex v, vBASE;
	FBNormal n;
	FBMatrix m;

	for (int i=0; i<numberOfVertices; ++i)
	{
		if (length[i] <= 0.001)
			continue;

		v = vertices[i];
		n = normals[i];
		m.Identity();

		if (UseCluster) 
		{
			// TRANSFORM EDIT POSITION
			//FBVertexMatrixMult( v, InvTM, vertices[i] );
			
			FBMatrixInverse( m, clusterAdvance.CalculateDeformedPositionMatrix(i) );
			//FBVertexMatrixMult( v, m, v );
		}
		else
		{
			//FBVertexMatrixMult( v, InvTM, vertices[i] );
			m = InvTM;
		}
			

		// UNTRANSFORM SKELETAL ANIMATION

		//FBVertexMatrixMult( v, m, positionsEDIT[i] );
		FBVertexMatrixMult( v, m, v );

		// UNTRANSFORM BLEND SHAPES

		v[0] -= allDiffPositions[i][0];
		v[1] -= allDiffPositions[i][1];
		v[2] -= allDiffPositions[i][2];

		//UnTransformBlendShapes( v, i, pModelBASE );

		// untransform TM
		//FBVertexMatrixMult( v, InvTM, v );

		outVertices[i] = v;
		outNormals[i] = n;
		//pMeshDELTA->VertexNormalSet( normalsBASE[i], i );
	}
}


/////////////////////////////////////////////////////////////////////////////////


bool HasModelCluster(FBModel *pmodel)
{
	FBCluster *pcluster = pmodel->Cluster;

	if (nullptr == pcluster)
		return false;

	return true;
}

bool HasModelBlendshapes(FBModel *pModel)
{

	FBGeometry *pGeometry = pModel->Geometry;

	if (nullptr == pGeometry)
		return false;

	if (0 == pGeometry->ShapeGetCount() )
		return false;

	return true;
}