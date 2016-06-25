#!/usr/bin/env python
#
# Fetch virtualenv.py securely with dependencies and run
# it standalone.
#
# This stuff is placed into public domain by
#    anatoly techtonik <techtonik@gmail.com>

# --- bootstrap .locally --
#
# this creates .locally/ subdirectory in the script's dir
# and sets a few global variables for convenience:
#
#   ROOT  - absolute path to script dir
#   LOOT  - absolute path to the .locally/ subdir
#
# this provides some helpers:
#
#   wgetsecure(targetdir, filespec, quiet=False)
#                    - download file and check hash/size

filespec = [
  # files needed for standalone virtualenv operation
  dict(
    # filename to save download to (not autodetected for security purpose)
    filename='pip-6.0.8-py2.py3-none-any.whl',
    # sha1 hash and size
    hashsize='de677018acd9707e2d4f1f11d2525ac6f833dcd8 1266491',
    url='https://pypi.python.org/packages/py2.py3/p/pip/pip-6.0.8-py2.py3-none-any.whl',
  ),

  dict(
    filename='setuptools-12.0.5-py2.py3-none-any.whl',
    hashsize='4a6cbc64f81f7db3d8f3f9144b79ae10f24daa2d 502329',
    url='https://pypi.python.org/packages/3.4/s/setuptools/setuptools-12.0.5-py2.py3-none-any.whl',
  ),

  dict(
    filename='virtualenv.py',
    hashsize='2beb129ca814c8604ce97b90a205ba805a069e48 99038',
    url='https://raw.githubusercontent.com/pypa/virtualenv/12.0.7/virtualenv.py',
  ),
]


# --- create .locally/ subdir ---
import os
import sys

ROOT = os.path.abspath(os.path.dirname(__file__))
LOOT = os.path.join(ROOT, '.locally/')
if not os.path.exists(LOOT):
  os.mkdir(LOOT)


# ---[ utilities ]---

from hashlib import sha1
from os.path import exists, getsize, join
import urllib
import os

def hashsize(path):
  '''
  Generate SHA-1 hash + file size string for the given
  filename path. Used to check integrity of downloads.
  Resulting string is space separated 'hash size':

    >>> hashsize('locally.py')
    'fbb498a1d3a3a47c8c1ad5425deb46b635fac2eb 2006'
  '''
  size = getsize(path)
  h = sha1()
  with open(path, 'rb') as source:
    while True:
      c = source.read(64*1024)  # read in 64k blocks
      if not c:
        break
      h.update(c)
  return '%s %s' % (h.hexdigest(), size)

class HashSizeCheckFailed(Exception):
  '''Throw when downloaded file fails hash and size check.'''
  pass

def getsecure(targetdir, filespec, quiet=False):
  '''
  Using description in `filespec` list, download
  files from specified URL (if they don't exist)
  and check that their size and sha-1 hash matches.

  Files are downloaded into `targetdir`. `filespec`
  is a list of entries, each entry is a dictionary
  with obligatory keys: filename, hashsize and url.

    filespec = [ {
      'filename': 'wget.py',
      'hashsize': '4eb39538d9e9f360643a0a0b17579f6940196fe4 12262',
      'url': 'https://bitbucket.org/techtonik/python-wget/raw/2.0/wget.py'
    } ]

  Raises HashSizeCheckFailed if hash/size check
  fails. Set quiet to false to skip printing
  progress messages.
  '''
  # [-] no rollback
  def check(filepath, shize, quiet):
    if not quiet:
      print('Checking hash/size for %s' % filepath)
    if hashsize(filepath) != shize:
      raise HashSizeCheckFailed(
                'Hash/Size mismatch for %s\n  exp: %s\n  act: %s'
                % (filepath, shize, hashsize(filepath)))

  for entry in filespec:
    filepath = join(targetdir, entry['filename'])
    downloaded = False
    if exists(filepath):
      if not quiet:
        print("Downloading " + entry['filename'] + " skipped (already downloaded).")
    else:
      if not quiet:
        print("Downloading %s into %s" % (entry['filename'], targetdir))
      urllib.urlretrieve(entry['url'], filepath)
      downloaded = True

    if not entry['hashsize']:
      if not quiet:
        print("Hash/size is not set, check skipped..")
    else:
      try:
        check(filepath, entry['hashsize'], quiet)
      except HashSizeCheckFailed:
        if downloaded:
          # [x] remove file only if it was just downloaded
          os.remove(filepath)
        raise

# ---[ /utils ]---

#--[inline shellrun 2.0 import run]
import subprocess

class Result(object):
    def __init__(self, command=None, retcode=None, output=None):
        self.command = command or ''
        self.retcode = retcode
        self.output = output
        self.success = False
        if retcode == 0:
            self.success = True

def run(command):
    process = subprocess.Popen(command, shell=True)
    process.communicate()
    return Result(command=command, retcode=process.returncode)
#--[/inline]


# ---[ download dependencies ]---
print("Fetching stuff into .locally/")

getsecure(LOOT, filespec)

# ---[ run virtualenv.py ]---

python = sys.executable
cmd = '"%s" %s/virtualenv.py --extra-search-dir="%s" --always-copy' % (python, LOOT, LOOT)
cmd += ' ' + ' '.join(sys.argv[1:])
print("Executing: " + cmd)
run(cmd)
print("Done.")
