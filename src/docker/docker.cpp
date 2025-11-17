#include "docker.h"
#include "../lexer/lexer.h"
#include "../args/args.h"

#include <filesystem>
#include <fstream>

namespace docker{
    namespace file{
        inline std::vector<descriptor::base*> used_files;

        namespace descriptor{
            base* base::create(const std::string_view file_name, args::base* env){
                if (local::is_compatible(file_name)){
                    return new local(file_name, env);
                }

                if (remote::is_compatible(file_name)){
                    return new remote(file_name, env);
                }

                // TODO: add error handling
                return nullptr;
            }

            local::local(std::string_view file_name, args::base* env, bool needs_to_exist) : base(types::LOCAL) {
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
                    if(last_dot != std::string::npos && last_dot > last_slash){
                        name = file_name.substr(last_slash + 1, last_dot - last_slash - 1);
                    } else {
                        name = file_name.substr(last_slash + 1);
                    }
                } else {
                    if(last_dot != std::string::npos){
                        name = file_name.substr(0, last_dot);
                    } else {
                        name = file_name;
                    }
                }

                if (needs_to_exist){
                    // we can now also get the real address of the file and from there extract the actual absolute path of the file
                    absolute_path = std::filesystem::absolute(file_name).string();

                    // check that the file pointed by absolute path exists
                    if(!std::filesystem::exists(absolute_path)){
                        // TODO: add error handling
                    }

                    // read the file into the buffer
                    std::ifstream reader;
                    reader.open(absolute_path, std::ios::binary);

                    if(!reader.is_open()){
                        // TODO: add error handling
                    }

                    reader.seekg(0, std::ios::end);
                    raw_buffer.resize(static_cast<size_t>(reader.tellg()));
                    reader.seekg(0, std::ios::beg);
                    reader.read(raw_buffer.data(), static_cast<std::streamsize>(raw_buffer.size()));
                    reader.close();
                }
                else{
                    // this case is handled by the upper subprocess calling this.
                }
                // now add the current file to the used_files container and get its own index
                id = used_files.size();
                used_files.push_back(this);
            }
        
            bool local::is_compatible(std::string_view file_name){

                // try to identify file path from URL
                size_t scheme_end = file_name.find("://");

                if(scheme_end == std::string::npos){
                    return true;
                }
                else{
                    return false;
                }
            }

            remote::remote(std::string_view url, args::base* env) : base(types::REMOTE) {
                // find the scheme from the start of the url
                size_t scheme_end = url.find("://");
                if(scheme_end == std::string::npos){
                    // TODO: add error handling
                }
                scheme = url.substr(0, scheme_end);

                // find the length of the host/provider
                size_t host_start = scheme_end + 3;
                size_t host_end = url.find("/", host_start);
                if(host_end == std::string::npos){
                    host_end = url.size();
                }
                host = url.substr(host_start, host_end - host_start);

                // find the start of the query if not found then set it to zero
                size_t query_start = url.find("?", host_end);
                if(query_start == std::string::npos){
                    query_start = url.size();
                }

                // now we can extract the resource and its path information from it
                resource = local(url.substr(host_end, query_start - host_end), env, false);

                // find the start of the fragment if not found then set it to zero
                size_t fragment_start = url.find("#", query_start);
                if(fragment_start == std::string::npos){
                    fragment_start = url.size();
                }

                // extract the query and fragment
                query = url.substr(query_start, fragment_start - query_start);
                fragment = url.substr(fragment_start);

                // we do not need to add this remote url as an used file, since its own local resource has already been logged
                localize(env);
            }

            void remote::localize(args::base* env){
                // make sure there is an folder path for the specified host the resource if coming from
                try{
                    // make sure the remote storage path exists
                    std::filesystem::create_directories(env->remoteStoredFile + "/" + host);

                    // TODO: make an git/curl fetching system.
                } catch (const std::filesystem::filesystem_error&){
                    // TODO: add error handling
                }
            }
        
