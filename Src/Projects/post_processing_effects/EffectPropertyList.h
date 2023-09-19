#pragma once

#include <vector>
#include <unordered_map>

#include "libraryMain.h"
#include "EffectProperty.h"

namespace PostProcessingEffects
{

	/// <summary>
	/// a holder for property collection
	/// </summary>
	class POST_PROCESSING_EFFECTS_API CEffectPropertyList
	{
	public:
		//! a constructor
		CEffectPropertyList()
		{}

		SEffectProperty&& CreateProperty() const
		{
			return std::move(SEffectProperty());
		}

		void AddProperty(std::string_view name, SEffectProperty&& prop)
		{
			int count = GetCount();
			m_Properties.emplace_back(std::move(prop));
			m_PropertyMap.emplace(name, count);
		}

		int GetCount() const { return static_cast<int>(m_Properties.size()); }

		const SEffectProperty& GetAt(const int index) const { return m_Properties[index]; }

		const SEffectProperty& operator[](int index) const { return m_Properties[index]; }

		const SEffectProperty* Find(std::string_view name) const {
			auto iter = m_PropertyMap.find(name);
			return (iter != end(m_PropertyMap) && iter->second < GetCount()) ? &m_Properties[iter->second] : nullptr;
		}

	private:
		std::vector<SEffectProperty>				m_Properties;
		std::unordered_map<std::string_view, int>	m_PropertyMap;
	};
}
