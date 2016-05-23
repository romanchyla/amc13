"""For running set of tests against AMC13."""

import click
import tabulate
from amct import utils, config


G = {}

@click.group(name='testsuite')
@click.pass_obj
def cli(config):
    """Run predefined test-suites against AMC13 boards."""
    G.update(config)


@cli.command()
@click.option('-n', '--name', default=config.DEFAULT_TEST_SUITE)
def run(name):
    """Run test-suite against AMC13. The tests are to be found
    inside `ROOT/testsuites` folder."""
    pass
    
