'''
Runner for the tests. It executes the testsuite as a set of commands
using AMCTool, collects the results and compares them against the
predefined/expected output.
'''

import os
import subprocess
from subprocess import TimeoutExpired
import tempfile

class AMCException(Exception): pass
class AMCRuntimeException(Exception): pass
class AMCExpectedResult(Exception): pass
class AMCPartialMatch(Exception): pass

class AMCTest(object):
    """Object that holds the commands/expected output"""
    def __init__(self, name):
        self.amc = None
        self.sh = None
        self.expected = None
        self.base = None
        self.name = None
        self.add(name)
        self.results = {}
        
    def add(self, name):
        """Receives the absolute filepath and adds the test 
        to the appropriate place."""
        
        b, n = os.path.split(name)
        
        if self.base is None:
            self.base = b
        
        if self.base != b:
            raise AMCException("Bases differ {0}:{1}".format(self.base, n))
        n, ext = os.path.splitext(n)
        
        if self.name is None:
            self.name = n
        elif self.name != n:
            raise AMCException('You can group only tests with the same name: has {0}, received {1}'
                            .format(self.name, n))
        
        if ext == 'amc':
            self.amc = True
        elif ext == 'sh':
            self.sh = True
        elif ext == 'expected':
            self.expected = True
        else:
            raise AMCException('Unknown type {0} for {1}'.format(ext, self.get_path()))
    
    def get_path(self):
        return '{0}/{1}'.format(self.base, self.name)
    
    def finalize(self):
        if self.expected and not (self.amc or self.sh):
            raise AMCException('Expected output, but no amc/sh runners inside: {0}'
                               .format(self.get_path()))

    def add_results(self, tipe, stdout, stderr):
        self.results[tipe] = (stdout, stderr)
        
    def get_results(self, tipe):
        return self.results.get(tipe)

class AMCStandardRunner(object):
    '''
    Standard implementation which compares output stdin/stdout.
    '''


    def __init__(self, config, suite_name):
        self.config = config
        self.suite_name = suite_name
        self.tmpdir = tempfile.mkdtemp()
        
    
    def run(self):
        """
        Default runner for the tests, it does the following:
            1. collects the files *.(amc|sh|expected)
            2. sorts them alphabetically
            3. starts executing them one by one
                .amc - will run the set of commands using AMCTool
                .sh - will execute (a shell) script and gives it
                      information about the previous results
                .expected - will compare the output from the .amc|.sh
                      against the output inside the file
        """
        tests = self._collect_files(os.path.join(self.config.get('testdir'), self.suite_name))
        for t in tests:
            if t.amc:
                self.execute(t, 'amc')
            if t.sh:
                self.execute(t, 'sh')
            if t.expected:
                self.check_results(t)
                
    def cleanup(self):
        pass #TODO: delete self.tmpdir

    def _collect_files(self, dist):
        files = []
        for f in os.listdir(dist):
            f = os.path.join(dist, f)
            if os.path.isfile(f):
                files.append(f)
        files = sorted(files)
        tests = []
        curr_test = None
        
        for f in files:
            if curr_test is None:
                curr_test = AMCTest(f)
                tests.append(curr_test)
            else:
                try:
                    curr_test.add(f)
                except AMCException:
                    curr_test = AMCTest(f)
                    tests.append(curr_test)
        return tests

            
    def execute(self, test, type):
        """
        Will execute commands using AMCTool and collect output
        into a temp folder.type
        """
        c = self.config
        if type == 'amc':
            out, err = self._exec('{0} {1}'.format(self.get_amc_exec(), test.get_path() + '.amc'))
            test.add_results(type, out, err)
        elif type == 'sh':
            out, err = self._exec('{0}'.format(self.get_amc_exec(), test.get_path() + '.sh'))
            test.add_results(type, out, err)
        else:
            raise AMCException('Unknown type {0}'.format(type))
            
            
            
    def _exec(self, cmd_line):
        proc = subprocess.Popen(cmd_line, shell=True) # I'm assuming our code can be trusted
        try:
            outs, errs = proc.communicate(timeout=int(self.config.get('CMD_TIMEOUT', 15)))
        except TimeoutExpired:
            proc.kill()
            outs, errs = proc.communicate()
            raise AMCRuntimeException('{0} failed with {1}\n{2}'.format(cmd_line, outs, errs))
        
    def check_results(self, test):
        """Will check the collected output against the expected results."""
        #TODO: check stderr?
        
        expected = ''
        with open(test.get_path() + '.expected', 'b') as f:
            expected = f.read()
        
        full_match = partial_match = False
        
        for k, v in test.results.items():
            if expected == v:
                full_match = k
            elif expected in v:
                partial_match = k
        
        if full_match: # bingo!
            return True
        elif partial_match:
            raise AMCPartialMatch(partial_match)
        else:
            raise AMCExpectedResult()