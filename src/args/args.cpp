#include "args.h"
#include <stdexcept>


args::base::base() {
    // For default, we're gonna use preprocessor conditionals to set them
    #ifdef _WIN32
        hostOS = OS::WIN;
        targetOS = OS::WIN;
    #else
        hostOS = OS::LINUX;
        targetOS = OS::LINUX;
    #endif

    #ifdef __x86_64__
        hostArchitecture = Architecture::X86;
        targetArchitecture = Architecture::X86;
    #elif defined(_M_X64) || defined(_M_IX86)
        hostArchitecture = Architecture::X86;
        targetArchitecture = Architecture::X86;
    #elif defined(__arm__) || defined(__aarch64__) || defined(_M_ARM)
        hostArchitecture = Architecture::ARM;
        targetArchitecture = Architecture::ARM;
    #else
        hostArchitecture = "unknown";
        targetArchitecture = "unknown";
    #endif

    if (sizeof(void*) == sizeof(int64_t)) {
        AddressSpace = AddressSpaces::BIT_64;
    } else if (sizeof(void*) == sizeof(int32_t)) {
        AddressSpace = AddressSpaces::BIT_32;
    } else if (sizeof(void*) == sizeof(int16_t)) {
        AddressSpace = AddressSpaces::BIT_16;
    } else {
        AddressSpace = AddressSpaces::BIT_8;
    }
}

void args::base::parse(int argc, char** argv) {
    // First lets convert it to an easier to use format
    arguments = std::vector<std::string>(argc);

    for (int i = 0; i < argc; i++) {
        arguments[i] = std::string(argv[i]);
    }

    // Now that we have mutable copies we can remove dashes because those are redundant
    for (auto& arg : arguments) {
        // Remove n amount of dashes from the start of the string only
        while (arg.find("-", 0) == 0) {
            arg.erase(0, 1);
        }
    }

    // We sadly need to keep uppercase wordings cause in unix files are determined via letter casing :(

    for (int i = 0; i < (int32_t)arguments.size(); i++) {
        matchInputFile(i);
        matchOutputFile(i);
        matchHostOS(i);
        matchTargetOS(i);
        matchHostArchitecture(i);
        matchTargetArchitecture(i);
        matchAddressSpace(i);
    }
}

std::string_view args::base::at(int i) {
    if (i < 0 || i >= (int32_t)arguments.size()) return "";

    return arguments[i];
}

bool args::base::isFileName(std::string_view potential) {
    // Checks if the potential string contains a dot and a file ending, .gitignores do not count!
    size_t dotPos = potential.find('.');

    if (dotPos == std::string_view::npos || dotPos == 0 || dotPos == potential.length() - 1) {
        return false;
    }

    return true;
}

void args::base::matchInputFile(int& i) {
    // either the first string or starting with "input"
    if (i == 0 && isFileName(at(i))) {
        inputFileName = at(i++);

        return;
    }

    if (at(i) == "input" || at(i) == "in") {
        if (i + 1 >= (int32_t)arguments.size()) throw std::runtime_error("Expected input file name after 'input' argument");

        if (!isFileName(at(i + 1))) throw std::runtime_error("Expected valid file name after 'input' argument");

        inputFileName = at(i + 1);

        i += 2;

        return;
    }
}

void args::base::matchOutputFile(int& i) {
    // Either output string or the non-first file name
    if (i != 0 && isFileName(at(i))) {
        outputFileName = at(i++);
        
        return;
    }

    if (at(i) == "output" || at(i) == "out" || at(i) == "o") {
        if (i + 1 >= (int32_t)arguments.size()) throw std::runtime_error("Expected output file name after 'output' argument");

        if (!isFileName(at(i + 1))) throw std::runtime_error("Expected valid file name after 'output' argument");

        outputFileName = at(i + 1);

        i += 2;

        return;
    }
}

void args::base::matchHostOS(int& i) {
    if (at(i) == "hostos" || at(i) == "hos") {
        if (i + 1 >= (int32_t)arguments.size()) throw std::runtime_error("Expected host OS after 'hostos' argument");

        hostOS = at(i + 1);

        i += 2;

        return;
    }
}

void args::base::matchTargetOS(int& i) {
    if (at(i) == "targetos" || at(i) == "tos") {
        if (i + 1 >= (int32_t)arguments.size()) throw std::runtime_error("Expected target OS after 'targetos' argument");

        targetOS = at(i + 1);

        i += 2;

        return;
    }
}

void args::base::matchHostArchitecture(int& i) {
    if (at(i) == "hostarch" || at(i) == "harch") {
        if (i + 1 >= (int32_t)arguments.size()) throw std::runtime_error("Expected host architecture after 'hostarch' argument");

        hostArchitecture = at(i + 1);

        i += 2;

        return;
    }
}

void args::base::matchTargetArchitecture(int& i) {
    if (at(i) == "targetarch" || at(i) == "tarch") {
        if (i + 1 >= (int32_t)arguments.size()) throw std::runtime_error("Expected target architecture after 'targetarch' argument");

        targetArchitecture = at(i + 1);

        i += 2;

        return;
    }
}

void args::base::matchAddressSpace(int& i) {
    if (at(i) == "addressspace" || at(i) == "aspace" || at(i) == "bits") {
        if (i + 1 >= (int32_t)arguments.size()) throw std::runtime_error("Expected address space after 'addressspace' argument");

        int IntValue = 0;

        try {
            IntValue = std::stoi(std::string(at(i + 1)));
        } catch (const std::exception& e) {
            throw std::runtime_error("Invalid address space value. Expected 1, 2, 4, or 8");
        }

        AddressSpace = static_cast<AddressSpaces>(IntValue);

        i += 2;

        return;
    }
}


