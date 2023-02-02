#include "Base.h"

#include "Help.h"

namespace graphtail
{

	Help::Help()
	{
		_DefineEntry(false, { "row_delim=<character>" },
		{
			"Character used as row deliminator in CSV files or 'new_line'. Defaults",
			"to 'new_line'."
		});

		_DefineEntry(false, { "column_delim=<character>" },
		{
			"Character used as column deliminator in CSV files. Defaults to ';'."
		});

		_DefineEntry(false, { "width=<width>", "height=<height>" },
		{
			"Sets the size of the window. Defaults to a 1000x500."
		});

		_DefineEntry(false, { "font_size=<size>" },
		{
			"Sets the size of the font used to display information. Defaults to 14."
		});

		_DefineEntry(true, { "x_step=<pixels>" },
		{
			"Instead of stretching graph to fit the width of the window, each data",
			"point will advance <pixels> on the x-axis."
		});

		_DefineEntry(true, { "y_min=<min>", "y_max=<min>" },
		{
			"Clamp the graph y-axis to the specified range. Default is to stretch."
		});

		_DefineEntry(true, { "histogram_threshold=<value>" },
		{
			"Histogram values must be higher than this to be rendered. Default is",
			"to not have a threshold."
		});

		_DefineEntry(false, { "groups=<definition>" },
		{
			"Defines graph groups. See example below. If no groups are defined,",
			"all columns will get their own group automatically."
		});

		_DefineEntry(false, { "config=<path>" },
		{
			"Loads configuration from specified file. See below for an example of a",
			"configuration file."
		});
	}

	Help::~Help()
	{

	}

	void	
	Help::PrintCommandLine() const
	{
		printf("usage: graphtail [options] <input files>\n"
			"\n"
			"    Renders numeric data from input CSV files into a window. Any changes to the\n"
			"    files will automatically update the window.\n"
			"\n"
			"options:\n"
		);

		for(const std::unique_ptr<Entry>& t : m_entries)
		{
			printf("\n");

			_PrintIndent();

			for(const std::string& option : t->m_options)
				printf("--%s ", option.c_str());

			printf("\n");

			for(const std::string& line : t->m_description)
			{
				_PrintIndent();
				_PrintIndent();
				printf("%s\n", line.c_str());
			}

			if (t->m_group)
			{
				_PrintIndent();
				_PrintIndent();
				printf("This option can be used in a group definition.\n");
			}
		}

		printf("\ngroups:\n"
			"\n"
			"    Group description syntax looks a bit wonky because it's easy to parse,\n"
			"    but it's quite simple:\n"
			"\n"
			"    '{' marks the beginning of a group and it's ended with '}'. Inside the\n"
			"    brackets you can use 'i(column)' to added 'column' to the group. 'column'\n"
			"    can also be a wildcard (for example '*something*'), which will cause any\n"
			"    column with a name matching the wildcard to be added to the group. Inside\n"
			"    the group you can also specify group-specific parameters with\n"
			"    '!option=value'. You can see which options can be specified per group in\n"
			"    the list above.\n"
			"    Use 'h(name)(column1, column2, ...)' to turn the group into a histogram\n"
			"    heatmap.\n"
			"\n"
			"    Example 1:\n"
			"\n"
			"        {i(foo)i(bar)!y_min=0!y_max=1}{i(baz)}\n"
			"\n"
			"        This will cause the columns 'foo' and 'bar' to be added to the same\n"
			"        group. The y-axis will be clamped between 0 and 1. The column 'baz'\n"
			"        will be put in a separate group.\n"
			"\n"
			"    Example 2:\n"
			"\n"
			"        {h(foo)(foo1,foo2,foo3,foo4)!histogram_threshold=0}\n"
			"\n"
			"        Render columns 'foo1', 'foo2', 'foo3', and 'foo4' as a histogram\n"
			"        heatmap named 'foo'. Cells of the heatmap must have a value of\n"
			"        atleast 0 to be rendered.\n"
		);	

		printf("\nconfig:\n"
			"\n"
			"    A configuration file is a list of statements:\n"
			"\n"
			"        input /path/to/some/csv-file\n"
			"        width 500\n"
			"        height 500\n"
			"        groups {i(foo)i(bar)}\n"
			"\n"
			"    Alternatively you can specify values like this:\n"
			"\n"
			"        begin groups\n"
			"            {\n"
			"                i(foo)\n"
			"                i(bar)\n"
			"            }\n"
			"        end\n"
			"\n"
			"    This is particularily useful if you have a lot of groups and you want\n"
			"    your configuration to be more readable.\n"
		);
	}

	void	
	Help::PrintMarkdown() const
	{
		printf("Option|Description\n-|-\n");
		
		for (const std::unique_ptr<Entry>& t : m_entries)
		{
			for(size_t i = 0; i < t->m_options.size(); i++)
			{
				if(i > 0)
					printf("<br>");
				printf("```--%s```", t->m_options[i].c_str());
			}

			printf("|");

			for(const std::string& line : t->m_description)
				printf(" %s", line.c_str());

			if(t->m_group)
				printf(" This option can be used in a group definition.");

			printf("\n");
		}
	}

	//---------------------------------------------------------------------------

	void	
	Help::_DefineEntry(
		bool							aGroup,
		const std::vector<std::string>& aOptions,
		const std::vector<std::string>& aDescription)
	{
		m_entries.push_back(std::make_unique<Entry>());
		std::unique_ptr<Entry>& t = m_entries[m_entries.size() - 1];
		t->m_description = aDescription;
		t->m_options = aOptions;
		t->m_group = aGroup;
	}

	void	
	Help::_PrintIndent() const
	{
		for(size_t i = 0; i < 4; i++)
			printf(" ");
	}

}