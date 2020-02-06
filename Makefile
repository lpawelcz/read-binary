all: primes count
primes: primes.c
	gcc -o primes primes.c
count: count.c
	gcc -o count count.c
