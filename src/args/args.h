#ifndef _args_h_
#define _args_h_

#include <string>
#include <string_view>
#include <vector>

namespace parser {
    namespace token {
        class base;
    }
}

namespace args {

    namespace OS {
        inline constexpr const char* WIN = "win";
        inline constexpr const char* LINUX = "linux";
    }

    namespace Architecture {
        inline constexpr const char* X86 = "x86";
        inline constexpr const char* ARM = "arm";
    }

    enum class AddressSpaces {
        BIT_8   = 1 << 0,
        BIT_16  = 1 << 1,
        BIT_32  = 1 << 2,
        BIT_64  = 1 << 3,
    };

    class base {
    public:
        std::string inputFileName = "";
        std::string outputFileName = "";

        std::string hostOS = "";
        std::string targetOS = "";

        std::string hostArchitecture = "";
        std::string targetArchitecture = "";

        AddressSpaces AddressSpace;

        base();

        void parse(int argc, char** argv);

        // Transforms the current arg into a class under the 
        void toToken(parser::token::base* scope);
    private:
        std::vector<std::string> arguments;

        std::string_view at(int i);

        bool isFileName(std::string_view potential);

        void matchInputFile(int& i);
        void matchOutputFile(int& i);
        void matchHostOS(int& i);
        void matchTargetOS(int& i);
        void matchHostArchitecture(int& i);
        void matchTargetArchitecture(int& i);
        void matchAddressSpace(int& i);
    };

}

#endif