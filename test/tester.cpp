#include "units/utils.h"
#include "units/bitmaskTester.h"
#include "units/setTester.h"
#include "units/lexerTester.h"

int main() {
    std::cout << tester::utils::colorText("GGUI Framework Test Suite\n", GGUI::COLOR::BLUE);
    std::cout << "========================================\n";
    std::cout << "Testing GGUI components and functionality...\n\n";
    
    try {
        tester::utils::run_all_tests({
            new tester::bitmaskTester(),
            new tester::lexerTester(),
            new tester::setTester(),
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