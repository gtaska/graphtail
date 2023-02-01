#pragma once

#include "Timer.h"

namespace graphtail
{

	struct Config;

	class CSVTail
	{
	public:
		class IListener
		{
		public:
			virtual ~IListener() {}

			// Virtual interface
			virtual void	OnDataReset(
								const char*		aId) = 0;
			virtual void	OnData(
								const char*		aId,
								float			aValue) = 0;
		};

							CSVTail(
								const char*		aPath,
								IListener*		aListener,
								const Config*	aConfig);
							~CSVTail();

		void				Update();

	private:	
		
		std::string					m_path;
		IListener*					m_listener;
		const Config*				m_config;

		int							m_fd;
		size_t						m_fileSize;
		Timer						m_timer;
		std::string					m_lastWarningMessage;
		size_t						m_currentColumnIndex;
		bool						m_hasHeaders;
		std::vector<std::string>	m_headers;
		
		char						m_parseBuffer[256];
		size_t						m_parseBufferBytes;

		uint32_t					m_lineNum;

		void				_OpenFile();
		void				_ReadFile();
		void				_ParseBuffer(
								const char*		aBuffer,
								size_t			aBufferSize);
		void				_ParseBufferFlushColumn();
		float				_ParseBufferFloat();
		void				_Warning(
								const char*		aMessage);
		void				_CloseFile();
		void				_ResetFile();
	};

}