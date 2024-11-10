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

void Executor::runFunctionAsMain(Function *function) {
    ExecutionState initialState(function);
    stateStack.push(initialState);

    // main interpreter loop
    while (!stateStack.empty()) {
        // 1. Select a state to work on.
        // FIXME: Need searcher to choose next state?
        ExecutionState &state = stateStack.top();
        // stateStack.pop();

        Instruction *i = &*state.pc;
        stepInstruction(state);

        executeInstruction(state, i);

        // 4. Update state
    }
}

void Executor::stepInstruction(ExecutionState& state) {
    state.prevPC = state.pc;
    ++state.pc;

    // TODO: Other logic code to handle haltExecution
}

void Executor::executeInstruction(ExecutionState& state, Instruction* i) {
    switch (i->getOpcode()) {
    case Instruction::Ret: {
        errs() << "Return\n";
        // // FIXME: Handle return
        stateStack.pop();
        break;
    }

    // Memory instructions...
    case Instruction::Alloca: {
        // TODO: remove debug info
        errs() << "Alloca\n";
        AllocaInst *ai = cast<AllocaInst>(i);
        assert("Support Int32 type only" 
                && ai->getAllocatedType() == Type::getInt32Ty(i->getContext()));
        const DataLayout &dataLayout = module->getDataLayout();
        unsigned sizeInBits = dataLayout.getTypeSizeInBits(ai->getAllocatedType());
        executeAlloc(state, sizeInBits, i);
        break;
    }

    case Instruction::Load: {
        errs() << "Load\n";
        LoadInst *li = cast<LoadInst>(i);
        
        Instruction *address = dyn_cast<Instruction>(li->getPointerOperand());
        assert(address && "Pointer Operand expected");

        executeMemoryOperation(state, false, address, 0, li /* simply the Load instr itself */);
        break;
    }

    case Instruction::Store: {
        errs() << "Store\n";
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

            // DEBUG INFO
            llvm::errs() << "Pointer: " << *ptr << "\n";
            // Now please interpret the Add Intruction, update its vlaue in the coresponding symbolic memory
            // DEBUG INFO

            ref<Expr> pointed = getValue(ptr, state);
            assert(pointed && "No coresponding symblic value found for pointer!");

            executeMemoryOperation(state, true, target, pointed, 0);
        }

        break;
    }

    case Instruction::Add: {
        errs() << "Add\n";
        BinaryOperator *ao = cast<BinaryOperator>(i);
        // FIXME: Assume the two operands are the constant type
        Instruction *lhs = cast<Instruction>(ao->getOperand(0));
        Instruction *rhs = cast<Instruction>(ao->getOperand(1));

        auto rawLshValue = getValue(lhs, state); assert(rawLshValue && "LHS Value Not Stored");
        ref<miniklee::ConstantExpr> lhsValue = dyn_cast<miniklee::ConstantExpr>(rawLshValue.get());
        assert(lhsValue && "TODO: Current only support ConstantValue, symbolic value TBD");

        auto rawRshValue = getValue(rhs, state); assert(rawRshValue && "RHS Value Not Stored");
        ref<miniklee::ConstantExpr> rhsValue = dyn_cast<miniklee::ConstantExpr>(rawRshValue.get());
        assert(rhsValue && "TODO: Current only support ConstantValue, symbolic value TBD");
        // WARNING: So many cast functions, caution when there is dangling pointers
        ref<miniklee::ConstantExpr> int32Value =
                miniklee::ConstantExpr::alloc(static_cast<int32_t>(lhsValue->getAPValue().getSExtValue())
                                            + static_cast<int32_t>(rhsValue->getAPValue().getSExtValue()), Expr::Int32);
        executeMemoryOperation(state, true, i /* simply the Load instr itself */, int32Value, 0);
        break;
    }
    case Instruction::Br: {
        errs() << "Br\n";
        BranchInst *bi = cast<BranchInst>(i);
        if (bi->isUnconditional()) {
            transferToBasicBlock(bi->getSuccessor(0), state);
        } else {
            assert(bi->getCondition() == bi->getOperand(0) &&
                    "Wrong operand index!");
            
            // // FIXME: Handle branches
            errs() << "Conditional Branch\n";
            transferToBasicBlock(bi->getSuccessor(1), state);

            // ref<Expr> cond = eval(ki, 0, state).value;

            // cond = optimizer.optimizeExpr(cond, false);
            // Executor::StatePair branches = fork(state, cond, false, BranchType::Conditional);

            // if (branches.first)
            //     transferToBasicBlock(bi->getSuccessor(0), *branches.first);
            // if (branches.second)
            //     transferToBasicBlock(bi->getSuccessor(1), *branches.second);
        }
        break;
    }

    case Instruction::ICmp: {
        CmpInst *ci = cast<CmpInst>(i);
        ICmpInst *ii = cast<ICmpInst>(ci);

        switch(ii->getPredicate()) {
        case ICmpInst::ICMP_EQ: {
            assert(false && "TODO: ICMP_EQ comparison");
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
            errs() << "ICMP_SLT comparison\n";
            // FIXME: Handle symbolic value instead of just constant value
            Instruction *lhs = cast<Instruction>(ii->getOperand(0));
            ref<Expr> rawLhsValue = getValue(lhs, state);
            assert(rawLhsValue && "LHS Value Not Stored");
            ref<miniklee::ConstantExpr> LhsValue = dyn_cast<miniklee::ConstantExpr>(rawLhsValue.get());
            assert(LhsValue && "Currently only support constant, TODO: symbolic value TBD");

            Value *value = ii->getOperand(1);
            ConstantInt *ci = dyn_cast<ConstantInt>(value);
            // TODO: Support compare two Variables instead of one Variable and one Constant
            assert(value && "Support constant value to be compared only. e.g., SLT <Variable> <Constant>");

            // Now interpret the SLT's semantics
            int32_t cmpRes = LhsValue->getAPValue().getSExtValue() < ci->getSExtValue();

            executeMemoryOperation(state, true, ii,
                miniklee::ConstantExpr::create(cmpRes, Expr::Int32), 0);

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


    case Instruction::Call:
        if (isa<DbgInfoIntrinsic>(i))
            break;
        assert(false && "Unknown call instruction");
    
    default:
        errs() << "Unknown instruction: " << *i << "\n";
        assert(false && "Unknown instruction");
        break;
    }

}

void Executor::updateStates(ExecutionState *current)  {
    errs() << "Updating states\n";
    // TODO: Implement me
    assert(current);
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

ref<Expr> Executor::getValue(Instruction* i, ExecutionState& state) {
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
        auto loadedValue = state.locals.find(address)->second;
        state.locals.insert({target, loadedValue});
    }
}