#!/usr/bin/env python

import geopy
import os
import sqlite3
import sys
import time

sys.path.insert(0, "%s/work/cp-mec/util" % os.path.expanduser("~"))
import Cons
import Util

import Conf


def main(argv):
  Conf.Init()
  Plot()


def Plot():
  fn_cf = Conf.Get("cf_locs")
  fn_wf = GetWfLocFile()
  fn_out = "%s/cloudfront-wholefoods-locations.pdf" % Conf.DnOut()

  with Cons.MT("Plotting Whole Foods store locations ..."):
    env = os.environ.copy()
    env["FN_CF"] = fn_cf
    env["FN_WF"] = fn_wf
    env["FN_OUT"] = fn_out
    Util.RunSubp("gnuplot %s/edge-server-locs.gnuplot" % os.path.dirname(__file__), env=env)
    Cons.P("Created %s %d" % (fn_out, os.path.getsize(fn_out)))


def GetWfLocFile():
  fn_out = "%s/whole-foods-locations" % Conf.DnOut()
  if os.path.exists(fn_out):
    return fn_out

  fn_in = Conf.Get("wf_addrs")
  with Cons.MT("Adding store locations from %s" % fn_in):
    addrs = set()
    num_dups = 0
    with open(fn_in) as fo:
      for line in fo:
        addr = line.strip()
        if len(addr) == 0:
          continue
        if addr[0] == "#":
          continue
        if addr in addrs:
          #Cons.P("Dup: %s" % addr)
          num_dups += 1
        else:
          addrs.add(addr)
    Cons.P("Filtered out %d duplicate addresses" % num_dups)

    db = WfLocDb()

    geolocator = geopy.Nominatim()

    for addr in sorted(addrs):
      if db.Exist(addr):
        continue
      loc = None
      try:
        loc = geolocator.geocode(addr)
      except geopy.exc.GeocoderTimedOut as e:
        Cons.P("%s [%s]" % (e, addr))
        sys.exit(1)

      if loc is None:
        raise RuntimeError("Unexpected: [%s]" % addr)

      Cons.P("\"%s\" %f %f\n" % (addr, loc.latitude, loc.longitude))
      db.Insert(addr, loc.latitude, loc.longitude)

      # At most 1 req per second
      #   https://operations.osmfoundation.org/policies/nominatim
      time.sleep(1)

  fn_in = Conf.Get("wf_addrs_googlemaps")
  with Cons.MT("Adding store locations from %s" % fn_in):
      with open(fn_in) as fo:
        for line in fo:
          #Cons.P(line)
          t = line.strip().split(" ")
          addr = " ".join(t[0:-1])
          t1 = t[-1].split(",")
          lat = t1[0]
          lon = t1[1]
          #Cons.P("%s|%s,%s" % (addr, lat, lon))

          if db.Exist(addr):
            continue
          db.Insert(addr, lat, lon)

  with open(fn_out, "w") as fo:
    fo.write("# addr lon lat\n")
    for r in db.GetAll():
      fo.write("\"%s\" %f %f\n" % (r["addr"], r["lat"], r["lon"]))
  Cons.P("Created %s %d" % (fn_out, os.path.getsize(fn_out)))
  return fn_out


class WfLocDb:
  def __init__(self):
    self.conn = None

    # Open or create DB
    fn_db = Conf.GetFn("whole_foods_loc_db")
    conn = None
    if os.path.exists(fn_db):
      with Cons.MT("Opening the existing db ..."):
        conn = sqlite3.connect(fn_db)
        if conn is None:
          raise RuntimeError("Error! cannot create the database connection.")
        conn.row_factory = sqlite3.Row
        cur = conn.cursor()
        q = "SELECT count(*) as cnt FROM whole_foods_loc"
        cur.execute(q)
        r = cur.fetchone()
        Cons.P("There are %d records" % r["cnt"])
    else:
      with Cons.MT("Creating a new db ..."):
        conn = sqlite3.connect(fn_db)
        if conn is None:
          raise RuntimeError("Error! cannot create the database connection.")
        conn.row_factory = sqlite3.Row
        cur = conn.cursor()
        q = """CREATE TABLE IF NOT EXISTS whole_foods_loc (
                 addr text NOT NULL
                 , lat real NOT NULL
                 , lon real NOT NULL
                 , PRIMARY KEY (addr)
               ); """
        cur.execute(q)
    self.conn = conn

  def Exist(self, addr):
    cur = self.conn.cursor()
    cur.execute("SELECT count(*) as cnt FROM whole_foods_loc WHERE addr = '%s'" % addr)
    r = cur.fetchone()
    return (r["cnt"] == 1)

  def Insert(self, addr, lat, lon):
    cur = self.conn.cursor()
    q = "INSERT INTO whole_foods_loc (addr, lat, lon) VALUES (?,?,?)"
    cur.execute(q, (addr, lat, lon))
    self.conn.commit()

  def GetAll(self):
    cur = self.conn.cursor()
    cur.execute("SELECT addr, lat, lon FROM whole_foods_loc")
    return cur.fetchall()


if __name__ == "__main__":
  sys.exit(main(sys.argv))
