#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Pass.h>
#include <llvm/Support/Debug.h>

#include <iostream>
#include <list>

#define DEBUG_TYPE "mine"

using namespace llvm;

namespace {
struct BoundsCheck : public FunctionPass {
private:
  Function *Assert = nullptr;

public:
  static char ID;
  BoundsCheck() : FunctionPass(ID) { }

  /// Entry point of the pass; this function performs the actual analysis or
  /// transformation, and is called for each function in the module.
  ///
  /// The returned boolean should be `true` if the function was modified,
  /// `false` if it wasn't.
  bool runOnFunction(Function &F) override {
    IRBuilder<> Builder(F.getContext());

    LLVM_DEBUG({
      dbgs() << "BoundsCheck: processing function '";
      dbgs().write_escaped(F.getName()) << "'\n";
    });

  // m_dl = F.getParent().getDataLayout();

    // Instantiate the assert function once per module
    if (Assert == nullptr || Assert->getParent() != F.getParent())
      Assert = getAssertFunction(F.getParent());

    // Find all GEP instructions
    // NOTE: we need to do this first, because the iterators get invalidated
    //       when modifying underlying structures
    std::list<GetElementPtrInst *> WorkList;
    for (auto &FI : F) {    // Iterate function -> basic blocks
      for (auto &BI : FI) { // Iterate basic block -> instructions
        if (auto *GEP = dyn_cast<GetElementPtrInst>(&BI))
          WorkList.push_back(GEP);
      }
    }

    // Process any GEP instructions
    bool changed = false;
    for (auto *GEP : WorkList) {
      LLVM_DEBUG(dbgs() << "\nGEP instruction: " << *GEP << "\n");
    
      auto arrayLength = -1;
      auto desiredIndex = -1;

      /* retrieve the number of elements in the array */
      if (auto *pointerOperandType = dyn_cast<PointerType>(GEP->getPointerOperandType())){
        if (auto *arrayElementType = dyn_cast<ArrayType>(pointerOperandType->getPointerElementType())){
          arrayLength = arrayElementType->getArrayNumElements();
          LLVM_DEBUG({dbgs() << "GEP array has length: " << arrayLength << "\n"; });
        }
        // XXX check the behaviour for dynamic alocated structures, matrices and so on (later)
      }

      /* check if the index to be assigned is a constant */
      if (GEP->hasAllConstantIndices()){
        LLVM_DEBUG({dbgs() << "GEP is composed of constant indices only\n"; });
        auto *constantInt = dyn_cast<ConstantInt>(GEP->getOperand(GEP->getNumIndices()));
        desiredIndex = constantInt->getZExtValue();
        LLVM_DEBUG({dbgs() << "GEP tries to assign to index: " << desiredIndex << "\n"; });
      }

      if (arrayLength >=0 && desiredIndex >=0 ){
        // TODO - perform the analysis
      } else { 
        // TODO - unable to perform the analysis (more info needed)
      }

      LLVM_DEBUG({dbgs() << "\n"; });
    }

    return changed;
  }

private:
  /// Get a function object pointing to the Sys V '__assert' function.
  ///
  /// This function displays a failed assertion, together with the source
  /// location (file name and line number). Afterwards, it abort()s the program.
  Function *getAssertFunction(Module *Mod) {
    Type *CharPtrTy = Type::getInt8PtrTy(Mod->getContext());
    Type *IntTy = Type::getInt32Ty(Mod->getContext());
    Type *VoidTy = Type::getVoidTy(Mod->getContext());

    std::vector<Type *> assert_arg_types;
    assert_arg_types.push_back(CharPtrTy); // const char *__assertion
    assert_arg_types.push_back(CharPtrTy); // const char *__file
    assert_arg_types.push_back(IntTy);     // int __line

    FunctionType *assert_type =
        FunctionType::get(VoidTy, assert_arg_types, true);

    Function *F = Function::Create(assert_type, Function::ExternalLinkage,
                                   "__assert", Mod);
    F->addFnAttr(Attribute::NoReturn);
    F->setCallingConv(CallingConv::C);
    return F;
  }
};
}

char BoundsCheck::ID = 0;
static RegisterPass<BoundsCheck> X("bounds-check",
                                   "Bounds Checking Pass", false, false);
