
import os, sys, subprocess

def run_tests(path):
    for item in os.listdir(os.path.join(*path)):
        abs = os.path.join(*path + [item])
        if os.path.isdir(abs):
            run_tests(list(path) + [item])
        elif abs.endswith('.py'):
            p = subprocess.Popen(['python', abs])
            p.communicate()
            if not p.returncode == 0:
                sys.stderr.write('%s: Failed (Return Code %s)\n' % (abs, p.returncode))
            else:
                sys.stdout.write('%s: Succeeded\n' % (abs))

if __name__ == '__main__':
    run_tests(['tests'])
