import os
import subprocess
import sys

def compile_and_run_c_file(c_file_path):
    # Check if the file exists
    if not os.path.isfile(c_file_path):
        print(f"Error: The file '{c_file_path}' does not exist.")
        return

    # Ensure the file has a .c extension
    if not c_file_path.endswith(".c"):
        print(f"Error: The file '{c_file_path}' is not a valid C source file.")
        return

    # Generate the output binary file name
    output_binary = c_file_path.replace(".c", "")

    # Compile the C file
    compile_command = ["gcc", c_file_path, "-o", output_binary]
    try:
        print(f"Compiling {c_file_path}...")
        subprocess.run(compile_command, check=True)
        print(f"Compilation successful. Output binary: {output_binary}")
    except subprocess.CalledProcessError:
        print("Compilation failed. Please check the C source code for errors.")
        return

    # Run the compiled binary
    try:
        print(f"Running {output_binary}...\n")
        subprocess.run([f"./{output_binary}"], check=True)
    except subprocess.CalledProcessError:
        print("Execution failed.")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python c_tool.py <path_to_c_file>")
    else:
        compile_and_run_c_file(sys.argv[1])