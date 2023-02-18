FROM archlinux:latest

RUN mkdir /home/fib

ADD cmake-build-debug/lab_server /home/fib/

WORKDIR /home/fib

CMD ["./lab_server", "0.0.0.0", "3080"]