all: comp docker

comp:
	cmake --build cmake-build-debug --target lab_server -j 12

docker:
	docker build -f ./Dockerfile -t fib_server:v1.0 .