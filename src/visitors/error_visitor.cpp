#include "error_visitor.h"
#include "declaration_visitor.h"
#include "parsetree.h"
#include "util.h"
#include "visitors/expansion_visitor.h"

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

            std::unordered_map<std::string, const Declaration *> foundDeclarations; // [name, decl]
            std::unordered_map<std::string, std::vector<parsetree::AstNode *>> referencedIdentifiers; // [name, referenced by]
            // Check for duplicate rules
            for (auto &decl: m_DeclVisitor.GetDeclarations()) {
                if (auto it = foundDeclarations.find(decl.className); it == foundDeclarations.end()) {
                    foundDeclarations[decl.className] = &decl;
                    if (decl.classInherit.has_value()) {
                        referencedIdentifiers[decl.classInherit.value()].push_back(decl.declaration->opt_inherit->node_ref.get());
                    }
                } else {
                    PROVIDE_ERROR(
                        Z001, decl.declaration->LineStart, decl.declaration->ColumnStart,
                        decl.className.c_str(),
                        it->second->declaration->LineStart, it->second->declaration->ColumnStart
                    );
                }
            }
            // Check for duplicate member names
            for (auto &exp: m_ExpVisitor.GetExpansions()) {
                std::unordered_map<std::string, std::vector<parsetree::AstNode *>> declaredMembers; // [name, referenced by]
                for (auto& mem : exp.members) {
                    for (auto& spec : mem.specifiers) {
                        if (spec.kind == EMember_Identifier) {
                            referencedIdentifiers[spec.nodeName].push_back(spec.idenRef);
                        }
                        declaredMembers[spec.label].push_back(spec.idenRef);
                        if (mem.kind == EExpansion_List) {
                            declaredMembers[spec.label + "s"].push_back(spec.idenRef);
                        }
                    }
                    if (mem.kind == EExpansion_Variant) {
                        std::string concat;
                        for (auto& spec : mem.specifiers) {
                            concat += spec.label;
                        }
                        // Just reference the first node
                        declaredMembers[concat].push_back(mem.specifiers[0].idenRef);
                    }
                }
                for (auto& [label, memDecl] : declaredMembers) {
                    if (memDecl.size() > 1) {
                        for (int i = 1; i < memDecl.size(); ++i) {
                            PROVIDE_ERROR(
                                Z002, memDecl[i]->LineStart, memDecl[i]->ColumnStart,
                                label.c_str(),
                                memDecl[0]->LineStart, memDecl[0]->ColumnStart
                            );
                        }
                    }
                }
            }

            // Check if inherited classes exist
            for (auto& [declName, refs]: referencedIdentifiers) {
                if (!foundDeclarations.contains(declName)) {
                    for (auto& ref : refs) {
                        PROVIDE_ERROR(
                            Z003, ref->LineStart, ref->ColumnStart,
                            declName.c_str()
                        );
                    }
                }
            }

            // Check unused classes, that are abstract
            for (auto& [declName, decl]: foundDeclarations) {
                if (decl->isAbstract && referencedIdentifiers[declName].empty()) {
                    PROVIDE_ERROR(
                        Z004, decl->rule->LineStart, decl->rule->ColumnStart,
                        declName.c_str()
                    );
                }
            }

            // Check if children were found in the opt_expansion
            for (auto& [declName, decl]: foundDeclarations) {
                if (!decl->isAbstract && !decl->rule->opt_expansion->IsEpsilon
                    && decl->rule->opt_expansion->child_list->IsEpsilon) {
                    PROVIDE_ERROR(
                        Z005, decl->rule->LineStart, decl->rule->ColumnStart,
                        declName.c_str()
                    );
                }
            }


            return VisitChildren(node, data);
        }
    } // namespace visitors
} // namespace zblast
