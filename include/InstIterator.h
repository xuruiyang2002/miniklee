#pragma once

#include <llvm/IR/Instructions.h>

class InstIterator {
    llvm::Instruction **it;

public:
    InstIterator() : it(0) {}
    InstIterator(llvm::Instruction **_it) : it(_it) {}

    bool operator==(const InstIterator &b) const {
        return it == b.it;
    }
    bool operator!=(const InstIterator &b) const {
        return !(*this == b);
    }

    InstIterator &operator++() {
        ++it;
        return *this;
    }

    operator llvm::Instruction *() const { return it ? *it : 0; }
    operator bool() const { return it != 0; }

    llvm::Instruction *operator->() const { return *it; }
};