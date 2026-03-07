compile: udp_client.c udp_server.c  
	g++ -Wall -o udp_client udp_client.c
	g++ -Wall -o udp_server udp_server.c

run: compile
	./udp_server &
	./udp_client 127.0.0.1 23EC30067

clean:
	rm -f udp_client udp_server
