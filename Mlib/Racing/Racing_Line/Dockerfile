FROM ubuntu:latest

RUN apt-get update && apt-get -y install \
    git \
    python3 \
    python3-pip \
    python3-venv
RUN python3 -m venv venv
RUN git clone https://github.com/gre-42/trajectory_planning_helpers.git \
 && cd trajectory_planning_helpers \
 && git checkout gre42/mgame-patches \
 && cd .. \
 && venv/bin/pip3 install -e trajectory_planning_helpers
RUN git clone https://github.com/gre-42/global_racetrajectory_optimization.git \
 && cd global_racetrajectory_optimization \
 && git checkout gre42/mgame-patches \
 && cd .. \
 && venv/bin/pip3 install -r global_racetrajectory_optimization/requirements.txt