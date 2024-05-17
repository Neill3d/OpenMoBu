
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: FBXUtils.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "FBXUtils.h"
#include "fbxsdk.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//


void InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene);
void DestroySdkObjects(FbxManager* pManager, bool pExitStatus);
void CreateAndFillIOSettings(FbxManager* pManager);

bool SaveScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename, int pFileFormat=-1, bool pEmbedMedia=false);
bool LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename);


////////////////////////////////////////////////////////////////////////////////////
//
// Make a snapshot using FBX SDK (helps to make needed number of uvs for each vertex)
//  generate fbx file and then import it into mobu scene as a snapshot
//
////////////////////////////////////////////////////////////////////////////////////

/*
FbxGeometryElement::EMappingMode ConvertORMappingToFBX(const FBGeometryMappingMode mapping)
{
	switch (mapping)
	{
	case kFBGeometryMapping_ALL_SAME:			return FbxGeometryElement::eAllSame;
	case kFBGeometryMapping_BY_CONTROL_POINT:	return FbxGeometryElement::eByControlPoint;
	case kFBGeometryMapping_BY_EDGE:			return FbxGeometryElement::eByEdge;
	case kFBGeometryMapping_BY_POLYGON:			return FbxGeometryElement::eByPolygon;
	case kFBGeometryMapping_BY_POLYGON_VERTEX:	return FbxGeometryElement::eByPolygonVertex;
	case kFBGeometryMapping_NONE:
		break;
	}

	return FbxGeometryElement::eAllSame;
}

FbxGeometryElement::EReferenceMode	ConvertORReferenceModeToFBX(const FBGeometryReferenceMode refMode)
{
	switch (refMode)
	{
	case kFBGeometryReference_DIRECT:		return FbxGeometryElement::eDirect;
	case kFBGeometryReference_INDEX:		return FbxGeometryElement::eIndex;
	case kFBGeometryReference_INDEX_TO_DIRECT:	return FbxGeometryElement::eIndexToDirect;
	}

	return FbxGeometryElement::eIndexToDirect;
}
*/

