
/**	\file	boxRBF.cxx
*	Definition of a RBF Interpolator box class.
*	Based of jsRadial - https://github.com/scorza/jsRadial
*
*	Sergei Solokhin (Neill3d) 2018-2026
*/

//--- Class declaration
#include "BoxRBF.h"

// Eigen
#include <Eigen/Dense>

//--- Registration defines
#define BOXRBF3__CLASS		BOXRBF3__CLASSNAME
#define BOXRBF3__NAME		BOXRBF3__CLASSSTR
#define	BOXRBF3__LOCATION	"OpenMobu"
#define BOXRBF3__LABEL		"RBF Vector"
#define	BOXRBF3__DESC		"RBF Vector Interpolator"

#define BOXRBF4__CLASS		BOXRBF4__CLASSNAME
#define BOXRBF4__NAME		BOXRBF4__CLASSSTR
#define	BOXRBF4__LOCATION	"OpenMobu"
#define BOXRBF4__LABEL		"RBF Quaternion"
#define	BOXRBF4__DESC		"RBF Quaternion Interpolator"

//--- implementation and registration
FBBoxImplementation(BOXRBF3__CLASS);		// Box class name
FBRegisterBox(	BOXRBF3__NAME,				// Unique name to register box.
				BOXRBF3__CLASS,				// Box class name
				BOXRBF3__LOCATION,			// Box location ('plugins')
				BOXRBF3__LABEL,				// Box label (name of box to display)
				BOXRBF3__DESC,			// Box long description.
									FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

FBBoxImplementation(BOXRBF4__CLASS);		// Box class name
FBRegisterBox(	BOXRBF4__NAME,				// Unique name to register box.
				BOXRBF4__CLASS,				// Box class name
				BOXRBF4__LOCATION,			// Box location ('plugins')
				BOXRBF4__LABEL,				// Box label (name of box to display)
				BOXRBF4__DESC,			// Box long description.
				FB_DEFAULT_SDK_ICON);	// Icon filename (default=Open Reality icon)


namespace internal
{
	double quatDist01(const Eigen::Quaterniond& a, const Eigen::Quaterniond& b) {
		double d = std::abs(a.dot(b));              // handle q ~ -q
		d = std::min(1.0, std::max(-1.0, d));       // clamp
		const double ang = 2.0 * std::acos(d);      // [0..pi]
		constexpr double M_PI{ 3.14159265358979323846 };
		return ang / M_PI;                          // normalize to [0..1]
	}
};

const char * FBPropertyBaseEnum<EFunctionType>::mStrings[] = {
	"Linear",
	"Gaussian",
	"Multiquadratic",
	"Inverse Multiquadratic",
	0 };

int BoxRBF3::ReadCountFromConfig() const
{
	FBConfigFile	lConfig("@OpenMoBu.txt");

	char buffer[64] = { 0 };
	sprintf_s(buffer, sizeof(char) * 64, "6");
	const char *cbuffer = buffer;
	bool lStatusAuto = lConfig.GetOrSet("Box RBF", "Number Of Connectors", cbuffer, "How many input targets box rbf will have");

	int count = 6;

	if (true == lStatusAuto)
	{
		count = atoi(cbuffer);
	}

	return count;
}

/************************************************
 *	Creation
 ************************************************/
bool BoxRBF3::FBCreate()
{
	// Properties

	FBPropertyPublish(this, FunctionType, "Function Type", nullptr, nullptr);
	FBPropertyPublish(this, Height, "Height", nullptr, nullptr);
	FBPropertyPublish(this, Sigma, "Sigma", nullptr, nullptr);
	FBPropertyPublish(this, Scale, "Data Scale", nullptr, nullptr);

	FBPropertyPublish(this, UseGlobalSigma, "Use Global Sigma", nullptr, nullptr);

	Height.SetMin(0.00001);
	Sigma.SetMin(0.00001);

	Height = 10.0;
	Sigma = 10.0;
	Scale = 100.0;
	UseGlobalSigma = true;

	m_NumberOfTargets = ReadCountFromConfig();

	m_NumberOfTargets = std::max(m_NumberOfTargets, MIN_NUMBER_OF_TARGETS);
	m_NumberOfTargets = std::min(m_NumberOfTargets, MAX_NUMBER_OF_TARGETS);

	m_Pose = nullptr;
	m_OutInterpolate = nullptr;
	for (int i = 0; i < MAX_NUMBER_OF_TARGETS; ++i)
	{
		m_Targets[i] = nullptr;
		m_OutScale[i] = nullptr;
		m_Radii[i] = nullptr;
	}

	// Input Node

	const int dim = GetPlugDim();

	m_Pose = AnimationNodeInCreate(0, "Pose", (dim == 3) ? ANIMATIONNODE_TYPE_VECTOR : ANIMATIONNODE_TYPE_VECTOR_4);

	char buffer[64];

	for (int i = 0; i < m_NumberOfTargets; ++i)
	{
		sprintf_s(buffer, sizeof(char)* 64, "Target %i", i);
		m_Targets[i] = AnimationNodeInCreate(i+2, buffer, (dim==3) ? ANIMATIONNODE_TYPE_VECTOR : ANIMATIONNODE_TYPE_VECTOR_4);
	}

	// local radii
	for (int i = 0; i < m_NumberOfTargets; ++i)
	{
		sprintf_s(buffer, sizeof(char)* 64, "Radius %i", i);
		m_Radii[i] = AnimationNodeInCreate(i+100, buffer, ANIMATIONNODE_TYPE_NUMBER);
	}

	// Output Nodes

	for (int i = 0; i < m_NumberOfTargets; ++i)
	{
		sprintf_s(buffer, sizeof(char)* 64, "OutScale %i", i);
		m_OutScale[i] = AnimationNodeOutCreate(i+200, buffer, ANIMATIONNODE_TYPE_NUMBER);
	}

    m_OutInterpolate = AnimationNodeOutCreate ( 1, "OutInterpolate", ANIMATIONNODE_TYPE_NUMBER );

	return true;
}

/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool BoxRBF3::AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo)
{
	using namespace internal;

	double	input[4];	// Transferring vector
	double targets[MAX_NUMBER_OF_TARGETS][4];
	double radii[MAX_NUMBER_OF_TARGETS];
	bool	lStatus;	// Status of input node

	double height, sigma, scale;

	// Read from properties

	const int ftype = FunctionType.AsInt();
	Height.GetData(&height, sizeof(double), pEvaluateInfo);
	Sigma.GetData(&sigma, sizeof(double), pEvaluateInfo);
	Scale.GetData(&scale, sizeof(double), pEvaluateInfo);

	constexpr double scaleFactor = 0.001;

	height *= 0.001;
	sigma *= scaleFactor;
	scale *= 0.01;

	for (int i = 0; i < MAX_NUMBER_OF_TARGETS; ++i)
	{
		radii[i] = sigma;
	}

	// Read the data from the input node

	lStatus = m_Pose->ReadData( input, pEvaluateInfo );

	// If the read was not from a dead node.
	if (!lStatus)
	{
		return false;
	}

	constexpr int dim = 3;
	Eigen::VectorXd inputVec(dim);
	inputVec << input[0], input[1], input[2];

	// compute connected targets
	int count = 0;

	for (int i = 0; i < m_NumberOfTargets; ++i)
	{
		if (m_Targets[i] && m_Targets[i]->ReadData(targets[count], pEvaluateInfo))
		{
			count += 1;
		}

		if (m_Radii[i])
		{
			m_Radii[i]->ReadData(&radii[i], pEvaluateInfo);
		}
	}
		
	if (count == 0)
	{
		return false;
	}

	// Initialize weights and solutions eigenlib vectors
	using RowMat = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
	Eigen::Map<RowMat, 0> targetsVec(targets[0], count, dim);	
	Eigen::Map<Eigen::VectorXd> radiiVec(radii, count);
	Eigen::VectorXd wvec(count);
	
	auto fn_calcDist = [](const auto& v1, const auto& v2) -> double {
		return (v1 - v2).norm();
	};

	// pre-normalize pose input vector
	inputVec *= scale;
	// Pre-normalize targets outside the loop
	targetsVec *= scale;
	
	Eigen::VectorXd weights(count);

	for (int i = 0; i < count; ++i)
	{
		const double d = fn_calcDist(inputVec, targetsVec.row(i));
		const double r = (UseGlobalSigma) ? sigma : scaleFactor * radiiVec[i];
		double val = RBF(d, height, r, ftype); // use local radius for each target instead of global sigma
		if (val < 1e-4) val = 0.0;
		
		weights[i] = val;
	}

	// normalize weights
	double sum = weights.sum();
	if (sum > 1e-12) weights /= sum;

	// Output
	
	for (int i = 0; i < count; ++i)
	{
		m_OutScale[i]->WriteData(&weights[i], pEvaluateInfo);
	}

	m_OutInterpolate->WriteData(&sum, pEvaluateInfo);
	AnimationNodesOutDisableIfNotWritten(pEvaluateInfo);
	return true;
}

