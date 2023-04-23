import os
import subprocess


class FailCompilationTest:
    def __init__(self, slangc_path: str, filenames: str, args: str, output_executable: str, expected_compiler_output: str, expected_compiler_error: str):
        self.slangc_path = slangc_path
        self.filenames = filenames
        self.args = args
        self.output_executable = output_executable
        self.expected_compiler_output = expected_compiler_output
        self.expected_compiler_error = expected_compiler_error
        self.outs = ""
        self.errs = ""

    def compile_test(self):
        cmd = [self.slangc_path] + self.args.split(" ")
        print("Building: " + str(cmd))
        r = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        self.outs = r.stdout.decode("utf-8", errors="ignore")
        self.errs = r.stderr.decode("utf-8", errors="ignore")

    def check_outs(self):
        return self.expected_compiler_output in self.outs

    def check_errs(self):
        return self.expected_compiler_error in self.errs

    def clean_up(self):
        os.remove(self.output_executable)
        os.remove(self.test_name + ".ll")
        os.remove(self.test_name + ".sl_optimized.ll")


class SuccessCompilationTest:
    def __init__(self, slangc_path: str, filenames: str, args: str, output_executable: str, expected_compiler_output: str, expected_program_output: str):
        self.slangc_path = slangc_path
        self.filenames = filenames
        self.args = args
        self.output_executable = output_executable
        self.expected_compiler_output = expected_compiler_output
        self.expected_program_output = expected_program_output
        self.outs = ""
        self.errs = ""
        self.program_outs = ""

    def compile_test(self):
        cmd = [self.slangc_path] + self.args.split(" ")
        print("Building: " + str(cmd))
        r = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        self.outs = r.stdout.decode("utf-8", errors="ignore")
        self.errs = r.stderr.decode("utf-8", errors="ignore")
        #print(self.outs)
        #print(self.errs)

    def run_compiled(self):
        cmd = os.getcwd() + "/" + self.output_executable
        print("Running: " + str(cmd))
        r = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        self.program_outs = r.stdout.decode("utf-8", errors="ignore")
        self.program_outs = self.program_outs.replace("\r\n", "\n")

    def check_outs(self):
        return self.expected_compiler_output in self.outs

    def check_errs(self):
        return self.errs == ""

    def check_program_outs(self):
        return self.expected_program_output in self.program_outs

    def clean_up(self):
        os.remove(self.output_executable)
        for filename in self.filenames:
            os.remove(filename + ".ll")

