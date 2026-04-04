#ifndef __BOX_RBF_H__
#define __BOX_RBF_H__


/**	\file	BoxRBF.h
*	Declaration of a RBF box class.
*	Based on jsRadial - https://github.com/scorza/jsRadial
*
*	Sergei Solokhin (Neill3d) 2018-2026
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

// C++
#include <chrono>
#include <vector>

//--- Registration defines
#define	BOXRBF3__CLASSNAME		BoxRBF3
#define BOXRBF3__CLASSSTR		"BoxRBF3"

#define	BOXRBF4__CLASSNAME		BoxRBF4
#define BOXRBF4__CLASSSTR		"BoxRBF4"

#define MIN_NUMBER_OF_TARGETS	6
#define MAX_NUMBER_OF_TARGETS	24

enum EFunctionType
{
	eLinear,
	eGaussian,
	eMultiquadratic,
	eInverseMultiquadratic
};

/**	RBF Interpolator of 3 parameters.
*	Box for a relation constraint
*/
class BoxRBF3 : public FBBox
{
	//--- box declaration.
	FBBoxDeclare(BoxRBF3, FBBox);

public:
	virtual bool FBCreate() override;		//!< creation function.
	
	//! Overloaded FBBox real-time evaluation function.
	virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode,FBEvaluateInfo* pEvaluateInfo) override;

public:

	FBPropertyBaseEnum<EFunctionType>	FunctionType;
	FBPropertyAnimatableDouble			Height;
	FBPropertyAnimatableDouble			Sigma;
	FBPropertyAnimatableDouble			Scale;

	FBPropertyBool 					UseGlobalSigma; // switch between global sigma or local sigma (radii) for each target

protected:

	FBAnimationNode* m_Pose{ nullptr };
	FBAnimationNode* m_Targets[MAX_NUMBER_OF_TARGETS]{ nullptr };

	FBAnimationNode* m_Radii[MAX_NUMBER_OF_TARGETS]{ nullptr };

	FBAnimationNode* m_OutScale[MAX_NUMBER_OF_TARGETS]{ nullptr };
	FBAnimationNode* m_OutInterpolate{ nullptr };

	double RBF(double input, double height, double sigma, int ftype) const;

	int ReadCountFromConfig() const;

protected:

	int								m_NumberOfTargets;
	std::vector<double>				m_Vecs;

	virtual const int GetPlugDim() const { return 3; }

};

/**	RBF Interpolator of quaternions.
*	Box for a relation constraint
*/
class BoxRBF4 : public BoxRBF3
{
	//--- box declaration.
	FBBoxDeclare(BoxRBF4, BoxRBF3);

public:

	//! Overloaded FBBox real-time evaluation function.
	virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo) override;

protected:
	const int GetPlugDim() const override { return 4; }
};


#endif /* __BOX_RBF_H__ */
