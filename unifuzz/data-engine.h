#ifndef _DATA_ENGINE_H_
#define _DATA_ENGINE_H_

#include <bson/bson.h>
#include <mongoc/mongoc.h>
#include <map>

#include "data-engine-ext.h"

#define RETURN_TRUE 1
#define RETURN_FALSE 0



#include <string>
#include <list>
class Seed;
class DataEngine;

class MongoDataEngine{
public:
    std::string db_name;
    std::string db_ip_port;
    // int db_port;
    std::string target_name;
    std::string target_version;
//    map_int_t* seed_map;
public:
    mongoc_client_t *client = nullptr;
    static MongoDataEngine* data_engine;//Static member that holds a unique instance of the object
    mongoc_client_pool_t *pool; //Mongo thread pool.
private:
    mongoc_cursor_t *cursor_seed; //Save the cursor of a query. Seed table
    mongoc_cursor_t *cursor_binary; //Save the cursor of a query. Binary table
    Binary* target_binary = nullptr;
    Binary* load_target();

protected:
    int update_information(bson_t * doc_selector, bson_t * update,  std::string collection_name); /*Update the information*/
    int delete_information(bson_t*  doc_selector, std::string collection_name);  /*Delete*/

public:
    MongoDataEngine(std::string db, std::string ip_port, std::string target_name, std::string version) ;
    static MongoDataEngine* instance(std::string db, std::string ip_port,  std::string target_name, std::string version);
    static MongoDataEngine* get_instance();
    static void destroy();

    bool reset_seed_iterator(bson_t *query); /*Reset the cursor that traverses the seeds*/

    c_seed_t* get_next_seed(mongoc_cursor_t *cursor);
    char *get_next_fname(mongoc_cursor_t *cursor);
    c_seed_t* get_seed(std::string seed_fname);
    c_seed_t* get_seed_without_binary(std::string seed_fname);
    c_seed_t* get_check_seed(std::string seed_fname);
    int update_seed(std::string seed_name, c_seed_t *seed); /*Update the seed information*/
    int delete_seed(std::string seed_name); //Remove the seed
    int mark_seed_passed_det(char *fname);
    int update_seed_check_data_engine(char *qtopfname); //Update the seed information, update the value of the checked field to 1 based on the seed name
    int delete_seed_check_fname_data_engine(int check, char *fname); //Remove the seed, tagged according to the seed name and checked field
    unsigned long long get_queue_info_cycle_number(); //Get the queue_cycle value of the queue_info table.
    int read_all_seeds(std::list<c_seed_t*>& seed_list);

    bool reset_binary_iterator(); /*Reset the cursor that traverses the binary*/
    Binary* get_next_binary(); /*Get binary information*/
//    Binary* get_target_binary();

    int mark_seed_was_fuzzed(char *fname); //The value of the was_fuzzed updated according to fname is 1
    int update_seed_handicap(char *fname, int handicap); //According to fname, update handicap.
    int insert_queue_info( c_seed_t *seed, char *describe);
    int insert_seed( c_seed_t *seed, char *type_name, char *describe);
    int update_seed(char* fname, u8* seed, u64 len);
    void download_target_binary_data_engine(char *binary_name);

    int find_seed_handicap(const char* seed_name); //Look for the information specified in the information fname, returning the handicap value.

    int upload_init_bitmap_to_data_engine( u8 *virgin_b);
    int update_bitmap_to_data_engine(u8 *virgin_b);
    int load_bitmap_from_data_engine(u8 *virgin_b);

    void update_cycle_number(u64 queue_cycle);
    void update_bitmap_cvg(double bitmap_cvg);

    bool get_check_to_queue(bson_t *query);
};


void print_bson(const bson_t *document);
MongoDataEngine* init_data_engine(std::string db, std::string ip_port, std::string target_name, std::string version);
MongoDataEngine* get_data_engine();








#endif //_DATA_ENGINE_H_