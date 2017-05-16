#include "cachelab.h"
#include "getopt.h"
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <math.h>
void cacher(int address);

int main(int argc, char *argv[])
{
  int s=0,miss=0,hit=0,eviction=0,b=0,E=0,index,option = 0,address,size,LRU=0;
  char * file_name;
  char oper;
  //define a structure
  typedef struct {
  	int valid;
  	int tag;
    int LRU;
  }line_t;
  typedef struct{
    line_t* lines;
  }set_t;
 typedef struct{
 	set_t* sets;
 }cache_t;


opterr = 0;

while ((option = getopt (argc, argv, "s:E:b:t:")) != -1)
switch (option)
  {
  case 's':
    s = atoi(optarg);
    break;
  case 'E':
    E = atoi(optarg);
    break;
  case 'b':
    b = atoi(optarg);
    break;
  case 't':
  	file_name = optarg;
  	break;
  case '?':
    if (optopt == 'c')
      fprintf (stderr, "Option -%c requires an argument.\n", optopt);
    else if (isprint (optopt))
      fprintf (stderr, "Unknown option `-%c'.\n", optopt);
    else
      fprintf (stderr,"Unknown option character `\\x%x'.\n",optopt);
    return 1;
  default:
    abort ();
  }
//print the arguments for the file
printf ("s = %d, E = %d, b = %d, t = %s\n", s, E, b,file_name);

cache_t cache;
cache.sets=calloc(sizeof (set_t),pow(2,s));
int c;
for(c=0; c< pow(2,s);c++){
	cache.sets[c].lines=calloc(sizeof (line_t), E);
}


void cacher(int address){
  int mask = (int) pow(2,s)-1;
  int setIndex=(address>>b) & mask;
  int tag=(address>>(b+s));
  int counter;
  int minLRU=0;
        /* code perform hit*/
  for ( counter = 0; counter < E ; counter++) {
    if (cache.sets[setIndex].lines[counter].tag==tag && cache.sets[setIndex].lines[counter].valid==1 ) {
      cache.sets[setIndex].lines[counter].LRU=LRU++;
      hit++;
      return;
    }
  }
  //code for miss
for ( counter = 0; counter < E ; counter++) {
     if(cache.sets[setIndex].lines[counter].valid==0){
       cache.sets[setIndex].lines[counter].valid=1;
       cache.sets[setIndex].lines[counter].tag=tag;
       cache.sets[setIndex].lines[counter].LRU=LRU++;
       miss++;
       return;
     }
}
  //code for miss eviction
  for ( counter = 0; counter < E ; counter++) {
    if(cache.sets[setIndex].lines[counter].LRU < cache.sets[setIndex].lines[minLRU].LRU ){
       minLRU=counter;
    }
  }
  cache.sets[setIndex].lines[minLRU].valid=1;
  cache.sets[setIndex].lines[minLRU].tag=tag;
  cache.sets[setIndex].lines[minLRU].LRU=LRU++;
  miss++;
  eviction++;
}
for (index = optind; index < argc; index++)
  printf ("Non-option argument %s\n", argv[index]);

//parsing a file line by line
   FILE *file = fopen ( file_name, "r" );
   if ( file != NULL )
   {
      char line [ 128 ]; /* or other suitable maximum line size */

      while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
      {
        // fputs ( line, stdout ); /* write the line */
         if(sscanf(line, " %c %x,%d", &oper, &address, &size)!=3){
         	continue;
		      }
          switch (oper) {
            case 'L':cacher(address);
            break;
            case 'M':{cacher(address);cacher(address);};
            break;
            case 'S':cacher(address);
            break;
          }

         //printf("%c %x %d\n",oper, address, size);
      }
      fclose ( file );
   }
   else
   {
      perror ( file_name ); /* why didn't the file open? */
   }
    printSummary(hit, miss, eviction);
    return 0;
}
