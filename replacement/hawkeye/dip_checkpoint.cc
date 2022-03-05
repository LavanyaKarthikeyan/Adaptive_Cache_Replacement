//Hawkeye Cache Replacement Tool v2.0
//UT AUSTIN RESEARCH LICENSE (SOURCE CODE)
//The University of Texas at Austin has developed certain software and documentation that it desires to
//make available without charge to anyone for academic, research, experimental or personal use.
//This license is designed to guarantee freedom to use the software for these purposes. If you wish to
//distribute or make other use of the software, you may purchase a license to do so from the University of
//Texas.
///////////////////////////////////////////////
//                                            //
//     Hawkeye [Jain and Lin, ISCA' 16]       //
//     Akanksha Jain, akanksha@cs.utexas.edu  //
//                                            //
///////////////////////////////////////////////

// Source code for configs 1 and 2

#include "../inc/champsim_crc2.h"
#include <map>

#define NUM_CORE 1
#define LLC_SETS NUM_CORE*2048
#define LLC_WAYS 16

//3-bit RRIP counters or all lines
#define maxRRPV 7
uint32_t rrpv[LLC_SETS][LLC_WAYS];


//Per-set timers; we only use 64 of these
//Budget = 64 sets * 1 timer per set * 10 bits per timer = 80 bytes
#define TIMER_SIZE 1024
uint64_t perset_mytimer[LLC_SETS];

// Signatures for sampled sets; we only use 64 of these
// Budget = 64 sets * 16 ways * 12-bit signature per line = 1.5B
uint64_t signatures[LLC_SETS][LLC_WAYS];
bool prefetched[LLC_SETS][LLC_WAYS];

// Hawkeye Predictors for demand and prefetch requests
// Predictor with 2K entries and 5-bit counter per entry
// Budget = 2048*5/8 bytes = 1.2KB
#define MAX_SHCT 31
#define SHCT_SIZE_BITS 10 // Edited
#define SHCT_SIZE (1<<SHCT_SIZE_BITS)
#include "submission/hawkeye_predictor.h"
HAWKEYE_PC_PREDICTOR* demand_predictor;  //Predictor
HAWKEYE_PC_PREDICTOR* prefetch_predictor;  //Predictor

#define OPTGEN_VECTOR_SIZE 128
#include "optgen.h"
OPTgen perset_optgen[LLC_SETS]; // per-set occupancy vectors; we only use 64 of these

#include <math.h>
#define bitmask(l) (((l) == 64) ? (unsigned long long)(-1LL) : ((1LL << (l))-1LL))
#define bits(x, i, l) (((x) >> (i)) & bitmask(l))
//Sample 64 sets per core
#define SAMPLED_SET(set) (bits(set, 0 , 5) == bits(set, ((unsigned long long)log2(LLC_SETS) - 5), 5) )

// Sampler to track 8x cache history for sampled sets
// 2800 entris * 4 bytes per entry = 11.2KB
#define SAMPLED_CACHE_SIZE (2800/2)
#define SAMPLER_WAYS 8
#define SAMPLER_SETS SAMPLED_CACHE_SIZE/SAMPLER_WAYS
vector<map<uint64_t, ADDR_INFO> > addr_history; // Sampler


// ReD replacement 

#define RED_SETS_BITS 8 //Edited
#define RED_WAYS 16
#define RED_TAG_SIZE_BITS 9 // Edited
#define RED_SECTOR_SIZE_BITS 2
#define RED_PC_FRACTION 4
#define PCs_BITS 7 // Edited

#define RED_SETS (1<<RED_SETS_BITS)
#define RED_TAG_SIZE (1<<RED_TAG_SIZE_BITS)
#define RED_SECTOR_SIZE (1<<RED_SECTOR_SIZE_BITS)
#define PCs (1<<PCs_BITS)

//Dynamic
#define TYPE_RED 0
#define TYPE_HAWK 1

// ReD 

struct ReD_ART_bl {
	uint64_t tag;                       // RED_TAG_SIZE_BITS
	uint8_t valid[RED_SECTOR_SIZE];     // 1 bit
};

