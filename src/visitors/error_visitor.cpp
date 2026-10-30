#include "error_visitor.h"
#include "declaration_visitor.h"
#include "parsetree.h"
#include "util.h"

#include <unordered_map>

namespace zblast {
    namespace visitors {
#define PROVIDE_ERROR(Code, L, C, ...)                                                \
        do {                                                                          \
            m_Errors.push_back({                                                      \
                .code = EError_##Code,                                                \
                .line = (L),                                                          \
                .column = (C),                                                        \
                .message = util::Format(k_ErrorDefs[EError_##Code].fmt, __VA_ARGS__), \
            });                                                                       \
        } while (false)

        std::string Error::PrettyPrint() const {
            return util::Format(
                "%s %s @ %d:%d | %s.",
                k_ErrorSevAlignedStrings[k_ErrorDefs[code].severity], k_ErrorDefs[code].code,
                line, column, message.c_str());
        }

        void *ErrorVisitor::Visit(parsetree::Start *node, void *data) {
            node->Accept(&m_DeclVisitor, nullptr);
            node->Accept(&m_ExpVisitor, nullptr);

            std::unordered_map<std::string, const Declaration *> foundDeclarations;
            for (auto &decl: m_DeclVisitor.GetDeclarations()) {
                if (auto it = foundDeclarations.find(decl.className); it == foundDeclarations.end()) {
                    foundDeclarations[decl.className] = &decl;
                } else {
                    PROVIDE_ERROR(Z001, decl.declaration->LineStart, decl.declaration->ColumnStart,
                                  decl.className.c_str(),
                                  it->second->declaration->LineStart, it->second->declaration->ColumnStart
                    );
                }
            }
            return VisitChildren(node, data);
        }
    } // namespace visitors
} // namespace zblast
