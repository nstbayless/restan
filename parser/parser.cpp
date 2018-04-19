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
        OptionalParameters <- 'parameters' '{' ( Declaration ';' )* '}' /
        OptionalTransformedParameters <- 'parameters' '{' ( DeclarationOrStatement ';' )* '}' /
        Model <- 'model' Statement
        DeclarationOrStatement <- Declaration / Statement

        # Declarations
        Declaration <- DeclarationLHS / DeclarationLHS AssignOp Expression
        DeclarationLHS <- Type Identifier / Type '<' BoundsList Identifier
        Type <- 'int' / 'real' / 'vector' / 'matrix'
        BoundsList <- Bound ',' BoundsList / Bound '>'
        Bound <- BoundType '=' Constant
        BoundType <- 'lower' / 'upper'

        # Statements
        Statements <- Statement ';' Statements /
        Statement <- VariableExpression '~' Distribution '(' ArgList ')' / Variable AssignOp Expression / Variable RelOp Expression / FunctionExpression / '{' Statements '}' #V
        AssignOp <- '=' / '<-'
        RelOp <- '+=' / '-=' / '*=' / '/='

        # Expressions
        Expression <- ExpressionTerm (TermOp ExpressionTerm)*
        ExpressionTerm <- ExpressionFactor (FactorOp ExpressionFactor)*
        TermOp <- '+' / '-'
        FactorOp <- '*' / '/'
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
  // variable / parameter names
  std::map<std::string, unsigned int> parameterNames;
  unsigned int parameterNameCount = 0;
  std::map<std::string, unsigned int> variableNames;
  unsigned int variableNameCount = 0;

  // fixed lookups
  std::map<std::string, fnExpr> distributionMap;
  std::map<std::string, fnExpr> functionMap;

  // currently declaring variables or parameters?
  bool declaringVariables = true;

  variableNames["target"] = variableNameCount++;

  parser p(syntax);
  p["Code"] = [&](const SemanticValues& sv)
  {
    std::cout<<"took root production\n";
    Statement* modelStatement = sv[2].get<Statement*>();
    auto tppair = sv[1].get<std::pair<Statement**, int>>();
    tppair.first[tppair.second] = modelStatement;
    Statement* lossStatement = new StatementBody(tppair.first, tppair.second+1);
    stHeap.push_back(lossStatement);
    pi.setLossStatement(lossStatement);
  };
  std::cout<<"parsed syntax\n";
  p["OptionalParameters"] = [&](const SemanticValues& sv)
  {
    declaringVariables = false;
    for (int i = 0; i < sv.size(); i ++)
      sv[i].get<Statement*>(); // trigger;
  };
  p["OptionalTransformedParameters"] = [&](const SemanticValues& sv)
  {
    declaringVariables = true;
    Statement** sl = new Statement*[sv.size()+1];
    Statement** slo = sl;
    stArrayHeap.push_back(sl);
    for (int i = 0; i < sv.size(); i ++)
    {
      Statement* s = sv[i].get<Statement*>();
      if (s)
      {
        *slo = s;
        slo ++;
      }
    }
    return std::pair<Statement**, int>(sl, slo - sl);
  };
  p["Model"] = [&](const SemanticValues& sv)
  {
    Statement* modelStatement = sv[2].get<Statement*>();
    return modelStatement;
  };
  p["DeclarationOrStatement"] = [&](const SemanticValues& sv)
  {
    return sv[0].get<Expression*>();
  }
  p["Declaration"] = [&](const SemanticValues& sv)
  {
    if (sv.choice == 1)
    {
      case 1: // type f = blah
        if (!declaringVariables)
        {
          throw ParseError("Attempted to assign to a parameter");
        }
        Statement* declareAssign = new statementAssign(sv[0].get<unsigned int>(), sv[2].get<Expression*>());
        exHeap.push_back(declareAssign);
        return declareAssign;
    }
    else
    {
      sv[0].get(); // trigger
      return static_cast<Statement*>(nullptr);
    }
  };
  p["DeclarationLHS"] = [&](const SemanticValues& sv)
  {
    std::string varName = sv[sv.size() - 1].get<std::string>();
    if (variableNames.count(varName) > 0 || parameterNames.count(varName) > 0)
      throw ParseException(std::string("Attempted to redefine variable or parameter ") + varName);
    if (declaringVariables)
    {
      // variable
      return variableNames[varname] = variableNamesCount++;
    }
    else
    {
      // parameter
      return parameterNames[varname] = parameterNamesCount++;
    }
  };
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
        Expression* targetValue = new ExpressionVariable(variableNames["target"])
        exHeap.push_back(targetValue);
        Expression* sum = new ExpressionArithmetic(restan::PLUS, targetValue, fn);
        exHeap.push_back(sum);
        Statement* s = new StatementAssign(variableNames["target"], sum);
        stHeap.push_back(s);
        return s;
      }
      case 1: // assignOp
      case 2: // relOp
      {
        unsigned int variableIndex = sv[0].get<unsigned int>();
        Expression* rhs = sv[2].get<Expression*>();
        std::string assignOp = sv[1].get<std::string>();
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
        Statement* s = new StatementAssign(variableIndex, rhs);
        stHeap.push_back(s);
        return s;
      }
      case 3: // function
      {
        StatementFunction* fn = new StatementFunction(sv[0].get<Expression*>);
        stHeap.push_back(fn);
        return fn;
      }
      case 4: // block
      {
        Statement** list = new Statement*[sv.size()];
        for (int i = 0; i < sv.size(); i++)
          list[i] = sv[i].get<Statement*>();
        StatementFunction* fn = new StatementBlock(list, sv.size());
      }
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
      throw ParseError(std::string("Unknown distribution \"") + sv.token() + "\"");
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
    throw ParseError(std::string("Unknown parameter or variable \"") + sv.token() + "\"");
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
    throw ParseError(std::string("Unknown variable \"") + sv.token() + "\"");
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