struct ReD_ART_set {
	struct ReD_ART_bl adds[RED_WAYS];
	uint32_t insert;                    // 4 bits, log2(RED_WAYS)
};

struct ReD_ART_set ReD_ART[NUM_CORE][RED_SETS];

struct ReD_ART_PCbl {
	uint64_t pc_entry[RED_SECTOR_SIZE];  // PCs_BITS
};

struct ReD_ART_PCbl ReD_ART_PC[NUM_CORE][RED_SETS/RED_PC_FRACTION][RED_WAYS];

struct ReD_PCRT_bl {
	uint32_t reuse_counter,            // 10-bit counter
			 noreuse_counter;          // 10-bit counter
};

struct ReD_PCRT_bl ReD_PCRT[NUM_CORE][PCs];

uint32_t misses[NUM_CORE];

// SRRIP

#define maxRRPV_red 3
uint32_t rrpv_red[LLC_SETS][LLC_WAYS];      // 2 bits

//DIP
#define SAMPLE_SETS (LLC_SETS/4)
struct DIP {
	uint64_t mem_addr[SAMPLE_SETS][LLC_WAYS];
};

DIP hawkeye_dip;
DIP red_dip;



//Dynamic
uint32_t hawkeye_hits;
uint32_t red_hits;
#define MAX_HITS 1023

// uint32_t type_dyn;
// uint32_t instr_window ;
// uint32_t miss_in_window;
// uint32_t prev_miss_in_window ;
// #define MAX_WINDOW 100000

// search for an address in ReD_ART
uint8_t lookup(uint32_t cpu, uint64_t PCnow, uint64_t block)
{
	uint64_t PCin_entry;
	uint64_t i, tag, subsector;
	uint64_t ART_set;
	
	subsector=block & (RED_SECTOR_SIZE-1);
	ART_set = (block>>RED_SECTOR_SIZE_BITS) & (RED_SETS -1);
	tag=(block>>(RED_SETS_BITS+RED_SECTOR_SIZE_BITS)) & (RED_TAG_SIZE-1);

	misses[cpu]++;
	
	for (i=0; i<RED_WAYS; i++) {
		if ((ReD_ART[cpu][ART_set].adds[i].tag == tag) && (ReD_ART[cpu][ART_set].adds[i].valid[subsector] == 1)) {
			if (ART_set % RED_PC_FRACTION == 0) {
				// if ART set stores PCs, count the reuse in PCRT
				PCin_entry = ReD_ART_PC[cpu][ART_set/RED_PC_FRACTION][i].pc_entry[subsector];
				ReD_PCRT[cpu][PCin_entry].reuse_counter++;

				if (ReD_PCRT[cpu][PCin_entry].reuse_counter > 1023) {
					// 10-bit counters, shift when saturated
					ReD_PCRT[cpu][PCin_entry].reuse_counter>>=1;
					ReD_PCRT[cpu][PCin_entry].noreuse_counter>>=1;
				}
				// Mark as invalid to count only once
				ReD_ART[cpu][ART_set].adds[i].valid[subsector] = 0; 
			}
			// found
			return 1;
		}
	}

	// not found
	return 0;
}

