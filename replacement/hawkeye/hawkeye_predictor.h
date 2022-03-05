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

#ifndef PREDICTOR_H
#define PREDICTOR_H

using namespace std;

#include <iostream>

#include <math.h>
#include <set>
#include <vector>
#include <map>

uint64_t CRC( uint64_t _blockAddress )
{
    static const unsigned long long crcPolynomial = 3988292384ULL;
    unsigned long long _returnVal = _blockAddress;
    for( unsigned int i = 0; i < 32; i++ )
        _returnVal = ( ( _returnVal & 1 ) == 1 ) ? ( ( _returnVal >> 1 ) ^ crcPolynomial ) : ( _returnVal >> 1 );
    return _returnVal;
}


class HAWKEYE_PC_PREDICTOR
{
    map<uint64_t, pair<short unsigned int, short unsigned int> > SHCT;

       public:

    void increment (uint64_t pc)
    {
        uint64_t signature = CRC(pc) % SHCT_SIZE;
        if(SHCT.find(signature) == SHCT.end()){
            SHCT[signature].first = 0;
            SHCT[signature].second = 0;
        }

        SHCT[signature].second = (SHCT[signature].first < MAX_SHCT) ? SHCT[signature].second : SHCT[signature].second >> 1;
        SHCT[signature].first = (SHCT[signature].first < MAX_SHCT) ? (SHCT[signature].first+1) : (SHCT[signature].first >> 1)+1;
        
         

    }

    void decrement (uint64_t pc)
    {
        uint64_t signature = CRC(pc) % SHCT_SIZE;
        if(SHCT.find(signature) == SHCT.end()){
            SHCT[signature].first = 0;
            SHCT[signature].second = 0;
        }
        // if(SHCT[signature] != 0)
        //     SHCT[signature] = SHCT[signature]-1;
        SHCT[signature].first = (SHCT[signature].second < MAX_SHCT) ? SHCT[signature].first  : SHCT[signature].first >> 1;
        SHCT[signature].second = (SHCT[signature].second < MAX_SHCT) ? (SHCT[signature].second+1) : (SHCT[signature].second >> 1)+1;
    }

    bool get_prediction (uint64_t pc)
    {
        uint64_t signature = CRC(pc) % SHCT_SIZE;
        if(SHCT.find(signature) != SHCT.end() && SHCT[signature].first < SHCT[signature].second)
            return false;
        return true;
    }

    short unsigned int get_reuse (uint64_t pc)
    {
        uint64_t signature = CRC(pc) % SHCT_SIZE;
        if(SHCT.find(signature) != SHCT.end())
            return SHCT[signature].first;
        return 0;
    }

    short unsigned int get_noreuse (uint64_t pc)
    {
        uint64_t signature = CRC(pc) % SHCT_SIZE;
        if(SHCT.find(signature) != SHCT.end())
            return SHCT[signature].second;
        return 0;
    }

    short unsigned int get_confidence (uint64_t pc)
    {
        uint64_t signature = CRC(pc) % SHCT_SIZE;
        if(SHCT.find(signature) != SHCT.end())
            return ((SHCT[signature].second - SHCT[signature].first) < 0) ? (-1*(SHCT[signature].second - SHCT[signature].first)) : ((SHCT[signature].second - SHCT[signature].first));
        return -1;
    }
};

#endif
