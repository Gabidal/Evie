#include "docker.h"
#include "../lexer/lexer.h"

#include <filesystem>

namespace docker{
    namespace file{
        inline std::vector<descriptor> used_files;

        descriptor::descriptor(std::string file_name){
            // extract from the string the path, name and the extension
            size_t last_slash = file_name.find_last_of('/');

            if(last_slash == std::string::npos){
                last_slash = file_name.find_last_of('\\');
            }

            if(last_slash != std::string::npos){
                relative_path = file_name.substr(last_slash + 1);
            }

            size_t last_dot = file_name.find_last_of('.');

            if(last_dot != std::string::npos){
                extension = file_name.substr(last_dot + 1);
            }

            if(last_slash != std::string::npos){
                name = file_name.substr(last_slash + 1, last_dot - last_slash - 1);
            }else{
                name = file_name.substr(0, last_dot);
            }

            // we can now also get the real address of the file and from there extract the actual absolute path of the file
            absolute_path = std::filesystem::absolute(file_name).string();

            // now add the current file to the used_files container and get its own index
            id = used_files.size();
            used_files.push_back(*this);
        }

        std::vector<lexer::token::base*> read(std::string file_name){

            // create file handle and find right file translator
            descriptor current(file_name);

            // call right translator based on the descriptor extension
            if(translators.find(current.extension) != translators.end())
                return translators[current.extension](current);

            // TODO: add error handling
        }
    }
}