// remember a block in ReD_ART
void remember(uint32_t cpu, uint64_t PCnow, uint64_t block)
{
	uint32_t where;
	uint64_t i, tag, subsector, PCev_entry, PCnow_entry;
	uint64_t ART_set;
	
	subsector=block & (RED_SECTOR_SIZE-1);
	ART_set = (block>>RED_SECTOR_SIZE_BITS) & (RED_SETS -1);
	tag=(block>>(RED_SETS_BITS+RED_SECTOR_SIZE_BITS)) & (RED_TAG_SIZE-1);

	PCnow_entry = (PCnow >> 2) & (PCs-1);

	// Look first for the tag in my set
	for (i=0; i<RED_WAYS; i++) {
		if (ReD_ART[cpu][ART_set].adds[i].tag == tag)
			break;
	}
	
	if (i != RED_WAYS) {
		// Tag found, remember in the specific subsector
		ReD_ART[cpu][ART_set].adds[i].valid[subsector] = 1;

		if (ART_set % RED_PC_FRACTION == 0) {
			ReD_ART_PC[cpu][ART_set/RED_PC_FRACTION][i].pc_entry[subsector] = PCnow_entry;
		}
	}
	else {
		// Tag not found, need to replace entry in ART
		where = ReD_ART[cpu][ART_set].insert;
		
		if (ART_set % RED_PC_FRACTION == 0) {
			// if ART set stores PCs, count noreuse of evicted PCs if needed
			for (int s=0; s<RED_SECTOR_SIZE; s++) {
				if (ReD_ART[cpu][ART_set].adds[where].valid[s]) {
					PCev_entry = ReD_ART_PC[cpu][ART_set/RED_PC_FRACTION][where].pc_entry[s];
					ReD_PCRT[cpu][PCev_entry].noreuse_counter++;

					// 10-bit counters, shift when saturated
					if (ReD_PCRT[cpu][PCev_entry].noreuse_counter > 1023) {
						ReD_PCRT[cpu][PCev_entry].reuse_counter>>=1;
						ReD_PCRT[cpu][PCev_entry].noreuse_counter>>=1;
					}
				}
			}
		}
		
		// replace entry to store new block address
		
		ReD_ART[cpu][ART_set].adds[where].tag = tag;
		for (int j=0; j<RED_SECTOR_SIZE; j++) {
			ReD_ART[cpu][ART_set].adds[where].valid[j] = 0;
		}
		ReD_ART[cpu][ART_set].adds[where].valid[subsector] = 1;
		
		if (ART_set % RED_PC_FRACTION == 0) {
			ReD_ART_PC[cpu][ART_set/RED_PC_FRACTION][where].pc_entry[subsector] = PCnow_entry;
		}
		
		// update pointer to next entry to replace
		ReD_ART[cpu][ART_set].insert++;
		if (ReD_ART[cpu][ART_set].insert == RED_WAYS) 
			ReD_ART[cpu][ART_set].insert = 0;
	}
}



// initialize replacement state
void InitReplacementState()
{
    for (int i=0; i<LLC_SETS; i++) {
        for (int j=0; j<LLC_WAYS; j++) {
            rrpv[i][j] = maxRRPV;
            signatures[i][j] = 0;
            prefetched[i][j] = false;
        }
        perset_mytimer[i] = 0;
        perset_optgen[i].init(LLC_WAYS-2);
    }

    addr_history.resize(SAMPLER_SETS);
    for (int i=0; i<SAMPLER_SETS; i++) 
        addr_history[i].clear();

    demand_predictor = new HAWKEYE_PC_PREDICTOR();
    prefetch_predictor = new HAWKEYE_PC_PREDICTOR();

    cout << "Initialize Hawkeye state" << endl;

    // ReD init
	for (int core=0; core<NUM_CORE; core++) {
		for (int i=0; i<RED_SETS; i++) {
			ReD_ART[core][i].insert = 0;
			for (int j=0; j<RED_WAYS; j++) {
				ReD_ART[core][i].adds[j].tag=0;
				for (int k=0; k<RED_SECTOR_SIZE; k++) {
					ReD_ART[core][i].adds[j].valid[k] = 0;
				}
			}
		}
	}

	for (int core=0; core<NUM_CORE; core++) {
		for (int i=0; i<RED_SETS/RED_PC_FRACTION; i++) {
			for (int j=0; j<RED_WAYS; j++) {
				for (int k=0; k<RED_SECTOR_SIZE; k++) {
					ReD_ART_PC[core][i][j].pc_entry[k] = 0;
				}
			}
		}
	}

	for (int core=0; core<NUM_CORE; core++) {
		for (int i=0; i<PCs; i++) {
			ReD_PCRT[core][i].reuse_counter = 3;
			ReD_PCRT[core][i].noreuse_counter = 0;
		}
	}

	for (int i=0; i<NUM_CORE; i++) {
	   misses[i]=0; 
	}
	
	// SRRIP init
	for (int i=0; i<LLC_SETS; i++) {
		for (int j=0; j<LLC_WAYS; j++) {
			rrpv_red[i][j] = maxRRPV_red;
		}
	}

    cout << "Initialize ReD state" << endl;

    //Dynamic
    hawkeye_hits = 0 ;
    red_hits = 0;

}

