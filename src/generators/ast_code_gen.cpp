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

    // Constructors

    AstNode();
    virtual ~AstNode();

    AstNode(const AstNode&)             = delete;
    AstNode(AstNode&&)                  = delete;
    AstNode& operator=(const AstNode&)  = delete;
    AstNode& operator=(AstNode&&)       = delete;

    // Ast properties

    [[nodiscard]] ::uint32_t line_start() const;
    [[nodiscard]] ::uint32_t line_end() const;
    [[nodiscard]] ::uint32_t column_start() const;
    [[nodiscard]] ::uint32_t column_end() const;

    [[nodiscard]] ::ast::AstNode*       parent();
    [[nodiscard]] ::ast::AstNode const* parent() const;
    [[nodiscard]] bool                  has_parent() const;
    [[nodiscard]] ChildList             children() const;

    template <class T, std::enable_if_t<std::is_base_of_v<AstNode, T>, bool> = true>
    [[nodiscard]] bool      is() const;
    template <class T, std::enable_if_t<std::is_base_of_v<AstNode, T>, bool> = true>
    [[nodiscard]] T*        as();
    template <class T, std::enable_if_t<std::is_base_of_v<AstNode, T>, bool> = true>
    [[nodiscard]] T const*  as() const;

    // Derived properties
    [[nodiscard]] virtual ::ast::AstNode*       child(::uint32_t index) = 0;
    [[nodiscard]] virtual ::ast::AstNode const* child(::uint32_t index) const = 0;
    [[nodiscard]] virtual ::uint32_t            num_children() const = 0;

    [[nodiscard]] virtual int64_t     id() const = 0;
    [[nodiscard]] virtual const char* name() const = 0;

    // Visitor

    [[maybe_unused]] virtual void* accept(AstVisitor* visitor, void* data);

protected:
    static void detail_set_parent(AstNode* child, AstNode* parent);
    [[nodiscard]] virtual bool detail_id_search(int64_t typeId) const;

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

::ast::AstNode*           AstNode::parent()           { assert(has_parent() && "Parent is not available!"); return this->m_Parent; }
::ast::AstNode const*     AstNode::parent() const     { return const_cast<AstNode*>(this)->parent(); }
bool                      AstNode::has_parent() const { return m_Parent != nullptr; }
::ast::AstNode::ChildList AstNode::children() const   { return ChildList(const_cast<::ast::AstNode*>(this)); }

template <>  bool           AstNode::is<AstNode>() const { return true; }
template <>  AstNode*       AstNode::as<AstNode>()       { return this; }
template <>  AstNode const* AstNode::as<AstNode>() const { return this; }

[[maybe_unused]] void* AstNode::accept(AstVisitor* visitor, void* data) {
    assert(false && "Visitors are not generated for this type!");
    return nullptr;
}

void AstNode::detail_set_parent(AstNode* child, AstNode* parent) { child->m_Parent = parent; }
[[nodiscard]] bool AstNode::detail_id_search(int64_t typeId) const { return false; }
)";

            // Now actually generate the files
            auto expansions = visitors::ExpansionVisitor().CollectExpansions(program);

            for (auto& exp : expansions) {
                std::string baseClass;
                std::string childCount = "";
                std::vector<std::pair<std::string, std::string>> constructor = {/* type, member */};
                if (!exp.rule->node->opt_inherit->IsEpsilon) {
                    baseClass = exp.rule->node->opt_inherit->node_ref->ID;
                    childCount = baseClass + "::num_children()";
                } else {
                    baseClass = "AstNode";
                    childCount = "0";
                }
                std::string typeFull = "::ast::" + exp.className;
                cpp << "\ntemplate<> bool AstNode::is<" << exp.className << ">() const "
                    "{ return this->detail_id_search(" << typeFull << "::TYPE_ID); }";
                cpp << "\ntemplate<> " << typeFull << "* AstNode::as<" << exp.className << ">() "
                   "{ assert(this->is<" << typeFull << ">() && \"Bad type conversion!\"); return static_cast<" << typeFull << "*>(this); }";
                cpp << "\ntemplate<> " << typeFull << " const* AstNode::as<" << exp.className << ">() const"
                                  "{ return const_cast<AstNode*>(this)->as<" << typeFull << ">(); }";
                hpp << R"(
class )" << exp.className << R"( : public ::ast::)" << baseClass << R"( {
public:
    static const int64_t TYPE_ID;

    [[nodiscard]]    ::ast::AstNode*       child(::uint32_t index) override;
    [[nodiscard]]    ::ast::AstNode const* child(::uint32_t index) const override;
    [[nodiscard]]    ::uint32_t            num_children() const override;
)";
                if (!exp.rule->IsAbstract) {
                    hpp << R"(
    [[maybe_unused]] void* accept(AstVisitor* visitor, void* data) override;
)";
                }
                cpp << "\nconst int64_t " << exp.className << "::TYPE_ID = " << "int64_t(" << static_cast<int64_t>(std::hash<std::string>{}(exp.className)) << ");";

                hpp << R"(
protected:    
    [[nodiscard]] bool detail_id_search(int64_t typeId) const override;

public:
)";
                cpp << R"(
