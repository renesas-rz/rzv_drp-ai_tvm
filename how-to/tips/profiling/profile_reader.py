
import sys
import argparse
def run(input_file,out_file):

    num_cpu_graph = 0
    num_drp_graph = 0

    time_drp = 0.0
    time_cpu = 0.0
    time_all = 0.0

    with open(input_file,"r") as f:
        line = f.readline()
        while(line):
            data = line.split()
            if(len(data)>=1):
                sub_graph_name = data[0]
            else:
                sub_graph_name = "blank"
            if(sub_graph_name.find("drp")!=-1):
                num_drp_graph += 1
                dur_time = float(data[1])
                time_drp += dur_time
            elif(sub_graph_name.find("tvmgen")!=-1):
                num_cpu_graph += 1
                dur_time = float(data[1])
                time_cpu += dur_time
            line = f.readline()
    # Calc time
    time_all = time_cpu + time_drp
    ratio_cpu = time_cpu/time_all * 100
    ratio_drp = time_drp/time_all * 100

    print("------------------------------------------------")
    print("[SUMMARY]")
    print("  CPU     sub-graph num : ",num_cpu_graph)
    print("  DRP-AI  sub-graph num : ",num_drp_graph)
    print("  CPU     inference time : {0:8.2f} [msec] ({1:4.1f}[%])"
              .format(time_cpu/1000, ratio_cpu))
    print("  DRP-AI  inference time : {0:8.2f} [msec] ({1:4.1f}[%])"\
              .format(time_drp/1000,ratio_drp))
    print("  Total inference time   : {0:8.2f} [msec]".format(time_all/1000))
    print("------------------------------------------------")

    out_txt = "------------------------------------------------\n"
    out_txt += "[SUMMARY]\n"
    out_txt +="  CPU     sub-graph num : {0}\n".format(num_cpu_graph)
    out_txt +="  DRP-AI  sub-graph num : {0}\n".format(num_drp_graph)
    out_txt +="  CPU     inference time : {0:8.2f} [msec] ({1:4.1f}[%])\n"\
              .format(time_cpu/1000, ratio_cpu)
    out_txt +="  DRP-AI  inference time : {0:8.2f} [msec] ({1:4.1f}[%])\n"\
              .format(time_drp/1000,ratio_drp)
    out_txt += "  Total inference time   : {0:8.2f} [msec]\n".format(time_all/1000)
    out_txt += "------------------------------------------------\n"
    with open(out_file,"w") as f:
        f.write(out_txt)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("txt",help="input profile text file")
    parser.parse_args()
    args = parser.parse_args()

    profile_txt   = args.txt
    out_file = profile_txt.replace(".txt","_summary.txt")
    run(profile_txt,out_file)
