#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"

using namespace llvm;

int main() {
    LLVMContext Context;
    IRBuilder<> Builder(Context);
    auto TheModule = std::make_unique<Module>("ArrayBoundsDynamic", Context);

    // Declare printf
    FunctionType *PrintfType = FunctionType::get(
        IntegerType::getInt32Ty(Context),
        PointerType::getUnqual(Type::getInt8Ty(Context)),
        true
    );
    FunctionCallee PrintfFunc = TheModule->getOrInsertFunction("printf", PrintfType);

    // Define array type
    ArrayType *ArrayTy = ArrayType::get(Type::getInt32Ty(Context), 3);

    // Global constant array: [10, 20, 30]
    Constant *ArrayVals[] = {
        ConstantInt::get(Type::getInt32Ty(Context), 10),
        ConstantInt::get(Type::getInt32Ty(Context), 20),
        ConstantInt::get(Type::getInt32Ty(Context), 30),
    };
    Constant *GlobalArrayInit = ConstantArray::get(ArrayTy, ArrayVals);
    GlobalVariable *GlobalArray = new GlobalVariable(
        *TheModule, ArrayTy, true, GlobalValue::PrivateLinkage,
        GlobalArrayInit, "myarray"
    );

    // Define helper function: void get_at_index(i32 index)
    FunctionType *GetterType = FunctionType::get(Type::getVoidTy(Context),
                                                 {Type::getInt32Ty(Context)}, false);
    Function *GetterFunc = Function::Create(GetterType, Function::ExternalLinkage,
                                            "get_at_index", TheModule.get());
    BasicBlock *EntryBB = BasicBlock::Create(Context, "entry", GetterFunc);
    BasicBlock *ThenBB = BasicBlock::Create(Context, "inbounds", GetterFunc);
    BasicBlock *ElseBB = BasicBlock::Create(Context, "outofbounds", GetterFunc);
    BasicBlock *MergeBB = BasicBlock::Create(Context, "merge", GetterFunc);

    Builder.SetInsertPoint(EntryBB);

    // Get function argument (index)
    Argument *IndexArg = &*GetterFunc->arg_begin();
    IndexArg->setName("index");

    // Check if index < 3
    Value *IsValid = Builder.CreateICmpULT(IndexArg,
        ConstantInt::get(Type::getInt32Ty(Context), 3), "in_bounds");

    Builder.CreateCondBr(IsValid, ThenBB, ElseBB);

    // THEN BLOCK: index in bounds
    Builder.SetInsertPoint(ThenBB);
    Value *ElemPtr = Builder.CreateGEP(ArrayTy, GlobalArray,
        {ConstantInt::get(Type::getInt32Ty(Context), 0), IndexArg}, "elem_ptr");
    Value *LoadedVal = Builder.CreateLoad(Type::getInt32Ty(Context), ElemPtr, "val");

    Constant *FormatStr1 = Builder.CreateGlobalStringPtr("Index %d = %d\n");
    Builder.CreateCall(PrintfFunc, {FormatStr1, IndexArg, LoadedVal});
    Builder.CreateBr(MergeBB);

    // ELSE BLOCK: out of bounds
    Builder.SetInsertPoint(ElseBB);
    Constant *FormatStr2 = Builder.CreateGlobalStringPtr("Index %d is out of bounds!\n");
    Builder.CreateCall(PrintfFunc, {FormatStr2, IndexArg});
    Builder.CreateBr(MergeBB);

    // MERGE
    Builder.SetInsertPoint(MergeBB);
    Builder.CreateRetVoid();

    // MAIN FUNCTION
    FunctionType *MainType = FunctionType::get(Type::getInt32Ty(Context), false);
    Function *MainFunc = Function::Create(MainType, Function::ExternalLinkage, "main", TheModule.get());
    BasicBlock *MainBB = BasicBlock::Create(Context, "entry", MainFunc);
    Builder.SetInsertPoint(MainBB);

    // Call get_at_index(2) — OK
    Builder.CreateCall(GetterFunc, {ConstantInt::get(Type::getInt32Ty(Context), 2)});

    // Call get_at_index(5) — Out of bounds
    Builder.CreateCall(GetterFunc, {ConstantInt::get(Type::getInt32Ty(Context), 5)});

    Builder.CreateRet(ConstantInt::get(Type::getInt32Ty(Context), 0));

    // Verify and output
    verifyFunction(*MainFunc);
    verifyFunction(*GetterFunc);
    TheModule->print(outs(), nullptr);

    return 0;
}