double BoxRBF3::RBF(double input, double height, double sigma, int ftype) const
{
	sigma = std::max(sigma, 1e-8);

	switch (ftype)
	{
	case EFunctionType::eLinear:
		return input * sigma;

	case EFunctionType::eGaussian:
		return std::exp(-(input * input) / (sigma * sigma));
		//return height * exp(-(input * input / 2.0 * sigma * sigma));

	case EFunctionType::eMultiquadratic:
		return height * pow((input * input + sigma * sigma), 0.5);

	case EFunctionType::eInverseMultiquadratic:
		return height * pow((input * input + sigma * sigma), -0.5);

	default:
		return 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Quaternion implementation (Vector4)

bool BoxRBF4::AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo)
{
	using namespace internal;

	double	input[4];	// Transferring vector
	double targets[MAX_NUMBER_OF_TARGETS][4];
	double radii[MAX_NUMBER_OF_TARGETS];
	bool	lStatus;	// Status of input node

	double height, sigma, scale;

	// Read from properties

	const int ftype = FunctionType.AsInt();
	Height.GetData(&height, sizeof(double), pEvaluateInfo);
	Sigma.GetData(&sigma, sizeof(double), pEvaluateInfo);
	Scale.GetData(&scale, sizeof(double), pEvaluateInfo);

	height *= 0.001;
	sigma *= 0.001;
	scale *= 0.01;

	for (int i=0; i<MAX_NUMBER_OF_TARGETS; ++i)
	{
		radii[i] = sigma;
	}

	// Read the data from the input node

	lStatus = m_Pose->ReadData(input, pEvaluateInfo);

	// If the read was not from a dead node.
	if (!lStatus)
	{
		return false;
	}

	constexpr int dim = 4;
	Eigen::VectorXd inputVec(dim);
	inputVec << input[0], input[1], input[2], input[3];

	// compute connected targets
	int count = 0;

	for (int i = 0; i < m_NumberOfTargets; ++i)
	{
		if (m_Targets[i] && m_Targets[i]->ReadData(targets[count], pEvaluateInfo))
		{
			count += 1;
		}

		if (m_Radii[i])
		{
			m_Radii[i]->ReadData(&radii[i], pEvaluateInfo);
		}
	}

	if (count == 0)
	{
		return false;
	}

	// Build eigenlib matrix
	Eigen::MatrixXd A(count, count); // NxN

	// Initialize weights and solutions eigenlib vectors
	using RowMat = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
	using Stride = Eigen::Stride<Eigen::Dynamic, Eigen::Dynamic>;
	Eigen::Map<RowMat, 0, Stride> targetsVec(targets[0], count, dim, Stride(4, 1));
	Eigen::Map<Eigen::VectorXd> radiiVec(radii, count);
	Eigen::VectorXd wvec(count);

	auto fn_calcDist = [dim](const auto& v1, const auto& v2) -> double 
	{	
		const Eigen::Quaterniond quat1(v1[3], v1[0], v1[1], v1[2]);
		const Eigen::Quaterniond quat2(v2[3], v2[0], v2[1], v2[2]);
		return quatDist01(quat1, quat2);
	};

	// RBF kernel on pose distance
	for (int i = 0; i < count; ++i)
	{
		Eigen::Vector4d vec = targetsVec.row(i);
		const double r = (UseGlobalSigma) ? sigma : radiiVec[i] * 0.001;
		for (int j = 0; j < count; ++j)
		{
			Eigen::Vector4d jvec = targetsVec.row(j);
			double val = RBF(fn_calcDist(vec, jvec), height, r, ftype);

			A(i, j) = val;
		}
	}

	//
	// regularization for stability (optional, can cause bias if too large). Uncomment and adjust lambda if you encounter instability issues.
	//constexpr double lambda = 1e-8;              // start tiny; increase if still unstable
	//A.diagonal().array() += lambda;

	// Solve linear system with colPivHouseholderQr()
	// W = A^{-1} (conceptually). Each column k corresponds to coefficients for weight_k.
	Eigen::MatrixXd I = Eigen::MatrixXd::Identity(count, count);
	Eigen::MatrixXd W = A.colPivHouseholderQr().solve(I);

	Eigen::VectorXd Phi(count);
	for (int i = 0; i < count; ++i)
	{
		const double r = (UseGlobalSigma) ? sigma : radiiVec[i] * 0.001;
		Phi[i] = RBF(fn_calcDist(inputVec, targetsVec.row(i)), height, r, ftype);
	}

	Eigen::VectorXd weights = W.transpose() * Phi;

	// normalize weights
	double sum = weights.sum();
	if (sum > 1e-12) weights /= sum;

	// Output

	for (int i = 0; i < count; ++i)
	{
		m_OutScale[i]->WriteData(&weights[i], pEvaluateInfo);
	}

	m_OutInterpolate->WriteData(&sum, pEvaluateInfo);
	AnimationNodesOutDisableIfNotWritten(pEvaluateInfo);
	return true;
}