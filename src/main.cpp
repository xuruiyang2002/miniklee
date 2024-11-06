#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>

#include "Executor.h"

int main(int argc, char** argv) {
    llvm::LLVMContext context;
    llvm::SMDiagnostic err;

    // Load the LLVM IR file
    auto module = llvm::parseIRFile("./test/example.ll", err, context);
    if (!module) {
        err.print(argv[0], llvm::errs());
        return 1;
    }

    // Run symbolic executor on each function
    Executor executor;
    for (auto& function : *module) {
        if (!function.isDeclaration()) {
            executor.run(function);
        }
    }

    return 0;
}
