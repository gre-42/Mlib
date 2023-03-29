APP ?= engine-sim

build:
	docker build -t engine-sim .

run:
	docker run -it --rm --volume="$$HOME/.Xauthority:/root/.Xauthority:rw" --net=host --env="DISPLAY" -e XDG_RUNTIME_DIR=/tmp --device /dev/snd engine-sim $(APP)
