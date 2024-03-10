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
                                      f"{test_name}.sl -o {test_name}.exe",
                                      f"{test_name}.exe",
                                      f"Building arrays.sl...\n",
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
                                      f"{test_name}.sl StdString.sl StdMath.sl -o {test_name}.exe -lm",
                                      f"{test_name}.exe",
                                      f"Building StdStringTests.sl...\n"
                                      f"Building StdString.sl...\n"
                                      f"Building StdMath.sl...\n"
                                      f"StdString.sl(362,35): warning: Implicit conversion from 'character' to 'integer'.\n"
                                      f"StdString.sl(362,12): warning: Implicit conversion from 'integer' to 'character'.\n"
                                      f"StdString.sl(375,8): warning: Implicit conversion from 'real' to 'integer'.\n"
                                      f"StdString.sl(376,37): warning: Implicit conversion from 'integer' to 'real'.\n"
                                      f"StdString.sl(384,41): warning: Implicit conversion from 'integer' to 'real'.\n"
                                      f"StdString.sl(395,41): warning: Implicit conversion from 'integer' to 'real'.\n"
                                      f"StdString.sl(399,46): warning: Implicit conversion from 'integer' to 'real'.\n"
                                      f"StdString.sl(399,51): warning: Implicit conversion from 'character' to 'real'.\n"
                                      f"StdString.sl(399,20): warning: Implicit conversion from 'real' to 'character'.\n"
                                      f"StdString.sl(403,45): warning: Implicit conversion from 'integer' to 'real'.\n",
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
                                      f"{test_name}.sl classes2.sl -o {test_name}.exe",
                                      f"{test_name}.exe",
                                      f"Building classes.sl...\n"
                                      f"Building classes2.sl...\n",
                                      expected_program_output)
        test.compile_test()
        self.assertEqual(test.check_outs(), True)
        test.run_compiled()
        self.assertEqual(test.check_program_outs(), True)
        test.clean_up()


class VirtualTests(unittest.TestCase):
    def runTest(self):
        expected_program_output = ""
        test_name = "virtual"
        filenames = ["virtual"]
        with open(f"{test_name}.txt", "r") as f:
            expected_program_output = f.read()
        test = SuccessCompilationTest(SLANGC_PATH, filenames,
                                      f"{test_name}.sl -o {test_name}.exe",
                                      f"{test_name}.exe",
                                      f"Building virtual.sl...\n"
                                      f"virtual.sl(29,4): warning: Implicit conversion from 'sample.B' to 'sample.A'.\n"
                                      f"virtual.sl(30,4): warning: Implicit conversion from 'sample.B' to 'sample.A'.\n",
                                      expected_program_output)
        test.compile_test()
        self.assertEqual(test.check_outs(), True)
        test.run_compiled()
        self.assertEqual(test.check_program_outs(), True)
        test.clean_up()


if __name__ == '__main__':
    unittest.main()
