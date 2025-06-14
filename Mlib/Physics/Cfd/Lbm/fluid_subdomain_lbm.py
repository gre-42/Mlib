#!/usr/bin/env python3

# from: https://en.wikipedia.org/wiki/lattice_boltzmann_methods

# this is a fluid simulator using the lattice boltzmann method.
# using d2q9 and peiodic boundary, and used no external library.
# it generates two ripples at 50,50 and 50,40.
# reference: erlend magnus viggen's master thesis, "the lattice boltzmann method with applications in acoustics".
# for wikipedia under cc-by-sa license.
import math
from typing import List
# define some utilities


def sum(a):
    s = 0
    for e in a:
        s = s+e
    return s


# weights in d2q9
weights = [1/36, 1/9, 1/36,
           1/9, 4/9, 1/9,
           1/36, 1/9, 1/36]
# discrete velocity vectors
discrete_velocity_vectors = [[-1, 1], [0, 1], [1, 1],
                             [-1, 0], [0, 0], [1, 0],
                             [-1, -1], [0, -1], [1, -1]
                             ]
# a field2d class


class field2d():
    def __init__(self, res: List[int]):
        self.field = []
        for b in range(res[0]):
            fm = []
            for a in range(res[1]):
                fm.append(weights[:])
            self.field.append(fm[:])
        self.res = res
    # this visualize the simulation, can only be used in a terminal

    def visualize(self, sc, ranges=None):
        if ranges is None:
            ranges = [range(res[0]), range(res[1])]
        stringr = ""
        for n in ranges[0]:
            row = ""
            for x in ranges[1]:
                flowmomentum = self.momentum(n, x)
                col = "\033[38;2;{0};{1};{2}m██".format(
                    int(127+sc*flowmomentum[0]), int(127+sc*flowmomentum[1]), 0)
                row = row+col
            # print(row)
            stringr = stringr+row+"\n"
        print(stringr)
        return stringr
    # momentum of the field

    def momentum(self, y, x):
        return velocity_field[y][x][0]*sum(self.field[y][x]), velocity_field[y][x][1]*sum(self.field[y][x])


# resolution of the simulation
res = [35, 70]
a = field2d(res)
# the velocity field


# def lerp(a, b, x):
#     return a + (b - a) * x


# medium_velocity = [0.1, -0.2]
# relaxation = 0.1
# damping = 1
frequency = 1 / 8
omega = 2 * math.pi * frequency
# center_density = 0.5
center_velocity = [0.2, 0.1]
# center_density = 0.3


# def modify_velocity(v):
#     return [
#         lerp(v[0] * damping, medium_velocity[0], relaxation),
#         lerp(v[1] * damping, medium_velocity[1], relaxation)]
# 

velocity_field = []
for dummy_variable in range(res[0]):
    dummy_list = []
    for dummy_variable2 in range(res[1]):
        dummy_list.append([0, 0])
    velocity_field.append(dummy_list[:])
# the density field
density_field = []
for dummy_variable in range(res[0]):
    dummy_list = []
    for dummy_variable2 in range(res[1]):
        dummy_list.append(1)
    density_field.append(dummy_list[:])
# set initial condition
#density_field[res[0]//2][res[1]//2] = 2
#density_field[res[0]//2-5][res[1]//2] = 2
# maximum solving steps
max_steps = 1200
# the speed of sound, specifically 1/sqrt(3) ~ 0.57
speed_of_sound = 1/math.sqrt(3)
# time relaxation constant
time_relaxation_constant = 0.55
# solve
for s in range(max_steps):
    for y in range(res[0] - 20):
        velocity_field[y + 10][res[1] //
                               2][0] = math.sin(s * omega) * center_velocity[0]
        velocity_field[y + 10][res[1] //
                               2][1] = math.sin(s * omega) * center_velocity[1]
        # density_field[res[0]//2][res[1]//2] = 1 + \
        #    math.sin(s * omega) * center_density
    # collision step
    df = field2d(res)
    for y in range(res[0]):
        for x in range(res[1]):
            for v in range(9):
                velocity = a.field[y][x][v]
                first_term = velocity
                # the flow velocity
                flow_velocity = velocity_field[y][x]
                dotted = flow_velocity[0]*discrete_velocity_vectors[v][0] + \
                    flow_velocity[1]*discrete_velocity_vectors[v][1]
                # #the taylor expainsion of equilibrium term
                taylor = 1+((dotted)/(speed_of_sound**2))+((dotted**2)/(2*speed_of_sound**4)) - \
                    ((flow_velocity[0]**2+flow_velocity[1]**2) /
                     (2*speed_of_sound**2))
                # the current density
                density = density_field[y][x]
                # the equilibrium
                equilibrium = density*taylor*weights[v]
                if y == 0 or y == res[0] - 1 or \
                   x == 0 or x == res[1] - 1:
                   df.field[y][x][v] = equilibrium
                else:
                    second_term = (equilibrium-velocity)/time_relaxation_constant
                    df.field[y][x][v] = first_term+second_term
    # streaming step
    for y in range(1, res[0] - 1):
        for x in range(1, res[1] - 1):
            for v in range(9):
                # target, the lattice point this iteration is solving
                source_y = y-discrete_velocity_vectors[v][1]
                source_x = x-discrete_velocity_vectors[v][0]
                # peiodic boundary
                a.field[y][x][v] = df.field[source_y][source_x][v]
    # calculate macroscopic variables
    for y in range(res[0]):
        for x in range(res[1]):
            # recompute density field
            density_field[y][x] = sum(a.field[y][x])
            # recompute flow velocity
            flow_velocity = [0, 0]
            for dummy_variable in range(9):
                flow_velocity[0] = flow_velocity[0] + \
                    discrete_velocity_vectors[dummy_variable][0] * \
                    a.field[y][x][dummy_variable]
            for dummy_variable in range(9):
                flow_velocity[1] = flow_velocity[1] + \
                    discrete_velocity_vectors[dummy_variable][1] * \
                    a.field[y][x][dummy_variable]
            flow_velocity[0] = flow_velocity[0]/density_field[y][x]
            flow_velocity[1] = flow_velocity[1]/density_field[y][x]
            # insert to velocity field
            velocity_field[y][x] = flow_velocity
    # for y in range(res[0]):
    #     velocity_field[y][0] = modify_velocity(velocity_field[y][0])
    #     velocity_field[y][res[1] -
    #                       1] = modify_velocity(velocity_field[y][res[1]-1])
    # for x in range(res[1]):
    #     velocity_field[0][x] = modify_velocity(velocity_field[0][x])
    #     velocity_field[res[0] -
    #                    1][x] = modify_velocity(velocity_field[res[0]-1][x])
    # visualize
    a.visualize(128)