// find replacement victim
// return value should be 0 ~ 15 or 16 (bypass)
uint32_t GetVictimInSet (uint32_t cpu, uint32_t set, const BLOCK *current_set, uint64_t PC, uint64_t paddr, uint32_t type){
    
    uint8_t present;
    uint64_t PCentry;
    uint64_t block;

    int red_result = -1;
    int hawk_result = -1;
   

    block = paddr >> 6; // assuming 64B line size, get rid of lower bits
	PCentry = (PC >> 2) & (PCs-1);
    	
	if (type == LOAD || type == RFO || type == PREFETCH) {
		present = lookup(cpu, PC, block);
		if (!present) {
			// Remember in ART only if reuse in PCRT is intermediate, or one out of eight times
			if (   (    ReD_PCRT[cpu][PCentry].reuse_counter * 64 > ReD_PCRT[cpu][PCentry].noreuse_counter
					 && ReD_PCRT[cpu][PCentry].reuse_counter * 3 < ReD_PCRT[cpu][PCentry].noreuse_counter)
				|| (misses[cpu] % 8 == 0)) 
			{
				remember(cpu, PC, block);
			}
			// bypass when address not in ART and reuse in PCRT is low or intermediate
			if (ReD_PCRT[cpu][PCentry].reuse_counter * 3 < ReD_PCRT[cpu][PCentry].noreuse_counter) {
				/* BYPASS */
				red_result = LLC_WAYS;    
                goto hawkeye_label;                     
			}
		} 
	}
	
	// NO BYPASS when address is present in ART or when reuse is high in PCRT
	// or for write-backs
	
	// SRRIP, look for the maxRRPV line
	while (1) {
		for (int i=0; i<LLC_WAYS; i++)
			if (rrpv_red[set][i] == maxRRPV_red){
				red_result =  i;
                goto hawkeye_label ;
            }

		for (int i=0; i<LLC_WAYS; i++)
			rrpv_red[set][i]++;
	}

	red_result =  0;
        
    hawkeye_label:
    // look for the maxRRPV line
    uint32_t max_rrip = 0;
    int32_t lru_victim = -1;
    for (uint32_t i=0; i<LLC_WAYS; i++)
        if (rrpv[set][i] == maxRRPV){
            hawk_result= i;
            goto end_label ;
        }

    //If we cannot find a cache-averse line, we evict the oldest cache-friendly line
    
    for (uint32_t i=0; i<LLC_WAYS; i++)
    {
        if (rrpv[set][i] >= max_rrip)
        {
            max_rrip = rrpv[set][i];
            lru_victim = i;
        }
    }

    assert (lru_victim != -1);
    //The predictor is trained negatively on LRU evictions
    if( SAMPLED_SET(set) )
    {
        if(prefetched[set][lru_victim])
            prefetch_predictor->decrement(signatures[set][lru_victim]);
        else
            demand_predictor->decrement(signatures[set][lru_victim]);
    }
    hawk_result= lru_victim;

    end_label:
    int j;
    if(set%128 == 0){
        for ( j = 0;j<LLC_WAYS;j++){
            if (hawkeye_dip.mem_addr[set/128][j] == block){
                hawkeye_hits++ ;
                if (hawkeye_hits == MAX_HITS){
                    hawkeye_hits = hawkeye_hits >> 1;
                    red_hits = red_hits >> 1;
                }
                break;
            }
        }
        hawkeye_dip.mem_addr[set/128][hawk_result] = block ;
    }
    else if(set%128 == 64){
        for ( j = 0;j<LLC_WAYS;j++){
            if (red_dip.mem_addr[set/128][j] == block){
                red_hits++ ;
                if (red_hits == MAX_HITS){
                    hawkeye_hits = hawkeye_hits >> 1;
                    red_hits = red_hits >> 1;
                }
                break;
            }
        }        
        red_dip.mem_addr[set/128][red_result] = block ;
    }

    if (hawkeye_hits >= red_hits){
        return hawk_result ;
    }
    else {
        return red_result ;
    }

        // WE SHOULD NOT REACH HERE
        assert(0);
        return 0;
        
    
}

