#!/bin/bash

unamestr=`uname`
if [[ "$unamestr" == 'Linux' ]]; then
  if [ -d "vpy" ]; then
    echo "pypy3 is already installed locally."
    echo "Please remove pypy3/ folder (and all installations therein)"
    echo "Before running this script again."
    echo ""
    echo ""
    echo "also, don't forget to type:"
    echo "source activate"
    exit
  fi

  echo "Downloading Linux pypy3..."
  sleep 1
  
  # create folder for pypy3
  mkdir vpy-ext
  cd vpy-ext
  
  # download and unpack pypy3
  wget https://bitbucket.org/pypy/pypy/downloads/pypy3-v5.10.1-linux64.tar.bz2
  if [ $? -eq 0 ]; then
    echo "OK"
  else
    echo "FAIL"
    exit
  fi
  tar xvjf *.tar.bz2
  if [ $? -eq 0 ]; then
    echo "OK"
  else
    echo "FAIL"
    exit
  fi
  rm *.tar.bz2
  mv pypy3-* pypy3
  cd ..
  mv vpy-ext/pypy3 .
  rm -r vpy-ext
  
  # install pip for pypy3
  echo "Ensuring pip is installed for pypy3..."
  sleep 1
  pypy3/bin/pypy3 -m ensurepip
  if [ $? -eq 0 ]; then
    echo "OK"
  else
    echo "FAIL"
    exit
  fi
  
  # install virtualenv for pypy3
  echo "Ensuring virtualenv is installed for pypy3..."
  sleep 1
  pypy3/bin/pypy3 -m pip install virtualenv
  if [ $? -eq 0 ]; then
    echo "OK"
  else
    echo "FAIL"
    exit
  fi
  
  #setup virtual environment
  echo "Setting up virtual environment..."
  sleep 1
  pypy3/bin/pypy3 -m virtualenv ./vpy
  if [ $? -eq 0 ]; then
    echo "OK"
  else
    echo "FAIL"
    exit
  fi
  
  echo "#!/bin/bash" > activate
  echo 'echo "Entering virtual environment..."' >> activate
  echo "source ./vpy/bin/activate" >> activate
  chmod a+wrx activate
  
  source activate
  chmod a+wrx ./install-modules.sh
  ./install-modules.sh
  
  echo ""
  echo ""
  echo "now please type:"
  echo "source activate"
else
  echo "Setup file is written for linux only :C good luck"
  exit
fi

exit