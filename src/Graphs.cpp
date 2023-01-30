#include "Base.h"

#include "Graphs.h"

namespace graphtail
{

	Graphs::Graphs(
		const Config*		aConfig)
		: m_config(aConfig)
	{
		// Initialize groups
		for(const std::unique_ptr<Config::Group>& configGroup : aConfig->m_groups)
			m_dataGroups.push_back(std::make_unique<DataGroup>(configGroup.get()));
	}
	
	Graphs::~Graphs()
	{

	}

	//-------------------------------------------------------------------------------------

	void	
	Graphs::OnDataReset(
		const char*			aId) 
	{
		_GetData(aId)->Reset();
	}
	
	void	
	Graphs::OnData(
		const char*			aId,
		float				aValue) 
	{
		_GetData(aId)->AddValue(aValue);
	}

	//-------------------------------------------------------------------------------------

	Graphs::Data*
	Graphs::_GetData(
		const char*			aId)
	{
		std::unordered_map<std::string, Data*>::iterator i = m_dataIndex.find(aId);
		if(i != m_dataIndex.end())
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

			data = dataGroup->CreateData(aId);
		}

		m_dataIndex[aId] = data;

		return data;
	}

	Graphs::DataGroup*
	Graphs::_CreateDataGroup()
	{
		m_dataGroups.push_back(std::make_unique<DataGroup>());
		return m_dataGroups[m_dataGroups.size() - 1].get();
	}

}