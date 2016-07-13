import sys
import os

from unittest import TestCase
import unittest
import mock
import json
from amct.amctool.commandline import AMCTool

class TestAMCTool(TestCase):
    
    def setUp(self):
        TestCase.setUp(self)
        self.testsuite_dir = \
            os.path.abspath(os.path.join(os.path.abspath(__file__), '../testsuites'))
        self.config = {'DEBUG': False,
                    'DEFAULT_PORT': '192.168.1.0',
                    'DEFAULT_TESTSUITE_DIR': self.testsuite_dir
                    }
        
        
    @mock.patch.object(AMCTool, '_exec')
    def test_amcscript(self, mockeroo):
        mockeroo.return_value = (0, 'foo', '')
        runner = AMCTool('AMCToolX', [1,2])
        ocode, stdout, stderr = runner.execute_amcscript("{0}/amc/00_init.amc".format(self.testsuite_dir))
        
        self.assertEquals(str(mockeroo.call_args_list[0]),
        "call('AMCToolX -c 1 -c 2 -X {0}/amc/00_init.amc', None)".format(self.testsuite_dir))

    

if __name__ == '__main__':
    unittest.main() 
        
    
