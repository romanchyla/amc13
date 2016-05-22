import uhal
from _amc13_python import *



def _exception_to_string(self):
    # what returns the exception string with one \n too many
    return self.what

exBase.__str__ = _exception_to_string


