
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: GeometryUtils.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "GeometryUtils.h"
#include "math3d.h"
#include "ClusterAdvance.h"
#include "CmdFBX.h"

const char *g_szDefaultUVSet = "DefaultUVSet";

///////////////////////////////////////////////////////////////////////////////////////////////////
//

FBModel *MakeSnapshot(FBModel *pModel, const bool ResetXForm)
{
	FBGeometry *lGeom = pModel->Geometry;
	FBMesh *lMesh = (FBMesh*) lGeom;
	
	FBModelVertexData	*pVertexData = pModel->ModelVertexData;
	
	pVertexData->VertexArrayMappingRequest();
	
	int vertCountInMesh = lGeom->VertexCount();
//	int *indices = pVertexData->GetIndexArray();

	//unsigned int dubCount = 0;
	//const int *dubIndices = pVertexData->GetVertexArrayDuplicationMap( dubCount );

	FBVertex *positions = (FBVertex*) pVertexData->GetVertexArray( kFBGeometryArrayID_Point );
	FBNormal *normals = (FBNormal*) pVertexData->GetVertexArray( kFBGeometryArrayID_Normal );
	//FBUV	*uvs = (FBUV*) pVertexData->GetUVSetArray();

	pVertexData->VertexArrayMappingRelease();

	lMesh->GeometryBegin();

	int uvCount; // , uvIndexCount;

	FBUV	*uvs = lMesh->GetUVSetDirectArray(uvCount);
//	int *uvIndices = lMesh->GetUVSetIndexArray(uvIndexCount);
//	FBGeometryMappingMode uvMode = lMesh->GetUVSetMappingMode();
	
	lMesh->GeometryEnd();
	

	// dublicate a model with the geometry
	FBString	str(pModel->LongName, "_dublicate" );

	FBModel *pNewModel = new FBModel(str);

	FBProperty *lProp = pNewModel->PropertyCreate( "SnapshotTime", kFBPT_double, ANIMATIONNODE_TYPE_NUMBER, false, true );

	double value;
	FBTime lTime = FBSystem::TheOne().LocalTime;
	value = lTime.GetSecondDouble();

	lProp->SetData(&value);

	lProp = pNewModel->PropertyCreate( "BaseModel", kFBPT_charptr, ANIMATIONNODE_TYPE_STRING, false, true );
	lProp->SetString( pModel->LongName );

	FBMatrix m;
	pModel->GetMatrix(m);

	//
	// save TRS in snapshot properties
	if (ResetXForm == false)
	{
		FBTVector	pos;
		FBRVector	rot;
		FBSVector	scale;

		FBMatrixToTRS( pos, rot, scale, m );

		lProp = pNewModel->PropertyCreate( "SnapshotPosition", kFBPT_Vector4D, ANIMATIONNODE_TYPE_VECTOR_4, false, true );
		if (lProp) lProp->SetData( pos );

		lProp = pNewModel->PropertyCreate( "SnapshotRotation", kFBPT_Vector3D, ANIMATIONNODE_TYPE_VECTOR, false, true );
		if (lProp) lProp->SetData( rot );

		lProp = pNewModel->PropertyCreate( "SnapshotScale", kFBPT_Vector3D, ANIMATIONNODE_TYPE_VECTOR, false, true );
		if (lProp) lProp->SetData( scale );
	}

	for (int i=0; i<pModel->Materials.GetCount(); ++i)
		pNewModel->Materials.Add( pModel->Materials.GetAt(i) );

	FBMesh	*newMesh = new FBMesh( FBString(str, "_newMesh") );
	pNewModel->Geometry = newMesh;

	newMesh->GeometryBegin();
	{
		newMesh->VertexArrayInit(vertCountInMesh,false); 

		// Step 1: copy vertices
		FBVertex vertex;
		FBNormal normal;
		FBUV	uv;

		for (int i=0; i<vertCountInMesh; i++) 
		{

			//vertex = positions[i];
			normal = normals[i];
			uv = uvs[i];

			// Reset X-Form
			if (ResetXForm)
				FBVertexMatrixMult( vertex, m, positions[i] );
			else vertex = positions[i];

			newMesh->VertexSet( vertex, i );
			newMesh->VertexNormalSet(normal, i);
			newMesh->VertexUVSet( uv[0], uv[1], i );
		}
		
		// Step 2: copy polygons
		FBGeometryMappingMode mapping = static_cast<FBGeometryMappingMode>(lMesh->MaterialMappingMode.AsInt());
		int polyCount = lMesh->PolygonCount();
			
		for (int i=0; i<polyCount; ++i)
		{
			int vertCount = lMesh->PolygonVertexCount(i);
			int matId = 0;
			
			if (mapping == kFBGeometryMapping_BY_POLYGON)
			{
				lMesh->PolygonMaterialIdGet(i);
			}

			newMesh->PolygonBegin(matId);
			for (int j=0; j<vertCount; ++j)
			{
				newMesh->PolygonVertexAdd( lMesh->PolygonVertexIndex(i,j) );
			}
			newMesh->PolygonEnd();
		}
	}
	newMesh->GeometryEnd();
	newMesh->ModifyNotify();
	pNewModel->SetupPropertiesForShapes();

	// Step 3: finalize, prepare to display
	pNewModel->Show = true;
	// Adjust the shading mode.
	pNewModel->ShadingMode.SetPropertyValue(kFBModelShadingTexture);
		
	FBScene *pScene = FBSystem::TheOne().Scene;
	for (int i=0; i<pScene->Components.GetCount(); ++i)
		pScene->Components[i]->Selected = false;

	pNewModel->Selected = true;

	if (ResetXForm == false)
	{
		pNewModel->SetMatrix( m );
	}

	return pNewModel;
}

