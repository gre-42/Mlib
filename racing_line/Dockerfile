FROM ubuntu:latest

RUN apt-get update && apt-get -y install \
    git \
    python3 \
    python3-pip
RUN git clone https://github.com/TUMFTM/global_racetrajectory_optimization.git
RUN cd global_racetrajectory_optimization \
 && pip3 install -r requirements.txt
