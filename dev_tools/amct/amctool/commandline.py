
import logging
import os
import subprocess
import json
import threading
import tempfile
from cStringIO import StringIO
import sys
from ..exceptions import AMCException, AMCRuntimeException, AMCEvalException


class CaptureStdout(list):
    def __enter__(self):
        self._stdout = sys.stdout
        sys.stdout = self._stringio = StringIO()
        return self
    def __exit__(self, *args):
        self.extend(self._stringio.getvalue().splitlines())
        sys.stdout = self._stdout
    
    def output(self):
        return self._stringio.getvalue()

class AMCTool(object):
    """A class which executes commands via standard command line 
    interface of the AMCTool13
    """
    
    def __init__(self, 
                 amc_tool_binary,
                 ip_or_port,
                 environ=None,
                 timeout=15):
        
        self.logger = logging.getLogger(__name__ + '.AMCTool')
        self.amc_tool_binary = amc_tool_binary
        self.ip_or_port = ip_or_port
        self._build_environ(environ)
        self._timeout = timeout
        self._eval_env = None
    
    def eval_code(self, code, filename, additional_env=None):
        """Executes python source code, after having extrapolated
        its values. This can be used to run arbitrary tests. If 
        errors happen, normal exceptions are raised.
        
        @param code: string, a python code with variables that 
            can be extrapolated, e.g. {AMCT_PORT}
        @param filename: string, name of the file that we are
            executing. This can be arbitrary string, but in case
            of errors, it will help debugging
        @param additional_env: dict, env vars that should be
            made available to the evaluated script.
            
        @return: string, the output from the evaluated script.
        
        NOTE: we are assuming that the code can be trusted, be
        very careful not to expose this eval() functionality to
        untrusted clients.
        
        """
        
        self.logger.debug('Code before extrapolation:', code)
        env = self.environ.copy()
        if additional_env:
            env.update(additional_env)
        self._eval_env = env
        code = code.format(**env)
        self.logger.debug('Code after extrapolation:', code)
        bytes = compile(code, filename, 'exec')
        
        # this will be evaluated inside our current environment
        # so the script can have access to the self object
        
        with CaptureStdout() as output:
            amc = self
            try:
                eval(bytes)
            except AMCRuntimeException, e:
                raise AMCEvalException(e)
            finally:
                self._eval_env = None
            return output.output()
            
    
    def execute_cmdline(self, cmd_line, additional_env=None):
        """Executes commandline using the current environment.
        
        @param cmdline: string (bash shell command)
        @return: tuple [return_code, stdout, stderr]
        """
        self.logger.debug('executing: %s', cmd_line)
        return self._exec(cmd_line, additional_env)
    
    def execute_amcscript(self, filepath, additional_env=None):
        """Executes AMC script.
        
        @param filepath: path to the file
        @return: tuple [return_code, stdout, stderr]
        """
        self._check_fpath(filepath)
        cmd_line = '{0} -X {1}'.format(self._get_amc_exec(), filepath)
        return self.execute_cmdline(cmd_line, additional_env)
        
    
    def run(self, amc_command):
        """Executes AMC commands using AMCTool13.exe. And returns just
        the STDOUT. 
        
        @param amc_command: string, new line separated amc instructions
        @return: string, standard output
        """
        # TODO: if amctool has ability to execute commands directly
        # use that
        
        fname = self._create_tempfile(amc_command)
        try:
            code, stdout, stderr = self.execute_amcscript(fname, self._eval_env)
            if code != 0:
                raise AMCRuntimeException(stderr)
            return stdout
        finally:
            os.remove(fname)

    
    def _create_tempfile(self, contents):    
        fd, fname = tempfile.mkstemp(text=True)
        os.close(fd)
        fo = open(fname, 'w')
        fo.write(contents)
        fo.close()
        return fname
        
    
    def _check_fpath(self, filepath):
        if not os.path.exists(filepath) and not os.path.isfile(filepath):
            raise AMCException('{} doesnt exist or is not a file'.format(filepath))
        
    
    def _build_environ(self, environ):
        
        env = os.environ.copy()
        env.update(environ or {})

        for k,v in env.items():
            env[k] = str(v)
        
        self.logger.debug('ENVIRON:\n%s', json.dumps(env, indent=2))
        self.environ = env
        
    def _get_amc_exec(self):
        def make_c(ip_port):
            if isinstance(ip_port, str):
                ip_port = [ip_port]
            s = []
            for x in ip_port:
                s.append('-c {0}'.format(x))
            return ' '.join(s)
        
        return '{0} {1}'.format(self.amc_tool_binary, make_c(self.ip_or_port))
            
    def _exec(self, cmd_line, additional_env=None, stdin=None):
        
        if additional_env:
            env = self.environ.copy()
            for k,v in (additional_env or {}).items():
                env[k] = str(v)
        else:
            env = self.environ
        
        proc = subprocess.Popen(cmd_line, env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True) # I'm assuming our code can be trusted
        def terminator(proc):
            if proc.poll() is None:
                return
            proc.kill()
            outs, errs = proc.communicate(stdin)
            raise AMCRuntimeException('{0} failed with {1}\n{2}'.format(cmd_line, outs, errs))
        
        timer = threading.Timer(self._timeout, terminator, (proc,))
        outs, errs = proc.communicate()
        timer.cancel()
        
        return (proc.returncode, outs, errs)