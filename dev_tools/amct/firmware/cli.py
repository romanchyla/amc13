"""AMCT module for testing AMC13 firmware."""

import click
import tabulate
from amct import utils, config
from amct.firmware import utils


G = {}

@click.group(name='firmware')
@click.pass_obj
def cli(config):
    """Operations for testing AMC13 firmware."""
    G.update(config)
    utils.fail_fast()


@cli.command()
@click.option('-c', '--xml_config', default=config.DEFAULT_CONNECTION_XML)
def version(xml_config):
    v = utils.get_amc_version(xml_config)
    print tabulate.tabulate([['T1 firmware version', 'T2 firmware version'], v], headers='firstrow')
    
