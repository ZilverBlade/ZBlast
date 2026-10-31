// example_visitor.cpp
// Claude Sonnet 5 AI Generated @ 31-10-2026.
// Example visitors for the ZBlast-generated AST (test_ast.hpp + its visitor header).
//
// Build (adjust file names to whatever zblast generated):
//   g++ -std=c++17 example_visitor.cpp test_ast.cpp test_ast_visitor.cpp -o example
//

#include <cassert>
#include <cstdio>
#include <string>

#include "test_ast.hpp"
#include "test_visitor.hpp"

using namespace ast;

// ---------------------------------------------------------------------------
// 1) Pretty printer
//    Only concrete node types are overridden. Abstract ones (Stmt, Expr, Type)
//    never reach a visitor directly because they can't be instantiated.
// ---------------------------------------------------------------------------
class PrintVisitor : public AstVisitor {
public:
    // Overriding some `visit` overloads hides the rest (name hiding),
    // so pull the base overloads back in.
    using AstVisitor::visit;

    void* visit(FuncDecl* node, void* data) override {
        std::string sig = "FuncDecl " + node->get_FuncName()->get_ID() + "(";
        for (uint32_t i = 0; i < node->num_Args(); ++i) {
            if (i) sig += ", ";
            sig += node->get_Arg(i)->get_ID();
        }
        sig += ")";
        if (node->has_ReturnType()) sig += " with return type";
        line(sig);
        return nested(node, data);
    }

    void* visit(IdDecl* node, void* data) override {
        line("IdDecl '" + node->get_ID() + "'");
        return data;
    }

    void* visit(IdUse* node, void* data) override {
        line("IdUse '" + node->get_ID() + "'");
        return data;
    }

    void* visit(VarStmt* node, void* data) override {
        line("VarStmt");
        return nested(node, data);
    }

    void* visit(BlockStmt* node, void* data) override {
        line("BlockStmt (" + std::to_string(node->num_Stmts()) + " stmts) Parent: " + node->parent()->name());
        return nested(node, data);
    }
    void* visit(IfStmt* node, void* data) override {
        line(node->has_Else() ? "IfStmt (with else)" : "IfStmt");
        return nested(node, data);
    }

    void* visit(BinaryExpr* node, void* data) override {
        return default_visit(node, data);
    }
    void* visit(UnaryExpr* node, void* data) override {
        return default_visit(node, data);
    }
    void* visit(IntType* node, void* data) override {
        return default_visit(node, data);
    }
    void* visit(UnknownType* node, void* data) override {
        return default_visit(node, data);
    }

private:
    void* default_visit(AstNode* node, void* data) {
        line(node->name());
        return nested(node, data);
    }

    int m_Depth = 0;

    void line(const std::string& text) {
        std::printf("%*s%s\n", m_Depth * 2, "", text.c_str());
    }

    void* nested(AstNode* node, void* data) {
        ++m_Depth;
        visit_children(node, data);
        --m_Depth;
        return data;
    }
};

// ---------------------------------------------------------------------------
// 2) Counter: state is passed through the `void* data` parameter
// ---------------------------------------------------------------------------
struct Counts {
    int funcs = 0, vars = 0, ifs = 0, exprs = 0, idUses = 0, total = 0;
};

class CountVisitor : public AstVisitor {
public:
    using AstVisitor::visit;

    void* visit(FuncDecl* n, void* d)   override { bump(d).funcs++;  return visit_children(n, d); }
    void* visit(VarStmt* n, void* d)    override { bump(d).vars++;   return visit_children(n, d); }
    void* visit(IfStmt* n, void* d)     override { bump(d).ifs++;    return visit_children(n, d); }
    void* visit(BinaryExpr* n, void* d) override { bump(d).exprs++;  return visit_children(n, d); }
    void* visit(UnaryExpr* n, void* d)  override { bump(d).exprs++;  return visit_children(n, d); }
    void* visit(IdUse* n, void* d)      override { bump(d).idUses++; return visit_children(n, d); }

private:
    static Counts& bump(void* d) {
        Counts& c = *static_cast<Counts*>(d);
        c.total++;
        return c;
    }
};

// ---------------------------------------------------------------------------
// 3) Generic traversal with no visitor, via children() / begin() / end()
// ---------------------------------------------------------------------------
static void dump_shape(AstNode* node, int depth = 0) {
    std::printf("%*snode with %u children\n", depth * 2, "", node->num_children());
    for (AstNode* child : node->children()) {
        dump_shape(child, depth + 1);
    }
}

// ---------------------------------------------------------------------------
// Build a tree with the generated constructors and run everything:
//
//   func foo(x) -> int {
//       var y;
//       if (x <op> <unary y>) { var z; } else { }
//   }
// ---------------------------------------------------------------------------
int main() {
    auto* cond = new BinaryExpr(
        new IdUse("x"),
        new UnaryExpr(new IdUse("y")));

    auto* thenBlock = new BlockStmt({ new VarStmt(new IdDecl("z")) });
    assert(thenBlock->is<AstNode>());
    assert(thenBlock->is<BlockStmt>());
    assert(thenBlock->is<Stmt>());
    assert(thenBlock->as<Stmt>()->is<BlockStmt>());

    auto* ifStmt = new IfStmt(cond, thenBlock, nullopt);

    auto* func = new FuncDecl(
        new IdDecl("foo"),
        new IntType(),                             // optional ReturnType (use nullopt for none)
        { new IdDecl("x") },                     // Arg*
        { new VarStmt(new IdDecl("y")), ifStmt } // Line*
    );

    auto* func2 = new FuncDecl(
        new IdDecl("bar"),
        nullopt,
        { },
        { new VarStmt(new IdDecl("x")), ifStmt }
    );
    auto* prog = new Program({func, func2});
    assert(func2->has_parent());
    assert(func2->parent()->is<Program>());
    assert(!func2->parent()->is<FuncDecl>());
    assert(func2->is<FuncDecl>());
    assert(!prog->has_parent());

    std::puts("=== PrintVisitor ===");
    PrintVisitor printer;
    prog->accept(&printer, nullptr);

    std::puts("\n=== CountVisitor ===");
    Counts counts;
    CountVisitor counter;
    func->accept(&counter, &counts);
    std::printf("funcs=%d vars=%d ifs=%d exprs=%d idUses=%d visited=%d\n",
                counts.funcs, counts.vars, counts.ifs, counts.exprs,
                counts.idUses, counts.total);

    std::puts("\n=== children() range-for ===");
    dump_shape(func);

    // Cleanup: whether `delete func;` also frees the children depends on what the
    // generated destructors do, so check test_ast.cpp before enabling it.
    return 0;
}