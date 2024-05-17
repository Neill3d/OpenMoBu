
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BlendShapeToolkit_brushesBase.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- OR SDK include
#include <fbsdk/fbsdk.h>
#include <map>
#include <set>
#include <vector>

enum FBBrushDirection
{
	kFBBrushScreen,
	kFBBrushFirstNormal,
	kFBBrushAverageNormal,
	kFBBrushVertexNormal,
	kFBBrushX,
	kFBBrushY,
	kFBBrushZ
};

struct OperationVertex
{
	FBVector3d		position;
	FBNormal		normal;			// do we need to recalculate normal after brush operation ?
	
	FBColorF		color;			// vertex color for paint brush
	FBUV			uv;

	float			freeze;			// freeze operation on vertice (don't transform it)
	float			weight;			// affect weights (calculated with brush position, radius and falloff type)
};

struct OperationBuffer
{
	HdlFBPlugTemplate<FBModel>				pModel;
	std::vector <OperationVertex>			vertices;	

	//! a constructor
	OperationBuffer()
	{}

	//! a copy constructor
	OperationBuffer( const OperationBuffer &buffer )
	{
		pModel = buffer.pModel;

		const size_t count = buffer.vertices.size();

		if (count > 0)
		{
			vertices.resize( count );
			memcpy( &vertices[0], &buffer.vertices[0], sizeof(OperationVertex) * count );
		}
	}
};


typedef std::set<int>				IntSet;

struct VertEdge
{
	int			vert;
	float		dist;

	//! a constructor
	VertEdge()
	{
		vert = -1;
		dist = 0.0f;
	}

	VertEdge(const int index, const float value)
		: vert(index)
		, dist(value)
	{}

	//! a copy constructor
	VertEdge(const VertEdge &ve)
	{
		vert = ve.vert;
		dist = ve.dist;
	}
};

struct MeshVertEdges
{
	int					thisVert;		// index of the base vertex
	unsigned char		flag;			// check if this vert is already processed
	std::set<int>		neighboars;
	float				*distances;		// pre cache distances

	void FreeMemory()
	{
		if (distances)
		{
			delete [] distances;
			distances = nullptr;
		}
	}

	//! a constructor
	MeshVertEdges()
	{
		flag = 0;
		thisVert = -1;
		distances = nullptr;
	}

	//! a copy constructor
	MeshVertEdges(const MeshVertEdges &item)
	{
		flag = 0;
		thisVert = item.thisVert;
		neighboars.clear();
		distances = nullptr;
	}

	//! destructor
	~MeshVertEdges()
	{
		FreeMemory();
	}

	void CalculateDistances( FBMatrix &pMatrix, FBMesh *pMesh, bool OnlyPosition )
	{
		if (neighboars.size() == 0) return;

		FBVertex thisPosition;
		//FBVertex thisPosition = pMesh->VertexGet(thisVert);
		FBVertexMatrixMult( thisPosition, pMatrix, pMesh->VertexGet(thisVert) );

		// reallocate memory
		if (OnlyPosition == false)
		{
			FreeMemory();
			distances = new float[neighboars.size()];
		}

		int ndx=0;
		for (auto iter=neighboars.begin(); iter!=neighboars.end(); ++iter)
		{
			FBVertex pos;
			//FBVertex pos = pMesh->VertexGet( *iter );
			FBVertexMatrixMult( pos, pMatrix, pMesh->VertexGet( *iter ) );
			float dist = (float) FBLength( FBVertex(pos[0]-thisPosition[0], pos[1]-thisPosition[1], pos[2]-thisPosition[2], 0.0f) );

			distances[ndx] = dist;
			ndx++;
		}

		if (distances == nullptr)
		{
			printf( "emptry distances!\n" );
		}
	}

	/////
	// work with processing flag

	void SetFlag() { flag = 1; }
	bool IsFlag() { return (flag > 0); }
	void ZeroFlag() { flag = 0; }
};

struct MeshEdgesGraph
{
	struct Edge
	{
		int a;
		int b;
	};

	std::vector<MeshVertEdges>		data;
	
	void FreeMemory()
	{
		for (auto iter=data.begin(); iter!=data.end(); ++iter)
		{
			iter->FreeMemory();
		}
	}

	void BuildGraph( FBMatrix &pMatrix, FBMesh *pMesh )
	{
		int vertCount = pMesh->VertexCount();
		if (vertCount == 0) return;

		int polyCount = pMesh->PolygonCount();
		if (polyCount == 0) return;

		FreeMemory();

		MeshVertEdges	newVertEdges;

		data.resize(vertCount, newVertEdges);

		int idx=0;
		for (auto iter=data.begin(); iter!=data.end(); ++iter)
		{
			iter->thisVert = idx;
			iter->flag = 0;
			iter->neighboars.clear();
			iter->distances = nullptr;
			idx++;
		}

		for (int i=0; i<polyCount; ++i)
		{
			int vertCount = pMesh->PolygonVertexCount(i);

			int n1, n2;

			for (int j=0; j<vertCount-1; ++j)
			{
				if (j==vertCount-1)
				{
					n1 = j;
					n2 = 0;
				}
				else
				{
					n1 = j;
					n2 = j+1;
				}
				
				data[pMesh->PolygonVertexIndex(i,n1)].neighboars.insert( pMesh->PolygonVertexIndex(i,n2) );
				data[pMesh->PolygonVertexIndex(i,n2)].neighboars.insert( pMesh->PolygonVertexIndex(i,n1) );
			}
		}

		// re calculate distances
		CalculateDistances( pMatrix, pMesh, false );
	}

