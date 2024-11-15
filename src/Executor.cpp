#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instruction.h>
#include "llvm/IR/IntrinsicInst.h"
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <stack>
#include <iostream>

#include "Executor.h"
#include "ExecutionState.h"


using namespace llvm;
using namespace miniklee;

Executor::Executor(std::unique_ptr<llvm::Module> module) 
    : module(std::move(module)) {
    this->solver = createCoreSolver(CoreSolverType::TINY_SOLVER);
}

void Executor::runFunctionAsMain(Function *function) {
    ExecutionState initialState(function);
    states.addState(&initialState);

    // main interpreter loop
    while (!states.isEmpty()) {
        // FIXME: Need searcher to choose next state?
        ExecutionState &state = states.selectState();

        Instruction *i = &*state.pc;
        stepInstruction(state);

        executeInstruction(state, i);

        updateStates(&state);
    }
}

void Executor::stepInstruction(ExecutionState& state) {
    state.prevPC = state.pc;
    ++state.pc;

    // TODO: Other logic code to handle haltExecution
}

void Executor::executeInstruction(ExecutionState& state, Instruction* i) {
    switch (i->getOpcode()) {
    // Control flow
    case Instruction::Ret: {
        errs() << "State " << state.getID() << " Ret\n";
        // // FIXME: Handle return
        removedStates.push_back(&state);
        break;
    }
    case Instruction::Br: {
        errs() << "State " << state.getID() << " Br\n";
        BranchInst *bi = cast<BranchInst>(i);
        if (bi->isUnconditional()) {
            transferToBasicBlock(bi->getSuccessor(0), state);
        } else {
            assert(bi->getCondition() == bi->getOperand(0) &&
                    "Wrong operand index!");
            
            Instruction *condInstr = dyn_cast<Instruction>(bi->getCondition()); assert(condInstr);
            ref<Expr> cond = getInstructionValue(state, condInstr); assert(cond);

            Executor::StatePair branches = fork(state, cond);
            if (branches.first)
                transferToBasicBlock(bi->getSuccessor(0), *branches.first);
            if (branches.second)
                transferToBasicBlock(bi->getSuccessor(1), *branches.second);
        }
        break;
    }
    case Instruction::Call: {
        if (isa<DbgInfoIntrinsic>(i))
            break;

        const CallBase *cb = cast<CallBase>(i);
        assert(cb->getCalledFunction()->getName() == "make_symbolic"
                && "Unknown call instruction");
        assert(cb->arg_size()  == 3 && "Unexpected Error");

        errs() << "State " << state.getID() << " Mk Sym\n";
        // 1. Make symbolic
        Instruction *sym = dyn_cast<Instruction>(cb->getArgOperand(0));
        assert(sym && "First argument should be a variable (Instruction type)");

        // 2. Deal with the size (4 bytes)
        ConstantInt *size = dyn_cast<ConstantInt>(cb->getArgOperand(1));
        assert(size->getSExtValue() == Expr::Int32 / CHAR_BIT);

        // 3. Retrieve the name
        Value *arg = cb->getArgOperand(2); 
        // GEP constant expression
        auto *gepExpr = cast<llvm::ConstantExpr>(arg);
        assert(gepExpr->getOpcode() == llvm::Instruction::GetElementPtr && "GEP Expected");
        // Get the global variable
        auto *globalVar = cast<GlobalVariable>(gepExpr->getOperand(0));
        auto *strArray = cast<ConstantDataArray>(globalVar->getInitializer());
        assert(strArray->isString() && "String Expected");

        executeMakeSymbolic(state, sym, strArray->getAsString().str());

        break;
    }
    // Memory instructions...
    case Instruction::Alloca: {
        // TODO: remove debug info
        errs() << "State " << state.getID() << ": Alloca\n";
        AllocaInst *ai = cast<AllocaInst>(i);
        assert("Support Int32 type only" 
                && ai->getAllocatedType() == Type::getInt32Ty(i->getContext()));
        const DataLayout &dataLayout = module->getDataLayout();
        unsigned sizeInBits = dataLayout.getTypeSizeInBits(ai->getAllocatedType());
        executeAlloc(state, sizeInBits, i);
        break;
    }

    case Instruction::Load: {
        errs() << "State " << state.getID() << " Load\n";
        LoadInst *li = cast<LoadInst>(i);
        
        Instruction *address = dyn_cast<Instruction>(li->getPointerOperand());
        assert(address && "Pointer Operand expected");

        executeMemoryOperation(state, false, address, 0, li /* simply the Load instr itself */);
        break;
    }

    case Instruction::Store: {
        errs() << "State " << state.getID() << " Store\n";
        StoreInst *si = cast<StoreInst>(i);
        // Retrieve the target address
        Instruction *target = dyn_cast<Instruction>(si->getOperand(1));
        if (!target)
            assert(false && "Target is not an Instruction");
        // Retrieve the value to be stored
        Value *value = si->getValueOperand();

        if (ConstantInt *ci = dyn_cast<ConstantInt>(value)) {
            // Deal with constant, just assign value
            assert(ci->getType()->isIntegerTy(32) && "Int32 expected");
            int32_t rawInt32Value = static_cast<int32_t>(ci->getSExtValue());
            ref<miniklee::ConstantExpr> int32Value = miniklee::ConstantExpr::alloc(rawInt32Value, Expr::Int32);
            executeMemoryOperation(state, true, target, int32Value, 0);
        } else {
            // Deal with pointer, then fetch pointed and assign
            Instruction *ptr = dyn_cast<Instruction>(value); assert(ptr);
            ref<Expr> pointed = getInstructionValue(state, ptr);

            assert(pointed && "No coresponding symblic value found for pointer!");
            executeMemoryOperation(state, true, target, pointed, 0);
        }

        break;
    }

    // Arithmetic
    case Instruction::Add: {
        errs() << "State " << state.getID() << " Add\n";
        BinaryOperator *ao = cast<BinaryOperator>(i);

        ref<Expr> lshValue = getValue(state, ao->getOperand(0));
        ref<Expr> rshValue = getValue(state, ao->getOperand(1));
        ref<Expr> add = AddExpr::create(lshValue, rshValue);

        executeMemoryOperation(state, true, i /* simply the Load instr itself */, add, 0);
        break;
    }

    case Instruction::Sub: {
        errs() << "State " << state.getID() << " Sub\n";
        BinaryOperator *ao = cast<BinaryOperator>(i);

        ref<Expr> lshValue = getValue(state, ao->getOperand(0));
        ref<Expr> rshValue = getValue(state, ao->getOperand(1));
        ref<Expr> sub = SubExpr::create(lshValue, rshValue);

        executeMemoryOperation(state, true, i /* simply the Load instr itself */, sub, 0);
        break;
    }

    // Compare
    case Instruction::ICmp: {
        CmpInst *ci = cast<CmpInst>(i);
        ICmpInst *ii = cast<ICmpInst>(ci);

        switch(ii->getPredicate()) {
        case ICmpInst::ICMP_EQ: {
            errs() << "State " << state.getID() << " ICMP_EQ comparison\n";
            ref<Expr> lshValue = getValue(state, ii->getOperand(0));
            ref<Expr> rshValue = getValue(state, ii->getOperand(1));
            ref<Expr> eq = EqExpr::create(lshValue, rshValue);

            executeMemoryOperation(state, true, ii, eq, 0);
            break;
        }
        case ICmpInst::ICMP_NE: {
            assert(false && "TODO: ICMP_NE comparison");
            break;
        }
        case ICmpInst::ICMP_UGT: {
            assert(false && "TODO: ICMP_UGT comparison");
            break;
        }
        case ICmpInst::ICMP_UGE: {
            assert(false && "TODO: ICMP_UGE comparison");
            break;
        }
        case ICmpInst::ICMP_ULT: {
            assert(false && "TODO: ICMP_ULT comparison");
            break;
        }
        case ICmpInst::ICMP_ULE: {
            assert(false && "TODO: ICMP_ULE comparison");
            break;
        }
        case ICmpInst::ICMP_SGT: {
            assert(false && "TODO: ICMP_SGT comparison");
            break;
        }
        case ICmpInst::ICMP_SGE: {
            assert(false && "TODO: ICMP_SGE comparison");
            break;
        }
        case ICmpInst::ICMP_SLT: {
            errs() << "State " << state.getID() << " ICMP_SLT comparison\n";
            ref<Expr> lshValue = getValue(state, ii->getOperand(0));
            ref<Expr> rshValue = getValue(state, ii->getOperand(1));
            ref<Expr> slt = SltExpr::create(lshValue, rshValue);

            executeMemoryOperation(state, true, ii, slt, 0);
            break;
        }
        case ICmpInst::ICMP_SLE: {
            assert(false && "TODO: ICMP_SLE comparison");
            break;
        }
        default:
            assert(false && " Unknown comparison. TODO: Use terminateStateOnExecError to finish.");
        }
        break;
    }
    
    default:
        errs() << "Unknown instruction: " << *i << "\n";
        assert(false && "Unknown instruction");
        break;
    }

}

