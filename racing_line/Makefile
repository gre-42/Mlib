build:
	docker build . -t racing_line

raceway1.csv: osm_to_racing_line.py ../../data/levels/raceway1/map_raceway1.osm Makefile
	./osm_to_racing_line.py \
		../../data/levels/raceway1/map_raceway1.osm \
		raceway1.csv \
		--translation raceway1_t.m \
		--rotation raceway1_R.m \
		--start_node_id -114585 \
		--street_width 12

optimize_raceway1: raceway1.csv
	docker run -dit --rm --name racing_line racing_line
	docker cp raceway1.csv racing_line:/global_racetrajectory_optimization/inputs/tracks/
	docker exec racing_line sed -i s/berlin_2018/raceway1/g /global_racetrajectory_optimization/main_globaltraj.py
	docker exec -t racing_line python3 /global_racetrajectory_optimization/main_globaltraj.py
	docker cp racing_line:/global_racetrajectory_optimization/outputs outputs_raceway1
	docker stop racing_line

racing_line_raceway1.m: racing_line_to_mgame.py
	./racing_line_to_mgame.py \
		outputs_raceway1 \
		racing_line_raceway1.m \
		--translation raceway1_t.m \
		--rotation raceway1_R.m
