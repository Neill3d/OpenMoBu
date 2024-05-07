
/**	\file	boxRBF.cxx
*	Definition of a RBF Interpolator box class.
*	Port of Maya jsRadial plugin
*
*	Sergei Solokhin (Neill3d) 2018
*/

//--- Class declaration
#include "BoxRBF.h"

//--- Registration defines
#define BOXRBF3__CLASS		BOXRBF3__CLASSNAME
#define BOXRBF3__NAME		BOXRBF3__CLASSSTR
#define	BOXRBF3__LOCATION	"OpenMobu"
#define BOXRBF3__LABEL		"RBF Vector"
#define	BOXRBF3__DESC		"RBF Vector Interpolator"

#define BOXRBF4__CLASS		BOXRBF4__CLASSNAME
#define BOXRBF4__NAME		BOXRBF4__CLASSSTR
#define	BOXRBF4__LOCATION	"OpenMobu"
#define BOXRBF4__LABEL		"RBF Vector4"
#define	BOXRBF4__DESC		"RBF Vector4 Interpolator"

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


const char * FBPropertyBaseEnum<EFunctionType>::mStrings[] = {
	"Gaussian",
	"Multiquadratic",
	"Inverse Multiquadratic",
	0 };

const int ReadCountFromConfig()
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
	FBPropertyPublish(this, RotationMultiply, "Rotation Multiply", nullptr, nullptr);

	Height.SetMin(0.00001);
	Sigma.SetMin(0.00001);

	Height = 10.0;
	Sigma = 10.0;
	RotationMultiply = 100.0;

	m_NumberOfTargets = ReadCountFromConfig();

	m_NumberOfTargets = std::max(m_NumberOfTargets, MIN_NUMBER_OF_TARGETS);
	m_NumberOfTargets = std::min(m_NumberOfTargets, MAX_NUMBER_OF_TARGETS);

	m_Pose = nullptr;
	m_OutInterpolate = nullptr;
	for (int i = 0; i < MAX_NUMBER_OF_TARGETS; ++i)
	{
		m_Targets[i] = nullptr;
		m_OutScale[i] = nullptr;
	}

	// Input Node

	const int dim = GetPlugDim();

	m_Pose = AnimationNodeInCreate(0, "Pose", (dim == 3) ? ANIMATIONNODE_TYPE_VECTOR : ANIMATIONNODE_TYPE_VECTOR_4);

	char buffer[64];

	for (int i = 0; i < m_NumberOfTargets; ++i)
	{
		sprintf_s(buffer, sizeof(char)* 64, "Target %i", i);
		m_Targets[i] = AnimationNodeInCreate(i+i, buffer, (dim==3) ? ANIMATIONNODE_TYPE_VECTOR : ANIMATIONNODE_TYPE_VECTOR_4);
	}

	// Output Nodes

	for (int i = 0; i < m_NumberOfTargets; ++i)
	{
		sprintf_s(buffer, sizeof(char)* 64, "OutScale %i", i);
		m_OutScale[i] = AnimationNodeOutCreate(i+1, buffer, ANIMATIONNODE_TYPE_NUMBER);
	}

    m_OutInterpolate = AnimationNodeOutCreate ( 0, "OutInterpolate", ANIMATIONNODE_TYPE_NUMBER );

	return true;
}


/************************************************
 *	Destruction.
 ************************************************/
void BoxRBF3::FBDestroy()
{
}


