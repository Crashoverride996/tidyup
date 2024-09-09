#include <vector>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <unordered_map>


enum class Options {
    Help
};


void print_usage() {
    std::cout << R"(
        Usage: tidyup -d directory
        This program groups and displays desktop items based on their type.

        Options:
          -h, --help                    Display this help message and exit.
          -d, --directory   DIRECTORY   Specify the input directory containing items.
          -e, --extensions  extensions  Specify extensions to consider
          -i, --ignore      extensions  List of extensions to ignore
          -v, --verbose                 Enable verbose mode to show additional details during processing.

        Description:
          The program groups a list of desktop items from an input folder, groups them by their type (e.g., images, extensions, shortcuts), and displays or saves the grouped items based on the specified options.

          Each item in the input file should be in the format:
            NAME.TYPE
          where NAME is the name of the item and TYPE is one of the following:
            File
            Folder
            Shortcut

        Examples:
          tidyup -d DIRECTORY
            Reads items from 'DESKTOP', groups them, and writes the results to appropriate folders.

          tidyup --verbose
            Runs the program in verbose mode, providing additional processing details.

        For more information, please refer to the documentation or visit the project website.)" << "\n";
}


std::string get_directory_by_extension(const std::string extension) {
    static const std::unordered_map<std::string, std::filesystem::path> extension_to_directory = {
        {".png", "./images"},
        {".jpg", "./images"},
        {".jpeg", "./images"},
        {".py", "./python"},
        {".cpp", "./c++"},
        {".java", "./java"},
        {".rs", "./rust"}
    };

    auto it = extension_to_directory.find(extension);
    if (it != extension_to_directory.end()) {
        return it->second;
    } else {
        return "./other";
    }
}


void tidyup(const std::string& folder, const std::vector<std::string>& relevant_extensions, const std::vector<std::string>& ignore_extensions) {
    if (folder == "." || (std::filesystem::exists(folder) && std::filesystem::is_directory(folder))) {
        if (folder != ".") {
          std::cout << "Found " << folder << ". Tidying...\n";
        }
        else {
          std::cout << "Tidying...\n";
        }
        for (const auto& file : std::filesystem::directory_iterator(folder)) {
            if (std::filesystem::is_regular_file(file.path())) {
                const std::string& extension= file.path().extension();
                if ((relevant_extensions.empty() || std::find(relevant_extensions.begin(), relevant_extensions.end(), extension) != relevant_extensions.end())
                      && std::find(ignore_extensions.begin(), ignore_extensions.end(), extension) == ignore_extensions.end()) {
                    std::filesystem::path directory = get_directory_by_extension(extension);
                    
                    if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
                        std::filesystem::create_directory(directory);
                    }        
                    std::filesystem::rename(file.path(), directory / file.path().filename());
                }
            }
        }
    }
    else {
        std::cerr << "Specified directory not found\n";
        std::exit(-1);
    }
}


int main(int argc, char *argv[]) {
    std::vector<std::string> args(argv, argv+argc);
    std::string directory = ".";
    std::vector<std::string> relevant_extensions;
    std::vector<std::string> ignore_extensions;

    for (int i = 1; i < argc; i++) {
        if (args[i] == "help") {
            print_usage(); 
            return 0;
        }
        else if (args[i] == "-d") {
            if (argc != 3) {
                std::cerr << "When using flag -d you must specify target directory\n";
                return -1;
          }
          directory = args[i++];
        }
        else if (args[i] == "-e") {
          int j = i+1;
          while (j < argc && args[j].at(0) != '-') {
              relevant_extensions.push_back(args[j]);
              j++;
          }
              i = j;
        }
        else if (args[i] == "-i") {
          int j = i+1;
          std::cout << "Here parsing " << j << std::endl;
          while (j < argc && args[j].at(0) != '-') {
              ignore_extensions.push_back(args[j]);
              j++;
          }
              i = j;
        }
    }

    tidyup(directory, relevant_extensions, ignore_extensions);
    return 0;
}