// DONE: support clusters !
// Snapshot of a pModel
FbxNode* CreateSnapshot(FbxScene* pScene, const char* pName, const InputModelData &data, const bool clusters)
{
	
	// create the main structure.
    FbxMesh* lMesh = FbxMesh::Create(pScene,"");
	
    // Create control points.
	int vertCountInMesh = data.positions.size();
    lMesh->InitControlPoints(vertCountInMesh);
    FbxVector4* vertex = lMesh->GetControlPoints();
	
	for (int i=0; i<vertCountInMesh; ++i)
	{
		vertex[i].Set( data.positions[i].x[0], data.positions[i].x[1], data.positions[i].x[2] );
	}
    //memcpy((void*)vertex, (void*)positions, vertCountInMesh*sizeof(FbxVector4));

    // create the materials.
    /* Each polygon face will be assigned a unique material.
    */
   
	const int materialIndexCount = data.materialIndices.size();
	if (materialIndexCount > 0)
	{
		FbxGeometryElementMaterial* lMaterialElement = lMesh->CreateElementMaterial();
   
		lMaterialElement->SetMappingMode( FbxGeometryElement::eByPolygon );
		lMaterialElement->SetReferenceMode( FbxGeometryElement::eIndexToDirect );
	
		for (int i=0; i<materialIndexCount; ++i)
			lMaterialElement->GetIndexArray().Add( i );
	}

	/*
	for (int i=0; i<data.polyInfo.size(); ++i)
		lMaterialElement->GetDirectArray().Add( data.polyInfo[i].materialId );
	*/
	//lMaterialElement->GetIndexArray().Add(0);

    // Create polygons later after FbxGeometryElementMaterial is created. Assign material indices.

	// Step 2: copy polygons
	int polyCount = data.polyInfo.size();
	for (int i=0; i<polyCount; ++i)
	{
		int vertCount = data.polyInfo[i].vertexCount;
		int matId = -1;
			
		
		//if (data.materialMapping == FbxGeometryElement::eByPolygon)
		//{
		matId = data.polyInfo[i].materialId;
		//}
	
		if (matId < 0)
		{
			//printf( "empty material id from polygon\n" );
		}
		

		lMesh->BeginPolygon( matId );
		for (int j=0; j<vertCount; ++j)
		{
			lMesh->AddPolygon( data.polyInfo[i].indices[j] );
		}
		lMesh->EndPolygon();
	}


    // specify normals per control point.
    FbxGeometryElementNormal* lNormalElement = lMesh->CreateElementNormal();
	lNormalElement->SetMappingMode( FbxGeometryElement::EMappingMode(data.normalMapping) );
	lNormalElement->SetReferenceMode( FbxGeometryElement::EReferenceMode(data.normalReferenceMode) );

	int normalDirectCount = data.normalsDirect.size();
	for (int i=0; i<normalDirectCount; ++i)
		lNormalElement->GetDirectArray().Add( FbxVector4(data.normalsDirect[i].x[0], data.normalsDirect[i].x[1], data.normalsDirect[i].x[2]) );

	int normalIndicesCount = data.normalIndices.size();
	for (int i=0; i<normalIndicesCount; ++i)
		lNormalElement->GetIndexArray().Add( data.normalIndices[i] );


    // Create the node containing the mesh
    FbxNode* lNode = FbxNode::Create(pScene,pName);

	
	lNode->LclTranslation.Set( FbxDouble3(data.LclPosition[0], data.LclPosition[1], data.LclPosition[2]) );
	lNode->LclRotation.Set( FbxDouble3(data.LclRotation[0], data.LclRotation[1], data.LclRotation[2]) );
	lNode->LclScaling.Set( FbxDouble3(data.LclScaling[0], data.LclScaling[1], data.LclScaling[2]) );

    lNode->SetNodeAttribute(lMesh);
    lNode->SetShadingMode(FbxNode::eTextureShading);   
    
    // create UVset

	if (data.uvSetName != nullptr)
	{
		FbxGeometryElementUV* lUVElement1 = lMesh->CreateElementUV( data.uvSetName );
		FBX_ASSERT( lUVElement1 != NULL);

		lUVElement1->SetMappingMode( FbxGeometryElement::EMappingMode(data.uvSetMapping) );
		lUVElement1->SetReferenceMode( FbxGeometryElement::EReferenceMode(data.uvSetReferenceMode) );
		
		int uvDirectCount = data.uvs.size();
		for (int j=0; j<uvDirectCount; ++j)
			lUVElement1->GetDirectArray().Add( FbxVector2(data.uvs[j].x[0], data.uvs[j].x[1]) );

		int uvIndicesCount = data.uvIndices.size();
		for (int j=0; j<uvIndicesCount; ++j)
			lUVElement1->GetIndexArray().Add(data.uvIndices[j]);
	}

    return lNode;
}

