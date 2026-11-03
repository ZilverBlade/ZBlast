#include "ast_code_gen.h"
#include "parsetree.h"
#include "visitors/expansion_visitor.h"

#include <stack>
#include <unordered_map>

namespace zblast {
    namespace generators {
        void AstCodeGen::Generate(parsetree::Start* program, std::ostream& hpp, std::ostream& cpp) {
            // forward declare everything
            auto expansions = visitors::ExpansionVisitor().CollectExpansions(program);

            for (auto& exp : expansions) {
                hpp << "class " << exp.className << ";\n";
            }

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

        [[nodiscard]] ZBLAST_API Iterator begin() const;
        [[nodiscard]] ZBLAST_API Iterator end() const;

        [[nodiscard]] ZBLAST_API AstNode* operator[](::uint32_t index);
        [[nodiscard]] ZBLAST_API AstNode const* operator[](::uint32_t index) const;

        [[nodiscard]] ZBLAST_API uint32_t size() const;
        [[nodiscard]] ZBLAST_API bool empty() const;

    private:
        AstNode* m_Node;

    };

    // Constructors

    ZBLAST_API AstNode();
    ZBLAST_API virtual ~AstNode();

    AstNode(const AstNode&)             = delete;
    AstNode(AstNode&&)                  = delete;
    AstNode& operator=(const AstNode&)  = delete;
    AstNode& operator=(AstNode&&)       = delete;

    // Ast properties

    [[nodiscard]] ZBLAST_API ::uint32_t line_start() const;
    [[nodiscard]] ZBLAST_API ::uint32_t line_end() const;
    [[nodiscard]] ZBLAST_API ::uint32_t column_start() const;
    [[nodiscard]] ZBLAST_API ::uint32_t column_end() const;

    [[nodiscard]] ZBLAST_API ::ast::AstNode*       parent();
    [[nodiscard]] ZBLAST_API ::ast::AstNode const* parent() const;
    [[nodiscard]] ZBLAST_API bool                  has_parent() const;
    [[nodiscard]] ZBLAST_API ChildList             children() const;

    template <class T, std::enable_if_t<std::is_base_of_v<AstNode, T>, bool> = true>
    [[nodiscard]] ZBLAST_API bool      is() const;
    template <class T, std::enable_if_t<std::is_base_of_v<AstNode, T>, bool> = true>
    [[nodiscard]] ZBLAST_API T*        as();
    template <class T, std::enable_if_t<std::is_base_of_v<AstNode, T>, bool> = true>
    [[nodiscard]] ZBLAST_API T const*  as() const;

    // Derived properties
    [[nodiscard]] virtual ::ast::AstNode*       child(::uint32_t index) = 0;
    [[nodiscard]] virtual ::ast::AstNode const* child(::uint32_t index) const = 0;
    [[nodiscard]] virtual ::uint32_t            num_children() const = 0;

    [[nodiscard]] virtual int64_t     id() const = 0;
    [[nodiscard]] virtual const char* name() const = 0;

    // Visitor

    [[maybe_unused]] ZBLAST_API virtual void* accept(AstVisitor* visitor, void* data);

protected:
    // Cannot be placed in AstNode constructor, due to in-order construction
    void detail_set_parent();
    // Cannot be placed in AstNode destructor, due to reverse-order destructor
    void detail_destroy_children();
    [[nodiscard]] virtual bool detail_id_search(int64_t typeId) const;

private:
    ::ast::AstNode* m_Parent      = nullptr;
    ::uint32_t      m_LineStart   = 0;
    ::uint32_t      m_LineEnd     = 0;
    ::uint32_t      m_ColumnStart = 0;
    ::uint32_t      m_ColumnEnd   = 0;
    bool            m_IsDestroyed = false;
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
AstNode::~AstNode() {
    assert(this->m_Parent == nullptr && "Do not destroy child nodes. Only call 'delete' on the root node!");
}

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

void* AstNode::accept(AstVisitor* visitor, void* data) {
    assert(false && "Visitors are not generated for this type!");
    return nullptr;
}

void AstNode::detail_set_parent() {
    ChildList clist = this->children();
    for (auto it = clist.begin(); it != clist.end(); ++it) {
        AstNode* c = *it;
        assert((c->m_Parent == this || c->m_Parent == nullptr) && "Child already has a parent set! Do not assign children to multiple parents!");
        (*it)->m_Parent = this;
    }
}

void AstNode::detail_destroy_children() {
    if (this->m_IsDestroyed) return;
    ChildList clist = this->children();
    for (auto it = clist.begin(); it != clist.end(); ++it) {
        AstNode* c = *it;
        c->m_Parent = nullptr;
        delete *it;
    }
    m_IsDestroyed = true;
}

bool AstNode::detail_id_search(int64_t typeId) const { return false; }
)";

