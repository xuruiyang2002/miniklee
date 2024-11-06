#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <stack>
#include <iostream>

#include "Executor.h"
#include "ExecutionState.h"


void Executor::run(llvm::Function& function) {
    State initialState;
    stateStack.push(initialState);

    while (!stateStack.empty()) {
        State currentState = stateStack.top();
        stateStack.pop();

        for (auto& block : function) {
            for (auto& inst : block) {
                executeInstruction(inst, currentState);
            }
        }
    }
}


void Executor::executeInstruction(llvm::Instruction& inst, State& state) {
    llvm::errs() << inst << "\n";
    if (auto* binOp = llvm::dyn_cast<llvm::BinaryOperator>(&inst)) {
        handleBinaryOperation(*binOp, state);
    } else {
        // Handle other instruction types as needed.
    }
}

void Executor::handleBinaryOperation(llvm::BinaryOperator& binOp, State& state) {
    llvm::Value* leftOp = binOp.getOperand(0);
    llvm::Value* rightOp = binOp.getOperand(1);

    Expression leftExpr = state.getSymbolic(leftOp);
    Expression rightExpr = state.getSymbolic(rightOp);

    Expression resultExpr("unknown");

    if (binOp.getOpcode() == llvm::Instruction::Add) {
        resultExpr = leftExpr + rightExpr;
    } else if (binOp.getOpcode() == llvm::Instruction::Sub) {
        resultExpr = leftExpr - rightExpr;
    }

    state.setSymbolic(&binOp, resultExpr);

    // Output symbolic expression for tracing purposes
    std::cout << "Executed " << binOp.getOpcodeName() << " instruction:\n";
    std::cout << "  Result: " << resultExpr.expr << "\n";
}