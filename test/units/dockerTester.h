#ifndef _docker_tester_h_
#define _docker_tester_h_

#include "utils.h"
#include "../../src/docker/docker.h"
#include "../../src/args/args.h"

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

namespace tester {

    class dockerTester : public utils::TestSuite {
    public:
        dockerTester() : utils::TestSuite("docker Tester") {
            add_test("Local File Compatibility", "local descriptor identifies file paths", test_local_file_compatibility);
            add_test("Remote File Compatibility", "remote descriptor identifies URLs", test_remote_file_compatibility);
            add_test("Local File Path Parsing", "local descriptor parses path components", test_local_file_path_parsing);
            add_test("Remote URL Parsing", "remote descriptor parses URL components", test_remote_url_parsing);
            add_test("Stack Contains File", "stack detects duplicate files", test_stack_contains_file);
            add_test("Stack Add File", "stack adds files and directories", test_stack_add_file);
            add_test("Stack Pop Directory", "stack removes directory from stack", test_stack_pop_directory);
            add_test("Stack Consolidate Path", "stack consolidates directory path", test_stack_consolidate_path);
            add_test("Descriptor Creation Local", "base descriptor creates local instance", test_descriptor_creation_local);
            add_test("Descriptor Creation Remote", "base descriptor creates remote instance", test_descriptor_creation_remote);
        }

    private:
        static void cleanup_test_file(const std::string& path) {
            if (std::filesystem::exists(path)) {
                std::filesystem::remove(path);
            }
        }

        static void create_test_file(const std::string& path, const std::string& content) {
            std::ofstream file(path);
            if (file.is_open()) {
                file << content;
                file.close();
            }
        }

        static void test_local_file_compatibility() {
            ASSERT_TRUE(docker::file::descriptor::local::is_compatible("path/to/file.txt"));
            ASSERT_TRUE(docker::file::descriptor::local::is_compatible("file.e"));
            ASSERT_TRUE(docker::file::descriptor::local::is_compatible("/absolute/path/file.cpp"));
            ASSERT_TRUE(docker::file::descriptor::local::is_compatible("relative/file.h"));
            
            ASSERT_FALSE(docker::file::descriptor::local::is_compatible("http://example.com/file.txt"));
            ASSERT_FALSE(docker::file::descriptor::local::is_compatible("https://github.com/user/repo"));
            ASSERT_FALSE(docker::file::descriptor::local::is_compatible("ftp://server.com/resource"));
        }

        static void test_remote_file_compatibility() {
            ASSERT_TRUE(docker::file::descriptor::remote::is_compatible("http://example.com/file.txt"));
            ASSERT_TRUE(docker::file::descriptor::remote::is_compatible("https://github.com/user/repo"));
            ASSERT_TRUE(docker::file::descriptor::remote::is_compatible("ftp://server.com/resource"));
            ASSERT_TRUE(docker::file::descriptor::remote::is_compatible("git://repository.org/path"));
            
            ASSERT_FALSE(docker::file::descriptor::remote::is_compatible("path/to/file.txt"));
            ASSERT_FALSE(docker::file::descriptor::remote::is_compatible("file.e"));
            ASSERT_FALSE(docker::file::descriptor::remote::is_compatible("/absolute/path/file.cpp"));
        }

        static void test_local_file_path_parsing() {
            std::string test_file = "test_docker_file.txt";
            create_test_file(test_file, "test content");

            args::base env;
            docker::file::descriptor::local desc(test_file, &env);

            ASSERT_EQ(std::string("test_docker_file"), desc.name);
            ASSERT_EQ(std::string("txt"), desc.extension);
            ASSERT_TRUE(std::filesystem::exists(desc.absolute_path));

            cleanup_test_file(test_file);
        }