	void CalculateDistances( FBMatrix &pMatrix, FBMesh *pMesh, bool OnlyPosition )
	{
		// re calculate distances
		for (auto iter=data.begin(); iter!=data.end(); ++iter)
		{
			iter->CalculateDistances( pMatrix, pMesh, OnlyPosition );
		}
	}

	void ZeroFlags()
	{
		for (auto iter=data.begin(); iter!=data.end(); ++iter)
			iter->ZeroFlag();
	}

	void SetFlag(const int index)
	{
		data[index].SetFlag();
	}

	bool IsFlag(const int index)
	{
		return data[index].IsFlag();
	}

	// index - vertex index in the mesh [0..vertexCount]
	IntSet &GetVertexNeighbores(const int index)
	{
		return data[index].neighboars;
	}

	float *GetVertexDistances(const int index)
	{
		return data[index].distances;
	}
};

////////////////
struct BrushCameraData
{
	int				mouseX;
	int				mouseY;

	int				absMouseX;
	int				absMouseY;

	int				prevMouseX;
	int				prevMouseY;

	int				deltaMouseX;
	int				deltaMouseY;

	FBBrushDirection		direction;
	FBVector3d		deltaView;			// delta translation in view plane

	FBTVector		pos;
	FBTVector		nor;

	FBTVector				viewPlane;
	FBTVector				averageNormal;

	//! a constructor
	BrushCameraData()
	{
		direction = kFBBrushScreen;

		mouseX = 0;
		mouseY = 0;
		prevMouseX = 0;
		prevMouseY = 0;
		deltaMouseX = 0;
		deltaMouseY = 0;
	}
};

// this struct is always availible with last information about brush position, common flags, etc
struct BrushData
{
	// point on surface
	FBColorF		color;

	double			radius;
	double			strength;
	
	bool			fillMode;	// fill with current brush

	bool			inverted;		// is strength inverted ?

	bool			isOnSurface;
	
	std::map<const FBCamera*, BrushCameraData>	mCameraData;

	//! a constructor
	BrushData()
	{
		mModel = nullptr;
		isOnSurface = false;

		color = FBColorF(1.0f, 1.0f, 1.0f, 1.0f);

		radius = 5.0;
		strength = 5.0;

		fillMode = false;

		inverted = false;
	}

	void ClearCameraData() {
		mCameraData.clear();
	}

	BrushCameraData *GetCameraData(const FBCamera *pCamera)
	{
		auto iter = mCameraData.find(pCamera);
		if (iter != mCameraData.end() )
			return &iter->second;

		return nullptr;
	}

	BrushCameraData *GetOrCreateCameraData(const FBCamera *pCamera)
	{
		auto iter = mCameraData.find(pCamera);
		if (iter != mCameraData.end() )
			return &iter->second;

		BrushCameraData		newData;
		mCameraData.insert( std::make_pair(pCamera, newData) );

		iter = mCameraData.find(pCamera);
		if (iter != mCameraData.end() )
			return &iter->second;

		return nullptr;
	}

	void SetModel(FBModel *pModel)
	{
		if (pModel != nullptr)
		{
			FBMatrix pMatrix;
			pModel->GetMatrix( pMatrix, kModelTransformation_Geometry );

			FBMesh *pMesh = (FBMesh*) (FBGeometry*) pModel->Geometry;
			mEdgesGraph.BuildGraph( pMatrix, pMesh );

			mModel = pModel;
		}
		else
		{
			mModel = nullptr;

			mEdgesGraph.FreeMemory();
			mEdgesGraph.data.clear();
		}
	}


	MeshEdgesGraph &GetEdgesGraph()
	{
		return mEdgesGraph;
	}

	IntSet &GetVertexNeighbores(const int index)
	{
		return mEdgesGraph.GetVertexNeighbores(index);
	}

	bool	IsModelOk() { return mModel.Ok(); }
	FBModel *GetModelPtr() { return mModel; }

private:
	// operate with the model
	HdlFBPlugTemplate<FBModel>		mModel;

	// edges graph
	MeshEdgesGraph					mEdgesGraph;
};


//////////////////////////////////////////////////////
//! BaseBrush
/*
	manipulator to operate with a mesh by user input
	and build layout with content of brush parameters
*/
class BaseBrush
{
public:
	//! a constructor
	BaseBrush( const char *name, FBComponent *pMaster )
	{
		RegisterProperties( name, pMaster );

		Override = false;
		Radius = 50.0;
		Strength = 50.0;

		UICreate();
		UIConfig();

		mMaster = pMaster;
	}

