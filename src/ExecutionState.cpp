#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>
#include <unordered_map>
#include <vector>
#include <string>

#include "ExecutionState.h"

ExecutionState::ExecutionState(llvm::Function* function)
    : pc(function->begin()->begin()), prevPC(pc) {
}

void ExecutionState::setSymbolic(const llvm::Value* v, const Expression& expr) {
    symbolics[v] = expr;
}

ExecutionState::ExecutionState(const ExecutionState& state):
    pc(state.pc),
    prevPC(state.prevPC),
    symbolics(state.symbolics),
    pathConstraints(state.pathConstraints){ }

Expression ExecutionState::getSymbolic(const llvm::Value* v) const {
    auto it = symbolics.find(v);
    return it != symbolics.end() ? it->second : Expression("unknown");
}

void ExecutionState::addConstraint(const std::string& constraint) {
    pathConstraints.push_back(constraint);
}