void replace_addr_history_element(unsigned int sampler_set)
{
    uint64_t lru_addr = 0;
    
    for(map<uint64_t, ADDR_INFO>::iterator it=addr_history[sampler_set].begin(); it != addr_history[sampler_set].end(); it++)
    {
   //     uint64_t timer = (it->second).last_quanta;

        if((it->second).lru == (SAMPLER_WAYS-1))
        {
            //lru_time =  (it->second).last_quanta;
            lru_addr = it->first;
            break;
        }
    }

    addr_history[sampler_set].erase(lru_addr);
}

void update_addr_history_lru(unsigned int sampler_set, unsigned int curr_lru)
{
    for(map<uint64_t, ADDR_INFO>::iterator it=addr_history[sampler_set].begin(); it != addr_history[sampler_set].end(); it++)
    {
        if((it->second).lru < curr_lru)
        {
            (it->second).lru++;
            assert((it->second).lru < SAMPLER_WAYS); 
        }
    }
}


// called on every cache hit and cache fill
void UpdateReplacementState (uint32_t cpu, uint32_t set, uint32_t way, uint64_t paddr, uint64_t PC, uint64_t victim_addr, uint32_t type, uint8_t hit)
{
    
    
    paddr = (paddr >> 6) << 6;
       

    
    if(type == PREFETCH)
    {
        if (!hit && (way < LLC_WAYS))
            prefetched[set][way] = true;
    }
    else if ((way < LLC_WAYS))
        prefetched[set][way] = false;

    //Ignore writebacks
    if (type == WRITEBACK)
        return;


    //If we are sampling, OPTgen will only see accesses from sampled sets
    if(SAMPLED_SET(set))
    {
        //The current timestep 
        uint64_t curr_quanta = perset_mytimer[set] % OPTGEN_VECTOR_SIZE;

        uint32_t sampler_set = (paddr >> 6) % SAMPLER_SETS; 
        uint64_t sampler_tag = CRC(paddr >> 12) % 256;
        assert(sampler_set < SAMPLER_SETS);

        // This line has been used before. Since the right end of a usage interval is always 
        //a demand, ignore prefetches
        if((addr_history[sampler_set].find(sampler_tag) != addr_history[sampler_set].end()) && (type != PREFETCH))
        {
            unsigned int curr_timer = perset_mytimer[set];
            if(curr_timer < addr_history[sampler_set][sampler_tag].last_quanta)
               curr_timer = curr_timer + TIMER_SIZE;
            bool wrap =  ((curr_timer - addr_history[sampler_set][sampler_tag].last_quanta) > OPTGEN_VECTOR_SIZE);
            uint64_t last_quanta = addr_history[sampler_set][sampler_tag].last_quanta % OPTGEN_VECTOR_SIZE;
            //and for prefetch hits, we train the last prefetch trigger PC
            if( !wrap && perset_optgen[set].should_cache(curr_quanta, last_quanta))
            {
                if(addr_history[sampler_set][sampler_tag].prefetched)
                    prefetch_predictor->increment(addr_history[sampler_set][sampler_tag].PC);
                else
                    demand_predictor->increment(addr_history[sampler_set][sampler_tag].PC);
            }
            else
            {
                //Train the predictor negatively because OPT would not have cached this line
                if(addr_history[sampler_set][sampler_tag].prefetched)
                    prefetch_predictor->decrement(addr_history[sampler_set][sampler_tag].PC);
                else
                    demand_predictor->decrement(addr_history[sampler_set][sampler_tag].PC);
            }
            //Some maintenance operations for OPTgen
            perset_optgen[set].add_access(curr_quanta);
            update_addr_history_lru(sampler_set, addr_history[sampler_set][sampler_tag].lru);

            //Since this was a demand access, mark the prefetched bit as false
            addr_history[sampler_set][sampler_tag].prefetched = false;
        }
        // This is the first time we are seeing this line (could be demand or prefetch)
        else if(addr_history[sampler_set].find(sampler_tag) == addr_history[sampler_set].end())
        {
            // Find a victim from the sampled cache if we are sampling
            if(addr_history[sampler_set].size() == SAMPLER_WAYS) 
                replace_addr_history_element(sampler_set);

            assert(addr_history[sampler_set].size() < SAMPLER_WAYS);
            //Initialize a new entry in the sampler
            addr_history[sampler_set][sampler_tag].init(curr_quanta);
            //If it's a prefetch, mark the prefetched bit;
            if(type == PREFETCH)
            {
                addr_history[sampler_set][sampler_tag].mark_prefetch();
                perset_optgen[set].add_prefetch(curr_quanta);
            }
            else
                perset_optgen[set].add_access(curr_quanta);
            update_addr_history_lru(sampler_set, SAMPLER_WAYS-1);
        }
        else //This line is a prefetch
        {
            assert(addr_history[sampler_set].find(sampler_tag) != addr_history[sampler_set].end());
            //if(hit && prefetched[set][way])
            uint64_t last_quanta = addr_history[sampler_set][sampler_tag].last_quanta % OPTGEN_VECTOR_SIZE;
            if (perset_mytimer[set] - addr_history[sampler_set][sampler_tag].last_quanta < 5*NUM_CORE) 
            {
                if(perset_optgen[set].should_cache(curr_quanta, last_quanta))
                {
                    if(addr_history[sampler_set][sampler_tag].prefetched)
                        prefetch_predictor->increment(addr_history[sampler_set][sampler_tag].PC);
                    else
                       demand_predictor->increment(addr_history[sampler_set][sampler_tag].PC);
                }
            }

            //Mark the prefetched bit
            addr_history[sampler_set][sampler_tag].mark_prefetch(); 
            //Some maintenance operations for OPTgen
            perset_optgen[set].add_prefetch(curr_quanta);
            update_addr_history_lru(sampler_set, addr_history[sampler_set][sampler_tag].lru);
        }

        // Get Hawkeye's prediction for this line
        bool new_prediction = demand_predictor->get_prediction (PC);
        if (type == PREFETCH)
            new_prediction = prefetch_predictor->get_prediction (PC);
        // Update the sampler with the timestamp, PC and our prediction
        // For prefetches, the PC will represent the trigger PC
        addr_history[sampler_set][sampler_tag].update(perset_mytimer[set], PC, new_prediction);
        addr_history[sampler_set][sampler_tag].lru = 0;
        //Increment the set timer
        perset_mytimer[set] = (perset_mytimer[set]+1) % TIMER_SIZE;
    }

    bool new_prediction = demand_predictor->get_prediction (PC);
    if (type == PREFETCH)
        new_prediction = prefetch_predictor->get_prediction (PC);

    if (way < LLC_WAYS)
        signatures[set][way] = PC;

    //Set RRIP values and age cache-friendly line
    if (way < LLC_WAYS){
        if(!new_prediction)
            rrpv[set][way] = maxRRPV;
        else
        {
            rrpv[set][way] = 0;
            if(!hit)
            {
                bool saturated = false;
                for(uint32_t i=0; i<LLC_WAYS; i++)
                    if (rrpv[set][i] == maxRRPV-1)
                        saturated = true;

                //Age all the cache-friendly  lines
                for(uint32_t i=0; i<LLC_WAYS; i++)
                {
                    if (!saturated && rrpv[set][i] < maxRRPV-1)
                        rrpv[set][i]++;
                }
            }
            rrpv[set][way] = 0;
        }
    }

        

    if (way== 16){ return;  }
    // SRRIP
    if (hit)
        rrpv_red[set][way] = 0;
    else
        rrpv_red[set][way] = maxRRPV_red-1;    
       

}


// use this function to print out your own stats on every heartbeat 
void PrintStats_Heartbeat()
{

}

// use this function to print out your own stats at the end of simulation
void PrintStats()
{
    unsigned int hits = 0;
    unsigned int accesses = 0;
    for(unsigned int i=0; i<LLC_SETS; i++)
    {
        accesses += perset_optgen[i].access;
        hits += perset_optgen[i].get_num_opt_hits();
    }

    std::cout << "OPTgen accesses: " << accesses << std::endl;
    std::cout << "OPTgen hits: " << hits << std::endl;
    std::cout << "OPTgen hit rate: " << 100*(double)hits/(double)accesses << std::endl;

    cout << endl << endl;
    return;
}
