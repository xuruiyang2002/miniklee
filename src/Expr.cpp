#include "Expr.h"
#include "llvm/Support/Casting.h"

using namespace miniklee;


unsigned Expr::count = 0;

void Expr::printKind(llvm::raw_ostream &os, Kind k) {
    switch(k) {
#define X(C) case C: os << #C; break
    X(Constant);
    X(Add);
    X(Sub);
    X(Mul);
    X(UDiv);
    X(SDiv);
    X(Not);
    X(Eq);
    X(Ne);
    X(Ult);
    X(Ule);
    X(Ugt);
    X(Uge);
    X(Slt);
    X(Sle);
    X(Sgt);
    X(Sge);
#undef X
    default:
    assert(0 && "invalid kind");
    }
}

unsigned Expr::computeHash() {
    unsigned res = getKind() * Expr::MAGIC_HASH_CONSTANT;

    int n = getNumKids();
    for (int i = 0; i < n; i++) {
        res <<= 1;
        res ^= getKid(i)->hash() * Expr::MAGIC_HASH_CONSTANT;
    }
    
    hashValue = res;
    return hashValue;
}

unsigned ConstantExpr::computeHash() {
    Expr::Width w = getWidth();
    if (w <= 64)
        hashValue = value.getLimitedValue() ^ (w * MAGIC_HASH_CONSTANT);
    else
        hashValue = hash_value(value) ^ (w * MAGIC_HASH_CONSTANT);

    return hashValue;
}

unsigned NotExpr::computeHash() {
    hashValue = expr->hash() * Expr::MAGIC_HASH_CONSTANT * Expr::Not;
    return hashValue;
}

void Expr::printWidth(llvm::raw_ostream &os, Width width) {
    switch(width) {
    case Expr::Bool: os << "Expr::Bool"; break;
    case Expr::Int32: os << "Expr::Int32"; break;
    default: os << "<invalid type: " << (unsigned) width << ">";
    }
}

ref<ConstantExpr> ConstantExpr::Not() {
    return ConstantExpr::alloc(~value);
}

ref<Expr> NotExpr::create(const ref<Expr> e) {
    if (ConstantExpr *CE = llvm::dyn_cast<ConstantExpr>(e.get())) {
        return CE->Not();
    }
    
    return NotExpr::alloc(e);
}