            // Now actually generate the files
            
            struct Constructor {
                std::string label;
                std::string elementType;
                std::string type;
                std::string member;
                const visitors::ExpansionMember* memberPtr;
                const visitors::MemberSpecifier* specifiers;
                size_t numSpecifiers;
                parsetree::Node* parent = nullptr;
            };

            std::unordered_map<std::string, std::vector<Constructor>> constructorMap;
            std::unordered_map<std::string, parsetree::Rule*> idToRuleMap;    

            for (auto& exp: expansions) {
                idToRuleMap[exp.className] = exp.rule;
                std::vector<Constructor> constructor;
                for (auto& mem : exp.members) {
                    std::string label = mem.specifiers[0].label;
                    std::string member0 = "m_Member_" + label;
                    std::string specifier0;
                    if (mem.specifiers[0].kind == visitors::EMember_Identifier) {
                        specifier0 = "::ast::" + mem.specifiers[0].nodeName + "*";
                    } else if (mem.specifiers[0].kind == visitors::EMember_Literal) {
                        specifier0 = "String /*" + mem.specifiers[0].nodeName + "*/";
                    }
                    if (mem.kind == visitors::EExpansion_Normal) {
                        constructor.emplace_back(label, specifier0, specifier0, member0, &mem, mem.specifiers.data(), mem.specifiers.size());
                    } else if (mem.kind == visitors::EExpansion_Opt) {
                        constructor.emplace_back(label, specifier0, "Opt<" + specifier0 + ">", member0, &mem, mem.specifiers.data(), mem.specifiers.size());
                    } else if (mem.kind == visitors::EExpansion_List) {
                        constructor.emplace_back(label, specifier0, "List<" + specifier0 + ">", member0, &mem, mem.specifiers.data(), mem.specifiers.size());
                    }
                }
                constructorMap.emplace(exp.className, std::move(constructor));
            }

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
    ZBLAST_API static const int64_t TYPE_ID;

