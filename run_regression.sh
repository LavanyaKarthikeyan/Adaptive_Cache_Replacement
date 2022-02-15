#!/bin/bash
# echo "Starting Regression Run ..." ;
# start=`date +%s`;
# for trace in /datasets/cs240c-wi22-a00-public/data/Assignment2-gz/*; 
# do 
   
# #     # bin/hawkeye-config1 --warmup_instructions 50000000 --simulation_instructions 200000000 -traces $trace > results_baseline_hawkeye_config1/${trace##*/}.log & 
# #     # bin/hawkeye-config2 --warmup_instructions 50000000 --simulation_instructions 200000000 -traces $trace > results_baseline_hawkeye_config2/${trace##*/}.log & 
# #     #bin/red-config1 --warmup_instructions 50000000 --simulation_instructions 200000000 -traces $trace > results_baseline_red_config1/${trace##*/}.log & 
# #     # bin/red-config2 --warmup_instructions 50000000 --simulation_instructions 200000000 -traces $trace > results_baseline_red_config2/${trace##*/}.log &
# #    # bin/lru-config1 --warmup_instructions 10000000 --simulation_instructions 100000000 -traces $trace > results_baseline_lru_small_config1/${trace##*/}.log & 
# #    # bin/lru-config2 --warmup_instructions 10000000 --simulation_instructions 100000000 -traces $trace > results_baseline_lru_small_config2/${trace##*/}.log & 
# #     # bin/red_ratio_2-config2 --warmup_instructions 10000000 --simulation_instructions 100000000 -traces $trace > results_baseline_red_ratio_2_config2/${trace##*/}.log & 
# #     # bin/red-config2 --warmup_instructions 10000000 --simulation_instructions 100000000 -traces $trace > results_baseline_red_ratio_3_config2/${trace##*/}.log &
# #     # bin/red_ratio_4-config2 --warmup_instructions 10000000 --simulation_instructions 100000000 -traces $trace > results_baseline_red_ratio_4_config2/${trace##*/}.log &
# #     # bin/red_ratio_5-config2 --warmup_instructions 10000000 --simulation_instructions 100000000 -traces $trace > results_baseline_red_ratio_5_config2/${trace##*/}.log &
# #     # bin/red_ratio_1-config2 --warmup_instructions 10000000 --simulation_instructions 100000000 -traces $trace > results_baseline_red_ratio_1_config2/${trace##*/}.log &
# #     #bin/hawkeye-assoc-4-config2 --warmup_instructions 10000000 --simulation_instructions 100000000 -traces $trace > results_baseline_hawkeye_assoc_4_config2/${trace##*/}.log &
# #     #bin/hawkeye-assoc-8-config2 --warmup_instructions 10000000 --simulation_instructions 100000000 -traces $trace > results_baseline_hawkeye_assoc_8_config2/${trace##*/}.log &
# #     #bin/hawkeye-assoc-16-config2 --warmup_instructions 10000000 --simulation_instructions 100000000 -traces $trace > results_baseline_hawkeye_assoc_16_config2/${trace##*/}.log &
# #     # bin/hawkeye-assoc-32-config2 --warmup_instructions 10000000 --simulation_instructions 100000000 -traces $trace > results_baseline_hawkeye_assoc_32_config2/${trace##*/}.log &
# #     #bin/red_64_config2 --warmup_instructions 10000000 --simulation_instructions 100000000 -traces $trace > results_baseline_red_64_config2/${trace##*/}.log &
# #     #bin/red_128_config2 --warmup_instructions 10000000 --simulation_instructions 100000000 -traces $trace > results_baseline_red_128_config2/${trace##*/}.log &
# #     # bin/red_256_config2 --warmup_instructions 10000000 --simulation_instructions 100000000 -traces $trace > results_baseline_red_256_config2/${trace##*/}.log &
# #     # bin/red_16_config2 --warmup_instructions 10000000 --simulation_instructions 100000000 -traces $trace > results_baseline_red_16_config2/${trace##*/}.log &
# #     bin/hawkeye-256-config2 --warmup_instructions 10000000 --simulation_instructions 100000000 -traces $trace > results_baseline_hawkeye_256_config2/${trace##*/}.log &
# #     bin/hawkeye-128-config2 --warmup_instructions 10000000 --simulation_instructions 100000000 -traces $trace > results_baseline_hawkeye_128_config2/${trace##*/}.log &
#     bin/hawkeye-64-config2 --warmup_instructions 10000000 --simulation_instructions 100000000 -traces $trace > results_baseline_hawkeye_64_config2/${trace##*/}.log &
#     # bin/hawkeye-16-config2 --warmup_instructions 10000000 --simulation_instructions 100000000 -traces $trace > results_baseline_hawkeye_16_config2/${trace##*/}.log &

