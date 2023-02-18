cmake_minimum_required(VERSION 3.24)
project(lab1)

set(CMAKE_CXX_STANDARD 11)


find_package(Protobuf REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(CURL REQUIRED)
find_package(gRPC REQUIRED)
find_package(absl REQUIRED)
find_package(opentelemetry-cpp CONFIG REQUIRED)


add_executable(lab_server main.cpp handlers.cpp handlers.h tracer_common.h)

target_link_libraries(lab_server opentelemetry-cpp::ostream_span_exporter opentelemetry-cpp::otlp_http_exporter opentelemetry-cpp::metrics opentelemetry-cpp::sdk)