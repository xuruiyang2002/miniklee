#ifndef EXPR_H
#define EXPR_H

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Casting.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/APFloat.h"
#include "Ref.h"

class Expr {
public:
    static unsigned count;
    static const unsigned MAGIC_HASH_CONSTANT = 39;

    // The type of an expression is simply its width, in bits. 
    typedef unsigned Width;

    // Currently only 8-bit integers are supported.
    static const Width Bool = 1;
    static const Width Int8 = 8;

    enum Kind {
        InvalidKind = -1,

        // Primitive
        
        Constant = 0,

        // Bit
        Not,

        // Arithmetic
        Add,
        Sub,
        Mul,
        UDiv,
        SDiv,

        // Compare
        Eq,
        Ne,  ///< Not used in canonical form
        Ult,
        Ule,
        Ugt, ///< Not used in canonical form
        Uge, ///< Not used in canonical form
        Slt,
        Sle,
        Sgt, ///< Not used in canonical form
        Sge, ///< Not used in canonical form

        LastKind = Sge,

        BinaryKindFirst = Add,
        BinaryKindLast = SDiv,
        CmpKindFirst = Eq,
        CmpKindLast = Sge
    };
public:
    unsigned hashValue;
    Expr() { Expr::count++; }

    virtual ~Expr() { Expr::count--; }

    virtual Kind getKind() const = 0;
    virtual Width getWidth() const = 0;

    virtual unsigned getNumKids() const = 0;
    virtual ref<Expr> getKid(unsigned i) const = 0;

    // TODO: Construct debug components such as print and dump
    //      to make develop life easier ?
    // virtual void print(llvm::raw_ostream& os) const;

    /// dump - Print the expression to stderr.
    // void dump() const;
      /// Returns the pre-computed hash of the current expression
    virtual unsigned hash() const { return hashValue; }

    /// (Re)computes the hash of the current expression.
    /// Returns the hash value. 
    virtual unsigned computeHash();

    /// isZero - Is this a constant zero.
    bool isZero() const;
    
    /// isTrue - Is this the true expression.
    bool isTrue() const;

    /// isFalse - Is this the false expression.
    bool isFalse() const;

    static void printKind(llvm::raw_ostream& os, Kind k);
    static void printWidth(llvm::raw_ostream& os, Width w);

};

class NonConstantExpr : public Expr {
public:
    static bool classof(const Expr* e) {
        return e->getKind() != Expr::Constant;
    }
    static bool classof(const NonConstantExpr *) { return true; }
};

class BinaryExpr : public NonConstantExpr {
public:
    ref<Expr> left, right;

public:
    unsigned getNumKids() const { return 2; }

    ref<Expr> getKid(unsigned i) const {
        switch (i) {
        case 0: return left;
        case 1: return right;
        default: return 0;
        }
    }
    protected:
    BinaryExpr(const ref<Expr> &l, const ref<Expr> &r) : left(l), right(r) {}

public:
    static bool classof(const Expr *E) {
        Kind k = E->getKind();
        return Expr::BinaryKindFirst <= k && k <= Expr::BinaryKindLast;
    }
    static bool classof(const BinaryExpr *) { return true; }
};

class CmpExpr : public BinaryExpr {

protected:
    CmpExpr(ref<Expr> l, ref<Expr> r) : BinaryExpr(l,r) {}
    
    public:                                                       
    Width getWidth() const { return Bool; }

    static bool classof(const Expr *E) {
        Kind k = E->getKind();
        return Expr::CmpKindFirst <= k && k <= Expr::CmpKindLast;
    }
    static bool classof(const CmpExpr *) { return true; }
};


class NotExpr : public NonConstantExpr { 
public:
    static const Kind kind = Not;
    static const unsigned numKids = 1;
    
    ref<Expr> expr;

    public:  
    static ref<Expr> alloc(const ref<Expr> &e) {
        ref<Expr> r(new NotExpr(e));
        r->computeHash();
        return r;
    }
    
    static ref<Expr> create(const ref<Expr> &e);

    Width getWidth() const { return expr->getWidth(); }
    Kind getKind() const { return Not; }

    unsigned getNumKids() const { return numKids; }
    ref<Expr> getKid(unsigned i) const { return expr; }

    virtual ref<Expr> rebuild(ref<Expr> kids[]) const { 
        return create(kids[0]);
    }

