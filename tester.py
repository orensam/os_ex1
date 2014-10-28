#!/usr/bin/python

import os
import re
import subprocess
import test_README
import time
import shutil
import glob

REASONABLE_RUNNING_TIME_SEC=10
EXPECTED_RUNNING_TIME_SEC=4


def getCC(f):
    CC = None
    for line in f:
        pass

def parseMeasureData(measureData):
    field_names = ['hostname', 'iterations', 'inst_time',
                   'func_time', 'trap_time', 'func_inst_ratio',
                   'trap_inst_ratio']
    res = {}
    measureData = measureData.split('\n')
    for i in xrange(len(field_names)):
        if measureData[i]:
            res[field_names[i]] = measureData[i].split(':')[-1].strip()

    return res


def compareData(iterations, school, student):
    errors = []
    # check hostname
    if (school['hostname'] != student['hostname']):
        errors.append('machine_name')
    
    if (int(student['iterations']) < iterations):
        errors.append('num_of_iters_smaller_than_input')
    
    times = ['func', 'inst', 'trap']
    diff = {}
    for t in times:
        if float(student[t+'_time']) < 0.01:
            errors.append(t+'_compare_with_school_sol_very_bad')
            continue

        diff[t] = float(school[t+'_time']) / float(student[t+'_time'])

        if (diff[t] > 2.5 or diff[t] < (1.0/2.5)):
            errors.append(t+'_compare_with_school_sol_very_bad')
        elif (diff[t] > 1.5 or diff[t] < (1.0/1.5)):
            errors.append(t+'_compare_with_school_sol_bad')

    return errors


def extract(filename, ex_obj):
    error_codes = []
    try:
        shutil.rmtree('tmp')
    except:
        pass

    subprocess.call('mkdir tmp', shell=True)

    # extract the archive to a different folder to avoid file overwriting
    e=subprocess.call('tar -xf '+filename+' -C tmp', shell=True)

    # add read permissions
    subprocess.call('chmod +r tmp/*', shell=True)
        
    # check tar executed with no errors
    if(e != 0):
        error_codes.append('bad_tar')
        return error_codes

    #the students should not submit the osm.h file
    if os.path.exists('tmp/osm.h'):
        error_codes.append('osm_h_submitted')
        subprocess.call('rm -rf tmp/osm.h', shell=True)

    return error_codes
    

def writeErrors(exobj, filename, usernames, error_codes):
    if usernames:
        exobj.write(usernames + ':\n')
    else:
        exobj.write(filename + ':\n')

    for error in error_codes:
        exobj.write(' ' + error + '\n')

    exobj.write('\n')



def testMeasurements(iterations):
    error_codes = set()

    # get the school's measurement
    p=subprocess.Popen('../sol/measure ' + str(iterations), shell=True, stdout=subprocess.PIPE)
    school_output = p.communicate()[0]

    # get the students' measurement
    p=subprocess.Popen('measure ' + str(iterations), shell=True, stdout=subprocess.PIPE)
    students_output = p.communicate()[0]

    # parse data
    school_data = parseMeasureData(school_output)
    students_data = parseMeasureData(students_output)

    # compare students' measurements with the school's
    error_codes.update(compareData(iterations, school_data, students_data))

    return error_codes

def test(filename, ex_obj, do_extract=True):
    usernames = None
    error_codes = set()

    if do_extract:
        error_codes = set(extract(filename, ex_obj))
        if 'bad_tar' in error_codes:
            writeErrors(ex_obj, filename, usernames, error_codes)
            return

    # switch to the tmp directory
    os.chdir('tmp')

    # test README file
    if not os.path.exists('./README'):
        error_codes.add('missing_README_file')

    (usernames, readme_errors) = test_README.test_README('.')
    error_codes.update(readme_errors)

    files = glob.glob('*.c*')
 
    # copy needed files from the solution directory
    shutil.copy('../sol/osm.h', '.')
    shutil.copy('../sol/measure.c', '.')
    shutil.copy('../sol/makefile.withapp', '.')
       
    # test their compilation
    print('Running make...\n')

    if (not os.path.exists('Makefile') and not os.path.exists('makefile')):
        error_codes.add('missing_makefile')
    else:
        subprocess.call('make clean', shell=True)
        p=subprocess.Popen('make', shell=True, stderr=subprocess.PIPE)
        res = p.communicate()

        # for some odd reason  'ar: creating libosm.a' is written to stderr
        if (res[1] != '' and res[1] != 'ar: creating libosm.a\n'):
            if not os.path.exists('libosm.a'):
                error_codes.add('compilation')
                writeErrors(ex_obj, filename, usernames, error_codes)
                return
            else:
                error_codes.add('compilation_warning')

    # clean everything
    subprocess.call('make clean', shell=True)
    subprocess.call('make clean -f makefile.withapp', shell=True)

    # school makefile that creates the 'measure' exec
    make_f = 'makefile.withapp'

    CC = 'gcc'
    # should we compile with gcc or g++?
    for fn in files:
        if fn.endswith('cc') or fn.endswith('cpp'):
            CC = 'g++'

    # compile
    FULL_MAKE_COMMAND = 'env CC=' + CC + ' make -e -f ' + make_f
    #FULL_MAKE_COMMAND = 'make -e -f ' + make_f
    print(FULL_MAKE_COMMAND)
    err=subprocess.call(FULL_MAKE_COMMAND, shell=True)

    # check for compilation errors or warnings 
    if( err != 0):
        if not os.path.exists('measure'):
            error_codes.add('compilation')
            writeErrors(ex_obj, filename, usernames, error_codes)
            return
        else:
            pass
            #error_codes.add('compilation_warning')


    error_codes.update(testMeasurements(1000000))
    error_codes.update(testMeasurements(1000007))
    
    writeErrors(ex_obj, filename, usernames, error_codes)

    # switch back to parent directory and remove tmp
    os.chdir('..')
    #shutil.rmtree('./tmp')

    return error_codes


def main():
    import time
    import sys
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument('filename',
            help='tar filename or submission directory',
            type=str,
            nargs='?')
    parser.add_argument('-t','--test',
            help='Just test the code in the ./tmp directory',
            action='store_true')
    args = parser.parse_args()

    if not args.filename and not args.test:
        parser.print_help()
        return

    curdir = os.getcwd()

    if args.test:
        print('Testing ./tmp...')
        errors = test('<TMP>', sys.stdout, False)
        os.chdir(curdir)
        return

    # open the grades file    
    subprocess.call('touch ex1', shell=True)
    ex_obj = open('ex1','a')


    if os.path.isdir(args.filename):
        submission_dir = args.filename
        
        for filename in glob.glob(os.path.join(submission_dir, '*')):
            print('Testing ' + filename + '#'*50)
            errors = test(filename, ex_obj)
            print('Errors: ' + str(errors))
            os.chdir(curdir)
            time.sleep(1)
    else:
        errors = test(args.filename, ex_obj)
        print('Errors: ' + str(errors))
        os.chdir(curdir)

    ex_obj.close()


if __name__ == '__main__':
    main()
