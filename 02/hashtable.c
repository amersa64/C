#include <stdlib.h>
#include <string.h>
#include "hashtable.h"

/* Daniel J. Bernstein's "times 33" string hash function, from comp.lang.C;
   See https://groups.google.com/forum/#!topic/comp.lang.c/lSKWXiuNOAk */
unsigned long hash(char *str) {
  unsigned long hash = 5381;
  int c;

  while ((c = *str++))
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return hash;
}

hashtable_t *make_hashtable(unsigned long size) {
  hashtable_t *ht = malloc(sizeof(hashtable_t));
  ht->size = size;
  ht->buckets = calloc(sizeof(bucket_t *), size);
  return ht;
}

void ht_put(hashtable_t *ht, char *key, void *val) {
  /* FIXME: the current implementation doesn't update existing entries */
  unsigned int idx = hash(key) % ht->size;
	bucket_t *b;
	if(!ht_get(ht, key)){/*checks in if the element already exists*/
		b = malloc(sizeof(bucket_t));
  		b->key = key;
		b->val = val;
  		b->next = ht->buckets[idx];
  		ht->buckets[idx] = b;
	} 
	else{/*I was to call ht_get and set its return value to val but I can't, ask why*/
		b = ht->buckets[idx];
		while (b) {
    		if (strcmp(b->key, key) == 0) {
    		 free(b->val);
    		 free(key);
     	 	 b->val=val;
     	 	 break;
   		 	}
   			 b = b->next;
   		}
   }
}

void *ht_get(hashtable_t *ht, char *key) {
  unsigned int idx = hash(key) % ht->size;
  bucket_t *b = ht->buckets[idx];
  while (b) {
    if (strcmp(b->key, key) == 0) {
      return b->val;
    }
    b = b->next;
  }
  return NULL;
}

void ht_iter(hashtable_t *ht, int (*f)(char *, void *)) {
  bucket_t *b;
  unsigned long i;
  for (i=0; i<ht->size; i++) {
    b = ht->buckets[i];
    while (b) {
      if (!f(b->key, b->val)) {
        return ; // abort iteration
      }
      b = b->next;
    }
  }
}

void free_hashtable(hashtable_t *ht) {
	bucket_t *b;
	bucket_t *temp;
  	unsigned long i;
 	for (i=0; i<ht->size; i++) {
  	 	 b = ht->buckets[i];
   		 while (b) {
   		 	free(b->val);
     		free(b->key);
			temp = b->next;
			free(b);
    		b= temp;
    	}
	}		
 		 free(ht->buckets); //try out later
 		 free(ht); // FIXME: must free all substructures!
}

/* TODO */
void  ht_del(hashtable_t *ht, char *key) {
	unsigned int idx = hash(key) % ht->size;
	  bucket_t *b = ht->buckets[idx];
	  if (b){
	  		if (strcmp(b->key, key)==0){
	  		ht->buckets[idx] =b->next;
	    	free(b->key);
	    	free(b->val);
	    	free(b);
			return;	
			  }
	  while (b->next) {
	    if (strcmp((b->next)->key, key) == 0) {
	    	bucket_t *temp =(b->next)->next;
	    	free((b->next)->key);
	    	free((b->next)->val);
	    	free(b->next);
	    	b->next= temp;
			return;
	    }
	    b = b->next;
	  }
}
}

void  ht_rehash(hashtable_t *ht, unsigned long newsize) {
		hashtable_t *new_ht= make_hashtable(newsize);
		bucket_t *b;
		bucket_t *temp;
  		unsigned long i;
 		 for (i=0; i<ht->size; i++) {
  	 		 b = ht->buckets[i];
   			 while (b) {
     			ht_put(new_ht, b->key, b->val);
     			temp = b->next;
     			free(b);
      			b = temp;
  		}
    }
    free(ht->buckets);
    ht->size=new_ht->size;
    ht->buckets=new_ht->buckets;
    free(new_ht);
  }
