#include "visitor_code_gen.h"
#include "parsetree.h"
#include "visitors/declaration_visitor.h"

namespace zblast {
    namespace generators {
        void VisitorCodeGen::Generate(parsetree::Start* program, std::ostream& hpp, std::ostream& cpp) {
            auto declarations = visitors::DeclarationVisitor().CollectDeclarations(program);
            
            hpp << "class AstNode;\n";
            for (auto& decl : declarations) {
                hpp << "class " << decl.className << ";\n";
                if (!decl.isAbstract) {
                    cpp << R"([[maybe_unused]] void* )" << decl.className <<  R"(::accept(AstVisitor* visitor, void* data) {
    return visitor->visit(this, data);
}
)";
                }
            }

            hpp << R"(
class AstVisitor {
public:
)";
            for (auto& decl : declarations) {
                if (!decl.isAbstract) {
                    hpp << R"(
    [[maybe_unused]] virtual void* visit(::ast::)" << decl.className << R"(* node, void* data);
)";
                    cpp << "void* AstVisitor::visit(::ast::" << decl.className << R"(* node, void* data) {
    return this->visit_children(static_cast<::ast::AstNode*>(node), data);
}
)";
                }
            }
            hpp << R"(
protected:
    [[maybe_unused]] void* visit_children(::ast::AstNode* node, void* data);
};
)";

            cpp << R"(
void* AstVisitor::visit_children(::ast::AstNode* node, void* data) {
    uint32_t count = node->num_children();
    for (uint32_t i = 0; i < count; ++i) {
        ::ast::AstNode* child = node->child(i);        
        if (child != nullptr) {
            child->accept(this, data);
        }
    }
    return data;
}
)";
        }

        std::string VisitorCodeGen::AdditionalIncludes(const std::string &fileName) {
            return "#include \"" + fileName.substr(0, fileName.length() - sizeof("_visitor") + 1) + "_ast.hpp\"\n";
        }
    } // namespace generators
} // namespace zblast