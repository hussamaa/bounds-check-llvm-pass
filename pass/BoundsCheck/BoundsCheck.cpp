#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Pass.h>
#include <llvm/Support/Debug.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

#include <iostream>
#include <list>
#include <vector>
#include <string>
#include <cstdarg>
#include <cstring>

#define DEBUG_TYPE "mine"
#define INITIAL_VALUE 0xCAFECAFE
#define IS_DATA_CORRECTLY_EXTRACTED(arrayLength, desiredIndex) ((arrayLength) != INITIAL_VALUE && (desiredIndex) != INITIAL_VALUE)
#define HAS_COMPILATION_TIME_VIOLATION(arrayLength, desiredIndex) (IS_DATA_CORRECTLY_EXTRACTED((arrayLength),(arrayLength)) && ((desiredIndex) >= (arrayLength)))

using namespace llvm;

bool forceRuntimeChecks; 
static cl::opt<bool, true>
paramRunTimeChecks("force-runtime-checks", cl::desc("Force runtime checks and ignore at compilation time"), cl::Hidden, cl::location(forceRuntimeChecks));

bool showByteCode; 
static cl::opt<bool, true> 
paramShowByteCode("show-byte-code", cl::desc("Show LLVM byte code for every function"), cl::Hidden, cl::location(showByteCode));

namespace {
struct BoundsCheck : public FunctionPass {
private:
  Function *assertFunction = nullptr;

public:
  static char ID;
  BoundsCheck() : FunctionPass(ID) { }

