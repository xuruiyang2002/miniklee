#include "Solver.h"
#include "Constraints.h"
#include "SolverImpl.h"

#include <memory>
#include <random>
#include <algorithm>

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
    int32_t generateRandomExcluding(const std::vector<int32_t>& cannot);
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

    int32_t res;
    bool assigned = false; // Record whether the res is assigned
    std::vector<int32_t> cannot;

    // Deal with current branch condition
    if (query.expr->getKind() == Expr::Eq) {
        solveConstraint(query.expr, res);
        assigned = true;
    } else if (query.expr->getKind() == Expr::Not){
        int32_t cannotbe;
        solveConstraint(query.expr->getKid(0), cannotbe);
        cannot.push_back(cannotbe);
    } else assert(false && "This compare expression currently not support");

    for (auto c : query.constraints) {
        if (c->getKind() == Expr::Eq) {
            int32_t prev;
            solveConstraint(c, prev);
            if (assigned && (res != prev)) return false;
            else {
                res = prev;
                assigned = true;
            }
        } else if (c->getKind() == Expr::Not) {
            int32_t cannotbe;
            solveConstraint(query.expr->getKid(0), cannotbe);
            if (assigned && (res == cannotbe)) return false;
            cannot.push_back(cannotbe);
        } else assert(false && "This compare expression currently not support");
    }

    if (objects && values) {
        if (!assigned)
            res = generateRandomExcluding(cannot);
        values->front().push_back(res);

        llvm::errs() << "Assigning " << res << " " << "\n";
    }
    return true;
}

/// FIXME: This is really a naive Implementation
void TinySolverImpl::solveConstraint(const ref<Expr> &e, int32_t &res) {
    assert(e->getKind() == Expr::Eq && "Constraint must be an equality");
    // ax + b = c
    // => x = (c - b) / a
    auto eq = dyn_cast<EqExpr>(e.get());
    assert(eq && "Failed to cast to EqExpr");

    auto left = dyn_cast<AddExpr>(eq->getKid(0).get());
    assert(left && "Left side of equality is not an AddExpr");

    auto left_const = dyn_cast<ConstantExpr>(left->getKid(1).get());
    assert(left_const && "Left side constant term is not a ConstantExpr");

    int32_t b = static_cast<int32_t>(left_const->getAPValue().getSExtValue());

    auto right = dyn_cast<ConstantExpr>(e->getKid(1).get());
    assert(right && "Right side of equality is not a ConstantExpr");
    int32_t c = static_cast<int32_t>(right->getAPValue().getSExtValue());

    res = c - b;
}

SolverImpl::SolverRunStatus TinySolverImpl::getOperationStatusCode() {
    return SOLVER_RUN_STATUS_FAILURE;
}

std::unique_ptr<Solver> createTinySolver() {
    return std::make_unique<Solver>(std::make_unique<TinySolverImpl>());
}

int32_t TinySolverImpl::generateRandomExcluding(const std::vector<int32_t>& cannot) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int32_t> dist(std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max());

    int32_t v;
    do {
        v = dist(gen);
    } while (std::find(cannot.begin(), cannot.end(), v) != cannot.end());

    return v;
}

}
