/**
 * Скелет прохода для LLVM.
 */
#include <map>
#include <stack>
#include <iostream>
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

void 
printStack(std::stack<std::string> stack) 
{
    while (!stack.empty()) 
    {
        errs() << stack.top() << "\n";
        stack.pop();
    }
}

void 
dfsWalk(
    const BasicBlock& BB,
    std::map<std::string, std::string>& colour,
    std::stack<std::string>& order) 
{
    colour[BB.getName().data()] = "grey";
    const Instruction* TInst = BB.getTerminator();
    for (int i = 0, n = TInst->getNumSuccessors(); i < n; ++i) 
    {
        BasicBlock *Succ  = TInst->getSuccessor(i); 
        if (colour[Succ->getName().data()] == "grey") 
        {
            errs() << "CYCLE: " << BB.getName() << " -> " << Succ->getName() << "\n"; 
        }

        if (colour[Succ->getName().data()] == "white")
        {
            dfsWalk(*Succ, colour, order);
        }
    }

    order.push(BB.getName().data());
    colour[BB.getName().data()] = "black";
}

void
printMap(const std::map<std::string, size_t>& map)
{
    for (auto it = map.cbegin(); it != map.cend(); ++it) 
    {
        std::cout << it->first << " " << it->second << "\n"; 
    }
}

void 
VisitFunction(Function &F) 
{
    errs() << "Function name: " << F.getName() << "\n";

    std::map<std::string, size_t> instructionFrequency;
    std::map<std::string, std::string> colour;
    std::stack<std::string> order;
    int BB_num = 1;
    for (auto &BB: F) 
    {
        BB.setName(std::to_string(BB_num++));
        colour[BB.getName().data()] = "white";

        for (auto &Inst: BB )
        {
            StringRef Name = Inst.getOpcodeName();
            instructionFrequency[Name.data()] += 1;
        }
    }

    dfsWalk(F.getEntryBlock(), colour, order);
    printStack(order);
    printMap(instructionFrequency);
}

struct MyPass : PassInfoMixin<MyPass>
{
    PreservedAnalyses 
    run(Function &Function, 
	 FunctionAnalysisManager &AnalysisManager)
    {
        VisitFunction(Function);
        return (PreservedAnalyses::all());
    }

    static bool 
    isRequired(void) 
    { 
        return (true); 
    }
};
} /* namespace */

bool
CallBackForPipelineParser( 
    StringRef Name, 
    FunctionPassManager &FPM,  
    ArrayRef<PassBuilder::PipelineElement>)
{
    if (Name == "MyPass" )
    {
        FPM.addPass( MyPass());
	    return (true);
    } else
    {
        return (false);
    }
} /* CallBackForPipelineParser */

void
CallBackForPassBuilder(PassBuilder &PB)
{
    PB.registerPipelineParsingCallback( &CallBackForPipelineParser); 
} /* CallBackForPassBuider */

PassPluginLibraryInfo 
getMyPassPluginInfo(void)
{
    uint32_t     APIversion =  LLVM_PLUGIN_API_VERSION;
    const char * PluginName =  "MyPass";
    const char * PluginVersion =  LLVM_VERSION_STRING;
    
    PassPluginLibraryInfo info = 
    { 
        APIversion, 
	    PluginName, 
	    PluginVersion, 
	    CallBackForPassBuilder
    };

  return (info);
} /* getMyPassPluginInfo */

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() 
{
  return (getMyPassPluginInfo());
} /* llvmGetPassPluginInfo */