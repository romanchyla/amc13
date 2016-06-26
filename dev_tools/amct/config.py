import os, sys

# Some config paths, do not change unless you know what you are doing...
AMCT_ROOT = os.path.abspath(os.path.join(os.path.abspath(__file__), '../'))
DEVTOOLS_ROOT = os.path.abspath(os.path.join(os.path.abspath(__file__), '../../'))

DEFAULT_TESTSUITE_DIR = DEVTOOLS_ROOT + '/testsuites'

# IP address of an AMC card that will be tested (unless overriden using options)
DEFAULT_IP = '192.168.1.83'

# IP of the machine that serves as a hub to AMC cards. It is used by the ipmitool
DEFAULT_CRATE_IP = '192.168.1.41'

# The card that will be tested (if no option is supplied that overrides it)
DEFAULT_PORT = '086/c'


DEFAULT_CONNECTION_XML = 'TODO'

# If not test-suite is specified, this one will be ran
DEFAULT_TEST_SUITE = 'hello'

#a command to invoke AMC13Tool.exe
DEFAULT_AMC_TOOL = 'locale;LD_LIBRARY_PATH=/home/semiray/python_update/amc13/tools/lib/:/home/semiray/python_update/amc13/amc13/lib/:/opt/cactus/lib/; AMC13Tool2.exe'

LOGGING = {
    'version': 1,
    'disable_existing_loggers': False,
    'formatters': {
        'default': {
            'format': '%(levelname)s\t%(process)d '
                      '[%(asctime)s]:\t%(message)s',
            'datefmt': '%m/%d/%Y %H:%M:%S',
        }
    },
    'handlers': {
        'file': {
            'formatter': 'default',
            'level': 'INFO',
            'class': 'logging.handlers.TimedRotatingFileHandler',
            'filename': 'amct.log'
        },
        'console': {
            'formatter': 'default',
            'level': 'CRITICAL',
            'class': 'logging.StreamHandler'
        }
    },
    'loggers': {
        '': {
            'handlers': ['file', 'console'],
            'level': 'DEBUG',
            'propagate': True,
        },
    },
}
