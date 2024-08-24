#include "docker.h"
#include "../lexer/lexer.h"

#include <filesystem>
#include <fstream>

namespace docker{
    namespace file{
        inline std::vector<descriptor::base*> used_files;

        namespace descriptor{
            static base* create(std::string file_name){
                if (local::is_compatible(file_name)){
                    return new local(file_name);
                }
                else if (remote::is_compatible(file_name)){
                    return new remote(file_name);
                }
                else{
                    // TODO: add error handling
                }
            }

            local::local(std::string file_name, bool needs_to_exist) : base(types::LOCAL) {
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
                    raw_buffer.resize(reader.tellg());
                    reader.seekg(0, std::ios::beg);
                    reader.read(raw_buffer.data(), raw_buffer.size());
                    reader.close();
                }
                else{
                    // this case is handled by the upper subprocess calling this.
                }
                // now add the current file to the used_files container and get its own index
                id = used_files.size();
                used_files.push_back(this);
            }
        
            bool local::is_compatible(std::string file_name){

                // try to identify file path from URL
                size_t scheme_end = file_name.find("://");

                if(scheme_end == std::string::npos){
                    return true;
                }
                else{
                    return false;
                }
            }

            remote::remote(std::string url) : base(types::REMOTE) {
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
                resource = local(url.substr(host_end, query_start - host_end), false);

                // find the start of the fragment if not found then set it to zero
                size_t fragment_start = url.find("#", query_start);
                if(fragment_start == std::string::npos){
                    fragment_start = url.size();
                }

                // extract the query and fragment
                query = url.substr(query_start, fragment_start - query_start);
                fragment = url.substr(fragment_start);

                // we do not need to add this remote url as an used file, since its own local resource has already been logged
                localize();
            }

            void remote::localize(){
                // make sure there is an folder path for the specified host the resource if coming from
                try{
                    // make sure the remote storage path exists
                    std::filesystem::create_directories(store_remote_files + "/" + host);

                    // TODO: make an git/curl fetching system.
                } catch (const std::filesystem::filesystem_error&){
                    // TODO: add error handling
                }


            }
        
            bool remote::is_compatible(std::string file_name){
                // try to identify file path from URL
                size_t scheme_end = file_name.find("://");

                if(scheme_end == std::string::npos){
                    return false;
                }
                else{
                    return true;
            }

        }

        std::vector<lexer::token::base*> translate(std::string file_name){

            // create file handle and find right file translator
            descriptor::base* current = descriptor::base::create(file_name);

            if (current->type == descriptor::types::REMOTE){
                // call right translator based on the descriptor extension
                if(remote_translators.find(static_cast<descriptor::remote*>(current)->resource.extension) != remote_translators.end())
                    return remote_translators[static_cast<descriptor::remote*>(current)->resource.extension](*static_cast<descriptor::remote*>(current));
            }
            else if (current->type == descriptor::types::LOCAL){
                // call right translator based on the descriptor extension
                if(local_translators.find(static_cast<descriptor::local*>(current)->extension) != local_translators.end())
                    return local_translators[static_cast<descriptor::local*>(current)->extension](*static_cast<descriptor::local*>(current));
            }

            // TODO: add error handling
        }
    
        std::unordered_map<std::string, std::function<std::vector<lexer::token::base*>(descriptor::local)>> local_translators; 
        std::unordered_map<std::string, std::function<std::vector<lexer::token::base*>(descriptor::remote)>> remote_translators; 

        void add_translators(){
            // lexer handles the '*.e' extensions
            local_translators["e"] = translate_e_file;
        }

        std::vector<lexer::token::base*> translate_e_file(descriptor::local desc){
            std::vector<lexer::token::base*> result;

            // create a lexer instance
            result = lexer::tokenize(std::string(desc.raw_buffer.data(), desc.raw_buffer.size()), desc.id);

            // add to the start of the result buffer the start of text and end of text at the end of the token buffer
            result.insert(result.begin(), new lexer::token::base(lexer::token::presets::start_of_file));
            result.push_back(new lexer::token::base(lexer::token::presets::end_of_file(result.back()->get_start())));
        }
    }
}