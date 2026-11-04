#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <optional>
#include <memory>
#include <vector>
#include <fstream>
#include <sstream>
#include <ostream>
#include <filesystem>

#include "generator.h"
#include "lexer.h"
#include "parser.h"
#include "parsetree.h"
#include "util.h"
#include "generators/ast_code_gen.h"
#include "generators/visitor_code_gen.h"
#include "visitors/error_visitor.h"

std::string srcFileAbsolute;
#ifdef WIN32
zblast::EErrorFormat errorFormat = zblast::EErrorFormat_MSVC;
#else
zblast::EErrorFormat errorFormat = zblast::EErrorFormat_GNU;
#endif

zblast::Error g_ExceptionErrorBuf = {
    .code = zblast::EError_Z999,
    .line = 0, .column = 0,
    .message = std::string(512, ' ')
};

/*

 start -> rule_list EOF
 rule_list -> rule rule_list
 rule_list -> \epsilon

 rule -> node opt_expansion ";"
 rule -> "abstract" node ";"
 opt_expansion -> "::=" child_list
 opt_expansion -> \epsilon
 node -> node_ref opt_inherit

 node_ref -> ID
 opt_inherit -> ":" node_ref
 opt_inherit -> \epsilon

 child_list -> child child_list
 child_list -> \epsilon

 labeled_iden -> LABEL ":" iden
 labeled_iden -> iden
 iden -> ID
 iden -> "<" ID ">"
 child -> "[" labeled_iden "]"    # Optional
 child -> labeled_iden child_qual # Literal/Identifier
 child_qual -> "*"                # List
 child_qual -> variant_qual       # Variant
 child_qual -> \epsilon
 variant_qual -> "|" labeled_iden variant_qual
 variant_qual -> \epsilon

*/


#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#define NOMINMAX
#include <windows.h>

LONG WINAPI CrashHandler(EXCEPTION_POINTERS* ExceptionInfo) {
    PEXCEPTION_RECORD record = ExceptionInfo->ExceptionRecord;
    DWORD exceptionCode = record->ExceptionCode;
    PVOID faultAddress = record->ExceptionAddress;
    char msg[512];
    int len = 0;

    switch (exceptionCode) {
    case EXCEPTION_ACCESS_VIOLATION: {
        const char* accessType = "unknown";
        PVOID targetAddress = 0;

        // Ensure the OS gave us the expected parameters
        if (record->NumberParameters >= 2) {
            ULONG_PTR flag = record->ExceptionInformation[0];
            if (flag == 0)
                accessType = "read";
            else if (flag == 1)
                accessType = "write";
            else if (flag == 8)
                accessType = "execute";

            targetAddress = (PVOID)record->ExceptionInformation[1];
        }


        len = snprintf(msg, sizeof(msg),
            "Access Violation: Attempted to %s memory at address %p. Faulting instruction at %p",
            accessType, targetAddress, faultAddress);
        break;
    }
    case EXCEPTION_STACK_OVERFLOW:
        len = snprintf(msg, sizeof(msg), "Stack Overflow. Faulting instruction at %p", faultAddress);
        break;
    case EXCEPTION_ILLEGAL_INSTRUCTION:
        len = snprintf(msg, sizeof(msg), "Illegal Instruction. Faulting instruction at %p", faultAddress);
        break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        len = snprintf(msg, sizeof(msg), "Divide by Zero. Faulting instruction at %p", faultAddress);
        break;
    default:
        len = snprintf(
            msg, sizeof(msg), "Unhandled exception: 0x%08X at instruction %p", exceptionCode, faultAddress);
        break;
    }

    if (len < 0)
        len = 0;
    if (len >= sizeof(msg))
        len = sizeof(msg) - 1;

    snprintf(g_ExceptionErrorBuf.message.data(), g_ExceptionErrorBuf.message.length(), zblast::k_ErrorDefs[zblast::EError_Z999].fmt, msg);
    g_ExceptionErrorBuf.PrettyPrint(std::cerr, srcFileAbsolute, errorFormat);

    ExitProcess(exceptionCode);

    return EXCEPTION_EXECUTE_HANDLER;
}
#define SETUP_EXCEPTION_HANDLER SetUnhandledExceptionFilter(CrashHandler)
#else
#include <signal.h>
#include <unistd.h>