# done
log_csv=./results/$1_metrics.csv ;
`rm -rf $log_csv` ;
`echo "TRACE,IPC,LLC_ACCESS,LLC_MISS,LLC_LOAD_ACCESS,LLC_LOAD_MISS,LLC_PREFETCH_ACCESS,LLC_PREFETCH_MISS,LLC_WRITEBACK_ACESS,LLC_WRITEBACK_MISS" >> $log_csv`;
# `echo "\n" >> $log_csv`;
for results in $1/* ;
do

    IPC=`grep "CPU 0 cummulative IPC" $results | cut '-d:' '-f2' | xargs | cut '-d ' '-f1'` ;

    LLC_ACCESS=`grep "LLC TOTAL" $results | cut '-d:' '-f2' | xargs | cut '-d ' '-f1'` ;

    LLC_MISS=`grep "LLC TOTAL" $results | cut '-d:' '-f4' | xargs` ;

    LLC_LOAD_ACCESS=`grep "LLC LOAD" $results | cut '-d:' '-f2' | xargs | cut '-d ' '-f1'` ;

    LLC_LOAD_MISS=`grep "LLC LOAD" $results | cut '-d:' '-f4' | xargs` ;

    LLC_PREFETCH_ACCESS=`grep "LLC PREFETCH" $results | cut '-d:' '-f2' | xargs | cut '-d ' '-f1'` ;

    LLC_PREFETCH_MISS=`grep "LLC PREFETCH" $results | cut '-d:' '-f4' | xargs` ;

    LLC_WRITEBACK_ACCESS=`grep "LLC WRITEBACK" $results | cut '-d:' '-f2' | xargs | cut '-d ' '-f1'` ;

    LLC_WRITEBACK_MISS=`grep "LLC WRITEBACK" $results | cut '-d:' '-f4' | xargs` ;

    results=${results##*/}
    echo "${results%%.*},$IPC,$LLC_ACCESS,$LLC_MISS,$LLC_LOAD_ACCESS,$LLC_LOAD_MISS,$LLC_PREFETCH_ACCESS,$LLC_PREFETCH_MISS,$LLC_WRITEBACK_ACCESS,$LLC_WRITEBACK_MISS" >> $log_csv;

done










# # wait

# # for trace in ipc1_public/server*; 
# # do 
# #     ./run_champsim.sh hashed_perceptron-FNL-MMA280520-next_line-spp_dev-no-lru-1core 50 50 ${trace##*/} &   
    
# # done
# # wait

# # for trace in ipc1_public/client*; 
# # do 
# #     ./run_champsim.sh hashed_perceptron-FNL-MMA280520-next_line-spp_dev-no-lru-1core 50 50 ${trace##*/} &   
    
# # done
# # wait



# # end=`date +%s`;
# # runtime=$((end-start)) ;
# # echo "Simulation Run Over. Time: $runtime" ;
 
# # log_csv=./results_50M/FNL-016kb-main_log_file.csv;
# # `rm -rf $log_csv` ;
# # `echo "TRACE,IPC,INSTRUCTIONS,CYCLES,L1I_MISS_LATENCY,L1I_ACCESS,L1I_MISS,L2C_ACCESS,L1I_LOAD_MISS,L1I_LOAD_ACCESS,L1I_PREFETCH_REQ,L1I_PREFETCH_USEFUL,L1I_PREFETCH_USELESS" >> $log_csv`;
# # # `echo "\n" >> $log_csv`;
# # for results in results_50M/*.xz-hashed_perceptron-FNL-MMA280520-next_line-spp_dev-no-lru-1core_016kb.txt ;
# # do 
    
# #     PERF=`grep -r $results -e "CPU 0 cumulative IPC"`;
    
# #     if [[ $PERF =~ IPC\:([^\i]+) ]] ;  then 
# #         IPC="${BASH_REMATCH[1]}"      
# #     fi
# #     if [[ $PERF =~ instructions\:([^\c]+) ]] ;  then 
# #         INSTRUCTIONS="${BASH_REMATCH[1]}"      
# #     fi
# #     if [[ $PERF =~ cycles\:([^\n]+) ]] ;  then 
# #         CYCLES="${BASH_REMATCH[1]}"      
# #     fi
 
# #     L1I_latency=`grep -r $results -e "L1I AVERAGE MISS LATENCY"`;
# #     if [[ $L1I_latency =~ LATENCY\:([^\c]+) ]] ;  then 
# #         L1I_latency="${BASH_REMATCH[1]}"      
# #     fi

# #     L1I_total=`grep -r $results -e "L1I TOTAL"` ;
# #     if [[ $L1I_total =~ ACCESS\:([^\H]+) ]] ;  then 
# #         L1I_ACCESS="${BASH_REMATCH[1]}"      
# #     fi
# #     if [[ $L1I_total =~ MISS\:([^\n]+) ]] ;  then 
# #         L1I_MISS="${BASH_REMATCH[1]}"      
# #     fi

# #     L1I_load=`grep -r $results -e "L1I LOAD"` ;
# #     if [[ $L1I_load =~ ACCESS\:([^\H]+) ]] ;  then 
# #         L1I_LOAD_ACCESS="${BASH_REMATCH[1]}"      
# #     fi
# #     if [[ $L1I_load =~ MISS\:([^\n]+) ]] ;  then 
# #         L1I_LOAD_MISS="${BASH_REMATCH[1]}"      
# #     fi

    
   
# #     L1I_PREFETCH_REQ=`grep -r $results -e "L1I PREFETCH  REQUESTED" | cut '-d:' '-f2' | xargs | cut '-d ' '-f1'` ;
# #     L1I_PREFETCH_USEFUL=`grep -r $results -e "L1I PREFETCH  REQUESTED" | cut '-d:' '-f4' | xargs | cut '-d ' '-f1'` ;
# #     L1I_PREFETCH_USELESS=`grep -r $results -e "L1I PREFETCH  REQUESTED" | cut '-d:' '-f5' | xargs ` ;
# #     L2C_ACCESS=`grep -r $results -e "L2C TOTAL" | cut '-d:' '-f2' | xargs | cut '-d ' '-f1'` ;
   

# #     results=${results##*/}
# #     echo "${results%%.*},$IPC,$INSTRUCTIONS,$CYCLES,$L1I_latency,$L1I_ACCESS,$L1I_MISS,$L2C_ACCESS,$L1I_LOAD_MISS,$L1I_LOAD_ACCESS,$L1I_PREFETCH_REQ,$L1I_PREFETCH_USEFUL,$L1I_PREFETCH_USELESS" >> $log_csv;


# # done
