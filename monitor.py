import time 
import os

def get_memory_usage():
    #open files to load
    with open('/proc/meminfo','r') as f:
        lines = f.readlines()

    #analysis contents of files, whereas saving them into dic
    mem_info = {}
    for line in lines :
        parts = line.split()
        key = parts[0].strip(':')
        value = int(parts[1]) #with kB as its unit
        mem_info[key] = value

    #calculation
    total = mem_info['MemTotal']
    available = mem_info['MemAvailable']
    used = total - available

    #convert kB to MB, precision set 2 decimal places
    return (f"--- Memory ---\n"
            f"Total: {total / 1024:.2f} MB\n"
            f"Used:  {used / 1024:.2f} MB\n"
            f"Free:  {available / 1024:.2f} MB\n"
            f"Usage: {used / total * 100:.2f} %")


def read_cpu_stat():
    with open('/proc/stat', 'r') as f:
        line = f.readline() #as the first line id all we need 
    parts = line.split()

    #parts[0] stands for 'cpu',numbers ensuing, and indemand of being transformed
    times = [int(x) for x in parts[1:]]

    #times[3:] intergrated to be total idle(lazy ones)
    idle_time = times[3]   #times consecutively add ---accumulate  
    total_time = sum(times)

    return total_time,idle_time

def calculate_cpu_usage():
    #fist time record
    t1_total, t1_idle = read_cpu_stat()
    #wait for a sec
    time.sleep(1)
    #second time record 
    t2_total, t2_idle = read_cpu_stat()

    delta_total = t2_total - t1_total
    delta_idle = t2_idle - t1_idle

    #just in case 
    if delta_total ==0:
        return 0.0

    usage = (delta_total - delta_idle) / delta_total * 100
    return usage


import os # 需要在这个文件头引入 os 库来获取核心数

def get_load_average():
    with open('/proc/loadavg', 'r') as f:
        data = f.read().split()

    # 获取前三个数据
    l1 = data[0]
    l5 = data[1]
    l15 = data[2]

    # 获取 CPU 核心数 (为了评估负载是否过高)
    # os.cpu_count() 是 Python 标准库提供的
    cpu_count = os.cpu_count()

    return (f"--- Load Average ---\n"
            f"1 min:  {l1}\n"
            f"5 min:  {l5}\n"
            f"15 min: {l15}\n"
            f"Cores:  {cpu_count}")


if __name__ == "__main__":
    try:
        # 【关键点】在循环开始前，先拍一张“照片” (初始化 T1)
        prev_total, prev_idle = read_cpu_stat()

        while True:
            # 1. 睡觉 (这是循环唯一的等待点)
            time.sleep(1)

            # 2. 醒来后，拍第二张“照片” (读取 T2)
            curr_total, curr_idle = read_cpu_stat()

            # 3. 计算 T2 - T1
            delta_total = curr_total - prev_total
            delta_idle = curr_idle - prev_idle

            cpu_usage = 0.0
            if delta_total > 0:
                cpu_usage = (delta_total - delta_idle) / delta_total * 100

            # 4. 更新“旧照片”，为下一轮做准备
            prev_total = curr_total
            prev_idle = curr_idle

            # 5. 获取内存信息
            mem_text = get_memory_usage()
            #6 Load Average
            load_average_text = get_load_average()

            # 6. 一次性清屏并打印所有内容 (减少闪烁)
            output = (
                "\033c"
                f"{mem_text}\n\n"
                f"--- CPU ---\n"
                f"CPU Usage: {cpu_usage:.2f} %\n\n"
                f"{load_average_text}\n\n"
            )
            print(output)


    except KeyboardInterrupt:
        print("\nExiting...")