            bool remote::is_compatible(std::string_view file_name){
                // try to identify file path from URL
                size_t scheme_end = file_name.find("://");

                if(scheme_end == std::string::npos){
                    return false;
                }
                else{
                    return true;
                }
            }

        }

        std::unordered_map<std::string_view, std::function<std::vector<lexer::token::base*>(descriptor::local)>> local_translators; 
        std::unordered_map<std::string_view, std::function<std::vector<lexer::token::base*>(descriptor::remote)>> remote_translators; 

        std::vector<lexer::token::base*> translate(std::string_view file_name, args::base* env){
            // create file handle and find right file translator
            descriptor::base* current = descriptor::base::create(file_name, env);

            if (!current) throw std::runtime_error("Unable to create file descriptor for file: " + std::string(file_name));

            if (current->type == descriptor::types::REMOTE){
                const auto* remote_descriptor = static_cast<descriptor::remote*>(current);
                const auto translator = remote_translators.find(remote_descriptor->resource.extension);
                if (translator != remote_translators.end()){
                    auto tokens = translator->second(*remote_descriptor);
                    delete current;
                    return tokens;
                }
                delete current;
            }
            else if (current->type == descriptor::types::LOCAL){
                const auto* local_descriptor = static_cast<descriptor::local*>(current);
                const auto translator = local_translators.find(local_descriptor->extension);
                if (translator != local_translators.end()){
                    return translator->second(*local_descriptor);
                }
            }

            throw std::runtime_error("No translator found for file: " + std::string(file_name));
        }
    
        void add_translators(){
            // lexer handles the '*.e' extensions
            local_translators["e"] = translate_e_file;
        }

        std::vector<lexer::token::base*> translate_e_file(descriptor::local desc){
            std::vector<lexer::token::base*> result;

            // tokenize the local file contents
            result = lexer::tokenize(std::string(desc.raw_buffer.data(), desc.raw_buffer.size()), desc.id);

            lexer::token::position eof_position = result.empty()
                ? lexer::token::position(0, 0, static_cast<unsigned short>(desc.id))
                : result.back()->get_start();

            // prepend start-of-file and append end-of-file markers so downstream stages know the bounds
            result.insert(result.begin(), new lexer::token::control(lexer::token::presets::start_of_file));
            result.push_back(new lexer::token::control(lexer::token::presets::end_of_file(eof_position)));

            return result;
        }
    
        std::filesystem::path stack::consolidate(){
            std::filesystem::path result;

            for (auto dir : dirs) {
                result += dir;
            }

            return result;
        }
        
        bool stack::contains(std::string_view fileName) {
            // First we need to get the absolute path of the incoming file name
            // For that we need to differentiate the handling to absolute fileName paths and relative paths-
            // For relative paths we need to fetch the current dirs as consolidated path appended before the fileName to then convert to absolute path.
            if (std::filesystem::path(fileName).is_absolute()) {
                std::filesystem::path absPath = std::filesystem::absolute(fileName);

                for (const auto& file : files) {
                    if (file == absPath) {
                        return true;
                    }
                }
            } else {
                std::filesystem::path consolidatedPath = consolidate();
                std::filesystem::path absPath = std::filesystem::absolute(consolidatedPath / fileName);

                for (const auto& file : files) {
                    if (file == absPath) {
                        return true;
                    }
                }
            }

            return false;
        }
    
        void stack::add(std::string_view pathAndFileName) {
            // First let's check if the incoming file is already added
            if (contains(pathAndFileName)) return; // No need to throw, because this way we enable multiple includes of files without breaking.

            // Next we'll split the incoming into the relative path and the absolute file path
            std::filesystem::path relativePath = std::filesystem::path(pathAndFileName).parent_path();
            std::filesystem::path absoluteFilePath = std::filesystem::absolute(pathAndFileName);

            files.push_back(absoluteFilePath);
            dirs.push_back(relativePath.string());
        }

        void stack::pop() {
            // Only remove the current relative dir stack, files are needed to be held for future comparisons.
            if (!dirs.empty()) {
                dirs.pop_back();
            }
        }
    }
}