#pragma once

#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>
#include <unordered_map>
#include <vector>
#include <string>

#include "InstIterator.h"

class Expression {
public:
    std::string expr;

    // Default constructor
    Expression() : expr("undefined") {}

     // Constructor with an expression
    Expression(const std::string& e) : expr(e) {}


    Expression operator+(const Expression& other) const {
        return Expression("(" + expr + " + " + other.expr + ")");
    }

    Expression operator-(const Expression& other) const {
        return Expression("(" + expr + " - " + other.expr + ")");
    }
};

class ExecutionState {
public:
    // FIXME:
    // Use Iterator instead? Later just use ++pc to get next instruction
    // When encountering a Branch instruction, check the condition and jump to the correct instruction

    // Pointer to instruction to be executed after the current instruction
    llvm::BasicBlock::iterator pc;

    // Pointer to instruction which is currently executed
    // REMOVEME InstIterator prevPC;
    llvm::BasicBlock::iterator prevPC;

    // Store symbolic variables and their values
    std::unordered_map<const llvm::Value*, Expression> symbolics;

    // Path constraints collected so far
    std::vector<std::string> pathConstraints;

public:
    ExecutionState() {}

    // only to create the initial state
    ExecutionState(llvm::Function *f);

    ExecutionState(const ExecutionState& state);

    // Symbolic getter and setter functions
    void setSymbolic(const llvm::Value* v, const Expression& expr);

    Expression getSymbolic(const llvm::Value* v) const;

    // Add path constraints to the current control flow
    void addConstraint(const std::string& constraint);
};
