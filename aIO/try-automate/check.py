"""
# FILE: generate-text-files.py
# ----------------------------
# This file generates text files of size 
# 1MB, 5MB, 20MB from /usr/share/dict/words
# to be used as test-inputs for aIO.
"""
import os # os.system   
import sys # sys.path
import re
import subprocess
import resource
import time

def generate_file(filepath, sz):
    dictfile = '/usr/share/dict/words'

    # Read words from word file
    with open(dictfile, 'r') as f_in, open(filepath, 'w') as f_out:
        bytes_written = 0
        for word in f_in:
            if bytes_written >= sz:
                break
            f_out.write(word.strip() + '\n')
            bytes_written += 1
            
def run(test_number, data_file, command, desc, max_run_time):
    global nkilled, nerror, ratios, basetimes

    # Generate STDIO test string for a given access pattern.
    base = command
    
    # "./cat files/text1meg.txt > files/out.txt"
    # --> "./stdio-cat files/text1meg.txt > files/out.txt"
    base = re.sub(r'(\./)([a-z]*)', r'\1stdio-\2', base)
    
    # "files/out.txt" --> "files/stdio-out.txt"
    base = re.sub(r'out\.txt', r'stdio-out.txt', base)

    # Print Test Title
    print(f"TEST:       {test_number}. {desc}\nCOMMAND      {command}\nSTDIO:       ", end='')
    print(base)

    base_time = run_time_median(base, max_run_time, "files/stdio-out.txt") # stdio time
    print(base_time)
    print("%.5fs (%.5fs user, %.5fs system, %dKiB memory)" % (base_time["time"], base_time["utime"], base_time["stime"], base_time["maxrss"]))

    output_size = None
    if "files/stdio-out" in base:
        output_size = os.path.getsize("files/stdio-out.txt") * 2

    # RUN with aIO SELF
    self_time = run_time_median(command, max_run_time, "files/out.txt", output_size)
    print(self_time)

    if "error" in self_time:
        print(f"KILLED ({self_time['error']})")
        nkilled += 1
    else:
        # format stat output
        # Ratio with stdio
        print("%.5fs (%.5fs user, %.5fs system, %dKiB memory)" % (base_time["time"], base_time["utime"], base_time["stime"], base_time["maxrss"]))
        print(f"RATIO:     {base_time['time'] / self_time['time']:.2f}x stdio", end='')
        
        if user_time['medianof'] != 1 and user_time['medianof'] == base_time['medianof']:
            print(f" (median of {user_time['medianof']} trials)")
        elif user_time['medianof'] != base_time['medianof']:
            print(f" ({base_time['medianof']} stdio trial{'s' if base_time['medianof'] != 1 else ''}, {user_time['medianof']} your code)", end="\n")
        
        ratios.append(base_time['time'] / user_time['time'])
        basetimes.append(base_time['time'])

        if 'files/stdio-out.txt' in base and subprocess.run(["cmp", "files/baseout.txt", "files/out.txt"]).returncode != 0:
            print("           ERROR! files/out.txt differs from stdio version")
            nerror += 1
    
    # Print stderr if present
    if 'stderr' in user_time and user_time['stderr'] != "":
        print(f"{user_time['stderr']}", end="\n")
    print()

def run_time_median(command, max_run_time, output, max_size=None):
    times = []
    stderr = ""
    total_time = 0

    while(len(times) < 5 and total_time < 3):
        # Get run-time statistics dict
        trial_time = run_time(command, max_run_time, output, max_size, len(times) + 1)
        print(trial_time)
        stderr += trial_time.get("stderr", "")

        if "error" in trial_time:
            # Return the first trial time if there were previous successful trials
            return trial_time if not times else times[0]
        else:
            times.append(trial_time)
            total_time += trial_time["time"]
        
        times.sort(key=lambda x: x["time"])
        
        median_index = len(times) // 2
        median_time = times[median_index]
        median_time["medianof"] = len(times)
        median_time["stderr"] = stderr
        
        return median_time

