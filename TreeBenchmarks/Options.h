#pragma once
#include <string>

struct Options
{
	std::string TestFilePath = "";
	size_t RandomCount = 0;
	size_t RandomSize = 0;

	bool help = false;
	bool errors = false;
	bool csvMode = false;
	bool noHeaders = false;

	std::string errorMessage = "";

	explicit Options(int argc, char* argv[])
	{
		for (int i = 1; i < argc; i++)
		{
			std::string arg = argv[i];

			if(arg == "-h" || arg == "--help")
			{
				help = true;
			}
			else if(arg == "-f" || arg == "--file")
			{
				if(i < argc-1)
				{
					TestFilePath = argv[++i];
				}
				else
				{
					errors = true;
					errorMessage += "\t* ";
					errorMessage += arg;
					errorMessage += ": Not enough parameters (must be <string>)\n";
				}
			}
			else if(arg == "-r" || arg == "--random-count")
			{
				if (i < argc - 1)
				{
					try
					{
						RandomCount = std::stoi(argv[++i]);
					}
					catch(std::exception ex)
					{
						errors = true;
						errorMessage += "\t* ";
						errorMessage += arg;
						errorMessage += ": Unable to parse argument (";
						errorMessage += ex.what();
						errorMessage += ")";
					}
				}
				else
				{
					errors = true;
					errorMessage += "\t* ";
					errorMessage += arg;
					errorMessage += ": Not enough parameters (must be <string>)\n";
				}
			}
			else if(arg == "-s" || arg == "--random-size")
			{
				if (i < argc - 1)
				{
					try
					{
						RandomSize = std::stoi(argv[++i]);
					}
					catch (std::exception ex)
					{
						errors = true;
						errorMessage += "\t* ";
						errorMessage += arg;
						errorMessage += ": Unable to parse argument (";
						errorMessage += ex.what();
						errorMessage += ")";
					}
				}
				else
				{
					errors = true;
					errorMessage += "\t* ";
					errorMessage += arg;
					errorMessage += ": Not enough parameters (must be <string>)\n";
				}
			}
			else if(arg == "-c" || arg == "--csv")
			{
				csvMode = true;
			}
			else if(arg == "-n" || arg == "--no-headers")
			{
				csvMode = noHeaders = true;
			}
		}
	}
};