    virtual unsigned computeHash();

    public:
    static bool classof(const Expr *E) {
        return E->getKind() == Expr::Not;
    }
  static bool classof(const NotExpr *) { return true; }

private:
    NotExpr(const ref<Expr> &e) : expr(e) {}

protected:
    virtual int compareContents(const Expr &b) const {
        // No attributes to compare.
        return 0;
    }
};

// Arithmetic/Bit Exprs

#define ARITHMETIC_EXPR_CLASS(_class_kind)                                     \
class _class_kind##Expr : public BinaryExpr {                                \
public:                                                                      \
    static const Kind kind = _class_kind;                                      \
    static const unsigned numKids = 2;                                         \
public:                                                                      \
    _class_kind##Expr(const ref<Expr> &l, const ref<Expr> &r)                  \
        : BinaryExpr(l, r) {}                                                  \
    static ref<Expr> alloc(const ref<Expr> &l, const ref<Expr> &r) {           \
    ref<Expr> res(new _class_kind##Expr(l, r));                              \
    res->computeHash();                                                      \
    return res;                                                              \
    }                                                                          \
    static ref<Expr> create(const ref<Expr> &l, const ref<Expr> &r);           \
    Width getWidth() const { return left->getWidth(); }                        \
    Kind getKind() const { return _class_kind; }                               \
    virtual ref<Expr> rebuild(ref<Expr> kids[]) const {                        \
    return create(kids[0], kids[1]);                                         \
    }                                                                          \
    static bool classof(const Expr *E) {                                       \
    return E->getKind() == Expr::_class_kind;                                \
    }                                                                          \
    static bool classof(const _class_kind##Expr *) { return true; }            \
    protected:                                                                   \
    virtual int compareContents(const Expr &b) const {                         \
    /* No attributes to compare.*/                                           \
    return 0;                                                                \
    }                                                                          \
};

ARITHMETIC_EXPR_CLASS(Add)
ARITHMETIC_EXPR_CLASS(Sub)
ARITHMETIC_EXPR_CLASS(Mul)
ARITHMETIC_EXPR_CLASS(UDiv)
ARITHMETIC_EXPR_CLASS(SDiv)

// Comparison Exprs

#define COMPARISON_EXPR_CLASS(_class_kind)                                     \
class _class_kind##Expr : public CmpExpr {                                   \
public:                                                                      \
    static const Kind kind = _class_kind;                                      \
    static const unsigned numKids = 2;                                         \
public:                                                                      \
    _class_kind##Expr(const ref<Expr> &l, const ref<Expr> &r)                  \
        : CmpExpr(l, r) {}                                                     \
    static ref<Expr> alloc(const ref<Expr> &l, const ref<Expr> &r) {           \
    ref<Expr> res(new _class_kind##Expr(l, r));                              \
    res->computeHash();                                                      \
    return res;                                                              \
    }                                                                          \
    static ref<Expr> create(const ref<Expr> &l, const ref<Expr> &r);           \
    Kind getKind() const { return _class_kind; }                               \
    virtual ref<Expr> rebuild(ref<Expr> kids[]) const {                        \
    return create(kids[0], kids[1]);                                         \
    }                                                                          \
    static bool classof(const Expr *E) {                                       \
    return E->getKind() == Expr::_class_kind;                                \
    }                                                                          \
    static bool classof(const _class_kind##Expr *) { return true; }            \
protected:                                                                   \
    virtual int compareContents(const Expr &b) const {                         \
      /* No attributes to compare. */                                          \
    return 0;                                                                \
    }                                                                          \
};

COMPARISON_EXPR_CLASS(Eq)
COMPARISON_EXPR_CLASS(Ne)
COMPARISON_EXPR_CLASS(Ult)
COMPARISON_EXPR_CLASS(Ule)
COMPARISON_EXPR_CLASS(Ugt)
COMPARISON_EXPR_CLASS(Uge)
COMPARISON_EXPR_CLASS(Slt)
COMPARISON_EXPR_CLASS(Sle)
COMPARISON_EXPR_CLASS(Sgt)
COMPARISON_EXPR_CLASS(Sge)


// Terminal Exprs

class ConstantExpr : public Expr {
public:
    static const Kind kind = Constant;
    static const unsigned numKids = 0;

private:
    llvm::APInt value;
    ConstantExpr(const llvm::APInt &v) : value(v) {}

public:
    ~ConstantExpr() {}

    Width getWidth() const { return value.getBitWidth(); }
    Kind getKind() const { return Constant; }

    unsigned getNumKids() const { return 0; }
    ref<Expr> getKid(unsigned i) const { return 0; }

  /// getAPValue - Return the arbitrary precision value directly.
  ///
  /// Clients should generally not use the APInt value directly and instead use
  /// native ConstantExpr APIs.
    const llvm::APInt &getAPValue() const { return value; }

  /// getZExtValue - Returns the constant value zero extended to the
  /// return type of this method.
  ///
  ///\param bits - optional parameter that can be used to check that the
  /// number of bits used by this constant is <= to the parameter
  /// value. This is useful for checking that type casts won't truncate
  /// useful bits.
  ///
  /// Example: unit8_t byte= (unit8_t) constant->getZExtValue(8);
    uint64_t getZExtValue(unsigned bits = 64) const {
        assert(getWidth() <= bits && "Value may be out of range!");
        return value.getZExtValue();
    }

  /// getLimitedValue - If this value is smaller than the specified limit,
  /// return it, otherwise return the limit value.
    uint64_t getLimitedValue(uint64_t Limit = ~0ULL) const {
        return value.getLimitedValue(Limit);
    }

  /// toString - Return the constant value as a string
  /// \param Res specifies the string for the result to be placed in
  /// \param radix specifies the base (e.g. 2,10,16). The default is base 10
    void toString(std::string &Res, unsigned radix = 10) const;

    int compareContents(const Expr &b) const {
        const ConstantExpr &cb = static_cast<const ConstantExpr &>(b);
        if (getWidth() != cb.getWidth())
        return getWidth() < cb.getWidth() ? -1 : 1;
        if (value == cb.value)
        return 0;
        return value.ult(cb.value) ? -1 : 1;
    }

    virtual ref<Expr> rebuild(ref<Expr> kids[]) const {
        assert(0 && "rebuild() on ConstantExpr");
        return const_cast<ConstantExpr *>(this);
    }

    virtual unsigned computeHash();

    static ref<Expr> fromMemory(void *address, Width w);
    void toMemory(void *address);

    static ref<ConstantExpr> alloc(const llvm::APInt &v) {
        ref<ConstantExpr> r(new ConstantExpr(v));
        r->computeHash();
        return r;
    }

    static ref<ConstantExpr> alloc(const llvm::APFloat &f) {
        return alloc(f.bitcastToAPInt());
    }

    static ref<ConstantExpr> alloc(uint64_t v, Width w) {
        return alloc(llvm::APInt(w, v));
    }

    static ref<ConstantExpr> create(uint64_t v, Width w) {
        return alloc(v, w);
    }

  static bool classof(const Expr *E) { return E->getKind() == Expr::Constant; }
  static bool classof(const ConstantExpr *) { return true; }

  /* Utility Functions */

  /// isZero - Is this a constant zero.
    bool isZero() const { return getAPValue().isMinValue(); }

  /// isOne - Is this a constant one.
    bool isOne() const { return getLimitedValue() == 1; }

  /// isTrue - Is this the true expression.
    bool isTrue() const {
        return (getWidth() == Expr::Bool && value.getBoolValue() == true);
    }

  /// isFalse - Is this the false expression.
    bool isFalse() const {
        return (getWidth() == Expr::Bool && value.getBoolValue() == false);
    }

  /// isAllOnes - Is this constant all ones.
    bool isAllOnes() const {
        return getAPValue().isAllOnes();
    }
};
    // Implementations
    inline bool Expr::isZero() const {
    if (const ConstantExpr *CE = llvm::dyn_cast<ConstantExpr>(this))
        return CE->isZero();
    return false;
    }
    
    inline bool Expr::isTrue() const {
    assert(getWidth() == Expr::Bool && "Invalid isTrue() call!");
    if (const ConstantExpr *CE = llvm::dyn_cast<ConstantExpr>(this))
        return CE->isTrue();
    return false;
    }
    
    inline bool Expr::isFalse() const {
    assert(getWidth() == Expr::Bool && "Invalid isFalse() call!");
    if (const ConstantExpr *CE = llvm::dyn_cast<ConstantExpr>(this))
        return CE->isFalse();
    return false;
    }

#endif // EXPR_H