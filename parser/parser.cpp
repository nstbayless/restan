#include <peglib.h>

#include "parser.h"
#include "pi/expressionTypes.h"

#include <adept.h>
#include <adept_arrays.h>

#include <string>
#include <vector>
#include <map>

using namespace peg;
using namespace restan;

class Statement;
class ExpressionValue;

auto syntax = R"(
        Code <- OptionalData OptionalParameters Model
        OptionalData <-
        OptionalParameters <-
        Model <- 'model' '{' Statements '}'
        ListOfDeclarationOrStatement <- DeclarationOrStatement ';' ListOfDeclarationOrStatement /
        DeclarationOrStatement <- Declaration / Statement

        # Declarations
        Declaration <- DeclarationLHS / DeclarationLHS AssignOp Expression
        DeclarationLHS <- Type Variable / Type '<' BoundsList Variable
        Type <- 'int' / 'real'
        BoundsList <- Bound ',' BoundsList / Bound '>'
        Bound <- BoundType '=' Constant
        BoundType <- 'lower' / 'upper'

        # Statements
        Statements <- Statement ';' Statements /
        Statement <- VariableExpression '~' Distribution '(' ArgList ')' / Variable AssignOp Expression / Variable RelOp Expression / FunctionExpression / '{' Statements '}'
        AssignOp <- '=' / '<-'
        RelOp <- '+=' / '-=' / '*=' / '/='

        # Expressions
        Expression <- ExpressionTerm '+' Expression / ExpressionTerm ExpressionSubtractionSuffixes / ExpressionTerm
        ExpressionSubtractionSuffixes <- ExpressionSubtractionSuffix ExpressionSubtractionSuffixes / ExpressionSubtractionSuffix
        ExpressionSubtractionSuffix <- '-' ExpressionTerm
        ExpressionTerm <- ExpressionFactor '*' ExpressionTerm / ExpressionFactor ExpressionDivisionSuffixes / ExpressionFactor
        ExpressionDivisionSuffixes <- ExpressionDivisionSuffix ExpressionDivisionSuffixes / ExpressionDivisionSuffix
        ExpressionDivisionSuffix <- '/' ExpressionFactor
        ExpressionFactor <- FunctionExpression / '(' Expression ')' / Constant / VariableExpression
        FunctionExpression <- Identifier '(' ArgList ')'
        ArgList <- Expression ',' ArgList / Expression '|' ArgList / Expression /
        Constant <- < [0-9]+ >
        Parameter <- Identifier
        VariableExpression <- Parameter / Variable
        Variable <- Identifier
        Distribution <- Identifier

        # General
        Identifier <- < [a-zA-Z_]+ >
        %whitespace <- [ \t\n\r]* / '//' < [^\n]* >
    )";

std::vector<Expression*> exHeap;
std::vector<Expression*> exArrayHeap;
std::vector<Statement*> stHeap;
std::vector<Statement**> stArrayHeap;

typedef  adept::aMatrix (*fnExpr) ( adept::aMatrix *, unsigned int);

