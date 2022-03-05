import os
directory = "/home/lkarthik/CSE240C/ChampSim_old/ChampSim_CRC2/results_dip_config4/"
# traces = "/datasets/cs240c-wi22-a00-public/data/Assignment2-gz/"
traces = "/datasets/cs240c-wi22-a00-public/data/Assignment2-multicore/"
log_file = "/home/lkarthik/CSE240C/ChampSim_old/ChampSim_CRC2/results/results_dip.log"



for root, dirs, files in os.walk(traces):
    for filename in files:
        if ("cassandra" in filename): 
            cmd = 'bin/dip_branch --warmup_instructions 10000000 --simulation_instructions 50000000 -traces ' + traces + '/' + filename +' > '+ directory + '/'+filename+'.log &'
            os.system(cmd)
    break

# file_out = open(log_file,"a")
# file_out.write("TESTCASE,IPC,MPKI\n")
# for root, dirs, files in os.walk(directory):
#     for filename in files:
#         entry = ""
#         entry = entry+filename
#         resultsfl = open(directory+"/"+filename,"r")
#         for line in resultsfl:
#             # print("Test \n") 
#             if ("CPU 0 cummulative IPC" in line):
#                 val = line.split(':')[1]
#                 val = val.split(' ')[1]
#                 entry = entry+","+val
#             elif ("LLC LOAD" in line):
#                 val = line.split(':')[-1]
#                 val = val.split('\n')[0]
#                 val = int(val)*1000/50000000
#                 val = str(val)
#                 entry = entry+","+val+","
#         resultsfl.close()
#         file_out.write(entry+"\n")            
# file_out.close()

# file_out = open(log_file,"a")
# file_out.write("TESTCASE,IPC,MPKI ALL, MPKI LOAD, MPKI PREFETCH\n")
# for root, dirs, files in os.walk(directory):
#     for filename in files:
#         entry = ""
#         entry = entry+filename
#         resultsfl = open(directory+"/"+filename,"r")
#         for line in resultsfl:
#             # print("Test \n") 
#             if ("CPU 0 cummulative IPC" in line):
#                 val = line.split(':')[1]
#                 val = val.split(' ')[1]
#                 entry = entry+","+val
#             elif ("LLC TOTAL" in line):
#                 val = line.split(':')[-1]
#                 val = val.split('\n')[0]
#                 val = int(val)*1000/100000000
#                 val = str(val)
#                 entry = entry+","+val
#             elif ("LLC LOAD" in line):
#                 val = line.split(':')[-1]
#                 val = val.split('\n')[0]
#                 val = int(val)*1000/100000000
#                 val = str(val)
#                 entry = entry+","+val
#             elif ("LLC PREFETCH" in line):
#                 val = line.split(':')[-1]
#                 val = val.split('\n')[0]
#                 val = int(val)*1000/100000000
#                 val = str(val)
#                 entry = entry+","+val+","
#         resultsfl.close()
#         file_out.write(entry+"\n")            
# file_out.close()


   
