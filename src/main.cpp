#include <cstdlib>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "version.hpp"
#include "elfparser.hpp"

#ifdef QT_GUI
#include "ui/mainwindow.hpp"
#include <QApplication>
#endif

bool parseCommandLine(int p_argCount, char *p_argArray[],
                      std::string &p_file, std::string &p_directory,
                      bool &p_print, bool &p_printReasons, bool &p_capabilities)
{
    boost::program_options::options_description description("options");
    description.add_options()
    ("help", "A list of command line options")
    ("version", "Display version information")
    ("file,f", boost::program_options::value<std::string>(), "The ELF file to examine")
    ("directory,d", boost::program_options::value<std::string>(), "The directory to look through.")
    ("reasons,r", "Print the scoring reasons")
    ("capabilities,c", "Print the files observed capabilities")
    ("print,p", "Print the ELF files various parsed structures.");

    boost::program_options::variables_map argv_map;
    try
    {
        boost::program_options::store(
            boost::program_options::parse_command_line(p_argCount, p_argArray, description), argv_map);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl << std::endl;
        std::cout << description << std::endl;
        return true;
    }

    boost::program_options::notify(argv_map);
    if (argv_map.empty() || argv_map.count("help"))
    {
        std::cout << description << std::endl;
        return true;
    }
    else if (argv_map.count("version"))
    {
        std::cout << version_elf_parser() << std::endl;
        return true;
    }

    p_print = argv_map.count("print") != 0;
    p_printReasons = argv_map.count("reasons") != 0;
    p_capabilities = argv_map.count("capabilities") != 0;

    if (argv_map.count("file") && argv_map.count("directory"))
    {
        std::cout << description << std::endl;
        return true;
    }

    if (argv_map.count("file"))
    {
        p_file.assign(argv_map["file"].as<std::string>());
        return true;
    }

    if (argv_map.count("directory"))
    {
        p_directory.assign(argv_map["directory"].as<std::string>());
        return true;
    }
    return true;
}

/*
 * pass the file to the parser and print the score if an error doesn't
 * occur. print other output based on passed in bools.
 * p_fileName the file to parse
 * p_printReasons indicates if we should print the score reasons
 * p_printCapabilities print extra knowledge about the binary
 * p_printELF print the various data structures we parse
 */
void do_parsing(const std::string &p_fileName, bool p_printReasons,
                bool p_printCapabilities, bool p_printELF)
{
    ELFParser parser;

    try
    {
        parser.parse(p_fileName);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error in parsing " << p_fileName << ": " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    parser.evaluate();

    std::cout << p_fileName << " - Score: " << parser.getScore();
    if (!parser.getFamily().empty())
        std::cout << " [Family: " << parser.getFamily() << "]" << std::endl << std::endl;

    if (p_printReasons)
        parser.printReasons();

    if (p_printCapabilities)
        parser.printCapabilities();

    if (p_printELF)
        parser.printAll();
}

#ifdef QT_GUI
int main(int p_argCount, char *p_argArray[])
{
    QApplication app(p_argCount, p_argArray);
    MainWindow window;
    window.show();
    return app.exec();
}
#else

int main(int p_argCount, char *p_argArray[])
{
    bool printElf = false;
    bool printReasons = false;
    bool printCapabilities = false;
    std::string fileName;
    std::string directoryName;

    if (!parseCommandLine(p_argCount, p_argArray, fileName, directoryName, printElf, printReasons, printCapabilities))
        exit(EXIT_FAILURE);

    if (!fileName.empty())
        do_parsing(fileName, printReasons, printCapabilities, printElf);

    else if (!directoryName.empty())
    {
        for (boost::filesystem::recursive_directory_iterator iter(directoryName);
             iter != boost::filesystem::recursive_directory_iterator(); ++iter)
                do_parsing(iter->path().string(), printReasons, printCapabilities, printElf);
    }

    return EXIT_SUCCESS;
}
#endif
