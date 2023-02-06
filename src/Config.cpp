#include "Base.h"

#include "Config.h"
#include "ErrorUtils.h"
#include "Wildcard.h"

namespace 
{

	struct InputFile
	{
		InputFile(
			const char*									aPath)
		{
			m_file = fopen(aPath, "rb");
			GRAPHTAIL_CHECK(m_file != NULL, "File not found: %s", aPath);

			fseek(m_file, 0, SEEK_END);
			m_size = (size_t)ftell(m_file);
			fseek(m_file, 0, SEEK_SET);

			m_data = new uint8_t[m_size + 1];
			size_t result = fread(m_data, 1, m_size, m_file);
			GRAPHTAIL_CHECK(result == m_size, "Failed to read file: %s", aPath);
			m_data[m_size] = 0;
		}

		~InputFile()
		{
			delete [] m_data;
			fclose(m_file);
		}

		bool
		IsEndStatement(
			size_t										aOffset) const
		{
			// Note that null termination comes after "m_size"
			if(m_size <= aOffset)
				return false;

			size_t remaining = m_size - aOffset + 1;
			if(remaining < 4)
				return false;

			const char* p = (const char*)m_data + aOffset;
			if(memcmp(p, "end", 3) != 0)
				return false;

			switch(p[3])
			{
			case '\n':
			case '\r':
			case '\t':
			case '\0':
				return true;
			
			default:
				return false;
			}
		}

		// Public data
		FILE*			m_file;
		uint8_t*		m_data;
		size_t			m_size;
	};

	std::pair<std::string, std::string> 
	_ParseCommandLineArgument(
		const char*										aArg)
	{
		size_t length = 0;
		std::optional<size_t> equalSignOffset;

		const char* p = aArg;
		while(*p != '\0')
		{
			if(*p == '=')
			{
				if(!equalSignOffset.has_value())
					equalSignOffset = length;
			}
			length++;
			p++;
		}

		GRAPHTAIL_CHECK(length > 0, "Command-line argument syntax error (empty string)");

		if(!equalSignOffset.has_value())
			return std::make_pair<std::string, std::string>(aArg, "");

		GRAPHTAIL_CHECK(equalSignOffset > 0 && equalSignOffset < length - 1, "Command-line argument syntax error: %s (invalid placement of equal sign)", aArg);

		std::string value = &aArg[equalSignOffset.value() + 1];
		std::string arg = aArg;
		arg.resize(equalSignOffset.value());
		return std::make_pair(arg, value);
	}

	bool
	_IsWhitespace(
		char											aCharacter)
	{
		return aCharacter == ' ' || aCharacter == '\t' || aCharacter == '\n' || aCharacter == '\r' || aCharacter == '\0';
	}

	bool
	_IsAlpha(
		char											aCharacter)
	{
		return (aCharacter >= 'A' && aCharacter <= 'Z') || (aCharacter >= 'a' && aCharacter <= 'z');
	}

