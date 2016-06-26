"""For running set of tests against AMC13."""

import click
from amct import config
from amct.testsuite.runner import AMCStandardRunner
from amct.firmware.utils import get_ip_from_slot
from tabulate import tabulate
import os

@click.group(name='testsuite')
@click.pass_obj
def cli(config):
    """Run predefined test-suites against AMC13 boards."""
    pass


@cli.command()
@click.pass_obj
def list(ctx):
    """List the `ROOT/testsuites` folder."""
    tests = []
    i = 0
    for x in os.listdir(ctx.get('DEFAULT_TESTSUITE_DIR')):
        if os.path.isdir(os.path.join(ctx.get('DEFAULT_TESTSUITE_DIR'), x)):
            readme = os.path.join(ctx.get('DEFAULT_TESTSUITE_DIR'), x, 'readme.txt')
            if os.path.exists(readme):
                with open(readme, 'r') as f:
                    desc = f.read().strip()
            else:
                desc = ''
            tests.append(('%02d' % i, x, desc))
            i += 1
    tests = sorted(tests, key=lambda x: x[1])
    ctx.logger.info('Listing testsuites from: {0}'.format(ctx.get('DEFAULT_TESTSUITE_DIR')))
    print tabulate(tests, headers=['#', 'name', 'description'])


@cli.command()
@click.option('-n', '--name', default=config.DEFAULT_TEST_SUITE,
              help="Name of the suite to execute, eg. hello")
@click.option('-i', '--ip', default=None,
              help='IP address of the AMC card. Can be a comma separated list.')
@click.option('-x', '--port', default=None,
              help='Port of the AMC card. Can be a comma separated list.')
@click.option('-s', '--slot', default=None,
              help='Slots of the AMC card. Can be a comma separated list.')
@click.pass_obj
def run(ctx, name, ip, port, slot):
    """Run test-suite against AMC13. The tests are to be found
    inside `ROOT/testsuites` folder."""
    
    if slot: # translate slots into IPs
        ip = []
        for x in slot.split(','):
            _, dec_ip = get_ip_from_slot(ctx.get('DEFAULT_CRATE_IP'), int(x))
            ip.append(dec_ip)
    
    if not ip and not port:
        raise Exception('At least one of --ip or --port or --slot must be used')
    
    # the idea is to create a test runner, it will discover all
    # files from a dedicated folder; those files should be amc
    # commands
    runner = AMCStandardRunner(ctx, name, ip=ip, port=port)
    runner.run()
    
    # if we got here, it means the test was successful
    ctx.logger.info('Testsuite {0} finished OK'.format(name))
    
