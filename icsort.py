#!/usr/bin/env python3

import os.path

for path, dirs, files in os.walk('.'):
    dirs[:] = [d for d in dirs if d not in [
        'MRelease',
        'MDebug',
        'LUDebug',
        'LURelease',
        'UDebug',
        'URelease',
        'VRelease'
        'TestOut',
        'Data',
        '.git',
        'poly2tri',
        'cpp-httplib',
        'boost']]
    files[:] = [f for f in files if f not in [
        'Array.hpp',
        'linmath.hpp',
        'glad_gl.cpp',
        'glad_vulkan.cpp',
        'Svd4.cpp',
        'Incomplete_Beta_Distribution.hpp']]
    for file in files:
        filename = os.path.join(path, file)
        if filename.endswith('.cpp') or filename.endswith('.hpp'):
            # print('Processing %s' % filename)
            with open(filename, 'r') as f:
                s = f.read()
                top = []
                inc = []
                rest = []
                rest_reached = False
                for l in s.split('\n'):
                    if l.startswith('#if'):
                        rest_reached = True
                    elif l.startswith('#define'):
                        rest_reached = True
                    elif 'impl.hpp' in l:
                        rest_reached = True
                    elif l.startswith('/'):
                        rest_reached = True
                    elif l == '#include <Mlib/Packed_Begin.hpp>':
                        rest_reached = True
                    if rest_reached:
                        rest.append(l)
                    elif l in [
                        '#pragma once',
                        '#include <glad/gl.h>',
                        '#define GLFW_INCLUDE_NONE',
                        '#include <GLFW/glfw3.h>']:
                        top.append(l)
                    elif l.startswith('#include'):
                        inc.append(l)
                    else:
                        rest.append(l)

                inc_project = [l for l in inc if l.startswith('include <Mlib')]
                inc_third_party = [l for l in inc if not l.startswith('include <Mlib')]
                new = '\n'.join(top + sorted(inc_project) + sorted(inc_third_party) + rest)
            if new != s:
                print('%s needs to be fixed' % filename)
                #print('--------------')
                #print(s)
                #print('--------------')
                #print(new)
                #print('--------------')
                with open(filename, 'w') as f:
                    f.write(new)
