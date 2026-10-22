#pragma once

/**	\file	posteffect_scenetexture.h

Sergei <Neill3d> Solokhin 2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- SDK include
#include <fbsdk/fbsdk.h>

#include <GL/glew.h>
#include <Windows.h>

#include "gl_texture_trace.h"

#include <mutex>
#include <unordered_map>

/**
* A scene texture (FBTexture) that is sampled by an effect has the gl texture object managed by MotionBuilder.
* The TextureOGLId property is not enough to trust: gl contexts of the viewer and of the video rendering share their objects,
*  but MotionBuilder could free and upload its textures when the context is switched, and the IDs are given to other textures.
*  The property keeps the old value, and the old ID could point to some other scene texture.
*
* The tracker remembers in which gl context and with which ID a texture was initialized last time and calls OGLInit again
*  on a switch of the context (a switch is rare, it's not a per frame cost) or when the ID is not valid anymore.
*/
class SceneTextureTracker
{
public:

	/// @return a gl texture object of the texture that is valid in the current gl context, 0 if the texture is not available
	static GLuint Acquire(FBTexture* texture)
	{
		if (!texture)
			return 0;

		HGLRC context = wglGetCurrentContext();
		GLuint textureId = static_cast<GLuint>(static_cast<int>(texture->TextureOGLId));

		Entry entry;
		{
			std::lock_guard<std::mutex> lock(GetMutex());
			entry = GetEntries()[texture];
		}

		const bool isContextChanged = (entry.context != context);
		const bool isIdChanged = (entry.id != textureId);
		const bool isInvalid = (textureId == 0) || (glIsTexture(textureId) != GL_TRUE);

		if (isContextChanged || isIdChanged || isInvalid)
		{
			const GLuint oldTextureId = textureId;

			// NOTE: not under the lock, it could be a long call and other contexts are rendering at the same time
			texture->OGLInit();
			textureId = static_cast<GLuint>(static_cast<int>(texture->TextureOGLId));

			if (GLTrace::Enabled())
			{
				LOGI("[GL TRACE] scene texture %s is initialized: context %p -> %p, ID %u -> %u (context changed %d, ID changed %d, invalid %d)\n",
					texture->LongName.AsString(), (void*)entry.context, (void*)context, oldTextureId, textureId,
					isContextChanged ? 1 : 0, isIdChanged ? 1 : 0, isInvalid ? 1 : 0);
			}

			std::lock_guard<std::mutex> lock(GetMutex());
			GetEntries()[texture] = Entry{ context, textureId };
		}

		return textureId;
	}

private:

	struct Entry
	{
		HGLRC	context{ nullptr };
		GLuint	id{ 0 };
	};

	static std::mutex& GetMutex()
	{
		static std::mutex mutex;
		return mutex;
	}

	// NOTE: never destroyed on purpose, textures are used during the application exit
	static std::unordered_map<FBTexture*, Entry>& GetEntries()
	{
		static auto* entries = new std::unordered_map<FBTexture*, Entry>();
		return *entries;
	}
};
