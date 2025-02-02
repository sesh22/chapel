/*
 * Copyright 2004-2016 Cray Inc.
 * Other additional copyright holders may be indicated within.
 *
 * The entirety of this work is licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License.
 *
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//
// The BaseAST class is the parent class of all AST node types.
//
// Notes on adding, removing, or changing AST node types
// -----------------------------------------------------
//
// 1. When adding or removing AST node types, update AstTag and
//    astTagName so that they are consistent.
//
// 2. Update the traversal macros as necessary.  The traversal must be
//    able to touch all AST nodes when traversing the AST.
//

#ifndef _BASEAST_H_
#define _BASEAST_H_

#include <ostream>
#include <string>

#include "map.h"
#include "vec.h"

//
// foreach_ast_sep: invoke a 'macro' for every AST node type,
//                  separating invocations by 'sep'
// foreach_ast: invoke a macro for every AST node type, separating
//              invocations by ;
//
#define foreach_ast_sep(macro, sep)                \
  macro(PrimitiveType) sep                         \
  macro(EnumType) sep                              \
  macro(AggregateType) sep                         \
                                                   \
  macro(ModuleSymbol) sep                          \
  macro(VarSymbol)    sep                          \
  macro(ArgSymbol)    sep                          \
  macro(TypeSymbol)   sep                          \
  macro(FnSymbol)     sep                          \
  macro(EnumSymbol)   sep                          \
  macro(LabelSymbol)  sep                          \
                                                   \
  macro(SymExpr) sep                               \
  macro(UnresolvedSymExpr) sep                     \
  macro(DefExpr) sep                               \
  macro(CallExpr) sep                              \
  macro(ContextCallExpr) sep                       \
  macro(NamedExpr) sep                             \
                                                   \
  macro(UseStmt) sep                               \
  macro(BlockStmt) sep                             \
  macro(CondStmt) sep                              \
  macro(GotoStmt) sep                              \
  macro(ExternBlockStmt)

#define foreach_ast(macro)                         \
  foreach_ast_sep(macro, ;)

class AstVisitor;
class Expr;
class GenRet;
class LcnSymbol;
class Symbol;
class Type;

class BlockStmt;
class LoopStmt;
class WhileStmt;
class WhileDoStmt;
class DoWhileStmt;
class ForLoop;
class CForLoop;
class ParamForLoop;

#define proto_classes(type) class type
foreach_ast(proto_classes);
#undef proto_classes

//
// declare global vectors for all AST node types
//
// These global vectors, named gSymExprs, gCallExprs, gFnSymbols, ...,
// contain all existing nodes of the given AST node type; they are
// updated automatically as new AST nodes are constructed.  Nodes are
// removed from these vectors between passes.
//
#define decl_gvecs(type) extern Vec<type*> g##type##s
foreach_ast(decl_gvecs);
#undef decl_gvecs

//
// type definitions for common maps
//
typedef Map<Symbol*,Symbol*>     SymbolMap;
typedef MapElem<Symbol*,Symbol*> SymbolMapElem;

// how an AST node knows its location in the source code
// (assumed to get copied upon assignment and parameter passing)
class astlocT {
public:
  astlocT(int linenoArg, const char* filenameArg) :
    filename(filenameArg), lineno(linenoArg)
    {}

  const char* filename;  // filename of location
  int         lineno;    // line number of location
};

//
// enumerated type of all AST node types
//
enum AstTag {
  E_SymExpr,
  E_UnresolvedSymExpr,
  E_DefExpr,
  E_CallExpr,
  E_ContextCallExpr,
  E_NamedExpr,
  E_UseStmt,
  E_BlockStmt,
  E_CondStmt,
  E_GotoStmt,
  E_ExternBlockStmt,

  E_ModuleSymbol,
  E_VarSymbol,
  E_ArgSymbol,
  E_TypeSymbol,
  E_FnSymbol,
  E_EnumSymbol,
  E_LabelSymbol,

  E_PrimitiveType,
  E_EnumType,
  E_AggregateType
};

static inline bool isExpr(AstTag tag)
{ return tag >= E_SymExpr        && tag <= E_ExternBlockStmt; }

static inline bool isSymbol(AstTag tag)
{ return tag >= E_ModuleSymbol   && tag <= E_LabelSymbol; }

static inline bool isType(AstTag tag)
{ return tag >= E_PrimitiveType  && tag <= E_AggregateType; }


//
// macros used to define the copy method on all AST node types, and to
// prototype the copyInner method
//
// The copyInner method is used to do the copy specific to any node.
// The outermost call to copy invokes the copyInner method used to
// implement the recursive copy.
//
#define DECLARE_COPY(type)                                              \
  type* copy(SymbolMap* map = NULL, bool internal = false) {            \
    SymbolMap localMap;                                                 \
    if (!map)                                                           \
      map = &localMap;                                                  \
    type* _this = copyInner(map);                                       \
    _this->astloc = astloc;                                             \
    if (!internal)                                                      \
      update_symbols(_this, map);                                       \
    return _this;                                                       \
  }                                                                     \
  virtual type* copyInner(SymbolMap* map)

// This should be expanded verbatim and overloaded, so we don't create a map if
// internal is false.
// copyInner must now copy flags.
#define DECLARE_SYMBOL_COPY(type)                                       \
  type* copy(SymbolMap* map = NULL, bool internal = false) {            \
    SymbolMap localMap;                                                 \
    if (!map)                                                           \
      map = &localMap;                                                  \
    type* _this = copyInner(map);                                       \
    _this->astloc = astloc;                                             \
    map->put(this, _this);                                              \
    if (!internal)                                                      \
      update_symbols(_this, map);                                       \
    return _this;                                                       \
  }                                                                     \
  virtual type* copyInner(SymbolMap* map)

//
// macro used to call copy from inside the copyInner method
//
#define COPY_INT(c) (c ? c->copy(map, true) : NULL)

//
// abstract parent of all AST node types
//
class BaseAST {
public:
  virtual GenRet    codegen()                                          = 0;
  virtual bool      inTree()                                           = 0;
  virtual Type*     typeInfo()                                         = 0;
  virtual void      verify()                                           = 0;
  virtual void      accept(AstVisitor* visitor)                        = 0;

  const char*       fname()                                      const;
  int               linenum()                                    const;
  const char*       stringLoc()                                  const;

  FnSymbol*         getFunction();
  ModuleSymbol*     getModule();
  Type*             getValType();
  Type*             getRefType();

  const char*       astTagAsString()                             const;

  AstTag            astTag;     // BaseAST subclass
  int               id;         // Unique ID
  astlocT           astloc;     // Location of this node in the source code

  void                printTabs(std::ostream *file, unsigned int tabs);
  virtual void        printDocsDescription(const char *doc, std::ostream *file, unsigned int tabs);

  static  const       std::string tabText;

protected:
                    BaseAST(AstTag type);
  virtual          ~BaseAST();

private:
                    BaseAST();

  Type*             getWideRefType();
};

GenRet baseASTCodegen(BaseAST* ast);
GenRet baseASTCodegenInt(int x);
GenRet baseASTCodegenString(const char* str);

// get the current AST node id
int    lastNodeIDUsed();

// trace various AST node removals
void   trace_remove(BaseAST* ast, char flag);


//
// macro to update the global line number used to set the line number
// of an AST node when it is constructed - or to print out the line
// number of code related to a core dump.
//
// This should be used before constructing new nodes to make sure the
// line number is correctly set. The global line number reverts to
// its previous value upon leaving the scope where the macro is used.
// The fixed variable name ensures a single macro per scope.
// Users of the macro are to create additional scopes when needed.
// todo - should we add it to DECLARE_COPY/DECLARE_SYMBOL_COPY ?
//
#define SET_LINENO(ast) astlocMarker markAstLoc(ast->astloc)

extern astlocT currentAstLoc;

class astlocMarker {
public:
  astlocMarker(astlocT newAstLoc);
  astlocMarker(int lineno, const char* filename);
  ~astlocMarker();

  astlocT previousAstLoc;
};

//
// vectors of modules
//
extern Vec<ModuleSymbol*> allModules;  // contains all modules
extern Vec<ModuleSymbol*> userModules; // contains user modules

//
// class test inlines: determine the dynamic type of a BaseAST*
//
static inline bool isExpr(const BaseAST* a)
{ return a && isExpr(a->astTag); }

static inline bool isSymbol(const BaseAST* a)
{ return a && isSymbol(a->astTag); }

static inline bool isType(const BaseAST* a)
{ return a && isType(a->astTag); }

static inline bool isLcnSymbol(const BaseAST* a)
{ return a && (a->astTag == E_ArgSymbol || a->astTag == E_VarSymbol); }

static inline bool isCallExpr(const BaseAST* a)
{ return a && (a->astTag == E_CallExpr || a->astTag == E_ContextCallExpr); }


#define def_is_ast(Type)                          \
  static inline bool is##Type(const BaseAST* a)   \
  {                                               \
    return a && a->astTag == E_##Type;            \
  }

def_is_ast(SymExpr)
def_is_ast(UnresolvedSymExpr)
def_is_ast(DefExpr)
def_is_ast(ContextCallExpr)
def_is_ast(NamedExpr)
def_is_ast(UseStmt)
def_is_ast(BlockStmt)
def_is_ast(CondStmt)
def_is_ast(GotoStmt)
def_is_ast(ExternBlockStmt)
def_is_ast(ModuleSymbol)
def_is_ast(VarSymbol)
def_is_ast(ArgSymbol)
def_is_ast(TypeSymbol)
def_is_ast(FnSymbol)
def_is_ast(EnumSymbol)
def_is_ast(LabelSymbol)
def_is_ast(PrimitiveType)
def_is_ast(EnumType)
def_is_ast(AggregateType)
#undef def_is_ast

bool isLoopStmt(const BaseAST* a);
bool isWhileStmt(const BaseAST* a);
bool isWhileDoStmt(const BaseAST* a);
bool isDoWhileStmt(const BaseAST* a);
bool isParamForLoop(const BaseAST* a);
bool isForLoop(const BaseAST* a);
bool isCoforallLoop(const BaseAST* a);
bool isCForLoop(const BaseAST* a);

//
// safe downcast inlines: downcast BaseAST*, Expr*, Symbol*, or Type*
//   note: toDerivedClass is equivalent to dynamic_cast<DerivedClass*>
//
#define def_to_ast(Type) \
  static inline Type * to##Type(BaseAST* a) { return is##Type(a) ? (Type*)a : NULL; } \
  static inline const Type * toConst##Type(const BaseAST* a) \
    { return is##Type(a) ? (const Type*)a : NULL; }

def_to_ast(SymExpr)
def_to_ast(UnresolvedSymExpr)
def_to_ast(DefExpr)
def_to_ast(ContextCallExpr)
def_to_ast(NamedExpr)
def_to_ast(UseStmt)
def_to_ast(BlockStmt)
def_to_ast(CondStmt)
def_to_ast(GotoStmt)
def_to_ast(ExternBlockStmt)
def_to_ast(Expr)
def_to_ast(ModuleSymbol)
def_to_ast(VarSymbol)
def_to_ast(ArgSymbol)
def_to_ast(TypeSymbol)
def_to_ast(FnSymbol)
def_to_ast(EnumSymbol)
def_to_ast(LabelSymbol)
def_to_ast(Symbol)
def_to_ast(PrimitiveType)
def_to_ast(EnumType)
def_to_ast(AggregateType)
def_to_ast(Type)

def_to_ast(LoopStmt);
def_to_ast(WhileStmt);
def_to_ast(WhileDoStmt);
def_to_ast(DoWhileStmt);
def_to_ast(ForLoop);
def_to_ast(CForLoop);
def_to_ast(ParamForLoop);

#undef def_to_ast

static inline LcnSymbol* toLcnSymbol(BaseAST* a)
{
  return isLcnSymbol(a) ? (LcnSymbol*) a : NULL;
}

static inline const LcnSymbol* toConstLcnSymbol(const BaseAST* a)
{
  return isLcnSymbol(a) ? (const LcnSymbol*) a : NULL;
}

CallExpr* getDesignatedCall(const ContextCallExpr* a);

static inline CallExpr* toCallExpr(BaseAST* a)
{
  if (!a) return NULL;
  if (a->astTag == E_CallExpr) return (CallExpr*) a;
  if (a->astTag == E_ContextCallExpr) return getDesignatedCall((ContextCallExpr*)a);
  return NULL;
}

static inline const CallExpr* toConstCallExpr(const BaseAST* a)
{
  if (!a) return NULL;
  if (a->astTag == E_CallExpr) return (const CallExpr*) a;
  if (a->astTag == E_ContextCallExpr) return getDesignatedCall((const ContextCallExpr*)a);
  return NULL;
}


//
// traversal macros
//
// These macros are used to implement the functions that collect all
// of the nodes in some part of the AST, e.g., collectSymExprs.  They
// can also be used to define recursive functions that work over a
// portion of the AST.  See collectSymExprs for a simple example.
//
#define AST_CALL_CHILD(_a, _t, _m, call, ...)                           \
  if (((_t*)_a)->_m) {                                                  \
    BaseAST* next_ast = ((_t*)_a)->_m;                                  \
    call(next_ast, __VA_ARGS__);                                        \
  }

#define AST_CALL_LIST(_a, _t, _m, call, ...)                            \
  for_alist(next_ast, ((_t*)_a)->_m) {                                  \
    call(next_ast, __VA_ARGS__);                                        \
  }

#define AST_CHILDREN_CALL(_a, call, ...)                                \
  switch (_a->astTag) {                                                 \
  case E_CallExpr:                                                      \
    AST_CALL_CHILD(_a, CallExpr, baseExpr, call, __VA_ARGS__);          \
    AST_CALL_LIST(_a, CallExpr, argList, call, __VA_ARGS__);            \
    break;                                                              \
  case E_ContextCallExpr:                                               \
    AST_CALL_LIST(_a, ContextCallExpr, options, call, __VA_ARGS__);     \
    break;                                                              \
  case E_NamedExpr:                                                     \
    AST_CALL_CHILD(_a, NamedExpr, actual, call, __VA_ARGS__);           \
    break;                                                              \
  case E_DefExpr:                                                       \
    AST_CALL_CHILD(_a, DefExpr, init, call, __VA_ARGS__);               \
    AST_CALL_CHILD(_a, DefExpr, exprType, call, __VA_ARGS__);           \
    AST_CALL_CHILD(_a, DefExpr, sym, call, __VA_ARGS__);                \
    break;                                                              \
  case E_UseStmt:                                                       \
    AST_CALL_CHILD(_a, UseStmt, mod, call, __VA_ARGS__);                \
    break;                                                              \
                                                                               \
  case E_BlockStmt: {                                                          \
    BlockStmt* stmt = toBlockStmt(_a);                                         \
                                                                               \
    if (stmt->isWhileDoStmt() == true) {                                       \
      AST_CALL_LIST (stmt, WhileStmt,    body,           call, __VA_ARGS__);   \
      AST_CALL_CHILD(stmt, WhileStmt,    condExprGet(),  call, __VA_ARGS__);   \
                                                                               \
    } else if (stmt->isDoWhileStmt()  == true) {                               \
      AST_CALL_LIST (stmt, WhileStmt,    body,           call, __VA_ARGS__);   \
      AST_CALL_CHILD(stmt, WhileStmt,    condExprGet(),  call, __VA_ARGS__);   \
                                                                               \
    } else if (stmt->isForLoop()      == true) {                               \
      AST_CALL_LIST (stmt, ForLoop,      body,           call, __VA_ARGS__);   \
      AST_CALL_CHILD(stmt, ForLoop,      indexGet(),     call, __VA_ARGS__);   \
      AST_CALL_CHILD(stmt, ForLoop,      iteratorGet(),  call, __VA_ARGS__);   \
                                                                               \
    } else if (stmt->isCoforallLoop() == true) {                               \
      AST_CALL_LIST (stmt, ForLoop,      body,           call, __VA_ARGS__);   \
      AST_CALL_CHILD(stmt, ForLoop,      indexGet(),     call, __VA_ARGS__);   \
      AST_CALL_CHILD(stmt, ForLoop,      iteratorGet(),  call, __VA_ARGS__);   \
                                                                               \
    } else if (stmt->isCForLoop()     == true) {                               \
      AST_CALL_LIST (stmt, CForLoop,     body,           call, __VA_ARGS__);   \
      AST_CALL_CHILD(stmt, CForLoop,     initBlockGet(), call, __VA_ARGS__);   \
      AST_CALL_CHILD(stmt, CForLoop,     testBlockGet(), call, __VA_ARGS__);   \
      AST_CALL_CHILD(stmt, CForLoop,     incrBlockGet(), call, __VA_ARGS__);   \
                                                                               \
    } else if (stmt->isParamForLoop() == true) {                               \
      AST_CALL_LIST (stmt, ParamForLoop, body,           call, __VA_ARGS__);   \
      AST_CALL_CHILD(stmt, ParamForLoop, resolveInfo(),  call, __VA_ARGS__);   \
                                                                               \
    } else  {                                                                  \
      AST_CALL_LIST (stmt, BlockStmt,    body,           call, __VA_ARGS__);   \
      AST_CALL_CHILD(stmt, BlockStmt,    blockInfoGet(), call, __VA_ARGS__);   \
      AST_CALL_CHILD(stmt, BlockStmt,    modUses,        call, __VA_ARGS__);   \
      AST_CALL_CHILD(stmt, BlockStmt,    byrefVars,      call, __VA_ARGS__);   \
    }                                                                          \
    break;                                                                     \
  }                                                                            \
                                                                               \
  case E_CondStmt:                                                      \
    AST_CALL_CHILD(_a, CondStmt, condExpr, call, __VA_ARGS__);          \
    AST_CALL_CHILD(_a, CondStmt, thenStmt, call, __VA_ARGS__);          \
    AST_CALL_CHILD(_a, CondStmt, elseStmt, call, __VA_ARGS__);          \
    break;                                                              \
  case E_GotoStmt:                                                      \
    AST_CALL_CHILD(_a, GotoStmt, label, call, __VA_ARGS__);             \
    break;                                                              \
  case E_ModuleSymbol:                                                  \
    AST_CALL_CHILD(_a, ModuleSymbol, block, call, __VA_ARGS__);         \
    break;                                                              \
  case E_ArgSymbol:                                                     \
    AST_CALL_CHILD(_a, ArgSymbol, typeExpr, call, __VA_ARGS__);         \
    AST_CALL_CHILD(_a, ArgSymbol, defaultExpr, call, __VA_ARGS__);      \
    AST_CALL_CHILD(_a, ArgSymbol, variableExpr, call, __VA_ARGS__);     \
    break;                                                              \
  case E_TypeSymbol:                                                    \
    AST_CALL_CHILD(_a, Symbol, type, call, __VA_ARGS__);                \
    break;                                                              \
  case E_FnSymbol:                                                      \
    AST_CALL_LIST(_a, FnSymbol, formals, call, __VA_ARGS__);            \
    AST_CALL_CHILD(_a, FnSymbol, body, call, __VA_ARGS__);              \
    AST_CALL_CHILD(_a, FnSymbol, where, call, __VA_ARGS__);             \
    AST_CALL_CHILD(_a, FnSymbol, retExprType, call, __VA_ARGS__);       \
    break;                                                              \
  case E_EnumType:                                                      \
    AST_CALL_LIST(_a, EnumType, constants, call, __VA_ARGS__);          \
    break;                                                              \
  case E_AggregateType:                                                 \
    AST_CALL_LIST(_a, AggregateType, fields, call, __VA_ARGS__);        \
    AST_CALL_LIST(_a, AggregateType, inherits, call, __VA_ARGS__);      \
    break;                                                              \
  default:                                                              \
    break;                                                              \
  }

//
// clean IR between passes by clearing some back pointers to dead AST
// nodes and removing dead AST nodes from the global vectors of AST
// nodes. "dead" means !isAlive && !isRootModule.
//
void cleanAst(void);

//
// reclaim memory associated with all AST nodes (called at the end)
//
void destroyAst(void);

//
// print memory-related statistics about the IR (called between passes
// if using --print-statistics)
//
void printStatistics(const char* pass);

void registerModule(ModuleSymbol* mod);

//
// update_symbols: substitute symbol uses in 'ast' using 'map'
//
// This function is primarily used when AST::copy is called.  If a
// Block containing a def and a use of that def is copied, then the
// def will be copied, and this routine will update the use to the new
// def.
//
void update_symbols(BaseAST* ast, SymbolMap* map);

#endif
