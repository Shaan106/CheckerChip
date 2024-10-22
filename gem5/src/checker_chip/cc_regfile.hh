
#ifndef cc_regfile_HH
#define cc_regfile_HH

#include <vector>

class CheckerRegfile {

private:
    unsigned long* clk; //pointer to the cc_buffer_clk

    unsigned int latency; //latency of checker chip regfile verification + update

    unsigned int bandwidth; // how many insts every cycle regfile can handle

    int latencyPointer; //current insts, initial = 0

    unsigned int instsStaged; //how many insts staged to send to regfile

    std::vector<unsigned int> instructionsInRegfileArray; // array to keep track of num insts being processed, initial size = latency
public:

    //Constructor
    CheckerRegfile(unsigned long* clk,
                        unsigned int latency,
                        unsigned int bandwidth);


    // returns how many insts freed, and adds insts into regfile, increments latencyPointer
    unsigned int updateRegfile();

    //stage one inst for regfile, sent during updateRegfile
    void stageInstToRegfile();

    bool isBandwidthFull();

    unsigned int getBandwidth();
    unsigned int getLatency();
};


#endif // cc_regfile_HH