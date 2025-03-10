#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <time.h>
#include <bson/bson.h>
#include <mongoc/mongoc.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include<stdint.h>
#include<unistd.h>
#include"hash.h"
//#include"map.h"
#include"map.h"
#include "data-engine-ext.h"
#define MAP_SIZE  (1 << 16)
#define HASH_CONST          0xa5b35705
map_int_t map_t;
// #define MAP_SIZE  3
//$(pkg-config --libs --cflags libmongoc-1.0)    add this command to gcc option to load mongodb driver
void do_evaluate_seed(c_seed_t * seedEvalTask){
    // empty impl, for compile.
}


void send_binary(mongoc_client_t *client, const char* target_name, char* target_path, char *binary_name, char *describe)
{
	mongoc_collection_t *collection,
						*collection_task_relation;
	bson_error_t error;
	bson_t *doc=bson_new(),
			*doc_tr=bson_new();
	char name[10];

  	collection = mongoc_client_get_collection (client, target_name, "binary");
  	FILE *fp=fopen(target_path, "rb");
	if(!fp)
		printf("open file error\n");
	fseek(fp, 0L, SEEK_END);
	int size=ftell(fp);
	fseek(fp, 0L, SEEK_SET);
 	uint8_t *buf=malloc(size);
 	fread(buf,  sizeof(uint8_t), size, fp);
	fclose(fp);

	int num=size/15000000+1;
	if(num==1)
	{
		BSON_APPEND_BINARY (doc, "code", BSON_SUBTYPE_BINARY, buf, size);
		BSON_APPEND_UTF8(doc, "name", binary_name);
		BSON_APPEND_INT32(doc, "length", size);
		BSON_APPEND_INT32(doc, "flag", 1);
		if(describe!=NULL)
			BSON_APPEND_UTF8(doc, "description", describe);
		if (!mongoc_collection_insert_one (
		collection, doc, NULL, NULL, &error)) {
		fprintf (stderr, "%s\n", error.message);
		}
	}
	else
	{
	    int i=0;
		for(i; i<num; i++)
		{
			sprintf(name, "code%d", i);
			if(i!=num-1)
				BSON_APPEND_BINARY (doc, "code", BSON_SUBTYPE_BINARY, buf+i*15000000, 15000000);
			else
				BSON_APPEND_BINARY (doc, "code", BSON_SUBTYPE_BINARY, buf+i*15000000, size%(i*15000000));
			if(i==0)
			{
				BSON_APPEND_UTF8(doc, "name", binary_name);
				BSON_APPEND_INT32(doc, "length", size);
				BSON_APPEND_INT32(doc, "flag", 1);
			}
			else
				BSON_APPEND_INT32(doc, "flag", 0);
			if (!mongoc_collection_insert_one (
			collection, doc, NULL, NULL, &error)) {
			fprintf (stderr, "%s\n", error.message);
			}
			bson_destroy(doc);
			doc=bson_new();
		}
	}

  	bson_destroy(doc);

	mongoc_collection_destroy (collection);

	//lyy add relation of target&binary to db(Task_Relation)
	collection_task_relation = mongoc_client_get_collection(client, "Task_Relation", binary_name);
	BSON_APPEND_UTF8(doc_tr, "task_name", target_name);
	BSON_APPEND_INT32(doc_tr, "seed_count", 0);
	BSON_APPEND_INT32(doc_tr, "total_crash", 0);
	BSON_APPEND_UTF8(doc_tr, "url", "http://xxx.xxx.xx/");
	if(describe!=NULL)
		BSON_APPEND_UTF8(doc_tr, "description", describe);
	if(!mongoc_collection_insert_one(collection_task_relation, doc_tr, NULL, NULL, &error)){
		fprintf(stderr, "Error_lyy%s\n", error.message );
	}
	bson_destroy(doc_tr);
	mongoc_collection_destroy (collection_task_relation);


	return;
}




