#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <stack>
#include <iostream>
#include "ExecutionState.h"
#include "Solver.h"

using namespace llvm;

// Color codes
#define COLOR_RESET   "\033[0m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_BLUE    "\033[1;34m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_GREY    "\033[1;30m"

// Macros for colored text
#define GREEN_TEXT(text) COLOR_GREEN text COLOR_RESET
#define BLUE_TEXT(text)  COLOR_BLUE text COLOR_RESET
#define RED_TEXT(text)   COLOR_RED text COLOR_RESET
#define GREY_TEXT(text)  COLOR_GREY text COLOR_RESET

class StateManager {
public:
    void addState(ExecutionState* state) {
        states.push_back(state);
    }

    template <typename Iterator>
    void addState(Iterator begin, Iterator end) {
        states.insert(states.end(), begin, end);
    }

    ExecutionState& selectState() {
        assert(!states.empty());
        return *states.front();
    }

    bool isEmpty() const { return states.empty(); }

    std::deque<ExecutionState*>::iterator find(ExecutionState* target) {
        return std::find(states.begin(), states.end(), target);
    }

    std::deque<ExecutionState*>::iterator begin() { return states.begin(); }

    std::deque<ExecutionState*>::iterator end() { return states.end(); }

    std::deque<ExecutionState*>::iterator erase(std::deque<ExecutionState*>::iterator pos) {
        return states.erase(pos);
    }

private:
    std::deque<ExecutionState*> states;
};
class Executor {
public:
    std::unique_ptr<llvm::Module> module;
    
    std::unique_ptr<Solver> solver;

    StateManager states;

    typedef std::pair<ExecutionState*,ExecutionState*> StatePair;

    void runFunctionAsMain(llvm::Function* function);

    // Constructor that accepts an llvm::Module pointer
    explicit Executor(std::unique_ptr<llvm::Module> module);

    // Used to track states that have been added during the current
    // instructions step. 
    std::vector<ExecutionState *> addedStates;
    // Used to track states that have been removed during the current
    // instructions step. 
    std::vector<ExecutionState *> removedStates;

private:

    void stepInstruction(ExecutionState& state);

    void executeInstruction(ExecutionState& state, llvm::Instruction* inst);

    void updateStates(ExecutionState *current);

    void transferToBasicBlock(llvm::BasicBlock *dst, ExecutionState &state);

    void executeAlloc(ExecutionState& state, unsigned size, llvm::Instruction* inst);

    ref<Expr> getInstructionValue(ExecutionState& state, llvm::Instruction* i);

    void executeMemoryOperation(ExecutionState& state, bool isWrite, llvm::Instruction *address, ref<Expr> value, llvm::Instruction* i);

    ref<Expr> getValue(ExecutionState& state, Value* value /* ConstantInt* or Instruction* */);
        
    void executeMakeSymbolic(ExecutionState& state, Instruction *sym, std::string name);

    StatePair fork(ExecutionState &current, ref<Expr> condition);

    /// Add the given (boolean) condition as a constraint on state. This
    /// function is a wrapper around the state's addConstraint function.
    void addConstraint(ExecutionState &state, ref<Expr> condition);
};


#endif // EXECUTOR_H