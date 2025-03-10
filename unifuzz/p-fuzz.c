#include "p-fuzz.h"



/* Count the number of bits set in the provided bitmap. Used for the status
   screen several times every second, does not have to be fast. */


/* Set up the mongo connection */
mongoc_client_t* mongo_conn_init(char *url_site){
	mongoc_init();
	mongoc_client_t *client= mongoc_client_new (url_site);
	return client;
}

/* Stop the mongo connection */
void mongo_conn_stop(mongoc_client_t *client){
	mongoc_client_destroy (client);
	mongoc_cleanup ();
	return;
}

/* Do the and operation on two bitmap */
void operate(u8*virgin_map, const uint8_t*new_bits)
{
  #ifdef __x86_64__

  u64* current = (u64*)new_bits;
  u64* virgin  = (u64*)virgin_map;

  u32  i = (MAP_SIZE >> 3);

#else

  u32* current = (u32*)new_bits;
  u32* virgin  = (u32*)virgin_map;

  u32  i = (MAP_SIZE >> 2);

#endif 

  while (i--) {

    *virgin &= *current;
    current++;
    virgin++;

  }
}

/* Download the to-be-fuzzed binary */


/* Download the to-be-fuzzed binary */
void load_mongo_binary(mongoc_client_t *client, u8 *target_name, u8 *binary_name){
    printf("%s\n", "in load binary");
    mongoc_collection_t *collection= mongoc_client_get_collection (client, target_name, "binary");
    bson_iter_t iter;
    mongoc_cursor_t *cursor;
    const uint8_t *strings;
    bson_t query;
    bson_init(&query);
    const bson_t *doc;
    bson_subtype_t subtype = BSON_SUBTYPE_BINARY;
    uint32_t binary_len=0;
    int size;

    printf("%s\n", "before find");
    cursor = mongoc_collection_find_with_opts (collection, &query, NULL, NULL);
    printf("%s\n", "before open");
    FILE *fp=fopen(binary_name,"wb");
    printf("%s\n", "after open");
    // if(mongoc_cursor_next(cursor, &doc))
    // {
    //   bson_iter_init_find(&iter, doc, "length");
    //   int size=bson_iter_int32(&iter);
    //   if (size<=15000000)
    //   {
    //     bson_iter_init_find(&iter, doc, "code");
    //     bson_iter_binary(&iter, &subtype, &binary_len, &strings);
    //     fwrite(strings, sizeof(uint8_t), binary_len, fp);
    //   }
    //   else
    //   {
    //     char name[50]
    //     int num=size/15000000+1;
    //     for(i )
    //   }
    //   chmod(binary_name, S_IRUSR|S_IWUSR|S_IXUSR);

    //   bson_destroy(&query);
    //   mongoc_cursor_destroy (cursor);
    //   mongoc_collection_destroy(collection);
    // }
    while (mongoc_cursor_next(cursor, &doc))
    {
        bson_iter_init_find(&iter, doc, "flag");
        int flag=bson_iter_int32(&iter);
        if(flag==1)
        {
            bson_iter_init_find(&iter, doc, "length");
            size=bson_iter_int32(&iter);
            bson_iter_init_find(&iter, doc, "code");
            bson_iter_binary(&iter, &subtype, &binary_len, &strings);
            fwrite(strings, sizeof(uint8_t), binary_len, fp);
            if (size<=15000000)
                break;
        }
        else
        {
            bson_iter_init_find(&iter, doc, "code");
            bson_iter_binary(&iter, &subtype, &binary_len, &strings);
            fwrite(strings, sizeof(uint8_t), binary_len, fp);
        }
    }
    printf("%s\n", "before fclose");
    fclose(fp);
    chmod(binary_name, S_IRUSR|S_IWUSR|S_IXUSR);
    return;
}

