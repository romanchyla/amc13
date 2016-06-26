import click
import os
import amct
import json
import tempfile
from amct import utils, config
from amct.firmware import cli as firmware_cli
from amct.testsuite import cli as testsuite_cli
import logging
from logutils import dictconfig

"""The main command line interface to AMCT; here we initialize and
set global defaults. Each of the modules of the AMCT have their 
own CLI commands. They are added through <module>.cli
"""

CONTEXT_SETTINGS = dict(help_option_names=['-h', '--help'],
                        auto_envvar_prefix='AMCT')

@click.group(context_settings=CONTEXT_SETTINGS)
@click.option('-v', '--verbose', count=True)
@click.option('-d', '--debug', default=False, is_flag=True)
@click.option('-l', '--log-level', default='ERROR',
              type=click.Choice(['INFO', 'DEBUG', 'WARNING', 'ERROR', 'CRITICAL']))
@click.option('-c', '--config', type=click.Path(exists=True, 
                    readable=True), help="Global configuration",
                    default=lambda: os.path.join(os.path.dirname(amct.__file__), './config.py')
              )
@click.version_option()
@click.pass_context
def cli(ctx, verbose, debug, log_level, config):
    class MyDict(dict):
        pass
    conf = utils.load_config(config)
    conf.setdefault('CONFIG', debug)
    conf.setdefault('VERBOSE', verbose)
    
    
    conf.setdefault('LOG_LEVEL', log_level)
    
    log_config = conf.get('LOGGING', {})
    if verbose or debug:
        log_level = debug and 'DEBUG' or 'INFO'
        for k,v in log_config.get('handlers', {}).items():
            v['level'] = log_level
    dictconfig.dictConfig(log_config)
    
    conf = MyDict(conf)
    conf.logger = logging.getLogger(__name__)
    
    ctx.obj = conf
    if debug:
        conf.logger.debug('Config loaded from {0}'.format(config))
        conf.logger.debug(json.dumps(conf, indent=2))

# expose nested sections
cli.add_command(firmware_cli.cli)
cli.add_command(testsuite_cli.cli)


