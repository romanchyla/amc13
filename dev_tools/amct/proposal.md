# Automated Testing for AMC13

## Goal

Develop a set of routines to *automatically* (i.e. without human intervention) test
AMC13 hardware. Tests should exercise the functionality of the AMC13 card(s) and
report errors/failures. No statistics/GUI is planned, interface is via a command line.


## Interface

`AMC13Tool.exe` (further on just `Tool`) is the interface to AMC13 cards. It can be used
to query and control the hardware operations. The tool has its own query language (interface)
and can execute scripts (i.e. has standard input/output capabilities).


## Possible strategies

This section briefly discusses available options and discusses reasons for selecting one over
the other(s).

### Python C-bindings to AMC13Tool.exe

The `tool` is written in C++; c bindings of exist inside
`amc13/python`. If complete and compiled/installed, AMC13 cards could be controlled directly
from python. Test could be realized using 'simple' python scripts.

### Use existing AMC13Tool stdout/stdin capabilities

The `tool` provides standard input and output. It can execute `amc` scripts. The output seems
to provide all necessary information (albeit in a human readable format only; machine-readable
format would be much better - maybe there is such an option? Or could be easily added?)

### Discussion

The Python C-bindings is a very elegant solution, but also expensive one. When
designing software, we have to keep in mind not only the cost of development, but also the
cost of maintenance. It requires a relatively high level of expertise (in building the
connection for c-types between Python and C++) and *very tight coupling* between software
and hardware. For example, the extension can be developed and tested only on the machine
where the AMC13 hardware is available.

Note: experts will probably disagree, certainly it is possible to develop software on a
local laptop, but that is a theory and reality is much less tidy. For all practical purposes
the current software development *is tightly coupled* and happens on machines with the
AMC13 cards and necessary linkers/compilers/headers. That makes development/testing/debugging
of a new tool very time consuming. (and it doesn't help that the machines run old versions of
software packages, e.g. python 2.6, and some basic tools are missing - e.g. `vim`).

Because AMC13Tool already provides an *interface* to AMC13 cards - and this interface is *good enough*
for purposes of automated testing - it is better to use it. The `tool` is actively
maintained and developed and we would avoid duplication of efforts. However, we also propose
to develop AMCT in a way that it is be possible to use the c-bindings (as drop-in replacement),
should somebody develop the c-bindins in the future.


## Proposal for the AMCT

### Overview

The idea is simple: AMCT will execute `amc` commands using the `tool`, it will collect output (results),
read and parse them, and then check their validity.

### More datailed work-flow

AMCT is built around the concept of `unittests` (we are misusing the terminolgy, AMCT is actually running
`functional tests`). The idea is that we want to think of `units` - unit of work/functionality -
that we wish to exercise and check their effects. For example, AMCT will set a value of certain registers,
then read them back and check the values. The next `unittest` will exercise functionality that is affected
by changed of *just this one* register. And so on, from basic assumptions towards more and more complicated
interactions.

AMCT will be using `testsuites`, those are conceptually related `unittests`. They will be saved inside a
folder. Each testsuite is saved in a separate folder, inside of which unittests are sorted alphabetically.
Example:

  * testsuites/register-wd1/
    * readme.txt - short description
    * 00_set_register.amc
    * 00_set_register.sh
    * 00_set_register.py
    * 00_set_register.expected
    * 01_read_config.amc
    * 01_read....

In this example, the first `unittest` has name `00_set_register`. It will be executed in this order:

  1. .amc - i.e. AMC13Tool.exe -X 00_set_register.amc ...
  1. .sh - /bin/bash 00_set_register.sh [ and it will receive output from the previous step + ENV VARS]
  1. .py - will be called directly from python, it will receive output of previous steps
  1. .expected - if present, contents of the file will be compared against output of any of the previous steps

Any of the files can be missing (the step will be skipped). If the tests succeed, the next unittest `01_read_config`
will start execution.

This testsuite can be executed against any combination of AMC13 cards, i.e.

    `amct testsuite run register-wd1 --slot 1,2,3` (here the test is working with 3 cards at once)

or, in loop

    `for i in (1,2,3) do; amct testsute run register-wd1 --slot $i; done` (here one testsuite is run against 3 cards)


In the future, AMCT can be extended with an option to run testsuite automatically against any discovered card.


### Pre-requisites

 * functional installation of AMC13Tool.exe
 * python 2.7 [note: 2.6 is very very old]

### Checking validity

There exists two general classes of properties that will be tested, let's call them:

  1. static
  1. dynamic

Static properties do not change between cards and do not depend on time (timers). For example,
when certain registers are turned ON/OFF - we can test for presence or absence of a bit in
a given WORD.

Dynamic properties, on the other hand, depend on external events (e.g. timers, randomly generated
events) - to test correct operation of the hardware, we must be able to *extract meaningful*
information from a dynamically changing output. And example is a count of WORDS (i.e. size of the
measurement) that was captured by AMC13 card under specific conditions.

Checking static properties is relatively easy, we need to extract a DATUM from the output and compare
it against the expected value. To check dynamic properties, we need to have a context, then compute
the expected result, also parse the output and then check the expected value.

### Configuration

In general, AMCT will read configuration from a file. Each user/project can have its own
configuration.

### Interface

AMCT prints output to stdout and logs into files. Users control operations via command line.
Example:

    `amct --help`
    `amct --config projectX.py --verbose testsuite run register-w1 --slot 1,3`


### Additional tools

AMCT can contain additional tools useful for AMC13 testing and/or operation. E.g. download and install hardware, start the `tool` against a card in a specific slot, test configuration, of provide utilities for computing state of the card.

Those additional tools can be added as separate packges and will be accessible as separate commands, e.g.

    `amct tools wordcount`
    `amct firmware install --version X --slot 1`


## Caveats, problems, questions

### Input/output

This proposal relies on the `tool`s ability to output data. If certain pieces of information
are not accessible, the `tool` must either be extended or this approach is not viable. Do you
see problems?

### Machine readable output

The current output is good for humans, but as a programmatic interface it is not ideal. Can the
`tool` be extended or is there another way to output data in a machine readable format?

### Conflicts

We are assuming that the `tool` has no problems running in several instances. I.e. it can be started
multiple times. However, AMCT should make sure that testsuites are operating against different cards.
(ie. maintain a shared state between multiple invocations of AMCT)

Or is it not an issue?

### Parallelism

A card in the slot 13 is used for testing. But that would suggest, that it carries a state - and therefore
tests cannot be run in parallel (if they rely on the card in slot 13). Is that correct?

### Speed

Related to previous. Technically, we should be able to execute the `tool` in parallel (multiple instances).
AMCT can also be executed in parallel. So theoretically, we could run tests in parallel - and achieve
much higher througput, shorter testing times. But do you see problems?

Also, the execution reading stdout/stderr is probably much slower - but we speak about milliseconds. I
do not think this should be a high priority, but better to ask.
