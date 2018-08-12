#! /usr/bin/python3
import os
from subprocess import run

INPUT_FILE = 'input.asm'

# cuts off last extension
OUTPUT_FILE = ''.join(INPUT_FILE.split('.')[:-1])

def disas(mnemo):
    # Writes user input to file to comC:\Users\hlz\Desktop\asmshellpile it with nasm
    f = open(INPUT_FILE, 'w')
    f.write("[bits 32]\n")
    f.write(mnemo)
    f.close()
    
    # Compilation
    # If successfull nasm will return binary in
    # file called the same but without .asm
    try:
        ret = run(['nasm', INPUT_FILE])
        os.remove(INPUT_FILE)
    except FileNotFoundError:
        print('Error: You need to have nasm installed to make asmshell work.')
        return

    if ret.returncode != 0:
        return
    
    # Compilation was successfull
    # Reading nasm output and printing to user
    f = open(OUTPUT_FILE, 'rb')
    out = []
    for byte in f.read():
        # Collect binary as hex values without 0x prefix
        # in 2 nibble format
        out.append('{:02x}'.format(byte))

    f.close()
    os.remove(OUTPUT_FILE)

    if len(out) == 0:
        return

    print('Code: ', ' '.join(out))

def main():
    while True:
        print('asmshell > ', end='')
        userin = input().lower()
        if userin == 'exit' or userin == 'quit':
            return

        disas(mnemo=userin)

if __name__ == '__main__':
    main()