bool CreateScene(FbxManager *pSdkManager, FbxScene* pScene, const char *snapshotName, InputModelData &data)
{
    // create scene info
    FbxDocumentInfo* sceneInfo = FbxDocumentInfo::Create(pSdkManager,"SceneInfo");
    sceneInfo->mTitle = "Snapshot scene";
    sceneInfo->mSubject = "Perform correct snapshot operation using FBX SDK.";
    sceneInfo->mAuthor = "Sergey Solohin (Neill3d)";
    sceneInfo->mRevision = "rev. 1.0";
    sceneInfo->mKeywords = "snapshot geometry mesh uvs";
    sceneInfo->mComment = "This is a temproary scene for import purpose only";

    // we need to add the sceneInfo before calling AddThumbNailToScene because
    // that function is asking the scene for the sceneInfo.
    pScene->SetSceneInfo(sceneInfo);

    //AddThumbnailToScene(pScene);

	FbxNode* lSnapshot = CreateSnapshot(pScene, snapshotName, data, false);

	/*
	FbxSkin *lSkin = FbxSkin::Create(pScene, "");

	// create all connected bones
	int numBones = data.clusters.size();
	
	for( auto iter=data.clusters.cbegin(); iter!=data.clusters.end(); ++iter )
	{
		const InputModelData::Cluster *srcCluster = *iter;

		FbxNode *skeleton_node = FbxNode::Create( pScene, srcCluster->modelname );
		skeleton_node->LclTranslation.Set( FbxVector4( srcCluster->LclPosition[0], srcCluster->LclPosition[1], srcCluster->LclPosition[2] ) );
		skeleton_node->LclRotation.Set( FbxVector4( srcCluster->LclRotation[0], srcCluster->LclRotation[1], srcCluster->LclRotation[2] ) );
		skeleton_node->LclScaling.Set( FbxVector4( srcCluster->LclScaling[0], srcCluster->LclScaling[1], srcCluster->LclScaling[2] ) );

		FbxSkeleton *skeleton_attribute = FbxSkeleton::Create( pScene, srcCluster->modelname );
		skeleton_attribute->SetSkeletonType( FbxSkeleton::eLimbNode );
		
		// TODO: add child
		
		skeleton_node->SetNodeAttribute( skeleton_attribute );

		FbxCluster *cluster = FbxCluster::Create( pScene, srcCluster->name );
		cluster->SetLink( skeleton_node );
		cluster->SetLinkMode( (FbxCluster::ELinkMode) srcCluster->mode );

		// Set proper transformation for each cluster and add them to the skin deformer.
		// ThansformMatrix is the global transformation of the mesh when the binding happens.
		// TransformLinkMatrix is the global transformation of the joint(Link) when the binding happens.

		FbxAMatrix	matrix( FbxVector4(data.LclPosition[0], data.LclPosition[1], data.LclPosition[2]),
			FbxVector4(data.LclRotation[0], data.LclRotation[1], data.LclRotation[2]),
			FbxVector4(data.LclScaling[0], data.LclScaling[1], data.LclScaling[2]) );
		//matrix.SetIdentity();
		cluster->SetTransformMatrix(matrix);

		matrix.SetTRS( FbxVector4(srcCluster->LinkPosition[0], srcCluster->LinkPosition[1], srcCluster->LinkPosition[2]),
			FbxVector4(srcCluster->LinkRotation[0], srcCluster->LinkRotation[1], srcCluster->LinkRotation[2]),
			FbxVector4(srcCluster->LinkScaling[0], srcCluster->LinkScaling[1], srcCluster->LinkScaling[2]) );

		cluster->SetTransformLinkMatrix( matrix );

		// add influence vertices

		std::for_each( srcCluster->vertices.cbegin(), srcCluster->vertices.cend(), [&cluster] ( const InputModelData::DeformedVertex &vertex ) {
			cluster->AddControlPointIndex( vertex.index, vertex.weight );
		} );

		lSkin->AddCluster(cluster);
	}
	
	
	if (lSnapshot->GetGeometry() ) lSnapshot->GetGeometry()->AddDeformer(lSkin);
	*/
	/*
	// create some special snapshot properties
	// store original model name, snapshot time, transformation
	FbxProperty lPropBaseModel = FbxProperty::Create(lSnapshot, FbxStringDT, "BaseModel", "BaseModel");
	FbxProperty lPropTime = FbxProperty::Create(lSnapshot, FbxDoubleDT, "SnapshotTime", "SnapshotTime");
	FbxProperty lPropPos = FbxProperty::Create(lSnapshot, FbxDouble4DT, "SnapshotPosition", "SnapshotPosition");
	FbxProperty	lPropRot = FbxProperty::Create(lSnapshot, FbxDouble3DT, "SnapshotRotation", "SnapshotRotation");
	FbxProperty lPropScale = FbxProperty::Create(lSnapshot, FbxDouble3DT, "SnapshotScale", "SnapshotScale");

	if (lPropBaseModel.IsValid() ) lPropBaseModel.Set( FbxString(data.baseModelName) );
	if (lPropTime.IsValid() ) lPropTime.Set( data.snapshotTime );
	if (lPropPos.IsValid() ) lPropPos.Set( FbxDouble4( data.LclPosition[0], data.LclPosition[1], data.LclPosition[2], 1.0 ) );
	if (lPropRot.IsValid() ) lPropRot.Set( FbxDouble3( data.LclRotation[0], data.LclRotation[1], data.LclRotation[2] ) );
	if (lPropScale.IsValid() ) lPropScale.Set( FbxDouble3( data.LclScaling[0], data.LclScaling[1], data.LclScaling[2] ) );
	*/
    // Build the node tree.
    FbxNode* lRootNode = pScene->GetRootNode();
    lRootNode->AddChild(lSnapshot);
	
    return true;
}