void FillInputModelData(FBModelList &modelList, InputModelData &data, FBArrayTemplate<FBMaterial*> &materialList, const bool DeformedPositions, const bool TransformedPositions)
{
	int vertCountInMesh = 0;
	int	materialIndexCount = 0;
	int polyCount = 0;
	int gPolyVertexCount = 0;

	int normalsDirectCount = 0;
	int normalsIndicesCount = 0;

	int uvDirectCount = 0;
	int uvIndicesCount = 0;

	int numberOfMaterials = 0;


//	int lmaterialIndexCount = 0;
		//int *lmaterialIndices = lMesh->GetMaterialIndexArray( lmaterialIndexCount );

		//data.materialIndices.resize( materialIndexCount + lmaterialIndexCount );
		//memcpy( &data.materialIndices[materialIndexCount], lmaterialIndices, sizeof(int) * lmaterialIndexCount );
		
	data.materialIndices.resize( materialList.GetCount() );
	for (int i=0; i<materialList.GetCount(); ++i)
	{
		data.materialIndices[i] = i;
	}
		
	//
	// pre allocate arrays
	//
	for (int nMesh=0; nMesh<modelList.GetCount(); ++nMesh)
	{
		FBModel *pModel = modelList[nMesh];
		FBString modelName(pModel->Name);
		printf ("modelName - %s\n", static_cast<const char*>(modelName) );

		FBGeometry *lGeom = pModel->Geometry;
		FBMesh *lMesh = (FBMesh*) lGeom;
//		FBModelVertexData	*pVertexData = pModel->ModelVertexData;

		const int lvertCountInMesh = lGeom->VertexCount();
		vertCountInMesh += lvertCountInMesh;

		const int lpolyCount = lMesh->PolygonCount();
		polyCount += lpolyCount;

		int polyVertexCount;
		lMesh->PolygonVertexArrayGet(polyVertexCount); // int *polyVertexIndices = (int*) 

		int lnormalDirectCount; // , lnormalIndicesCount;
		lMesh->GetNormalsDirectArray( lnormalDirectCount ); // float *normals = (float*) 
		//int *normalIndices = lMesh->GetNormalsIndexArray( lnormalIndicesCount );

		// Important! For indices I will use polygon vertex mapping type, so indices count == poly vertices count

		normalsDirectCount += lnormalDirectCount;
		normalsIndicesCount += polyVertexCount;

		//
		// copy uvset

		FBStringList uvSets = lMesh->GetUVSets();

		if (uvSets.GetCount() )
		{
			int luvDirectCount; // , luvIndicesCount;

			lMesh->GetUVSetDirectArray( luvDirectCount ); // FBUV *uvs =
			//int *uvIndices = lMesh->GetUVSetIndexArray( luvIndicesCount );

			uvDirectCount += luvDirectCount;
			uvIndicesCount += polyVertexCount;
		}
	}

	data.positions.resize( vertCountInMesh );
	data.polyInfo.resize( polyCount );
	data.normalsDirect.resize( normalsDirectCount );
	data.normalIndices.resize( normalsIndicesCount );
	data.uvs.resize( uvDirectCount );
	data.uvIndices.resize( uvIndicesCount );

	vertCountInMesh = 0;
	polyCount = 0;
	normalsDirectCount = 0;
	normalsIndicesCount = 0;
	uvDirectCount = 0;
	uvIndicesCount = 0;

	for (int nMesh=0; nMesh<modelList.GetCount(); ++nMesh)
	{
		FBModel *pModel = modelList[nMesh];
		FBString modelName(pModel->Name);
		printf ("modelName - %s\n", static_cast<const char*>(modelName) );

		FBGeometry *lGeom = pModel->Geometry;
		FBMesh *lMesh = (FBMesh*) lGeom;
		FBModelVertexData	*pVertexData = pModel->ModelVertexData;

		//
		// query deformed mesh vertex positions

		pVertexData->VertexArrayMappingRequest();
		
		const int lvertCountInMesh = lGeom->VertexCount();
		//data.positions.resize( vertCountInMesh + lvertCountInMesh );

		FBVertex *positions = (FBVertex*) pVertexData->GetVertexArray( kFBGeometryArrayID_Point, DeformedPositions );
		FBVertex *lposition = (FBVertex*) &data.positions[vertCountInMesh].x[0];
		
		if ( TransformedPositions && (pModel->Deformers.GetCount() == 0) )
		{
			FBMatrix m;
			pModel->GetMatrix( m, kModelTransformation_Geometry );

			// transform vertices
			
			for (int i=0; i<lvertCountInMesh; ++i)
			{	
				FBVertexMatrixMult( lposition[i], m, positions[i] );
			}
		}
		else
		{
			memcpy( lposition, positions, sizeof(FBVertex)*lvertCountInMesh );
		}

		pVertexData->VertexArrayMappingRelease();

		//
		// material mapping

		data.materialMapping = lMesh->MaterialMappingMode.AsInt();
		
		int lmaterialIndexCount = 0;
		//int *lmaterialIndices = lMesh->GetMaterialIndexArray( lmaterialIndexCount );

		//data.materialIndices.resize( materialIndexCount + lmaterialIndexCount );
		//memcpy( &data.materialIndices[materialIndexCount], lmaterialIndices, sizeof(int) * lmaterialIndexCount );
		/*
		for (int i=0; i<lmaterialIndexCount; ++i)
		{
			// SHIFT
			FBMaterial *pMaterial = pModel->Materials[ lmaterialIndices[i] ];
			data.materialIndices[materialIndexCount + i] = materialList.Find(pMaterial);
		}
		*/

		

		//
		// polygon indices

		const int lpolyCount = lMesh->PolygonCount();

		int ndx=0;
		int polyVertexCount;
		int *polyVertexIndices = (int*) lMesh->PolygonVertexArrayGet(polyVertexCount);

		//data.polyInfo.resize( polyCount + lpolyCount );

		for (int i=0; i<lpolyCount; ++i)
		{
			int lpolyVertexCount = lMesh->PolygonVertexCount(i);
			_ASSERT( lpolyVertexCount > 0 );

			InputModelData::PolyInfo &info = data.polyInfo[polyCount + i];

			info.vertexCount = lpolyVertexCount;

			// SHIFT material index
			info.materialId = lMesh->PolygonMaterialIdGet(i);
			if ( info.materialId >=0 ) 
			{
				if ( (pModel->Materials.GetCount() > 0) && (info.materialId < pModel->Materials.GetCount()) )
				{
					FBMaterial *pMaterial = pModel->Materials[info.materialId];
					info.materialId = materialList.Find(pMaterial);
				}
			}
			else
			{
				printf( "unset material id for poligon\n" );
				
				if (pModel->Materials.GetCount() > 0)
					info.materialId = materialList.Find( pModel->Materials[0] );
			}

			info.indices = new int[lpolyVertexCount];
			_ASSERT( info.indices != nullptr );

			for (int j=0; j<lpolyVertexCount; ++j)
			{
				// SHIFT indices with vertCountInMesh (start vertex index for this mesh)
				_ASSERT( ndx < polyVertexCount );
				info.indices[j] = vertCountInMesh + polyVertexIndices[ndx];
				ndx++;
			}
		}
		polyCount += lpolyCount;

		//
		// normals info

		FBGeometryMappingMode lNormalMapping = static_cast<FBGeometryMappingMode>(lMesh->NormalMappingMode.AsInt());
		FBGeometryReferenceMode lNormalReferenceMode = static_cast<FBGeometryReferenceMode>(lMesh->NormalReferenceMode.AsInt());

		data.normalMapping = (int) kFBGeometryMapping_BY_POLYGON_VERTEX;
		data.normalReferenceMode = (int) kFBGeometryReference_INDEX_TO_DIRECT;

		int lnormalDirectCount, lnormalIndicesCount;
		float *normals = (float*) lMesh->GetNormalsDirectArray( lnormalDirectCount );
		int *normalIndices = lMesh->GetNormalsIndexArray( lnormalIndicesCount );
		
		//data.normalsDirect.resize( normalsDirectCount + lnormalDirectCount );
		//data.normalIndices.resize( normalsIndicesCount + lnormalIndicesCount);

		memcpy( &data.normalsDirect[normalsDirectCount], normals, sizeof(Float4) * lnormalDirectCount );

		/*
		for (int i=0; i<lnormalIndicesCount; ++i)
		{
			// SHIFT indices
			data.normalIndices[normalsIndicesCount + i] = vertCountInMesh + normalIndices[i];
		}
		*/
		//memcpy( &data.normalIndices[normalsIndicesCount], normalIndices, sizeof(int) * lnormalIndicesCount );

		// convert mapping mode to specified in combined mesh (polygon vertex)
		switch (lNormalMapping)
		{
		case kFBGeometryMapping_BY_CONTROL_POINT:

			switch(lNormalReferenceMode)
			{
			case kFBGeometryReference_DIRECT:
			
				//assert( lnormalDirectCount == polyVertexCount );

				lnormalIndicesCount = polyVertexCount;
				//data.uvIndices.resize( uvIndicesCount + luvIndicesCount);

				for (int i=0; i<lnormalIndicesCount; ++i)
				{
					// SHIFT
					data.normalIndices[normalsIndicesCount + i] = normalsDirectCount + i;
				}
			break;

			default:
				printf( "unsupported normal reference!\n" );
			}

			break;

		case kFBGeometryMapping_BY_POLYGON_VERTEX:

			if (lNormalReferenceMode == kFBGeometryReference_INDEX_TO_DIRECT)
			{
				//data.uvIndices.resize( uvIndicesCount + luvIndicesCount);

				for (int i=0; i<lnormalIndicesCount; ++i)
				{
					// SHIFT
					data.normalIndices[normalsIndicesCount + i] = normalsDirectCount + normalIndices[i];
				}
			}
			else if (lNormalReferenceMode == kFBGeometryReference_DIRECT )
			{
				// do nothing with indices, only direct normal array
				lnormalIndicesCount = polyVertexCount;
				//data.uvIndices.resize( uvIndicesCount + luvIndicesCount);

				for (int i=0; i<lnormalIndicesCount; ++i)
				{
					// SHIFT
					data.normalIndices[normalsIndicesCount + i] = normalsDirectCount + i;
				}
			}
			else
			{
				printf( "unsupported normal mapping mode\n" );
			}

			break;

		default:
			printf("unsupported model mapping!\n");
		}

		normalsDirectCount += lnormalDirectCount;
		normalsIndicesCount += lnormalIndicesCount;

		//
		// transformation matrix

		FBMatrix TM;
		pModel->GetMatrix(TM);
		FBTVector pos;
		FBRVector rot;
		FBSVector scale;

		FBMatrixToTRS( pos, rot, scale, TM );

		memcpy( data.LclPosition, pos, sizeof(double)*3 );
		memcpy( data.LclRotation, rot, sizeof(double)*3 );
		memcpy( data.LclScaling, scale, sizeof(double)*3 );

		//
		// copy uvset

		FBStringList uvSets = lMesh->GetUVSets();

		data.uvSetMapping = (int) kFBGeometryMapping_BY_POLYGON_VERTEX;
		data.uvSetReferenceMode = (int) kFBGeometryReference_INDEX_TO_DIRECT;

		if (uvSets.GetCount() )
		{
			//data.uvSetName = g_szDefaultUVSet;
			data.SetUVSetName( g_szDefaultUVSet );

			FBGeometryMappingMode lMappingMode = lMesh->GetUVSetMappingMode();
			FBGeometryReferenceMode lRefMode = lMesh->GetUVSetReferenceMode();

			int luvDirectCount, luvIndicesCount;

			FBUV *uvs = lMesh->GetUVSetDirectArray( luvDirectCount );
			int *uvIndices = lMesh->GetUVSetIndexArray( luvIndicesCount );

			//data.uvs.resize( uvDirectCount + luvDirectCount );
			memcpy( &data.uvs[uvDirectCount], uvs, sizeof(FBUV) * luvDirectCount );

			// convert mapping mode to specified in combined mesh (polygon vertex)
			switch (lMappingMode)
			{
			case kFBGeometryMapping_BY_CONTROL_POINT:

				switch(lRefMode)
				{
				case kFBGeometryReference_DIRECT:
			
					//assert( luvDirectCount == polyVertexCount );

					luvIndicesCount = polyVertexCount;
					//data.uvIndices.resize( uvIndicesCount + luvIndicesCount);

					for (int i=0; i<luvIndicesCount; ++i)
					{
						// SHIFT
						data.uvIndices[uvIndicesCount + i] = uvDirectCount + i;
					}
				break;

				default:
					printf( "unsupported uv reference!\n" );
				}

				break;

			case kFBGeometryMapping_BY_POLYGON_VERTEX:

				if (lRefMode == kFBGeometryReference_INDEX_TO_DIRECT)
				{
					//data.uvIndices.resize( uvIndicesCount + luvIndicesCount);

					for (int i=0; i<luvIndicesCount; ++i)
					{
						// SHIFT
						data.uvIndices[uvIndicesCount + i] = uvDirectCount + uvIndices[i];
					}
				}
				else
				{
					printf( "unsupported uv mapping mode\n" );
				}

				break;

			default:
				printf("unsupported model mapping!\n");
			}

			uvDirectCount += luvDirectCount;
			uvIndicesCount += luvIndicesCount;
		}

		//
		// clusters information
		/*
		FBCluster *pCluster = pModel->Cluster;

		if (pCluster && pCluster->LinkGetCount())
		{
			FBString linkName;
			FBModel* linkModel;
			FBModel		*linkAssociateModel;
			int numLinks, numVerts, vertIndex;
			double vertWeight;

			FBMatrix m, m2, tm, tmInvTranspose, pLM;
			FBTVector pos;
			FBRVector rot, pDof;
			FBSVector scale;
		
			numLinks = pCluster->LinkGetCount();

			for (int n=0; n < numLinks; n++) 
			{
				

				pCluster->ClusterBegin(n);			// Set the current cluster index

				linkName = pCluster->LinkGetName(n);
				linkModel = pCluster->LinkGetModel(n);

				if (linkModel == nullptr)
				{
					continue;
				}
				
				InputModelData::Cluster		*newCluster = new InputModelData::Cluster;

				sprintf_s( newCluster->name, STRING_MAX_LENGTH, "%s", (char*) linkName );
				sprintf_s( newCluster->modelname, STRING_MAX_LENGTH, "%s", (char*) (FBString) linkModel->Name );

				auto iter = data.clusters.find( newCluster );
				if (iter != data.clusters.end() )
				{
					delete newCluster;
					newCluster = *iter;
				}
				else
				{
					linkAssociateModel = pCluster->LinkGetAssociateModel(n);
					FBClusterMode mode = pCluster->ClusterMode;
				
					newCluster->mode = (int) mode;

					FBVector3d temp;
					pCluster->VertexGetTransform(temp, rot, scale);
				
					memcpy(newCluster->LinkPosition, temp, sizeof(double)*3 );
					memcpy(newCluster->LinkRotation, rot, sizeof(double)*3 );
					memcpy(newCluster->LinkScaling, scale, sizeof(double)*3 );

					FBString linkModelName = linkModel->Name;
					linkModel->GetVector( temp, kModelTranslation );
					linkModel->GetVector( rot, kModelRotation );
					linkModel->GetVector( scale, kModelScaling );
				
					memcpy(newCluster->LclPosition, temp, sizeof(double)*3 );
					memcpy(newCluster->LclRotation, rot, sizeof(double)*3 );
					memcpy(newCluster->LclScaling, scale, sizeof(double)*3 );
				}

				numVerts = pCluster->VertexGetCount();	// Using the current cluster index
				for (int v=0; v < numVerts; v++) 
				{
					vertIndex = pCluster->VertexGetNumber(v);		// Using the current cluster index
					vertWeight = pCluster->VertexGetWeight(v);	// Using the current cluster index

					//vertices[vertIndex].AddVertexLink( tm, tmInvTranspose, vertWeight, mode );

					// TODO: increase index by model vertex count !

					//! SHIFT vertex index
					newCluster->vertices.push_back( InputModelData::DeformedVertex( vertCountInMesh + vertIndex, vertWeight) );
				}
				pCluster->ClusterEnd();		

				//
				//
				data.clusters.insert( newCluster );
			}
		}
		*/
		//
		// finally increase vertex counter for the next mesh

		vertCountInMesh += lvertCountInMesh;
		gPolyVertexCount += polyVertexCount;
		materialIndexCount += lmaterialIndexCount;
		numberOfMaterials += pModel->Materials.GetCount();
	}
}

