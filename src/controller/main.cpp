#include "Controller.h";
using namespace IOStormPlus;

int main(int argc,char *argv[]) {
    Controller Controller_instance(agents_config_file);
    if (Controller_instance.has_error())
        return 0;

    if (argc == 1)
        Controller_instance.PrintUsage();
    else if (strcmp(argv[1], "agent") == 0) {
        Controller_instance.agent_worker(argc - 2, argv + 2);
    }else if (strcmp(argv[1], "start") == 0) {
        Controller_instance.start_worker(argc - 2, argv + 2);
    }else if (strcmp(argv[1], "init") == 0) {
        Controller_instance.initialize();
    }else
        Controller_instance.PrintUsage();
    return 0;
}