bool MakeSnapshotFBX(const char *filename, const char *snapshotName, InputModelData &data, const bool ResetXForm)
{
	//
	// Create a new FBX file
	//

	FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;
    bool lResult;

    // Prepare the FBX SDK.
    InitializeSdkObjects(lSdkManager, lScene);

    // Create the scene.
    lResult = CreateScene(lSdkManager, lScene, snapshotName, data);

    if(lResult == false)
    {
        FBXSDK_printf("\n\nAn error occurred while creating the scene...\n");
        DestroySdkObjects(lSdkManager, lResult);
        return 0;
    }

    // Save the scene.

    // A default output file name is given otherwise.
   
	
    lResult = SaveScene(lSdkManager, lScene, filename);
    
    if(lResult == false)
    {
        FBXSDK_printf("\n\nAn error occurred while saving the scene...\n");
        DestroySdkObjects(lSdkManager, lResult);
        return 0;
    }

    // Destroy all objects created by the FBX SDK.
    DestroySdkObjects(lSdkManager, lResult);

	return lResult;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//


#ifdef IOS_REF
	#undef  IOS_REF
	#define IOS_REF (*(pManager->GetIOSettings()))
#endif

void InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene)
{
    //The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
    pManager = FbxManager::Create();
    if( !pManager )
    {
        FBXSDK_printf("Error: Unable to create FBX Manager!\n");
        exit(1);
    }
	else FBXSDK_printf("Autodesk FBX SDK version %s\n", pManager->GetVersion());

	//Create an IOSettings object. This object holds all import/export settings.
	FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
	pManager->SetIOSettings(ios);

	//Load plugins from the executable directory (optional)
	FbxString lPath = FbxGetApplicationDirectory();
	pManager->LoadPluginsDirectory(lPath.Buffer());

    //Create an FBX scene. This object holds most objects imported/exported from/to files.
    pScene = FbxScene::Create(pManager, "My Scene");
	if( !pScene )
    {
        FBXSDK_printf("Error: Unable to create FBX scene!\n");
        exit(1);
    }
}

void DestroySdkObjects(FbxManager* pManager, bool pExitStatus)
{
    //Delete the FBX Manager. All the objects that have been allocated using the FBX Manager and that haven't been explicitly destroyed are also automatically destroyed.
    if( pManager ) pManager->Destroy();
	if( pExitStatus ) FBXSDK_printf("Program Success!\n");
}

