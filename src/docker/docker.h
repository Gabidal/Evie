#ifndef _docker_h_
#define _docker_h_

#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <filesystem>

#include "../args/args.h"

// prototype import of token base
namespace lexer{
    namespace token{
        class base;
    }
}

namespace docker{

    namespace file{
        namespace descriptor{
            enum class types{
                UNKNOWN,
                LOCAL,
                REMOTE,
            };

            class base{
            public:
                types type;

                static base* create(const std::string_view file_name, args::base* env);

                base(types descriptor_type) : type(descriptor_type) {}
            };

            // used for local computer files
            class local : public base{
            public:
                std::string name;               // name if the file not containing the path nor file extension
                std::string relative_path;      // this is the pure representation how the file was used in the source file
                std::string absolute_path;      // the absolute path an relative path is pointing to can solve situations where two different relative paths would indicate to same file
                std::string extension;          // the file extension

                unsigned int id;                // this is used to internally check file id fast

                std::vector<char> raw_buffer;

                local(std::string_view file_name, args::base* env, bool needs_to_exist = true);
                local() : base(types::LOCAL){};

                // this will determine if the given buffer is compatible with local file path syntax
                static bool is_compatible(std::string_view raw);
            };

            // used for remote files like HTTPS URLS and URIs
            class remote : public base{
            public:
                std::string scheme;
                std::string host;
                local resource;
                std::string query;
                std::string fragment;

                remote(std::string_view url, args::base* env);

                // fetches the remote file and allocates points into it as if it was a local.
                void localize(args::base* env);

                // this will determine if the given buffer is compatible with URL syntax
                static bool is_compatible(std::string_view raw);
            };

        }

        extern std::unordered_map<std::string_view, std::function<std::vector<lexer::token::base*>(descriptor::local)>> local_translators; 
        extern std::unordered_map<std::string_view, std::function<std::vector<lexer::token::base*>(descriptor::remote)>> remote_translators; 
        
        extern std::vector<lexer::token::base*> translate(std::string_view file_name, args::base* env);

        // initializes all the translators and their respective handlers
        extern void add_translators();

        extern std::vector<lexer::token::base*> translate_e_file(descriptor::local desc);
    }

    class stack {
    public:
        std::vector<std::string_view> dirs;
        std::vector<std::filesystem::path> files;

        std::filesystem::path consolidate();

        bool contains(std::string_view fileName);

        void add(std::string_view pathAndFileName);

        void pop();
    };

}

#endif