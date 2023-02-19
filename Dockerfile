FROM archlinux:latest

RUN mkdir /home/fib

ADD cmake-build-debug/lab_server /home/fib/
COPY libprotobuf.so /usr/lib
COPY libprotobuf.so.32 /usr/lib
WORKDIR /home/fib

CMD ["./lab_server", "0.0.0.0", "3080"]