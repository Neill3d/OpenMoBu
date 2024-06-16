
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: fb_shader_particles_association.cpp
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "fb_shader_particles_association.h"

#include "fb_shader_particles_shader.h"
#include "model_force_gravity.h"
#include "model_force_motor.h"
#include "model_force_wind.h"

#include "model_collision_sphere.h"
#include "model_collision_terrain.h"

FBApplyManagerRuleImplementation(KShaderParticlesAssociation);
FBRegisterApplyManagerRule( KShaderParticlesAssociation, "KShaderParticlesAssociation", "Apply Manager Rule for the GPU Particles Shader");



/////////////////////////////////////////////////////////////////////////////////////////////////////////
//

bool KShaderParticlesAssociation::IsValidSrc( FBComponent *pSrc)
{
	if (FBIS(pSrc, ForceGravity) || FBIS(pSrc, ForceMotor) || FBIS(pSrc, ForceWind) || FBIS(pSrc, CollisionSphere) || FBIS(pSrc, CollisionTerrain))
	{
		return true;
	}
	else
	if (FBIS(pSrc, FBModel) )
	{
		return true;
	}
	else if (FBIS(pSrc, GPUshader_Particles) )
	{
		return true;
	}

	return false;
}

bool KShaderParticlesAssociation::IsValidConnection( FBComponent* pSrc, FBComponent*& pDst, bool& pAllowMultiple)
{
	if (FBIS(pDst, GPUshader_Particles) )
	{
		pAllowMultiple = true;
		GPUshader_Particles *pShader = (GPUshader_Particles*) pDst;
		
		if (FBIS(pSrc, ForceGravity) || FBIS(pSrc, ForceMotor) || FBIS(pSrc, ForceWind))
		{
			if (pShader->Forces.Find(pSrc) < 0)
			{
				FBConnect( pSrc, &pShader->Forces );
			}
		}
		else if (FBIS(pSrc, CollisionSphere) || FBIS(pSrc, CollisionTerrain))
		{
			if (pShader->Collisions.Find(pSrc) < 0)
			{
				FBConnect( pSrc, &pShader->Collisions );
			}
		}
		else if (FBIS(pSrc, FBModel) )
		{
			pAllowMultiple = false;

			POINT point;
			GetCursorPos(&point);

			FBString itemText("Insert As An Instance Object ?");
			if (pShader->InstanceObject.GetCount() > 0)
			{
				itemText = "Replace The Instance object ?";
			}

			std::unique_ptr<FBGenericMenu> menu(new FBGenericMenu() );
			menu.get()->InsertLast(itemText, 1, nullptr);

			auto result = menu.get()->Execute(point.x, point.y);
			if (result != nullptr)
			{
				pShader->InstanceObject.RemoveAll();
				FBConnect( pSrc, &pShader->InstanceObject );
			}
		}
	}
	else if (FBIS(pSrc, GPUshader_Particles))
	{
		pAllowMultiple = true;
		GPUshader_Particles *pShader = (GPUshader_Particles*) pSrc;

		if (FBIS(pDst, ForceGravity) || FBIS(pDst, ForceMotor) || FBIS(pDst, ForceWind))
		{
			if (pShader->Forces.Find(pDst) < 0)
			{
				FBConnect( pDst, &pShader->Forces );
			}
		}
		else if (FBIS(pDst, CollisionSphere) || FBIS(pDst, CollisionTerrain))
		{
			if (pShader->Collisions.Find(pDst) < 0)
			{
				FBConnect( pDst, &pShader->Collisions );
			}
		}

		// this one is used to assign shader for the model (models)
		/*
		else if (FBIS(pDst, FBModel) )
		{
			pAllowMultiple = false;

			POINT point;
			GetCursorPos(&point);

			FBString itemText("Insert As An Instance Object ?");
			if (pShader->InstanceObject.GetCount() > 0)
			{
				itemText = "Replace The Instance object ?";
			}

			std::auto_ptr<FBGenericMenu> menu(new FBGenericMenu() );
			menu.get()->InsertLast(itemText, 1, nullptr);

			auto result = menu.get()->Execute(point.x, point.y);
			if (result != nullptr)
			{
				pShader->InstanceObject.RemoveAll();
				FBConnect( pDst, &pShader->InstanceObject );
			}
		}
		*/
	}
	
	return false;
}

bool KShaderParticlesAssociation::MenuBuild( FBAMMenu* pAMMenu, FBComponent* pFocusedObject)
{
	
	return false;
}

bool KShaderParticlesAssociation::MenuAction( int pMenuId, FBComponent* pFocusedObject)
{
	bool result = false;

	
	return result;
}