    [[nodiscard]]    ZBLAST_API ::ast::AstNode*       child(::uint32_t index) override;
    [[nodiscard]]    ZBLAST_API ::ast::AstNode const* child(::uint32_t index) const override;
    [[nodiscard]]    ZBLAST_API ::uint32_t            num_children() const override;
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
                if (baseClass != "AstNode") {
                    getChildFromIndex += " else if (index<::ast::" + baseClass + "::num_children()) { return ::ast::" + baseClass + "::child(index); }";
                }
                const auto& constructor = constructorMap[exp.className];
                for (auto& mem : constructor) {
                    if (mem.memberPtr->kind == visitors::EExpansion_Normal) {
                        hpp << "public: [[nodiscard]] ZBLAST_API " << mem.type << " get_" << mem.specifiers[0].label << "();\n";
                        hpp << "private: " << mem.type << " " << mem.member << ";\n";
                        cpp << mem.type << " " << exp.className << "::get_" << mem.specifiers[0].label << "() { "
                            "return " << mem.member << "; }\n";
                        if (mem.specifiers[0].kind == visitors::EMember_Identifier) {
                            getChildFromIndex += "\n else if (index==(" + childCount + ")) { return " + mem.member + "; }";
                            childCount += "+1U";
                        }
                    } else if (mem.memberPtr->kind == visitors::EExpansion_Opt) {
                        hpp << "public: [[nodiscard]] ZBLAST_API bool has_" << mem.specifiers[0].label << "();\n";
                        hpp << "public: [[nodiscard]] ZBLAST_API " << mem.elementType << " get_" << mem.specifiers[0].label << "();\n";
                        hpp << "private: " << mem.type << " " << mem.member << ";\n";
                        cpp << "bool " << exp.className << "::has_" << mem.specifiers[0].label << "() { return this->" << mem.member << ".has_value(); }\n";
                        cpp << mem.elementType << " " << exp.className << "::get_" << mem.specifiers[0].label << "() { "
                            "assert(this->has_" << mem.specifiers[0].label << "() && \"Cannot get " << mem.specifiers[0].label << "!\"); "
                            "return this->" << mem.member << ".value(); }\n";
                        if (mem.specifiers[0].kind == visitors::EMember_Identifier) {
                            getChildFromIndex += "\n else if (" + mem.member + ".has_value() && index==(" + childCount + ")) { return " + mem.member + ".value(); }";
                            childCount += "+(this->" + mem.member + ".has_value()?1U:0U)";
                        }
                    } else if (mem.memberPtr->kind == visitors::EExpansion_List) {
                        hpp << "public: [[nodiscard]] ZBLAST_API uint32_t num_" << mem.specifiers[0].label << "s();\n";
                        hpp << "public: [[nodiscard]] ZBLAST_API " << mem.type << " const& get_" << mem.specifiers[0].label << "s();\n";
                        hpp << "public: [[nodiscard]] ZBLAST_API " << mem.elementType << " get_" << mem.specifiers[0].label << "(uint32_t index);\n";
                        hpp << "private: " << mem.type << " " << mem.member << ";\n";
                        cpp << "uint32_t " << exp.className << "::num_" << mem.specifiers[0].label << "s() { return static_cast<uint32_t>(this->" << mem.member << ".size()); }\n";
                        cpp << mem.type << " const& " << exp.className << "::get_" << mem.specifiers[0].label << "s() { return this->" << mem.member << "; }\n";
                        cpp << mem.elementType << " " << exp.className << "::get_" << mem.specifiers[0].label << "(uint32_t index) { "
                            "assert(index < this->" << mem.member << ".size() && \"Index out of bounds\"); return this->" << mem.member << ".at(index); }\n";

                        if (mem.specifiers[0].kind == visitors::EMember_Identifier) {
                            getChildFromIndex += "\n else if (index >= (" + childCount + ") && index < ((" + childCount + ") + static_cast<uint32_t>(" + mem.member + ".size()))) { return " + mem.member + ".at(index-(" + childCount + ")); }";
                            childCount += "+static_cast<uint32_t>(this->" + mem.member + ".size())";
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
                if (!exp.rule->IsAbstract) {
                    hpp << "    [[nodiscard]] int64_t     id() const override;\n";
                    hpp << "    [[nodiscard]] const char* name() const override;\n";

                    cpp << "    int64_t     " << exp.className << "::id() const { return " << typeFull << "::TYPE_ID; }\n";
                    cpp << "    const char* " << exp.className << "::name() const { return \"" << exp.className << "\"; }\n";
                }

                std::stack<parsetree::Node*> parents;
                { 
                    parsetree::Node* parent = exp.rule->node.get();
                    while (parent != nullptr) {
                        parents.emplace(parent);
                        if (parent->opt_inherit->IsEpsilon)
                            parent = nullptr;
                        else parent = idToRuleMap[parent->opt_inherit->node_ref->ID]->node.get();
                    }
                }

                std::string fullConstructorList = "";
                std::string parentInitList = "";
                while (!parents.empty()) {
                    parsetree::Node* parent = parents.top();
                    parents.pop();
                    const auto &parentConstructor = constructorMap[parent->node_ref->ID];
                    for (int i = 0; i < parentConstructor.size(); ++i) {
                        if (!fullConstructorList.empty()) {
                            fullConstructorList += ", ";
                        }
                        fullConstructorList += parentConstructor[i].type + " init_" + parentConstructor[i].label;
                        if (!parents.empty()) {
                            if (!parentInitList.empty()) {
                                parentInitList += ", ";
                            }
                            parentInitList += " std::move(init_" + parentConstructor[i].label + ")";
                        }
                    }
                }

                if (exp.rule->IsAbstract) {
                    hpp << "protected: \n";
                } else {
                    hpp << "public: \n";
                }
                hpp << "    ZBLAST_API " << exp.className << "(" << fullConstructorList << ");\n";
                hpp << "    ZBLAST_API virtual ~" << exp.className << "() override;\n";

                cpp << "\n" << exp.className << "::" << exp.className << "(" << fullConstructorList << ")";
                
                parsetree::Node* immediateParent = exp.rule->node->opt_inherit->IsEpsilon ? nullptr 
                    : idToRuleMap[exp.rule->node->opt_inherit->node_ref->ID]->node.get();
                
                if (constructor.size() > 0 || immediateParent) {
                    cpp << "\n: ";
                    bool addComma = false;
                    if (immediateParent) {
                        cpp << immediateParent->node_ref->ID << "(" << parentInitList << ")";
                        addComma = true;
                    }
                    for (int i = 0; i < constructor.size(); ++i) {
                        if (addComma) {
                            cpp << ", ";
                        } else {
                            addComma = true;
                        }
                        cpp << constructor[i].member << "(std::move(init_" << constructor[i].label << "))";
                    }
                }
                cpp << "{ this->detail_set_parent(); }\n";
                cpp << exp.className << "::~" << exp.className << "() { this->detail_destroy_children(); }\n";


                hpp << "};\n";

            }

        }

    } // namespace generators
} // namespace zblast