// parses stan code string
void restan::parseStan(std::string stanCode)
{
  // parameter names
  std::vector<std::string> parameterNames;
  std::map<std::string, fnExpr> distributionMap;
  std::map<std::string, fnExpr> functionMap;
  std::map<std::string, unsigned int> AssignMap = {
    {"=", restan::EQUALS},
    {"<-", restan::EQUALS},
    {"+=", restan::PLUSEQUALS},
    {"-=", restan::MINUSEQUALS},
    {"*=", restan::TIMESEQUALS},
    {"/=", restan::DIVEQUALS}
  };

  parameterNames.push_back("target");

  parser p(syntax);
  p["Code"] = [&](const SemanticValues& sv)
  {
    std::cout<<"took root production\n";
    Statement* modelStatement = sv[2].get<Statement*>();
    pi.setLossStatement(modelStatement);
  };
  std::cout<<"parsed syntax\n";
  p["Statements"] = [&](const SemanticValues& sv)
  {
    switch (sv.choice())
    {
      case 0:
      {
        StatementBody* second = sv[1].get<Statement*>();
        Statement* first = sv[0].get<Statement*>();
        if (second)
        {
          Statement** sl = new Statement*[2] {first, second};
          stArrayHeap.push_back(sl);
          StatementBody* sb = new StatementBody(sl, 2);
          stHeap.push_back(sb);
          return sb;
        }
        else
          return first;
      }
      default:
        return static_cast<Statement*>(nullptr);
    }
  }
  p["Statement"] = [&](const SemanticValues& sv)
  {
    switch (sv.choice())
    {
      case 0: // a ~ D
      {
        Expression* varExpr = sv[0].get<Expression*>();
        fnExpr func = sv[1].get<fnExpr>();
        std::vector<Expression*> argVec(sv[2].get());
        argVec.insert(argVec.begin(), varExpr);

        // copy vector into array
        Expression** args = new Expression*[argVec.size()];
        for (int i = 0; i < argVec.size(); i++)
          args[i] = argVec[i];
        exArrayHeap.push_back(args);

        // += statement conversion
        Expression* fn = new ExpressionFunction(func, args, argVec.size());
        exHeap.push_back(fn);
        Statement* s = new StatementAssign(restan::PLUSEQUALS, fn);
        stHeap.push_back(s);
        return s;
      }
      case 1: // assignOp
      case 2: // relOp
      {
        unsigned int variableIndex = sv[0].get<unsigned int>();
        Statement* s = new StatementAssign(assignMap[sv[1].get<std::string>()], variableIndex, sv[2].get<Expression*>());
        stHeap.push_back(s);
        return s;
      }
      case 3: // todo:
    }
  }
  p["AssignOp"] = [&](const SemanticValues& sv)
  {
    return sv.token();
  }
  p["RelOp"] = [&](const SemanticValues& sv)
  {
    return sv.token();
  }
  p["ArgList"] = [&](const SemanticValues& sv)
  {
    // very bad code that passes vectors around :C
    switch (sv.choice())
    {
      case 0:
      case 1:
      {
        std::vector<Expression*> svexp = sv[1].get<std::vector<Expression*>>();
        svexp.insert(svexp.begin(), sv[0].get<Expression*>());
        return svexp;
      }
      case 2:
      {
        std::vector<Expression*> svexp;
        svexp.push_back(sv[0].get<Expression*>());
        return svexp;
      }
      case 3:
        return std::vector<Expression*>();
    }
  }
  p["Distribution"] = [&](const SemanticValues& sv)
  {
    if (distributionMap[sv.token()])
      return distributionMap[sv.token()]
    else
      throw ParseError("Unknown distribution: \"" + sv.token() + "\"");
  }
  p["Expressions"] = [&](const SemanticValues& sv)
  {
    switch (sv.choice())
    {
      case 0: // Definition Definitions
      {
        Expression* exprNext = sv[1].get<Expression*>();
        if (exprNext)
        {
           Expression* expr = new ExpressionArithmetic(restan::PLUS, sv[0].get<Expression*>(), exprNext);
           exHeap.push_back(expr);
           return expr;
        }
        else
           return sv[0].get<Expression*>();
      }
      default: // '}'
        return static_cast<Expression*>(nullptr);
    }
  };

  p["Expression"] = [&](const SemanticValues& sv)
  {
    switch (sv.choice())
    {
      case 0: // +
      {
        Expression* expr = new ExpressionArithmetic(restan::PLUS, sv[0].get<Expression*>(), sv[1].get<Expression*>());
        exHeap.push_back(expr);
        return expr;
      }
      case 1:
      {
        Expression* expr = new ExpressionArithmetic(restan::MINUS, sv[0].get<Expression*>(), sv[1].get<Expression*>());
        exHeap.push_back(expr);
        return expr;
      }
      case 2:
        return sv[0].get<Expression*>();
    }
  };

  p["ExpressionSubtractionSuffixes"] = [&](const SemanticValues& sv)
  {
    switch (sv.choice())
    {
      case 0: // -
      {
        Expression* expr = new ExpressionArithmetic(restan::PLUS, sv[0].get<Expression*>(), sv[1].get<Expression*>());
        exHeap.push_back(expr);
        return expr;
      }
      case 2:
        return sv[0].get<Expression*>();
    }
  };

  p["ExpressionSubtractionSuffix"] = [&](const SemanticValues& sv)
  {
    return sv[0].get<Expression*>();
  };


  p["ExpressionTerm"] = [&](const SemanticValues& sv)
  {
    switch (sv.choice())
    {
      case 0: // *
      {
        Expression* expr = new ExpressionArithmetic(restan::TIMES, sv[0].get<Expression*>(), sv[1].get<Expression*>());
        exHeap.push_back(expr);
        return expr;
      }
      case 1: // /
      {
        Expression* expr = new ExpressionArithmetic(restan::DIV, sv[0].get<Expression*>(), sv[1].get<Expression*>());
        exHeap.push_back(expr);
        return expr;
      }
      case 2:
        return sv[0].get<Expression*>();
    }
  };

  p["ExpressionDivisionSuffixes"] = [&](const SemanticValues& sv)
  {
    switch (sv.choice())
    {
      case 0: // *
      {
        Expression* expr = new ExpressionArithmetic(restan::TIMES, sv[0].get<Expression*>(), sv[1].get<Expression*>());
        exHeap.push_back(expr);
        return expr;
      }
      case 1:
        return sv[0].get<Expression*>();
    }
  };

  p["ExpressionDivisionSuffix"] = [&](const SemanticValues& sv)
  {
    return sv[0].get<Expression*>();
  };

  p["ExpressionFactor"] = [&](const SemanticValues& sv)
  {
    switch (sv.choice())
    {
      case 0:
      case 1:
      case 2:
      case 3:
        return sv[0].get<Expression*>();
    }
  };

  p["Constant"] = [&](const SemanticValues& sv)
  {
    Expression* expr = new ExpressionConstant(stoi(sv.token(), nullptr, 10));
    exHeap.push_back(expr);
    return expr;
  };

  p["Parameter"] = [&](const SemanticValues& sv)
  {
    // lookup and see if parameter previously mentioned
    for (unsigned int i = 0; i < parameterNames.size(); i++)
    {
      if (parameterNames[i] == sv.token())
      {
        return i;
      }
    }

    // add new parameter
    parameterNames.push_back(sv.token());
    return sv.size() - 1;
  };

  p["Variable"] = [&]
  {
    // lookup and see if parameter previously mentioned
    for (unsigned int i = 0; i < parameterNames.size(); i++)
    {
      if (variableNames[i] == sv.token())
      {
        return i;
      }
    }

    // add new variable
    variableNames.push_back(sv.token());
    return sv.size() - 1;
  }

  p["VariableExpression"] = [&]
  {
    Expression* expr;
    switch (sv.choice())
    {
        case 0: // parameter
          expr = new ExpressionParameter(sv.get[0]);
          exHeap.push_back(expr);
          return expr;
        case 1: // variable
          expr = new ExpressionVariable(sv.get[0]);
          exHeap.push_back(expr);
          return expr;
    }
  }

  std::cout<<"set up actions\n";

  p.enable_packrat_parsing();
  Expression* e;
  p.parse(stanCode.c_str(), e);
  pi.setLossExpression(e);

  std::cout<<"parsed\n";
  if (e)
    std::cout << "computed AST successfully\n";
}

// frees parsed expressions
void restan::parseStanCleanup()
{
  for (Expression* expr : exHeap)
    delete(expr);
  for (Expression** expra : exArrayHeap)
    delete(expra);
  for (Statement* st : stHeap)
    delete(st);
  for (Statement** sta : stArrayHeap)
    delete [] sta;
}
