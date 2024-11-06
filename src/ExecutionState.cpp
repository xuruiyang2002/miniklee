#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>
#include <unordered_map>
#include <vector>
#include <string>

#include "ExecutionState.h"

void State::setSymbolic(const llvm::Value* v, const Expression& expr) {
    variables[v] = expr;
}

Expression State::getSymbolic(const llvm::Value* v) const {
    auto it = variables.find(v);
    return it != variables.end() ? it->second : Expression("unknown");
}

void State::addConstraint(const std::string& constraint) {
    pathConstraints.push_back(constraint);
}
