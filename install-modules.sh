#!/bin/bash
# installs python modules

version=`python --help`
if [[ $version =~ "PyPy" ]];
then
  echo "Installing dependencies..."
  # install dependencies
  echo "This command may hang for several minutes..."
  sleep 2
  python -m pip install autograd
else
  echo "Cannot install dependencies --"
  echo "You must first set up the virtual environment!"
  echo "./setup.sh"
fi