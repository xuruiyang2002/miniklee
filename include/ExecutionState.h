#pragma once

#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>
#include <unordered_map>
#include <vector>
#include <string>

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

class State {
public:
    std::unordered_map<const llvm::Value*, Expression> symbolics;
    std::vector<std::string> pathConstraints;

    void setSymbolic(const llvm::Value* v, const Expression& expr);

    Expression getSymbolic(const llvm::Value* v) const;

    void addConstraint(const std::string& constraint);
};