	void
	_LoadConfig(
		const char*										aPath,
		std::unordered_map<std::string, std::string>&	aConfigTable,
		std::vector<std::string>&						aInputs)
	{
		InputFile file(aPath);

		enum ParseState
		{
			PARSE_STATE_INIT,
			PARSE_STATE_IDENTIFIER,
			PARSE_STATE_INPUT,
			PARSE_STATE_VALUE,
			PARSE_STATE_BEGIN,
			PARSE_STATE_MULTI_LINE_VALUE
		};

		enum CommentState
		{
			COMMENT_STATE_NONE,
			COMMENT_STATE_C_STYLE,
			COMMENT_STATE_CPP_STYLE
		};

		ParseState parseState = PARSE_STATE_INIT;
		CommentState commentState = COMMENT_STATE_NONE;
		std::string argName;
		std::vector<char> value;
		uint32_t lineNum = 1;

		for(size_t i = 0; i < file.m_size + 1; i++)
		{
			char c = (char)file.m_data[i];

			if(c == '\n')
				lineNum++;

			switch(commentState)
			{
			case COMMENT_STATE_NONE:	
				// We can safely look ahead 1 because of null termination
				if(c == '/' && file.m_data[i + 1] == '*')
				{
					commentState = COMMENT_STATE_C_STYLE;
					i++;
					continue;
				}
				else if (c == '/' && file.m_data[i + 1] == '/')
				{
					commentState = COMMENT_STATE_CPP_STYLE;
					i++;
					continue;
				}
				break;

			case COMMENT_STATE_C_STYLE:				
				if (c == '*' && file.m_data[i + 1] == '/')
				{
					commentState = COMMENT_STATE_NONE;
					i++;
					continue;
				}
				break;

			case COMMENT_STATE_CPP_STYLE:
				if(c == '\n')
					commentState = COMMENT_STATE_NONE;
				break;
			}

			if(commentState != COMMENT_STATE_NONE)
				continue;

			switch(parseState)
			{
			case PARSE_STATE_INIT:
				if(_IsAlpha(c))
				{
					value.push_back(c);
					parseState = PARSE_STATE_IDENTIFIER;
				}
				else if(!_IsWhitespace(c))
				{
					GRAPHTAIL_FATAL_ERROR("%s(%u): Unexpected character: '%c'", aPath, lineNum, c);
				}
				break;

			case PARSE_STATE_IDENTIFIER:
				if(_IsWhitespace(c))
				{
					value.push_back('\0');
					std::string identifier = &value[0];

					if(identifier == "input")
					{
						value.clear();
						parseState = PARSE_STATE_INPUT;
					}
					else if(identifier == "begin")
					{
						value.clear();
						parseState = PARSE_STATE_BEGIN;
					}
					else
					{
						argName = identifier;
						value.clear();
						parseState = PARSE_STATE_VALUE;						
					}
				}
				else if (_IsAlpha(c) || c == '_')
				{
					value.push_back(c);
				}
				else
				{
					GRAPHTAIL_FATAL_ERROR("%s(%u): Unexpected character in identifier: '%c'", aPath, lineNum, c);
				}
				break;

			case PARSE_STATE_INPUT:
				if(c == '\n' || c == '\0')
				{
					value.push_back('\0');
					aInputs.push_back(&value[0]);
					value.clear();
					parseState = PARSE_STATE_INIT;
				}
				else if(!_IsWhitespace(c))
				{
					value.push_back(c);
				}
				break;

			case PARSE_STATE_VALUE:
				if (_IsWhitespace(c))
				{
					value.push_back('\0');
					aConfigTable.insert(std::pair<std::string, std::string>(argName, &value[0]));
					value.clear();
					parseState = PARSE_STATE_INIT;
				}
				else
				{
					value.push_back(c);
				}
				break;

			case PARSE_STATE_BEGIN:
				if (_IsWhitespace(c))
				{
					value.push_back('\0');
					argName = &value[0];
					value.clear();
					parseState = PARSE_STATE_MULTI_LINE_VALUE;					
				}
				else if (_IsAlpha(c))
				{
					value.push_back(c);
				}
				else
				{
					GRAPHTAIL_FATAL_ERROR("%s(%u): Unexpected character in begin statement: '%c'", aPath, lineNum, c);
				}
				break;

			case PARSE_STATE_MULTI_LINE_VALUE:
				if(file.IsEndStatement(i)) // We need to look ahead a bit to see if this an end statement
				{
					i += strlen("end");

					value.push_back('\0');
					aConfigTable.insert(std::pair<std::string, std::string>(argName, &value[0]));
					value.clear();
					parseState = PARSE_STATE_INIT;
				}
				else if(!_IsWhitespace(c))
				{
					value.push_back(c);
				}
				break;
			}
		}
	}

	char
	_ParseDelimiter(
		const char*												aString)
	{
		if(strcmp(aString, "new_line") == 0)
			return '\n';

		GRAPHTAIL_CHECK(strlen(aString) == 1, "Invalid delimiter: %s", aString);
		return aString[0];
	}

	size_t 
	_ParseInput(
		const char*												aString,
		graphtail::Config::Group*								aGroup)
	{		
		size_t i = 0;
		GRAPHTAIL_CHECK(aString[i++] == '(', "Unexpected '%c' in group input definition (expected '(').", aString[0]);
		std::vector<char> value;

		for (;;)
		{
			char c = aString[i++];
			GRAPHTAIL_CHECK(c != '\0', "Unexpected null-termination in group input definition.");
			 
			if(c == ')')
				break;
			else
				value.push_back(c);
		}

		GRAPHTAIL_CHECK(value.size() > 0, "Group input definition is empty.");
		value.push_back('\0');

		aGroup->m_idWildcards.push_back(std::make_unique<graphtail::Wildcard>(&value[0]));

		return i;
	}

