#!/usr/bin/env python

import subprocess, json, os
from multiprocessing import Pool

POLICIES = ["LRU", "Random", "LIP", "BIP", "DIP"]
TRACES = ["../traces/h264ref_178B.trace.gz",
          "../traces/bzip2_183B.trace.gz",
          "../traces/astar_23B.trace.gz",
          "../traces/milc_744B.trace.gz",
          "../traces/omnetpp_4B.trace.gz",
          "../traces/gcc_13B.trace.gz"]

RESULT_DIR = "result"

LOCAL_CFG_FILE = "cfg.json"
LOCAL_TRACE_CFG_FILE = "traces.json"

TEMPLATE_CFG_FILE = "../cfg/cfg.json"
TEMPLATE_TRACE_CFG_FILE = "../cfg/traces.json"

def set_policy_lambda(policy):
    def f(node):
        if node['name'].find('L2') != -1:
            node['policy'] = policy
    return f

def create_cfg_file(filename, policy):
    with open(TEMPLATE_CFG_FILE, "r") as f:
        buf = f.read()
        c = json.loads(buf)
        map(set_policy_lambda(policy), c["nodes"])
        with open(filename, "w") as wf:
            wf.write(json.dumps(c))

def create_trace_cfg_file(filename, traces):
    with open(TEMPLATE_TRACE_CFG_FILE, "r") as f:
        buf = f.read()
        c = json.loads(buf)
        c["traces"] = traces
        with open(filename, "w") as wf:
            wf.write(json.dumps(c))

def run(policy, traces, sub_dir):
    create_cfg_file(policy+LOCAL_CFG_FILE, policy)
    create_trace_cfg_file(policy+LOCAL_TRACE_CFG_FILE, traces)
    batcmd="./lightsim -c %s -t %s -p %d -n 20000000" % \
            (policy+LOCAL_CFG_FILE, policy+LOCAL_TRACE_CFG_FILE, len(traces))
    result = subprocess.check_output(batcmd, shell=True)
    os.remove(policy+LOCAL_CFG_FILE)
    os.remove(policy+LOCAL_TRACE_CFG_FILE)

    result_file = '/'.join([sub_dir, policy])
    with open(result_file, "w") as f:
        f.write(result)

def run_stand_alone_trace(trace):
    trace_name = trace.split("/")[-1].split(".")[0]

    if not os.path.exists(RESULT_DIR):
        os.makedirs(RESULT_DIR)
        
    sub_dir = '/'.join([RESULT_DIR, trace_name+"_alone"])
    if os.path.exists(sub_dir):
        print ("%s exists, delete the dir first"%sub_dir)
        return
    os.makedirs(sub_dir)

    pool = Pool(len(POLICIES))
    asyncret = []
    for policy in POLICIES:
        asyncret.append(pool.apply_async(run, (policy, [trace], sub_dir)))

    for ret in asyncret:
        ret.get()

def run_duo_trace(trace1, trace2):
    trace1_name = trace1.split("/")[-1].split(".")[0]
    trace2_name = trace2.split("/")[-1].split(".")[0]

    if not os.path.exists(RESULT_DIR):
        os.makedirs(RESULT_DIR)
        
    sub_dir = '/'.join([RESULT_DIR, trace1_name + "_with_" + trace2_name])
    if os.path.exists(sub_dir):
        print ("%s exists, delete the dir first"%sub_dir)
        return
    os.makedirs(sub_dir)

    pool = Pool(len(POLICIES))
    asyncret = []
    for policy in POLICIES:
        asyncret.append(pool.apply_async(run, (policy, [trace1, trace2], sub_dir)))

    for ret in asyncret:
        ret.get()
    

if __name__ == "__main__":
    run_stand_alone_trace("../traces/gcc_13B.trace.gz")
    for trace in TRACES:
        run_stand_alone_trace(trace)
        # run_duo_trace("../traces/gcc_13B.trace.gz", trace)
