'''
Runner for the tests. It executes the testsuite as a set of commands
using AMCTool, collects the results and compares them against the
predefined/expected output.
'''

import os
import subprocess
import threading
import json
import logging
from ..amctool.commandline import AMCTool
from ..exceptions import AMCException, AMCExpectedResult, AMCRuntimeException, AMCPartialMatch

class AMCTest(object):
    """Object that holds the commands/expected output"""
    def __init__(self, name):
        self.amc = None
        self.sh = None
        self.expected = None
        self.base = None
        self.name = None
        self.results = {}
        self.desc = None
        
        self.add(name)

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
        
        if ext == '.amc':
            self.amc = True
        elif ext == '.sh':
            self.sh = True
        elif ext == '.expected':
            self.expected = True
        elif ext == '.txt':
            readme = os.path.join(self.base, 'readme.txt')
            if self.desc is None and os.path.exists(readme):
                with open(readme, 'r') as f:
                    self.desc = f.read().strip()
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
    def __init__(self, config, suite_name, ip=None, port=None):
        
        if ip and not isinstance(ip, list):
            ip = map(unicode.strip, ip.split(','))
        
        if port and not isinstance(port, list):
            port = map(unicode.strip, port.split(','))
        
        self.config = config
        self.suite_name = suite_name
        self.run_number = 0
        self.test_name = None
        self.results = []
        self.logger = logging.getLogger(__name__ + '.AMCStandardRunner')
        
        env = config.copy()
        env['AMCT_TEMP_DIR'] = self.config.get('AMCT_TEMP_DIR', '/tmp')
        
        
        self.amctool = AMCTool(self.config.get('DEFAULT_AMC_TOOL', 'AMCTool'), 
                                ip or port or config.get('DEFAULT_PORT'),
                                env,
                                timeout = self.config.get('DEFAULT_TIMEOUT', 15))
    
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
        
        tests = self._collect_files(os.path.join(self.config.get('DEFAULT_TESTSUITE_DIR'), 
                                                 self.suite_name))
        
        self.logger.debug('Will run %s tests', len(tests))
        self.results = []
        for t in tests:
            if t.amc:
                self.test_name = t.name + '.amc'
                self.execute(t, 'amc')
            if t.sh:
                self.test_name = t.name + '.sh'
                self.execute(t, 'sh')
            if t.expected:
                self.test_name = t.name + '.expected'
                self.check_results(t)
            self.run_number += 1
            self.results.append(t.results)
    
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
        
        self.logger.debug('Executing %s', self.test_name)
        
        env = {}
        env['AMCT_TEST_NUM'] = self.run_number
        env['AMCT_TEST_NAME'] = self.test_name
            
        if type == 'amc':
            ocode, out, err = self.amctool.execute_amcscript(test.get_path() + '.amc', env)
            test.add_results(type, out, err)
        elif type == 'sh':
            ocode, out, err = self.amctool.execute_cmdline(test.get_path() + '.sh', env)
            test.add_results(type, out, err)
        else:
            raise AMCException('Unknown type {0}'.format(type))
        
        if ocode != 0:
            raise AMCException('error exacuting {0}:{1}'.format(type,err))

        self.logger.info('STDOUT:\n%s\n%s', out, '=' * 80)
        self.logger.debug('STDERR:\n%s\n%s', err, '+' * 80)
        
    
        
    def check_results(self, test):
        """Will check the collected output against the expected results."""
        #TODO: check stderr?
        
        expected = ''
        with open(test.get_path() + '.expected', 'r') as f:
            expected = f.read()
        
        self.logger.debug('Comparing results, expected:\n%s', expected)
        
        full_match = partial_match = False
        
        for k, data in test.results.items():
            if k not in ('sh', 'amc'):
                continue
            if len(data) != 2: # stdout, stderr
                continue
            if expected == data[0]:
                full_match = k
            elif expected in data[0]:
                partial_match = k
        
        if full_match: # bingo!
            self.logger.debug('Full match found')
            return True
        elif partial_match:
            self.logger.debug('Partial match found')
            raise AMCPartialMatch(partial_match)
        else:
            self.logger.debug('Zero match found')
            raise AMCExpectedResult()
