# ZBLAST
### An abstract syntax tree generator for C++

*A play on my username, and AST*

The syntax is similar to JastAdd, but contains some cool extras.

[![Tests](https://github.com/ZilverBlade/ZBlast/actions/workflows/cmake-run-tests.yml/badge.svg)](https://github.com/ZilverBlade/ZBlast/actions/workflows/cmake-run-tests.yml)

[![Package](https://github.com/ZilverBlade/ZBlast/actions/workflows/cmake-package.yml/badge.svg)](https://github.com/ZilverBlade/ZBlast/actions/workflows/cmake-package.yml)

### Syntax

Similar to JastAdd, it is based off of a BNF syntax.
There are some slight differences, for instance, `A | B`  is supported to have variant members.

```zast
abstract Stmt; // Define an abstract node;
Program ::= FuncDecl; // Define a basic program node, who's member is "FuncDecl"

FuncDecl ::= Stmt*; // Define a list of Stmt 
IfStmt : Stmt ::= True:Stmt Else:Stmt; // Extend Stmt, and label the two members True and False
LetStmt : Stmt ::= <ID> [Value:<INT>]; // Hold a string named "ID" and an optional string named "Value"
ConstStmt : Stmt ::= <ID> Value:<INT>; 
NewStmt : Stmt ::= LetStmt | ConstStmt; // A variant between LetStmt and ConstStmt.

```

### Running

You can integrate ZBlast in a cmake project, as it is a command line application that can generate c++ files.

- Generate the AST files: `zblast -a <source> <target>`
  - Creates `<target>.hpp` and `<target>.cpp`
  
- Generate visitor files: `zblast -v <source> <target>`
  - Creates `<target>.hpp` and `<target>.cpp` 
  > Note: required if you want to use the AstNode::accept(AstVisitor* visitor, void* data); function.