/* When finding new coverage, insert the bitmap in mongodb and share it */
void insert_mongo_bitmap(mongoc_client_t *client, u8 *target_name, u8 *virgin_b){
  mongoc_collection_t *collection;
  bson_iter_t iter;
  bson_error_t error;
  mongoc_cursor_t *cursor;
  bson_t  *doc=bson_new(), *query=bson_new();
  const bson_t *search;
  bson_subtype_t subtype = BSON_SUBTYPE_BINARY;
  collection = mongoc_client_get_collection (client, target_name, "virgin_bits");
  uint32_t bitmap_len;
  BSON_APPEND_INT32(query, "flag", 1);

  bson_t *opts = BCON_NEW ("sort", "{", "time", BCON_INT32 (1), "}");
  cursor=mongoc_collection_find_with_opts (collection, query, opts, NULL);
  if(! mongoc_cursor_next (cursor, &search))
  {
    time_t myt=time(NULL);
    long ii = time(&myt);
    BSON_APPEND_INT64(doc, "time", ii);
    BSON_APPEND_BINARY(doc, "bitmap", BSON_SUBTYPE_BINARY, virgin_b, MAP_SIZE);
    BSON_APPEND_INT32(doc, "flag", 1);
    if (!mongoc_collection_insert_one (collection, doc, NULL, NULL, &error)) {
      fprintf (stderr, "%s\n", error.message);
    }
  }
  else
  {
    const uint8_t *out_bitmap;
    if (bson_iter_init_find(&iter, search, "bitmap"))
      bson_iter_binary(&iter, &subtype, &bitmap_len, &out_bitmap);
    operate(virgin_b, out_bitmap);
    BSON_APPEND_BINARY(doc, "bitmap", BSON_SUBTYPE_BINARY, virgin_b, MAP_SIZE);
    BSON_APPEND_INT32(doc, "flag", 1);
    time_t myt=time(NULL);
    long ii = time(&myt);
    BSON_APPEND_INT64(doc, "time", ii);
    bson_destroy(query);
    query=BCON_NEW("time", "{", "$lt", BCON_INT64 (ii), "}");
    bson_t *update=BCON_NEW ("$set",
                      "{",
                      "flag",
                      BCON_INT32 (0),
                      "}");
    if (!mongoc_collection_insert_one (collection, doc, NULL, NULL, &error))
      fprintf (stderr, "%s\n", error.message);

    if (!mongoc_collection_update_many (collection, query, update, NULL, NULL, &error)) 
      fprintf (stderr, "%s\n", error.message);
    bson_destroy(update);
  }
  bson_destroy(opts);
  bson_destroy(doc);
  bson_destroy(query);
  mongoc_cursor_destroy(cursor);
  mongoc_collection_destroy (collection);
  return;

}

/* Read the up-to-date bitmap from mongodb */
u8 read_mongo_bitmap(mongoc_client_t *client, u8 *target_name, u8 *type_name,  u8 *virgin_b){
  u8 result=0;
  mongoc_collection_t *collection;
  bson_iter_t iter;
  mongoc_cursor_t *cursor;
  bson_t *query;
  const bson_t *doc;
  bson_subtype_t subtype = BSON_SUBTYPE_BINARY;
  uint32_t binary_len=0;
  const uint8_t *virgin;

  collection = mongoc_client_get_collection (client, target_name, type_name);

  query=bson_new();
  BSON_APPEND_INT32(query, "flag", 1);
  
  cursor = mongoc_collection_find_with_opts (collection, query, NULL, NULL);
  if (mongoc_cursor_next (cursor, &doc))
      {
        if (bson_iter_init_find(&iter, doc, "bitmap")) {
          bson_iter_binary(&iter, &subtype, &binary_len, &virgin);
          result=1;
          if(count_non_255_bytes2(virgin_b)!=count_non_255_bytes3(virgin))
          {
            memcpy(virgin_b,virgin,MAP_SIZE);
          }
        }
      }
  
  bson_destroy(query);
  mongoc_cursor_destroy (cursor);
  mongoc_collection_destroy (collection);
  return result;
}

