#ifndef _args_tester_h_
#define _args_tester_h_

#include "utils.h"
#include "../../src/args/args.h"

#include <string>
#include <vector>
#include <cstring>

namespace tester {

    class argsTester : public utils::TestSuite {
    public:
        argsTester() : utils::TestSuite("args Tester") {
            add_test("Default Construction", "default OS and architecture detection", test_default_construction);
            add_test("Input File First Position", "first argument as input file", test_input_file_first_position);
            add_test("Input File Explicit", "explicit input flag", test_input_file_explicit);
            add_test("Input File Short Form", "input shorthand flag", test_input_file_short_form);
            add_test("Output File Second Position", "second file argument as output", test_output_file_second_position);
            add_test("Output File Explicit", "explicit output flag", test_output_file_explicit);
            add_test("Output File Short Form", "output shorthand flags", test_output_file_short_form);
            add_test("Host OS", "host OS parsing", test_host_os);
            add_test("Target OS", "target OS parsing", test_target_os);
            add_test("Host Architecture", "host architecture parsing", test_host_architecture);
            add_test("Target Architecture", "target architecture parsing", test_target_architecture);
            add_test("Address Space", "address space parsing", test_address_space);
            add_test("Combined Arguments", "multiple arguments together", test_combined_arguments);
            add_test("Dash Stripping", "leading dashes removed", test_dash_stripping);
        }

    private:
        // Helper to convert vector of strings to char** format
        static char** make_argv(const std::vector<std::string>& args) {
            char** argv = new char*[args.size()];
            for (size_t i = 0; i < args.size(); ++i) {
                argv[i] = new char[args[i].length() + 1];
                std::strcpy(argv[i], args[i].c_str());
            }
            return argv;
        }

        static void free_argv(char** argv, int argc) {
            for (int i = 0; i < argc; ++i) {
                delete[] argv[i];
            }
            delete[] argv;
        }

        struct ArgvGuard {
            char** argv;
            int argc;

            ArgvGuard(const std::vector<std::string>& args) 
                : argv(make_argv(args)), argc(static_cast<int>(args.size())) {}

            ~ArgvGuard() {
                free_argv(argv, argc);
            }
        };

        static void test_default_construction() {
            args::base parser;
            
            // Check that default values are set based on platform
            #ifdef _WIN32
                ASSERT_EQ(std::string(args::OS::WIN), parser.hostOS);
                ASSERT_EQ(std::string(args::OS::WIN), parser.targetOS);
            #else
                ASSERT_EQ(std::string(args::OS::LINUX), parser.hostOS);
                ASSERT_EQ(std::string(args::OS::LINUX), parser.targetOS);
            #endif

            #ifdef __x86_64__
                ASSERT_EQ(std::string(args::Architecture::X86), parser.hostArchitecture);
                ASSERT_EQ(std::string(args::Architecture::X86), parser.targetArchitecture);
            #elif defined(__arm__) || defined(__aarch64__)
                ASSERT_EQ(std::string(args::Architecture::ARM), parser.hostArchitecture);
                ASSERT_EQ(std::string(args::Architecture::ARM), parser.targetArchitecture);
            #endif

            // Check address space based on pointer size
            if (sizeof(void*) == sizeof(int64_t)) {
                ASSERT_TRUE(parser.AddressSpace == args::AddressSpaces::BIT_64);
            } else if (sizeof(void*) == sizeof(int32_t)) {
                ASSERT_TRUE(parser.AddressSpace == args::AddressSpaces::BIT_32);
            }
        }

        static void test_input_file_first_position() {
            args::base parser;
            ArgvGuard guard({"input.txt"});
            
            parser.parse(guard.argc, guard.argv);
            
            ASSERT_EQ(std::string("input.txt"), parser.inputFileName);
            ASSERT_EQ(std::string(""), parser.outputFileName);
        }

        static void test_input_file_explicit() {
            args::base parser;
            ArgvGuard guard({"--input", "source.e"});
            
            parser.parse(guard.argc, guard.argv);
            
            ASSERT_EQ(std::string("source.e"), parser.inputFileName);
        }

        static void test_input_file_short_form() {
            args::base parser;
            ArgvGuard guard({"-in", "test.e"});
            
            parser.parse(guard.argc, guard.argv);
            
            ASSERT_EQ(std::string("test.e"), parser.inputFileName);
        }

        static void test_output_file_second_position() {
            args::base parser;
            ArgvGuard guard({"input.e", "output.o"});
            
            parser.parse(guard.argc, guard.argv);
            
            ASSERT_EQ(std::string("input.e"), parser.inputFileName);
            ASSERT_EQ(std::string("output.o"), parser.outputFileName);
        }

        static void test_output_file_explicit() {
            args::base parser;
            ArgvGuard guard({"input.e", "--output", "result.o"});
            
            parser.parse(guard.argc, guard.argv);
            
            ASSERT_EQ(std::string("input.e"), parser.inputFileName);
            ASSERT_EQ(std::string("result.o"), parser.outputFileName);
        }