void crash_handler(int sig, siginfo_t* info, void* ucontext) {
    char msg[128];
    int len = snprintf(msg, sizeof(msg), "Segmentation fault: signal %d. Faulting address: %p", sig, info->si_addr);

    snprintf(g_ExceptionErrorBuf.message.data(), g_ExceptionErrorBuf.message.length(), zblast::k_ErrorDefs[zblast::EError_Z999].fmt, msg);
    g_ExceptionErrorBuf.PrettyPrint(std::cerr, srcFileAbsolute, errorFormat);

    _exit(128 + sig);
}
#define SETUP_EXCEPTION_HANDLER                                                                                        \
    struct sigaction sa;                                                                                               \
    memset(&sa, 0, sizeof(struct sigaction));                                                                          \
    sigemptyset(&sa.sa_mask);                                                                                          \
    sa.sa_sigaction = crash_handler;                                                                                   \
    sa.sa_flags = SA_SIGINFO;                                                                                          \
    sigaction(SIGSEGV, &sa, NULL);                                                                                     \
    sigaction(SIGFPE, &sa, NULL);                                                                                      \
    sigaction(SIGILL, &sa, NULL)
#endif


static std::vector<zblast::Token> Tokenize(const char* src) {
    std::vector<zblast::Token> tokens;
    bool isEnd = false;
    const char* cursor = src;
    int l = 1, c = 1;
    while (!isEnd) {
        const char* start = cursor;
        zblast::Token tok = zblast::NextToken(cursor);
        zblast::AdvanceLocation(l, c, start, cursor);
        tok.colEnd = c;
        tok.lineEnd = l;
        tok.lineStart = l;
        tok.colStart = tok.colEnd - static_cast<int>(tok.text.length());
        if (tok.type == zblast::EToken_EOF) {
            isEnd = true;
        }
        tokens.push_back(tok);
    }
    return tokens;
}

enum EGenerationMode {
    EGeneration_Ast,
    EGeneration_Visitor,

    EGeneration_UNKNOWN,
};

std::vector<EGenerationMode> ParseGenerationModes(const char* modes_) {
    std::vector<EGenerationMode> modeList;
#ifdef _MSC_VER
    char* modes = _strdup(modes_);
#else
    char* modes = strdup(modes_);
#endif
    for (char* t = strtok(modes, ","); t; t = strtok(nullptr, ",")) {
        if (strcmp(t, "ast") == 0) {
            modeList.push_back(EGeneration_Ast);
        } else if (strcmp(t, "visitor") == 0) {
            modeList.push_back(EGeneration_Visitor);
        } else {
            modeList.push_back(EGeneration_UNKNOWN);
        }
    }
    free(modes);
    return modeList;
}

static void PrintHelp(std::ostream& out) {
    out << R"(
Usage:
  zblast --help | --version
  zblast --generate <targets> [-d DIR] <input>
  zblast --validate <input>

Arguments:
  <input>                   ZBlast (.zast) file to process

Modes:
  -g, --generate TARGETS    Generate code; TARGETS is comma-separated: ast, visitor
  -h, --help                Show this help and exit
      --validate            Check the file for errors; writes nothing
      --version             Show version and exit

Options:
      --error-format FMT    Specifies the error format: gnu, msvc (default: gnu on linux/apple, msvc on windows) 
      --list-outputs        List the files --generate would write, then exit
  -d, --out-dir DIR         Output directory (default: .)
  -v, --verbose             Increase verbosity

Examples:
  zblast --validate grammar.zast
  zblast --generate ast,visitor -d gen/zast/ grammar.zast
)";
}

struct Args {
    bool help = false, version = false, validate = false;
    bool listOutputs = false, verbose = false;
    std::optional<std::string> generate;
    std::optional<std::string> errorFormat;
    std::string outDir = ".";
    std::vector<std::string> operands;
    std::vector<std::string> usageErrors; 
};

