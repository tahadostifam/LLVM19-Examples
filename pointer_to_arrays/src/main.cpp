#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/IR/InlineAsm.h"

using namespace llvm;

int main() {
    LLVMContext Context;
    IRBuilder<> Builder(Context);
    auto TheModule = std::make_unique<Module>("ArrayPrintModule", Context);

    // Declare printf
    FunctionType *PrintfType = FunctionType::get(
        IntegerType::getInt32Ty(Context),
        PointerType::getUnqual(Type::getInt8Ty(Context)),
        true
    );
    FunctionCallee PrintfFunc = TheModule->getOrInsertFunction("printf", PrintfType);

    // Create main function
    FunctionType *MainType = FunctionType::get(Type::getInt32Ty(Context), false);
    Function *MainFunc = Function::Create(MainType, Function::ExternalLinkage, "main", TheModule.get());
    BasicBlock *Entry = BasicBlock::Create(Context, "entry", MainFunc);
    Builder.SetInsertPoint(Entry);

    // Define array type: [3 x i32]
    ArrayType *ArrayTy = ArrayType::get(Type::getInt32Ty(Context), 3);

    // Step 1: Allocate the actual array
    AllocaInst *ArrayInst = Builder.CreateAlloca(ArrayTy, nullptr, "actual_array");

    // Step 2: Allocate pointer to array type: [3 x i32]*
    AllocaInst *ArrayPtr = Builder.CreateAlloca(PointerType::getUnqual(ArrayTy), nullptr, "array_ptr");

    // Step 3: Store array address into the pointer
    Builder.CreateStore(ArrayInst, ArrayPtr);

    // Store values into the array via pointer
    for (unsigned i = 0; i < 3; ++i) {
        // Load array pointer
        Value *LoadedArray = Builder.CreateLoad(PointerType::getUnqual(ArrayTy), ArrayPtr, "loaded_array");

        // GEP to get element address
        Value *IndexList[] = {
            ConstantInt::get(Type::getInt32Ty(Context), 0),
            ConstantInt::get(Type::getInt32Ty(Context), i)
        };
        Value *ElemPtr = Builder.CreateGEP(ArrayTy, LoadedArray, IndexList, "elem_ptr");

        // Store value
        Builder.CreateStore(ConstantInt::get(Type::getInt32Ty(Context), i * 10), ElemPtr);
    }

    // Create format string
    Constant *FormatStr = Builder.CreateGlobalStringPtr("Value at index %d: %d\n");

    // Load and print values via pointer
    for (unsigned i = 0; i < 3; ++i) {
        // Load the array pointer again
        Value *LoadedArray = Builder.CreateLoad(PointerType::getUnqual(ArrayTy), ArrayPtr, "loaded_array");

        // GEP to element
        Value *IndexList[] = {
            ConstantInt::get(Type::getInt32Ty(Context), 0),
            ConstantInt::get(Type::getInt32Ty(Context), i)
        };
        Value *ElemPtr = Builder.CreateGEP(ArrayTy, LoadedArray, IndexList, "elem_ptr");

        // Load value
        Value *LoadedVal = Builder.CreateLoad(Type::getInt32Ty(Context), ElemPtr, "load_val");

        // Printf call
        Builder.CreateCall(PrintfFunc, {
            FormatStr,
            ConstantInt::get(Type::getInt32Ty(Context), i),
            LoadedVal
        });
    }

    // Return 0
    Builder.CreateRet(ConstantInt::get(Type::getInt32Ty(Context), 0));

    // Verify and output the module
    verifyFunction(*MainFunc);
    TheModule->print(outs(), nullptr);

    return 0;
}
