#!/usr/bin/env python3

import os

abspath = os.path.abspath(__file__)
dname = os.path.dirname(abspath)
os.chdir(dname)

def is_project_header(l):
    return l.startswith('#include <Mlib') or l.startswith('#include "')

def patch_file(filename):
    # print('Processing %s' % filename)
    with open(filename, 'r', encoding='utf-8') as f:
        s = f.read()
        top0 = []
        top1 = []
        inc = []
        rest = []
        rest_reached = False
        for l in s.split('\n'):
            if l.startswith('#if'):
                rest_reached = True
            elif l.startswith('#define'):
                rest_reached = True
            elif l.startswith('#pragma clang'):
                rest_reached = True
            elif l.startswith('#pragma GCC'):
                rest_reached = True
            elif 'impl.hpp' in l:
                rest_reached = True
            elif l == '#include <Mlib/Misc/Packed_Begin.hpp>':
                rest_reached = True
            if rest_reached:
                rest.append(l)
            elif (len(inc) == 0) and (l in [
                '#pragma once',
                '#include <glad/gl.h>',
                '#define GLFW_INCLUDE_NONE',
                '#include <GLFW/glfw3.h>']):
                top1.append(l)
            elif (len(inc) == 0) and (len(top1) == 0) and (l in [
                '']):
                top0.append(l)
            elif (len(inc) == 0) and (len(top1) == 0) and l.startswith('/'):
                top0.append(l)
            elif l.startswith('#include'):
                inc.append(l)
            else:
                rest.append(l)

        inc_project = [l for l in inc if is_project_header(l)]
        inc_third_party = [l for l in inc if not is_project_header(l)]
        new = '\n'.join(top0 + top1 + sorted(inc_project) + sorted(inc_third_party) + rest)
    if new != s:
        print('%s needs to be fixed' % filename)
        #print('--------------')
        #print(s)
        #print('--------------')
        #print(new)
        #print('--------------')
        with open(filename, 'w', encoding='utf-8') as f:
            f.write(new)

for path, dirs, files in os.walk('.'):
    dirs[:] = [d for d in dirs if d not in {
        'MRelease',
        'MDebug',
        'LUDebug',
        'LURelease',
        'LURelWithDebInfo',
        'UDebug',
        'URelease',
        'URelWithDebInfo',
        'VRelease',
        'AURelWithDebInfo',
        'TURelWithDebInfo',
        'GURelease',
        'GURelWithDebInfo',
        'TGURelWithDebInfo',
        'LGURelWithDebInfo',
        'LTGURelWithDebInfo',
        'LGMRelWithDebInfo',
        'GUDebug',
        'AUDebug',
        'AURelease',
        'TestOut',
        'Data',
        '.git',
        'poly2tri',
        'cpp-httplib',
        'boost',
        'recastnavigation',
        'engine-sim',
        'openal-soft',
        'freealut',
        'bullet3',
        'cereal',
        'zlib',
        'Triangle',
        'cork',
        'siv',
        'nv_dds',
        '.cxx'}]
    files[:] = [f for f in files if f not in {
        'Array.hpp',
        'Dynamic_Base.hpp',
        'linmath.hpp',
        'glad_gl.cpp',
        'glad_vulkan.cpp',
        'Incomplete_Beta_Distribution.hpp',
        'Svd4.cpp',
        'stb_image_write_png16.hpp'}]
    for file in files:
        filename = os.path.join(path, file)
        if filename.endswith('.cpp') or filename.endswith('.hpp'):
            try:
                patch_file(filename)
            except Exception as e:
                raise ValueError(f'Error patching file "{filename}": {e}')
