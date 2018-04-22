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
    auto lookup = std::string(production) + " >~!~< token-(\"" + sv.token + std::string("\")");
    std::cout << lookup << std::endl << " (proposed pointer:" << proposed << ")" << std::endl;
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

  void* (*eval)(const Ast&) = [&](const Ast& sv) -> void* {
    auto& ast = sv;
    if (ast.name == "Code")
    {
      std::cout<<"took root production\n";
      Statement* modelStatement = (Statement*)eval(*ast.nodes[2]);
      auto* _tppair = (std::pair<Statement**, int>*)eval(*(ast.nodes[1]));
      std::pair<Statement**, int> ttpair(*_tppair);
      delete(_tppair);
      ttpair.first[ttpair.second] = modelStatement;
      Statement* lossStatement = new StatementBody(ttpair.first, ttpair.second+1);
      stHeap.push_back(lossStatement);
      pi.setLossStatement(lossStatement);
    };
    std::cout<<"parsed syntax\n";
    if (ast.name == "OptionalParameters")
    {
    };
    if (ast.name == "OptionalTransformedParameters")
    {
      Statement** sl = new Statement*[sv.nodes.size()+1];
      Statement** slo = sl;
      stArrayHeap.push_back(sl);
      for (int i = 0; i < sv.nodes.size(); i ++)
      {
        Statement* s = (Statement*)eval(*ast.nodes[i]);
        if (s)
        {
          *slo = s;
          slo ++;
        }
      }
      return new std::pair<Statement**, int>(sl, slo - sl);
    };
    if (ast.name == "Model")
    {
      Statement* modelStatement = (Statement*)eval(*ast.nodes[2]);
      return memoize(sv, "Model", (void*)modelStatement);
    };
    if (ast.name == "DeclarationOrStatement")
    {
      return (Expression*)eval(*ast.nodes[0]);
    };
    if (ast.name == "VariableDeclaration")
    {
      if (sv.choice() == 1)
      {
        // type f = blah
        Statement* declareAssign = new StatementAssign((unsigned int)eval(*ast.nodes[0]), (Expression*)eval(*ast.nodes[2]));
        stHeap.push_back(declareAssign);
        return (Statement*) memoize(sv, "VD", declareAssign, true);
      }
      else
      {
        (Statement*)eval(*ast.nodes[0]); // trigger
        return static_cast<Statement*>(nullptr);
      }
    };
    if (ast.name == "DeclarationLHS")
    {
      std::string varName = trim(ast.nodes[sv.nodes.size() - 1].get<std::string>());
      return variableNames[trim(varName)];
    };
    if (ast.name == "Identifier")
    {
      return memoize(sv, "Ident", new std::string(trim(sv.token)), true);
    };
    if (ast.name == "Statements")
    {
      std::cout<< " statements -- " << sv.token << std::endl;
      switch (sv.choice())
      {
        case 0:
        {
          Statement* second = (Statement*)eval(*ast.nodes[1]);
          Statement* first = (Statement*)eval(*ast.nodes[0]);
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
    if (ast.name == "Statement")
    {
      std::cout << "statement -- " << sv.token << std::endl;
      int choice = 4;
      if (sv.nodes.size() > 1)
        if (ast.nodes[1]->name == "Distribution")
          choice = 0;
      if (ast.nodes[0]->name == "AssignOp")
        choice = 1;
      if (ast.nodes[0]->name == "RelOp")
        choice = 2;
      if (ast.nodes[0]->name == "FunctionExpression")
        choice = 3;
      switch (choice)
      {
        case 0: // a ~ D
        {
          Expression* varExpr = (Expression*)eval(*ast.nodes[0]);
          fnExpr func = (fnExpr)eval(*ast.nodes[1]);
          std::vector<Expression*> argVec(ast.nodes[2].get<std::vector<Expression*> >());
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
          unsigned int variableIndex = (unsigned int)eval(*ast.nodes[0]);
          Expression* rhs = (Expression*)eval(*ast.nodes[2]);
          std::string assignOp = (std::string)eval(*(ast.nodes[1]))
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
          StatementFunction* fn = new StatementFunction((ExpressionFunction*)eval(*ast.nodes[0]));
          stHeap.push_back(fn);
          return memoize(sv, "Sfn", fn, true);
        }
        case 4: // block
        {
          Statement** list = new Statement*[sv.nodes.size()];
          for (int i = 0; i < sv.nodes.size(); i++)
            list[i] = (Statement*)eval(*ast.nodes[i]);
          Statement* fn = new StatementBody(list, sv.nodes.size());
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
      switch (sv.choice())
      {
        case 0:
        case 1:
        {
          std::vector<Expression*>* svexp = new std::vector<Expression*>(*(ast.nodes[1].get<std::vector<Expression*>*>()));
          svexp->insert(svexp->begin(), (Expression*)eval(*ast.nodes[0]));
          return memoize(sv, "ArglA", svexp, true);
        }
        case 2:
        {
          std::vector<Expression*>* svexp = new std::vector<Expression*>();
          svexp->push_back((Expression*)eval(*ast.nodes[0]));
          return memoize(sv, "ArglB", svexp, true);
        }
        case 3:
          return memoize(sv, "ArglEmpty", new std::vector<Expression*>(), true);
      }
    };
    if (ast.name == "Distribution")
    {
      if (distributionMap[sv.token])
        return distributionMap[sv.token];
      return static_cast<fnExpr>(nullptr);
    };

    if (ast.name == "Expression")
    {
      std::cout<< "Expression: " << sv.token << std::endl;
  	  Expression* expr = (Expression*)eval(*ast.nodes[0]);
      for (int i=1; i<sv.nodes.size(); i+=2)
      {
    		if (ast.nodes[i].get<std::string>().compare("+") == 0) {
    			expr = new ExpressionArithmetic(restan::PLUS, expr, ast.nodes[i+1].get<Expression*>());
    			exHeap.push_back(expr);
    		} else if (ast.nodes[i].get<std::string>().compare("-") == 0) {
    			expr = new ExpressionArithmetic(restan::MINUS, expr, ast.nodes[i+1].get<Expression*>());
    			exHeap.push_back(expr);
    		} else {
          // ??
    		}
  	  }
  	  return memoize(sv, "Expression", expr);
    };

    if (ast.name == "FunctionExpression")
    {
      std::string funcId = (std::string)eval(*(ast.nodes[0]))
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
    		if (ast.nodes[i].token.compare("*") == 0) {
    			expr = new ExpressionArithmetic(restan::TIMES, expr, ast.nodes[i+1].get<Expression*>());
    			exHeap.push_back(expr);
    		} else if (ast.nodes[i].token.compare("/") == 0) {
    			expr = new ExpressionArithmetic(restan::DIV, expr, ast.nodes[i+1].get<Expression*>());
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
      Expression* expr = new ExpressionConstant(stoi(sv.token, nullptr, 10));
      exHeap.push_back(expr);
      return memoize(sv, "Constant", expr, true);
    };

    if (ast.name == "Parameter")
    {
      std::string varName = trim(sv.token);
      if (!parameterNames.count(varName))
        return -1;
      else
        return parameterNames[varName];
    };

    if (ast.name == "Variable")
    {
      std::string varName = trim(sv.token);
      if (!variableNames.count(varName))
        return -1;
      else
        return variableNames[varName];
    };

    if (ast.name == "VariableExpression")
    {
      Expression* expr;
      int choice = 1;
      if (sv.node[0].name == "Parameter")
        choice = 0;
      switch (sv.choice())
      {
          case 0: // parameter
            expr = new ExpressionParameter((unsigned int)eval(*ast.nodes[0]));
            exHeap.push_back(expr);
            return memoize(sv, "varExpr (Parameter)", expr, true);
          case 1: // variable
            expr = new ExpressionVariable((unsigned int)eval(*ast.nodes[0]));
            exHeap.push_back(expr);
            return memoize(sv, "varExpr (Variable)", expr, true);;
      }
    };
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
