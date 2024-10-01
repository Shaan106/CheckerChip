// cc_creditSystem.cc

#include "checker_chip/cc_creditSystem.hh"

#include <stdio.h>

//Constructor
CheckerCreditSystem::CheckerCreditSystem(
    unsigned long* clk,
    int max_credits,
    unsigned long default_latency_add,
    unsigned long default_latency_remove
) : 
    clk(clk),
    current_credits(max_credits),
    max_credits(max_credits),
    default_latency_add(default_latency_add),
    default_latency_remove(default_latency_remove) {}


int CheckerCreditSystem::getCredits() {
    return current_credits;
}

bool CheckerCreditSystem::zeroCreditCheck() {
    return zero_credit_flag;
}

void CheckerCreditSystem::addCredit(bool instant, unsigned long additional_latency, int num_credits) {
    // values defaulted in .hh
    // saying at time "*clk + latency" add "num_credits" credits to the buffer
    if (instant) {
        if (current_credits + num_credits >= max_credits) {
            current_credits = max_credits;
        } else {
            current_credits = current_credits + max_credits;
        }
        //printf("ADD : *clk + latency %lu =  ========================== num_credits = %d \n", 0, num_credits);
    } else {
        unsigned long latency = default_latency_add + additional_latency;
        credit_transit_map[*clk + latency] = credit_transit_map[*clk + latency] + num_credits;  
        //printf("ADD : *clk + latency %lu =  ========================== num_credits = %d \n", *clk+latency, num_credits);
    }


    
}


void CheckerCreditSystem::decrementCredit(bool instant, unsigned long additional_latency, int num_credits) {

    if (instant) {
        if (current_credits - num_credits <= 0) {
            current_credits = 0;
        } else {
            current_credits = current_credits - num_credits;
        }
        //printf("REM : *clk + latency %lu =  ========================== num_credits = %d \n", 0, num_credits);
    } else {
        unsigned long latency = default_latency_add + additional_latency;
        credit_transit_map[*clk + latency] = credit_transit_map[*clk + latency] - num_credits;  
        //printf("REM : *clk + latency %lu =  ========================== num_credits = %d, test: %d \n", *clk+latency, num_credits, credit_transit_map[*clk + latency]);
    }
    

    
}


void CheckerCreditSystem::updateCredits() {
    //called every cc_clock cycle.
    // update current_credits with credits in transit that have arrived

    //printf("curr *clk = %lu, credit_transit_map[*clk] = %d \n", *clk, credit_transit_map[*clk]);

    int temp_new_credits = current_credits + credit_transit_map[*clk]; 

    if (temp_new_credits >= max_credits) {
        // if already max_credits then set current credits to max_credits
        current_credits = max_credits;
    } else if (temp_new_credits <= 0) {
        // cannot go lower than 0 credits
        current_credits = 0;
    } else {
        current_credits = temp_new_credits;
    }

    // setting zero credit flag
    if (current_credits <= 0) {
        zero_credit_flag = true;
    } else {
        zero_credit_flag = false;
    }

    // erasing the transited credit
    credit_transit_map.erase(*clk);
}

