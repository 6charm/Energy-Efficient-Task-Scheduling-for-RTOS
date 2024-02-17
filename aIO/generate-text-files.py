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

if __name__ == "__main__":
    main()