bool )" << exp.className << R"(::detail_id_search(int64_t typeId) const {
    if (typeId == ::ast::)" << exp.className << R"(::TYPE_ID) return true;
    else return )" << baseClass << R"(::detail_id_search(typeId);
}
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
                        hpp << "public: [[nodiscard]] " << specifier0 << " get_" << mem.specifiers[0].label << "();\n";
                        hpp << "private: " << specifier0 << " " << member0 << ";\n";
                        cpp << specifier0 << " " << exp.className << "::get_" << mem.specifiers[0].label << "() { "
                            "return " << member0 << "; }\n";
                        if (mem.specifiers[0].kind == visitors::EMember_Identifier) {
                            getChildFromIndex += "\n else if (index==(" + childCount + ")) { return " + member0 + "; }";
                            childCount += "+1U";
                        }
                        constructor.emplace_back(specifier0, member0);
                    } else if (mem.kind == visitors::EExpansion_Opt) {
                        hpp << "public: [[nodiscard]] bool has_" << mem.specifiers[0].label << "();\n";
                        hpp << "public: [[nodiscard]] " << specifier0 << " get_" << mem.specifiers[0].label << "();\n";
                        hpp << "private: Opt<" << specifier0 << "> " << member0 << ";\n";
                        cpp << "bool " << exp.className << "::has_" << mem.specifiers[0].label << "() { return this->" << member0 << ".has_value(); }\n";
                        cpp << specifier0 << " " << exp.className << "::get_" << mem.specifiers[0].label << "() { "
                            "assert(this->has_" << mem.specifiers[0].label << "() && \"Cannot get " << mem.specifiers[0].label << "!\"); "
                            "return this->" << member0 << ".value(); }\n";
                        if (mem.specifiers[0].kind == visitors::EMember_Identifier) {
                            getChildFromIndex += "\n else if (" + member0 + ".has_value() && index==(" + childCount + ")) { return " + member0 + ".value(); }";
                            childCount += "+(this->" + member0 + ".has_value()?1U:0U)";
                        }
                        constructor.emplace_back("Opt<" + specifier0 + ">", member0);
                    } else if (mem.kind == visitors::EExpansion_List) {
                        hpp << "public: [[nodiscard]] uint32_t num_" << mem.specifiers[0].label << "s();\n";
                        hpp << "public: [[nodiscard]] List<" << specifier0 << "> const& get_" << mem.specifiers[0].label << "s();\n";
                        hpp << "public: [[nodiscard]] " << specifier0 << " get_" << mem.specifiers[0].label << "(uint32_t index);\n";
                        hpp << "private: List<" << specifier0 << "> " << member0 << ";\n";
                        cpp << "uint32_t " << exp.className << "::num_" << mem.specifiers[0].label << "s() { return static_cast<uint32_t>(this->" << member0 << ".size()); }\n";
                        cpp << "List<" << specifier0 << "> const& " << exp.className << "::get_" << mem.specifiers[0].label << "s() { return this->" << member0 << "; }\n";
                        cpp << specifier0 << " " << exp.className << "::get_" << mem.specifiers[0].label << "(uint32_t index) { "
                            "assert(index < this->" << member0 << ".size() && \"Index out of bounds\"); return this->" << member0 << ".at(index); }\n";

                        if (mem.specifiers[0].kind == visitors::EMember_Identifier) {
                            getChildFromIndex += "\n else if (index >= (" + childCount + ") && index < ((" + childCount + ") + static_cast<uint32_t>(" + member0 + ".size()))) { return " + member0 + ".at(index-(" + childCount + ")); }";
                            childCount += "+static_cast<uint32_t>(this->" + member0 + ".size())";
                        }
                        constructor.emplace_back("List<" + specifier0 + ">", member0);
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
                if (!exp.rule->IsAbstract) {
                    hpp << "    [[nodiscard]] int64_t     id() const override;\n";
                    hpp << "    [[nodiscard]] const char* name() const override;\n";

                    cpp << "    [[nodiscard]] int64_t     " << exp.className << "::id() const { return " << typeFull << "::TYPE_ID; }\n";
                    cpp << "    [[nodiscard]] const char* " << exp.className << "::name() const { return \"" << exp.className << "\"; }\n";
                }
                std::string constructorList = "";
                for (int i = 0; i < constructor.size(); ++i) {
                    if (i != 0) {
                        constructorList += ", ";
                    }
                    constructorList += constructor[i].first + " init_" + constructor[i].second;
                }
                if (exp.rule->IsAbstract) {
                    hpp << "protected: \n";
                } else {
                    hpp << "public: \n";
                }
                hpp << "    " << exp.className << "(" << constructorList << ");\n";
                hpp << "    ~" << exp.className << "() override;\n";

                cpp << "\n" << exp.className << "::" << exp.className << "(" << constructorList << ")";
                if (constructor.size() > 0) {
                    cpp << "\n: ";
                    for (int i = 0; i < constructor.size(); ++i) {
                        if (i != 0) {
                            cpp << ", ";
                        }
                        cpp << constructor[i].second << "(std::move(init_" << constructor[i].second << "))";
                    }
                }
                cpp << R"( {
    m_Parent = nullptr;
    for (auto it = children().begin(); it != children().end(); ++it) ::ast::AstNode::detail_set_parent(*it, this);
}
)";
                cpp << exp.className << "::~" << exp.className << "() {}\n";


                hpp << "};\n";

            }

        }

    } // namespace generators
} // namespace zblast