static bool TakeValue(int argc, char** argv, int& i, std::string& out, Args& args) {
    if (i + 1 >= argc) {
        args.usageErrors.push_back(std::string("missing value for ") + argv[i]);
        return false;
    }
    out = argv[++i];
    return true;
}

static bool IsArg(const char* arg, const char* long_, const char* short_ = nullptr) {
    return (short_ ? strcmp(arg, short_) == 0 : false) || strcmp(arg, long_) == 0;
}

static Args ParseArgs(int argc, char** argv) {
    Args args;
    for (int i = 1; i < argc; ++i) {
        const char* a = argv[i];
        std::string v;
        if (strcmp(a, "--") == 0) {
            for (++i; i < argc; ++i) args.operands.push_back(argv[i]);
            break;
        }
        else if (IsArg(a, "--help", "-h"))      args.help = true;
        else if (IsArg(a, "--version"))         args.version = true;
        else if (IsArg(a, "--validate"))        args.validate = true;
        else if (IsArg(a, "--list-outputs"))    args.listOutputs = true;
        else if (IsArg(a, "--verbose", "-v"))   args.verbose = true;
        else if (IsArg(a, "--generate", "-g")) { if (TakeValue(argc, argv, i, v, args)) args.generate = v; }
        else if (IsArg(a, "--out-dir", "-d")) { if (TakeValue(argc, argv, i, v, args)) args.outDir = v; }
        else if (IsArg(a, "--error-format")) { if (TakeValue(argc, argv, i, v, args)) args.errorFormat = v; }
        else if (a[0] == '-' && a[1] != '\0') args.usageErrors.push_back(std::string("unknown option ") + a);
        else                                  args.operands.push_back(a);
    }
    return args;
}