void send_seed(mongoc_client_t *client, const char* target_name, u8 *in_dir)
{
  map_init(&map_t);
  mongoc_collection_t *collection1, *collection2, *queue_info_collection;
  bson_error_t error;
  bson_t *doc, *selector, *update;
  const bson_t *doc2;
  uint32_t seed_key;
  int size;
  u32 havoc_time;
    havoc_time = 0;
    u32 seed_id = 1;

    collection1 = mongoc_client_get_collection (client, target_name, "SeedBook");
    collection2 = mongoc_client_get_collection (client, target_name, "seed");
    queue_info_collection = mongoc_client_get_collection (client, target_name, "queue_info");
    printf("%s\n", "get collection ok");
    if(in_dir==NULL)
      in_dir="input";
    u8 locate[100];
    struct dirent **nl;
    int nl_cnt;
    FILE *fp;
    nl_cnt=scandir(in_dir, &nl, NULL, alphasort);
    if(nl_cnt<0){
      printf("indir connot open\n");
      return;
    }
    int i=0;
    for(i; i<nl_cnt; i++){
      if(nl[i]->d_name[0]=='.')
        continue;
      sprintf(locate,"%s/%s",in_dir, nl[i]->d_name);
      fp=fopen(locate, "r");
      if(!fp)
        printf("open file error\n");
      fseek(fp, 0L, SEEK_END);
      size=ftell(fp);
      fseek(fp, 0L, SEEK_SET);
      uint8_t *buf=malloc(size);
      fread(buf,  sizeof(uint8_t), size, fp);
      fclose(fp);
      seed_key=abs(hash32(buf,size,HASH_CONST));
      sprintf(locate, "init%d", seed_key);
      if(map_get(&map_t, locate)){
        continue;
      }

      map_set(&map_t, locate, 1);

      doc=bson_new();
      BSON_APPEND_INT32(doc,"len", size);
      BSON_APPEND_UTF8(doc,"fname", locate);
      // BSON_APPEND_INT32 (doc, "havoc_time", havoc_time);
      BSON_APPEND_INT64(doc,"time",0xffffffff);
      if (!mongoc_collection_insert_one (collection1, doc, NULL, NULL, &error)) {
          fprintf (stderr, "%s\n", error.message);
      }
      bson_destroy(doc);
      doc=bson_new();
      BSON_APPEND_UTF8 (doc, "fname", locate);
    BSON_APPEND_BINARY(doc, "seed", BSON_SUBTYPE_BINARY, buf, size);
    BSON_APPEND_INT32 (doc, "havoc_time", havoc_time);
    BSON_APPEND_INT32(doc, "len", size);
    BSON_APPEND_INT32(doc, "depth", 1);
    BSON_APPEND_INT32(doc, "handicap", 0);
    BSON_APPEND_INT32(doc, "was_fuzzed", 0);
    BSON_APPEND_INT32(doc, "passed_det", 0);
    BSON_APPEND_INT32(doc, "checked", 1);
    BSON_APPEND_INT32(doc, "seed_id", seed_id);
    BSON_APPEND_UTF8(doc,"describe", "original_seeds");
      BSON_APPEND_INT64(doc,"time",0xffffffff);
      if (!mongoc_collection_insert_one (collection2, doc, NULL, NULL, &error)) {
          fprintf (stderr, "%s\n", error.message);
      }else{
        seed_id = seed_id + 1;
      }
      bson_destroy(doc);

    free(buf);
    }
    doc=bson_new();
    u32 queue_cur = 1;
    u64 queue_cycle = 1;
    u32 queue_max = seed_id - 1;
  BSON_APPEND_INT32 (doc, "queue_cur", queue_cur);
  BSON_APPEND_INT32(doc, "queue_max", queue_max);
  BSON_APPEND_INT64(doc, "queue_cycle", queue_cycle);
  BSON_APPEND_INT64(doc, "total_parallel_time", 0);
  BSON_APPEND_DOUBLE(doc, "bitmap_coverage", 0.00);
  BSON_APPEND_INT32(doc, "flag", 1);
  if (!mongoc_collection_insert_one (queue_info_collection, doc, NULL, NULL, &error)) {
        fprintf (stderr, "%s\n", error.message);
    }
    bson_destroy(doc);
    bson_iter_t iter;
    long ii;
    bson_t *query=BCON_NEW("time", "{", "$gte", BCON_INT64(0xffffffff), "}");
    mongoc_cursor_t *cursor= mongoc_collection_find_with_opts (collection1, query, NULL, NULL);
    while(mongoc_cursor_next(cursor, &doc2))
    {
      bson_iter_init_find(&iter, doc2, "_id");
      ii=bson_oid_get_time_t(bson_iter_oid(&iter));
      selector = bson_new();
      BSON_APPEND_OID(selector, "_id", bson_iter_oid(&iter));
      update=BCON_NEW ("$set", "{", "time", BCON_INT64(ii), "}");
    if (!mongoc_collection_update_one (collection1, selector, update, NULL, NULL, &error)){
      fprintf (stderr, "%s\n", error.message);
    }
    bson_destroy(selector);
    bson_destroy(update);
    }
    mongoc_cursor_destroy(cursor);
    cursor= mongoc_collection_find_with_opts (collection2, query, NULL, NULL);
    while(mongoc_cursor_next(cursor, &doc2))
    {
      bson_iter_init_find(&iter, doc2, "_id");
      ii=bson_oid_get_time_t(bson_iter_oid(&iter));
      selector = bson_new();
      BSON_APPEND_OID(selector, "_id", bson_iter_oid(&iter));
      update=BCON_NEW ("$set", "{", "time", BCON_INT64(ii), "}");
    if (!mongoc_collection_update_one (collection2, selector, update, NULL, NULL, &error)){
      fprintf (stderr, "%s\n", error.message);
    }
    bson_destroy(selector);
    bson_destroy(update);
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy (collection1);
    mongoc_collection_destroy (collection2);
    mongoc_collection_destroy (queue_info_collection);
    map_deinit(&map_t);
  return;
}

void insert_mongo_bitmap(mongoc_client_t *client, u8 *target_name){
  mongoc_collection_t *collection;
  bson_error_t error;
  bson_t  *doc=bson_new();
  bson_subtype_t subtype = BSON_SUBTYPE_BINARY;
  collection = mongoc_client_get_collection (client, target_name, "virgin_bits");

  BSON_APPEND_BINARY(doc, "bitmap", BSON_SUBTYPE_BINARY, "0", MAP_SIZE);
  BSON_APPEND_INT32(doc, "flag", 0);
  if (!mongoc_collection_insert_one (collection, doc, NULL, NULL, &error)) {
    fprintf (stderr, "%s\n", error.message);
  }

  bson_destroy(doc);
  mongoc_collection_destroy (collection);
  return;

}

void usage()
{
	printf("Required parameters:\n\n-u       - the url of mongodb server\n");
    printf("-b       - the excuting binary name or the database name\n\n");
}

int main(int argc,  char**argv)
{
	mongoc_init ();
	 char mongo_url[40]="mongodb://";
	char *url=NULL, *target_path=NULL, *ed=NULL;
  char target_name[40], binary_name[40];
  // char *target_name = NULL, binary_name = NULL;
	char *describe=NULL;
	u8 *in_dir=NULL;
	s32 opt;
//  mongoc_client_t *client=NULL;
	while((opt=getopt(argc, argv, "u:b:i:e:m:"))>0)
		switch(opt){
			case 'u':
				url=optarg;
				break;
			case 'b':
				target_path=optarg;
				break;
			case 'i':
				in_dir=optarg;
				break;
			case 'e':
				ed=optarg;
				break;
			case 'm':
				describe=optarg;
				break;
			default:
				usage();
		}
	if (!url || !target_path)
		{
			usage();
			return 0;
		}
    else
        strcat(mongo_url, url);

 	/*
  * mongodb initialization, according to url, target_pat,
  * Create mongo client, and generate target_name, binary_name
  */
  /*
  int ret = init_data_engine_pfcon(url, target_path, ed, &client, target_name, binary_name);

  if(ret == 0){
    return 0;
  }
  printf("ret: %d\n", ret);
  if(client == NULL){
    printf("client is null error\n");
  }
  printf("client:  %s\n",client);
  */
 	if(!url)
 		strcpy(mongo_url, "mongodb://localhost:27017");
 	if(!target_path)
 		target_path="yinqidi";
 	if(strrchr(target_path, '/'))
 		strcpy(target_name, strrchr(target_path, '/')+1);
 	else
 		strcpy(target_name, target_path);
 	strcpy(binary_name, target_name);
 	if(ed!=NULL)
 		strcat(target_name, ed);
 	mongoc_client_t *client=mongoc_client_new (mongo_url);
 	int ii;
 	char ** names;
 	bson_error_t error;
 	if((names=mongoc_client_get_database_names_with_opts (client, NULL, &error)))
 	{
 		for (ii = 0; names[ii]; ii++)
 		{
        	if(strcmp(names[ii], target_name)==0){
        		printf("the target already exists\n");
        		bson_strfreev (names);
        		mongoc_client_destroy (client);
				mongoc_cleanup ();
        		return 0;
        	}
        }
      	bson_strfreev (names);
   	}
   	else {
      fprintf (stderr, "Command failed: %s\n", error.message);
   	}

  printf("target_name: %s\n", target_name);
  printf("binary_name: %s\n", binary_name);

	send_binary(client, target_name, target_path, binary_name, describe);
	if(!in_dir)
		send_seed(client, target_name, NULL);
	else
		send_seed(client, target_name, in_dir);

  insert_mongo_bitmap(client, target_name);

	mongoc_client_destroy (client);
	mongoc_cleanup ();
	return 0;
}