	//! a destructor
	virtual ~BaseBrush()
	{
		UIDestroy();
	}

	void	FBDestroy()
	{
		UnRegisterProperties( mMaster );
	}

	//! return manipulator caption for the viewport
	virtual const char		*GetCaption() = 0;
	virtual const char		*GetPicturePath() = 0;

	virtual	bool		WantToReacalcWeights() = 0;

	// update mesh vertices according to this brush algorithm
	virtual void		Process( const BrushData &brushData, BrushCameraData *pCameraData, const OperationBuffer &bufferZero, OperationBuffer &buffer ) = 0;

	// ui for each brush
	FBLayout			*GetLayoutPtr() { return &mLayout; }

	// main brush properties
	bool	IsOverride() { 
		UpdateData();
		return Override; }

	FBColor &GetColor() {
		UpdateData();
		return Color;
	}
	double	GetRadius() { 
		UpdateData();
		return Radius; }
	double	GetStrength() { 
		UpdateData();
		return Strength; }

	void	SetColor(const FBColor value) {
		Color = value;
		UIReset();
	}
	void	SetRadius(const double value) { 
		Radius = value; 
		UIReset(); 
	}
	void	SetStrength(const double value) { 
		Strength = value; 
		UIReset(); 
	}

protected:
	bool				Override;
	FBColor				Color;
	double				Radius;
	double				Strength;

	FBString			FullPath;

protected:
	FBLayout			mLayout;		// brush options layout

	//
	//

	FBButton			mButtonOverride;
	FBEditColor			mEditColor;
	FBEditNumber		mEditRadius;
	FBEditNumber		mEditStrength;

	virtual void		UICreate();
	virtual void		UIDestroy();
	virtual void		UIConfig();
	virtual void		UIReset();
	virtual	void		UpdateData();

	// register brush properties in master
	virtual void RegisterProperties( const char *name, FBComponent *pMaster )
	{
		
	}

	virtual void UnRegisterProperties( FBComponent *pMaster )
	{
		
	}

private:

	FBComponent			*mMaster;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// BaseFalloff
class	BaseFalloff
{
public:
	//! a constructor
	BaseFalloff()
	{}

	virtual ~BaseFalloff()
	{}

	virtual const char* GetCaption() = 0;
	virtual const char* GetPicturePath() = 0;

	virtual double Calculate(double t) = 0;

protected:
	FBString	FullPath;
};


//////////////////////////////////////////////////////
//! BrushManager
/*!
	 class for operate with my custom brushes
*/
enum FBDrawState
{
	kFBDrawBefore,
	kFBDrawAfter
};

class BrushManager
{
public:
	//! a constructor
	BrushManager()
	{
		mCurrentBrush = -1;
		mCurrentFalloff = -1;
	}
	//! a destructor
	~BrushManager()
	{
		FreeMem();
	}

	void	FBDestroy()
	{
		for (int i=0; i<mBrushes.GetCount(); ++i)
			mBrushes[i]->FBDestroy();
	}

	int			AddBrush( BaseBrush *brush );
	int			AddFalloff( BaseFalloff *falloff );

	//! set current local brush as current
	void		SetCurrentBrush(const int index);
	void		SetCurrentFalloff(const int index);

	void		ToggleSmooth(const bool active, const int index);


	const int	GetNumberOfBrushes() { return mBrushes.GetCount(); }
	BaseBrush	*GetBrushPtr(const int index) { return mBrushes[index]; }
	BaseBrush	*GetCurrentBrushPtr();
	int			GetCurrentBrush();

	const int	GetNumberOfFalloffs() { return mFalloffs.GetCount(); }
	BaseFalloff *GetFalloffPtr( const int index ) { return mFalloffs[index]; }
	BaseFalloff *GetCurrentFalloffPtr() { return (mCurrentFalloff>=0) ? mFalloffs[mCurrentFalloff] : nullptr; }
	int			GetCurrentFalloff() { return mCurrentFalloff; }

	bool		IsOverride();
	FBColor		&GetColor();
	double		GetRadius();
	double		GetStrength();

	void		SetCurrentBrushRadius(const double value);
	void		SetCurrentBrushStrength(const double value);

	void		SetColor(const FBColor value);
	void		SetRadius(const double value);
	void		SetStrength(const double value);

	//
	bool		Input(int pMouseX, int pMouseY, FBInputType pAction, int pButtonKey,int pModifier);

	//! output 2d drawing
	void		Draw( const FBDrawState state );

	// check if current brush want to recalc weight on mouse move (before processing)
	bool		WantToReacalcWeights();

	// update mesh vertices according to this brush algorithm
	bool		Process( const BrushData &brushData, BrushCameraData *pCameraData, const OperationBuffer &bufferZero, OperationBuffer &buffer );

private:
	
	BrushData								mBrushData;
	
	int										mCurrentBrush;
	int										mSmoothBrush;
	FBArrayTemplate<BaseBrush*>				mBrushes;

	int										mCurrentFalloff;
	FBArrayTemplate<BaseFalloff*>			mFalloffs;

	void		FreeMem();
};