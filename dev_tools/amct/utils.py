import subprocess
import os
import imp
import sys

def load_config(local_config=None):
    """
    Loads configuration from config.py and also from local_config.py

    :return dictionary
    """
    conf = {}
    PROJECT_HOME = os.path.abspath(os.path.join(os.path.dirname(__file__), './'))
    if PROJECT_HOME not in sys.path:
        sys.path.append(PROJECT_HOME)
    conf['PROJ_HOME'] = PROJECT_HOME

    conf.update(load_module(os.path.join(PROJECT_HOME, 'config.py')))
    if local_config:
        conf.update(load_module(local_config))

    return conf

def load_module(filename):
    """
    Loads module, first from config.py then from local_config.py

    :return dictionary
    """

    filename = os.path.join(filename)
    d = imp.new_module('config')
    d.__file__ = filename
    try:
        with open(filename) as config_file:
            exec(compile(config_file.read(), filename, 'exec'), d.__dict__)
    except IOError as e:
        pass
    res = {}
    from_object(d, res)
    return res

def from_object(from_obj, to_obj):
    """Updates the values from the given object.  An object can be of one
    of the following two types:
    Objects are usually either modules or classes.
    Just the uppercase variables in that object are stored in the config.
    :param obj: an import name or object
    """
    for key in dir(from_obj):
        if key.isupper():
            to_obj[key] = getattr(from_obj, key)
            
def cmd(cmd, input=None):
    p = subprocess.Popen(cmd, shell=True,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        close_fds=True)
    if input:
        p.communicate(input)
    retcode = p.wait()

    class Object(object):
        def __str__(self):
            return str(dict(cmd=self.cmd, out=self.out, err=self.err,
                retcode=self.retcode))

    out = Object()
    setattr(out, 'cmd', cmd)
    setattr(out, 'out', p.stdout.read())
    setattr(out, 'err', p.stderr.read())
    setattr(out, 'retcode', retcode)

    p.stdout.close()
    p.stderr.close()
    p.stdin.close()

    if retcode:
        raise Exception(dict(cmd=cmd, out=out.out, err=out.err, retcode=out.retcode))

    return out
