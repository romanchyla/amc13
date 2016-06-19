import click
import os
import amct
import json
from amct import utils, config
from amct.firmware import cli as firmware_cli
from amct.testsuite import cli as testsuite_cli

"""The main command line interface to AMCT; here we initialize and
set global defaults. Each of the modules of the AMCT have their 
own CLI commands. They are added through <module>.cli
"""

CONTEXT_SETTINGS = dict(help_option_names=['-h', '--help'],
                        auto_envvar_prefix='AMCT')

@click.group(context_settings=CONTEXT_SETTINGS)
@click.option('-v', '--verbose', count=True)
@click.option('-d', '--debug', default=False, is_flag=True)
@click.option('-l', '--log-level', type=click.Choice(['info', 'debug']))
@click.option('-c', '--config', type=click.Path(exists=True, 
                    readable=True), help="Global configuration",
                    default=lambda: os.path.join(os.path.dirname(amct.__file__), './config.py')
              )
@click.version_option()
@click.pass_context
def cli(ctx, verbose, debug, log_level, config):
    conf = utils.load_config(config)
    if debug:
        print 'Config loaded from {0}'.format(config)
        print json.dumps(conf, indent=2)
    conf.setdefault('CONFIG', debug)
    conf.setdefault('VERBOSE', verbose)
    conf.setdefault('LOG_LEVEL', log_level)
    
    ctx.obj = conf

# expose nested sections
cli.add_command(firmware_cli.cli)
cli.add_command(testsuite_cli.cli)


