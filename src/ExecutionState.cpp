#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>
#include <unordered_map>
#include <vector>
#include <string>

#include "ExecutionState.h"

ExecutionState::ExecutionState(llvm::Function* function)
    : pc(function->begin()->begin()), prevPC(nullptr) {
}

ExecutionState::ExecutionState(const ExecutionState& state):
    pc(state.pc),
    prevPC(state.prevPC),
    locals(state.locals),
    pathConstraints(state.pathConstraints){ }


void ExecutionState::addConstraint(const std::string& constraint) {
    pathConstraints.push_back(constraint);
}
