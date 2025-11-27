#pragma once

/**	\file	shaderpropertystorage.h

Sergei <Neill3d> Solokhin 2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

#include "effectshaderconnections.h"
#include <atomic>
#include <memory>
#include <unordered_map>
#include <map>

/**
 * Lock-free double buffering 
 * write property values in evaluation thread, read values in render thread
 */
class ShaderPropertyStorage
{
public:

    /**
     * Compound key combining effect, variation, and property
     * Enables flat map storage with single lookup
     */
    /*
    struct ValueKey
    {
        uint32_t effectHash;
        uint32_t propertyHash;
        int16_t variationIndex;
        int16_t padding; // Alignment

        ValueKey(uint32_t effect, int variation, uint32_t property)
            : effectHash(effect)
            , propertyHash(property)
            , variationIndex(static_cast<int16_t>(variation))
            , padding(0)
        {
        }

        bool operator==(const ValueKey& other) const
        {
            return effectHash == other.effectHash
                && propertyHash == other.propertyHash
                && variationIndex == other.variationIndex;
        }
        bool operator<(const ValueKey& other) const
        {
            if (effectHash != other.effectHash) return effectHash < other.effectHash;
            if (variationIndex != other.variationIndex) return variationIndex < other.variationIndex;
            return propertyHash < other.propertyHash;
        }
    };

    struct ValueKeyHash
    {
        size_t operator()(const ValueKey& key) const
        {
            // Combine hashes efficiently
            size_t h = key.effectHash;
            h ^= key.propertyHash + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= static_cast<size_t>(key.variationIndex) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };
    */
public:
    ShaderPropertyStorage()
        : mReadIndex(0)
    {
        //mBuffers[0].reserve(200);
        //mBuffers[1].reserve(200);
    }

    // Same ValueKey structure as above
    using PropertyValueMap = std::vector<IEffectShaderConnections::ShaderPropertyValue>;
    using EffectMap = std::unordered_map<uint32_t, PropertyValueMap>;

    /**
     * Write to inactive buffer (no locks needed)
     */
    PropertyValueMap& GetWritePropertyMap(uint32_t effectHash)
    {
        assert(effectHash != 0);
        const int writeIndex = 1 - mReadIndex.load(std::memory_order_relaxed);
        return mBuffers[writeIndex][effectHash];
    }

    void WriteValue(uint32_t effectHash, IEffectShaderConnections::ShaderProperty& property, float valueIn)
    {
        PropertyValueMap& propertyMap = GetWritePropertyMap(effectHash);
        IEffectShaderConnections::ShaderPropertyValue propValue(property.GetType());
        propValue.SetValue(valueIn);
        propertyMap.emplace_back(std::move(propValue));
    }

    const PropertyValueMap* GetReadPropertyMap(uint32_t effectHash) const
    {
        assert(effectHash != 0);
        const int readIndex = mReadIndex.load(std::memory_order_relaxed);
        auto it = mBuffers[readIndex].find(effectHash);
        return (it != end(mBuffers[readIndex])) ? &it->second : nullptr;
    }
    
    IEffectShaderConnections::ShaderPropertyValue& GetWriteValue(
        uint32_t effectHash, IEffectShaderConnections::ShaderProperty& property)
    {
        assert(propertyHash != 0);
        auto& writeMap = GetWritePropertyMap(effectHash);
        writeMap.emplace_back(property.GetDefaultValue());

        IEffectShaderConnections::ShaderPropertyValue& result = writeMap.back();
        return result;
    }
    
    /**
     * Atomic swap - zero contention
     */
    void CommitWrite(uint64_t timestamp)
    {
        int writeIndex = 1 - mReadIndex.load(std::memory_order_relaxed);
        mTimestamps[writeIndex] = timestamp;

        // Atomic swap
        mReadIndex.store(writeIndex, std::memory_order_release);
    }

    /**
     * Read from stable buffer (render thread)
     */
    /*
    const IEffectShaderConnections::ShaderPropertyValue* GetReadValue(
        uint32_t effectHash, uint32_t propertyHash) const
    {
        assert(propertyHash != 0);
        int readIndex = mReadIndex.load(std::memory_order_acquire);
        ValueKey key(effectHash, variation, propertyHash);
        auto it = mBuffers[readIndex].find(key);
        return (it != mBuffers[readIndex].end()) ? &it->second : nullptr;
    }

    void CopyValuesToEffect(
        uint32_t effectHash, int variation,
        std::unordered_map<uint32_t, IEffectShaderConnections::ShaderProperty>& properties) const
    {
        int readIndex = mReadIndex.load(std::memory_order_acquire);
        const auto& buffer = mBuffers[readIndex];

        // Create range bounds
        ValueKey lowerBound(effectHash, variation, 0);
        ValueKey upperBound(effectHash, variation + 1, 0);

        // Iterate only relevant range (cache-friendly sequential access)
        auto start = buffer.lower_bound(lowerBound);
        auto end = buffer.lower_bound(upperBound);

        for (auto it = start; it != end; ++it)
        {
            assert(it->first.propertyHash != 0);
            auto propIt = properties.find(it->first.propertyHash);
            if (propIt != properties.end())
            {
                propIt->second.GetWriteValue() = it->second;
            }
        }
        
        
        for (auto& [propHash, prop] : properties)
        {
            ValueKey key(effectHash, variation, propHash);
            auto it = buffer.find(key);
            if (it != buffer.end())
            {
                prop.GetWriteValue() = it->second;
            }
        }
    }
    */
    size_t GetMemoryUsage() const
    {
        // TODO:
        return 0;
        //return (mBuffers[0].size() + mBuffers[1].size()) *
        //    (sizeof(ValueKey) + sizeof(IEffectShaderConnections::ShaderPropertyValue));
    }

private:
    std::array<EffectMap, 2> mBuffers;
    std::array<uint64_t, 2> mTimestamps = { 0ull, 0ull };
    std::atomic<int> mReadIndex;
};
