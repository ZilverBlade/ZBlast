// example_visitor.cpp
// Example visitors for the ZBlast-generated AST (test.hpp / test_visitor.hpp).
//
// Build:  g++ -std=c++17 example_visitor.cpp test.cpp test_visitor.cpp -o example

// Include std headers first so the `#define private public` demo hack below
// can't leak into them.
#include <cstdio>
#include <string>
#include <vector>
#include <optional>
#include <variant>

// DEMO ONLY: the generated classes have no constructors/setters for their
// members, so this hack lets main() build a tree by hand. Your real parser
// would need friend access or generator support instead.
#define private public
#define protected public
#include "test_ast.hpp"
#undef private
#undef protected
#include "test_visitor.hpp"

using namespace ast;

// ---------------------------------------------------------------------------
// 1) Pretty printer: overrides a few visit() overloads, uses visit_children()
//    for the default depth-first traversal.
// ---------------------------------------------------------------------------
class PrintVisitor : public AstVisitor {
public:
    // IMPORTANT: overriding some overloads of `visit` hides the others
    // (C++ name hiding). This brings the base-class overloads back.
    using AstVisitor::visit;

    void* visit(FuncDecl* node, void* data) override {
        std::string sig = "FuncDecl " + node->get_FuncName()->get_ID() + "(";
        for (uint32_t i = 0; i < node->num_Args(); ++i) {
            if (i) sig += ", ";
            sig += node->get_Arg(i)->get_ID();
        }
        sig += ")";
        if (node->has_ReturnType()) sig += " -> returns a type";
        line(sig);
        return nested(node, data);
    }

    void* visit(IdDecl* node, void* data) override {
        line("IdDecl '" + node->get_ID() + "'");
        return data;
    }

    void* visit(VarStmt* node, void* data) override {
        line("VarStmt");
        return nested(node, data);
    }

    void* visit(BlockStmt* node, void* data) override {
        line("BlockStmt (" + std::to_string(node->num_Stmts()) + " stmts)");
        return nested(node, data);
    }

    void* visit(IfStmt* node, void* data) override {
        line(node->has_Else() ? "IfStmt (with else)" : "IfStmt");
        return nested(node, data);
    }

    void* visit(BinaryExpr* node, void* data) override {
        line("BinaryExpr");
        return nested(node, data);
    }

    void* visit(UnaryExpr* node, void* data) override {
        line("UnaryExpr");
        return nested(node, data);
    }

    void* visit(IntType* node, void* data) override {
        line("IntType");
        return data;
    }

    // Generic placeholder nodes (plain Expr / Stmt / Type) fall back to the
    // base implementation, which just recurses into children.

private:
    int m_Depth = 0;

    void line(const std::string& text) {
        std::printf("%*s%s\n", m_Depth * 2, "", text.c_str());
    }

    // Indent, recurse into children via the generated helper, un-indent.
    void* nested(AstNode* node, void* data) {
        ++m_Depth;
        visit_children(node, data);
        --m_Depth;
        return data;
    }
};

// ---------------------------------------------------------------------------
// 2) Counter: shows how to use the `void* data` parameter to pass state in,
//    instead of storing it in the visitor.
// ---------------------------------------------------------------------------
struct Counts {
    int funcs = 0, vars = 0, ifs = 0, exprs = 0, total = 0;
};

class CountVisitor : public AstVisitor {
public:
    using AstVisitor::visit;

    void* visit(FuncDecl* n, void* d) override { bump(d).funcs++; return visit_children(n, d); }
    void* visit(VarStmt* n, void* d)  override { bump(d).vars++;  return visit_children(n, d); }
    void* visit(IfStmt* n, void* d)   override { bump(d).ifs++;   return visit_children(n, d); }
    void* visit(BinaryExpr* n, void* d) override { bump(d).exprs++; return visit_children(n, d); }
    void* visit(UnaryExpr* n, void* d)  override { bump(d).exprs++; return visit_children(n, d); }

private:
    static Counts& bump(void* d) {
        Counts& c = *static_cast<Counts*>(d);
        c.total++;
        return c;
    }
};

// ---------------------------------------------------------------------------
// 3) Generic traversal WITHOUT a visitor, using children() / begin() / end()
// ---------------------------------------------------------------------------
static void dump_shape(AstNode* node, int depth = 0) {
    std::printf("%*snode with %u children\n", depth * 2, "", node->num_children());
    for (AstNode* child : node->children()) {
        dump_shape(child, depth + 1);
    }
}

// ---------------------------------------------------------------------------
// Build a tiny tree by hand and run everything:
//
//   func foo(x) -> int {
//       var y;
//       if (<expr> <op> <unary expr>) { var z; } else { }
//   }
// ---------------------------------------------------------------------------
static IdDecl* make_id(const char* name) {
    auto* id = new IdDecl();
    id->m_Member_ID = name;
    return id;
}

int main() {
    auto* varY = new VarStmt();
    varY->m_Member_Variable = make_id("y");

    auto* varZ = new VarStmt();
    varZ->m_Member_Variable = make_id("z");

    auto* thenBlock = new BlockStmt();
    thenBlock->m_Member_Stmt.push_back(varZ);

    auto* elseBlock = new BlockStmt();  // empty

    auto* unary = new UnaryExpr();
    unary->m_Member_Expr = new Expr();

    auto* cond = new BinaryExpr();
    cond->m_Member_Lhs = new Expr();
    cond->m_Member_Rhs = unary;

    auto* ifStmt = new IfStmt();
    ifStmt->m_Member_Cond = cond;
    ifStmt->m_Member_Then = thenBlock;
    ifStmt->m_Member_Else = elseBlock;

    auto* func = new FuncDecl();
    func->m_Member_FuncName = make_id("foo");
    func->m_Member_ReturnType = new IntType();
    func->m_Member_Arg.push_back(make_id("x"));
    func->m_Member_Line.push_back(varY);
    func->m_Member_Line.push_back(ifStmt);

    std::puts("=== PrintVisitor ===");
    PrintVisitor printer;
    func->accept(&printer, nullptr);

    std::puts("\n=== CountVisitor ===");
    Counts counts;
    CountVisitor counter;
    func->accept(&counter, &counts);
    std::printf("funcs=%d vars=%d ifs=%d exprs=%d visited=%d\n",
                counts.funcs, counts.vars, counts.ifs, counts.exprs, counts.total);

    std::puts("\n=== children() range-for ===");
    dump_shape(func);

    // (Nodes are leaked on purpose: the generated classes don't own or free children.)
    return 0;
}