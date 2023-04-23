import unittest
from builder import *

with open("slangc_path.txt", "r") as f:
    SLANGC_PATH = f.read()


class ArraysTest(unittest.TestCase):
    def runTest(self):
        expected_program_output = ""
        test_name = "arrays"
        filenames = ["arrays"]
        with open(f"{test_name}.txt", "r") as f:
            expected_program_output = f.read()
        test = SuccessCompilationTest(SLANGC_PATH, filenames,
                                      f"{test_name}.sl -o {test_name}.exe -lgc",
                                      f"{test_name}.exe",
                                      f"[INFO] Compiling module \"{test_name}\"...\n[INFO] Linking...\n",
                                      expected_program_output)
        test.compile_test()
        self.assertEqual(test.check_outs(), True)
        test.run_compiled()
        self.assertEqual(test.check_program_outs(), True)
        test.clean_up()


class StringTest(unittest.TestCase):
    def runTest(self):
        expected_program_output = ""
        test_name = "StdStringTests"
        filenames = ["StdStringTests", "StdString", "StdMath"]
        with open(f"{test_name}.txt", "r") as f:
            expected_program_output = f.read()
        test = SuccessCompilationTest(SLANGC_PATH, filenames,
                                      f"{test_name}.sl StdString.sl StdMath.sl -o {test_name}.exe -lgc",
                                      f"{test_name}.exe",
                                      f"[INFO] Compiling module \"StdMath\"...\n"
                                      f"[INFO] Compiling module \"StdString\"...\n"
                                      f"[INFO] Compiling module \"StdStringTests\"...\n"
                                      f"[INFO] Linking...\n",
                                      expected_program_output)
        test.compile_test()
        self.assertEqual(test.check_outs(), True)
        test.run_compiled()
        self.assertEqual(test.check_program_outs(), True)
        test.clean_up()


class ClassesTest(unittest.TestCase):
    def runTest(self):
        expected_program_output = ""
        test_name = "classes"
        filenames = ["classes", "classes2"]
        with open(f"{test_name}.txt", "r") as f:
            expected_program_output = f.read()
        test = SuccessCompilationTest(SLANGC_PATH, filenames,
                                      f"{test_name}.sl classes2.sl -o {test_name}.exe -lgc",
                                      f"{test_name}.exe",
                                      f"[INFO] Compiling module \"classes2\"...\n"
                                      f"[INFO] Compiling module \"classes\"...\n"
                                      f"[INFO] Linking...\n",
                                      expected_program_output)
        test.compile_test()
        self.assertEqual(test.check_outs(), True)
        test.run_compiled()
        self.assertEqual(test.check_program_outs(), True)
        test.clean_up()


if __name__ == '__main__':
    unittest.main()
