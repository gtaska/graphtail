#pragma once

namespace graphtail
{

	class FontData
	{
	public: 
						FontData();
						~FontData();

		// Data access
		const void*		GetData() const { return m_data; }
		size_t			GetSize() const { return m_size; }

	private:

		uint8_t*		m_data;
		size_t			m_size;
	};

}