bool SaveScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename, int pFileFormat, bool pEmbedMedia)
{
    int lMajor, lMinor, lRevision;
    bool lStatus = true;

    // Create an exporter.
    FbxExporter* lExporter = FbxExporter::Create(pManager, "");

    if( pFileFormat < 0 || pFileFormat >= pManager->GetIOPluginRegistry()->GetWriterFormatCount() )
    {
        // Write in fall back format in less no ASCII format found
        pFileFormat = pManager->GetIOPluginRegistry()->GetNativeWriterFormat();

        //Try to export in ASCII if possible
        int lFormatIndex, lFormatCount = pManager->GetIOPluginRegistry()->GetWriterFormatCount();

        for (lFormatIndex=0; lFormatIndex<lFormatCount; lFormatIndex++)
        {
            if (pManager->GetIOPluginRegistry()->WriterIsFBX(lFormatIndex))
            {
                FbxString lDesc =pManager->GetIOPluginRegistry()->GetWriterFormatDescription(lFormatIndex);
                const char *lASCII = "ascii";
                if (lDesc.Find(lASCII)>=0)
                {
                    pFileFormat = lFormatIndex;
                    break;
                }
            }
        } 
    }

    // Set the export states. By default, the export states are always set to 
    // true except for the option eEXPORT_TEXTURE_AS_EMBEDDED. The code below 
    // shows how to change these states.
    IOS_REF.SetBoolProp(EXP_FBX_MATERIAL,        true);
    IOS_REF.SetBoolProp(EXP_FBX_TEXTURE,         true);
    IOS_REF.SetBoolProp(EXP_FBX_EMBEDDED,        pEmbedMedia);
    IOS_REF.SetBoolProp(EXP_FBX_SHAPE,           true);
    IOS_REF.SetBoolProp(EXP_FBX_GOBO,            true);
    IOS_REF.SetBoolProp(EXP_FBX_ANIMATION,       true);
    IOS_REF.SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);

    // Initialize the exporter by providing a filename.
    if(lExporter->Initialize(pFilename, pFileFormat, pManager->GetIOSettings()) == false)
    {
        FBXSDK_printf("Call to FbxExporter::Initialize() failed.\n");
        //FBXSDK_printf("Error returned: %s\n\n", lExporter->GetLastErrorString());
        return false;
    }

    FbxManager::GetFileFormatVersion(lMajor, lMinor, lRevision);
    FBXSDK_printf("FBX file format version %d.%d.%d\n\n", lMajor, lMinor, lRevision);

    // Export the scene.
    lStatus = lExporter->Export(pScene); 

    // Destroy the exporter.
    lExporter->Destroy();
    return lStatus;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

size_t InputModelData::PolyInfo::ComputeTotalSize()
{
	size_t size = 0;
	
	size += 4;	// materialId
	size += 4;	// vertexCount
	size += 4;	// number of indices
	size += sizeof(int) * vertexCount;

	return size;
}

size_t	InputModelData::DeformedVertex::ComputeTotalSize()
{
	return (sizeof(int) + sizeof(float));
}

size_t	InputModelData::Cluster::ComputeTotalSize()
{
	size_t size = 0;

	size += sizeof(char) * STRING_MAX_LENGTH;
	size += sizeof(int);
	size += sizeof(char) * STRING_MAX_LENGTH;
	
	size += sizeof(double) * 9;
	size += sizeof(double) * 9;

	size += sizeof(int);

	size += sizeof(int);
	for (auto iter=vertices.begin(); iter!=vertices.end(); ++iter)
		size += iter->ComputeTotalSize();

	return size;
}

size_t	InputModelData::ComputeTotalSize()
{
	size_t size = 0;

	// positions
	size += 4; // number of positions
	size += (sizeof(Float4) * positions.size() );

	// material mapping
	size += 4;	// int
	size += 4;	// number of material indices
	size += (sizeof(int) * materialIndices.size() );

	// number of polyInfo
	size += 4;

	for (auto iter=polyInfo.begin(); iter!=polyInfo.end(); ++iter)
		size += iter->ComputeTotalSize();

	// normals

	size += 4;	// normal mapping
	size += 4;	// normal reference mode

	size += 4; // number of normals direct
	size += sizeof(Float4) * normalsDirect.size();
	size += 4; // number of normals indices
	size += sizeof(int) * normalIndices.size();

	// transformation 

	// Lcl position, rotation, scaling
	size += sizeof(double) * 9;

	//
	// uvset

	size += sizeof(char) * STRING_MAX_LENGTH;
	size += 4; // uvsetmapping
	size += 4; // uvset reference mode

	size += 4; //number of uvs
	size += sizeof(Float2) * uvs.size();
	size += 4; // number of uv indices
	size += sizeof(int) * uvIndices.size();

	// properties

	//
	size += sizeof(char) * STRING_MAX_LENGTH;	// baseModelname
	size += sizeof(double);	// snapshot time

	// clusters

	size += sizeof(int);
	for (auto iter=clusters.begin(); iter!=clusters.end(); ++iter)
		size += (*iter)->ComputeTotalSize();

	//

	return size;
}

