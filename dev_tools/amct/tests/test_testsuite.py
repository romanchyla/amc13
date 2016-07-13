import sys
import os

from unittest import TestCase
import unittest
import mock
import json
from amct.testsuite.runner import AMCStandardRunner

class TestAMCStandardRunner(TestCase):
    
    def setUp(self):
        TestCase.setUp(self)
        self.testsuite_dir = \
            os.path.abspath(os.path.join(os.path.abspath(__file__), '../testsuites'))
        self.config = {'DEBUG': False,
                    'DEFAULT_PORT': '192.168.1.0',
                    'DEFAULT_TESTSUITE_DIR': self.testsuite_dir
                    }
        

    @mock.patch.object(AMCStandardRunner, 'check_results')
    def test_expected(self, mocked):
        runner = AMCStandardRunner(self.config, 'expected')
        runner.run() # this should succeed
        
        
    def test_sh(self):
        runner = AMCStandardRunner(self.config, 'sh')
        runner.run()
        self.assertEquals(runner.results[0]['sh'][0], "test output\n")
        self.assertTrue('AMCT_TEST_NUM 1' in runner.results[1]['sh'][0])

if __name__ == '__main__':
    unittest.main() 
        
    
