#include <peglib.h>

#include "parser.h"
#include "pi/expressionTypes.h"
#include "pi/statementTypes.h"

#include <adept.h>
#include <adept_arrays.h>

#include <string>
#include <vector>
#include <map>
#include <regex>

using namespace peg;
using namespace restan;

class Statement;
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
        Code <- '__BEGIN_STAN_CODE__' OptionalData OptionalParameters Model '__END_STAN_CODE__'
        OptionalData <-
        OptionalParameters <- 'parameters' '{' ( ParameterDeclaration ';' )* '}' /
        OptionalTransformedParameters <- 'parameters' '{' ( DeclarationOrStatement ';' )* '}' /
        Model <- 'model' Statement
        DeclarationOrStatement <- VariableDeclaration / Statement

        # Declarations
        VariableDeclaration <- DeclarationLHS / DeclarationLHS AssignOp Expression
        ParameterDeclaration <- Type Identifier ### / Type '<' BoundsList Identifier
        DeclarationLHS <- Type Identifier ### / Type '<' BoundsList Identifier
        Type <- 'int' / 'real' / 'vector' / 'matrix'
        BoundsList <- Bound ',' BoundsList / Bound '>'
        Bound <- BoundType '=' Constant
        BoundType <- 'lower' / 'upper'

        # Statements
        Statements <- Statement ';' Statements /
        Statement <- VariableExpression '~' Distribution '(' ArgList ')' /
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

std::vector<restan::Expression*> exHeap;
std::vector<restan::Expression**> exArrayHeap;
std::vector<restan::Statement*> stHeap;
std::vector<restan::Statement**> stArrayHeap;

typedef  adept::aMatrix (*fnExpr) ( adept::aMatrix *, unsigned int);

void fillNames(std::string& stanCode, std::map<std::string, unsigned int>& parameterNames, int& pcount, std::map<std::string, unsigned int>& variableNames, int& vcount)
{
  std::regex varBlockRegex("parameters\\s*\\{([^\\}])*\\}");
  std::regex varNamesRegex("(real|int|vector|matrix)\\s*(<[^>]*>)?\\s*([a-zA-Z0-9_]+)");
  std::match_results<std::string::const_iterator> cdmatch;
  std::string::difference_type searchVarIndex = 0;
  if (std::regex_search(stanCode, cdmatch, varBlockRegex))
  {
    std::match_results<std::string::const_iterator> matchParams;
    searchVarIndex = cdmatch.position() + cdmatch.length();

    auto searchIndex = cdmatch.position(1);

    while (true)
    {
	  std::string strFind = stanCode.substr(searchIndex, searchVarIndex - searchIndex);
      if (std::regex_search(strFind, matchParams, varNamesRegex))
      {
        std::string param = matchParams.str(2);
        parameterNames[param] = pcount++;
        searchVarIndex = matchParams.position(matchParams.size());
      }
      else
        break;
    }

  }

  std::match_results<std::string::const_iterator> matchVars;
  while (true)
  {
	std::string strFind = stanCode.substr(searchVarIndex);
    if (std::regex_search(strFind, matchVars, varNamesRegex))
    {
      std::string param = matchVars.str(2);
      parameterNames[param] = vcount++;
      searchVarIndex = matchVars.position(matchVars.size());
    }
    else
      break;
  }
}

