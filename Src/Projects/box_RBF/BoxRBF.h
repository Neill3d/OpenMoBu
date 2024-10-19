#ifndef __BOX_RBF_H__
#define __BOX_RBF_H__


/**	\file	BoxRBF.h
*	Declaration of a RBF box class.
*	Port of Maya's jsRadial
*
*	Sergei Solokhin (Neill3d) 2018
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

// C++
#include <chrono>
#include <vector>

// Eigen
#include <Eigen/Dense>


//--- Registration defines
#define	BOXRBF3__CLASSNAME		BoxRBF3
#define BOXRBF3__CLASSSTR		"BoxRBF3"

#define	BOXRBF4__CLASSNAME		BoxRBF4
#define BOXRBF4__CLASSSTR		"BoxRBF4"

#define MIN_NUMBER_OF_TARGETS	6
#define MAX_NUMBER_OF_TARGETS	36

enum EFunctionType
{
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
	virtual bool FBCreate();		//!< creation function.
	virtual void FBDestroy();		//!< destruction function.

	//! Overloaded FBBox real-time evaluation function.
	virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode,FBEvaluateInfo* pEvaluateInfo);

public:

	FBPropertyBaseEnum<EFunctionType>	FunctionType;
	FBPropertyAnimatableDouble			Height;
	FBPropertyAnimatableDouble			Sigma;
	FBPropertyAnimatableDouble			RotationMultiply;

private:

	FBAnimationNode*	m_Pose;
	FBAnimationNode*	m_Targets[MAX_NUMBER_OF_TARGETS];

	FBAnimationNode*	m_OutScale[MAX_NUMBER_OF_TARGETS];
	FBAnimationNode*	m_OutInterpolate;

	double RBF(const double r, const double height, const double sigma, const short ftype);

protected:

	int								m_NumberOfTargets;

	std::vector<Eigen::VectorXd>	vecs_;

	virtual const int GetPlugDim() const { return 3; }

};

/**	RBF Interpolator of 4 parameters.
*	Box for a relation constraint
*/
class BoxRBF4 : public BoxRBF3
{
	//--- box declaration.
	FBBoxDeclare(BoxRBF4, BoxRBF3);

protected:
	const int GetPlugDim() const override { return 4; }
};


#endif /* __BOX_RBF_H__ */
