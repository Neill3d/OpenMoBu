
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BlendShapeToolkit_brushes.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <GL/glew.h>
//--- OR SDK include
#include <fbsdk/fbsdk.h>
#include "BlendShapeToolkit_brushesBase.h"
#include "FileUtils.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////
// BrushMove

class BrushDrag : public BaseBrush
{
public:
	//! a constructor
	BrushDrag( FBComponent *pMaster );

	virtual ~BrushDrag();

	// information
	const char* GetCaption() override { return "Drag"; }
	const char* GetPicturePath() override { 
		char buffer[256]{ 0 };
		if (FindEffectLocation("\\System\\BlendShapeToolkit\\Grab.png", buffer, 256))
		{
			FullPath = buffer;
			return FullPath;
		}
		
		return "\\BlendShapeToolkit\\Grab.png"; 
	}

	bool	WantToReacalcWeights() { return false; }

	// update mesh vertices according to this brush algorithm
	void	Process( const BrushData &brushData, BrushCameraData *pCameraData, const OperationBuffer &bufferZero, OperationBuffer &buffer );
};

///////////////////////////////////////////////////////////////////////////////////////////////////////
// BrushPush

class BrushPush : public BaseBrush
{
public:
	//! a constructor
	BrushPush( FBComponent *pMaster );

	virtual ~BrushPush();

	// information
	const char* GetCaption() { return "Push"; }
	const char* GetPicturePath() { 
		char buffer[256]{ 0 };
		if (FindEffectLocation("\\System\\BlendShapeToolkit\\Bulge.png", buffer, 256))
		{
			FullPath = buffer;
			return FullPath;
		}
		
		return "\\BlendShapeToolkit\\Bulge.png"; 
	}

	bool	WantToReacalcWeights() { return true; }

	// update mesh vertices according to this brush algorithm
	void	Process( const BrushData &brushData, BrushCameraData *pCameraData, const OperationBuffer &bufferZero, OperationBuffer &buffer );
};

///////////////////////////////////////////////////////////////////////////////////////////////////////
// BrushPush

class BrushFreeze : public BaseBrush
{
public:
	//! a constructor
	BrushFreeze( FBComponent *pMaster );

	virtual ~BrushFreeze();

	// information
	const char* GetCaption() override { return "Freeze"; }
	const char* GetPicturePath() override { 
		char buffer[256]{ 0 };
		if (FindEffectLocation("\\System\\BlendShapeToolkit\\Freeze.png", buffer, 256))
		{
			FullPath = buffer;
			return FullPath;
		}
		
		return "\\BlendShapeToolkit\\Freeze.png";
	}

	bool	WantToReacalcWeights() { return true; }

	// update mesh vertices according to this brush algorithm
	void	Process( const BrushData &brushData, BrushCameraData *pCameraData, const OperationBuffer &bufferZero, OperationBuffer &buffer );
};

///////////////////////////////////////////////////////////////////////////////////////////////////////
// BrushSmooth

class BrushSmooth : public BaseBrush
{
public:
	//! a constructor
	BrushSmooth( FBComponent *pMaster );

	virtual ~BrushSmooth();

	// information
	const char* GetCaption() override { return "Smooth"; }
	const char* GetPicturePath() override { 
		char buffer[256]{ 0 };
		if (FindEffectLocation("\\System\\BlendShapeToolkit\\Smooth.png", buffer, 256))
		{
			FullPath = buffer;
			return FullPath;
		}
		
		return "\\BlendShapeToolkit\\Smooth.png";
	}

	bool	WantToReacalcWeights() { return true; }

	// update mesh vertices according to this brush algorithm
	void	Process( const BrushData &brushData, BrushCameraData *pCameraData, const OperationBuffer &bufferZero, OperationBuffer &buffer );
};

///////////////////////////////////////////////////////////////////////////////////////////////////////
// BrushErase

class BrushErase : public BaseBrush
{
public:
	//! a constructor
	BrushErase( FBComponent *pMaster );

	virtual ~BrushErase();

