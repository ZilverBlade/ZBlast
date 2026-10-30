#pragma once

#include <string>
#include <vector>
#include <memory>
#include <ostream>
#include <fstream>
#include <sstream>
#include <type_traits>

#include "visitor.h"

namespace zblast {
    namespace parsetree {
        using Str = std::string;

        template<typename T>
        using Ptr = std::unique_ptr<T>;

        template<typename T>
        using List = std::vector<T>;

        struct AstNode {
            virtual ~AstNode() = default;

            virtual const char* NodeName() const = 0;

            virtual List<AstNode*> Children() const { return {}; }
            virtual List<Str> Values() const { return {}; }
            [[maybe_unused]] virtual void* Accept(Visitor* visitor, void* data) {
                char errBuf[128];
                snprintf(errBuf, 128, "Missing Accept() implementation for %s!", typeid(this).name());
                throw std::runtime_error(errBuf);
            }

            int LineStart = -1;
            int ColumnStart = -1;
            int LineEnd = -1;
            int ColumnEnd = -1;
        };

        struct NodeRef : AstNode {
            Str ID;
            const char* NodeName() const override { return "NodeRef"; }
            List<Str> Values() const override { return { ID }; }
            void* Accept(Visitor* visitor, void* data) override { return visitor->Visit(this, data); }
        };

        struct OptInherit : AstNode {
            bool IsEpsilon = false;
            Ptr<NodeRef> node_ref;
            const char* NodeName() const override { return "OptInherit"; }
            List<AstNode*> Children() const override { return { node_ref.get() }; }
            void* Accept(Visitor* visitor, void* data) override { return visitor->Visit(this, data); }
        };

        struct Node : AstNode {
            Ptr<NodeRef> node_ref;
            Ptr<OptInherit> opt_inherit;
            const char* NodeName() const override { return "Node"; }
            List<AstNode*> Children() const override { return { node_ref.get(), opt_inherit.get() }; }
            void* Accept(Visitor* visitor, void* data) override { return visitor->Visit(this, data); }
        };

        struct Child;

        struct Iden : AstNode {
            bool IsLiteral = false;
            Str ID;
            const char* NodeName() const override { return "Iden"; }
            List<Str> Values() const override { return { ID }; }
            void* Accept(Visitor* visitor, void* data) override { return visitor->Visit(this, data); }
        };

        struct LabeledIden : AstNode {
            bool HasLabel = false;
            Str LABEL;
            Ptr<Iden> iden;
            const char* NodeName() const override { return "LabeledIden"; }
            List<AstNode*> Children() const override { return { iden.get() }; }
            List<Str> Values() const override { return { LABEL }; }
            void* Accept(Visitor* visitor, void* data) override { return visitor->Visit(this, data); }
        };

        struct VariantQual : AstNode {
            bool IsEpsilon;
            Ptr<LabeledIden> labeled_iden;
            Ptr<VariantQual> variant_qual;
            const char* NodeName() const override { return "VariantQual"; }

            List<AstNode*> Children() const override { return { labeled_iden.get(), variant_qual.get() }; }
            void* Accept(Visitor* visitor, void* data) override { return visitor->Visit(this, data); }
        };

        struct ChildQual : AstNode {
            enum {
                Mode_List,
                Mode_Variant,
                IsEpsilon,
            } Mode;

            Ptr<VariantQual> variant_qual;
            const char* NodeName() const override { return "ChildQual"; }

            List<AstNode*> Children() const override { return { variant_qual.get() }; }
            void* Accept(Visitor* visitor, void* data) override { return visitor->Visit(this, data); }
        };

        struct Child : AstNode {
            enum {
                Mode_IdentifierWQualifier,
                Mode_Optional,
            } Mode;

            Ptr<LabeledIden> labeled_iden;
            Ptr<ChildQual> child_qual;

            const char* NodeName() const override { return "Child"; }
            List<AstNode*> Children() const override { return { labeled_iden.get(), child_qual.get() }; }
            void* Accept(Visitor* visitor, void* data) override { return visitor->Visit(this, data); }
        };

        struct ChildList : AstNode {
            bool IsEpsilon = false;
            Ptr<Child> child;
            Ptr<ChildList> child_list;

            const char* NodeName() const override { return "ChildList"; }
            List<AstNode*> Children() const override { return { child.get(), child_list.get() }; }
            void* Accept(Visitor* visitor, void* data) override { return visitor->Visit(this, data); }
        };


        struct OptExpansion : AstNode {
            bool IsEpsilon = false;
            Ptr<ChildList> child_list;
            const char* NodeName() const override { return "OptExpansion"; }
            List<AstNode*> Children() const override { return { child_list.get() }; }
            void* Accept(Visitor* visitor, void* data) override { return visitor->Visit(this, data); }
        };

        struct Rule : AstNode {
            bool IsAbstract = false;
            Ptr<Node> node;
            Ptr<OptExpansion> opt_expansion;
            const char* NodeName() const override { return "Rule"; }
            List<AstNode*> Children() const override { return { node.get(), opt_expansion.get() }; }
            void* Accept(Visitor* visitor, void* data) override { return visitor->Visit(this, data); }
        };

        struct RuleList : AstNode {
            bool IsEpsilon = false;
            Ptr<Rule> rule;
            Ptr<RuleList> rule_list;

            const char* NodeName() const override { return "RuleList"; }
            List<AstNode*> Children() const override { return { rule.get(), rule_list.get() }; }
            void* Accept(Visitor* visitor, void* data) override { return visitor->Visit(this, data); }
        };

        struct Start : AstNode {
            Ptr<RuleList> rule_list;

            const char* NodeName() const override { return "Start"; }
            List<AstNode*> Children() const override { return { rule_list.get() }; }
            void* Accept(Visitor* visitor, void* data) override { return visitor->Visit(this, data); }
        };

        inline void PrintTree(const AstNode* node, std::ostream& out, int depth = 0) {
            if (!node) return;
            for (int i = 0; i < depth; ++i) {
                out << "  ";
            }
            out << node->NodeName();
            auto values = node->Values();
            bool first = true;
            for (const auto& v : values) {
                if (!v.empty()) {
                    if (first) {
                        out << ": ";
                        first = false;
                    }
                    else {
                        out << ", ";
                    }
                    out << v;
                }
            }
            out << "\n";
            for (const auto* child : node->Children()) {
                if (child) {
                    PrintTree(child, out, depth + 1);
                }
            }
        }

        inline std::string PrintTreeToString(const AstNode* node) {
            std::ostringstream ss;
            PrintTree(node, ss);
            return ss.str();
        }

        inline void WriteTreeToFile(const AstNode* node, const std::string& filepath) {
            std::ofstream out(filepath);
            PrintTree(node, out);
        }
    } // namespace parsetree
} // namespace zblast
