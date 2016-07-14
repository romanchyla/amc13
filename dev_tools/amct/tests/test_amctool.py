import sys
import os

from unittest import TestCase
import unittest
import mock
import json
import traceback
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

    @mock.patch.object(AMCTool, 'run')
    def test_eval(self, mockeroo):
        code = """
for x in range(3):
    v = amc.run(x)
    print x, "{FOO}", v
    """
        mockeroo.return_value = 'ok'
        runner = AMCTool('AMCTool13.exe', 1)
        out = runner.eval_code(code, 'foo.amc', {'FOO': 'bar'})
        assert out.splitlines() == ['0 bar ok', '1 bar ok', '2 bar ok']
        
        # check the amc.run() was called 3 times and what command it actually got
        assert mockeroo.call_count == 3
        assert str(mockeroo.call_args_list) == '[call(0), call(1), call(2)]'
        
    
    @mock.patch.object(AMCTool, 'run')
    def test_eval_error(self, mockeroo):
        """Evaluating syntactically incorrect code."""
        code = """
for x in range(3):
    v = amc.run(x)
    print x, "{FOO}, v
    """
        mockeroo.return_value = 'ok'
        runner = AMCTool('AMCTool13.exe', 1)
        try:
            out = runner.eval_code(code, 'foo.amc', {'FOO': 'bar'})
        except SyntaxError, e:
            exc = traceback.format_exc()
            assert 'File "foo.amc", line 4\n    print x, "bar, v\n                   ^' in exc
            assert str(e) == 'EOL while scanning string literal (foo.amc, line 4)'

if __name__ == '__main__':
    unittest.main() 
        
    