int main(int argc, char** argv) {
    SETUP_EXCEPTION_HANDLER;

    std::string srcText; // Keep alive because of std::string_view in exceptions!


    try {
        Args args = ParseArgs(argc, argv);
        if (args.usageErrors.size() > 0) {
            for (auto& err : args.usageErrors) {
                std::cerr << err << std::endl;
            }
            return 2;
        }
        int numModes = args.help + args.version + args.generate.has_value() + args.validate;
        if (numModes == 0) {
            std::cerr << "Insufficient arguments provided.\n\n";
            PrintHelp(std::cerr);
            return 2;
        }
        else if (numModes > 1) {
            std::cerr << "Too many arguments provided. Use --help to see valid usages.\n";
            return 2;
        }

        if (args.version) {
            std::cout << ZBLAST_VERSION << std::endl;
            return EXIT_SUCCESS;
        }
        else if (args.help) {
            std::cout << "zblast - ZBlast generator, a C++ table generator for abstract syntax trees.\n";
            PrintHelp(std::cout);
            return EXIT_SUCCESS;
        }

        if (args.operands.size() != 1) {
            std::cerr << "usage error: expected 1 input file, got " << args.operands.size() << std::endl;
            return 2;
        }

        if (args.errorFormat) {
            if (*args.errorFormat == "gnu") {
                errorFormat = zblast::EErrorFormat_GNU;
            } else if (*args.errorFormat == "msvc") {
                errorFormat = zblast::EErrorFormat_MSVC;
            } else {
                std::cerr << "usage error: invalid error format provided, got " << args.errorFormat.value() << std::endl;
                return 2;
            }
        }

        const std::string& srcFile = args.operands[0];
        auto srcFilePath = std::filesystem::absolute(srcFile);
        srcFileAbsolute = srcFilePath.string();
        std::string srcFileStem = srcFilePath.stem().string();
        std::string srcFileExtension = srcFilePath.extension().string();
        std::filesystem::path absoluteDirOutput = std::filesystem::absolute(args.outDir);

        if (args.generate) {
            if (srcFileExtension != ".zast") {
                std::cerr << "usage error: unsupported file extension '" << srcFileExtension << "', expected '.zast'" << std::endl;
                return 2;
            }
        }

        std::vector<std::unique_ptr<zblast::Generator>> generators;
        bool generateAst = false;
        bool generateVisitor = false;
        bool unknownGenArg = false;
        if (args.generate) {
            for (EGenerationMode mode : ParseGenerationModes(args.generate.value().c_str())) {
                generateAst |= mode == EGeneration_Ast;
                generateVisitor |= mode == EGeneration_Visitor;
                unknownGenArg |= mode == EGeneration_UNKNOWN;
            }
            if (unknownGenArg) {
                std::cerr << "usage error: unknown --generate argument (provided " << args.generate.value() << ")" << std::endl;
                return 2;
            }
            if (!generateAst && !generateVisitor) {
                std::cerr << "usage error: no --generate arguments provided" << std::endl;
                return 2;
            }

            if (generateAst) {
                generators.emplace_back(new zblast::generators::AstCodeGen);
            }
            if (generateVisitor) {
                generators.emplace_back(new zblast::generators::VisitorCodeGen);
            }
            if (args.listOutputs) {
                for (auto& gen : generators) {
                    std::string stem = gen->GatherOutputStem(srcFileStem);
                    std::cout << (absoluteDirOutput / stem).string() << ".hpp" << "\n";
                    std::cout << (absoluteDirOutput / stem).string() << ".cpp" << "\n";
                }
                return EXIT_SUCCESS;
            }
        }

        // Step 0: reading
        std::ifstream in(srcFileAbsolute, std::ios::in);
        if (!in) {
            throw std::runtime_error(zblast::util::Format("Failed to open source file '%s'!", srcFile.c_str()));
        }
        {
            std::ostringstream ss;
            ss << in.rdbuf();
            srcText = ss.str();
        }
        // Step 1: scanning
        auto tokens = Tokenize(srcText.c_str());

        // Step 2: parsing
        zblast::Parser parser;
        std::unique_ptr<zblast::parsetree::Start> prog;
        try {
            prog = parser.Parse(tokens.data());
        } catch (const zblast::ParserException& ex) {
            if (ex.HasExpectedToken()) {
                MAKE_ERROR(Z101, ex.Line(), ex.Column(),
                    zblast::ETokenType_ToString_Literal(ex.Expected()),
                    zblast::Token_ToString(ex.Received()).c_str()
                ).PrettyPrint(std::cerr, srcFileAbsolute, errorFormat);
            } else {
                MAKE_ERROR(Z102, ex.Line(), ex.Column(),
                    zblast::Token_ToString(ex.Received()).c_str(), ex.Rule()
                ).PrettyPrint(std::cerr, srcFileAbsolute, errorFormat);
            }
            std::cerr << std::endl;
            return EXIT_FAILURE;
        }

        // Step 3: Check the AST
        bool hasError = false;
        auto errors = zblast::visitors::ErrorVisitor().CollectErrors(prog.get());
        for (auto& error : errors) {
            error.PrettyPrint(std::cerr, srcFileAbsolute, errorFormat);
            std::cerr << std::endl;
            hasError |= zblast::k_ErrorDefs[error.code].severity == zblast::ESeverity_Error;
        }

        if (hasError) {
            return EXIT_FAILURE;
        }

        if (args.validate) {
            return EXIT_SUCCESS;
        }

        // Step 5: finally GENERATE
        if (args.generate) {
            std::filesystem::create_directories(absoluteDirOutput);
            for (auto& gen : generators) {
                std::string stem = gen->GatherOutputStem(srcFileStem);
                   
                std::string outHpp = (absoluteDirOutput / stem).string() + ".hpp";
                std::string outCpp = (absoluteDirOutput / stem).string() + ".cpp";
                std::ofstream outHppFile(outHpp, std::ios::out);
                if (!outHppFile) {
                    throw std::runtime_error("Failed to open output \"" + outHpp + "\"!");
                }
                std::ofstream outCppFile(outCpp, std::ios::out);
                if (!outCppFile) {
                    throw std::runtime_error("Failed to open output \"" + outCpp + "\"!");
                }
                gen->Generate(prog.get(), outHppFile, outCppFile, srcFileStem);
            }
        }
    } catch (const std::exception& ex) {
        MAKE_ERROR(Z998, 0, 0, ex.what()).PrettyPrint(std::cerr, srcFileAbsolute, errorFormat);
        std::cerr << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
