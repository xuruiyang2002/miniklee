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

    // FIXME: DEBUGU INFO
    auto cur_pc = initialState.pc;
    cur_pc++;
    cur_pc++;
    cur_pc++;
    cur_pc++;
    cur_pc++;
    cur_pc++;
    cur_pc++;
    cur_pc++;
    cur_pc++;
    cur_pc++;
    cur_pc++;
    cur_pc++;
    cur_pc++;
    cur_pc++;
    cur_pc++;
    cur_pc++;
    cur_pc++;
    llvm::errs() << *cur_pc << "\n";

    if (auto bi = llvm::dyn_cast<llvm::BranchInst>(cur_pc)) {
        if (bi->isUnconditional()) {
            cur_pc = bi->getSuccessor(0)->begin();
            llvm::errs() << *cur_pc << "\n";
        }
    }

    
    stateStack.push(initialState);

    // main interpreter loop
    while (!stateStack.empty()) {
        // 1. Select a state to work on.
        // FIXME: Need searcher to choose next state?
        ExecutionState &state = stateStack.top();
        // stateStack.pop();

        llvm::Instruction *inst = &*state.pc;
        stepInstruction(state);

        executeInstruction(state, inst);

        // 4. Update state

        // FIXME: DEBUG INFO 
        // for (auto& block : *function) {
        //     for (auto& inst : block) {
        //         executeInstruction(inst, currentState);
        //     }
        //     llvm::errs() << "------ \n";
        // }
    }
}

void Executor::stepInstruction(ExecutionState& state) {
    state.prevPC = state.pc;
    ++state.pc;

    // TODO: Other logic code to handle haltExecution
}

void Executor::executeInstruction(ExecutionState& state, llvm::Instruction* inst) {
    switch (inst->getOpcode()) {
    case llvm::Instruction::Alloca:
        llvm::errs() << "Alloca\n";
        break;

    case llvm::Instruction::Add:
        llvm::errs() << "Add\n";
        break;

    case llvm::Instruction::Load:
        llvm::errs() << "Load\n";
        break;

    case llvm::Instruction::Store:
        llvm::errs() << "Store\n";
        break;

    case llvm::Instruction::Br: {
        llvm::errs() << "Br\n";
        llvm::BranchInst *bi = llvm::cast<llvm::BranchInst>(inst);
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
        llvm::CmpInst *ci = llvm::cast<llvm::CmpInst>(inst);
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
        if (llvm::isa<llvm::DbgInfoIntrinsic>(inst))
            break;
        assert(false && "Unknown call instruction");
    
    default:
        llvm::errs() << "Unknown instruction: " << *inst << "\n";
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
void Executor::handleBinaryOperation(llvm::BinaryOperator& binOp, ExecutionState& state) {
    llvm::Value* leftOp = binOp.getOperand(0);
    llvm::Value* rightOp = binOp.getOperand(1);

    Expression leftExpr = state.getSymbolic(leftOp);
    Expression rightExpr = state.getSymbolic(rightOp);

    Expression resultExpr("unknown");

    if (binOp.getOpcode() == llvm::Instruction::Add) {
        resultExpr = leftExpr + rightExpr;
    } else if (binOp.getOpcode() == llvm::Instruction::Sub) {
        resultExpr = leftExpr - rightExpr;
    }

    state.setSymbolic(&binOp, resultExpr);

    // Output symbolic expression for tracing purposes
    // std::cout << "Executed " << binOp.getOpcodeName() << " instruction:\n";
    // std::cout << "  Result: " << resultExpr.expr << "\n";
}