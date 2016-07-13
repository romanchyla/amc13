
import logging
import os
import subprocess
import json
import threading

from ..exceptions import AMCException, AMCRuntimeException

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

    
    def execute_code(self, code, filename):
        """Executes python source code, after having extrapolated
        its values. This can be used to run arbitrary tests.
        
        NOTE: we are assuming that the code can be trusted, be
        very careful not to expose this eval() functionality to
        untrusted clients.
        
        """
        
        self.logger.debug('Code before extrapolation:', code)
        code = code.format(self.environ)
        self.logger.debug('Code after extrapolation:', code)
        bytes = compile(code, filename, 'exec')
        
        # this will be evaluated inside our current environment
        # so the script can have access to the self object
        
        amc = self
        eval(bytes)
        
    
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