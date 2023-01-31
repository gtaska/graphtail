#pragma once

#include "Config.h"
#include "CSVTail.h"

namespace graphtail
{

	class Graphs
		: public CSVTail::IListener
	{
	public:
		struct Data
		{
			Data(
				const char*													aId)
				: m_id(aId)
				, m_min(0)
				, m_max(0)
				, m_sum(0)
			{

			}

			void
			Reset()
			{
				m_min = 0;
				m_max = 0;
				m_sum = 0;
				m_values.clear();
			}

			void
			AddValue(
				float														aValue)
			{
				if(m_values.size() > 0)
				{
					m_min = std::min(m_min, aValue);
					m_max = std::max(m_max, aValue);
				}
				else
				{
					m_min = aValue;
					m_max = aValue;
				}

				m_sum += aValue;
				m_values.push_back(aValue);
			}

			// Public data
			std::string							m_id;
			std::vector<float>					m_values;
			float								m_min;
			float								m_max;
			float								m_sum;
		};

		struct DataGroup
		{
			DataGroup(
				const Config::Group*										aConfig = NULL)
				: m_config(aConfig)
			{

			}

			Data*
			CreateData(
				const char*													aId)
			{
				m_data.push_back(std::make_unique<Data>(aId));
				return m_data[m_data.size() - 1].get();
			}			

			float
			GetMin() const
			{
				if(m_config != NULL && m_config->m_config.m_yMin.has_value())
					return m_config->m_config.m_yMin.value();

				std::optional<float> value;

				for(const std::unique_ptr<Data>& data : m_data)
				{
					if(value.has_value())
						value = std::min<float>(value.value(), data->m_min);
					else
						value = data->m_min;
				}

				return value.has_value() ? value.value() : 0.0f;
			}

			float
			GetMax() const
			{
				if(m_config != NULL && m_config->m_config.m_yMax.has_value())
					return m_config->m_config.m_yMax.value();

				std::optional<float> value;

				for(const std::unique_ptr<Data>& data : m_data)
				{
					if(value.has_value())
						value = std::max<float>(value.value(), data->m_max);
					else
						value = data->m_max;
				}

				return value.has_value() ? value.value() : 0.0f;
			}

			// Public data
			const Config::Group*				m_config;

			std::vector<std::unique_ptr<Data>>	m_data;
		};

														Graphs(
															const Config*	aConfig);
		virtual											~Graphs();

		// CSVTail::IListener implementation
		void											OnDataReset(
															const char*		aId) override;
		void											OnData(
															const char*		aId,
															float			aValue) override;

		// Data access
		const std::vector<std::unique_ptr<DataGroup>>&	GetDataGroups() const { return m_dataGroups; }
		uint32_t										GetVersion() const { return m_version; }

	private:

		const Config*							m_config;
		
		std::vector<std::unique_ptr<DataGroup>>	m_dataGroups;
		std::unordered_map<std::string, Data*>	m_dataIndex;

		uint32_t								m_version;

		Data*											_GetData(
															const char*		aId);
		DataGroup*										_CreateDataGroup();
	};

}