        static void test_remote_url_parsing() {
            args::base env;
            docker::file::descriptor::remote desc("https://example.com/path/to/resource.e?query=value#fragment", &env);

            ASSERT_EQ(std::string("https"), desc.scheme);
            ASSERT_EQ(std::string("example.com"), desc.host);
            ASSERT_EQ(std::string("resource"), desc.resource.name);
            ASSERT_EQ(std::string("e"), desc.resource.extension);
            ASSERT_EQ(std::string("?query=value"), desc.query);
            ASSERT_EQ(std::string("#fragment"), desc.fragment);
        }

        static void test_stack_contains_file() {
            docker::stack stack;
            
            std::string test_file = "test_stack_file.txt";
            create_test_file(test_file, "stack test content");

            stack.add(test_file);
            ASSERT_TRUE(stack.contains(test_file));
            ASSERT_FALSE(stack.contains("non_existent_file.txt"));

            cleanup_test_file(test_file);
        }

        static void test_stack_add_file() {
            docker::stack stack;
            
            std::string test_file1 = "test_stack_file1.txt";
            std::string test_file2 = "test_stack_file2.txt";
            create_test_file(test_file1, "content 1");
            create_test_file(test_file2, "content 2");

            stack.add(test_file1);
            ASSERT_EQ(static_cast<std::size_t>(1), stack.files.size());
            ASSERT_EQ(static_cast<std::size_t>(1), stack.dirs.size());

            stack.add(test_file2);
            ASSERT_EQ(static_cast<std::size_t>(2), stack.files.size());
            ASSERT_EQ(static_cast<std::size_t>(2), stack.dirs.size());

            stack.add(test_file1);
            ASSERT_EQ(static_cast<std::size_t>(2), stack.files.size());
            ASSERT_EQ(static_cast<std::size_t>(2), stack.dirs.size());

            cleanup_test_file(test_file1);
            cleanup_test_file(test_file2);
        }

        static void test_stack_pop_directory() {
            docker::stack stack;
            
            std::string test_file = "test_pop_file.txt";
            create_test_file(test_file, "pop test");

            stack.add(test_file);
            ASSERT_EQ(static_cast<std::size_t>(1), stack.dirs.size());
            ASSERT_EQ(static_cast<std::size_t>(1), stack.files.size());

            stack.pop();
            ASSERT_EQ(static_cast<std::size_t>(0), stack.dirs.size());
            ASSERT_EQ(static_cast<std::size_t>(1), stack.files.size());

            cleanup_test_file(test_file);
        }

        static void test_stack_consolidate_path() {
            docker::stack stack;
            
            stack.dirs.push_back("path");
            stack.dirs.push_back("to");
            stack.dirs.push_back("directory");

            std::filesystem::path consolidated = stack.consolidate();
            ASSERT_EQ(std::string("pathtodirectory"), consolidated.string());
        }

        static void test_descriptor_creation_local() {
            std::string test_file = "test_descriptor_local.txt";
            create_test_file(test_file, "descriptor test");

            args::base env;
            docker::file::descriptor::base* desc = docker::file::descriptor::base::create(test_file, &env);

            ASSERT_TRUE(desc != nullptr);
            ASSERT_TRUE(desc->type == docker::file::descriptor::types::LOCAL);

            auto* local_desc = static_cast<docker::file::descriptor::local*>(desc);
            ASSERT_EQ(std::string("test_descriptor_local"), local_desc->name);
            ASSERT_EQ(std::string("txt"), local_desc->extension);

            delete desc;
            cleanup_test_file(test_file);
        }

        static void test_descriptor_creation_remote() {
            args::base env;
            docker::file::descriptor::base* desc = docker::file::descriptor::base::create("https://example.com/file.e", &env);

            ASSERT_TRUE(desc != nullptr);
            ASSERT_TRUE(desc->type == docker::file::descriptor::types::REMOTE);

            auto* remote_desc = static_cast<docker::file::descriptor::remote*>(desc);
            ASSERT_EQ(std::string("https"), remote_desc->scheme);
            ASSERT_EQ(std::string("example.com"), remote_desc->host);

            delete desc;
        }
    };
}

#endif
