#!/usr/bin/env python

import datetime
import math
import multiprocessing
import os
import pprint
import re
import sys

sys.path.insert(0, "%s/work/mutant/ec2-tools/lib/util" % os.path.expanduser("~"))
import Cons
import Util

import Conf


def main(argv):
  Conf.ParseArgs()
  Util.MkDirs(Conf.GetDir("output_dir"))

  #dist_sq_thresholds = [
  #    0
  #    , 0.008
  #    , 0.01
  #    , 0.02
  #    , 0.05]

  # Still quite big. 277 KB
  dist_sq_thresholds = [0.02]

  dist_sq_thresholds_str = []
  for d in dist_sq_thresholds:
    dist_sq_thresholds_str.append(_NoTrailing0s(d))

  fns_co_loc = []
  co_loc_file_sizes = []
  with Cons.MT("Generating reduced central office locations ..."):
    for d in dist_sq_thresholds_str:
      fn_co_loc = "filter-out-almost-duplicate-points/.output/centraloffices-wo-almost-dup-points-%s" % d
      fns_co_loc.append(fn_co_loc)
      if not os.path.exists(fn_co_loc):
        cmd = "cd filter-out-almost-duplicate-points && ./build-and-run.sh --dist_sq_threshold=%s" % d
        Util.RunSubp(cmd)
      co_loc_file_sizes.append(os.path.getsize(fn_co_loc))

  dn_out = "%s/.output" % os.path.dirname(__file__)
  fn_out = "%s/central-office-locations.pdf" % dn_out
  with Cons.MT("Plotting ..."):
    env = os.environ.copy()
    env["IN_FNS"] = " ".join(fns_co_loc)
    env["IN_FN_SIZES"] = " ".join(str(s) for s in co_loc_file_sizes)
    env["DIST_SQ_THRESHOLDS"] = " ".join(dist_sq_thresholds_str)
    env["OUT_FN"] = fn_out
    Util.RunSubp("gnuplot %s/central-office-on-map.gnuplot" % os.path.dirname(__file__), env=env)
    Cons.P("Created %s %d" % (fn_out, os.path.getsize(fn_out)))

  sys.exit(0)


  if False:
    # Parallel processing
    params = []
    for line in re.split(r"\s+", exps):
      t = line.split("/quizup/")
      if len(t) != 2:
        raise RuntimeError("Unexpected")
      job_id = t[0]
      exp_dt = t[1]
      params.append((job_id, exp_dt))
    p = multiprocessing.Pool(8)
    p.map(Plot, params)
  else:
    for line in re.split(r"\s+", exps):
      t = line.split("/quizup/")
      if len(t) != 2:
        raise RuntimeError("Unexpected")
      job_id = t[0]
      exp_dt = t[1]
      Plot((job_id, exp_dt))


def _NoTrailing0s(d):
  d_str = "%f" % d
  while True:
    mo = re.match(r".+\.\d+0+$", d_str)
    if mo is None:
      break
    d_str = d_str[:-1]
  return d_str


def Plot(param):
  job_id = param[0]
  exp_dt = param[1]
  dn_log_job = "%s/work/mutant/log/quizup/sla-admin/%s" % (os.path.expanduser("~"), job_id)

  fn_log_quizup  = "%s/quizup/%s" % (dn_log_job, exp_dt)
  fn_log_rocksdb = "%s/rocksdb/%s" % (dn_log_job, exp_dt)
  fn_log_dstat   = "%s/dstat/%s.csv" % (dn_log_job, exp_dt)

  log_q = QuizupLog(fn_log_quizup)
  SimTime.Init(log_q.SimTime("simulated_time_begin"), log_q.SimTime("simulated_time_end")
      , log_q.SimTime("simulation_time_begin"), log_q.SimTime("simulation_time_end"))

  qz_std_max = _QzSimTimeDur(log_q.quizup_options["simulation_time_dur_in_sec"])
  qz_opt_str = _QuizupOptionsFormattedStr(log_q.quizup_options)
  error_adj_ranges = log_q.quizup_options["error_adj_ranges"].replace(",", " ")

  (fn_rocksdb_sla_admin_log, pid_params, num_sla_adj) = RocksdbLog.ParseLog(fn_log_rocksdb, exp_dt)

  fn_dstat = DstatLog.GenDataFileForGnuplot(fn_log_dstat, exp_dt)

  fn_out = "%s/sla-admin-by-time-%s.pdf" % (Conf.GetDir("output_dir"), exp_dt)

  with Cons.MT("Plotting ..."):
    env = os.environ.copy()
    env["STD_MAX"] = qz_std_max
    env["ERROR_ADJ_RANGES"] = error_adj_ranges
    env["IN_FN_QZ"] = fn_log_quizup
    env["IN_FN_SLA_ADMIN"] = "" if num_sla_adj == 0 else fn_rocksdb_sla_admin_log
    env["QUIZUP_OPTIONS"] = qz_opt_str
    env["PID_PARAMS"] = "%s %s %s %s" % (pid_params["target_value"], pid_params["p"], pid_params["i"], pid_params["d"])
    env["WORKLOAD_EVENTS"] = " ".join(str(t) for t in log_q.simulation_time_events)
    env["IN_FN_DS"] = fn_dstat
    env["OUT_FN"] = fn_out
    Util.RunSubp("gnuplot %s/sla-admin-by-time.gnuplot" % os.path.dirname(__file__), env=env)
    Cons.P("Created %s %d" % (fn_out, os.path.getsize(fn_out)))


# List the options in 2 columns, column first.
def _QuizupOptionsFormattedStr(quizup_options):
  max_width = 0
  i = 0
  num_rows = int(math.ceil(len(quizup_options) / 2.0))
  for k, v in sorted(quizup_options.iteritems()):
    max_width = max(max_width, len("%s: %s" % (k, v)))
    i += 1
    if i == num_rows:
      break

  strs = []
  fmt = "%%-%ds" % (max_width + 1)
  for k, v in sorted(quizup_options.iteritems()):
    strs.append(fmt % ("%s: %s" % (k, v)))
  #Cons.P("\n".join(strs))
  for i in range(num_rows):
    if i + num_rows < len(strs):
      strs[i] += strs[i + num_rows]
  strs = strs[:num_rows]
  #Cons.P("\n".join(strs))
  qz_opt_str = "\\n".join(strs).replace("_", "\\\\_").replace(" ", "\\ ")
  #Cons.P(qz_opt_str)
  return qz_opt_str


# Simulation time duration
def _QzSimTimeDur(std):
  s = int(std)
  std_s = s % 60
  std_m = int(s / 60) % 60
  std_h = int(s / 3600)
  return "%02d:%02d:%02d" % (std_h, std_m, std_s)


if __name__ == "__main__":
  sys.exit(main(sys.argv))
