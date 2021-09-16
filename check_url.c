#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


//#define DEBUG
#define DATA_SIZE_MAX_BYTES (5)
#define DATA_SIZE_RANGE_STR ("0-4")
#define COMPARE_STR_TRUE ("true")

enum check_expect { RETURN_TRUE = 1, RETURN_2XX = 200, CONNECT_ONLY = 0 };

// Forward declerations:

void START();
void END(int);


void ERR(const char *msg, int code) {
  printf("ERROR: %s\n", msg);
  END(code);
}

void INFO(const char *msg) {
  printf("INFO : %s\n", msg);
}

struct MemoryStruct {
  char *memory;
  size_t size;
};

struct MemoryStruct chunk;

CURL *curl = NULL;

void START() {
  chunk.memory = malloc(DATA_SIZE_MAX_BYTES); 
  chunk.size = 0;

  CURL *curl = curl_easy_init();
  if (! curl) {
    ERR("Could not initialize curl", 1);
  }
}

void END(int code) {
  free(chunk.memory);
  if (curl) {
    curl_easy_cleanup(curl);
  }
  exit(code);
}

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  
  size_t real_size = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  // calculate copy_size
  size_t copy_size = DATA_SIZE_MAX_BYTES - 1 - mem->size;
  if (copy_size <= 0) {
#ifdef DEBUG
    printf("read unnecessary %ld bytes..", real_size);
#endif
    return real_size;    
  }
  if (copy_size > real_size) {
    copy_size = real_size;
  }

  memcpy(&(mem->memory[mem->size]), contents, copy_size);
  mem->size += copy_size;
  mem->memory[mem->size] = 0;

  return real_size;
}

void pre_set(CURL *curl) {

#ifdef DEBUG
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif
  // we need only first 4 bytes..
  curl_easy_setopt(curl, CURLOPT_RANGE, DATA_SIZE_RANGE_STR);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
}

bool perform(CURL *curl) {

    int res = curl_easy_perform(curl);

    if(res != CURLE_OK) {
#ifdef DEBUG
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
#endif
    }
    
    return res == CURLE_OK ? true : false;
}

bool check_url(CURL *curl, const char *url, enum check_expect expect, struct MemoryStruct *data) {

  curl_easy_setopt(curl, CURLOPT_URL, url);
  
  if        ( expect == CONNECT_ONLY ) {

    curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1L);
    
    return perform(curl);

  } else if ( expect == RETURN_2XX ) {

    curl_easy_setopt(curl, CURLOPT_NOBODY, 1);

    if ( perform (curl) != true) {
      return false;
    }
    long http_code = 0;
    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
    return (http_code >= 200 && http_code < 300 ? true : false);

  } else if ( expect == RETURN_TRUE ) {

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)data);

    if (! perform(curl) ) {
      return false;
    }
#ifdef DEBUG
    printf("%lu bytes retrieved\n", (long)chunk.size);
    printf("received: %s\n", chunk.memory);
#endif
    
    return strncmp(COMPARE_STR_TRUE, chunk.memory, DATA_SIZE_MAX_BYTES - 1) == 0 ? true : false;

  } else {
    ERR("unknown check_expect: %d", expect);
  }
  
  return false; // should only happen in unknown check_expect ..

}

int main(int argc, char *argv[]) {

#ifdef DEBUG  
  printf("program %s started..\n", argv[0]);
#endif

  START();

  CURL *curl = curl_easy_init();
  if (! curl) {
    ERR("Could not initialize curl", 1);
  }
  
  //TODO: use getopt or argp for argument parsing..
  int ret = check_url( curl, argv[1], strtol( argv[2], (char **)NULL, 10 ), &chunk );

#ifdef DEBUG
  printf("program %s returned: %s\n", argv[0], (ret==true?"true":"false"));  
#endif
 
  END(ret==true?0:1);

}