size_t InputModelData::PolyInfo::CopyToMemory(void *memory)
{
	int *intPtr = (int*) memory;
	*intPtr = materialId;
	intPtr++;
	*intPtr = vertexCount;
	intPtr++;
	if (vertexCount > 0)
		memcpy( intPtr, indices, sizeof(int) * vertexCount );

	return (sizeof(int) + sizeof(int) + sizeof(int) * vertexCount);
}

size_t InputModelData::DeformedVertex::CopyToMemory(void *memory)
{
	int *intPtr = (int*) memory;
	*intPtr = index;
	intPtr++;
	float *floatptr = (float*) intPtr;
	*floatptr = weight;

	return (sizeof(int) + sizeof(float));
}

size_t InputModelData::Cluster::CopyToMemory(void *memory)
{
	unsigned char *szptr = (unsigned char*) memory;

	memcpy( szptr, name, sizeof(char) * STRING_MAX_LENGTH );
	szptr += sizeof(char) * STRING_MAX_LENGTH;

	*( (int*) szptr ) = parent;
	szptr += sizeof(int);

	memcpy( szptr, modelname, sizeof(char) * STRING_MAX_LENGTH );
	szptr += sizeof(char) * STRING_MAX_LENGTH;

	memcpy( szptr, LclPosition, sizeof(double) * 3 );
	szptr += sizeof(double) * 3;
	memcpy( szptr, LclRotation, sizeof(double) * 3 );
	szptr += sizeof(double) * 3;
	memcpy( szptr, LclScaling, sizeof(double) * 3 );
	szptr += sizeof(double) * 3;

	memcpy( szptr, LinkPosition, sizeof(double) * 3 );
	szptr += sizeof(double) * 3;
	memcpy( szptr, LinkRotation, sizeof(double) * 3 );
	szptr += sizeof(double) * 3;
	memcpy( szptr, LinkScaling, sizeof(double) * 3 );
	szptr += sizeof(double) * 3;

	*( (int*) szptr ) = mode;
	szptr += sizeof(int);

	*( (int*) szptr ) = (int) vertices.size();
	szptr += sizeof(int);

	for (auto iter=vertices.begin(); iter!=vertices.end(); ++iter)
	{
		size_t localsize = iter->CopyToMemory(szptr);
		szptr += localsize;
	}

	return ComputeTotalSize();
}

