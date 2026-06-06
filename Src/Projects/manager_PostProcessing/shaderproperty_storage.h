#pragma once

/**	\file	shaderproperty_storage.h

Sergei <Neill3d> Solokhin 2025-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

#include "shaderproperty_value.h"
#include <atomic>
#include <memory>
#include <unordered_map>
#include <map>
#include <mobu_logging.h>

/**
 * Lock-free double buffering 
 * write property values in evaluation thread, read values in render thread
 */
class ShaderPropertyStorage
{
public:
    ShaderPropertyStorage()
        : mReadIndex(1) // start with buffer 0 as write, 1 as read
    {
    }

    // Same ValueKey structure as above
    using PropertyValueMap = std::vector<ShaderPropertyValue>;
    using EffectMap = std::unordered_map<uint32_t, PropertyValueMap>;

    /**
     * Write to inactive buffer (no locks needed)
     */
    PropertyValueMap& GetWritePropertyMap(uint32_t effectHash)
    {
        VERIFY(effectHash != 0);
        const int writeIndex = 1 - mReadIndex.load(std::memory_order_relaxed);
        return mBuffers[writeIndex][effectHash];
    }

    EffectMap& GetWriteEffectMap()
    {
        const int writeIndex = 1 - mReadIndex.load(std::memory_order_relaxed);
        return mBuffers[writeIndex];
	}

    void WriteValue(uint32_t effectHash, const ShaderPropertyValue& defaultPropertyValue, float valueIn)
    {
        PropertyValueMap& propertyMap = GetWritePropertyMap(effectHash);
        ShaderPropertyValue propValue(defaultPropertyValue);
        propValue.SetValue(valueIn);
        propertyMap.emplace_back(std::move(propValue));
    }

    static bool CheckPropertyExists(const PropertyValueMap& valuesMap, uint32_t propertyHash)
    {
        auto iter = std::find_if(cbegin(valuesMap), cend(valuesMap), [propertyHash](const ShaderPropertyValue& value)
            {
                return value.GetNameHash() == propertyHash;
            });
		return iter != cend(valuesMap);
	}

    [[nodiscard]] const PropertyValueMap* GetReadPropertyMap(uint32_t effectHash) const
    {
        VERIFY(effectHash != 0);
        const int readIndex = mReadIndex.load(std::memory_order_relaxed);
        auto it = mBuffers[readIndex].find(effectHash);
        return (it != end(mBuffers[readIndex])) ? &it->second : nullptr;
    }

    [[nodiscard]] EffectMap& GetReadEffectMap()
    {
        const int readIndex = mReadIndex.load(std::memory_order_relaxed);
        return mBuffers[readIndex];
    }

    [[nodiscard]] PropertyValueMap* GetReadPropertyMap(uint32_t effectHash)
    {
        VERIFY(effectHash != 0);
        const int readIndex = mReadIndex.load(std::memory_order_relaxed);
        auto it = mBuffers[readIndex].find(effectHash);
        return (it != end(mBuffers[readIndex])) ? &it->second : nullptr;
    }
    
    [[nodiscard]] ShaderPropertyValue& GetWriteValue(uint32_t effectHash, const ShaderPropertyValue& defaultPropertyValue)
    {
        VERIFY(effectHash != 0);
        auto& writeMap = GetWritePropertyMap(effectHash);
        writeMap.emplace_back(defaultPropertyValue);

        ShaderPropertyValue& result = writeMap.back();
        return result;
    }
    
    /**
     * Atomic swap - zero contention
     */
    void CommitWrite(uint64_t timestamp)
    {
        const int newReadIndex = 1 - mReadIndex.load(std::memory_order_relaxed);
        mTimestamps[newReadIndex] = timestamp;
        mReadIndex.store(newReadIndex, std::memory_order_release);

        const int newWriteIndex = 1 - newReadIndex;
        mBuffers[newWriteIndex].clear();
    }

    void Clear()
    {
        mBuffers[0].clear();
        mBuffers[1].clear();
		mReadIndex.store(1, std::memory_order_relaxed);
	}

	// Get memory usage of both buffers in bytes
    size_t GetMemoryUsage() const
    {
        size_t totalSize = 0;
        for (const auto& effectPair : mBuffers[0])
        {
            totalSize += sizeof(effectPair.first); // size of the key
            totalSize += effectPair.second.size() * sizeof(ShaderPropertyValue); // size of the values
        }
        for (const auto& effectPair : mBuffers[1])
        {
            totalSize += sizeof(effectPair.first); // size of the key
            totalSize += effectPair.second.size() * sizeof(ShaderPropertyValue); // size of the values
        }
		return totalSize;
    }

private:
    std::array<EffectMap, 2> mBuffers;
    std::array<uint64_t, 2> mTimestamps = { 0ull, 0ull };
    std::atomic<int> mReadIndex;
};