def run_time(command, max_run_time, output, max_size, trial=None):
    # Create pipes for communication with the child process
    print(f"{command}\n{max_run_time}\n{output}\n, {trial}")
    pr, pw = os.pipe()
    or_, ow = os.pipe()
    print("here")
    # Fork a child process
    pid = os.fork()
    if pid == 0:  # Child process
        # Set the child process group ID to its own PID
        print('here1c')
        os.setpgrp()
    
        # Redirect stdin to pw, stdout, and stderr to ow
        # write execvp output to fd 100
        os.close(pr)
        os.dup2(pw, 100)
        os.close(pw)
        os.close(or_)
        os.dup2(ow, 1)
        os.dup2(ow, 2)
        os.close(ow)
        
        # Execute the command
        os.execvp(command[0], command)
    else:  # Parent process
        before = time.time()  # Record the start time
        died = False  # Flag to track if the process terminated due to an error
        
        # Monitor the child process
        while True:
            try:
                # Check if the child process has terminated
                if os.waitpid(-1, os.WNOHANG)[0] > 0:
                    break
            except ChildProcessError:
                break
                
        # Calculate the elapsed time
        delta = time.time() - before
        
        # Close the write end of the pipe to the child process
        os.close(pw)
        
        # Read the output from the child process
        nb = os.read(pr, 2000)
        os.close(pr)
        
        # Parse the output and store it in a dictionary
        answer = {}
        while nb and (m := re.findall(r'"(.*?)"\s*:\s*([\d.]+)', buf.decode())):
            for key, value in m:
                answer[key] = float(value)
        
        # Set default values for time-related metrics
        if "time" not in answer:
            answer["time"] = delta
        if "utime" not in answer:
            answer["utime"] = delta
        if "stime" not in answer:
            answer["stime"] = delta
        if "maxrss" not in answer:
            answer["maxrss"] = -1
        
        # Close the read end of the pipe from the child process
        os.close(ow)
        
        # Read the stderr output from the child process
        buf = None
        nb = os.read(or_, 2000)
        os.close(or_)
        
        # Store the stderr output in the answer dictionary
        answer["stderr"] = ""
        if buf and nb:
            tx = ""
            for l in buf.decode().split('\n'):
                if l:
                    tx += ("        : " if tx else "") + l + "\n"
            if tx:
                answer["stderr"] = f"    MY STDERR (TRIAL {trial}): {tx}"
        
        # Return the parsed output and metrics
        return answer

def main():
    # Create the text data files.
    if not os.path.exists("files"):
        try:
            os.makedirs("files")
        except OSError as e:
            print(f"*** Cannot run tests because 'files' cannot be created: {e}", file=sys.stderr)
            print(f"*** Remove 'files' and try again.", file=sys.stderr)
            sys.exit(1)
    
    generate_file("files/text1meg.txt", 1 << 20)
    generate_file("files/text5meg.txt", 5 << 20)
    generate_file("files/text20meg.txt", 20 << 20)

    # Test cases start here
    run(1, "files/text1meg.txt",
    "./cat files/text1meg.txt > files/out.txt",
    "sequential regular small file 1B", 10) 

    run(2, "files/text1meg.txt",
        "cat files/text1meg.txt | ./cat | cat > files/out.txt",
        "sequential piped small file 1B", 10) 

    run(3, "files/text5meg.txt",
        "./cat files/text5meg.txt > files/out.txt",
        "sequential regular medium file 1B", 10) 

    run(4, "files/text5meg.txt",
        "cat files/text5meg.txt | ./cat | cat > files/out.txt",
        "sequential piped medium file 1B", 10) 

    run(5, "files/text20meg.txt",
        "./cat files/text20meg.txt > files/out.txt",
        "sequential regular large file 1B", 20) 

    run(6, "files/text20meg.txt",
        "cat files/text20meg.txt | ./cat | cat > files/out.txt",
        "sequential piped large file 1B", 20) 

    run(7, "files/text5meg.txt",
        "./blockcat -b 1024 files/text5meg.txt > files/out.txt",
        "sequential regular medium file 1KB", 10) 

    run(8, "files/text5meg.txt",
        "cat files/text5meg.txt | ./blockcat -b 1024 | cat > files/out.txt",
        "sequential piped medium file 1KB", 10) 

    run(9, "files/text20meg.txt",
        "./blockcat -b 1024 files/text20meg.txt > files/out.txt",
        "sequential regular large file 1KB", 20) 

    run(10, "files/text20meg.txt",
        "cat files/text20meg.txt | ./blockcat -b 1024 | cat > files/out.txt",
        "sequential piped large file 1KB", 20) 

    run(11, "files/text20meg.txt",
        "./blockcat files/text20meg.txt > files/out.txt",
        "sequential regular large file 4KB", 20) 

    run(12, "files/text20meg.txt",
        "cat files/text20meg.txt | ./blockcat | cat > files/out.txt",
        "sequential piped large file 4KB", 20)


if __name__ == "__main__":
    main()