#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/DerivedTypes.h"

using namespace llvm;

int main() {
    LLVMContext Context;
    IRBuilder<> Builder(Context);
    auto TheModule = std::make_unique<Module>("StringCharAccess", Context);

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

    // Create string: "Hello, world!"
    Constant *HelloStr = Builder.CreateGlobalStringPtr("Hello, world!", "mystr");

    // Index to access
    int index = 3; // 'l' in "Hello"

    // GEP to get the character at index 3
    Value *CharPtr = Builder.CreateGEP(Type::getInt8Ty(Context), HelloStr, ConstantInt::get(Type::getInt32Ty(Context), index), "char_ptr");

    // Load the character
    Value *CharVal = Builder.CreateLoad(Type::getInt8Ty(Context), CharPtr, "char_val");

    // Format string for printf
    Constant *FormatStr = Builder.CreateGlobalStringPtr("Character at index %d: %c\n");

    // Call printf
    Builder.CreateCall(PrintfFunc, {
        FormatStr,
        ConstantInt::get(Type::getInt32Ty(Context), index),
        CharVal
    });

    // Return 0
    Builder.CreateRet(ConstantInt::get(Type::getInt32Ty(Context), 0));

    // Verify and print
    verifyFunction(*MainFunc);
    TheModule->print(outs(), nullptr);

    return 0;
}
