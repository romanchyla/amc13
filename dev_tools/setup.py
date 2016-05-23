from setuptools import setup
import os

setup(name='amct',
      version='0.0.1',
      packages=['amct'],
      description='Testing tools for AMC13',
      install_requires=[
          'Click==6.2',
          'netifaces==0.10.4',
          'tabulate==0.7.5',
          'requests==2.9.1',
      ],
      entry_points={
          'console_scripts': [
              'amct = amct.cli:cli',
          ]
      },
  )