void	InputModelData::CopyToMemory(void *memory)
{
	size_t size = 0;

	unsigned char *szPtr = (unsigned char*) memory;
	int *intPtr = (int*) memory;

	auto fn_putInt = [&] (const int value) {
		intPtr = (int*) szPtr;
		*intPtr = value;
		szPtr += sizeof(int);
	};

	fn_putInt( (int) positions.size() );

	size = sizeof(Float4) * positions.size();
	if (size > 0)
	{
		memcpy( szPtr, positions.data(), size );
		szPtr += size;
	}


	// material
	fn_putInt(materialMapping);
	fn_putInt( (int) materialIndices.size() );

	size = sizeof(int) * materialIndices.size();
	if (size > 0)
	{
		memcpy( szPtr, materialIndices.data(), size );
		szPtr += size;
	}

	// poly info
	fn_putInt( (int) polyInfo.size() );

	for (auto iter=polyInfo.begin(); iter!=polyInfo.end(); ++iter)
	{
		size_t localSize = iter->CopyToMemory(szPtr);
		szPtr += localSize; // offset ptr
	}

	// normals

	fn_putInt( normalMapping );
	fn_putInt( normalReferenceMode );

	fn_putInt( (int) normalsDirect.size() );
	size = sizeof(Float4) * normalsDirect.size();
	if (size > 0)
	{
		memcpy( szPtr, normalsDirect.data(), size );
		szPtr += size;
	}

	fn_putInt( (int) normalIndices.size() );
	size = sizeof(int) * normalIndices.size();
	if (size > 0)
	{
		memcpy( szPtr, normalIndices.data(), size );
		szPtr += size;
	}

	// transformation

	memcpy( szPtr, LclPosition, sizeof(double) * 3 );
	szPtr += sizeof(double) * 3;
	memcpy( szPtr, LclRotation, sizeof(double) * 3 );
	szPtr += sizeof(double) * 3;
	memcpy( szPtr, LclScaling, sizeof(double) * 3 );
	szPtr += sizeof(double) * 3;

	// uvset

	memcpy( szPtr, uvSetName, sizeof(char) * STRING_MAX_LENGTH );
	szPtr += sizeof(char) * STRING_MAX_LENGTH;

	fn_putInt( uvSetMapping );
	fn_putInt( uvSetReferenceMode );

	fn_putInt( (int) uvs.size() );
	size = sizeof(Float2) * uvs.size();
	if (size > 0)
	{
		memcpy( szPtr, uvs.data(), size );
		szPtr += size;
	}
	fn_putInt( (int) uvIndices.size() );
	size = sizeof(int) * uvIndices.size();
	if (size > 0)
	{
		memcpy( szPtr, uvIndices.data(), size );
		szPtr += size;
	}

	// properties

	memcpy( szPtr, baseModelName, sizeof(char) * STRING_MAX_LENGTH );
	szPtr += sizeof(char) * STRING_MAX_LENGTH;

	memcpy( szPtr, &snapshotTime, sizeof(double) );
	szPtr += sizeof(double);

	// clusters

	fn_putInt( (int) clusters.size() );
	for (auto iter=clusters.begin(); iter!=clusters.end(); ++iter)
	{
		size_t localsize = (*iter)->CopyToMemory(szPtr);
		szPtr += localsize;
	}
}

size_t InputModelData::PolyInfo::InitFromMemory(void *memory)
{
	Free();

	int *intPtr = (int *) memory;
	materialId = *intPtr;
	intPtr++;
	vertexCount = *intPtr;
	intPtr++;

	if (vertexCount > 0)
	{
		indices = new int[vertexCount];
		memcpy( indices, intPtr, sizeof(int) * vertexCount );
	}

	return (sizeof(int) + sizeof(int) + sizeof(int) * vertexCount);
}

size_t InputModelData::DeformedVertex::InitFromMemory(void *memory)
{
	unsigned char *szptr = (unsigned char*) memory;
	index = *( (int*) szptr );
	szptr += sizeof(int);
	weight = *( (float*) szptr );
	
	return (sizeof(int) + sizeof(float) );
}

size_t InputModelData::Cluster::InitFromMemory(void *memory)
{
	unsigned char *szptr = (unsigned char*) memory;

	memcpy( name, szptr, sizeof(char)*STRING_MAX_LENGTH );
	szptr += sizeof(char) * STRING_MAX_LENGTH;

	parent = *( (int*) szptr );
	szptr += sizeof(int);

	memcpy( modelname, szptr, sizeof(char)*STRING_MAX_LENGTH );
	szptr += sizeof(char) * STRING_MAX_LENGTH;

	memcpy( LclPosition, szptr, sizeof(double)*3 );
	szptr += sizeof(double) * 3;
	memcpy( LclRotation, szptr, sizeof(double)*3 );
	szptr += sizeof(double) * 3;
	memcpy( LclScaling, szptr, sizeof(double)*3 );
	szptr += sizeof(double) * 3;

	memcpy( LinkPosition, szptr, sizeof(double)*3 );
	szptr += sizeof(double) * 3;
	memcpy( LinkRotation, szptr, sizeof(double)*3 );
	szptr += sizeof(double) * 3;
	memcpy( LinkScaling, szptr, sizeof(double)*3 );
	szptr += sizeof(double) * 3;

	mode = *( (int*) szptr );
	szptr += sizeof(int);

	int value = *( (int*) szptr );
	szptr += sizeof(int);

	vertices.resize(value);
	for (auto iter=vertices.begin(); iter!=vertices.end(); ++iter)
	{
		size_t localsize = iter->InitFromMemory(szptr);
		szptr += localsize;
	}

	return ComputeTotalSize();
}

