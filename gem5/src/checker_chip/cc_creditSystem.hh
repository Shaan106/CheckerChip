
#ifndef cc_creditSystem_HH
#define cc_creditSystem_HH

#include <unordered_map>
#include <array>

class CheckerCreditSystem {

private:
    unsigned long* clk; //pointer to the cc_buffer_clk
    
    int current_credits; // component's current credits
    int max_credits; //component's max credits
    bool zero_credit_flag = false; // turns on when 0 credits -> indicates a stall
    
    unsigned long default_latency_add; //default latency in adding a credit
    unsigned long default_latency_remove; //default latency in removing a credit
    
    //hashmap to track credit updates in transit
    // std::unordered_map<unsigned long,int> credit_transit_map = {};

    std::array<int, 32> credit_transit_array = {}; // assuming it never takes more than 32 cycles to transmit a piece of data

    const unsigned long mapping_bit_mask = 31; // bit mask used to find where to put cycle transit data into

public:

    //Constructor
    CheckerCreditSystem(unsigned long* clk,
                        int max_credits,
                        unsigned long default_latency_add,
                        unsigned long default_latency_remove);

    // get how many credits left
    int getCredits();

    // are there no more credits left, do we need to stall?
    // true if zero credits, else false
    bool zeroCreditCheck();

    // add a credit (or multiple) to object 
    // uses default arguments that can be overwritten
    void addCredit(bool instant = false, unsigned long additional_latency = 0, int num_credits = 1);

    // remove a credit (or multiple) from an object 
    void decrementCredit(bool instant = true, unsigned long additional_latency = 0, int num_credits = 1);

    // update the credit state
    void updateCredits();

    // // update the clock of the credits
    // void updateClock(int cc_clock);

};


#endif // cc_creditSystem_HH