"""For running set of tests against AMC13."""

import click
import tabulate
from amct import utils, config
from amct.testsuite.runner import AMCStandardRunner
from amct.firmware.utils import get_ip_from_slot
from tabulate import tabulate
G = {}

@click.group(name='testsuite')
@click.pass_obj
def cli(config):
    """Run predefined test-suites against AMC13 boards."""
    G.update(config)


@cli.command()
@click.option('-n', '--name', default=config.DEFAULT_TEST_SUITE)
@click.option('-i', '--ip', default=None)
@click.option('-x', '--port', default=None)
@click.option('-s', '--slot', default=None, type=int)
def run(name, ip, port, slot):
    """Run test-suite against AMC13. The tests are to be found
    inside `ROOT/testsuites` folder."""
    #G['DEBUG']= True
    if slot:
        hexIP,decIP = get_ip_from_slot(config.DEFAULT_CRATE_IP, slot)
        ip = decIP
    print ip, port,
    # the idea is to create a test runner, it will discover all
    # files from a dedicated folder; those files should be amc
    # commands
    runner = AMCStandardRunner(G, name, ip, port)
    runner.run()
    #log = open("deneme.txt", "w")
    #print >>log, runner.results
<<<<<<< HEAD
    #print runner.results
    print tabulate(runner.results[0])
    #print tabulate.tabulate(runner.results[0])
=======
    print runner.results
    #print tabulate(runner.results)
    #print tabulate.tabulate(runner.results)
>>>>>>> 658a836b63c05ee62af57addbaac22d79f34baac
    # if we got here, it means the test was successful
    if not G.get('verbose'):
        runner.cleanup()
        print 'Testsuite {0} finished OK.'
    else:
        print 'Testsuite {0} finished OK, results can be found in: {1}'.format(name, runner.tmpdir)
    
