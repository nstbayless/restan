#!/bin/bash
# installs python modules

version=`python --help`
if [[ $version =~ "PyPy" ]];
then
  echo "Installing dependencies..."
  # install dependencies
  python -m pip install ad
  python -m pip install matplotlib
  python -m pip install numpy
else
  echo "Cannot install dependencies --"
  echo "You must first set up the virtual environment!"
  echo "./setup.sh"
fi