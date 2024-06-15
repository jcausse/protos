import os
import argparse

COMPILER: str = 'gcc'
COMPILER_VERSION_CMD: str= 'gcc -v'
CFLAGS:str = '-std=c11 -pedantic -pedantic-errors -Wall -Werror -Wextra -D_POSIX_C_SOURCE=200112L'

VALGRIND:str = 'valgrind'
VALGRIND_VERSION_CMD:str = 'valgrind --version'

def parse_arguments():
    parser = argparse.ArgumentParser(description='Library test automation tool')
    parser.add_argument('-l', '--libfile', type=str, required=False, default='libs.txt', help='File indicating all libraries to be tested')
    parser.add_argument('-v', '--verbose', required=False, action='store_true', help='Enable verbose output')
    return parser.parse_args()

def check_compiler():
    return os.system(f'{COMPILER_VERSION_CMD} > /dev/null 2>&1') == 0

def check_valgrind():
    return os.system(f'{VALGRIND_VERSION_CMD} > /dev/null 2>&1') == 0

class Library:
    def __init__(self, name: str, verbose: bool):
        self.__name: str = name
        self.__verbose: bool =  verbose
        self.__source: str = f'{name}.c'
        self.__header: str = f'{name}.h'
        self.__tester: str = f'{name}_test.c'
        self.__files: list[str] = [self.__source, self.__header, self.__tester]
    
    def check(self, file_list: list[str]) -> bool:
        return all(file in file_list for file in self.__files)
    
    def compile(self) -> bool:
        cmd: str = f"{COMPILER} {CFLAGS} {self.__source} {self.__tester} -o {self.__name}.bin {'' if self.__verbose else ' > /dev/null 2>&1'}"
        if self.__verbose:
            print(cmd)
        return os.system(cmd) == 0
    
    def test(self) -> bool:
        cmd: str = f"{VALGRIND} -s ./{self.__name}.bin {'' if self.__verbose else ' > /dev/null 2>&1'}"
        if self.__verbose:
            print(cmd)
        return os.system(cmd) == 0

def test_all():
    if check_compiler() is False:
        print(f'Program unavailable: {COMPILER}')
        exit(1)
    if check_valgrind() is False:
        print(f'Program unavailable: {VALGRIND}')
        exit(1)
    
    args = parse_arguments()
    
    with open(args.libfile, 'r') as libs_file:
        libraries: list[str] = libs_file.readlines()
    for i in range(len(libraries)):
        libraries[i] = libraries[i].strip()
    
    file_list: list[str] = [f for f in os.listdir('.') if os.path.isfile(f)]
    
    for lib_name in libraries:
        if lib_name.startswith('#'):
            continue
        lib = Library(lib_name, args.verbose)
        if lib.check(file_list) is False:
            print(f'Missing files for library: {lib_name}')
            continue
        if lib.compile() is False:
            print(f'Compilation error for library: {lib_name}')
            continue
        if lib.test() is False:
            print(f'Valgrind failed for library: {lib_name}')
            continue
        if args.verbose:
            print()
        print(f'Tests passed: {lib_name}')
        if args.verbose:
            print()

    os.system('rm ./*.bin')

if __name__ == '__main__':
    test_all()