void	InputModelData::InitFromMemory(void *memory)
{
	unsigned char *szPtr = (unsigned char *) memory;
	int *intPtr = (int*) szPtr;
	int value;
	size_t size = 0;

	auto fn_getInt = [&] () -> int {
		intPtr = (int*) szPtr;
		value = *intPtr;
		szPtr += sizeof(int);
		return value;
	};

	// positions

	fn_getInt();
	positions.resize( value );
	if (value > 0)
	{
		size = sizeof(Float4) * value;
		memcpy( positions.data(), szPtr, size );
		szPtr += size;
	}

	// material

	fn_getInt();
	materialMapping = value;
	
	fn_getInt();
	materialIndices.resize(value);
	if (value > 0) {
		size = sizeof(int) * value;
		memcpy( materialIndices.data(), szPtr, size );
		szPtr += size;
	}

	// poly info

	fn_getInt();
	polyInfo.resize( value );
	if (value > 0)
	{
		for (auto iter=polyInfo.begin(); iter!=polyInfo.end(); ++iter)
		{
			size_t localsize = iter->InitFromMemory(szPtr);
			szPtr += localsize;
		}
	}

	// normals

	fn_getInt();
	normalMapping = value;
	fn_getInt();
	normalReferenceMode = value;

	fn_getInt();
	normalsDirect.resize( value );
	if (value > 0)
	{
		size = sizeof(Float4) * value;
		memcpy( normalsDirect.data(), szPtr, size );
		szPtr += size;
	}

	fn_getInt();
	normalIndices.resize( value );
	if (value > 0)
	{
		size = sizeof(int) * value;
		memcpy( normalIndices.data(), szPtr, size );
		szPtr += size;
	}

	// transformation

	memcpy( LclPosition, szPtr, sizeof(double) * 3 );
	szPtr += sizeof(double) * 3;
	memcpy( LclRotation, szPtr, sizeof(double) * 3 );
	szPtr += sizeof(double) * 3;
	memcpy( LclScaling, szPtr, sizeof(double) * 3 );
	szPtr += sizeof(double) * 3;

	// uvset

	memcpy( uvSetName, szPtr, sizeof(char) * STRING_MAX_LENGTH );
	szPtr += sizeof(char) * STRING_MAX_LENGTH;

	fn_getInt();
	uvSetMapping = value;
	fn_getInt();
	uvSetReferenceMode = value;

	fn_getInt();
	uvs.resize(value);
	if (value > 0)
	{
		size = sizeof(Float2) * value;
		memcpy( uvs.data(), szPtr, size );
		szPtr += size;
	}

	fn_getInt();
	uvIndices.resize(value);
	if (value > 0)
	{
		size = sizeof(int) * value;
		memcpy( uvIndices.data(), szPtr, size );
		szPtr += size;
	}

	// properties

	memcpy( baseModelName, szPtr, sizeof(char) * STRING_MAX_LENGTH );
	szPtr += sizeof(char) * STRING_MAX_LENGTH;

	memcpy( &snapshotTime, szPtr, sizeof(double) );
	szPtr += sizeof(double);

	// clusters

	fn_getInt();
	
	for (int i=0; i<value; ++i)
	{
		Cluster *pCluster = new Cluster();
		size_t localsize = pCluster->InitFromMemory(szPtr);
		szPtr += localsize;

		clusters.insert(pCluster);
	}

}