#include "ast_code_gen.h"
#include "parsetree.h"

namespace zblast {
    namespace generators {
        void AstCodeGen::Generate(parsetree::Start* program, std::ostream& hpp, std::ostream& cpp) {
            hpp << R"(
class AstNode {
public:
    AstNode();
    virtual ~AstNode();

    // Ast properties
    
    ::ast::AstNode* parent();
    ::ast::AstNode const* parent() const;
    ::size_t line_start() const;
    ::size_t line_end() const;
    ::size_t column_start() const;
    ::size_t column_end() const;

    // Derived properties

    List<::ast::AstNode*> const&       children() const;
    ::ast::AstNode*                    child(size_t index);
    ::ast::AstNode const*              child(size_t index) const;
    ::size_t                           num_children() const;

protected:

    ::ast::AstNode* m_Parent      = nullptr;
    ::size_t        m_LineStart   = 0; 
    ::size_t        m_LineEnd     = 0; 
    ::size_t        m_ColumnStart = 0; 
    ::size_t        m_ColumnEnd   = 0; 

    List<::ast::AstNode*> m_ChildrenCache;
};             
)";

            cpp << R"(
AstNode::AstNode() {}
AstNode::~AstNode() {}

::ast::AstNode*       AstNode::parent()             { return m_Parent;      }
::ast::AstNode const* AstNode::parent() const       { return m_Parent;      }
::size_t              AstNode::line_start() const   { return m_LineStart;   }
::size_t              AstNode::line_end() const     { return m_LineEnd;     }
::size_t              AstNode::column_start() const { return m_ColumnStart; }
::size_t              AstNode::column_end() const   { return m_ColumnEnd;   }


List<::ast::AstNode*> const& AstNode::children() const           { return m_ChildrenCache; }
::ast::AstNode*              AstNode::child(size_t index)        { assert(index < num_children() && "Index out of range!"); return m_ChildrenCache.at(index); }
::ast::AstNode const*        AstNode::child(size_t index) const  { assert(index < num_children() && "Index out of range!"); return m_ChildrenCache.at(index); }
::size_t                     AstNode::num_children() const       { return m_ChildrenCache.size(); }
)";
        }

    } // namespace generators
} // namespace zblast