bool DoNeedUniqueName()
{
	FBConfigFile	lFile( "@BlendShapeTookit.txt", true );
	
	FBString strDefault( "true" );
	const char *szDefault = strDefault;
	lFile.GetOrSet( "Mesh Operations", "UniqueName", szDefault, "Do you want to replace snapshot each time or create a new unique one?" );

	return ( strstr(szDefault, "true") != nullptr );
}

FBModel *MakeSnapshot2( FBModel *pModel, const bool ResetXForm, const bool CopyShaders )
{
	//
	//

	FBArrayTemplate<FBMaterial*>	materialList;
	materialList.SetCount( pModel->Materials.GetCount() );
	
	for (int i=0; i<pModel->Materials.GetCount(); ++i)
		materialList[i] = pModel->Materials[i];
			
	//
	InputModelData data;

	FBModelList modelList;
	modelList.Add(pModel);

	FillInputModelData( modelList, data, materialList, true, false );

	//
	//
	//
	FBTime localTime = FBSystem::TheOne().LocalTime;

	FBString configPath = FBSystem().TempPath.AsString();
	FBString filename(configPath, "\\model.fbx");
	
	//data.baseModelName = pModel->LongName;
	data.SetBaseModelName( pModel->LongName );
	data.snapshotTime = localTime.GetSecondDouble();

	// DONE: prepare unique name
	FBString snapshotName (pModel->LongName, "_Snapshot");

	if (DoNeedUniqueName() )
	{
		// generate unique name
		FBString strBase( snapshotName );

		const int size_text=128;
		char text[size_text] = "";

		int idx = 0;
		while (true)
		{
			sprintf_s( text, size_text, "_%.4d", idx );
			snapshotName = strBase + text;

			if (FBFindModelByLabelName( snapshotName ) == nullptr)
				break;

			idx++;
		}
	}

	if (CmdMakeSnapshotFBX_Send(filename, snapshotName, data, ResetXForm) )
	{
		FBApplication::TheOne().FileMerge( filename );

		FBModel *pNewModel = FBFindModelByLabelName( snapshotName );
		if (pNewModel)
		{
			for (int i=0; i<pModel->Materials.GetCount(); ++i)
				pNewModel->Materials.Add( pModel->Materials[i] );

			// copy shader settings
			if (CopyShaders)
			{
				for (int i=0; i<pModel->Shaders.GetCount(); ++i)
					pNewModel->Shaders.Add( pModel->Shaders[i] );
			}

			FBProperty *lProp = pNewModel->PropertyCreate( "SnapshotTime", kFBPT_double, ANIMATIONNODE_TYPE_NUMBER, false, true );

			double value = localTime.GetSecondDouble();

			lProp->SetData(&value);

			lProp = pNewModel->PropertyCreate( "BaseModel", kFBPT_charptr, ANIMATIONNODE_TYPE_STRING, false, true );
			lProp->SetString( pModel->LongName );

			FBMatrix m;
			pModel->GetMatrix(m);

			//
			// save TRS in snapshot properties
			if (ResetXForm == false)
			{
				FBTVector	pos;
				FBRVector	rot;
				FBSVector	scale;

				FBMatrixToTRS( pos, rot, scale, m );

				lProp = pNewModel->PropertyCreate( "SnapshotPosition", kFBPT_Vector4D, ANIMATIONNODE_TYPE_VECTOR_4, false, true );
				if (lProp) lProp->SetData( pos );

				lProp = pNewModel->PropertyCreate( "SnapshotRotation", kFBPT_Vector3D, ANIMATIONNODE_TYPE_VECTOR, false, true );
				if (lProp) lProp->SetData( rot );

				lProp = pNewModel->PropertyCreate( "SnapshotScale", kFBPT_Vector3D, ANIMATIONNODE_TYPE_VECTOR, false, true );
				if (lProp) lProp->SetData( scale );
			}

			pNewModel->Selected = true;
		}
	}

	//
	// finalyze, import back to mobu a snapshot model
	//
	FBPlayerControl::TheOne().Goto( localTime );

	return nullptr;
}




