#include "../H/UI/Usr.h"
#include "../H/UI/Safe.h"
#include "../H/UI/Producer.h"
#include "../H/UI/Satellite.h"
#include "../H/UI/Service.h"
#include "../H/Lexer/Lexer.h"
#include "../H/PreProsessor/PreProsessor.h"
#include "../H/Parser/Parser.h"
#include "../H/Parser/PostProsessor.h"
#include "../H/Parser/Analyzer.h"
#include "../H/Nodes/Node.h"
#include "../H/Flags.h"
#include "../H/Docker/Mangler.h"
#include "../H/Docker/Docker.h"
#include "../H/Docker/HTTPS.h"
#include "../H/Docker/OBJ.h"
#include "../H/Docker//DLL.h"
#include "../H/BackEnd/BackEnd.h"
#include "../H/BackEnd/IRGenerator.h"
#include "../H/BackEnd/Selector.h"
#include "../H/BackEnd/x86.h"
#include "../H/BackEnd/ARM.h"
#include "../H/BackEnd/IRPostProsessor.h"
#include "../H/BackEnd/IROptimizer.h"
#include "../H/BackEnd/DebugGenerator.h"
#include "../H/Assembler/Assembler.h"
#include "../H/Linker/Linker.h"

#include <sstream>
#include <iostream>
#include <map>
#include <fstream>
#include <vector>
#include <ctime>
using namespace std;

Usr* sys; 
Node* Global_Scope;
Selector* selector;
Analyzer analyzer;
x86_64 X86_64;
ARM_64 _ARM_64;
Assembler* assembler;
Safe* safe;
int _SYSTEM_BIT_SIZE_ = 4;

string Output = "";

