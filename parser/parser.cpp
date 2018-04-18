#include <peglib.h>

#include "parser.h"
#include "pi/expressionTypes.h"

#include <adept.h>
#include <adept_arrays.h>

#include <string>
#include <vector>

using namespace peg;

auto syntax = 
R"(
    # Grammar for Stan...
    Code <- 'model' '{' Expressions
    Expressions <- Expression Expressions / '}'
    Expression <- 'increment_log_prob' '(' Expression ')' / '(' Expression ')' / Constant / Parameter / Expression '+' Expression / Expression '*' Expression / Expression '-' Expression / Expression '/' Expression
    Constant <- < [0-9]+ >
    Parameter <- < [a-zA-Z_]+ >
    %whitespace <- [ \t\n]*
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
    return sv[0].get<Expression*>();
  }
  
  p["Expressions"] = [&](const SemanticValues& sv)
  {
    switch (sv.choice)
    {
      case 0: // Definition Definitions
        Expression* exprNext = sv[1].get<Expression*>();
        if (exprNext)
        {
           Expression* expr = new ExpressionArithmetic(restan::PLUS, sv[0].get<Expression*>(), exprNext);
           exHeap.push_back(expr);
           return expr;
        }
        else
           return sv[0].get<Expression*>();
      default: // '}'
        return nullptr;
    }
  }
  
  p["Expression"] = [&](const SemanticValues& sv)
  {
    switch (sv.choice)
    {
      case 0: // increment_log_prob
      case 1: // ()
        return sv[0].get<Expression*>();
      case 2: // constant
        return sv[0].get<Expression*>();
      case 3: // parameter
        return sv[0].get<Expression*>();
      case 4: // +
        Expression* expr = new ExpressionArithmetic(restan::PLUS, sv[0].get<Expression*>(), sv[1].get<Expression*>());
        exHeap.push_back(expr);
        return expr;
      case 5: // *
        Expression* expr = new ExpressionArithmetic(restan::TIMES, sv[0].get<Expression*>(), sv[1].get<Expression*>());
        exHeap.push_back(expr);
        return expr;
      case 6: // -
        Expression* expr = new ExpressionArithmetic(restan::MINUS, sv[0].get<Expression*>(), sv[1].get<Expression*>());
        exHeap.push_back(expr);
        return expr;
      case 7: // /
        Expression* expr = new ExpressionArithmetic(restan::DIV, sv[0].get<Expression*>(), sv[1].get<Expression*>());
        exHeap.push_back(expr);
        return expr;
      default: // '}'
        return;
    }
  }
  
  p["Constant"] = [&](const SemanticValues& sv)
  {
    Expression* expr = new ExpressionConstant(stoi(sv.token(), nullptr, 10));
    exHeap.push_back(expr);
    return expr;
  }
  
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
    parameterNames.push_back(sv.token())
    Expression* expr = new ExpressionParameter(sv.size() - 1);
    exHeap.push_back(expr);
    return expr;
  }
}

// frees parsed expressions
void restan::parseStanCleanup()
{
  for (Expression* expr : exHeap)
    delete(expr);
}