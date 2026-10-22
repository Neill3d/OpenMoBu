#pragma once

/**	\file	shaderproperty_generation.h

Sergei <Neill3d> Solokhin 2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

#include <atomic>
#include <cstdint>

/**
* counts scene changes that could destroy components with FBProperties cached in property schemes
*  (file new / open, merge replace, delete of a post process data or a user effect object)
* A scheme keeps the generation it was associated in, a scheme of an older generation must not read its cached FBProperties
* NOTE: no fbsdk dependency, the counter is a plugin memory only, it's never stored in a scene
*/
class SceneGeneration
{
public:

	//! 0 is never current, it means "not associated"
	static constexpr uint32_t NotAssociated = 0;

	static uint32_t Current() noexcept { return sValue.load(std::memory_order_acquire); }

	// call before (or while) scene components are destroyed
	static void Bump() noexcept
	{
		uint32_t value = sValue.load(std::memory_order_relaxed);
		while (!sValue.compare_exchange_weak(value, Next(value), std::memory_order_acq_rel, std::memory_order_relaxed))
		{}
	}

	// the next generation value, a wrap around skips NotAssociated
	static constexpr uint32_t Next(uint32_t value) noexcept
	{
		return (value + 1 == NotAssociated) ? 1 : value + 1;
	}

	static bool IsCurrent(uint32_t generation) noexcept { return generation != NotAssociated && generation == Current(); }

	// a file new / open / merge is in progress, scene components are being destroyed and made,
	//  pane cameras and post process data must not be used until the change ends
	static void BeginChange() noexcept { sIsChanging.store(true, std::memory_order_release); }
	static bool IsChanging() noexcept { return sIsChanging.load(std::memory_order_acquire); }
	// ends the change, components collected during it are outdated (the generation is bumped)
	// @return false when there was no change in progress
	static bool EndChange() noexcept
	{
		if (!sIsChanging.exchange(false, std::memory_order_acq_rel))
			return false;
		Bump();
		return true;
	}

private:
	static inline std::atomic<uint32_t> sValue{ 1 };
	static inline std::atomic<bool> sIsChanging{ false };
};
