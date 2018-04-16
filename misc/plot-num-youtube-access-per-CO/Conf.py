import argparse
import os
import pprint
import sys
import yaml

sys.path.insert(0, "%s/work/cp-mec/util" % os.path.expanduser("~"))
import Cons
import Util


_args = None
_dn_out = None


def Init():
  global _dn_out
  _dn_out = "%s/.output" % os.path.dirname(__file__)
  Util.MkDirs(_dn_out)

  # Load yaml
  fn = "%s/config.yaml" % os.path.dirname(__file__)
  yaml_root = None
  with open(fn) as fo:
    yaml_root = yaml.load(fo)
  #Cons.P(pprint.pformat(yaml_root))

  # Override the yaml configuration when specified
  parser = argparse.ArgumentParser(
      #description="Desc",
      formatter_class=argparse.ArgumentDefaultsHelpFormatter)

  if yaml_root:
    for k, v in yaml_root.iteritems():
      parser.add_argument("--%s" % k
          , type=str
          , default=(v)
          #, help="desc"
          )

  global _args
  _args = parser.parse_args()
  # Cons.P("Parameters:")
  # for a in vars(_args):
  #   Cons.P("%s: %s" % (a, getattr(_args, a)), ind=2)


def Get(k):
  return getattr(_args, k)


def GetFn(k):
  return Get(k).replace("~", os.path.expanduser("~"))


def DnOut():
  return _dn_out