/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool BoxRBF3::AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo)
{
	double	pose[4];	// Transferring vector
	double targets[MAX_NUMBER_OF_TARGETS][4];
	bool	lStatus;	// Status of input node

	short ftype;
	double height, sigma, rotMult;

	// Read from properties

	ftype = static_cast<short>(FunctionType.AsInt());
	Height.GetData(&height, sizeof(double), pEvaluateInfo);
	Sigma.GetData(&sigma, sizeof(double), pEvaluateInfo);
	RotationMultiply.GetData(&rotMult, sizeof(double), pEvaluateInfo);

	height *= 0.001;
	sigma *= 0.001;
	rotMult *= 0.01;

	//
	const short dim = GetPlugDim();

	// Read the data from the input node

	lStatus = m_Pose->ReadData( pose, pEvaluateInfo );

	// compute connected targets
	unsigned int count = 0;

	// If the read was not from a dead node.
    if( lStatus ) // && lStatusTargets[0] && lStatusTargets[1] && lStatusTargets[2])
	{
		Eigen::VectorXd poseVec(3);

		for (int i = 0; i < 4; ++i)
		{
			pose[i] *= rotMult;
		}
		
		if (dim == 3)
		{
			poseVec << pose[0], pose[1], pose[2];
		}
		else
		{
			poseVec << pose[0], pose[1], pose[2], pose[3];
		}
		
		
		for (int i = 0; i < MAX_NUMBER_OF_TARGETS; ++i)
		{
			if (m_Targets[i])
			{
				lStatus = m_Targets[i]->ReadData(targets[count], pEvaluateInfo);

				if (lStatus)
				{
					count += 1;
				}
			}
		}
		
		if (count == 0)
		{
			return false;
		}

		// Build eigenlib matrix
		Eigen::MatrixXd mtx(count, count);

		// Initialize weights and solutions eigenlib vectors
		norms_.resize(count);
		vecs_.resize(count);
		Eigen::VectorXd wvec(count);
		Eigen::VectorXd rvec(count);
		rvec << height, height, height;
		
		for (unsigned int i = 0; i < count; ++i)
		{
			// Apply rotMult to "normalize" rotation to translation
			for (int k = 0; k < 4; ++k)
			{
				targets[i][k] *= rotMult;
			}
			
			// Initialize eigenlib vector to scale target spheres
			Eigen::VectorXd vec(dim);
			if (dim == 3)
			{
				vec << targets[i][0], targets[i][1], targets[i][2];
			}
			else
			{
				vec << targets[i][0], targets[i][1], targets[i][2], targets[i][3];
			}
			
			double norm = vec.norm();
			norms_[i] = norm;
			vecs_[i] = vec;
			Eigen::VectorXd distVec = poseVec - vec;
			double dist = fabs(distVec.norm());
			double rbf_scale = RBF(dist, height, sigma, ftype);

			// Populate eigenlib matrix
			for (unsigned int j = 0; j < count; ++j)
			{
				// Get norm of every target and find rbf of inorm - jnorm
				
				// Apply rotMult to "normalize" rotation to translation
				for (int k = 0; k < 4; ++k)
				{
					targets[j][k] *= rotMult;
				}
				
				Eigen::VectorXd jvec(dim);
				if (dim == 3)
				{
					jvec << targets[j][0], targets[j][1], targets[j][2];
				}
				else
				{
					jvec << targets[j][0], targets[j][1], targets[j][2], targets[j][3];
				}
				Eigen::VectorXd diffVec = vec - jvec;

				double diffNorm = fabs(diffVec.norm());
				double val = RBF(diffNorm, height, sigma, ftype);
				mtx(i, j) = val;
			}

			m_OutScale[i]->WriteData(&rbf_scale, pEvaluateInfo);
		}

		// Solve linear system with colPivHouseholderQr()
		wvec = mtx.colPivHouseholderQr().solve(rvec);

		// Sum weighted RBFs
		double sum = 0.0;
		for (unsigned int i = 0; i < count; ++i)
		{
			Eigen::VectorXd diffVec = poseVec - vecs_[i];
			double rbf = RBF(fabs(diffVec.norm()), height, sigma, ftype);
			rbf *= wvec[i];
			sum += rbf;
		}

		m_OutInterpolate->WriteData(&sum, pEvaluateInfo);
		AnimationNodesOutDisableIfNotWritten(pEvaluateInfo);
	    return true;
	}
    return false;
}

double BoxRBF3::RBF(const double &r, const double &height, const double &sigma, const short &ftype)
{
	double out;
	if (ftype == 0)
	{
		// Gaussian
		out = height * exp(-(r*r / 2 * sigma*sigma));
	}
	else if (ftype == 1)
	{
		// Multiquadratic
		out = height * pow((r*r + sigma*sigma), 0.5);
	}
	else if (ftype == 2)
	{
		// Inverse multiquadratic
		out = height * pow((r*r + sigma*sigma), -0.5);
	}
	else
	{
		return 0.0;
	}

	return out;
}