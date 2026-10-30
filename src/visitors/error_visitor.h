#pragma once
#include "visitor.h"
#include <string>
#include <vector>

namespace zblast {
    namespace visitors {
        enum ESeverityLevel {
            ESeverity_Message,
            ESeverity_Warning,
            ESeverity_Error,
        };

        static const char* k_ErrorSevAlignedStrings[] = {
            "[ MESSAGE ]",
            "[ WARNING ]",
            "[  ERROR  ]",
        };

#define ERROR_CODES(X)                                                               \
        X(Z001, Error,   "Redefinition of class \"%s\", already defined at %d:%d")   \
        X(Z002, Error,   "Conflicting member name \"%s\", already defined at %d:%d") \
        X(Z003, Error,   "Undefined class \"%s\" referenced")                        \
        X(Z004, Warning, "Unused abstract class \"%s\"")                             \
        X(Z005, Warning, "Class \"%s\" has no children, but \"::=\" was present")
        
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

            std::string PrettyPrint() const;
        };

        class ErrorVisitor : public Visitor {
        public:
            std::vector<Error> CollectErrors(parsetree::Start* program);

        protected:
            void* Visit(parsetree::Start* node, void* data) override;

        private:

            std::vector<Error> m_Errors;
        };
    } // namespace visitors
} // namespace zblast