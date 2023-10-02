all: frontend backend

frontend:
	gcc -o frontend frontend.c

backend:
	gcc -o backend backend.c users_lib.o -pthread

clean:
	rm frontend
	rm backend