FBModel *CombineModels(FBModelList &inList)
{

	// keep only models with geometry
	FBModelList combineList;

	for (int i=0, count=inList.GetCount(); i<count; ++i)
	{
		FBModel *pModel = inList[i];
	
		if ( FBIS(pModel, FBLight) || FBIS(pModel, FBCamera) || FBIS(pModel, FBModelNull) || FBIS(pModel, FBModelOptical) || FBIS(pModel, FBModelMarker) || FBIS(pModel, FBModelMarkerOptical) )
			continue;

		FBGeometry *pGeometry = pModel->Geometry;
		if (pGeometry == nullptr) continue;

		const int numVerts = pGeometry->VertexCount();
		if (numVerts == 0) continue;

		FBMesh *pMesh = (FBMesh*) pGeometry;
		const int numPolys = pMesh->PolygonCount();
		if (numPolys == 0) continue;

		combineList.Add(pModel);
	}

	if (combineList.GetCount() == 0)
	{
		return nullptr;
	}

	//
	// compute combined material list
	//

	FBArrayTemplate<FBMaterial*>	materialList;
	FBArrayTemplate<int>			materialCorr;

	for (int i=0, count=combineList.GetCount(); i<count; ++i)
	{
		FBModel *pModel = combineList[i];
		for (int j=0; j<pModel->Materials.GetCount(); ++j)
		{
			FBMaterial *pMaterial = pModel->Materials[j];

			int idx = materialList.Find(pMaterial);
			if (idx == -1)
			{
				idx = materialList.Add(pMaterial);
			}

			materialCorr.Add(idx);
		}
	}


	//
	//

	InputModelData data;

	FillInputModelData( combineList, data, materialList, false, true );
	
	//
	//
	//
	FBTime localTime = FBSystem::TheOne().LocalTime;

	FBString configPath = FBSystem().TempPath.AsString();
	FBString filename(configPath, "\\model.fbx");

	FBString CombinedName ( "Combined");

	if (DoNeedUniqueName() )
	{
		// generate unique name
		FBString strBase( CombinedName );

		const int size_text=128;
		char text[size_text] = "";

		int idx = 0;
		while (true)
		{
			sprintf_s( text, size_text, "_%.4d", idx );
			CombinedName = strBase + text;

			if (FBFindModelByLabelName( CombinedName ) == nullptr)
				break;

			idx++;
		}
	}

	if (CmdMakeSnapshotFBX_Send(filename, CombinedName, data, false) )
	{
		FBApplication::TheOne().FileMerge( filename );

		FBModel *pNewModel = FBFindModelByLabelName( CombinedName );
		if (pNewModel)
		{
			
			for (int i=0; i<materialList.GetCount(); ++i)
			{
				pNewModel->Materials.Add( materialList.GetAt(i) );
			}

			SkinCopy( combineList, pNewModel );
			pNewModel->SkeletonDeformable = true;

			pNewModel->Selected = true;
			FBMatrix tm;
			tm.Identity();
			pNewModel->SetMatrix( tm );
		}
	}
	else
	{
		FBMessageBox( "Combine Models", "Failed to process fbx file", "Ok" );
	}

	//
	// finalyze, import back to mobu a snapshot model
	//
	FBPlayerControl::TheOne().Goto( localTime );

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

void CenterPivot( FBModel *pModel )
{
	FBMatrix tm;
	FBVector3d newCenter, currCenter, bb[2];

	pModel->GetMatrix( tm, kModelTransformation_Geometry );
	pModel->GetBoundingBox( bb[0], bb[1] );
	VectorTransform( bb[0], tm, bb[0] );
	VectorTransform( bb[1], tm, bb[1] );
	pModel->GetVector( currCenter );

	newCenter = VectorCenter( bb, 2 );
	
	
	tm.Identity();

	tm[12] = -(newCenter[0] - currCenter[0]);
	tm[13] = -(newCenter[1] - currCenter[1]);
	tm[14] = -(newCenter[2] - currCenter[2]);

	//
	// freeze translation on vertices
	//
	FBGeometry *pGeometry = pModel->Geometry;

	if (pGeometry)
	{
		int numVerts = pGeometry->VertexCount();

		pGeometry->GeometryBegin();

		FBVertex *vertices = pGeometry->GetVertexes();

		for (int i=0; i<numVerts; ++i)
		{
			FBVertexMatrixMult( vertices[i], tm, vertices[i] );
		}

		pGeometry->GeometryEnd();
		pGeometry->ModifyNotify();
	}

	pModel->SetVector( newCenter );
}

void ReComputeNormals(FBModel *pModel)
{
	FBGeometry *pGeometry = pModel->Geometry;
	FBMesh *pMesh = (FBMesh*) pGeometry;

	if (pGeometry)
	{	
		pMesh->ComputeVertexNormals();
	}
}

void InvertNormals(FBModel *pModel)
{
	FBGeometry *pGeometry = pModel->Geometry;
//	FBMesh *pMesh = (FBMesh*) pGeometry;

	if (pGeometry)
	{	
		pGeometry->GeometryBegin();

		int normalsCount = 0;
		FBNormal *pNormals = pGeometry->GetNormalsDirectArray( normalsCount );

		for (int i=0; i<normalsCount; ++i)
			pNormals[i] = FBNormal( -1.0 * pNormals[i][0], -1.0*pNormals[i][1], -1.0*pNormals[i][2], pNormals[i][3] );

		pGeometry->GeometryEnd();
	}
}