/* If needed, update the seed content in the mongodb */
u8 update_mongo_seed(mongoc_client_t *client, u8 *target_name, u8* fname, u8* seed, u64 len){
  mongoc_collection_t *collection;
  bson_error_t error;
  bson_t *doc;
  bson_t *update;
  u32 result=1;
  
  seed[len]='\0';
  collection = mongoc_client_get_collection (client, target_name, "seed");

  doc=bson_new();
  BSON_APPEND_UTF8(doc, "fname", fname);
  update=bson_new();
  BSON_APPEND_INT64(doc, "len", len);
  BSON_APPEND_BINARY(doc, "seed", BSON_SUBTYPE_BINARY, seed, len);


   if (!mongoc_collection_update_one (collection, doc, update, NULL, NULL, &error)){
      fprintf (stderr, "%s\n", error.message);
      result=0;
   }

  bson_destroy(doc);
  bson_destroy(update);
  mongoc_collection_destroy (collection);
  return result;
}

/* When finding a new seed, insert it in the mongodb */
void insert_mongo_seed(mongoc_client_t *client, u8 * target_name, u8 * type_name, u8 *const fname, u8 *const mem,  u32 len, u64 time, u8 *const describe){
  mongoc_collection_t *collection;
  bson_error_t error;
  bson_t *doc;
  int32_t havoc_time;
  havoc_time = 0;
  collection = mongoc_client_get_collection (client, target_name, type_name);

  doc = bson_new ();
  BSON_APPEND_UTF8 (doc, "fname", fname);
  BSON_APPEND_BINARY(doc, "seed", BSON_SUBTYPE_BINARY, mem, len);
  BSON_APPEND_INT32 (doc, "havoc_time", havoc_time);
  BSON_APPEND_INT32(doc, "len", len);
  if (describe)
    BSON_APPEND_UTF8(doc,"describe",describe);
  else
    BSON_APPEND_UTF8(doc,"describe","original_seeds");
  BSON_APPEND_INT64(doc,"time",time);

  if (!mongoc_collection_insert_one (
         collection, doc, NULL, NULL, &error)) {
      fprintf (stderr, "%s\n", error.message);
  }

  bson_destroy (doc);
  mongoc_collection_destroy (collection);
  return;
}

void insert_mongo_seed_in_queue(mongoc_client_t *client, u8 * target_name, u8 *const fname, u8 *const mem,  u32 len, u32 depth, u32 handicap, u64 time, u8 *const describe){
  mongoc_collection_t *collection, *queue_info_collection;
  bson_error_t error;
  bson_t *doc;
  const bson_t *doc_queue;
  int32_t havoc_time;
  havoc_time = 0;
  u32 queue_max = 0;
  u32 seed_id=0;
  int32_t flag=1;
  bson_t *query, *update, *doc_selector;
  mongoc_cursor_t *cursor;
  bson_iter_t iter;

  queue_info_collection = mongoc_client_get_collection (client, target_name, "queue_info");
  query=bson_new();
  BSON_APPEND_INT32(query,"flag",flag);
  cursor = mongoc_collection_find_with_opts (queue_info_collection, query, NULL, NULL);
  if (mongoc_cursor_next (cursor, &doc_queue))
  {
    if (bson_iter_init_find(&iter, doc_queue, "queue_max")) {
      queue_max = bson_iter_int32(&iter);
      queue_max = queue_max + 1;
      seed_id = queue_max;
    }
  }

  doc_selector = BCON_NEW("flag", BCON_INT32(flag));
  update = BCON_NEW("$set", "{", "queue_max", BCON_INT32(queue_max), "}");
  if (!mongoc_collection_update_one (queue_info_collection, doc_selector, update, NULL, NULL, &error)) {
    fprintf (stderr, "%s\n", error.message);
  }

  mongoc_collection_destroy (queue_info_collection);
  bson_destroy(update);
  bson_destroy(doc_selector);
  bson_destroy(query);
  bson_destroy (doc_queue);
  mongoc_cursor_destroy (cursor);


  collection = mongoc_client_get_collection (client, target_name, "seed");

  doc = bson_new ();
  BSON_APPEND_UTF8 (doc, "fname", fname);
  BSON_APPEND_BINARY(doc, "seed", BSON_SUBTYPE_BINARY, mem, len);
  BSON_APPEND_INT32 (doc, "havoc_time", havoc_time);
  BSON_APPEND_INT32(doc, "len", len);
  BSON_APPEND_INT32(doc, "seed_id", seed_id);
  BSON_APPEND_INT32(doc, "depth", depth);
  BSON_APPEND_INT32(doc, "handicap", handicap);
  BSON_APPEND_INT32(doc, "was_fuzzed", 0);
  BSON_APPEND_INT32(doc, "checked", 0);
  BSON_APPEND_INT32(doc, "passed_det", 0);
  if (describe)
    BSON_APPEND_UTF8(doc,"describe",describe);
  else
    BSON_APPEND_UTF8(doc,"describe","original_seeds");
  BSON_APPEND_INT64(doc,"time",time);

  if (!mongoc_collection_insert_one (
         collection, doc, NULL, NULL, &error)) {
      fprintf (stderr, "%s\n", error.message);
  }

  bson_destroy (doc);
  mongoc_collection_destroy (collection);
  return;
}

