#include "Base.h"

#include "Config.h"
#include "CSVTail.h"
#include "ErrorUtils.h"

namespace graphtail
{

	CSVTail::CSVTail(
		const char*		aPath,
		IListener*		aListener,
		const Config*	aConfig)
		: m_path(aPath)
		, m_listener(aListener)
		, m_config(aConfig)
		, m_fd(-1)
		, m_fileSize(0)
		, m_parseBufferBytes(0)
		, m_currentColumnIndex(0)
		, m_hasHeaders(false)
		, m_lineNum(0)
	{
		GRAPHTAIL_ASSERT(m_listener != NULL);
	}
	
	CSVTail::~CSVTail()
	{
		_CloseFile();
	}

	void	
	CSVTail::Update()
	{
		if(m_fd == -1 && m_timer.HasExpired())
			_OpenFile();

		if(m_fd != -1)
			_ReadFile();
	}

	//-----------------------------------------------------------------------------

	void				
	CSVTail::_OpenFile()
	{
		int flags = O_RDONLY;

		m_fd = open(m_path.c_str(), flags);

		if (m_fd == -1)
		{
			// Try opening it again in a second
			_Warning("Unable to open file for input.");

			m_timer.SetTimeout(1000);
		}
		else
		{
			m_fileSize = 0;
		}

		m_lineNum = 1;
	}
	
	void				
	CSVTail::_ReadFile()
	{
		GRAPHTAIL_ASSERT(m_fd != -1);

		bool shouldRead = false;

		{
			struct stat s;
			int result = fstat(m_fd, &s);
			if (result == -1)
			{
				_ResetFile();
				_CloseFile();
			}				
			else
			{
				size_t newFileSize = (size_t)s.st_size;

				if(newFileSize > m_fileSize)
				{
					shouldRead = true;
				}
				else if(newFileSize < m_fileSize)
				{
					_ResetFile();
					_CloseFile();
				}

				m_fileSize = newFileSize;
			}
		}

		while(shouldRead)
		{
			char buffer[1024];
			int result = read(m_fd, buffer, sizeof(buffer));
			if(result == 0)
			{
				shouldRead = false;
			}
			else if(result > 0)
			{
				_ParseBuffer(buffer, (size_t)result);
			}
			else
			{
				_ResetFile();
				_CloseFile();

				shouldRead = false;
			}
		}
	}

	void				
	CSVTail::_ParseBuffer(
		const char*		aBuffer,
		size_t			aBufferSize)
	{
		for(size_t i = 0; i < aBufferSize; i++)
		{
			char c = aBuffer[i];

			if(c == '\n')
				m_lineNum++;

			if(c == m_config->m_columnDelimiter)
			{
				_ParseBufferFlushColumn();

				m_currentColumnIndex++;
			}
			else if(c == m_config->m_rowDelimiter)
			{
				_ParseBufferFlushColumn();

				m_currentColumnIndex = 0;
				
				if(!m_hasHeaders)
					m_hasHeaders = true;
			}
			else if(m_parseBufferBytes < sizeof(m_parseBuffer) - 1)
			{
				m_parseBuffer[m_parseBufferBytes++] = c;
			}
			else
			{
				_Warning("Column value too large.");
			}
		}
	}

	void				
	CSVTail::_ParseBufferFlushColumn()
	{
		GRAPHTAIL_ASSERT(m_parseBufferBytes < sizeof(m_parseBuffer));
		m_parseBuffer[m_parseBufferBytes] = '\0';

		if(m_hasHeaders)
		{
			GRAPHTAIL_CHECK(m_currentColumnIndex < m_headers.size(), "Header/column count mismatch.");

			m_listener->OnData(m_headers[m_currentColumnIndex].c_str(), _ParseBufferFloat());
		}
		else
		{
			m_headers.push_back(m_parseBuffer);
		}

		m_parseBufferBytes = 0;
	}

	float
	CSVTail::_ParseBufferFloat()
	{
		// Make sure number is represented in the default C locale - and also see if this is a valid number
		bool notNumber = false;
		for (size_t i = 0; i < m_parseBufferBytes; i++)
		{
			if(m_parseBuffer[i] == ',')
				m_parseBuffer[i] = '.';

			char c = m_parseBuffer[i];
			if(c != '.' && !(c >= '0' && c <='9'))
				notNumber = true;
		}

		if(notNumber || m_parseBufferBytes == 0)
			_Warning("Non-numeric data encountered.");
		
		return (float)atof(m_parseBuffer);
	}

	void				
	CSVTail::_Warning(
		const char*		aMessage)
	{
		if(m_lastWarningMessage != aMessage)
		{
			fprintf(stderr, "%s (line %u): %s\n", m_path.c_str(), m_lineNum, aMessage);

			m_lastWarningMessage = aMessage;
		}
	}

	void				
	CSVTail::_CloseFile()
	{
		if (m_fd != -1)
		{
			int result = close(m_fd);
			GRAPHTAIL_CHECK(result != -1, "close() failed: %d (path: %s)", errno, m_path.c_str());

			m_fd = -1;
		}
	}

	void
	CSVTail::_ResetFile()
	{
		for(const std::string& header : m_headers)
			m_listener->OnDataReset(header.c_str());

		m_parseBufferBytes = 0;
		m_currentColumnIndex = 0;
		m_hasHeaders = false;
		m_headers.clear();
	}

}
