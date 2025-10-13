#ifndef _TEST_UTILS_H_
#define _TEST_UTILS_H_

#include <ggui.h>

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <functional>
#include <cassert>
#include <iomanip>
#include <cctype>

namespace tester {
    namespace utils {
        std::string colorText(const char* text, GGUI::RGB color) {
            GGUI::UTF result = GGUI::UTF(text, {color, {}});

            return GGUI::toString(result, true);
        }

        // Test statistics
        struct TestStats {
            int total_tests = 0;
            int passed_tests = 0;
            int failed_tests = 0;
            std::vector<std::string> failed_test_names;
            
            void reset() {
                total_tests = 0;
                passed_tests = 0;
                failed_tests = 0;
                failed_test_names.clear();
            }
            
            void print_summary() {
                std::cout << colorText("\n=== TEST SUMMARY ===\n", GGUI::COLOR::CYAN);
                std::cout << "Total Tests: " << total_tests << "\n";
                std::cout << colorText("Passed: ", GGUI::COLOR::GREEN) << passed_tests << "\n";
                std::cout << colorText("Failed: ", GGUI::COLOR::RED) << failed_tests << "\n";
                
                if (!failed_test_names.empty()) {
                    std::cout << colorText("\nFailed Tests:\n", GGUI::COLOR::RED);
                    for (const auto& name : failed_test_names) {
                        std::cout << "  - " << name << "\n";
                    }
                }
                
                std::cout << colorText("\nSuccess Rate: ", GGUI::COLOR::YELLOW) 
                        << (total_tests > 0 ? (passed_tests * 100.0 / total_tests) : 0) 
                        << "%\n\n";
            }

            void add(const TestStats& stats) {
                total_tests += stats.total_tests;
                passed_tests += stats.passed_tests;
                failed_tests += stats.failed_tests;
                failed_test_names.insert(failed_test_names.end(), stats.failed_test_names.begin(), stats.failed_test_names.end());
            }
        };



        // Test case function type
        using TestFunction = std::function<void()>;

        // Test case structure
        struct TestCase {
            std::string name;
            std::string description;
            TestFunction test_func;
            
            TestCase(const std::string& n, const std::string& desc, TestFunction func)
                : name(n), description(desc), test_func(func) {}
        };

        // Test suite class
        class TestSuite {
        private:
            std::string suite_name;
            std::vector<TestCase> test_cases;
            TestStats stats;
        public:
            TestSuite(const std::string& name) : suite_name(name) {}
            
            void add_test(const std::string& name, const std::string& description, TestFunction test_func) {
                test_cases.emplace_back(name, description, test_func);
            }
            
            void run_all() {
                std::cout << colorText("\n=== Running Test Suite: ", GGUI::COLOR::BLUE) << suite_name << colorText(" ===\n", GGUI::COLOR::BLUE);
                
                for (const auto& test_case : test_cases) {
                    run_test(test_case);
                }
            }

            TestStats getStats() {
                return stats;
            }
            
        private:
            void run_test(const TestCase& test_case) {
                std::cout << "Running: " << test_case.name << " - " << test_case.description << " ... ";
                
                stats.total_tests++;
                
                try {
                    test_case.test_func();
                    std::cout << colorText("PASSED", GGUI::COLOR::GREEN) << "\n";
                    stats.passed_tests++;
                } catch (const std::exception& e) {
                    std::cout << colorText("FAILED", GGUI::COLOR::RED) << " - " << e.what() << "\n";
                    stats.failed_tests++;
                    stats.failed_test_names.push_back(test_case.name);
                } catch (...) {
                    std::cout << colorText("FAILED", GGUI::COLOR::RED) << " - Unknown exception\n";
                    stats.failed_tests++;
                    stats.failed_test_names.push_back(test_case.name);
                }
            }
        };

        // Helper function to convert a single character to safe hex representation
        std::string char_to_hex(unsigned char c) {
            const char* hex_chars = "0123456789abcdef";
            std::string result = "\\x";
            result += hex_chars[(c >> 4) & 0xF];
            result += hex_chars[c & 0xF];
            return result;
        }

        // Helper function to safely convert values to string for error messages
        template<typename T>
        std::string safe_to_string(const T& value) {
            // For most types, try normal stringstream conversion first
            std::stringstream ss;
            ss << value;
            std::string result = ss.str();
            
            // Replace control characters and non-printable characters with their hex representation
            std::string safe_result;
            for (char c : result) {
                if (std::iscntrl(c) || !std::isprint(c)) {
                    safe_result += char_to_hex(static_cast<unsigned char>(c));
                } else {
                    safe_result += c;
                }
            }
            return safe_result;
        }

        // Specialization for char to handle control characters properly
        template<>
        std::string safe_to_string<char>(const char& value) {
            if (std::iscntrl(value) || !std::isprint(value)) {
                return char_to_hex(static_cast<unsigned char>(value));
            } else {
                return std::string(1, value);
            }
        }