extern string VT_API;
//Evie.exe -in ~/test.e -out ~/test.asm -f exe -os win32 -arch x86 -mode 32 -d
//Evie.exe -in ~/test.e
int Build(int argc, const char* argv[])
{
    if (argc == 1) {
        //this happends when no parameter are given
        cout << "No arguments detected. Starting help...\n\n";
        cout << "Argument types are: \n";
        cout << "  -in ------------------ [relative path/source file]\n";
        cout << "  -out ----------------- [relative path/output file name]\n";
        cout << "  -os ------------------ [target operating system (win/unix)]\n";
        cout << "  -host ---------------- [the host operating system (win/unix)]\n";
        cout << "  -arch ---------------- [output assembly type (x86/arm)]\n";
        cout << "  -lib ----------------- [relative path/lib name]\n";
        cout << "  -repo-dir ------------ [relative/absolute path for saving git repos]\n";
        cout << "  -f ------------------- [supported output file formats are: asm(outputs respective assembly into the output), exe(executable (works for unix as well)), lib(static lib), dll(dynamic library (support is not made yet!))]\n";
        cout << "  -mode ---------------- [bit mode for assembly output (32/64)]\n";
        cout << "  -debug --------------- [supported debug symbol types: dwarf2]\n";
        cout << "  -vt ------------------ [virus total API-key]\n";
        cout << "  -reference-count-size  [reference count size]\n";
        cout << "  -service ------------- [starts Evie as a service with a port returned in standard out]\n";
        cout << "  -start --------------- [Tells the linker what is the starting function name (default main). Also no need to mangle the name before giving it]\n";
        cout << "  -allow-inconsistencies [True by default, disabling it makes variable addressing manual  'int ptr b = a->address'  where 'a' is a 'int']\n";
        cout << "  -use-scraper --------- [True by default, searches automatically for suitable dll/lib's to link with]\n";

        cout << "\nQuick usage:\n";
        cout << "  ./Evie foo/bar/baz.e\n";

        cout << "\nRemember to set up an environment path named: \'Repo-Dir\'\n";
        cout << endl;
        return -1;
    }
    Lexer::DecimalSeparator = '.';
    Lexer::ExponentSeparator = 'e';
    Lexer::SingleLineCommentIdentifier = '#';
    Lexer::StringIdentifier = '\"';
    Lexer::Keywords = {
        "type", "func", "export", "import", "use", "if", "while", "else", "ptr", "ref", "jump",
        /*"size", size and deciaml and integer and format is no more a keyword because it can be also a variable name, only special in a class scoope*/ 
        "return", "state", "internal",
        "cpp", "evie", "vivid", "plain", "static", "break", "continue"
    };
    sys = new Usr(argv, argc);

    if (VT_API != "")
        sys->Info.VT_API = VT_API;
    _SYSTEM_BIT_SIZE_ = atoi(sys->Info.Bytes_Mode.c_str());
    
    string start_file = sys->Info.Source_File.c_str();

    Satellite satellite;

    MANGLER::Add_ID("cpp", { "P",{MANGLER::PREFIX, "ptr"} });
    MANGLER::Add_ID("cpp", { "R",{MANGLER::PREFIX, "ref"} });
    MANGLER::Add_ID("cpp", { "c",{MANGLER::VARIABLE, "1 integer"} });
    MANGLER::Add_ID("cpp", { "s",{MANGLER::VARIABLE, "2 integer"} });
    MANGLER::Add_ID("cpp", { "i",{MANGLER::VARIABLE, "4 integer"} });
    MANGLER::Add_ID("cpp", { "f",{MANGLER::VARIABLE, "4 decimal"} });
    MANGLER::Add_ID("cpp", { "d",{MANGLER::VARIABLE, "8 decimal"} });
    MANGLER::Add_ID("cpp", { "l",{MANGLER::VARIABLE, "4 integer"} });
    MANGLER::Add_ID("cpp", { "x",{MANGLER::VARIABLE, "8 integer"} });
    MANGLER::Add_ID("cpp", { { "N" }, { MANGLER::CLASS, "" } });
    MANGLER::Add_ID("cpp", { { "E" }, { MANGLER::END_CLASS, "" } });
    //temporary
    MANGLER::Add_ID("cpp", { "t",{MANGLER::VARIABLE, "type"} });

    MANGLER::Add_ID("vivid", { "P",{MANGLER::PREFIX, "ptr"} });
    MANGLER::Add_ID("vivid", { "R",{MANGLER::PREFIX, "ref"} });
    MANGLER::Add_ID("vivid", { "h",{MANGLER::VARIABLE, "1 integer"} });
    MANGLER::Add_ID("vivid", { "c",{MANGLER::VARIABLE, "1 integer"} });
    MANGLER::Add_ID("vivid", { "t",{MANGLER::VARIABLE, "2 integer"} });
    MANGLER::Add_ID("vivid", { "s",{MANGLER::VARIABLE, "2 integer"} });
    MANGLER::Add_ID("vivid", { "j",{MANGLER::VARIABLE, "4 integer"} });
    MANGLER::Add_ID("vivid", { "i",{MANGLER::VARIABLE, "4 integer"} });
    MANGLER::Add_ID("vivid", { "f",{MANGLER::VARIABLE, "4 decimal"} });
    MANGLER::Add_ID("vivid", { "d",{MANGLER::VARIABLE, "8 decimal"} });
    MANGLER::Add_ID("vivid", { "x",{MANGLER::VARIABLE, "8 integer"} });
    MANGLER::Add_ID("vivid", { "y",{MANGLER::VARIABLE, "8 integer"} });
    MANGLER::Add_ID("vivid", { { "N" }, { MANGLER::CLASS, "" } });
    MANGLER::Add_ID("vivid", { { "E" }, { MANGLER::END_CLASS, "" } });
    MANGLER::Add_ID("vivid", { { "_r" }, { MANGLER::RETURN, "" } });
    //temporary
    //MANGLER::Add_ID("vivid", { "t",{MANGLER::VARIABLE, "type"} });

    DOCKER::Slicer = TXT::Unwrap;
    DOCKER::Set_Default_Translator(TXT::TXT_Analyzer);
    DOCKER::Add_Translator(Location::Header, "\x7F" "ELF", ELF::ELF_Analyzer);
    DOCKER::Add_Translator(Location::Header, "!<arch>", LIB::LIB_Analyzer);
    DOCKER::Add_Translator(Location::Header, "#analyze", ASM::ASM_Analyzer);
    DOCKER::Add_Translator(Location::Header, "https", HTTPS::HTTPS_Analyser);
    DOCKER::Add_Translator(Location::Header, "L\x1", PE::OBJ_Analyser);
    DOCKER::Add_Translator(Location::Header, "\x64\x86", PE::OBJ_Analyser);
    DOCKER::Add_Translator(Location::Header, "\x32\x86", PE::OBJ_Analyser);
    DOCKER::Add_Translator(Location::Header, "\x4D\x5A", DLL::DLL_Analyser);

    DOCKER::Add_Translator(Location::File_Name, "asm", ASM::ASM_Analyzer);

    vector<Component> Input;
    PreProsessor preprosessor(Input);

    Global_Scope = new Node(CLASS_NODE, new Position());
    Global_Scope->Name = "GLOBAL_SCOPE";
    Global_Scope->Inheritted.push_back("static");

    selector = new Selector();
    
    preprosessor.Defined_Constants =
    {
        {"SOURCE_FILE",             Component("\"" + sys->Info.Source_File + "\"", Flags::STRING_COMPONENT)},
        {"DESTINATION_FILE",        Component("\"" + sys->Info.Destination_File + "\"", Flags::STRING_COMPONENT)},
        {"OS",                      Component("\"" + sys->Info.OS + "\"", Flags::STRING_COMPONENT)},
        {"HOST_OS",                 Component("\"" + sys->Info.HOST_OS + "\"", Flags::STRING_COMPONENT)},
        {"ARCHITECTURE",            Component("\"" + sys->Info.Architecture + "\"", Flags::STRING_COMPONENT)},
        {"FORMAT",                  Component("\"" + sys->Info.Format + "\"", Flags::STRING_COMPONENT)},
        {"BYTES_MODE",              Component(sys->Info.Bytes_Mode, Flags::NUMBER_COMPONENT)},
        {"true",                    Component("1", Flags::NUMBER_COMPONENT)},
        {"false",                   Component("0", Flags::NUMBER_COMPONENT)},
    };
    
    if (sys->Info.Is_Service) {
        Service service = Service();
        return 0;
    }

    preprosessor.Include(sys->Info.Source_File.c_str());
    
    preprosessor.Factory();

    Parser p(Global_Scope);
    p.Input = Input;
    p.Factory();

    safe = new Safe();
    safe->Components = p.Input;
    safe->Parser_Factory();

    PostProsessor postprosessor(Global_Scope);
    postprosessor.Components = p.Input;
    postprosessor.Factory();
    
    analyzer.Factory();

    vector<IR*> IRs;
    IRGenerator g(Global_Scope, Global_Scope->Childs, &IRs);

    IRPostProsessor IRpost(&IRs);

    IROptimizer IROP(IRs);

    if (sys->Info.Debug)
        DebugGenerator DG(IRs);

    satellite.Scraper();    // If the scraper is enabled, then we try to fetch all the files which contains the imported functions.
    Producer producer(IRs);

    return 0;
}

#ifndef Test

//Evie.exe -in ~/test.e -out ~/test.asm -f exe -os win32 -arch x86 -mode 32 -debug dwarf2
//Evie.exe -in ~/test.e
int main(int argc, const char* argv[])
{
    Build(argc, argv);
    return 0;
}

#endif