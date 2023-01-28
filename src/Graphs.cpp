#include "Base.h"

#include "Config.h"
#include "Graphs.h"

namespace graphtail
{

	Graphs::Graphs(
		const Config*		aConfig)
		: m_config(aConfig)
	{

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

		for(std::unique_ptr<DataGroup>& dataGroup : m_dataGroups)
		{
			data = dataGroup->FindData(aId);

			if(data != NULL)
				break;
		}

		if(data == NULL)
		{
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