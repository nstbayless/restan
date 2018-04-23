#include "parseData.h"
#include "pi/expression.h"
#include <fstream>

using namespace restan;
using namespace std;

int restan::parseData(const char* path)
{
  // read CSV file
  ifstream f;
  f.open(path);
  int dataCount = restan::pi.numObservedData;
  if (f.is_open())
  {
    std::string val;
    int dataIndex = 0;
    std::vector<double> dataVec[dataCount];
    while (getline (f, val, ','))
    {
      if (val.length() > 0)
      {
        double v = atof(val.c_str());
        dataVec[dataIndex].push_back(v);
        dataIndex = (dataIndex + 1) % dataCount;
      }
    }
    f.close();

    // copy to array
    for (int i = 0; i < dataCount; i++)
    {
      ExpressionValue* eV = new ExpressionValue(1,dataVec[i].size());
      // TODO: cleanup
      for (int j = 0; j < dataVec[i].size(); j++)
      {
        double v = dataVec[i][j];
        (*eV)(0,j) = v;
      }
      restan::pi.data[i] = eV;
    }
    return 0;
  }
  else
  {
    cout << "Unable to open file" << path << std::endl;
    return 1;
  }
}
