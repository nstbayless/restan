#include <peglib.h>

#include "parser.h"
#include "pi/expressionTypes.h"

#include <adept.h>
#include <adept_arrays.h>

#include <string>
#include <vector>

using namespace peg;
using namespace restan;

auto syntax = R"(
        Code <- 'data' '{' ListOfDeclarationOrStatement 'parameters' '{' ListOfDeclarationOrStatement 'transformed' 'parameters' '{' ListOfDeclarationOrStatement 'model' '{' Statements
        
        ListOfDeclarationOrStatement <- DeclarationOrStatement ';' ListOfDeclarationOrStatement / '}'
        DeclarationOrStatement <- Declaration / Statement
        
        # Declarations
        Declaration <- DeclarationLHS / DeclarationLHS AssignOp Expression
        DeclarationLHS <- Type Variable / Type '<' BoundsList Variable
        Type <- 'int' / 'real'
        BoundsList <- Bound ',' BoundsList / Bound '>'
        Bound <- BoundType '=' Constant
        BoundType <- 'lower' / 'upper'
        
        # Statements
        Statements <- Statement ';' Statements / '}'
        Statement <- Variable '~' Distribution '(' ArgList / Variable AssignOp Expression / Variable RelOp Expression / FunctionExpression
        AssignOp <- '=' / '<-'
        RelOp <- '+=' / '-=' / '*=' / '/=' 
        
        # Expressions
        Expression <- ExpressionTerm '+' Expression / ExpressionTerm ExpressionSubtractionSuffixes / ExpressionTerm
        ExpressionSubtractionSuffixes <- ExpressionSubtractionSuffix ExpressionSubtractionSuffixes / ExpressionSubtractionSuffix
        ExpressionSubtractionSuffix <- '-' ExpressionTerm
        ExpressionTerm <- ExpressionFactor '*' ExpressionTerm / ExpressionFactor ExpressionDivisionSuffixes / ExpressionFactor
        ExpressionDivisionSuffixes <- ExpressionDivisionSuffix ExpressionDivisionSuffixes / ExpressionDivisionSuffix
        ExpressionDivisionSuffix <- '/' ExpressionFactor
        ExpressionFactor <- FunctionExpression / '(' Expression ')' / Constant / Variable
        FunctionExpression <- Identifier '(' ArgList
        ArgList <- Expression ',' ArgList / Expression ')' / ')'
        Constant <- < [0-9]+ >
        Variable <- Identifier
        Distribution <- Identifier
        
        # General
        Identifier <- < [a-zA-Z_]+ >
        %whitespace <- [ \t\n\r]*
    )";

std::vector<Expression*> exHeap;

// parses stan code string
void restan::parseStan(std::string stanCode)
{
  // parameter names
  std::vector<std::string> parameterNames;


  parser p(syntax);
  p["Code"] = [&](const SemanticValues& sv)
  {
    std::cout<<"took root production\n";
    return sv[0].get<Expression*>();
  };
  std::cout<<"parsed syntax\n";
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
    // lookup and see if paramater previously mentioned
    for (unsigned int i = 0; i < parameterNames.size(); i++)
    {
      if (parameterNames[i] == sv.token())
      {
        Expression* expr = new ExpressionParameter(i);
        exHeap.push_back(expr);
        return expr;
      }
    }
    
    // add new parameter
    parameterNames.push_back(sv.token());
    Expression* expr = new ExpressionParameter(sv.size() - 1);
    exHeap.push_back(expr);
    return expr;
  };
  
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
}