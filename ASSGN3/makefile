compile: tcp_client.c tcp_server.c  
	g++ -Wall -o tcp_client tcp_client.c
	g++ -Wall -o tcp_server tcp_server.c

run: compile
	./tcp_server &
	./tcp_client 127.0.0.1 23EC30067

clean:
	rm -f tcp_client tcp_server
