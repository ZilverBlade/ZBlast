#pragma once

#include <string>
#include <ostream>
#include "util.h"

namespace zblast {
    enum ESeverityLevel {
        ESeverity_Message,
        ESeverity_Warning,
        ESeverity_Error,
    };

    enum EErrorFormat {
        EErrorFormat_GNU,
        EErrorFormat_MSVC,
    };

    static const char* k_ErrorSevStrings[] = {
        "message",
        "warning",
        "error",
    };

#define ERROR_CODES(X)                                                           \
\
    X(Z101, Error,   "Syntax error, expected \"%s\" but got \"%s\"")             \
    X(Z102, Error,   "Syntax error, unexpected \"%s\" (rule %s)")                \
\
    X(Z201, Error,   "Redefinition of class \"%s\", already defined at %d:%d")   \
    X(Z202, Error,   "Conflicting member name \"%s\", already defined at %d:%d") \
    X(Z203, Error,   "Undefined class \"%s\" referenced")                        \
    X(Z204, Warning, "Unused abstract class \"%s\"")                             \
    X(Z205, Warning, "Class \"%s\" has no children, but \"::=\" was present")    \
\
    X(Z998, Error,   "Fatal compiler error occurred, exception thrown: \"%s\"")  \
    X(Z999, Error,   "Fatal compiler error occurred, %s")        

#define EXPAND(Code, ...) EError_##Code,

    enum EErrorCode {
        ERROR_CODES(EXPAND)
    };

#undef EXPAND

    struct ErrorMetadata {
        ESeverityLevel severity;
        const char* code;
        const char* fmt;
    };

#define EXPAND(Code, Sev, Fmt, ...)\
    { ESeverity_##Sev, #Code, Fmt },

    static const ErrorMetadata k_ErrorDefs[] = {
        ERROR_CODES(EXPAND)
    };

#undef EXPAND

#undef ERROR_CODES

    struct Error {
        EErrorCode code;
        int line;
        int column;
        std::string message;

        void PrettyPrint(std::ostream& out, const std::string& inputFile, EErrorFormat format) const;
    };

#define MAKE_ERROR(Code, L, C, ...)              \
        (::zblast::Error{                        \
                .code = ::zblast::EError_##Code, \
                .line = (L),                     \
                .column = (C),                   \
                .message = ::zblast::util::Format(::zblast::k_ErrorDefs[::zblast::EError_##Code].fmt, __VA_ARGS__), \
        })
} // namespace zblast