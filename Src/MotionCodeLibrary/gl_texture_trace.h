#pragma once

/**	\file	gl_texture_trace.h

Sergei <Neill3d> Solokhin 2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

Diagnostics for GL textures that a plugin makes and deletes. It helps to find
 - a texture that is deleted while another gl context is current (IDs are not valid there, a texture of the scene could be deleted)
 - a texture that is deleted by a code that didn't make it
 - gl contexts that share or don't share their objects (see ProbeSharing)
 - a scene texture whose ID belongs to a texture made by the plugin (see CheckSceneTexture)

It is off by default (a disabled call is a check of one cached flag). It is a verbose output, the level comes from
 - the POSTPROCESSING_GL_TRACE environment variable (0 - off, 1 - problems and probes, 2 - every make and delete)
 - or, when the variable is not set, the OPENMOBU_VERBOSE_LOG environment variable (any value other than "0" gives the level 1)
read once before the application starts. Everything goes into the log (see Logger.h)
*/

#include <GL/glew.h>
#include <Windows.h>
#include "Logger.h"

#include <cstdint>
#include <cstdlib>
#include <functional>
#include <mutex>
#include <set>
#include <tuple>
#include <unordered_map>

namespace GLTrace
{
	// NOTE: a plugin makes textures in several contexts, and IDs of different contexts could be the same
	struct Key
	{
		HGLRC	context{ nullptr };
		GLuint	id{ 0 };

		bool operator==(const Key& other) const { return context == other.context && id == other.id; }
	};

	struct KeyHash
	{
		size_t operator()(const Key& key) const
		{
			return std::hash<void*>()(static_cast<void*>(key.context)) * 31u + std::hash<GLuint>()(key.id);
		}
	};

	struct State
	{
		std::mutex mutex;
		std::unordered_map<Key, const char*, KeyHash> textures; //!< a texture and its tag
		// a scene texture warning is printed once per state, not every frame (context, id, gl width, gl height, exists, plugin texture)
		std::set<std::tuple<HGLRC, GLuint, GLint, GLint, bool, bool>> warnedSceneTextures;
	};

	inline State& GetState()
	{
		// never destroyed on purpose, the textures are deleted during the application exit
		static State* state = new State();
		return *state;
	}

	inline int Level()
	{
		static const int level = []() {
			char value[16]{};
			DWORD length = GetEnvironmentVariableA("POSTPROCESSING_GL_TRACE", value, sizeof(value));
			if (length > 0 && length < sizeof(value))
				return std::atoi(value);

			// the common verbose switch, see mobu_logging.h
			length = GetEnvironmentVariableA("OPENMOBU_VERBOSE_LOG", value, sizeof(value));
			return (length > 0 && length < sizeof(value) && !(value[0] == '0' && value[1] == '\0')) ? 1 : 0;
		}();
		return level;
	}

	inline bool Enabled() { return Level() > 0; }

	// call right after glGenTextures
	inline void Gen(GLuint id, const char* tag)
	{
		if (!Enabled() || id == 0)
			return;

		HGLRC context = wglGetCurrentContext();
		State& state = GetState();
		std::lock_guard<std::mutex> lock(state.mutex);

		state.textures[Key{ context, id }] = tag;

		if (Level() > 1)
		{
			LOGI("[GL TRACE] make texture %u (%s), context %p\n", id, tag, (void*)context);
		}
	}

	// call right before glDeleteTextures
	inline void Delete(GLuint id, const char* tag)
	{
		if (!Enabled() || id == 0)
			return;

		HGLRC context = wglGetCurrentContext();
		State& state = GetState();
		std::lock_guard<std::mutex> lock(state.mutex);

		auto it = state.textures.find(Key{ context, id });
		if (it != state.textures.end())
		{
			if (Level() > 1)
			{
				LOGI("[GL TRACE] delete texture %u (%s), context %p\n", id, tag, (void*)context);
			}
			state.textures.erase(it);
			return;
		}

		// look for an owner in other contexts
		for (const auto& pair : state.textures)
		{
			if (pair.first.id == id)
			{
				LOGE("[GL TRACE] WARNING: delete texture %u (%s) in context %p, but it was made as (%s) in context %p\n",
					id, tag, (void*)context, pair.second, (void*)pair.first.context);
				return;
			}
		}

		LOGE("[GL TRACE] WARNING: delete texture %u (%s) in context %p that the plugin didn't make (a scene texture, or already deleted)\n",
			id, tag, (void*)context);
	}

