#include "Solver.h"
#include "Constraints.h"
#include "SolverImpl.h"

#include <memory>

namespace miniklee {

class TinySolverImpl : public SolverImpl {
public:
    TinySolverImpl();

    bool computeValidity(const Query &);
    bool computeTruth(const Query &, bool &isValid);
    bool computeValue(const Query &, ref<Expr> &result);
    bool computeInitialValues(const Query &,
                                const std::vector<const SymbolicExpr *> &objects,
                                std::vector<std::vector<int32_t> > &values);
    bool internalRunSolver(const Query &query, 
                            const std::vector<const SymbolicExpr *> *objects,
                            std::vector<std::vector<int32_t> > *values);
    void solveConstraint(const ref<Expr> &e, int32_t &res);
    SolverRunStatus getOperationStatusCode();
};

TinySolverImpl::TinySolverImpl() {}

bool TinySolverImpl::computeValidity(const Query &query) {
    llvm::errs() << "DEBUG: computeValidity\n";
    return internalRunSolver(query, NULL /* objects */, NULL /* values */);
}

bool TinySolverImpl::computeTruth(const Query &, bool &isValid) {
    return false;
}

bool TinySolverImpl::computeValue(const Query &, ref<Expr> &result) {
    return false;
}

bool TinySolverImpl::computeInitialValues(
    const Query &query, const std::vector<const SymbolicExpr *> &objects,
    std::vector<std::vector<int32_t> > &values) {
    assert(objects.size() == 1 && "Currently support only one object");
    assert(values.size()  == 1 && "Currently support only one object");
    return internalRunSolver(query, &objects,  &values);
}

bool TinySolverImpl::internalRunSolver(
    const Query &query, const std::vector<const SymbolicExpr *> *objects,
    std::vector<std::vector<int32_t> > *values) {

    llvm::errs() << "DEBUG: internalRunSolver 1\n";
    int32_t res;
    solveConstraint(query.expr, res);
    llvm::errs() << "DEBUG: HIT internalRunSolver 2\n";

    for (auto c : query.constraints) {
        assert(c && "Not Null");
        // Logically, constraints are added incrementally, 
        // so we only need to check any one of the previous ones. 
        // But what if someone is unreasonable? So we check all of them.
        int32_t prev;
        solveConstraint(c, prev);
        if (res != prev) return false;
    }
    llvm::errs() << "DEBUG: HIT internalRunSolver 3\n";

    if (objects && objects->size() == 1) {
        assert(values);
        values->front().push_back(res);
    }
    return true;
}

void TinySolverImpl::solveConstraint(const ref<Expr> &e, int32_t &res) {
    llvm::errs() << "DEBUG: solveConstraint\n";
    assert(e->getKind() == Expr::Eq && "Constraint must be an equality");
    llvm::errs() << "DEBUG: HIT solveConstraints\n";
    // ax + b = c
    // => x = (c - b) / a

    ref<EqExpr> eq = dyn_cast<EqExpr>(e.get());
    llvm::errs() << "DEBUG: HIT 1\n";

    // Assume a is 1
    auto left = dyn_cast<AddExpr>(eq->getKid(0).get());

    llvm::errs() << "DEBUG: HIT 2\n";

    auto left_const = dyn_cast<ConstantExpr>(left->getKid(1).get());
    int32_t b = static_cast<int32_t>(left_const->getAPValue().getSExtValue());

    llvm::errs() << "DEBUG: HIT 3\n";
    
    auto right = dyn_cast<ConstantExpr>(e->getKid(1).get());
    int32_t c = static_cast<int32_t>(right->getAPValue().getSExtValue());
    assert(right && "Currently follow the form of ax + b = c (c is ConstantExpr)");

    // WARNING: This is a hack. Caution when loss of accuracy when a != 1

    llvm::errs() << "DEBUG: " << c - b << " \n";

    res = c - b;
}

SolverImpl::SolverRunStatus TinySolverImpl::getOperationStatusCode() {
    return SOLVER_RUN_STATUS_FAILURE;
}

std::unique_ptr<Solver> createTinySolver() {
    return std::make_unique<Solver>(std::make_unique<TinySolverImpl>());
}
}
