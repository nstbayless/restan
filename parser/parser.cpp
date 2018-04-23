#include <peglib.h>

#include "parser.h"
#include "pi/expressionTypes.h"
#include "pi/statementTypes.h"
#include "pi/distributions.h"
#include "pi/functions.h"

#include <adept.h>
#include <adept_arrays.h>

#include <string>
#include <vector>
#include <map>
#include <regex>

using namespace peg;
using namespace restan;

typedef  adept::aMatrix (*fnExpr) ( adept::aMatrix *, unsigned int);

namespace restan
{
  class Statement;
}
class ExpressionValue;

std::string trim(std::string s)
{
  while (s.length() > 0 && iswspace(s[0]))
    s = s.substr(1);
  while (s.length() > 0 && iswspace(s[s.length() - 1]))
    s = s.substr(0, s.length() - 1);
  return s;
}

auto syntax = R"(
        Code <- OptionalData OptionalParameters OptionalTransformedParameters Model
        OptionalData <-
        OptionalParameters <- ParametersString '{' ( Declaration ';' )* '}' /
        OptionalTransformedParameters <- 'transformed' ParametersString '{' ( DeclarationOrStatement ';' )* '}' /
        Model <- 'model' Statement
        DeclarationOrStatement <- DeclarationAssignment / Statement

        # string anchors
        ParametersString <- 'parameters'

        # Declarations
        DeclarationAssignment <- Declaration AssignOp Expression / Declaration
        Declaration <- Type Identifier / Type '<' Bound (',' Bound)* '>' Identifier
        Type <- 'int' / 'real' / 'vector' / 'matrix'
        Bound <- BoundType '=' Constant
        BoundType <- 'lower' / 'upper'

        # Statements
        Statements <- Statement ';' Statements /
        Statement <- VariableExpression '~' Distribution '(' Expression (',' Expression)* ')' /
                    Variable AssignOp Expression / Variable RelOp Expression /
                    FunctionExpression / '{' Statements '}'
        AssignOp <- '=' / '<-'
        RelOp <- '+=' / '-=' / '*=' / '/='

        # Expressions
        Expression <- ExpressionTerm (TermOp ExpressionTerm)*
        ExpressionTerm <- ExpressionFactor (FactorOp ExpressionFactor)*
        TermOp <- '+' / '-'
        FactorOp <- '*' / '/'
        ExpressionFactor <- FunctionExpression / '(' Expression ')' / Constant / VariableExpression
        FunctionExpression <- Identifier '(' Expression (',' Expression)* ')' / Identifier '(' ')'
        Constant <- < [0-9]+ '.' [0-9]+ > / < [0-9]+ >
        Parameter <- < [a-zA-Z_]+ >
        Variable <- < [a-zA-Z_]+ >
        VariableExpression <- < [a-zA-Z_]+ >
        Distribution <- < [a-zA-Z_]+ >

        # General
        Identifier <- < [a-zA-Z_]+ >
        %whitespace <- [ \t\n\r]* / '//' < [^\n]* >
    )";

std::vector<restan::Expression*> exHeap;
std::vector<restan::Expression**> exArrayHeap;
std::vector<restan::Statement*> stHeap;
std::vector<restan::Statement**> stArrayHeap;

// variable / parameter names
std::map<std::string, unsigned int> parameterNames;
std::map<std::string, unsigned int> variableNames;
int pCount = 0;
int vCount = 0;

int declaring = false;

// fixed lookups
std::map<std::string, fnExpr> distributionMap = {{"normal", restan::distributions::normal}};
std::map<std::string, fnExpr> functionMap = {{"exp", restan::functions::exp}};

// memoize
void* memoize(const Ast& sv, std::string production, void* proposed, bool dispose = false)
{
  return proposed;
};

