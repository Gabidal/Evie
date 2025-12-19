#include "args/args.h"
#include "docker/docker.h"

#include "lexer/lexer.h"

#include "preprocessor/preprocessor.h"
#include "parser/parser.h"

int main(int argc, char** argv) {
    args::base Args;
    Args.parse(argc, argv);

    docker::stack fileStack;

    parser::unit::lexerOutput inputFileTokens = docker::file::translate(Args.inputFileName, &Args, &fileStack);
    fileStack.add(Args.inputFileName);

    parser::token::scope::base* globalScope = new parser::token::scope::base(
        parser::token::info(
            parser::token::types::SCOPE,
            lexer::token::position{0, 0, 0},
            nullptr,
            "global_scope"
        ),
        inputFileTokens
    );

    // Add OS constants into global scope
    Args.toToken(globalScope);

    parser::unit::base problematicParser(parser::unit::pass::FIRST, globalScope);
    preprocessor::unit fixerPreprocessor(globalScope, &Args, &fileStack);

    utils::forEverThrowFixer(
        [&]() { problematicParser.factory(); fixerPreprocessor.factory(); },
        [&]() { fixerPreprocessor.factory(); }
    );

    return 0;
}