	// information
	const char* GetCaption() override { return "Erase"; }
	const char* GetPicturePath() override { 
		char buffer[256]{ 0 };
		if (FindEffectLocation("\\System\\BlendShapeToolkit\\Erase.png", buffer, 256))
		{
			FullPath = buffer;
			return FullPath;
		}
		
		return "\\BlendShapeToolkit\\Erase.png";
	}

	bool	WantToReacalcWeights() { return true; }

	// update mesh vertices according to this brush algorithm
	void	Process( const BrushData &brushData, BrushCameraData *pCameraData, const OperationBuffer &bufferZero, OperationBuffer &buffer );
};

///////////////////////////////////////////////////////////////////////////////////////////////////////
// BrushPaint

class BrushPaint : public BaseBrush
{
public:
	//! a constructor
	BrushPaint( FBComponent *pMaster );

	virtual ~BrushPaint();

	// information
	const char* GetCaption() override { return "Paint"; }
	const char* GetPicturePath() override { 
		char buffer[256]{ 0 };
		if (FindEffectLocation("\\System\\BlendShapeToolkit\\Paint.png", buffer, 256))
		{
			FullPath = buffer;
			return FullPath;
		}
		
		return "\\BlendShapeToolkit\\Paint.png";
	}

	bool	WantToReacalcWeights() { return true; }

	// update mesh vertices according to this brush algorithm
	void	Process( const BrushData &brushData, BrushCameraData *pCameraData, const OperationBuffer &bufferZero, OperationBuffer &buffer );
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// FalloffLinear

class FalloffLinear : public BaseFalloff
{
public:
	//! a constructor
	FalloffLinear()
		: BaseFalloff()
	{}

	virtual ~FalloffLinear()
	{}

	const char* GetCaption() override { return "Linear"; }
	const char* GetPicturePath() override { 
		char buffer[256]{ 0 };
		if (FindEffectLocation("\\System\\BlendShapeToolkit\\falloffLinear.png", buffer, 256))
		{
			FullPath = buffer;
			return FullPath;
		}
		
		return "\\BlendShapeToolkit\\falloffLinear.png";
	}

	double	Calculate(double t) override { return t; }
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// FalloffHard

class FalloffHard: public BaseFalloff
{
public:
	//! a constructor
	FalloffHard()
		: BaseFalloff()
	{}

	virtual ~FalloffHard()
	{}

	const char* GetCaption() override { return "Hard"; }
	const char* GetPicturePath() override { 
		char buffer[256]{ 0 };
		if (FindEffectLocation("\\System\\BlendShapeToolkit\\falloffTop.png", buffer, 256))
		{
			FullPath = buffer;
			return FullPath;
		}
		
		return "\\BlendShapeToolkit\\falloffTop.png";
	}

	double	Calculate(double t) override;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// FalloffPoint

class FalloffPoint: public BaseFalloff
{
public:
	//! a constructor
	FalloffPoint()
		: BaseFalloff()
	{}

	virtual ~FalloffPoint()
	{}

	const char* GetCaption() override { return "Point"; }
	const char* GetPicturePath() override { 
		char buffer[256]{ 0 };
		if (FindEffectLocation("\\System\\BlendShapeToolkit\\falloffPoint.png", buffer, 256))
		{
			FullPath = buffer;
			return FullPath;
		}
		
		return "\\BlendShapeToolkit\\falloffPoint.png";
	}

	double	Calculate(double t) override;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// FalloffSmooth

class FalloffSmooth: public BaseFalloff
{
public:
	//! a constructor
	FalloffSmooth()
		: BaseFalloff()
	{}

	virtual ~FalloffSmooth()
	{}

	const char* GetCaption() override { return "Smooth"; }
	const char* GetPicturePath() override { 
		char buffer[256]{ 0 };
		if (FindEffectLocation("\\System\\BlendShapeToolkit\\falloffSmooth.png", buffer, 256))
		{
			FullPath = buffer;
			return FullPath;
		}
		
		return "\\BlendShapeToolkit\\falloffSmooth.png";
	}

	double	Calculate(double t) override;
};