void Executor::updateStates(ExecutionState *current)  {
    // std::cout << GREY_TEXT("Updating states") << std::endl;
    assert(current);

    states.addState(addedStates.begin(), addedStates.end());
    addedStates.clear();

    for (std::vector<ExecutionState *>::iterator it = removedStates.begin(),
                                                ie = removedStates.end();
        it != ie; ++it) {
        ExecutionState *es = *it;
        std::deque<ExecutionState*>::iterator it2 = states.find(es);
        assert(it2 != states.end());

        states.erase(it2);
    }
    removedStates.clear();
}

void Executor::transferToBasicBlock(BasicBlock *dst, ExecutionState &state) {
    state.pc = dst->begin();
}


void Executor::executeAlloc(ExecutionState& state, unsigned size, Instruction* inst) {
    assert("Only Support Int32"
        && size == Type::getInt32Ty(inst->getContext())->getPrimitiveSizeInBits());
    // WARNING: Dangling pointer?
    //          Maybe we should refactor the ConstantExpr to be more specific
    state.locals.insert({inst, miniklee::InvalidKindExpr::create(0, size)});
}

ref<Expr> Executor::getInstructionValue(ExecutionState& state, Instruction* i) {
    // return state.locals.find(i)->second;
    auto it = state.locals.find(i);
    if (it != state.locals.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}


void Executor::executeMemoryOperation(ExecutionState& state, 
                            bool isWrite, 
                            Instruction *address,
                            ref<Expr> value, /* undef if read */
                            Instruction* target /* undef if wirte*/) {
    if (isWrite) { // Interpret the Store instruction
        // WARNING: Check whether override the latent value?
        assert(!target);
        auto it = state.locals.find(address);
        if (it != state.locals.end()) {
            state.locals.erase(it);
        }
        state.locals.insert({address, value});
    } else { // Interpret the Load instruction
        assert(!value);
        // FIXME: Wrap a set of APIs
        auto loadedValue = state.locals.find(address);
        if (loadedValue == state.locals.end()) {
            assert(false && "Unexpected Error");
        } else {
            auto it = state.locals.find(target);
            if (it != state.locals.end()) {
                state.locals.erase(it);
            }
            state.locals.insert({target, loadedValue->second});
        }
    }
}

ref<Expr> Executor::getValue(ExecutionState& state, Value* value) {
    if (Instruction *v= dyn_cast<Instruction>(value)) {
        ref<Expr> rawValue = getInstructionValue(state, v);
        assert(rawValue && "Value Not Stored");
        return rawValue;
    } else if (ConstantInt *constValue = dyn_cast<ConstantInt>(value)) {
        return miniklee::ConstantExpr::create(
            static_cast<int32_t>(constValue->getSExtValue()),
            Expr::Int32);
    } else {
        assert(false && "Unexpected Error");
    }

}

void Executor::executeMakeSymbolic(ExecutionState& state, Instruction *symAddress, std::string name) {
    // Register the variable (Instruction type) to be symbolic
    executeMemoryOperation(state, true, symAddress, SymbolicExpr::create(name), 0);
}


Executor::StatePair Executor::fork(ExecutionState &current,
                                    ref<Expr> condition) {
    // Invoke solver to determinie the feasibility of the condition
    bool trueBranch = this->solver->evaluate(Query(current.constraints, condition));
    bool falseBranch = this->solver->evaluate(Query(current.constraints, NotExpr::create(condition)));

    if (trueBranch && !falseBranch /* Solver::True */) {
        return StatePair(&current, nullptr);
    } else if (!trueBranch && falseBranch /* Solver::False */) {
        return StatePair(nullptr, &current);
    } else if (trueBranch && falseBranch /* Solver::Unknown */ ) {
        ExecutionState *falseState, *trueState = &current;
        falseState = trueState->branch();
        addedStates.push_back(falseState);

        addConstraint(*trueState, condition);
        addConstraint(*falseState, NotExpr::create(condition));

        return StatePair(trueState, falseState);
    } else { assert(false && "Unexpected Error"); }
}

void Executor::addConstraint(ExecutionState &state, ref<Expr> condition) {
    if (miniklee::ConstantExpr *CE = dyn_cast<miniklee::ConstantExpr>(condition.get())) {
        if (!CE->isTrue())
        llvm::report_fatal_error("attempt to add invalid constraint");
        return;
    }

    state.addConstraint(condition);
}
