#include "cmdline/cmdline.h"

#include "memory_hierarchy.h"
#include "trace_loader.h"
#include "cfg_loader.h"

#include <iostream>

void run_simulation(string cfg, string trace_cfg, unsigned int processes) {
  auto cfg_loader = CfgLoaderObj::get_instance();
  auto builder = PipeLineBuilderObj::get_instance();
  auto trace_cfg_loader = TraceCfgLoaderObj::get_instance();
  auto trace_loader = MultiTraceLoaderObj::get_instance();

  // 1. load trace file 
  trace_cfg_loader->parse(trace_cfg);
  auto traces = trace_cfg_loader->get_traces();
  for (auto &trace: traces) {
    trace_loader->adding_trace(trace);
  }
  if (traces.size() != processes) {
    SIMLOG(SIM_ERROR, "process number should equals to trace files\n");
    exit(1);
  }
 
  // 2. load architecture
  cfg_loader->parse(cfg);
  builder->load(cfg_loader->get_nodes());

  auto connectors = builder->get_connectors();
  if (connectors.size() < processes) {
    SIMLOG(SIM_WARNING, "available cores is fewer than processes, \
           only %d processes will be simulated\n", (int)connectors.size());
  }
  
  for (unsigned int i = 0; i < processes; i++) {
    connectors[i]->start();
  }

  EventEngine *evnet_queue = EventEngineObj::get_instance();
  while (true) {
    auto ret = evnet_queue->loop();
    if (ret == 0)
      break;
  }
  // print stats
  auto stats_manager = MemoryStatsManagerObj::get_instance();
  stats_manager->display_all(stdout);
}

int main(int argc, char *argv[])
{
  cmdline::parser a;
  a.add<string>("cfg", 'c', "configuration file in json format", true, "");
  a.add<string>("trace", 't', "trace configuration file in json format", true, "");
  a.add<unsigned int>("process", 'p', "processes to simulate", true, 0, cmdline::range(1, 8));
  a.add("verbose", 'v', "verbose output");

  a.parse_check(argc, argv);

  run_simulation(a.get<string>("cfg"), 
                 a.get<string>("trace"),
                 a.get<unsigned int>("process"));
  return 0;
}
