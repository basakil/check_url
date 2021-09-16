# check_url
A C executable (and related scripts) to check an http endpoint (for availability or for a specific result).

## prerequisites:

### install libcurl:
`sudo apt-get install libcurl4-openssl-dev`

## compile
`gcc -o check_url check_url.c -lcurl`
`gcc -Wall -O2 -o check_url check_url.c -lcurl`
  -ggdb3  for debug stuff
  
## [OPTIONAL] run valgrind memory tests:
`valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./check_url localhost:8080 0`

## run
server.sh script can be used as a test server. eg: `./server.sh 8080 "true"`
then run the resulting binary:
`./check_url localhost:8080 0`

`./check_url [url] [check_option]`
check_option: 0 = check only connection, 1 = check for a text response starting with "true", 200 = check for http 200 return code.

## TODO:
- [ ] use a makefile and change to respective directory structure.
- [ ] check/verify valgrind report results.
- [ ] produce test scripts.