	size_t
	_ParseHistogram(
		const char*												aString,
		graphtail::Config::Group*								aGroup)
	{
		GRAPHTAIL_CHECK(!aGroup->m_histogram, "Histogram already defined for group.");
		aGroup->m_histogram = std::make_unique<graphtail::Config::GroupHistogram>();

		size_t i = 0;

		// Description
		{
			GRAPHTAIL_CHECK(aString[i++] == '(', "Unexpected '%c' in group histogram definition (expected '(').", aString[0]);
			std::vector<char> value;

			for (;;)
			{
				char c = aString[i++];
				GRAPHTAIL_CHECK(c != '\0', "Unexpected null-termination in group histogram definition.");

				if (c == ')')
				{
					GRAPHTAIL_CHECK(value.size() > 0, "Group histogram description is empty.");
					value.push_back('\0');
					aGroup->m_histogram->m_name = &value[0];
					break;
				}
				else
				{
					value.push_back(c);
				}
			}
		}

		// List of inputs
		{
			GRAPHTAIL_CHECK(aString[i++] == '(', "Unexpected '%c' in group histogram definition (expected '(').", aString[0]);
			std::vector<char> value;

			for (;;)
			{
				char c = aString[i++];
				GRAPHTAIL_CHECK(c != '\0', "Unexpected null-termination in group histogram definition.");

				if (c == ')')
				{
					GRAPHTAIL_CHECK(value.size() > 0, "Group histogram input definition is empty.");
					value.push_back('\0');
					aGroup->m_histogram->m_ids.push_back(&value[0]);
					break;
				}
				else if (c == ',')
				{
					GRAPHTAIL_CHECK(value.size() > 0, "Group histogram input definition is empty.");
					value.push_back('\0');
					aGroup->m_histogram->m_ids.push_back(&value[0]);
					value.clear();
				}
				else
				{
					value.push_back(c);
				}
			}
		}

		return i;
	}

	size_t
	_ParseGroupParameter(
		const char*												aString,
		graphtail::Config::Group*								aGroup)
	{
		size_t i = 0;
		std::vector<char> arg;
		std::vector<char> value;
		bool inValue = false;

		while (aString[i] != '\0')
		{
			char c = aString[i++];

			if (c == '!' || c == '}')
			{
				i--;
				break;
			}
			else if (c == '=')
			{
				inValue = true;
			}
			else if(inValue)
			{				
				value.push_back(c);
			}
			else
			{
				arg.push_back(c);
			}
		}

		arg.push_back('\0');
		value.push_back('\0');

		if(!aGroup->m_config.TrySetMember(&arg[0], &value[0]))
			GRAPHTAIL_FATAL_ERROR("Invalid group parameter: %s", &arg[0]);

		return i;
	}

	size_t
	_ParseGroup(
		const char*												aString,
		std::vector<std::unique_ptr<graphtail::Config::Group>>& aGroups)
	{
		std::unique_ptr<graphtail::Config::Group> group = std::make_unique<graphtail::Config::Group>();

		size_t i = 0;

		while (aString[i] != '\0')
		{
			char c = aString[i++];

			if (c == '}')
				break;
			else if(c == 'i')
				i += _ParseInput(aString + i, group.get());
			else if(c == 'h')
				i += _ParseHistogram(aString + i, group.get());
			else if(c == '!')
				i += _ParseGroupParameter(aString + i, group.get());
			else
				GRAPHTAIL_FATAL_ERROR("Unexpected '%c' in group definition.", c);
		}

		aGroups.push_back(std::move(group));

		return i;
	}

	void
	_ParseGroups(
		const char*												aString,
		std::vector<std::unique_ptr<graphtail::Config::Group>>&	aGroups)
	{
		size_t i = 0;

		while(aString[i] != '\0')
		{
			char c = aString[i++];

			if(c == '{')
				i += _ParseGroup(aString + i, aGroups);
			else
				GRAPHTAIL_FATAL_ERROR("Unexpected '%c' in groups definition (expected '{').", c);
		}
	}

