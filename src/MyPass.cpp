/**
 * Скелет прохода для LLVM.
 */
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

/* Так как мы пишем библиотеку, то можно выбрать анонимную область видимости */
namespace {
 
/**
 * Определяет, что будем делать при посещении функции. 
 */
void 
VisitFunction( Function &F) 
{
    /* LLVM использует кастомные потоки вывода. 
     * llvm::errs() (поток ошибок), 
     * llvm::outs() (аналогичен стандартному stdout), 
     * llvm::nulls() (отбрасывает весь вывод) */
    errs() << "Function name: " << F.getName() << "\n";
    errs() << "    number of arguments: " << F.arg_size() << "\n";
} /* VisitFunction */

/**
 * Структура, необходимая для регистрации своего прохода в менеджере проходов.
 */
struct MyPass : PassInfoMixin<MyPass> 
{
    /* PreservedAnalyses - множество анализов, которые сохраняются после данного прохода,
     * чтобы не запускать их заново.
     *
     * run() непосредственно нужен для запуска прохода.
     *
     * Так как мы просто хотим вывести: "имя функции - количество аргументов", то мы 
     * возвращаем all(), что говорит о том, что ни один анализ не будет испорчен */
    PreservedAnalyses 
    run( Function &Function, 
	 FunctionAnalysisManager &AnalysisManager) 
    {
        VisitFunction( Function);
        return (PreservedAnalyses::all());
    }

    /* По умолчанию данный проход будет пропущен, если функция помечена атрибутом 
     * optnone (не производить оптимизаций над ней). Поэтому необходимо вернуть true, 
     * чтобы мы могли обходить и их. 
     * (в режиме сборки -O0 все функции помечены как неоптимизируемые) */
    static bool 
    isRequired( void) 
    { 
        return (true); 
    }
};
} /* namespace */

/**
 * Наш проход будет реализован в виде отдельно подключаемого плагина (расширения языка).
 * Это удобный способ расширить возможности компилятора. Например, сделать поддержку
 * своей прагмы, своей оптимизации, выдачи своего предупреждения.
 *
 * PassPluginLibraryInfo - структура, задающая базовые параметры для нашего плагина.
 * Её надо составить из:
 * - Версии API (для отслеживания совместимости ABI можно 
 *   использовать LLVM_PLUGIN_API_VERSION)
 * - Имя плагина
 * - Версии плагина
 * - Callback для регистрации плагина через PassBuilder
 */
/**
 * По-модному это делают с помощью лямбда-функции, но можно и по-старинке.
 */
bool
CallBackForPipelineParser( 
    StringRef Name, 
    FunctionPassManager &FPM,  
    ArrayRef<PassBuilder::PipelineElement>)
{
    if ( Name == "MyPass" )
    {
        FPM.addPass( MyPass());
	return (true);
    } else
    {
        return (false);
    }
} /* CallBackForPipelineParser */

void
CallBackForPassBuilder( PassBuilder &PB)
{
    PB.registerPipelineParsingCallback( &CallBackForPipelineParser); 
} /* CallBackForPassBuider */

PassPluginLibraryInfo 
getMyPassPluginInfo( void)
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

/**
 * Интерфейс, который гарантирует, что "opt" распознаст наш проход. 
 * "-passes=MyPass"
 */
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() 
{
  return (getMyPassPluginInfo());
} /* llvmGetPassPluginInfo */
