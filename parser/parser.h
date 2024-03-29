
#ifndef RESTAN_PARSER_H
#define RESTAN_PARSER_H

#include "pi/pi.h"

namespace restan
{
  // parses the given stanCode and sets the global pi model accordingly
  void parseStan(std::string stanCode);
  void parseStanCleanup();

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