	uint32_t
	_ParseUInt(
		const char*												aString)
	{
		int v = atoi(aString);
		GRAPHTAIL_CHECK(v >= 0, "Invalid unsigned integer: %s", aString);
		return (uint32_t)v;
	}

	float
	_ParseFloat(
		const char*												aString)
	{
		float v = (float)atof(aString);
		return v;
	}

}

namespace graphtail
{

	bool	
	Config::GroupConfig::TrySetMember(
		const std::string&										aArg,
		const std::string&										aValue)
	{
		if(aArg == "x_step")
		{
			m_xStep = _ParseUInt(aValue.c_str());
			return true;
		}
		else if (aArg == "y_min")
		{
			m_yMin = _ParseFloat(aValue.c_str());
			return true;
		}
		else if (aArg == "y_max")
		{
			m_yMax = _ParseFloat(aValue.c_str());
			return true;
		}
		else if (aArg == "histogram_threshold")
		{
			m_histogramThreshold = _ParseFloat(aValue.c_str());
			return true;
		}
		else if (aArg == "is_size")
		{
			GRAPHTAIL_CHECK(aValue == "", "'is_size' does not have a value.");
			m_isSize = true;
			return true;
		}

		return false;
	}

	void	
	Config::GroupConfig::ApplyDefaults(
		const GroupConfig&										aDefaults)
	{
		if(!m_xStep.has_value())
			m_xStep = aDefaults.m_xStep;

		if (!m_yMin.has_value())
			m_yMin = aDefaults.m_yMin;

		if (!m_yMax.has_value())
			m_yMax = aDefaults.m_yMax;

		if(!m_histogramThreshold.has_value())
			m_histogramThreshold = aDefaults.m_histogramThreshold;

		if (!m_isSize.has_value())
			m_isSize = aDefaults.m_isSize;
	}

	//------------------------------------------------------------------------------------

	Config::Config(
		int														aNumArgs,
		char**													aArgs)
	{
		std::unordered_map<std::string, std::string> configTable;

		if(aNumArgs == 1)
		{
			m_showHelp = true;
			return;
		}

		// Convert command line args into an unordered map
		for (int i = 1; i < aNumArgs; i++)
		{
			const char* arg = aArgs[i];

			if (*arg == '-')
			{
				arg++;
				GRAPHTAIL_CHECK(*arg == '-', "Command line argument syntax error: %s", arg);
				arg++;

				std::pair<std::string, std::string> p = _ParseCommandLineArgument(arg);

				if (p.first == "config")
				{
					_LoadConfig(p.second.c_str(), configTable, m_inputs);
				}
				else if (p.first == "help")
				{
					m_showHelp = true;
					m_showHelpMarkdown = p.second == "markdown";
					return;
				}
				else
				{
					configTable.insert(p);
				}
			}
			else
			{
				m_inputs.push_back(arg);
			}
		}

		// Apply configuration
		std::unordered_map<std::string, std::string>::const_iterator i = configTable.cbegin();
		for(; i != configTable.cend(); i++)
		{
			const std::string& arg = i->first;
			const std::string& value = i->second;

			if(arg == "row_delim")
				m_rowDelimiter = _ParseDelimiter(value.c_str());
			else if (arg == "column_delim")
				m_columnDelimiter = _ParseDelimiter(value.c_str());
			else if (arg == "width")
				m_width = _ParseUInt(value.c_str());
			else if (arg == "height")
				m_height = _ParseUInt(value.c_str());
			else if (arg == "font_size")
				m_fontSize = _ParseUInt(value.c_str());
			else if(arg == "groups")
				_ParseGroups(value.c_str(), m_groups);
			else if(!m_defaultGroupConfig.TrySetMember(arg, value))
				GRAPHTAIL_FATAL_ERROR("Invalid configuration item: %s", arg.c_str());
		}

		// Apply group config defaults to unassigned values
		for(std::unique_ptr<Group>& group : m_groups)
			group->m_config.ApplyDefaults(m_defaultGroupConfig);
	}
	
	Config::~Config()
	{

	}

}