	// how many plugin textures of other contexts are visible in the current context (glIsTexture)
	// all visible means the contexts share their objects, none visible means they don't
	inline void ProbeSharing(const char* where)
	{
		if (!Enabled())
			return;

		HGLRC context = wglGetCurrentContext();
		if (!context)
			return;

		State& state = GetState();
		std::lock_guard<std::mutex> lock(state.mutex);

		int own = 0, foreign = 0, visible = 0;
		for (const auto& pair : state.textures)
		{
			if (pair.first.context == context)
			{
				++own;
				continue;
			}

			++foreign;
			if (glIsTexture(pair.first.id))
			{
				++visible;
			}
		}

		const char* verdict = (foreign == 0) ? "nothing to compare" : (visible == foreign) ? "SHARED"
			: (visible == 0) ? "NOT shared" : "partly visible";
		LOGI("[GL TRACE] sharing probe at %s: context %p, own textures %d, textures of other contexts %d, visible here %d (%s)\n",
			where, (void*)context, own, foreign, visible, verdict);
	}

	// probe the sharing once per switch of the current context of the calling thread (viewer, file render, etc.)
	inline void ProbeOnContextChange(const char* where)
	{
		if (!Enabled())
			return;

		thread_local HGLRC lastContext = nullptr;
		HGLRC context = wglGetCurrentContext();
		if (context != lastContext)
		{
			lastContext = context;
			ProbeSharing(where);
		}
	}

	// a texture of the scene that is going to be sampled, tells if its ID is not valid or is an ID of a plugin texture
	inline void CheckSceneTexture(const char* name, GLuint id, int sceneWidth, int sceneHeight)
	{
		if (!Enabled() || id == 0)
			return;

		HGLRC context = wglGetCurrentContext();
		GLint glWidth = 0, glHeight = 0;
		const bool exists = (glIsTexture(id) == GL_TRUE);
		if (exists)
		{
			GLint lastTexture = 0;
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);
			glBindTexture(GL_TEXTURE_2D, id);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &glWidth);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &glHeight);
			glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(lastTexture));
		}

		const char* pluginTag = nullptr;
		{
			State& state = GetState();
			std::lock_guard<std::mutex> lock(state.mutex);
			auto it = state.textures.find(Key{ context, id });
			if (it != state.textures.end())
			{
				pluginTag = it->second;
			}
		}

		const bool isSizeMismatch = exists && sceneWidth > 0 && sceneHeight > 0 && (glWidth != sceneWidth || glHeight != sceneHeight);

		if (pluginTag || !exists || isSizeMismatch)
		{
			{
				State& state = GetState();
				std::lock_guard<std::mutex> lock(state.mutex);
				if (!state.warnedSceneTextures.emplace(context, id, glWidth, glHeight, exists, pluginTag != nullptr).second)
					return;
			}
			LOGE("[GL TRACE] WARNING: scene texture %s, ID %u, context %p: exists %d, gl size %dx%d, scene size %dx%d, plugin texture: %s\n",
				name, id, (void*)context, exists ? 1 : 0, glWidth, glHeight, sceneWidth, sceneHeight, pluginTag ? pluginTag : "no");
		}
		else if (Level() > 1)
		{
			LOGI("[GL TRACE] scene texture %s, ID %u, context %p is ok, size %dx%d\n", name, id, (void*)context, glWidth, glHeight);
		}
	}
}
