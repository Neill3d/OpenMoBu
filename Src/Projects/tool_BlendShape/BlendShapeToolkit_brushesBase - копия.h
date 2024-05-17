
#pragma once

//--- OR SDK include
#include <fbsdk/fbsdk.h>
#include <map>
#include <set>


struct OperationVertex
{
	FBVector3d		position;
	FBNormal		normal;			// do we need to recalculate normal after brush operation ?

	float			freeze;			// freeze operation on vertice (don't transform it)
	float			weight;			// affect weights (calculated with brush position, radius and falloff type)
};

struct OperationBuffer
{
	HdlFBPlugTemplate<FBModel>				pModel;
	FBArrayTemplate <OperationVertex>		vertices;	

	//! a constructor
	OperationBuffer()
	{}

	//! a copy constructor
	OperationBuffer( const OperationBuffer &buffer )
	{
		pModel = buffer.pModel;

		int count = buffer.vertices.GetCount();

		if (count)
		{
			vertices.SetCount( count );
			memcpy( &vertices[0], &buffer.vertices[0], sizeof(OperationVertex) * count );
		}
	}
};

typedef std::multimap<int, int>		IntToIntMap;
typedef IntToIntMap::iterator		mapIter;
typedef std::set<int>				IntSet;


struct MeshVertEdges
{
	int					thisVert;		// index of the base vertex
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
		thisVert = -1;
		distances = nullptr;
	}

	//! destructor
	~MeshVertEdges()
	{
		FreeMemory();
	}

	void CalculateDistances( FBMesh *pMesh )
	{
		if (neighboars.size() == 0) return;

		FBVertex thisPosition = pMesh->VertexGet(thisVert);

		// reallocate memory
		FreeMemory();

		distances = new float[neighboars.size()];

		int ndx=0;
		for (auto iter=neighboars.begin(); iter!=neighboars.end(); ++iter)
		{
			FBVertex pos = pMesh->VertexGet( *iter );
			float dist = (float) FBLength( FBVertex(pos[0]-thisPosition[0], pos[1]-thisPosition[1], pos[2]-thisPosition[2], 0.0f) );

			distances[ndx] = dist;
			ndx++;
		}
	}
};

struct MeshEdgesGraph
{
	struct Edge
	{
		int a;
		int b;
	};

	IntToIntMap		edges;
	
	void BuildGraph( FBMesh *pMesh )
	{
		int polyCount = pMesh->PolygonCount();
		if (polyCount == 0) return;

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

				edges.insert( std::make_pair(pMesh->PolygonVertexIndex(i,n1), pMesh->PolygonVertexIndex(i,n2)) );
				edges.insert( std::make_pair(pMesh->PolygonVertexIndex(i,n2), pMesh->PolygonVertexIndex(i,n1)) );
			}
		}
	}

	void FindVertexNeighbores(const int index, IntSet &_set)
	{

		//IntToIntMap::const_iterator		m_it, s_it;
		/*
		for (m_it = edges.begin();  m_it != edges.end();  m_it = s_it)
		{
			int theKey = (*m_it).first;
*/
			std::pair<mapIter, mapIter> keyRange = edges.equal_range(index);

			// Iterate over all map elements with key == theKey

			for (auto s_it = keyRange.first;  s_it != keyRange.second;  ++s_it)
			{
				_set.insert( (*s_it).second );
			   //cout << "    value = " << (*s_it).second << endl;
			}
		//}
		/*
		auto iter = edges.find(index);
		while (iter != edges.end() )
		{
			int value = iter->second;

			_set.insert(value);

			iter++;
		}
		*/
	}
};

// this struct is always availible with last information about brush position, common flags, etc
struct BrushData
{
	// point on surface
	double			radius;
	double			strength;
	
	bool			isOnSurface;
	
	FBTVector		pos;
	FBTVector		nor;

	int				mouseX;
	int				mouseY;

	int				prevMouseX;
	int				prevMouseY;

	int				deltaMouseX;
	int				deltaMouseY;

	FBVector3d		deltaView;			// delta translation in view plane

	//! a constructor
	BrushData()
	{
		mModel = nullptr;
		isOnSurface = false;

		radius = 5.0;
		strength = 5.0;

		mouseX = 0;
		mouseY = 0;
		prevMouseX = 0;
		prevMouseY = 0;
		deltaMouseX = 0;
		deltaMouseY = 0;
	}

	void SetModel(FBModel *model)
	{
		mModel = model;

		if (mModel.Ok() )
		{
			FBMesh *pMesh = (FBMesh*) (FBGeometry*) mModel->Geometry;
			mEdgesGraph.BuildGraph(pMesh);
		}
	}

	void FindVertexNeighbores(const int index, IntSet &_set)
	{
		mEdgesGraph.FindVertexNeighbores(index, _set);
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
	BaseBrush()
	{}

	//! a destructor
	virtual ~BaseBrush()
	{}

	//! return manipulator caption for the viewport
	virtual char		*GetCaption() = 0;
	virtual char		*GetPicturePath() = 0;

	virtual	bool		WantToReacalcWeights() = 0;

	// update mesh vertices according to this brush algorithm
	virtual void		Process( const BrushData &brushData, const OperationBuffer &bufferZero, OperationBuffer &buffer ) = 0;

	// ui for each brush
	FBLayout			*GetLayoutPtr() { return &mLayout; }

protected:
	FBLayout			mLayout;		// brush options layout
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

	virtual char	*GetCaption() = 0;
	virtual char	*GetPicturePath() = 0;

	virtual double Calculate(double t) = 0;
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


	void		AddBrush( BaseBrush *brush );
	void		AddFalloff( BaseFalloff *falloff );

	//! set current local brush as current
	void		SetCurrentBrush(const int index);
	void		SetCurrentFalloff(const int index);

	const int	GetNumberOfBrushes() { return mBrushes.GetCount(); }
	BaseBrush	*GetBrushPtr(const int index) { return mBrushes[index]; }
	BaseBrush	*GetCurrentBrushPtr() { return (mCurrentBrush>=0) ? mBrushes[mCurrentBrush] : nullptr; }

	const int	GetNumberOfFalloffs() { return mFalloffs.GetCount(); }
	BaseFalloff *GetFalloffPtr( const int index ) { return mFalloffs[index]; }
	BaseFalloff *GetCurrentFalloffPtr() { return (mCurrentFalloff>=0) ? mFalloffs[mCurrentFalloff] : nullptr; }

	//
	bool		Input(int pMouseX, int pMouseY, FBInputType pAction, int pButtonKey,int pModifier);

	//! output 2d drawing
	void		Draw( const FBDrawState state );

	// check if current brush want to recalc weight on mouse move (before processing)
	bool		WantToReacalcWeights();

	// update mesh vertices according to this brush algorithm
	bool		Process( const BrushData &brushData, const OperationBuffer &bufferZero, OperationBuffer &buffer );

private:
	
	BrushData								mBrushData;
	
	int										mCurrentBrush;
	FBArrayTemplate<BaseBrush*>				mBrushes;

	int										mCurrentFalloff;
	FBArrayTemplate<BaseFalloff*>			mFalloffs;

	void		FreeMem();
};