// parses stan code string
void restan::parseStan(std::string stanCode)
{
  // variable / parameter names
  std::map<std::string, unsigned int> parameterNames;
  std::map<std::string, unsigned int> variableNames;
  int pCount = 0;
  int vCount = 0;

  variableNames["target"] = vCount++;
  fillNames(stanCode, parameterNames, pCount, variableNames, vCount);

  // fixed lookups
  std::map<std::string, fnExpr> distributionMap;
  std::map<std::string, fnExpr> functionMap;

  // memoize
  std::map<std::string, void*> memoizedPointers;
  auto memoize = [&](const Ast& sv, std::string production, void* proposed, bool dispose = false) -> void*
  {
    auto lookup = std::string(production) + " >~!~< token-(\"" + sv.token + std::string("\") ~~&>@ using choice(): ") + std::to_string((sv.choice()));
    std::cout << lookup << " (proposed pointer:" << proposed << ")" << std::endl;
    if (memoizedPointers.count(lookup) > 0)
    {
      // use pre-existing
      if (dispose)
        delete (char*)proposed;
      std::cout << "  found in cache!" << std::endl;
      return memoizedPointers[lookup];
    }
    else
    {
      // memoize and return
      memoizedPointers[lookup] = proposed;
      std::cout << "  memoized for future use." << std::endl;
      return proposed;
    }
  };

  

  parser p(syntax);



  p["Code"] = [&](const Ast& sv)
  {
    std::cout<<"took root production\n";
    Statement* modelStatement = sv[2].get<Statement*>();
    auto tppair = sv[1].get<std::pair<Statement**, int> >();
    tppair.first[tppair.second] = modelStatement;
    Statement* lossStatement = new StatementBody(tppair.first, tppair.second+1);
    stHeap.push_back(lossStatement);
    pi.setLossStatement(lossStatement);
  };
  std::cout<<"parsed syntax\n";
  p["OptionalParameters"] = [&](const Ast& sv)
  {
    for (int i = 0; i < sv.size(); i ++)
      sv[i].get<Statement*>(); // trigger;
  };
  p["OptionalTransformedParameters"] = [&](const Ast& sv)
  {
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
  p["Model"] = [&](const Ast& sv) -> void*
  {
    Statement* modelStatement = sv[2].get<Statement*>();
    return memoize(sv, "Model", (void*)modelStatement);
  };
  p["DeclarationOrStatement"] = [&](const Ast& sv) -> void*
  {
    return sv[0].get<Expression*>();
  };
  p["VariableDeclaration"] = [&](const Ast& sv) -> void*
  {
    if (sv.choice() == 1)
    {
      // type f = blah
      Statement* declareAssign = new StatementAssign(sv[0].get<unsigned int>(), sv[2].get<Expression*>());
      stHeap.push_back(declareAssign);
      return (Statement*) memoize(sv, "VD", declareAssign, true);
    }
    else
    {
      sv[0].get<Statement*>(); // trigger
      return static_cast<Statement*>(nullptr);
    }
  };
  p["DeclarationLHS"] = [&](const Ast& sv) -> unsigned int
  {
    std::string varName = trim(sv[sv.size() - 1].get<std::string>());
    return variableNames[trim(varName)];
  };
  p["Identifier"] = [&](const Ast& sv) -> void*
  {
    return memoize(sv, "Ident", new std::string(trim(sv.token)), true);
  };
  p["Statements"] = [&](const Ast& sv) -> void*
  {
    std::cout<< " statements -- " << sv.token << std::endl;
    switch (sv.choice())
    {
      case 0:
      {
        Statement* second = sv[1].get<Statement*>();
        Statement* first = sv[0].get<Statement*>();
        if (second)
        {
          Statement** sl = new Statement*[2] {first, second};
          stArrayHeap.push_back(sl);
          Statement* sb = new StatementBody(sl, 2);
          stHeap.push_back(sb);
          return memoize(sv, "Ss1", sb, true);
        }
        else
          return memoize(sv, "Ss2", first, false);
      }
      default:
        return static_cast<Statement*>(nullptr);
    }
  };
  p["Statement"] = [&](const Ast& sv) -> void*
  {
    std::cout << "statement -- " << sv.token << std::endl;
    switch (sv.choice())
    {
      case 0: // a ~ D
      {
        Expression* varExpr = sv[0].get<Expression*>();
        fnExpr func = sv[1].get<fnExpr>();
        std::vector<Expression*> argVec(sv[2].get<std::vector<Expression*> >());
        argVec.insert(argVec.begin(), varExpr);

        // copy vector into array
        Expression** args = new Expression*[argVec.size()];
        for (int i = 0; i < argVec.size(); i++)
          args[i] = argVec[i];
        exArrayHeap.push_back(args);

        // += statement conversion
        Expression* fn = new ExpressionFunction(func, args, argVec.size());
        exHeap.push_back(fn);
        Expression* targetValue = new ExpressionVariable(variableNames["target"]);
        exHeap.push_back(targetValue);
        Expression* sum = new ExpressionArithmetic(restan::PLUS, targetValue, fn);
        exHeap.push_back(sum);
        Statement* s = new StatementAssign(variableNames["target"], sum);
        stHeap.push_back(s);
        return memoize(sv, "S~", s, true);
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
        return memoize(sv, "Sasgn", s, true);
      }
      case 3: // function
      {
        StatementFunction* fn = new StatementFunction(sv[0].get<ExpressionFunction*>());
        stHeap.push_back(fn);
        return memoize(sv, "Sfn", fn, true);
      }
      case 4: // block
      {
        Statement** list = new Statement*[sv.size()];
        for (int i = 0; i < sv.size(); i++)
          list[i] = sv[i].get<Statement*>();
        Statement* fn = new StatementBody(list, sv.size());
        stHeap.push_back(fn);
        return memoize(sv, "S{}", fn, true);
      }
    }
  };
  p["AssignOp"] = [&](const Ast& sv) -> void*
  {
    return memoize(sv, "AsOp", new std::string(sv.token), true);
  };
  p["RelOp"] = [&](const Ast& sv) -> void*
  {
    return memoize(sv, "RelOp", new std::string(sv.token), true);
  };
  p["TermOp"] = [&](const Ast& sv) -> void*
  {
    return memoize(sv, "TOp", new std::string(sv.token), true);
  };
  p["FactorOp"] = [&](const Ast& sv) -> void*
  {
    return memoize(sv, "FOp", new std::string(sv.token), true);
  };


  p["ArgList"] = [&](const Ast& sv) -> void*
  {
    // very bad code that passes vectors around :C
    switch (sv.choice())
    {
      case 0:
      case 1:
      {
        std::vector<Expression*>* svexp = new std::vector<Expression*>(*(sv[1].get<std::vector<Expression*>*>()));
        svexp->insert(svexp->begin(), sv[0].get<Expression*>());
        return memoize(sv, "ArglA", svexp, true);
      }
      case 2:
      {
        std::vector<Expression*>* svexp = new std::vector<Expression*>();
        svexp->push_back(sv[0].get<Expression*>());
        return memoize(sv, "ArglB", svexp, true);
      }
      case 3:
        return memoize(sv, "ArglEmpty", new std::vector<Expression*>(), true);
    }
  };
  p["Distribution"] = [&](const Ast& sv) -> fnExpr
  {
    if (distributionMap[sv.token])
      return distributionMap[sv.token];
    return static_cast<fnExpr>(nullptr);
  };

  p["Expression"] = [&](const Ast& sv) -> void*
  {
    std::cout<< "Expression: " << sv.token << std::endl;
	  Expression* expr = sv[0].get<Expression*>();
    for (int i=1; i<sv.size(); i+=2)
    {
  		if (sv[i].get<std::string>().compare("+") == 0) {
  			expr = new ExpressionArithmetic(restan::PLUS, expr, sv[i+1].get<Expression*>());
  			exHeap.push_back(expr);
  		} else if (sv[i].get<std::string>().compare("-") == 0) {
  			expr = new ExpressionArithmetic(restan::MINUS, expr, sv[i+1].get<Expression*>());
  			exHeap.push_back(expr);
  		} else {
        // ??
  		}
	  }
	  return memoize(sv, "Expression", expr);
  };

  p["FunctionExpression"] = [&](const Ast& sv) -> void*
  {
    std::string funcId = sv[0].get<std::string>();
		if (functionMap.find(funcId) == functionMap.end())
    {
  		//throw ParseError(std::string("Did not find functionId at line"));
		}

		fnExpr func = functionMap[funcId];

    // copy vector into array
    Expression** args = new Expression*[sv.size()];
    for (int i = 0; i < sv.size(); i++)
      args[i] = sv[i].get<Expression*>();
    exArrayHeap.push_back(args);

    Expression* fn = new ExpressionFunction(func, args, sv.size());
    exHeap.push_back(fn);
    return memoize(sv, "FnExpr", fn, true);
  };

  p["ExpressionTerm"] = [&](const Ast& sv) -> void*
  {
  	Expression* expr = sv[0].get<Expression*>();
    for (int i=1; i<sv.size(); i+=2)
    {
  		if (sv[i].get<std::string>().compare("*") == 0) {
  			expr = new ExpressionArithmetic(restan::TIMES, expr, sv[i+1].get<Expression*>());
  			exHeap.push_back(expr);
  		} else if (sv[i].get<std::string>().compare("/") == 0) {
  			expr = new ExpressionArithmetic(restan::DIV, expr, sv[i+1].get<Expression*>());
  			exHeap.push_back(expr);
  		}
  	}
  	return memoize(sv, "ExprTrm", expr, true);
  };

  p["ExpressionFactor"] = [&](const Ast& sv) -> void*
  {
    std::cout<< "ExpressionFactor: " << sv.token << std::endl;
    switch (sv.choice())
    {
      case 0:
		    return sv[0].get<Expression*>();
      case 1:
		    return sv[0].get<Expression*>();
      case 2: // constant
        return sv[0].get<Expression*>();
      case 3:
        return sv[0].get<Expression*>();
    }
  };

  p["Constant"] = [&](const Ast& sv) -> void*
  {
    Expression* expr = new ExpressionConstant(stoi(sv.token, nullptr, 10));
    exHeap.push_back(expr);
    return memoize(sv, "Constant", expr, true);
  };

  p["Parameter"] = [&](const Ast& sv) -> unsigned int
  {
    std::string varName = trim(sv.token);
    if (!parameterNames.count(varName))
      return -1;
    else
      return parameterNames[varName];
  };

  p["Variable"] = [&](const Ast& sv) -> unsigned int
  {
    std::string varName = trim(sv.token);
    if (!variableNames.count(varName))
      return -1;
    else
      return variableNames[varName];
  };

  p["VariableExpression"] = [&](const Ast& sv) -> void*
  {
    Expression* expr;
    switch (sv.choice())
    {
        case 0: // parameter
          expr = new ExpressionParameter(sv[0].get<unsigned int>());
          exHeap.push_back(expr);
          return memoize(sv, "varExpr (Parameter)", expr, true);
        case 1: // variable
          expr = new ExpressionVariable(sv[0].get<unsigned int>());
          exHeap.push_back(expr);
          return memoize(sv, "varExpr (Variable)", expr, true);;
    }
  };

  std::cout<<"set up actions\n";

  //p.enable_packrat_parsing();
  p.parse(stanCode.c_str());

  std::cout<<"parsed\n";
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