        // Specialization for unsigned char
        template<>
        std::string safe_to_string<unsigned char>(const unsigned char& value) {
            if (std::iscntrl(value) || !std::isprint(value)) {
                return char_to_hex(value);
            } else {
                return std::string(1, static_cast<char>(value));
            }
        }

        // Specialization for C-style strings
        template<>
        std::string safe_to_string<const char*>(const char* const& value) {
            if (value == nullptr) {
                return "(null)";
            }
            
            std::string safe_result;
            for (size_t i = 0; value[i] != '\0'; ++i) {
                char c = value[i];
                if (std::iscntrl(c) || !std::isprint(c)) {
                    safe_result += char_to_hex(static_cast<unsigned char>(c));
                } else {
                    safe_result += c;
                }
            }
            return safe_result;
        }

        // Specialization for std::string
        template<>
        std::string safe_to_string<std::string>(const std::string& value) {
            std::string safe_result;
            for (char c : value) {
                if (std::iscntrl(c) || !std::isprint(c)) {
                    safe_result += char_to_hex(static_cast<unsigned char>(c));
                } else {
                    safe_result += c;
                }
            }
            return safe_result;
        }

        // Simple assertion functions that don't use problematic macros
        void assert_true(bool condition, const std::string& message, const std::string& file, int line) {
            if (!condition) {
                std::stringstream ss;
                ss << "Assertion failed: " << message << " at " << file << ":" << line;
                throw std::runtime_error(ss.str());
            }
        }

        void assert_false(bool condition, const std::string& message, const std::string& file, int line) {
            if (condition) {
                std::stringstream ss;
                ss << "Assertion failed: " << message << " should be false at " << file << ":" << line;
                throw std::runtime_error(ss.str());
            }
        }

        template<typename T>
        void assert_eq(const T& expected, const T& actual, const std::string& file, int line) {
            if (!(expected == actual)) {
                // Build the error message using string concatenation to avoid stringstream issues
                std::string error_msg = "Assertion failed: expected ";
                error_msg += safe_to_string(expected);
                error_msg += " but got ";
                error_msg += safe_to_string(actual);
                error_msg += " at ";
                error_msg += file;
                error_msg += ":";
                error_msg += std::to_string(line);
                throw std::runtime_error(error_msg);
            }
        }

        template<typename T>
        void assert_ne(const T& expected, const T& actual, const std::string& file, int line) {
            if (expected == actual) {
                // Build the error message using string concatenation to avoid stringstream issues
                std::string error_msg = "Assertion failed: expected ";
                error_msg += safe_to_string(expected);
                error_msg += " to not equal ";
                error_msg += safe_to_string(actual);
                error_msg += " at ";
                error_msg += file;
                error_msg += ":";
                error_msg += std::to_string(line);
                throw std::runtime_error(error_msg);
            }
        }

        void assert_float_eq(float expected, float actual, float epsilon, const std::string& file, int line) {
            if (std::abs(expected - actual) > epsilon) {
                // Build the error message using string concatenation to avoid stringstream issues
                std::string error_msg = "Assertion failed: expected ";
                error_msg += std::to_string(expected);
                error_msg += " but got ";
                error_msg += std::to_string(actual);
                error_msg += " (difference: ";
                error_msg += std::to_string(std::abs(expected - actual));
                error_msg += " > ";
                error_msg += std::to_string(epsilon);
                error_msg += ") at ";
                error_msg += file;
                error_msg += ":";
                error_msg += std::to_string(line);
                throw std::runtime_error(error_msg);
            }
        }

        // Helper macros using the functions
        #define ASSERT_TRUE(condition) tester::utils::assert_true((condition), #condition, __FILE__, __LINE__)
        #define ASSERT_FALSE(condition) tester::utils::assert_false((condition), #condition, __FILE__, __LINE__)
        #define ASSERT_EQ(expected, actual) tester::utils::assert_eq((expected), (actual), __FILE__, __LINE__)
        #define ASSERT_NE(expected, actual) tester::utils::assert_ne((expected), (actual), __FILE__, __LINE__)
        #define ASSERT_FLOAT_EQ(expected, actual, epsilon) tester::utils::assert_float_eq((expected), (actual), (epsilon), __FILE__, __LINE__)

        // Function to run all test suites
        void run_all_tests(const std::vector<TestSuite*>& test_suites) {
            TestStats result;
            
            std::cout << colorText("Starting GGUI Test Framework\n", GGUI::COLOR::MAGENTA);
            std::cout << "Running " << test_suites.size() << " test suites...\n";
            
            for (auto* suite : test_suites) {
                suite->run_all();

                // Add suite stats to the result
                result.add(suite->getStats());
            }
            
            result.print_summary();
        }
    }
}

#endif // _TEST_UTILS_H_