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
    constraints(state.constraints) {}

ExecutionState *ExecutionState::ExecutionState::branch() {
    return new ExecutionState(*this);
}

void ExecutionState::addConstraint(ref<Expr> e) {
    // FIXME: Consider adding a constraint manager for optimization.
    constraints.push_back(e);
}
