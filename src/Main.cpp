#include "Base.h"

#include "Config.h"
#include "CSVTail.h"
#include "Graphs.h"
#include "SDLWindow.h"

int
main(
	int		aNumArgs,
	char**	aArgs)
{
	graphtail::Config config(aNumArgs, aArgs);

	config.m_inputs = { "D:\\git\\jelly\\build\\test\\test.csv" };
	
	graphtail::SDLWindow window(&config);
	graphtail::Graphs graphs(&config);

	std::vector<std::unique_ptr<graphtail::CSVTail>> csvTails;
	for(const std::string& input : config.m_inputs)
		csvTails.push_back(std::make_unique<graphtail::CSVTail>(input.c_str(), &graphs, &config));

	while(window.Update())
	{
		for(std::unique_ptr<graphtail::CSVTail>& csvTail : csvTails)
			csvTail->Update();

		window.DrawGraphs(graphs);
		
		window.Present();

		SDL_Delay(30);
	}

	return EXIT_SUCCESS;
}