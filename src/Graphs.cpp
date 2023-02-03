#include "Base.h"

#include "ErrorUtils.h"
#include "Graphs.h"

namespace graphtail
{

	Graphs::Graphs(
		const Config*		aConfig)
		: m_config(aConfig)
		, m_version(0)
	{
		m_defaultGroupConfig.m_config.ApplyDefaults(m_config->m_defaultGroupConfig);

		// Initialize groups
		for(const std::unique_ptr<Config::Group>& configGroup : aConfig->m_groups)
		{
			std::unique_ptr<DataGroup> group = std::make_unique<DataGroup>(configGroup.get());

			if(group->m_config->m_histogram)
			{
				std::unique_ptr<Data> histogram = std::make_unique<Data>();

				for(const std::string& id : group->m_config->m_histogram->m_ids)
					m_dataTable.insert(std::pair<std::string, Data*>(id, histogram.get()));

				group->m_data.push_back(std::move(histogram));
			}

			m_dataGroups.push_back(std::move(group));
		}
	}
	
	Graphs::~Graphs()
	{

	}

	//-------------------------------------------------------------------------------------

	void	
	Graphs::OnDataReset(
		const char*			aId) 
	{
		std::unordered_map<std::string, Data*>::iterator i = m_dataTable.find(aId);
		if (i == m_dataTable.end())
			return;

		Data* data = i->second;

		if(data->m_isInAutoGroup)
		{
			// This data isn't in a configuration-defined group. Find it's group
			// and remove it too.
			bool found = false;

			for (size_t j = 0; j < m_dataGroups.size(); j++)
			{
				std::unique_ptr<DataGroup>& dataGroup = m_dataGroups[j];

				if(dataGroup->m_isAutoGroup)
				{
					GRAPHTAIL_ASSERT(dataGroup->m_data.size() == 1);

					if(dataGroup->m_data[0].get() == data)
					{
						m_dataGroups.erase(m_dataGroups.begin() + j);

						found = true;
						break;
					}
				}
			}

			GRAPHTAIL_ASSERT(found);

			m_dataTable.erase(i);
		}
		else
		{
			data->Reset();
		}

		m_version++;
	}
	
	void	
	Graphs::OnData(
		const char*			aId,
		float				aValue) 
	{
		_GetData(aId)->AddValue(aValue);

		m_version++;
	}

	//-------------------------------------------------------------------------------------

	Graphs::Data*
	Graphs::_GetData(
		const char*			aId)
	{
		std::unordered_map<std::string, Data*>::iterator i = m_dataTable.find(aId);
		if(i != m_dataTable.end())
			return i->second;

		Data* data = NULL;

		// See if any group has a id-wildcard that matches this
		for(std::unique_ptr<DataGroup>& dataGroup : m_dataGroups)
		{
			if(dataGroup->m_config != NULL)
			{
				bool matchesWildcard = false;

				for(const std::unique_ptr<Wildcard>& wildcard : dataGroup->m_config->m_idWildcards)
				{
					if(wildcard->Match(aId))
					{
						matchesWildcard = true;
						break;
					}
				}

				if(matchesWildcard)
				{
					data = dataGroup->CreateData(aId);
					break;
				}
			}
		}

		if(data == NULL)
		{
			// No matching group, create a new automatic group
			DataGroup* dataGroup = _CreateDataGroup();

			dataGroup->m_isAutoGroup = true;

			data = dataGroup->CreateData(aId);

			data->m_isInAutoGroup = true;
		}

		m_dataTable[aId] = data;

		return data;
	}

	Graphs::DataGroup*
	Graphs::_CreateDataGroup()
	{
		m_dataGroups.push_back(std::make_unique<DataGroup>(&m_defaultGroupConfig));
		return m_dataGroups[m_dataGroups.size() - 1].get();
	}

}