  /// Entry point of the pass; this function performs the actual analysis or
  /// transformation, and is called for each function in the module.
  ///
  /// The returned boolean should be `true` if the function was modified,
  /// `false` if it wasn't.
  bool runOnFunction(Function &F) override {
    IRBuilder<> builder(F.getContext());

    LLVM_DEBUG({
      dbgs() << "BoundsCheck: processing function '";
      dbgs().write_escaped(F.getName()) << "'\n";
    });

    // Instantiate the assert function once per module
    if (assertFunction == nullptr || assertFunction->getParent() != F.getParent())
      assertFunction = getAssertFunction(F.getParent());

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

    LLVM_DEBUG(dbgs() << "Forcing runtime checks (and ignoring at compilation time): " << forceRuntimeChecks << "\n");

    // Process any GEP instructions
    bool changed = false;
    for (auto* GEP : WorkList) {
      LLVM_DEBUG(dbgs() << "\nGEP instruction: " << *GEP << "\n");
      auto arrayLength = INITIAL_VALUE;
      auto desiredIndex = INITIAL_VALUE;

      // Check if the index to be assigned is a constant and retrieve the index 
      if (auto* constantInt = dyn_cast<ConstantInt>(GEP->getOperand(GEP->getNumIndices()))) {
        desiredIndex = constantInt->getZExtValue();
        LLVM_DEBUG({dbgs() << "GEP tries to assign to index: " << desiredIndex << "\n"; });
      } else {
        LLVM_DEBUG({dbgs() << "GEP does not contain the index to be assigned, including runtime checks...\n"; });
         forceRuntimeChecks = true;
      }

      // Retrieve the number of elements in the array
      if (auto* pointerOperandType = dyn_cast<PointerType>(GEP->getPointerOperandType())){
        if (auto* arrayElementType = dyn_cast<ArrayType>(pointerOperandType->getPointerElementType())) {
          arrayLength = arrayElementType->getArrayNumElements();
          LLVM_DEBUG({dbgs() << "GEP array has length: " << arrayLength << "\n"; });
        } else {
          LLVM_DEBUG({dbgs() << "GEP does not contain the array length, including runtime checks...\n"; });
          forceRuntimeChecks = true;
        }
      } 

      // Perform the check (runtime - with code instrumentation or at compilation time)
      if (forceRuntimeChecks) {     
        // FIXME - limitation for malloc instructions. To be done.
        if (!IS_DATA_CORRECTLY_EXTRACTED(arrayLength, desiredIndex)){
          LLVM_DEBUG({dbgs() << "I'm sorry. I'm still not able to instrument dynamic allocated pointers / indexes. Trying next instruction...\n"; });
          continue;
        }

        // Based on llvm::SplitBlockAndInsertIfThenElse code.  
        BasicBlock* beforeGEP = GEP->getParent();
        // Creating cont block
        BasicBlock* afterGEP = beforeGEP->splitBasicBlock(GEP->getIterator(), "cont");

        // Retrieving the index and array length dynamically - TBD 
        ConstantInt* dynamicDesiredIndex  = ConstantInt::get(F.getContext(), llvm::APInt(/*nbits*/32, desiredIndex, /*bool*/true));
        ConstantInt* dynamicDesiredArrayLength  = ConstantInt::get(F.getContext(), llvm::APInt(/*nbits*/32, arrayLength, /*bool*/true));

        // Create trap block
        BasicBlock* trapBlock = createTrapBlock(builder, GEP, F, "fakefile.c", 33); //XXX retrive correct file name and line number

        // Inserting branching with comparison
        builder.SetInsertPoint(beforeGEP);
        Value* xGreaterEqualThan = builder.CreateICmpUGE(dynamicDesiredIndex, dynamicDesiredArrayLength, "bound-check");
        BranchInst* beforeGEPNewTerminator = BranchInst::Create(trapBlock, afterGEP, xGreaterEqualThan);
        ReplaceInstWithInst(beforeGEP->getTerminator(), beforeGEPNewTerminator);

        changed = true;
      } else if (HAS_COMPILATION_TIME_VIOLATION(arrayLength, desiredIndex)) {
        report_fatal_error(stringFormat("Wrong assignment to index %d (zero-based) while array has length %d! Aborting...", desiredIndex, arrayLength), false);
      } else if (IS_DATA_CORRECTLY_EXTRACTED(arrayLength, desiredIndex)) {
        LLVM_DEBUG({dbgs() << "GEP instruction uses the correct bounds [DONE]\n"; });
      } else {
        LLVM_DEBUG({dbgs() << "BoundsCheck has NOT been able to analyse this instruction [!!!!]\n"; });
      }   

      LLVM_DEBUG({dbgs() << "\n"; });
    }

    if (showByteCode) {
      F.dump();
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

  /// Function that creates a trap block for a given GEP instruction.
  /// 
  /// A trap block contains a call to ABI assert function which displays
  /// the file name and line number in which the violation occurred. 
  BasicBlock* createTrapBlock(IRBuilder<> builder, GetElementPtrInst* GEP, Function &F, std::string fileName, uint32_t lineNumber) {
    LLVM_DEBUG({dbgs() << "Creating trap block for instruction in file: " << fileName << " line: " << lineNumber << "\n"; });
    BasicBlock* trapBlock = BasicBlock::Create(GEP->getContext(), "trap", &F);
    builder.SetInsertPoint(trapBlock);
    Value* assertionMessage = builder.CreateGlobalStringPtr("a violation has been found");
    Value* fileNameValue = builder.CreateGlobalStringPtr(fileName);                             
    ConstantInt* lineNumberConst  = ConstantInt::get(F.getContext(), llvm::APInt(32, lineNumber, true));
    Value* assertFunctionArguments[3];
    assertFunctionArguments[0] = assertionMessage; /* assertion message */
    assertFunctionArguments[1] = fileNameValue;    /* file name         */
    assertFunctionArguments[2] = lineNumberConst;  /* line number       */
    builder.CreateCall(assertFunction, assertFunctionArguments);
    builder.CreateUnreachable();
    return trapBlock;
  }

  /// Function responsible create a string using a pattern format 
  /// specified together with a number of arguments. 
  ///
  /// source: http://www.martinbroadhurst.com/string-formatting-in-c.html
  std::string stringFormat(const std::string & format, ...){
    va_list args;
    va_start (args, format);
    size_t len = std::vsnprintf(NULL, 0, format.c_str(), args);
    va_end (args);
    std::vector<char> vec(len + 1);
    va_start (args, format);
    std::vsnprintf(&vec[0], len + 1, format.c_str(), args);
    va_end (args);
    return &vec[0];
  }
};
}

char BoundsCheck::ID = 0;
static RegisterPass<BoundsCheck> X("bounds-check",
                                   "BoundsCheck LLVM Pass", false, false);
