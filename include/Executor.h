#pragma once
// #include <llvm/IR/Function.h>
// #include <llvm/IR/BasicBlock.h>
// #include <llvm/IR/Instruction.h>
// #include <llvm/IR/Instructions.h>
// #include <llvm/IR/Module.h>
// #include <llvm/Support/raw_ostream.h>
#include <stack>
#include <iostream>
#include "ExecutionState.h"

class Executor {
public:
    std::stack<State> stateStack;

    void run(llvm::Function& function);

    // Executor(llvm::LLVMContext& context) : context_(context) {}

    Executor() {}

private:
    void executeInstruction(llvm::Instruction& inst, State& state);

    void handleBinaryOperation(llvm::BinaryOperator& binOp, State& state);
};
