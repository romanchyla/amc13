Development tools
=================


~~~ Installation
 

Typically, you want to use `virtualenv` to create a separate python environment and install all dependencies.

Before starting, make sure you have `pip` and `virtualenv`. Use your favorite package manager to install them.
 
Installation instructions:

    * cd ./dev_tools
    * virtualenv virtualenv
    * source virtualenv/bin/activate
    * pip install --upgrade pip
    * python setup.py develop

Note, if you want to install `amct` package into your normal python installation, just do: `python setup.py install`.
    

~~~ Usage

    $ amct --help
    
Will print help. Usually, tests are organized into suites. You will want to execute them against every AMC13
board like so:

    $ amct testsuite run --name hello --slot 13
    
        