        static void test_output_file_short_form() {
            args::base parser;
            
            // Test "out" shorthand
            {
                ArgvGuard guard({"-out", "build.o"});
                parser.parse(guard.argc, guard.argv);
                ASSERT_EQ(std::string("build.o"), parser.outputFileName);
            }
            
            // Test "o" shorthand
            args::base parser2;
            {
                ArgvGuard guard({"-o", "program.exe"});
                parser2.parse(guard.argc, guard.argv);
                ASSERT_EQ(std::string("program.exe"), parser2.outputFileName);
            }
        }

        static void test_host_os() {
            args::base parser;
            
            // Test full form
            {
                ArgvGuard guard({"--hostos", "win"});
                parser.parse(guard.argc, guard.argv);
                ASSERT_EQ(std::string("win"), parser.hostOS);
            }
            
            // Test short form
            args::base parser2;
            {
                ArgvGuard guard({"-hos", "linux"});
                parser2.parse(guard.argc, guard.argv);
                ASSERT_EQ(std::string("linux"), parser2.hostOS);
            }
        }

        static void test_target_os() {
            args::base parser;
            
            // Test full form
            {
                ArgvGuard guard({"--targetos", "linux"});
                parser.parse(guard.argc, guard.argv);
                ASSERT_EQ(std::string("linux"), parser.targetOS);
            }
            
            // Test short form
            args::base parser2;
            {
                ArgvGuard guard({"-tos", "win"});
                parser2.parse(guard.argc, guard.argv);
                ASSERT_EQ(std::string("win"), parser2.targetOS);
            }
        }

        static void test_host_architecture() {
            args::base parser;
            
            // Test full form
            {
                ArgvGuard guard({"--hostarch", "x86"});
                parser.parse(guard.argc, guard.argv);
                ASSERT_EQ(std::string("x86"), parser.hostArchitecture);
            }
            
            // Test short form
            args::base parser2;
            {
                ArgvGuard guard({"-harch", "arm"});
                parser2.parse(guard.argc, guard.argv);
                ASSERT_EQ(std::string("arm"), parser2.hostArchitecture);
            }
        }

        static void test_target_architecture() {
            args::base parser;
            
            // Test full form
            {
                ArgvGuard guard({"--targetarch", "arm"});
                parser.parse(guard.argc, guard.argv);
                ASSERT_EQ(std::string("arm"), parser.targetArchitecture);
            }
            
            // Test short form
            args::base parser2;
            {
                ArgvGuard guard({"-tarch", "x86"});
                parser2.parse(guard.argc, guard.argv);
                ASSERT_EQ(std::string("x86"), parser2.targetArchitecture);
            }
        }

        static void test_address_space() {
            args::base parser;
            
            // Test full form
            {
                ArgvGuard guard({"--addressspace", "8"});
                parser.parse(guard.argc, guard.argv);
                ASSERT_TRUE(parser.AddressSpace == args::AddressSpaces::BIT_64);
            }
            
            // Test "aspace" shorthand
            args::base parser2;
            {
                ArgvGuard guard({"-aspace", "4"});
                parser2.parse(guard.argc, guard.argv);
                ASSERT_TRUE(parser2.AddressSpace == args::AddressSpaces::BIT_32);
            }
            
            // Test "bits" shorthand
            args::base parser3;
            {
                ArgvGuard guard({"-bits", "2"});
                parser3.parse(guard.argc, guard.argv);
                ASSERT_TRUE(parser3.AddressSpace == args::AddressSpaces::BIT_16);
            }
            
            // Test 8-bit
            args::base parser4;
            {
                ArgvGuard guard({"--bits", "1"});
                parser4.parse(guard.argc, guard.argv);
                ASSERT_TRUE(parser4.AddressSpace == args::AddressSpaces::BIT_8);
            }
        }

        static void test_combined_arguments() {
            args::base parser;
            ArgvGuard guard({
                "input.e",
                "-o", "output.o",
                "--targetos", "linux",
                "--targetarch", "x86",
                "-bits", "8"
            });
            
            parser.parse(guard.argc, guard.argv);
            
            ASSERT_EQ(std::string("input.e"), parser.inputFileName);
            ASSERT_EQ(std::string("output.o"), parser.outputFileName);
            ASSERT_EQ(std::string("linux"), parser.targetOS);
            ASSERT_EQ(std::string("x86"), parser.targetArchitecture);
            ASSERT_TRUE(parser.AddressSpace == args::AddressSpaces::BIT_64);
        }

        static void test_dash_stripping() {
            args::base parser;
            
            // Test single dash
            {
                ArgvGuard guard({"-input", "file1.e"});
                parser.parse(guard.argc, guard.argv);
                ASSERT_EQ(std::string("file1.e"), parser.inputFileName);
            }
            
            // Test double dash
            args::base parser2;
            {
                ArgvGuard guard({"--input", "file2.e"});
                parser2.parse(guard.argc, guard.argv);
                ASSERT_EQ(std::string("file2.e"), parser2.inputFileName);
            }
            
            // Test triple dash (should still work)
            args::base parser3;
            {
                ArgvGuard guard({"---input", "file3.e"});
                parser3.parse(guard.argc, guard.argv);
                ASSERT_EQ(std::string("file3.e"), parser3.inputFileName);
            }
        }
    };
}

#endif
