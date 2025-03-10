#include "data-engine.h"

#include <iostream>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <fstream>
#include <list>

using namespace std;

extern "C" {

/*
* The initialization of mongodb uses a Singleton. Create and initialize the mongo client
* Parameters: uri_string the ip address and port of the database. For example, localhost:27017
*/
int init_data_engine_c(char *db, char *uri_string, char *target_name, char *version) {
    /*
    * Create a new client instance
    */
    MongoDataEngine *data = init_data_engine(db, uri_string, target_name, version);
    //        data->client  = mongoc_client_new_from_uri (uri);
    if (!data->client) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


/*
* destroy mongo client
*/
int destroy_data_engine_c(){
    MongoDataEngine *data = MongoDataEngine::get_instance();
    mongoc_client_destroy(data->client);
    mongoc_cleanup();
    MongoDataEngine::destroy();
    return 0;
}


//Update the seed information, update the value of the checked field to 1 based on the seed name
int update_seed_check_data_engine_c(char *qtopfname){
    MongoDataEngine *data = MongoDataEngine::get_instance();
    int ret = data->update_seed_check_data_engine(qtopfname);
    return ret;
}

int delete_seed_check_fname_data_engine_c(int check, char *fname ){
    MongoDataEngine *data = MongoDataEngine::get_instance();
    int ret = data->delete_seed_check_fname_data_engine(check,fname);
    return ret;
}


//Get the queue_cycle value of the queue_info table.
unsigned long long get_queue_info_cycle_number_c(){
    MongoDataEngine *data = MongoDataEngine::get_instance();
    unsigned long long queue_cycle = data->get_queue_info_cycle_number();
    return queue_cycle;
}

//The update seed was_fuzzed value is 1
int mark_seed_was_fuzzed_c(char *fname){
    MongoDataEngine *data = MongoDataEngine::get_instance();
    return data->mark_seed_was_fuzzed(fname);
}

//According to fname, update handicap.
int update_seed_handicap_c(char *fname, int handicap){
    MongoDataEngine *data = MongoDataEngine::get_instance();
    return data->update_seed_handicap(fname, handicap);
}

///* When finding a new seed, insert it in the mongodb */
int insert_seed_c( c_seed_t *seed, char *type_name, char *describe){
    MongoDataEngine *data = MongoDataEngine::get_instance();
    return data->insert_seed(seed, type_name, describe);
}

//
int insert_queue_info_c( c_seed_t *seed, char *describe){
    MongoDataEngine *data = MongoDataEngine::get_instance();
    int ret = data->insert_queue_info(seed, describe );
    return ret;
}


/* If needed, update the seed content in the mongodb */
int update_seed_c(char* fname, u8* seed, u64 len){
    MongoDataEngine *data = MongoDataEngine::get_instance();
    return data->update_seed(fname, seed, len);
}

int upload_init_bitmap_to_data_engine_c( u8 *virgin_b){
    MongoDataEngine *data = MongoDataEngine::get_instance();
    return data->upload_init_bitmap_to_data_engine(virgin_b);
}
int update_bitmap_to_data_engine_c(u8 *virgin_b){
    MongoDataEngine *data = MongoDataEngine::get_instance();
    return data->update_bitmap_to_data_engine(virgin_b);
}
int load_bitmap_from_data_engine_c(u8 *virgin_b){
    MongoDataEngine *data = MongoDataEngine::get_instance();
    return data->load_bitmap_from_data_engine(virgin_b);
}

void update_cycle_number_c(u64 queue_cycle){
    MongoDataEngine *data = MongoDataEngine::get_instance();
    return data->update_cycle_number(queue_cycle);
}
void update_bitmap_cvg_c(double bitmap_cvg){
    MongoDataEngine *data = MongoDataEngine::get_instance();
    return data->update_bitmap_cvg(bitmap_cvg);
}


c_seed_t *read_all_seeds_c() {
    MongoDataEngine *data = MongoDataEngine::get_instance();
    std::list<c_seed_t *> seed_list;
    data->read_all_seeds(seed_list);
    c_seed_t *curr = nullptr, *head = nullptr;
    if(seed_list.size() == 0){
        return nullptr;
    }
    for (std::list<c_seed_t *>::iterator i = seed_list.begin(); i != seed_list.end(); i++) {
        c_seed_t *se = *i;
        if (head == nullptr) {
            head = se;
            curr = se;
        } else {
            curr->next = se;
            curr = se;
        }
    }
    return head;
}

/*
* Use mongodb's command_simple to test databases, test functions.
* The normal print is: command: { "ping" : 1 }
*                        { "ok" : 1.0 }
*/
int test_data_engine_command_simple() {
    bson_t *command, reply;
    bson_error_t error;
    char *str;
    bool retval;
    mongoc_cursor_t *cursor;
    const bson_t *doc;
    bson_t *query;
    int32_t checked = 0;

    mongoc_collection_t *collection;
    /*
    * Do work. This example pings the database, prints the result as JSON and
    * performs an insert
    */
    // DataEngine * data = DataEngine::Instance();
    // 调用MongoDataEngine的对象.
    MongoDataEngine *data = MongoDataEngine::get_instance();
    collection = mongoc_client_get_collection(data->client, "whoABC", "seed");
//        query = BCON_NEW("checked", BCON_INT32(checked));
    query = bson_new();
    cursor = mongoc_collection_find_with_opts(collection, query, NULL, NULL);
    while (mongoc_cursor_next(cursor, &doc)) {
        str = bson_as_canonical_extended_json(doc, NULL);
        printf("%s\n", str);
        bson_free(str);
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(collection);
    mongoc_cleanup();
    return EXIT_SUCCESS;
}



} //end extern "C"



/*  -----  c++ function ----- */
/* ------  MongoDataEngine member functions ---------*/
MongoDataEngine *MongoDataEngine::data_engine = nullptr; //static member variables

//Get the MongoDataEngine object
MongoDataEngine *get_data_engine() {
    MongoDataEngine *data = MongoDataEngine::get_instance();
    if (data->data_engine == nullptr) {
        assert(false); //data_engine initialization is required
    }
    return data->data_engine;
}

timespec show_timespec(timespec start, timespec end, char* phrase)
{
    timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
//    return temp;
//    std::cout<<"The time spent: "<<phrase<<std::endl;
    printf("The time spent: %s #", phrase);
//    std::cout<<temp.tv_sec<<":"<<(temp.tv_nsec / 1000000000.0) * 1000.0 <<" (ms) "<< std::endl;
    printf("%ld (s): %f (ms) \n", temp.tv_sec, (temp.tv_nsec / 1000000000.0) * 1000.0);
//    std::cout<<temp.tv_sec<<":"<<temp.tv_nsec <<" (us) "<< std::endl;
//    printf("%ld (s): %ld (us) \n", temp.tv_sec, temp.tv_nsec );

//    std::cout<<start.tv_sec<<":"<<start.tv_nsec <<" (time1) "<< std::endl;
//    std::cout<<end.tv_sec<<":"<<end.tv_nsec <<" (time2) "<< std::endl;
    std::cout<<""<<std::endl;
    return temp;
}

void my_gettime(timespec* t){
    clock_gettime(CLOCK_REALTIME, t);
}

//Initialize the MongoDataEngine object
MongoDataEngine *init_data_engine(std::string db, std::string ip_port, std::string target_name, std::string version) {
    MongoDataEngine *data = MongoDataEngine::instance(db, ip_port, target_name, version);
    return data;
}

//The value of the passed_det is 1, according to the sed's fname update
int MongoDataEngine::mark_seed_passed_det(char *fname){
    mongoc_collection_t *collection;
    bson_t *doc_selector;
    bson_t *update;
    int32_t flag=1;

    doc_selector = BCON_NEW("fname", BCON_UTF8(fname));
    update = BCON_NEW("$set", "{", "passed_det", BCON_INT32(flag), "}");

    this->update_information(doc_selector, update, "seed");

    bson_destroy(update);
    bson_destroy (doc_selector);
    mongoc_collection_destroy (collection);
    return 0;
}

//Returns data_engine instance
MongoDataEngine *MongoDataEngine::get_instance() {
    if (NULL == data_engine) {
        printf("data_engine is null\n");
        assert(false);
    }
//    cout << "   ....MongoDataEngine::get_instance..." << endl;
    return data_engine;
}

//Initialize using the single-case mode mongodb.
MongoDataEngine *MongoDataEngine::instance(std::string db, std::string ip_port, std::string target_name, std::string version) {
    if (NULL == data_engine) {
        data_engine = new MongoDataEngine(db, ip_port, target_name, version);
        mongoc_init();
        std::string mongo_url = "mongodb://";
        mongo_url.append(ip_port);
        /*
        * Safely create a MongoDB URI object from the given string
        */
        bson_error_t error;
        mongoc_uri_t *uri;
        uri = mongoc_uri_new_with_error(mongo_url.c_str(), &error);
        if (!uri) {
            fprintf(stderr,
                    "failed to parse URI: %s\n"
                    "error message:       %s\n",
                    mongo_url.c_str(),
                    error.message);
            exit(0);
            return data_engine;
        }
        data_engine->pool = mongoc_client_pool_new (uri);
        mongoc_client_pool_set_error_api (data_engine->pool, 2);

        data_engine->client = mongoc_client_new_from_uri(uri);
        if (!data_engine->client) {
            data_engine->client = nullptr;
            printf("mongo connection failed\n");
            exit(0);
        }
        int ii;
        char **names;
        int flag = 0;
        if ((names = mongoc_client_get_database_names_with_opts(data_engine->client, NULL, &error))) {
            for (ii = 0; names[ii]; ii++) {
                if (strcmp(names[ii], target_name.c_str()) == 0) {
                    flag = 1;
                    break;
                }
            }
            bson_strfreev(names);
        } else {
            fprintf(stderr, "Command failed: %s\n", error.message);
        }
        if (flag == 0) {
            data_engine->client = nullptr;
            printf("mongo init failed \n The task %s was not found in Mongo\n\n", target_name.c_str());
            exit(0);
        }
    }
//    cout<< "   ....MongoDataEngine::Instance()...mongo_init..\n" << endl;
    return data_engine;
}

MongoDataEngine::MongoDataEngine(std::string db, std::string ip_port, std::string target_name, std::string version) :
        db_name(db), db_ip_port(ip_port), target_name(target_name), target_version(version) {

}


void MongoDataEngine::destroy() {
    mongoc_client_pool_destroy (data_engine->pool);
    delete data_engine;
    data_engine = NULL;
    std::cout << "   ....MongoDataEngine::destroy()..." << std::endl;
}

/*
 * Feature: Launch a new query, according to target_name, version:"seed", query
 * Get the mongo's mongoc_cursor_t scursor cursor cursor
 * */
bool MongoDataEngine::reset_seed_iterator(bson_t *query) {
    mongoc_collection_t *collection;
    collection = mongoc_client_get_collection(client, target_name.c_str(), "seed");
    printf("debug reset_cursor_seed\n");
    cursor_seed = mongoc_collection_find_with_opts(collection, query, NULL, NULL);
    return true;
}


/*
 * Feature: Launch a new query,
 * Get the mongo's mongoc_cursor_t scursor cursor cursor
 * */
bool MongoDataEngine::reset_binary_iterator() {
    mongoc_collection_t *collection;
    bson_t *query = bson_new();
    collection = mongoc_client_get_collection(client, target_name.c_str(), "binary");
    cursor_binary = mongoc_collection_find_with_opts(collection, query, NULL, NULL);
    return true;
}

/*
 * Function: Get the current value based on the mongoc_cursor_t scursor cursor cursor and point to the next one.
 * */
c_seed_t *MongoDataEngine::get_next_seed(mongoc_cursor_t *cursor) {
//    assert(false);
    /*  If the parameter is empty, use the member variable  */
    if (cursor == nullptr) {
        cursor = this->cursor_seed;
    }
    const bson_t *doc;
    bson_iter_t iter;
    bson_error_t error;
    bson_subtype_t subtype = BSON_SUBTYPE_BINARY;
    uint32_t binary_len = 0; //种子长度size
    c_seed_t *sd_cur = (c_seed_t*)malloc(sizeof (c_seed_t));
    if (mongoc_cursor_next(cursor, &doc) == false) {
        //The iterative cursor encountered an error.
        if (mongoc_cursor_error(cursor, &error)) {
            fprintf(stderr, "Failed to iterate all documents: %s\n", error.message);
        }
        sd_cur = nullptr;
        return sd_cur;
    }
//    print_bson(doc);
    bson_iter_init_find(&iter, doc, "time");
    sd_cur->time = bson_iter_int64(&iter);
//    printf("\nsd_cur->time: %lld\n", sd_cur->time);
    bson_iter_init_find(&iter, doc, "fname");
    const char *strs = bson_iter_utf8(&iter, NULL);
    sd_cur->recv_fname = (char *) malloc(50 * sizeof(char));
    strcpy(sd_cur->recv_fname, strs);
    printf("recv fname:%s\n", sd_cur->recv_fname);

//    if(!map_get(&seed_map, sd_cur->recv_fname)) //TODO:
    bson_iter_init_find(&iter, doc, "len");
    sd_cur->len_bin = bson_iter_int32(&iter);
    const uint8_t *file_binary = NULL;
    sd_cur->seed_bin = (uint8_t *) malloc(sd_cur->len_bin);
    if (bson_iter_init_find(&iter, doc, "seed")) {
        bson_iter_binary(&iter, &subtype, &binary_len, &file_binary);
        memcpy(sd_cur->seed_bin, file_binary, binary_len);


    }
    bson_iter_init_find(&iter, doc, "depth");
    sd_cur->depth = bson_iter_int32(&iter);
//    cout << "sd_cur->depth: " << sd_cur->depth<<endl;
    bson_iter_init_find(&iter, doc, "handicap");
    sd_cur->handicap = bson_iter_int32(&iter);

    bson_iter_init_find(&iter, doc, "was_fuzzed");
    sd_cur->was_fuzzed = bson_iter_int32(&iter);

    bson_iter_init_find(&iter, doc, "passed_det");
    sd_cur->passed_det = bson_iter_int32(&iter);
    return sd_cur;
//    return nullptr;

}

//reset seed iterator, get check==0 , to push in the queue.
bool MongoDataEngine::get_check_to_queue(bson_t *query){
    if(query == nullptr){
        int32_t checked = 0;
        query = BCON_NEW("checked", BCON_INT32(checked)); //Filter for tags
    }
    mongoc_client_t *client_pool;
    client_pool = mongoc_client_pool_pop (pool);
    mongoc_collection_t *collection;
    collection = mongoc_client_get_collection(client_pool, target_name.c_str(), "seed");
    printf("debug reset_cursor_seed\n");
    mongoc_cursor_t *cursor_seed;
    timespec timec1, timec2;
    clock_gettime(CLOCK_REALTIME, &timec1);
    cursor_seed = mongoc_collection_find_with_opts(collection, query, NULL, NULL);
    clock_gettime(CLOCK_REALTIME, &timec2);
    // show_timespec(timec1, timec2, "mongoc_collection_find_with_opts");
    int debug_i = 0;
    while (true) {
        timespec timeds1, timeds2;
        clock_gettime(CLOCK_REALTIME, &timeds1);
        const bson_t *doc;
        bson_iter_t iter;
        bson_error_t error;
        bson_subtype_t subtype = BSON_SUBTYPE_BINARY;
        uint32_t binary_len = 0; //种子长度size
        c_seed_t *sd_cur = (c_seed_t *) malloc(sizeof(c_seed_t));
        if (mongoc_cursor_next(cursor_seed, &doc) == false) {
            //The iterative cursor encountered an error.
            if (mongoc_cursor_error(cursor_seed, &error)) {
                fprintf(stderr, "Failed to iterate all documents: %s\n", error.message);
            }
            sd_cur = nullptr;
            bson_destroy(query);
            mongoc_client_pool_push (pool, client_pool);
            return true;
        }
//        print_bson(doc);
        bson_iter_init_find(&iter, doc, "time");
        sd_cur->time = bson_iter_int64(&iter);
        //    printf("\nsd_cur->time: %lld\n", sd_cur->time);
        bson_iter_init_find(&iter, doc, "fname");
        const char *strs = bson_iter_utf8(&iter, NULL);
        sd_cur->recv_fname = (char *) malloc(50 * sizeof(char));
        strcpy(sd_cur->recv_fname, strs);
//        printf("recv fname:%s\n", sd_cur->recv_fname);

        //    if(!map_get(&seed_map, sd_cur->recv_fname)) //TODO:
        bson_iter_init_find(&iter, doc, "len");
        sd_cur->len_bin = bson_iter_int32(&iter);
        const uint8_t *file_binary = NULL;
        sd_cur->seed_bin = (uint8_t *) malloc(sd_cur->len_bin);
        if (bson_iter_init_find(&iter, doc, "seed")) {
            bson_iter_binary(&iter, &subtype, &binary_len, &file_binary);
            memcpy(sd_cur->seed_bin, file_binary, binary_len);
        }
        bson_iter_init_find(&iter, doc, "depth");
        sd_cur->depth = bson_iter_int32(&iter);
        //    cout << "sd_cur->depth: " << sd_cur->depth<<endl;
        bson_iter_init_find(&iter, doc, "handicap");
        sd_cur->handicap = bson_iter_int32(&iter);

        bson_iter_init_find(&iter, doc, "was_fuzzed");
        sd_cur->was_fuzzed = bson_iter_int32(&iter);

        bson_iter_init_find(&iter, doc, "passed_det");
        sd_cur->passed_det = bson_iter_int32(&iter);

//        printf("get_seeds_to_queue: fname: %s\n", sd_cur->recv_fname);
        clock_gettime(CLOCK_REALTIME, &timeds2);
        // printf("get_seeds_to_queue: fname: %s\n", sd_cur->recv_fname);
        // if(debug_i == 0 ){
        //     show_timespec(timeds1, timeds2, "download seed first");
        //     debug_i ++;
        // }else{
        //     show_timespec(timeds1, timeds2, "download seed ");
        // }
        timespec timeque1, timeque2;
        clock_gettime(CLOCK_REALTIME, &timeque1);
        produce_se_task(sd_cur);  //Add to queue
        clock_gettime(CLOCK_REALTIME, &timeque2);
        // show_timespec(timeque1, timeque2, "push queue ");

    }


}

//Get fname according to mongo cursor
char *MongoDataEngine::get_next_fname(mongoc_cursor_t *cursor) {
//    assert(false);
    /*  If the parameter is empty, use the member variable  */
    if (cursor == nullptr) {
        cursor = this->cursor_seed;
    }
    const bson_t *doc;
    bson_iter_t iter;
    bson_error_t error;
    bson_subtype_t subtype = BSON_SUBTYPE_BINARY;
    uint32_t binary_len = 0; //种子长度size
//    c_seed_t *sd_cur = (c_seed_t*)malloc(sizeof (c_seed_t));
    char *fname = nullptr;
    if (mongoc_cursor_next(cursor, &doc) == false) {
        //The iterative cursor encountered an error.
        if (mongoc_cursor_error(cursor, &error)) {
            fprintf(stderr, "Failed to iterate all documents: %s\n", error.message);
        }
        return fname;
    }
//    bson_iter_init_find(&iter, doc, "time");
//    unsigned long long time = bson_iter_int64(&iter);
    bson_iter_init_find(&iter, doc, "fname");
    const char *strs = bson_iter_utf8(&iter, NULL);
    fname = (char *) malloc(50 * sizeof(char));
    strcpy(fname, strs);
    printf("recv fname:%s\n", fname);
    return fname;
}

//Get specified seed information, including passed_det, was_fuzzed, depth, and more
c_seed_t *MongoDataEngine::get_seed(std::string seed_fname) {
//    std::cout << "get_seed() " << std::endl;
//    printf("get_seed_c:\n seed_name: %s\n",seed_fname.c_str());
    bson_t *query;
    mongoc_collection_t *collection;

    query = BCON_NEW("fname", BCON_UTF8(seed_fname.c_str()));
    collection = mongoc_client_get_collection(this->client, this->target_name.c_str(), "seed");
    mongoc_cursor_t *cursor_tmp = mongoc_collection_find_with_opts(collection, query, NULL, NULL);
    c_seed_t *sd_cur = (c_seed_t*)malloc(sizeof(c_seed_t));
//    sd_cur = get_next_seed(cursor_tmp);
    const bson_t *doc;
    bson_iter_t iter;
    bson_error_t error;
    bson_subtype_t subtype = BSON_SUBTYPE_BINARY;
    uint32_t binary_len = 0; //Seed lengthize
    if (mongoc_cursor_next(cursor_tmp, &doc) == false) {
        //迭代游标遇到错误.
        if (mongoc_cursor_error(cursor_tmp, &error)) {
            printf("error.message: %s\n",error.message);
            fprintf(stderr, "Failed to iterate all documents: %s\n", error.message);
        }
        sd_cur = nullptr;
        mongoc_cursor_destroy(cursor_tmp);
        bson_destroy(query);
        return sd_cur;
    }
    bson_iter_init_find(&iter, doc, "time");
    sd_cur->time = bson_iter_int64(&iter);
//    printf("sd_cur->time: %lld\n", sd_cur->time);
    bson_iter_init_find(&iter, doc, "fname");
    const char *strs = bson_iter_utf8(&iter, NULL);
    sd_cur->recv_fname = (char *) malloc(50 * sizeof(char));
    strcpy(sd_cur->recv_fname, strs);
    bson_iter_init_find(&iter, doc, "len");
    sd_cur->len_bin = bson_iter_int32(&iter);
    const uint8_t *file_binary = NULL;
    sd_cur->seed_bin = (uint8_t *) malloc(sd_cur->len_bin);
    if (bson_iter_init_find(&iter, doc, "seed")) {
        bson_iter_binary(&iter, &subtype, &binary_len, &file_binary);
        memcpy(sd_cur->seed_bin, file_binary, binary_len);
    }
    bson_iter_init_find(&iter, doc, "depth");
    sd_cur->depth = bson_iter_int32(&iter);
//    std::cout << "sd_cur->depth: 1: " << sd_cur->depth<<std::endl;

    bson_iter_init_find(&iter, doc, "handicap");
    sd_cur->handicap = bson_iter_int32(&iter);

    bson_iter_init_find(&iter, doc, "was_fuzzed");
    sd_cur->was_fuzzed = bson_iter_int32(&iter);

    bson_iter_init_find(&iter, doc, "passed_det");
    sd_cur->passed_det = bson_iter_int32(&iter);

    mongoc_cursor_destroy(cursor_tmp);
    bson_destroy(query);
//    std::cout << "get_seed: " << std::endl;
//    printf("get_seed: recv fname:%s\n\n", sd_cur->recv_fname);
    return sd_cur;
}

c_seed_t *MongoDataEngine::get_seed_without_binary(std::string seed_fname) {
//    std::cout << "get_seed() " << std::endl;
//    printf("get_seed_c:\n seed_name: %s\n",seed_fname.c_str());
    bson_t *query;
    mongoc_collection_t *collection;

    query = BCON_NEW("fname", BCON_UTF8(seed_fname.c_str()));
    collection = mongoc_client_get_collection(this->client, this->target_name.c_str(), "seed");
    mongoc_cursor_t *cursor_tmp = mongoc_collection_find_with_opts(collection, query, NULL, NULL);
    c_seed_t *sd_cur = (c_seed_t*)malloc(sizeof(c_seed_t));
//    sd_cur = get_next_seed(cursor_tmp);
    const bson_t *doc;
    bson_iter_t iter;
    bson_error_t error;

    if (mongoc_cursor_next(cursor_tmp, &doc) == false) {
        //迭代游标遇到错误.
        if (mongoc_cursor_error(cursor_tmp, &error)) {
            printf("error.message: %s\n",error.message);
            fprintf(stderr, "Failed to iterate all documents: %s\n", error.message);
        }
        sd_cur = nullptr;
        mongoc_cursor_destroy(cursor_tmp);
        bson_destroy(query);
        return sd_cur;
    }
    bson_iter_init_find(&iter, doc, "time");
    sd_cur->time = bson_iter_int64(&iter);
//    printf("sd_cur->time: %lld\n", sd_cur->time);
    bson_iter_init_find(&iter, doc, "fname");
    const char *strs = bson_iter_utf8(&iter, NULL);
    sd_cur->recv_fname = (char *) malloc(50 * sizeof(char));
    strcpy(sd_cur->recv_fname, strs);
    bson_iter_init_find(&iter, doc, "len");
    sd_cur->len_bin = bson_iter_int32(&iter);

    bson_iter_init_find(&iter, doc, "depth");
    sd_cur->depth = bson_iter_int32(&iter);
//    std::cout << "sd_cur->depth: 1: " << sd_cur->depth<<std::endl;

    bson_iter_init_find(&iter, doc, "handicap");
    sd_cur->handicap = bson_iter_int32(&iter);

    bson_iter_init_find(&iter, doc, "was_fuzzed");
    sd_cur->was_fuzzed = bson_iter_int32(&iter);

    bson_iter_init_find(&iter, doc, "passed_det");
    sd_cur->passed_det = bson_iter_int32(&iter);

    mongoc_cursor_destroy(cursor_tmp);
    bson_destroy(query);
//    std::cout << "get_seed: " << std::endl;
//    printf("get_seed: recv fname:%s\n\n", sd_cur->recv_fname);
    return sd_cur;
}

c_seed_t *MongoDataEngine::get_check_seed(std::string seed_fname) {
//    std::cout << "get_seed() " << std::endl;
    printf("get_seed_c:\n seed_name: %s\n",seed_fname.c_str());
    bson_t *query;
    mongoc_collection_t *collection;

    query = BCON_NEW("fname", BCON_UTF8(seed_fname.c_str()), "checked", BCON_INT32(1));
    collection = mongoc_client_get_collection(this->client, this->target_name.c_str(), "seed");
    mongoc_cursor_t *cursor_tmp = mongoc_collection_find_with_opts(collection, query, NULL, NULL);
    c_seed_t *sd_cur = (c_seed_t*)malloc(sizeof(c_seed_t));
//    sd_cur = get_next_seed(cursor_tmp);
    const bson_t *doc;
    bson_iter_t iter;
    bson_error_t error;
    bson_subtype_t subtype = BSON_SUBTYPE_BINARY;
    uint32_t binary_len = 0; //Seed lengthize
    if (mongoc_cursor_next(cursor_tmp, &doc) == false) {
        //迭代游标遇到错误.
        if (mongoc_cursor_error(cursor_tmp, &error)) {
            printf("error.message: %s\n",error.message);
            fprintf(stderr, "Failed to iterate all documents: %s\n", error.message);
        }
        sd_cur = nullptr;
        mongoc_cursor_destroy(cursor_tmp);
        bson_destroy(query);
        return sd_cur;
    }
    bson_iter_init_find(&iter, doc, "time");
    sd_cur->time = bson_iter_int64(&iter);
//    printf("sd_cur->time: %lld\n", sd_cur->time);
    bson_iter_init_find(&iter, doc, "fname");
    const char *strs = bson_iter_utf8(&iter, NULL);
    sd_cur->recv_fname = (char *) malloc(50 * sizeof(char));
    strcpy(sd_cur->recv_fname, strs);
    bson_iter_init_find(&iter, doc, "len");
    sd_cur->len_bin = bson_iter_int32(&iter);
    const uint8_t *file_binary = NULL;
    sd_cur->seed_bin = (uint8_t *) malloc(sd_cur->len_bin);
    if (bson_iter_init_find(&iter, doc, "seed")) {
        bson_iter_binary(&iter, &subtype, &binary_len, &file_binary);
        memcpy(sd_cur->seed_bin, file_binary, binary_len);
    }
    bson_iter_init_find(&iter, doc, "depth");
    sd_cur->depth = bson_iter_int32(&iter);
//    std::cout << "sd_cur->depth: 1: " << sd_cur->depth<<std::endl;

    bson_iter_init_find(&iter, doc, "handicap");
    sd_cur->handicap = bson_iter_int32(&iter);

    bson_iter_init_find(&iter, doc, "was_fuzzed");
    sd_cur->was_fuzzed = bson_iter_int32(&iter);

    bson_iter_init_find(&iter, doc, "passed_det");
    sd_cur->passed_det = bson_iter_int32(&iter);

    mongoc_cursor_destroy(cursor_tmp);
    bson_destroy(query);
//    std::cout << "get_seed: " << std::endl;
//    printf("get_seed: recv fname:%s\n\n", sd_cur->recv_fname);
    return sd_cur;
}

//Get binary seeds.
Binary* MongoDataEngine::load_target(){

    const bson_t *doc;
    bson_iter_t iter;
    bson_error_t error;
    bson_subtype_t subtype = BSON_SUBTYPE_BINARY;
    uint32_t binary_len = 0; //Seed lengthize

    mongoc_collection_t *collection;
    bson_t *query = bson_new();
    collection = mongoc_client_get_collection(client, target_name.c_str(), "binary");
    mongoc_cursor_t *cursor_binary;
    cursor_binary = mongoc_collection_find_with_opts(collection, query, NULL, NULL);

    struct Binary *binary;
    binary = (struct Binary *) malloc(sizeof(struct Binary));

    if (mongoc_cursor_next(cursor_binary, &doc) == false) {
        //迭代游标遇到错误.
        if (mongoc_cursor_error(cursor_binary, &error)) {
            fprintf(stderr, "Failed to iterate all documents: %s\n", error.message);
        }
        return nullptr;
    }
    bson_iter_init_find(&iter, doc, "flag");
    int flag = bson_iter_int32(&iter);
    binary->flag = flag;
    const uint8_t *bin;
    if (flag == 1) {
        bson_iter_init_find(&iter, doc, "length");
        binary->length = bson_iter_int32(&iter);

        bson_iter_init_find(&iter, doc, "code");
        bson_iter_binary(&iter, &subtype, &binary_len, &bin);
        binary->seed_bin = (uint8_t *) bin;
        binary->bin_len = binary_len;
        printf("binary->bin_len: %d\n", binary->bin_len);
    } else {
        bson_iter_init_find(&iter, doc, "code");
        bson_iter_binary(&iter, &subtype, &binary_len, &bin);
        binary->seed_bin = (uint8_t *) bin;
        binary->bin_len = binary_len;
        printf("binary->bin_len: %d\n", binary->bin_len);
    }
    return binary;
}

//
int MongoDataEngine::update_seed_check_data_engine(char *qtopfname){
    bson_t *doc_selector = BCON_NEW("checked", BCON_INT32(0), "fname", BCON_UTF8(qtopfname));
    bson_t *update = BCON_NEW("$set", "{", "checked", BCON_INT32(1), "}");
    int ret = this->update_information(doc_selector, update, "seed");
    bson_destroy(update);
    bson_destroy(doc_selector);
    return ret;
}

int MongoDataEngine::delete_seed_check_fname_data_engine(int check, char *fname){
    bson_t*  doc_selector = bson_new();
    BSON_APPEND_INT32(doc_selector,"checked",check);
    BSON_APPEND_UTF8 (doc_selector, "fname", fname);
    MongoDataEngine *data = MongoDataEngine::get_instance();
    int ret = data->delete_information(doc_selector, "seed");
    bson_destroy(doc_selector);
    return ret;
}

unsigned long long MongoDataEngine::get_queue_info_cycle_number(){
    mongoc_collection_t *queue_info_collection;
    bson_t *query;
    bson_iter_t iter;
    const bson_t *doc_queue;
    mongoc_cursor_t *cursor;
    unsigned long long queue_cycle = 0;
    queue_info_collection = mongoc_client_get_collection (client,target_name.c_str(),  "queue_info");
    query = BCON_NEW("flag", BCON_INT32(1));
    cursor = mongoc_collection_find_with_opts (queue_info_collection, query, NULL, NULL);
    if (mongoc_cursor_next (cursor, &doc_queue))
    {
        if (bson_iter_init_find(&iter, doc_queue, "queue_cycle")) {
            queue_cycle = bson_iter_int64(&iter);
        }
    }

    mongoc_cursor_destroy (cursor);
    bson_destroy(query);
    mongoc_collection_destroy (queue_info_collection);
    return queue_cycle;
}

/*Update seed information, not tested */
int MongoDataEngine::update_seed(std::string seed_name, c_seed_t *seed){
    bson_t *doc_selector = BCON_NEW("checked", BCON_INT32(0), "fname", BCON_UTF8(seed_name.c_str()));
    bson_t *update = BCON_NEW("$set", "{",
                              "checked", BCON_INT32(1),
                              "time", BCON_INT64(seed->time),
                              "depth", BCON_INT32(seed->depth),
                              "handicap", BCON_INT32(seed->handicap),
                              "was_fuzzed", BCON_INT32(seed->was_fuzzed),
                              "passed_det", BCON_INT32(seed->passed_det),
                              "passed_det", BCON_INT32(seed->passed_det),
                              "}");

    mongoc_collection_t *collection;
    collection = mongoc_client_get_collection(this->client, this->target_name.c_str(), "seed");
    bson_error_t error;
    bool ret = mongoc_collection_update_one(collection, doc_selector, update, NULL, NULL, &error);
    bson_destroy(update);
    bson_destroy(doc_selector);
    if (!ret) {
        fprintf(stderr, "update_checked:%s\n", error.message);
        return -1;
    }
    return 0;
}

//Remove the seed
int MongoDataEngine::delete_seed(std::string seed_name){
    bson_t*  doc_selector = bson_new();
    BSON_APPEND_UTF8 (doc_selector, "fname", seed_name.c_str());
    MongoDataEngine *data = MongoDataEngine::get_instance();
    int ret = data->delete_information(doc_selector, "seed");
    bson_destroy(doc_selector);
    return ret;
}


/* To be optimized, use the value of the seed structure update database */
int MongoDataEngine::update_information(bson_t *doc_selector, bson_t *update, std::string collection_name) {
    mongoc_collection_t *collection;
    mongoc_client_t *client_pool;
    client_pool = mongoc_client_pool_pop (pool);
    collection = mongoc_client_get_collection(client_pool, this->target_name.c_str(), collection_name.c_str());
    bson_error_t error;
    if (!mongoc_collection_update_one(collection, doc_selector, update, NULL, NULL, &error)) {
        fprintf(stderr, "update_checked:%s\n", error.message);
        mongoc_client_pool_push (pool, client_pool);
        return -1;
    }
    mongoc_client_pool_push (pool, client_pool);
    return 0;
}

int MongoDataEngine::delete_information(bson_t *doc_selector, std::string collection_name) {
    mongoc_collection_t *collection;
    mongoc_client_t *client_pool;
    client_pool = mongoc_client_pool_pop (pool);
    collection = mongoc_client_get_collection(client_pool, this->target_name.c_str(), collection_name.c_str());
    bson_error_t error;
    if (!mongoc_collection_remove(collection, MONGOC_REMOVE_SINGLE_REMOVE, doc_selector, NULL, &error)) {
        fprintf(stderr, "Delete failed: %s\n", error.message);
        mongoc_client_pool_push (pool, client_pool);
        return -1;
    }
    mongoc_client_pool_push (pool, client_pool);
    return 0;
}



int MongoDataEngine::read_all_seeds(std::list<c_seed_t *> &seed_list) {
    bson_t *query;
    int32_t checked = 1;
    query = bson_new();
    BSON_APPEND_INT32(query, "checked", checked);
    this->reset_seed_iterator(query);
    while (true) {
        c_seed_t *seed = this->get_next_seed(nullptr);
        if (seed == NULL) {
            break;
        }
        seed_list.push_back(seed);
    }

    printf("read_all_seeds: seed_list.size(): ", seed_list.size());
    return seed_list.size();
}

//The value of the was_fuzzed is 1 according to the sed's fname update
int MongoDataEngine::mark_seed_was_fuzzed(char *fname){
    bson_t *doc_selector;
    bson_t *update;
    int32_t flag=1;

    doc_selector = BCON_NEW("fname", BCON_UTF8(fname));
    update = BCON_NEW("$set", "{", "was_fuzzed", BCON_INT32(flag), "}");
    this->update_information(doc_selector, update, "seed");

    bson_destroy(update);
    bson_destroy (doc_selector);
    return 0;
}

//According to fname, update handicap.
int MongoDataEngine::update_seed_handicap(char *fname, int handicap){
    //TODO: handicap类型为int
    bson_error_t error;
    bson_t *doc_selector;
    bson_t *update;


    doc_selector = BCON_NEW("fname", BCON_UTF8(fname));
    update = BCON_NEW("$set", "{", "handicap", BCON_INT32(handicap), "}");
    this->update_information(doc_selector, update, "seed");

    bson_destroy(update);
    bson_destroy (doc_selector);
    return 0;
}

int MongoDataEngine::insert_queue_info( c_seed_t *seed, char *describe){
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
    MongoDataEngine *data = MongoDataEngine::get_instance();
    queue_info_collection = mongoc_client_get_collection (data->client, data->target_name.c_str(), "queue_info");
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
//    bson_destroy (doc_queue);
    mongoc_cursor_destroy (cursor);

    collection = mongoc_client_get_collection (data->client, data->target_name.c_str(), "seed");

    doc = bson_new ();
    BSON_APPEND_UTF8 (doc, "fname", seed->recv_fname);
    BSON_APPEND_BINARY(doc, "seed", BSON_SUBTYPE_BINARY, seed->seed_bin, seed->len_bin);
    BSON_APPEND_INT32 (doc, "havoc_time", havoc_time);
    BSON_APPEND_INT32(doc, "len", seed->len_bin);
    BSON_APPEND_INT32(doc, "seed_id", seed_id);
    BSON_APPEND_INT32(doc, "depth", seed->depth);
    BSON_APPEND_INT32(doc, "handicap", seed->handicap);
    BSON_APPEND_INT32(doc, "was_fuzzed", 0);
    BSON_APPEND_INT32(doc, "checked", 0);
    BSON_APPEND_INT32(doc, "passed_det", 0);
    if (describe){
        BSON_APPEND_UTF8(doc,"describe",describe);
    }
    else {
        BSON_APPEND_UTF8(doc, "describe", "original_seeds");
    }
    BSON_APPEND_INT64(doc,"time",seed->time);

    if (!mongoc_collection_insert_one (
            collection, doc, NULL, NULL, &error)) {
        fprintf (stderr, "%s\n", error.message);
    }

    bson_destroy (doc);
    mongoc_collection_destroy (collection);
    return 0;
}

int MongoDataEngine::insert_seed( c_seed_t *seed, char *type_name, char *describe){
    mongoc_collection_t *collection;
    bson_error_t error;
    bson_t *doc;
    int32_t havoc_time;
    havoc_time = 0;
    MongoDataEngine *data = MongoDataEngine::get_instance();
    collection = mongoc_client_get_collection (data->client, data->target_name.c_str(), type_name);
    printf("insert_seed fname: %s\n", seed->recv_fname);
    doc = bson_new ();
    BSON_APPEND_UTF8 (doc, "fname", seed->recv_fname);
    BSON_APPEND_BINARY(doc, "seed", BSON_SUBTYPE_BINARY, seed->seed_bin, seed->len_bin);
    BSON_APPEND_INT32 (doc, "havoc_time", havoc_time);
    BSON_APPEND_INT32(doc, "len", seed->len_bin);
    if (describe)
        BSON_APPEND_UTF8(doc,"describe",describe);
    else
        BSON_APPEND_UTF8(doc,"describe","original_seeds");
    BSON_APPEND_INT64(doc,"time",seed->time);

    if (!mongoc_collection_insert_one (
            collection, doc, NULL, NULL, &error)) {
        fprintf (stderr, "%s\n", error.message);
    }

    bson_destroy (doc);
    mongoc_collection_destroy (collection);
    return 0;
}

int MongoDataEngine::update_seed(char* fname, u8* seed, u64 len){
    mongoc_collection_t *collection;
    bson_error_t error;
    bson_t *doc;
    bson_t *update;
    u32 result=1;

    seed[len]='\0';
    MongoDataEngine *data = MongoDataEngine::get_instance();
    collection = mongoc_client_get_collection (data->client, data->target_name.c_str(), "seed");

    doc=bson_new();
    BSON_APPEND_UTF8(doc, "fname", fname);
    BSON_APPEND_INT64(doc, "len", len);
    BSON_APPEND_BINARY(doc, "seed", BSON_SUBTYPE_BINARY, seed, len);

    update=bson_new();

    if (!mongoc_collection_update_one (collection, doc, update, NULL, NULL, &error)){
        fprintf (stderr, "%s\n", error.message);
        result=0;
    }

    bson_destroy(doc);
    bson_destroy(update);
    mongoc_collection_destroy (collection);
    return result;
}

void MongoDataEngine::download_target_binary_data_engine(char *binary_name){
    printf("%s\n", "in load binary");
    printf("binary_name: %s\n", binary_name);
    bson_iter_t iter;
    int size = 0;

    this->reset_binary_iterator();
    FILE *fp = fopen(binary_name, "wb");

    struct Binary *binary;
    binary = get_next_binary();
    if (binary == nullptr) {
        printf("error: download binary is null\n");
        assert(false);
        return;
    }
    int flag = binary->flag;
    if (flag == 1) {
        size = binary->length;
        fwrite(binary->seed_bin, sizeof(uint8_t), binary->bin_len, fp);
        printf("fwrite binary->bin_len: %d\n", binary->bin_len);
    }
    //For oversized seeds stored in multiple rows, traversal merge saves are made at download time.
    if(size > 15000000){
        while(true){
            binary = get_next_binary();
            if (binary == nullptr) {
                break;
            }
            fwrite(binary->seed_bin, sizeof(uint8_t), binary->bin_len, fp);
            printf("fwrite binary->bin_len: %d\n", binary->bin_len);
        }
    }
    fclose(fp);
    chmod(binary_name, S_IRUSR | S_IWUSR | S_IXUSR);
    return;

}

//Get the handicap, return returnicap for the specified fname
int MongoDataEngine::find_seed_handicap(const char* seed_name){
    mongoc_collection_t *collection;
    bson_t *query;
    bson_iter_t iter;
    const bson_t *doc_queue;
    mongoc_cursor_t *cursor;
//    unsigned long long handicap = 1;
    int ret = 0;

    MongoDataEngine *data = MongoDataEngine::get_instance();
    collection = mongoc_client_get_collection (data->client, data->target_name.c_str(), "seed");

    query = BCON_NEW("checked", BCON_INT32(1), "fname", BCON_UTF8(seed_name));
    cursor = mongoc_collection_find_with_opts(collection, query, NULL, NULL);
    if (mongoc_cursor_next(cursor, &doc_queue)) {
        if(bson_iter_init_find(&iter, doc_queue, "handicap") == true ){
            ret = 1;
        }
    }

    mongoc_cursor_destroy(cursor);
    bson_destroy(query);
    mongoc_collection_destroy(collection);
    return ret;
}


//Just get seed binary information.
//The default size for storing files in mongo is 16M.
//Therefore, upload the file according to 15M cutting upload, divided into multiple lines of storage.
//Traverse and merge to save when downloading.
Binary *MongoDataEngine::get_next_binary() {
    const bson_t *doc;
    bson_iter_t iter;
    bson_error_t error;
    bson_subtype_t subtype = BSON_SUBTYPE_BINARY;
    uint32_t binary_len = 0;
    struct Binary *binary;
    binary = (struct Binary *) malloc(sizeof(struct Binary));

    if (mongoc_cursor_next(cursor_binary, &doc) == false) {
        //The iterative cursor encountered an error.
        if (mongoc_cursor_error(cursor_binary, &error)) {
            fprintf(stderr, "Failed to iterate all documents: %s\n", error.message);
        }
        return nullptr;
    }
    bson_iter_init_find(&iter, doc, "flag");
    int flag = bson_iter_int32(&iter);
    binary->flag = flag;
    const uint8_t *bin;
    if (flag == 1) {
        bson_iter_init_find(&iter, doc, "length");
        binary->length = bson_iter_int32(&iter);

        bson_iter_init_find(&iter, doc, "code");
        bson_iter_binary(&iter, &subtype, &binary_len, &bin);
        binary->seed_bin = (uint8_t *) bin;
        binary->bin_len = binary_len;
        printf("binary->bin_len: %d\n", binary->bin_len);
    } else {
        bson_iter_init_find(&iter, doc, "code");
        bson_iter_binary(&iter, &subtype, &binary_len, &bin);
        binary->seed_bin = (uint8_t *) bin;
        binary->bin_len = binary_len;
        printf("binary->bin_len: %d\n", binary->bin_len);
    }
    return binary;

}

int MongoDataEngine::upload_init_bitmap_to_data_engine(u8 *virgin_b) {
    mongoc_collection_t *collection;
    bson_error_t error;
    bson_t *doc_selector;
    bson_t *update;
    mongoc_cursor_t *cursor;
    const bson_t *doc;
    int mp_size = 65536;

    MongoDataEngine *data = MongoDataEngine::get_instance();
    collection = mongoc_client_get_collection(data->client, data->target_name.c_str(), "virgin_bits");

    doc_selector = BCON_NEW("flag", BCON_INT32(0));
    cursor = mongoc_collection_find_with_opts(collection, doc_selector, NULL, NULL);
    update = bson_new();
    if (mongoc_cursor_next(cursor, &doc)) {
        // BSON_APPEND_BINARY(update, "bitmap", BSON_SUBTYPE_BINARY, virgin_b, MAP_SIZE);
        // BSON_APPEND_INT32(update, "flag", 1);
        update = BCON_NEW("$set", "{", "bitmap", BCON_BIN(BSON_SUBTYPE_BINARY, virgin_b, mp_size), "flag",
                          BCON_INT32(1), "}");
        if (!mongoc_collection_update_one(collection, doc_selector, update, NULL, NULL, &error)) {
            fprintf(stderr, "update_bitmap:%s\n", error.message);
        }
    }

    bson_destroy(update);
    bson_destroy(doc_selector);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(collection);
    return 0;
}


int MongoDataEngine::update_bitmap_to_data_engine(u8 *virgin_b) {
    mongoc_collection_t *collection;
    bson_error_t error;
    bson_t *doc_selector;
    bson_t *update;
    int mp_size = 65536;
    MongoDataEngine *data = MongoDataEngine::get_instance();
    mongoc_client_t *client_pool;
    client_pool = mongoc_client_pool_pop (pool);
    collection = mongoc_client_get_collection(client_pool, data->target_name.c_str(), "virgin_bits");

    doc_selector = BCON_NEW("flag", BCON_INT32(1));
    // update=bson_new();
    // BSON_APPEND_BINARY(update, "bitmap", BSON_SUBTYPE_BINARY, virgin_b, MAP_SIZE);
    update = BCON_NEW("$set", "{", "bitmap", BCON_BIN(BSON_SUBTYPE_BINARY, virgin_b, mp_size), "}");
    if (!mongoc_collection_update_one(collection, doc_selector, update, NULL, NULL, &error)) {
        fprintf(stderr, "update_bitmap:%s\n", error.message);
    }

    bson_destroy(update);
    bson_destroy(doc_selector);
    mongoc_collection_destroy(collection);
    mongoc_client_pool_push (pool, client_pool);
    return 0;
}

int MongoDataEngine::load_bitmap_from_data_engine(u8 *virgin_b) {
    mongoc_collection_t *collection;
    bson_iter_t iter;
    mongoc_cursor_t *cursor;
    bson_t *query;
    const bson_t *doc;
    bson_subtype_t subtype = BSON_SUBTYPE_BINARY;
    uint32_t binary_len = 0;
    const uint8_t *virgin;
    int mp_size = 65536;

    MongoDataEngine *data = MongoDataEngine::get_instance();
    collection = mongoc_client_get_collection(data->client, data->target_name.c_str(), "virgin_bits");

    query = bson_new();
    BSON_APPEND_INT32(query, "flag", 1);

    cursor = mongoc_collection_find_with_opts(collection, query, NULL, NULL);
    if (mongoc_cursor_next(cursor, &doc)) {
        if (bson_iter_init_find(&iter, doc, "bitmap")) {
            bson_iter_binary(&iter, &subtype, &binary_len, &virgin);
            memcpy(virgin_b, virgin, mp_size);
        }
    }

    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(collection);
    return 0;
}

void MongoDataEngine::update_cycle_number(u64 queue_cycle) {
    mongoc_collection_t *queue_info_collection;
    bson_t *doc_selector;
    bson_t *update;
    bson_error_t error;
    int32_t flag = 1;

    MongoDataEngine *data = MongoDataEngine::get_instance();
    queue_info_collection = mongoc_client_get_collection(data->client, data->target_name.c_str(), "queue_info");
    doc_selector = BCON_NEW("flag", BCON_INT32(flag));
    update = BCON_NEW("$set", "{", "queue_cycle", BCON_INT64(queue_cycle), "}");
    if (!mongoc_collection_update_one(queue_info_collection, doc_selector, update, NULL, NULL, &error)) {
        fprintf(stderr, "update_cycle_number:%s\n", error.message);
    }
    bson_destroy(update);
    bson_destroy(doc_selector);
    mongoc_collection_destroy(queue_info_collection);

}

void MongoDataEngine::update_bitmap_cvg(double bitmap_cvg) {
    mongoc_collection_t *queue_info_collection;
    bson_t *doc_selector;
    bson_t *update;
    bson_error_t error;

    bson_t *query;
    bson_iter_t iter;
    const bson_t *doc_queue;
    mongoc_cursor_t *cursor;

    int32_t flag = 1;
    double coverage_temp = 0.00;

    MongoDataEngine *data = MongoDataEngine::get_instance();
    queue_info_collection = mongoc_client_get_collection(data->client, data->target_name.c_str(), "queue_info");
    query = BCON_NEW("flag", BCON_INT32(1));
    cursor = mongoc_collection_find_with_opts(queue_info_collection, query, NULL, NULL);
    if (mongoc_cursor_next(cursor, &doc_queue)) {
        if (bson_iter_init_find(&iter, doc_queue, "bitmap_coverage")) {
            coverage_temp = bson_iter_double(&iter);
        }
    }

    if (bitmap_cvg > coverage_temp) {
        doc_selector = BCON_NEW("flag", BCON_INT32(flag));
        update = BCON_NEW("$set", "{", "bitmap_coverage", BCON_DOUBLE(bitmap_cvg), "}");
        if (!mongoc_collection_update_one(queue_info_collection, doc_selector, update, NULL, NULL, &error)) {
            fprintf(stderr, "update_bitmap_coverage:%s\n", error.message);
        }

        bson_destroy(update);
        bson_destroy(doc_selector);
    }

    mongoc_cursor_destroy(cursor);
    bson_destroy(query);
    mongoc_collection_destroy(queue_info_collection);

}


//Binary* MongoDataEngine::get_target_binary() {
//    if(target_binary == nullptr) {
//        target_binary = load_target();
//    }
//    return target_binary;
//}


/* end ------  MongoDataEngine Member functions --------- */



/*
* Print the document as a JSON string.
*/
void print_bson(const bson_t *document) {
    /*
    * Print the document as a JSON string.
    */
    char *str;
    str = bson_as_canonical_extended_json(document, NULL);
    printf("%s\n\n", str);
    bson_free(str);
}



/* Download the to-be-fuzzed binary */
void download_target_binary_data_engine_c(char *binary_name){
    MongoDataEngine *data = MongoDataEngine::get_instance();
    data->download_target_binary_data_engine(binary_name);
}

/* Get seed from mongodb according to the fname  */
u8 *get_seed_c(const char* fname){
    printf("get_seed_c: fname: %s\n",fname);
    MongoDataEngine *data = MongoDataEngine::get_instance();
    assert(data != nullptr);
    c_seed_t *sd_cur = data->get_seed(fname);
    if(sd_cur == NULL){
        return NULL;
    }
    return sd_cur->seed_bin;
}

c_seed_t* get_seed_info(const char* fname){
    MongoDataEngine *data = MongoDataEngine::get_instance();
    assert(data != nullptr);
    c_seed_t *sd_cur = data->get_seed(fname);
    return sd_cur;
}

c_seed_t* get_seed_info_without_binary(const char* fname){
    MongoDataEngine *data = MongoDataEngine::get_instance();
    assert(data != nullptr);
    c_seed_t *sd_cur = data->get_seed_without_binary(fname);
    return sd_cur;
}

//Get checked seeds.
c_seed_t* get_check_seed_info(const char* fname){
    MongoDataEngine *data = MongoDataEngine::get_instance();
    assert(data != nullptr);
    c_seed_t *sd_cur = data->get_check_seed(fname);
    return sd_cur;
}

int mark_seed_passed_det_c(char *fname){
    MongoDataEngine *data = MongoDataEngine::get_instance();
    return data->mark_seed_passed_det(fname);;
}

int find_seed_handicap_c(const char* seed_name){
    MongoDataEngine *data = MongoDataEngine::get_instance();
    return data->find_seed_handicap(seed_name);
}


//Encapsulating reset_seed_interator C++ functions
// checked = -1 means query all.
int reset_seed_iterator_c(int checked, u64 seed_time) {
    // bson_t *query = bson_new();
    // if (checked != -1) {
    //     BSON_APPEND_INT32(query, "checked", checked);
    // }
    bson_t *query;
    if (seed_time == 0){
        query = bson_new();
        BSON_APPEND_INT32(query, "checked", checked);
    }else{
        query = BCON_NEW("time", "{", "$gte", BCON_INT64(seed_time), "}", "checked", BCON_INT32(checked));
    }
    MongoDataEngine *data_engine = get_data_engine();
    assert(data_engine != nullptr);
    data_engine->reset_seed_iterator( query);
    return 0;
}

//Encapsulating get_next_seed C++ functions
c_seed_t *get_next_seed_c() {
    MongoDataEngine *data_engine = get_data_engine();
    assert(data_engine != nullptr);
    c_seed_t *sd_cur = data_engine->get_next_seed(nullptr);
    return sd_cur;
}

char *get_next_seed_fname_c() {
    MongoDataEngine *data_engine = get_data_engine();
    assert(data_engine != nullptr);
    char *fname = data_engine->get_next_fname(nullptr);
    return fname;
}






