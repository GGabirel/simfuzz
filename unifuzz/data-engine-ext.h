//
// Created by hunter on 2021/4/13.
//

#ifndef UNIFUZZPROJECT_DATA_ENGINE_EXT_H
#define UNIFUZZPROJECT_DATA_ENGINE_EXT_H

#include "unifuzz.h"
#include "alloc-inl.h"
#include "types.h"

#include "map.h"
#include "hash.h"
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif


extern int init_data_engine_c(char *db, char *uri_string, char *target_name, char *version); //The initialization of mongodb uses a single -case pattern.
extern int destroy_data_engine_c(); //The destruction of mongodb.
extern c_seed_t *read_all_seeds_c();

extern int test_data_engine_command_simple(); //Test the database using mongodb's command_simple


extern int update_seed_check_data_engine_c(char *qtopfname);//Update the mongo seed check information
extern int delete_seed_check_fname_data_engine_c(int check, char *fname );


extern int mark_seed_passed_det_c(char *fname); //The value of the passed_det updated according to fname is 1
extern int mark_seed_was_fuzzed_c(char *fname); //The value of the was_fuzzed updated according to fname is 1
extern unsigned long long get_queue_info_cycle_number_c(); //Get the queue_cycle value of the queue_info table.
extern int update_seed_handicap_c(char *fname, int handicap); //According to fname, update handicap.


extern int insert_queue_info_c( c_seed_t *seed, char *describe);
/* When finding a new seed, insert it in the mongodb */
extern int insert_seed_c( c_seed_t *seed, char *type_name, char *describe);

/* If needed, update the seed content in the mongodb */
extern int update_seed_c(char* fname, u8* seed, u64 len);

extern void download_target_binary_data_engine_c(char *binary_name);

extern int find_seed_handicap_c(const char* seed_name);  //Look for the information specified in the information fname, returning the handicap value.

extern int upload_init_bitmap_to_data_engine_c( u8 *virgin_b);
extern int update_bitmap_to_data_engine_c(u8 *virgin_b);
extern int load_bitmap_from_data_engine_c(u8 *virgin_b);

extern void update_cycle_number_c(u64 queue_cycle);
extern void update_bitmap_cvg_c(double bitmap_cvg);

u8 *get_seed_c(const char* fname);

c_seed_t* get_seed_info(const char* fname);
c_seed_t* get_seed_info_without_binary(const char* fname);
c_seed_t* get_check_seed_info(const char* fname);


int reset_seed_iterator_c(int checked, u64 seed_time);
c_seed_t *get_next_seed_c(); //Gets information in the seed table, including seed binary, c interfaces
char *get_next_seed_fname_c(); //Get fname according to mongo cursor

struct timespec show_timespec(struct timespec start, struct timespec end, char* phrase);
void my_gettime(struct timespec *t);
#ifdef __cplusplus
}
#endif


#endif //UNIFUZZPROJECT_DATA_ENGINE_EXT_H
