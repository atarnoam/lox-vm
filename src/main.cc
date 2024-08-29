#include "src/compiler/compiler.h"
#include "src/syntactics/token.h"
#include "src/vm/vm.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>

int interpret(VM &vm, const std::string &source) {
    Compiler compiler(source);
    Chunk chunk = compiler.compile();

    if (compiler.had_error()) {
        return 65;
    }

    // InterpretResult result = vm.interpret(chunk);
    // if (result != InterpretResult::OK) {
    //     return 70;
    // }

    return 0;
}

int repl() {
    VM vm{};

    std::string s;
    std::cout << "> ";
    while (std::getline(std::cin, s)) {
        interpret(vm, s);
        std::cout << "> ";
    }
    std::cout << std::endl;
    return 0;
}

int run_file(char *filename) {
    VM vm;
    std::ifstream ifs(filename);
    std::string source(std::istreambuf_iterator<char>(ifs), {});
    return interpret(vm, source);
}

int main(int argc, char **argv) {
    if (argc == 1) {
        return repl();
    } else if (argc == 2) {
        return run_file(argv[1]);
    } else {
        std::cout << "Usage: cpplox [script]" << std::endl;
        return 1;
    }
}