void* eval(const Ast& sv) {
  std::cout<<sv.name<<std::endl;
  auto& ast = sv;
  if (ast.name == "Code")
  {
    std::cout<<"took root production\n";
    std::vector<restan::Statement*> topLevelBlocks;
    // clear output Expressions
    // restan::pi.outputExpressions.resize(0);

    // read parameters block
    if (ast.nodes[1]->name != "ParametersString")
    {
      declaring = 1; // parameters
      topLevelBlocks.push_back((restan::Statement*)eval(*ast.nodes[1]));
    }

    // read transformed parameters block
    if (ast.nodes[2]->name != "ParametersString")
    {
      declaring = 2; // variables
      topLevelBlocks.push_back((restan::Statement*)eval(*ast.nodes[2]));
    }

    // read model
    declaring = 0;
    topLevelBlocks.push_back((restan::Statement*)eval(*ast.nodes[3])); // model

    // aggregate top-level blocks
    restan::Statement** lossStatementArray = new restan::Statement*[topLevelBlocks.size()];
    for (int i = 0; i < topLevelBlocks.size(); i++)
      lossStatementArray[i] = topLevelBlocks[i];
    StatementBody* lossStatement = new StatementBody(lossStatementArray, topLevelBlocks.size());
    stArrayHeap.push_back(lossStatementArray);
    stHeap.push_back(lossStatement);

    // setup pi
    pi.setLossStatement(lossStatement);
    pi.numParams = pCount;
    pi.numVariables = vCount;
    pi.discreteIndexStart = pCount;
  	pi.setParams(pi.numParams);
  	pi.setVariables(pi.numVariables);
  };
  if (ast.name == "OptionalTransformedParameters" || ast.name == "OptionalParameters")
  {
    restan::Statement** sl = new restan::Statement*[sv.nodes.size()+1];
    restan::Statement** slo = sl;
    stArrayHeap.push_back(sl);
    for (int i = 1; i < sv.nodes.size(); i ++)
    {
      restan::Statement* s = (restan::Statement*)eval(*ast.nodes[i]);
      if (ast.nodes[i]->name == "Declaration" && declaring != 1)
      {
        delete(s);
        continue;
      }
      if (s)
      {
        *slo = s;
        slo ++;
      }
    }
    restan::Statement* sb = new StatementBody(sl, slo - sl);
    stHeap.push_back(sb);
    return sb;
  };
  if (ast.name == "Model")
  {
    restan::Statement* modelStatement = (restan::Statement*)eval(*ast.nodes[2]);
    return memoize(sv, "Model", (void*)modelStatement);
  };
  if (ast.name == "DeclarationOrStatement")
  {
    return (Expression*)eval(*ast.nodes[0]);
  };
  if (ast.name == "DeclarationAssignment")
  {
    Expression* rhs = (Expression*)eval(*sv.nodes[2]);
    unsigned int* _idx = (unsigned int*)eval(*sv.nodes[0]);
    unsigned int idx = *_idx;
    delete(_idx);
    StatementAssign* sa = new StatementAssign(idx, rhs);
    stHeap.push_back(sa);
    return sa;
  };
  if (ast.name == "Declaration")
  {
    std::string varName = sv.nodes[sv.nodes.size() - 1]->token;
    switch (declaring)
    {
      case 0: // error
        throw ParseError("Cannot declare here");
        break;
      case 1: // parameter
      {
        // find bounds:
        unsigned int pIndex;
        bool useLower = false;
        bool useUpper = false;
        Expression* upper;
        Expression* lower;
        bool useDiscrete = false;
        if (sv.nodes[1]->name == "Bound")
        {
          for (int i = 1; i < sv.nodes.size() - 1; i++)
          {
            auto& bound = *sv.nodes[i];
            std::string boundType = trim(bound.nodes[0]->token);
            Expression* boundValue = (Expression*)eval(*bound.nodes[1]);

            if (boundType == "lower")
            {
              useLower = true;
              lower = boundValue;
            }
            if (boundType == "upper")
            {
              useUpper = true;
              upper = boundValue;
            }
          }
        }

        // determine bounds remap:
        restan::StatementAssign* remapStatement = nullptr;
        restan::Expression* remapExpression;
        if (!useDiscrete)
        {
          if (!useUpper && !useLower)
          {
            // raw parameter
            parameterNames[varName] = pCount;
            remapExpression = new ExpressionParameter(pCount);
          }
          if (useLower && !useUpper)
          {
            // lower-bound
            variableNames[varName] = vCount;
            Expression** exprs = new Expression*[1];
            exprs[0] = new ExpressionParameter(pCount);
            exHeap.push_back(exprs[0]);
            exArrayHeap.push_back(exprs);
            ExpressionFunction* exprFn = new ExpressionFunction(functionMap["exp"], exprs, 1);
            exHeap.push_back(exprFn);
            ExpressionArithmetic* remapExpression = new ExpressionArithmetic(restan::PLUS, lower, exprFn);
            remapStatement = new StatementAssign(vCount, remapExpression);
            vCount++;
          }
          if (!useLower && useUpper)
          {
            // upper-bound
            variableNames[varName] = vCount;
            Expression** exprs = new Expression*[1];
            exprs[0] = new ExpressionParameter(pCount);
            exHeap.push_back(exprs[0]);
            exArrayHeap.push_back(exprs);
            ExpressionFunction* exprFn = new ExpressionFunction(functionMap["exp"], exprs, 1);
            exHeap.push_back(exprFn);
            ExpressionArithmetic* remapExpression = new ExpressionArithmetic(restan::MINUS, upper, exprFn);
            remapStatement = new StatementAssign(vCount, remapExpression);
            vCount++;
          }
          if (useLower && useUpper)
          {
            // log-odds transform
            variableNames[varName] = vCount;
            Expression** exprs = new Expression*[1];
            ExpressionConstant* oneConstant = new ExpressionConstant(1);
            exHeap.push_back(oneConstant);
            exprs[0] = new ExpressionParameter(pCount);
            exHeap.push_back(exprs[0]);
            exArrayHeap.push_back(exprs);
            ExpressionFunction* exprFn = new ExpressionFunction(functionMap["exp"], exprs, 1);
            exHeap.push_back(exprFn);
            ExpressionArithmetic* exprPlusit = new ExpressionArithmetic(restan::PLUS, oneConstant, exprFn);
            exHeap.push_back(exprPlusit);
            ExpressionArithmetic* exprDivit = new ExpressionArithmetic(restan::DIV, exprFn, exprPlusit);
            exHeap.push_back(exprDivit);
            ExpressionArithmetic* exprRange = new ExpressionArithmetic(restan::MINUS, upper, lower);
            exHeap.push_back(exprRange);
            ExpressionArithmetic* exprMult = new ExpressionArithmetic(restan::TIMES, exprRange, exprDivit);
            exHeap.push_back(exprMult);
            ExpressionArithmetic* remapExpression = new ExpressionArithmetic(restan::PLUS, lower, exprMult);
            remapStatement = new StatementAssign(vCount, remapExpression);
            vCount++;
          }
        }
        else
        {
          // TODO: discrete remapping...
        }

        pCount++;
        exHeap.push_back(remapExpression);
        //restan::pi.outputExpressions.pushBack(remapExpression);
        if (remapStatement)
          stHeap.push_back(remapStatement);
        return remapStatement;
      }
      case 2: // variable
        return new unsigned int(variableNames[varName] = vCount++);
    }
  };
  if (ast.name == "Identifier")
  {
    return memoize(sv, "Ident", new std::string(trim(sv.token)), true);
  };
  if (ast.name == "Statements")
  {
    std::cout<< " statements -- " << sv.token << std::endl;
    switch (sv.nodes.size() > 0)
    {
      case true:
      {
        restan::Statement* second = (restan::Statement*)eval(*ast.nodes[1]);
        restan::Statement* first = (restan::Statement*)eval(*ast.nodes[0]);
        if (second)
        {
          restan::Statement** sl = new restan::Statement*[2] {first, second};
          stArrayHeap.push_back(sl);
          restan::Statement* sb = new restan::StatementBody(sl, 2);
          stHeap.push_back(sb);
          return memoize(sv, "Ss1", sb, true);
        }
        else
          return memoize(sv, "Ss2", first, false);
      }
      case false:
        return static_cast<restan::Statement*>(nullptr);
    }
  };
  if (ast.name == "Statement")
  {
    std::cout << "statement -- " << sv.token << std::endl;
    int choice = 4;
    if (sv.nodes.size() > 1)
      if (ast.nodes[1]->name .compare("Distribution") == 0)
        choice = 0;
    if (ast.nodes[1]->name .compare("AssignOp") == 0)
      choice = 1;
    if (ast.nodes[1]->name.compare("RelOp") == 0)
      choice = 2;
    if (ast.nodes[0]->name .compare("FunctionExpression") == 0)
      choice = 3;
    switch (choice)
    {
      case 0: // a ~ D
      {
        Expression* varExpr = (Expression*)eval(*ast.nodes[0]);
        std::string dstHandle(ast.nodes[1]->token);
        fnExpr func = distributionMap[dstHandle];

        // copy vector into array
        Expression** args = new Expression* [sv.nodes.size() - 1];
        for (int i = 2; i < sv.nodes.size(); i++)
          args[i-1] = (Expression*)eval(*sv.nodes[i]);
        args[0] = varExpr;
        exArrayHeap.push_back(args);

        // += statement conversion
        Expression* fn = new ExpressionFunction(func, args, sv.nodes.size()-1);
        exHeap.push_back(fn);
        Expression* targetValue = new ExpressionVariable(variableNames["target"]);
        exHeap.push_back(targetValue);
        Expression* sum = new ExpressionArithmetic(restan::PLUS, targetValue, fn);
        exHeap.push_back(sum);
        restan::Statement* s = new restan::StatementAssign(variableNames["target"], sum);
        stHeap.push_back(s);
        return memoize(sv, "S~", s, true);
      }
      case 1: // assignOp
      case 2: // relOp
      {
        unsigned int* _variableIndex = (unsigned int*)eval(*ast.nodes[0]);
        unsigned int variableIndex = *_variableIndex;
        delete(_variableIndex);
        Expression* rhs = (Expression*)eval(*ast.nodes[2]);
        std::string* _assignOp = (std::string*)eval(*(ast.nodes[1]));
        std::string assignOp = trim(*_assignOp);
        delete(_assignOp);
        if (assignOp.compare("==") != 0 && assignOp.compare("<-") != 0)
        {
          // relative operations
          restan::Operation op;
          if (assignOp.compare("+=") == 0)
          {
            op = restan::PLUS;
          }
          else if (assignOp.compare("-=") == 0)
          {
            op = restan::MINUS;
          }
          else if (assignOp.compare("*=") == 0)
          {
            op = restan::TIMES;
          }
          else if (assignOp.compare("/=") == 0)
          {
            op = restan::DIV;
          }

          Expression* varexpr = new ExpressionVariable(variableIndex);
          exHeap.push_back(varexpr);
          rhs = new ExpressionArithmetic(op, varexpr, rhs);
        }
        restan::Statement* s = new restan::StatementAssign(variableIndex, rhs);
        stHeap.push_back(s);
        return memoize(sv, "Sasgn", s, true);
      }
      case 3: // function
      {
        restan::StatementFunction* fn = new restan::StatementFunction((ExpressionFunction*)eval(*ast.nodes[0]));
        stHeap.push_back(fn);
        return memoize(sv, "Sfn", fn, true);
      }
      case 4: // block
      {
        restan::Statement** list = new restan::Statement*[sv.nodes.size() - 1];
        for (int i = 0; i < sv.nodes.size(); i++)
          list[i] = (restan::Statement*)eval(*ast.nodes[i]);
        restan::Statement* fn = new restan::StatementBody(list, sv.nodes.size() - 1);
        stHeap.push_back(fn);
        return memoize(sv, "S{}", fn, true);
      }
    }
  };
  if (ast.name == "AssignOp")
  {
    return memoize(sv, "AsOp", new std::string(sv.token), true);
  };
  if (ast.name == "RelOp")
  {
    return memoize(sv, "RelOp", new std::string(sv.token), true);
  };
  if (ast.name == "TermOp")
  {
    return memoize(sv, "TOp", new std::string(sv.token), true);
  };
  if (ast.name == "FactorOp")
  {
    return memoize(sv, "FOp", new std::string(sv.token), true);
  };

  if (ast.name == "ArgList")
  {
    // very bad code that passes vectors around :C
  int choice = 4;
  if (ast.nodes.size() > 0)
  {
    if (ast.nodes[0]->name == "Expression" && ast.nodes[1]->name == "ArgList")
    choice = 1;
    else if (ast.nodes[0]->name == "Expression")
    choice = 2;
  }
  else if (ast.nodes.size() == 0)
  {
  choice = 3;
  }

    switch (choice)
    {
      case 0:
      case 1:
      {
        std::vector<Expression*>* _svexp = (std::vector<Expression*>*)eval(*ast.nodes[1]);
        std::vector<Expression*> svexp = *_svexp;
        delete(_svexp);
        svexp.insert(svexp.begin(), (Expression*)eval(*ast.nodes[0]));
        return memoize(sv, "ArglA", new std::vector<Expression*>(svexp), true);
      }
      case 2:
      {
        std::vector<Expression*>* _svexp = (std::vector<Expression*>*)eval(*ast.nodes[1]);
        std::vector<Expression*> svexp = *_svexp;
        delete(_svexp);
        svexp.push_back((Expression*)eval(*ast.nodes[0]));
        return memoize(sv, "ArglB", new std::vector<Expression*>(svexp), true);
      }
      case 3:
        return memoize(sv, "ArglEmpty", new std::vector<Expression*>(), true);
    }
  };

  if (ast.name == "Expression")
  {
    std::cout<< "Expression: " << sv.token << std::endl;
    Expression* expr = (Expression*)eval(*ast.nodes[0]);
    for (int i=1; i<sv.nodes.size(); i+=2)
    {
      if (trim(ast.nodes[i]->token).compare("+") == 0) {
        expr = new ExpressionArithmetic(restan::PLUS, expr, (Expression*) eval(*ast.nodes[i+1]));
        exHeap.push_back(expr);
      } else if (trim(ast.nodes[i]->token).compare("-") == 0) {
        expr = new ExpressionArithmetic(restan::MINUS, expr, (Expression*) eval(*ast.nodes[i+1]));
        exHeap.push_back(expr);
      } else {
        // ??
      }
    }
    return memoize(sv, "Expression", expr);
  };

  if (ast.name == "FunctionExpression")
  {
    std::string* _funcId = (std::string*)eval(*(ast.nodes[0]));
    std::string funcId = *_funcId;
    delete(_funcId);
    if (functionMap.find(funcId) == functionMap.end())
    {
      //throw ParseError(std::string("Did not find functionId at line"));
    }

    fnExpr func = functionMap[funcId];

    // copy vector into array
    Expression** args = new Expression*[sv.nodes.size()];
    for (int i = 0; i < sv.nodes.size(); i++)
      args[i] = (Expression*)eval(*ast.nodes[i]);
    exArrayHeap.push_back(args);

    Expression* fn = new ExpressionFunction(func, args, sv.nodes.size());
    exHeap.push_back(fn);
    return memoize(sv, "FnExpr", fn, true);
  };

  if (ast.name == "ExpressionTerm")
  {
    Expression* expr = (Expression*)eval(*ast.nodes[0]);
    for (int i=1; i<sv.nodes.size(); i+=2)
    {
      if (trim(ast.nodes[i]->token).compare("*") == 0) {
        expr = new ExpressionArithmetic(restan::TIMES, expr, (Expression*) eval(*ast.nodes[i+1]));
        exHeap.push_back(expr);
      } else if (trim(ast.nodes[i]->token).compare("/") == 0) {
        expr = new ExpressionArithmetic(restan::DIV, expr, (Expression*) eval(*ast.nodes[i+1]));
        exHeap.push_back(expr);
      }
    }
    return memoize(sv, "ExprTrm", expr, true);
  };

  if (ast.name == "ExpressionFactor")
  {
    std::cout<< "ExpressionFactor: " << sv.token << std::endl;
      return (Expression*)eval(*ast.nodes[0]);
  };

  if (ast.name == "Constant")
  {
	double constant;
	std::stringstream strConst(sv.token); strConst >> constant;
    Expression* expr = new ExpressionConstant( constant );
    exHeap.push_back(expr);
    return memoize(sv, "Constant", expr, true);
  };

  if (ast.name == "VariableExpression")
  {
    Expression* expr;
    int choice = 1;
    if (parameterNames.count(sv.token) > 0)
      choice = 0;
    switch (choice)
    {
        case 0: // parameter
        {
          Ast* _ast = reinterpret_cast<Ast*>((void*) (&ast));
          *(std::string*)(void*)(&_ast->name) = std::string("Parameter");
          unsigned int * a = (unsigned int*)eval(*_ast);
          unsigned int b = *a;
          delete(a);
          expr = new ExpressionParameter(b);
          exHeap.push_back(expr);
          return memoize(sv, "varExpr (Parameter)", expr, true);
        }
        case 1: // variable
        {
          Ast* _ast = reinterpret_cast<Ast*>((void*)(&ast));
          *(std::string*)(void*)(&_ast->name) = std::string("Variable");
          unsigned int * a = (unsigned int*)eval(*_ast);
          unsigned int b = *a;
          delete(a);
          expr = new ExpressionVariable(b);
          exHeap.push_back(expr);
          return memoize(sv, "varExpr (Variable)", expr, true);
        }
    }
  };

  if (ast.name == "Parameter")
  {
    std::string varName = trim(sv.token);
    if (!parameterNames.count(varName))
    {
      if (declaring == 1)
      {
        parameterNames[varName] = pCount++;
        return eval(sv);
      }
      else throw ParseError("Cannot define parameter " + varName);
    }
    else
      return new unsigned int (parameterNames[varName]);
  };

  if (ast.name == "Variable")
  {
    std::string varName = trim(sv.token);
    if (!variableNames.count(varName))
    {
      if (declaring == 2)
      {
        variableNames[varName] = vCount++;
        return eval(sv);
      }
      else throw ParseError("Cannot define variable " + varName);
    }
    else
      return new unsigned int (variableNames[varName]);
  };
};

// parses stan code string
void restan::parseStan(std::string stanCode)
{
  vCount = 0;
  pCount = 0;
  variableNames["target"] = vCount++;


  parser p(syntax);

  p.enable_ast();

  std::cout<< "about to parse\n";

  //p.enable_packrat_parsing();
  std::shared_ptr<peg::Ast> ast;
  if (p.parse(stanCode.c_str(), ast)) {
      ast = AstOptimizer(true).optimize(ast);
      std::cout << ast_to_s(ast);
      eval(*ast);
      std::cout<<"parsed\n";
  }
  else
  {
    throw ParseError("Failed to parse AST");
  }

}

// frees parsed expressions
void restan::parseStanCleanup()
{
  std::cout<<"cleaning up "<<std::endl;
  for (Expression* expr : exHeap)
    delete(expr);
  for (Expression** expra : exArrayHeap)
    delete(expra);
  for (restan::Statement* st : stHeap)
    delete(st);
  for (restan::Statement** sta : stArrayHeap)
    delete [] sta;
}
