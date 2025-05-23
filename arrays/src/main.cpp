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

    // Allocate fixed array [5 x i32]
    ArrayType *ArrayTy = ArrayType::get(Type::getInt32Ty(Context), 5);
    AllocaInst *ArrayAlloca = Builder.CreateAlloca(ArrayTy, nullptr, "array");

    // Store values into array
    for (unsigned i = 0; i < 5; ++i) {
        // Get pointer to array[i] using GEP
        Value *IndexList[] = {
            ConstantInt::get(Type::getInt32Ty(Context), 0),  // First index for the array
            ConstantInt::get(Type::getInt32Ty(Context), i)   // Element index
        };
        Value *ElemPtr = Builder.CreateGEP(ArrayTy, ArrayAlloca, IndexList, "elem_ptr");

        // Store integer i * 10
        Builder.CreateStore(ConstantInt::get(Type::getInt32Ty(Context), i * 10), ElemPtr);
    }

    // Format string for printf
    Constant *FormatStr = Builder.CreateGlobalStringPtr("Value at index %d: %d\n");

    // Print each element using printf
    for (unsigned i = 0; i < 5; ++i) {
        Value *IndexList[] = {
            ConstantInt::get(Type::getInt32Ty(Context), 0),
            ConstantInt::get(Type::getInt32Ty(Context), i)
        };
        Value *ElemPtr = Builder.CreateGEP(ArrayTy, ArrayAlloca, IndexList, "elem_ptr");
        Value *LoadedVal = Builder.CreateLoad(Type::getInt32Ty(Context), ElemPtr, "load_val");

        Builder.CreateCall(PrintfFunc, {
            FormatStr,
            ConstantInt::get(Type::getInt32Ty(Context), i),
            LoadedVal
        });
    }

    // return 0;
    Builder.CreateRet(ConstantInt::get(Type::getInt32Ty(Context), 0));

    // Verify & output the module
    verifyFunction(*MainFunc);
    TheModule->print(outs(), nullptr);

    return 0;
}
