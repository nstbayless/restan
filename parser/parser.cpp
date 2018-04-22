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
        Code <- '__BEGIN_STAN_CODE__' OptionalData OptionalParameters OptionalTransformedParameters Model '__END_STAN_CODE__'
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
        Parameter <- < [a-zA-Z_]+ >
        Variable <- < [a-zA-Z_]+ >
        VariableExpression <- Parameter / Variable
        Distribution <- Identifier

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
std::map<std::string, fnExpr> distributionMap;
std::map<std::string, fnExpr> functionMap;

// memoize
std::map<std::string, void*> memoizedPointers;
void* memoize(const Ast& sv, std::string production, void* proposed, bool dispose = false)
{
  return proposed;
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

void* eval(const Ast& sv) {
  std::cout<<sv.name<<std::endl;
  auto& ast = sv;
  if (ast.name == "Code")
  {
    std::cout<<"took root production\n";
    declaring = 1;
    eval(*ast.nodes[1]); // parameters
    declaring = 2;
    auto* _tppair = (std::pair<restan::Statement**, int>*)eval(*(ast.nodes[2])); // transformed parameters
    std::pair<restan::Statement**, int> ttpair(*_tppair);
    delete(_tppair);
    declaring = 0;
    restan::Statement* modelStatement = (restan::Statement*)eval(*ast.nodes[3]); // model
    ttpair.first[ttpair.second] = modelStatement;
    restan::Statement* lossStatement = new restan::StatementBody(ttpair.first, ttpair.second+1);
    stHeap.push_back(lossStatement);
    pi.setLossStatement(lossStatement);
  };
  if (ast.name == "OptionalParameters")
  {
    for (int i = 0; i < sv.nodes.size(); i ++)
    {
      eval(*ast.nodes[i]);
    }
  };
  if (ast.name == "OptionalTransformedParameters")
  {
    restan::Statement** sl = new restan::Statement*[sv.nodes.size()+1];
    restan::Statement** slo = sl;
    stArrayHeap.push_back(sl);
    for (int i = 0; i < sv.nodes.size(); i ++)
    {
      restan::Statement* s = (restan::Statement*)eval(*ast.nodes[i]);
      if (s)
      {
        *slo = s;
        slo ++;
      }
    }
    return new std::pair<restan::Statement**, int>(sl, slo - sl);
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
  if (ast.name == "VariableDeclaration")
  {
    int choice = 2;

    if (ast.nodes.size() > 1) {
      choice = 1;
    } else if (ast.nodes.size() == 1) {
      choice = 0;
    }
    if (choice == 1)
    {
      // type f = blah
      unsigned int* a = reinterpret_cast<unsigned int*>(eval(*ast.nodes[0]));
      restan::Statement* declareAssign = new restan::StatementAssign(*a, (Expression*)eval(*ast.nodes[2]));
      delete(a);
      stHeap.push_back(declareAssign);
      return (restan::Statement*) memoize(sv, "VD", declareAssign, true);
    }
    else
    {
      (restan::Statement*)eval(*ast.nodes[0]); // trigger
      return static_cast<restan::Statement*>(nullptr);
    }
  };
  if (ast.name == "DeclarationLHS")
  {
    std::string* varName = (std::string*)eval(*ast.nodes[sv.nodes.size() - 1]);
    std::string s = trim(*varName);
    delete(varName);
    return new unsigned int (variableNames[s]);
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
        std::vector<Expression*>* _argVec((std::vector<Expression*>*)eval(*ast.nodes[2]));
        std::vector<Expression*> argVec = *_argVec;
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
        std::string assignOp = *_assignOp;
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
        restan::Statement** list = new restan::Statement*[sv.nodes.size()];
        for (int i = 0; i < sv.nodes.size(); i++)
          list[i] = (restan::Statement*)eval(*ast.nodes[i]);
        restan::Statement* fn = new restan::StatementBody(list, sv.nodes.size());
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
      if (ast.nodes[i]->token.compare("+") == 0) {
        expr = new ExpressionArithmetic(restan::PLUS, expr, (Expression*) eval(*ast.nodes[i+1]));
        exHeap.push_back(expr);
      } else if (ast.nodes[i]->token.compare("-") == 0) {
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
      if (ast.nodes[i]->token.compare("*") == 0) {
        expr = new ExpressionArithmetic(restan::TIMES, expr, (Expression*) eval(*ast.nodes[i+1]));
        exHeap.push_back(expr);
      } else if (ast.nodes[i]->token.compare("/") == 0) {
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
    Expression* expr = new ExpressionConstant(stoi(sv.token, nullptr, 10));
    exHeap.push_back(expr);
    return memoize(sv, "Constant", expr, true);
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

  if (ast.name == "VariableExpression")
  {
    Expression* expr;
    int choice = 1;
    if (sv.nodes[0]->name == "Parameter")
      choice = 0;
    switch (choice)
    {
        case 0: // parameter
        {
          unsigned int * a = (unsigned int*)eval(*ast.nodes[0]);
          unsigned int b = *a;
          delete(a);
          expr = new ExpressionParameter(b);
          exHeap.push_back(expr);
          return memoize(sv, "varExpr (Parameter)", expr, true);
        }
        case 1: // variable
        {
          unsigned int * a = (unsigned int*)eval(*ast.nodes[0]);
          unsigned int b = *a;
          delete(a);
          expr = new ExpressionVariable(b);
          exHeap.push_back(expr);
          return memoize(sv, "varExpr (Variable)", expr, true);
        }
    }
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
  }

  std::cout<<"parsed\n";
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
