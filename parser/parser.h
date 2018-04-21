
#ifndef RESTAN_PARSER_H
#define RESTAN_PARSER_H

#include "pi/pi.h"

#include <peglib.h>

namespace restan
{
  // parses the given stanCode and sets the global pi model accordingly
  void parseStan(std::string stanCode);
  void parseStanCleanup();

  void tracer(const char* name, const char* s, size_t n, const peg::SemanticValues& sv, const peg::Context& c, const peg::any& dt);

  class ParseError : public std::exception
  {
    public:
      ParseError(std::string s): whatText(s) {}
      std::string what()
      {
        return whatText.c_str();
      }
    private:
    std::string whatText;
  };
}

#endif
