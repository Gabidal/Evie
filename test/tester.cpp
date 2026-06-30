#include "units/utils.h"
#include "units/lexerTester.h"
#include "units/parserTester.h"
#include "units/argsTester.h"
// #include "units/dockerTester.h"
#include "units/preprocessorTester.h"

int main() {
    std::cout << tester::utils::colorText("GGUI Framework Test Suite\n", GGUI::COLOR::BLUE);
    std::cout << "========================================\n";
    std::cout << "Testing GGUI components and functionality...\n\n";
    
    try {
        tester::utils::run_all_tests({
            new tester::lexerTester(),
            new tester::parserTester(),
            new tester::argsTester(),
            // new tester::dockerTester(),
            new tester::preprocessorTester()
        });
    } catch (const std::exception& e) {
        std::cout << tester::utils::colorText("Test suite crashed with exception: ", GGUI::COLOR::RED) << e.what() << "\n";
        return 2;
    } catch (...) {
        std::cout << tester::utils::colorText("Test suite crashed with unknown exception!\n", GGUI::COLOR::RED);
        return 2;
    }

    return 0; 
}