/* Get seed from mongodb according to the fname */
u8 *read_mongo_seed(mongoc_client_t *client, u8 *target_name,  const u8* fname, u32 len){
  mongoc_collection_t *collection;
  const bson_t *doc;
  bson_iter_t iter;
  mongoc_cursor_t *cursor;
  bson_t *query;
  const u8 *strs=NULL;
  bson_subtype_t subtype = BSON_SUBTYPE_BINARY;
  uint32_t binary_len=0;

  collection = mongoc_client_get_collection (client, target_name, "seed");
  query=bson_new();
  BSON_APPEND_UTF8(query,"fname",fname);
  cursor = mongoc_collection_find_with_opts (collection, query, NULL, NULL);
  if (mongoc_cursor_next (cursor, &doc))
  {
    if (bson_iter_init_find(&iter, doc, "seed")) {
      bson_iter_binary(&iter, &subtype, &binary_len, &strs);
    }
  }
  mongoc_cursor_destroy (cursor);
  bson_destroy(query);
  mongoc_collection_destroy (collection);
  return strs;
}

/* When finding new coverage in virgin_tmout, insert it to mongodb */
void insert_tmout_bitmap(mongoc_client_t *client, u8 *target_name, u8 *virgin_b){
  if (count_non_255_bytes2(virgin_b)==65536)
    return;
  mongoc_collection_t *collection;
  bson_iter_t iter;
  bson_error_t error;
  mongoc_cursor_t *cursor;
  bson_t  *doc=bson_new(), *query=bson_new();
  const bson_t *search;
  bson_subtype_t subtype = BSON_SUBTYPE_BINARY;
  collection = mongoc_client_get_collection (client, target_name, "virgin_tmout");
  uint32_t bitmap_len;
  BSON_APPEND_INT32(query, "flag", 1);

  bson_t *opts = BCON_NEW ("sort", "{", "time", BCON_INT32 (1), "}");
  cursor=mongoc_collection_find_with_opts (collection, query, opts, NULL);
  if(! mongoc_cursor_next (cursor, &search))
  {
    time_t myt=time(NULL);
    long ii = time(&myt);
    BSON_APPEND_INT64(doc, "time", ii);
    BSON_APPEND_BINARY(doc, "bitmap", BSON_SUBTYPE_BINARY, virgin_b, MAP_SIZE);
    BSON_APPEND_INT32(doc, "flag", 1);
    if (!mongoc_collection_insert_one (collection, doc, NULL, NULL, &error)) {
      fprintf (stderr, "%s\n", error.message);
    }
  }
  else
  {
    const uint8_t *out_bitmap;
    if (bson_iter_init_find(&iter, search, "bitmap"))
      bson_iter_binary(&iter, &subtype, &bitmap_len, &out_bitmap);
    printf("run%d,\t", count_non_255_bytes2(virgin_b));
    operate(virgin_b, out_bitmap);
    BSON_APPEND_BINARY(doc, "bitmap", BSON_SUBTYPE_BINARY, virgin_b, MAP_SIZE);
    BSON_APPEND_INT32(doc, "flag", 1);
    time_t myt=time(NULL);
    long ii = time(&myt);
    BSON_APPEND_INT64(doc, "time", ii);
    bson_destroy(query);
    query=BCON_NEW("time", "{", "$lt", BCON_INT64 (ii), "}");
    bson_t *update=BCON_NEW ("$set",
                      "{",
                      "flag",
                      BCON_INT32 (0),
                      "}");
    if (!mongoc_collection_insert_one (collection, doc, NULL, NULL, &error))
      fprintf (stderr, "%s\n", error.message);

    if (!mongoc_collection_update_many (collection, query, update, NULL, NULL, &error)) 
      fprintf (stderr, "%s\n", error.message);
    bson_destroy(update);
  }
  bson_destroy(opts);
  bson_destroy(doc);
  bson_destroy(query);
  mongoc_cursor_destroy(cursor);
  mongoc_collection_destroy (collection);
  return;

}

