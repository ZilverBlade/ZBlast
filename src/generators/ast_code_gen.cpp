#include "ast_code_gen.h"
#include "parsetree.h"

namespace zblast {
    namespace generators {
        void AstCodeGen::Generate(parsetree::Start* program, std::ostream& hpp, std::ostream& cpp) {
            hpp << R"(
class AstNode {
public:
    struct ChildList {
        struct Iterator {
            Iterator(AstNode* node, uint32_t index);
            ~Iterator();

            Iterator operator++ (int);
            Iterator& operator++();

            Iterator operator-- (int);
            Iterator& operator--();

            bool operator== (const Iterator& other);
            bool operator!= (const Iterator& other);

            AstNode* operator*();
            AstNode const* operator*() const;
            
        private:
            AstNode* m_Node;
            uint32_t m_Index;
            uint32_t m_Size;
        };

        ChildList(AstNode* node);
        ~ChildList();

        Iterator start() const;
        Iterator end() const;

        AstNode* operator[](size_t index);
        AstNode const* operator[](size_t index) const;

    private:
        AstNode* m_Node;

    };

    AstNode();
    virtual ~AstNode();

    // Ast properties
    
    ::uint32_t line_start() const;
    ::uint32_t line_end() const;
    ::uint32_t column_start() const;
    ::uint32_t column_end() const;
    
    ::ast::AstNode*       parent();
    ::ast::AstNode const* parent() const;
    ChildList             children() const;

    // Derived properties
    virtual ::ast::AstNode*       child(uint32_t index) = 0;
    virtual ::ast::AstNode const* child(uint32_t index) const = 0;
    virtual ::uint32_t            num_children() const = 0;

protected:

    ::ast::AstNode* m_Parent      = nullptr;
    ::uint32_t      m_LineStart   = 0; 
    ::uint32_t      m_LineEnd     = 0; 
    ::uint32_t      m_ColumnStart = 0; 
    ::uint32_t      m_ColumnEnd   = 0; 
};
)";

            cpp << R"(
AstNode::ChildList::Iterator::Iterator(AstNode* node, uint32_t index) 
    : m_Node(node), m_Index(index), m_Size(node->num_children()) {}
AstNode::ChildList::Iterator::~Iterator() {}

::ast::AstNode::ChildList::Iterator AstNode::ChildList::Iterator::operator++ (int) {
    Iterator copy = *this;
    this->operator++();
    return copy;
}

::ast::AstNode::ChildList::Iterator& AstNode::ChildList::Iterator::operator ++() {
    ++this->m_Index;
    return *this;
}

::ast::AstNode::ChildList::Iterator AstNode::ChildList::Iterator::operator-- (int) {
    Iterator copy = *this;
    this->operator--();
    return copy;
}

::ast::AstNode::ChildList::Iterator& AstNode::ChildList::Iterator::operator --() {
    --this->m_Index;
    return *this;
}

bool AstNode::ChildList::Iterator::operator== (const AstNode::ChildList::Iterator& other) {
    return this->m_Index == other.m_Index && this->m_Size == other.m_Size && this->m_Node == other.m_Node;
}

bool AstNode::ChildList::Iterator::operator!= (const AstNode::ChildList::Iterator& other) {
    return !(*this == other);
}

::ast::AstNode* AstNode::ChildList::Iterator::operator*() { 
    return this->m_Node->child(this->m_Index); 
} 

::ast::AstNode const* AstNode::ChildList::Iterator::operator*() const { 
    return this->m_Node->child(this->m_Index); 
} 

AstNode::ChildList::ChildList(AstNode* node) : m_Node(node) {}
AstNode::ChildList::~ChildList() {}

::ast::AstNode::ChildList::Iterator AstNode::ChildList::start() const {
    return Iterator(this->m_Node, 0);
}

::ast::AstNode::ChildList::Iterator AstNode::ChildList::end() const {
    return Iterator(this->m_Node, this->m_Node->num_children());
}

::ast::AstNode* AstNode::ChildList::operator[](::size_t index) {
    return this->m_Node->child(index);
}

::ast::AstNode const* AstNode::ChildList::operator[](::size_t index) const {
    return this->m_Node->child(index);
}

AstNode::AstNode() {}
AstNode::~AstNode() {}

::uint32_t AstNode::line_start() const   { return this->m_LineStart;   }
::uint32_t AstNode::line_end() const     { return this->m_LineEnd;     }
::uint32_t AstNode::column_start() const { return this->m_ColumnStart; }
::uint32_t AstNode::column_end() const   { return this->m_ColumnEnd;   }

::ast::AstNode*           AstNode::parent()         { return this->m_Parent; }
::ast::AstNode const*     AstNode::parent() const   { return this->m_Parent; }
::ast::AstNode::ChildList AstNode::children() const { return ChildList(const_cast<::ast::AstNode*>(this)); }
)";
        }

    } // namespace generators
} // namespace zblast