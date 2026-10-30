#include "ast_code_gen.h"
#include "parsetree.h"
#include "visitors/expansion_visitor.h"

namespace zblast {
    namespace generators {
        void AstCodeGen::Generate(parsetree::Start* program, std::ostream& hpp, std::ostream& cpp) {
            // biiig header
            hpp << R"(
class AstVisitor; // zblast -v

class AstNode {
public:
    struct ChildList {
        struct Iterator {
            Iterator(AstNode* node, uint32_t index);
            ~Iterator();

            [[maybe_unused]] Iterator operator++ (int);
            [[maybe_unused]] Iterator& operator++();

            [[maybe_unused]] Iterator operator-- (int);
            [[maybe_unused]] Iterator& operator--();

            [[nodiscard]] bool operator== (const Iterator& other) const;
            [[nodiscard]] bool operator!= (const Iterator& other) const;

            [[nodiscard]] AstNode* operator*();
            [[nodiscard]] AstNode const* operator*() const;
            
        private:
            AstNode* m_Node;
            uint32_t m_Index;
            uint32_t m_Size;
        };

        ChildList(AstNode* node);
        ~ChildList();

        [[nodiscard]] Iterator begin() const;
        [[nodiscard]] Iterator end() const;

        [[nodiscard]] AstNode* operator[](::uint32_t index);
        [[nodiscard]] AstNode const* operator[](::uint32_t index) const;

        [[nodiscard]] uint32_t size() const;
        [[nodiscard]] bool empty() const;
        
    private:
        AstNode* m_Node;

    };

    AstNode();
    virtual ~AstNode();

    // Ast properties
    
    [[nodiscard]] ::uint32_t line_start() const;
    [[nodiscard]] ::uint32_t line_end() const;
    [[nodiscard]] ::uint32_t column_start() const;
    [[nodiscard]] ::uint32_t column_end() const;
    
    [[nodiscard]] ::ast::AstNode*       parent();
    [[nodiscard]] ::ast::AstNode const* parent() const;
    [[nodiscard]] ChildList             children() const;

    // Derived properties
    [[nodiscard]] virtual ::ast::AstNode*       child(::uint32_t index) = 0;
    [[nodiscard]] virtual ::ast::AstNode const* child(::uint32_t index) const = 0;
    [[nodiscard]] virtual ::uint32_t            num_children() const = 0;

    // Visitor

    [[maybe_unused]] virtual void* accept(AstVisitor* visitor, void* data);
    
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

bool AstNode::ChildList::Iterator::operator== (const AstNode::ChildList::Iterator& other) const {
    return this->m_Index == other.m_Index && this->m_Size == other.m_Size && this->m_Node == other.m_Node;
}

bool AstNode::ChildList::Iterator::operator!= (const AstNode::ChildList::Iterator& other) const {
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

::ast::AstNode::ChildList::Iterator AstNode::ChildList::begin() const {
    return Iterator(this->m_Node, 0);
}

::ast::AstNode::ChildList::Iterator AstNode::ChildList::end() const {
    return Iterator(this->m_Node, this->m_Node->num_children());
}

::ast::AstNode* AstNode::ChildList::operator[](::uint32_t index) {
    return this->m_Node->child(index);
}

::ast::AstNode const* AstNode::ChildList::operator[](::uint32_t index) const {
    return this->m_Node->child(index);
}

uint32_t AstNode::ChildList::size() const {
    return this->m_Node->num_children();
}

bool AstNode::ChildList::empty() const {
    return this->size() == 0;
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

[[maybe_unused]] void* AstNode::accept(AstVisitor* visitor, void* data) {
    assert(false && "Visitors are not generated!");
    return nullptr;
}
)";

            // Now actually generate the files
            auto expansions = visitors::ExpansionVisitor().CollectExpansions(program);
            
            for (auto& exp : expansions) {
                std::string baseClass;
                std::string childCount = "";
                if (!exp.rule->node->opt_inherit->IsEpsilon) {
                    baseClass = exp.rule->node->opt_inherit->node_ref->ID;
                    childCount = baseClass + "::num_children()";
                } else {
                    baseClass = "AstNode";
                    childCount = "0";
                }
                hpp << R"(
class )" << exp.className << R"( : public ::ast::)" << baseClass << R"( {
public:
    [[nodiscard]]    ::ast::AstNode*       child(::uint32_t index) override;
    [[nodiscard]]    ::ast::AstNode const* child(::uint32_t index) const override;
    [[nodiscard]]    ::uint32_t            num_children() const override;

    [[maybe_unused]] void* accept(AstVisitor* visitor, void* data) override;
)";
            std::string getChildFromIndex = " if (false) { assert(false); }";
            for (auto& mem : exp.members) {
                std::string member0 = "m_Member_" + mem.specifiers[0].label;
                std::string specifier0;
                if (mem.specifiers[0].kind == visitors::EMember_Identifier) {
                    specifier0 = "::ast::" + mem.specifiers[0].nodeName + "*";
                } else if (mem.specifiers[0].kind == visitors::EMember_Literal) {
                    specifier0 = "String /*" + mem.specifiers[0].nodeName + "*/";
                }
                if (mem.kind == visitors::EExpansion_Normal) {
                    hpp << "public: " << specifier0 << " get_" << mem.specifiers[0].label << "();\n";
                    hpp << "private: " << specifier0 << " " << member0 << ";\n";
                    cpp << specifier0 << " " << exp.className << "::get_" << mem.specifiers[0].label << "() { "
                        "return " << member0 << "; }\n";
                    if (mem.specifiers[0].kind == visitors::EMember_Identifier) {
                        getChildFromIndex += "\n else if (index==(" + childCount + ")) { return " + member0 + "; }";
                        childCount += "+1U";
                    }
                } else if (mem.kind == visitors::EExpansion_Opt) {
                    hpp << "public: bool has_" << mem.specifiers[0].label << "();\n";
                    hpp << "public: " << specifier0 << " get_" << mem.specifiers[0].label << "();\n";
                    hpp << "private: Opt<" << specifier0 << "> " << member0 << ";\n";
                    cpp << "bool " << exp.className << "::has_" << mem.specifiers[0].label << "() { return this->" << member0 << ".has_value(); }\n";
                    cpp << specifier0 << " " << exp.className << "::get_" << mem.specifiers[0].label << "() { "
                        "assert(this->has_" << mem.specifiers[0].label << "() && \"Cannot get " << mem.specifiers[0].label << "!\"); "
                        "return this->" << member0 << ".value(); }\n";
                    if (mem.specifiers[0].kind == visitors::EMember_Identifier) {
                        getChildFromIndex += "\n else if (" + member0 + ".has_value() && index==(" + childCount + ")) { return " + member0 + ".value(); }";
                        childCount += "+(this->" + member0 + ".has_value()?1U:0U)";
                    }
                } else if (mem.kind == visitors::EExpansion_List) {
                    hpp << "public: uint32_t num_" << mem.specifiers[0].label << "s();\n";
                    hpp << "public: List<" << specifier0 << "> const& get_" << mem.specifiers[0].label << "s();\n";
                    hpp << "public: " << specifier0 << " get_" << mem.specifiers[0].label << "(uint32_t index);\n";
                    hpp << "private: List<" << specifier0 << "> " << member0 << ";\n";
                    cpp << "uint32_t " << exp.className << "::num_" << mem.specifiers[0].label << "s() { return static_cast<uint32_t>(this->" << member0 << ".size()); }\n";
                    cpp << "List<" << specifier0 << "> const& " << exp.className << "::get_" << mem.specifiers[0].label << "s() { return this->" << member0 << "; }\n";
                    cpp << specifier0 << " " << exp.className << "::get_" << mem.specifiers[0].label << "(uint32_t index) { "
                        "assert(index < this->" << member0 << ".size() && \"Index out of bounds\"); return this->" << member0 << ".at(index); }\n";
                    
                    if (mem.specifiers[0].kind == visitors::EMember_Identifier) {
                        getChildFromIndex += "\n else if (index >= (" + childCount + ") && index < ((" + childCount + ") + static_cast<uint32_t>(" + member0 + ".size()))) { return " + member0 + ".at(index-(" + childCount + ")); }";
                        childCount += "+static_cast<uint32_t>(this->" + member0 + ".size())";
                    }
                }
            }
            cpp << R"(
::ast::AstNode*       )" << exp.className << R"(::child(::uint32_t index) { 
)" << getChildFromIndex << R"(
 else { assert(false && "Index out of bounds"); return nullptr;
 } 
}
::ast::AstNode const* )" << exp.className << R"(::child(::uint32_t index) const { return const_cast<)" << exp.className  <<  R"(*>(this)->child(index); }
::uint32_t            )" << exp.className << R"(::num_children() const { return )" << childCount << R"(; }
            )";

            hpp << "};\n";
            
            }

        }

    } // namespace generators
} // namespace zblast