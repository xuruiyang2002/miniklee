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


void Executor::runFunctionAsMain(llvm::Function *function) {
    ExecutionState initialState(function);
    stateStack.push(initialState);

    // main interpreter loop
    while (!stateStack.empty()) {
        // 1. Select a state to work on.
        // FIXME: Need searcher to choose next state?
        ExecutionState &state = stateStack.top();
        // stateStack.pop();

        llvm::Instruction *i = &*state.pc;
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

void Executor::executeInstruction(ExecutionState& state, llvm::Instruction* i) {
    switch (i->getOpcode()) {
    case llvm::Instruction::Ret: {
        llvm::errs() << "Return\n";
        // // FIXME: Handle return
        stateStack.pop();
        break;
    }

    // Memory instructions...
    case llvm::Instruction::Alloca: {
        // TODO: remove debug info
        llvm::errs() << "Alloca\n";
        llvm::AllocaInst *ai = llvm::cast<llvm::AllocaInst>(i);
        assert("Support Int32 type only" 
                && ai->getAllocatedType() == llvm::Type::getInt32Ty(i->getContext()));
        const llvm::DataLayout &dataLayout = module->getDataLayout();
        unsigned sizeInBits = dataLayout.getTypeSizeInBits(ai->getAllocatedType());
        executeAlloc(state, sizeInBits, i);
        break;
    }

    case llvm::Instruction::Load: {
        llvm::errs() << "Load\n";
        llvm::LoadInst *li = llvm::cast<llvm::LoadInst>(i);
        
        llvm::Instruction *address = llvm::dyn_cast<llvm::Instruction>(li->getPointerOperand());
        assert(address && "Pointer Operand expected");

        executeMemoryOperation(state, false, address, 0, li /* simply the Load instr itself */);
        break;
    }

    case llvm::Instruction::Store: {
        llvm::errs() << "Store\n";
        llvm::StoreInst *si = llvm::cast<llvm::StoreInst>(i);
        // Retrieve the target address
        llvm::Instruction *target = llvm::dyn_cast<llvm::Instruction>(si->getOperand(1));
        if (!target)
            assert(false && "Target is not an Instruction");
        // Retrieve the value to be stored
        llvm::Value *value = si->getValueOperand();
        llvm::ConstantInt *ci = llvm::dyn_cast<llvm::ConstantInt>(value);
        assert(ci && "ConstantInt expected");
        assert(ci->getType()->isIntegerTy(32) && "Int32 expected");

        int32_t rawInt32Value = static_cast<int32_t>(ci->getSExtValue());
        ref<Expr> int32Value = ConstantExpr::alloc(rawInt32Value, Expr::Int32);

        executeMemoryOperation(state, true, target, int32Value, 0);

        break;
    }

    case llvm::Instruction::Add:
        llvm::errs() << "Add\n";
        break;

    case llvm::Instruction::Br: {
        llvm::errs() << "Br\n";
        llvm::BranchInst *bi = llvm::cast<llvm::BranchInst>(i);
        if (bi->isUnconditional()) {
            transferToBasicBlock(bi->getSuccessor(0), state);
        } else {
            assert(bi->getCondition() == bi->getOperand(0) &&
                    "Wrong operand index!");
            
            // // FIXME: Handle branches
            llvm::errs() << "Conditional Branch\n";
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

    case llvm::Instruction::ICmp: {
        llvm::CmpInst *ci = llvm::cast<llvm::CmpInst>(i);
        llvm::ICmpInst *ii = llvm::cast<llvm::ICmpInst>(ci);

        switch(ii->getPredicate()) {
        case llvm::ICmpInst::ICMP_EQ: {
            assert(false && "TODO: ICMP_EQ comparison");
            break;
        }
        case llvm::ICmpInst::ICMP_NE: {
            assert(false && "TODO: ICMP_NE comparison");
            break;
        }
        case llvm::ICmpInst::ICMP_UGT: {
            assert(false && "TODO: ICMP_UGT comparison");
            break;
        }
        case llvm::ICmpInst::ICMP_UGE: {
            assert(false && "TODO: ICMP_UGE comparison");
            break;
        }
        case llvm::ICmpInst::ICMP_ULT: {
            assert(false && "TODO: ICMP_ULT comparison");
            break;
        }
        case llvm::ICmpInst::ICMP_ULE: {
            assert(false && "TODO: ICMP_ULE comparison");
            break;
        }
        case llvm::ICmpInst::ICMP_SGT: {
            assert(false && "TODO: ICMP_SGT comparison");
            break;
        }
        case llvm::ICmpInst::ICMP_SGE: {
            assert(false && "TODO: ICMP_SGE comparison");
            break;
        }
        case llvm::ICmpInst::ICMP_SLT: {
            llvm::errs() << "ICMP_SLT comparison\n";
            break;
        }
        case llvm::ICmpInst::ICMP_SLE: {
            assert(false && "TODO: ICMP_SLE comparison");
            break;
        }
        default:
            assert(false && " Unknown comparison. TODO: Use terminateStateOnExecError to finish.");
        }
        break;
    }


    case llvm::Instruction::Call:
        if (llvm::isa<llvm::DbgInfoIntrinsic>(i))
            break;
        assert(false && "Unknown call instruction");
    
    default:
        llvm::errs() << "Unknown instruction: " << *i << "\n";
        assert(false && "Unknown instruction");
        break;
    }

}

void Executor::updateStates(ExecutionState *current)  {
    llvm::errs() << "Updating states\n";
    // TODO: Implement me
    assert(current);
}

void Executor::transferToBasicBlock(llvm::BasicBlock *dst, ExecutionState &state) {
    state.pc = dst->begin();
}


void Executor::executeAlloc(ExecutionState& state, unsigned size, llvm::Instruction* inst) {
    assert("Only Support Int32"
        && size == llvm::Type::getInt32Ty(inst->getContext())->getPrimitiveSizeInBits());
    // WARNING: Dangling pointer?
    //          Maybe we should refactor the ConstantExpr to be more specific
    state.locals.insert({inst, PhantomExpr::create(0, size)});
}

ref<Expr> Executor::getValue(llvm::Instruction* i, unsigned index, ExecutionState& state) {
    assert(index < i->getNumOperands());
    return state.locals.find(i)->second;
}


void Executor::executeMemoryOperation(ExecutionState& state, 
                            bool isWrite, 
                            llvm::Instruction *address,
                            ref<Expr> value, /* undef if read */
                            llvm::Instruction* target /* undef if wirte*/) {
    if (isWrite) { // Interpret the Store instruction
        // WARNING: Check whether override the latent value?
        assert(!target);
        state.locals.insert({address, value});
    } else { // Interpret the Load instruction
        assert(!value);
        // FIXME: Wrap a set of APIs
        auto loadedValue = state.locals.find(address)->second;
        state.locals.insert({target, loadedValue});
    }
}