/* When finding new coverage in virgin_crash, insert it to mongodb */
void insert_crash_bitmap(mongoc_client_t *client, u8 *target_name, u8 *virgin_b){
  if (count_non_255_bytes2(virgin_b)==65536)
    return;
  mongoc_collection_t *collection;
  bson_iter_t iter;
  bson_error_t error;
  mongoc_cursor_t *cursor;
  bson_t  *doc=bson_new(), *query=bson_new();
  const bson_t *search;
  bson_subtype_t subtype = BSON_SUBTYPE_BINARY;
  collection = mongoc_client_get_collection (client, target_name, "virgin_crash");
  uint32_t bitmap_len;
  BSON_APPEND_INT32(query, "flag", 1);

  bson_t *opts = BCON_NEW ("sort", "{", "time", BCON_INT32 (1), "}");
  cursor=mongoc_collection_find_with_opts (collection, query, opts, NULL);
  if(! mongoc_cursor_next (cursor, &search))
  {
    time_t myt=time(NULL);
    long ii = time(&myt);
    BSON_APPEND_INT64(doc, "time", ii);
    BSON_APPEND_BINARY(doc, "bitmap", BSON_SUBTYPE_BINARY, virgin_b, MAP_SIZE);
    BSON_APPEND_INT32(doc, "flag", 1);
    if (!mongoc_collection_insert_one (collection, doc, NULL, NULL, &error)) {
      fprintf (stderr, "%s\n", error.message);
    }
  }
  else
  {
    const uint8_t *out_bitmap;
    if (bson_iter_init_find(&iter, search, "bitmap"))
      bson_iter_binary(&iter, &subtype, &bitmap_len, &out_bitmap);
    printf("run%d,\t", count_non_255_bytes2(virgin_b));
    operate(virgin_b, out_bitmap);
    BSON_APPEND_BINARY(doc, "bitmap", BSON_SUBTYPE_BINARY, virgin_b, MAP_SIZE);
    BSON_APPEND_INT32(doc, "flag", 1);
    time_t myt=time(NULL);
    long ii = time(&myt);
    BSON_APPEND_INT64(doc, "time", ii);
    bson_destroy(query);
    query=BCON_NEW("time", "{", "$lt", BCON_INT64 (ii), "}");
    bson_t *update=BCON_NEW ("$set",
                      "{",
                      "flag",
                      BCON_INT32 (0),
                      "}");
    if (!mongoc_collection_insert_one (collection, doc, NULL, NULL, &error))
      fprintf (stderr, "%s\n", error.message);

    if (!mongoc_collection_update_many (collection, query, update, NULL, NULL, &error)) 
      fprintf (stderr, "%s\n", error.message);
    bson_destroy(update);
  }
  bson_destroy(opts);
  bson_destroy(doc);
  bson_destroy(query);
  mongoc_cursor_destroy(cursor);
  mongoc_collection_destroy (collection);
  return;
}