#ifndef _docker_h_
#define _docker_h_

#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

// prototype import of token base
namespace lexer{
    namespace token{
        class base;
    }
}

namespace docker{

    namespace file{
        class descriptor{
        public:
            std::string name;               // name if the file not containing the path nor file extension
            std::string relative_path;      // this is the pure representation how the file was used in the source file
            std::string absolute_path;      // the absolute path an relative path is pointing to can solve situations where two different relative paths would indicate to same file
            std::string extension;          // the file extension

            unsigned int id;                // this is used to internally check file id fast

            descriptor(std::string file_name);
        };

        std::unordered_map<std::string, std::function<std::vector<lexer::token::base*>(descriptor)>> translators; 
        
        std::vector<lexer::token::base*> read(std::string file_name);
    }



}

#endif