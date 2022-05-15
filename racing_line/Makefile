build:
	docker build . -t racing_line

raceway1.csv: ../../data/levels/raceway1/map_raceway1.osm
	./osm_to_racing_line.py ../../data/levels/raceway1/map_raceway1.osm raceway1.csv --start_node_id -114585 --street_width 12

optimize: raceway1.csv
	docker run -dit --rm --name racing_line racing_line
	docker cp raceway1.csv racing_line:/global_racetrajectory_optimization/inputs/tracks/
	docker exec racing_line sed -i s/berlin_2018/raceway1/g /global_racetrajectory_optimization/main_globaltraj.py
	docker exec -t racing_line python3 /global_racetrajectory_optimization/main_globaltraj.py
	docker cp /global_racetrajectory_optimization/outputs outputs
	docker stop racing_line
