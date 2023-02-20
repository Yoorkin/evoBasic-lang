from asyncio import subprocess
import os
import sys

ebc_path = ""
evm_path = ""
core_folder = ""
build_path = ""

def compile(target,build_path):
    return False


def run(target_path):
    subprocess.run([evm_path,target_path,"-p",core_folder])
