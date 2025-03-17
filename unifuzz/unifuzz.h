#ifndef UNIFUZZ_H
#define UNIFUZZ_H


#include <stdint.h>
#include <bson/bson.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "alloc-inl.h"

#define MAX_SEED_NAME_LEN 128

#ifdef __x86_64__
typedef unsigned long long u64;
#else
typedef uint64_t u64;
#endif

typedef struct c_seed_t_ {
    uint8_t *seed_bin;
    char *recv_fname;
    int len_bin;
    unsigned long long depth;
    unsigned long long handicap;
    uint8_t was_fuzzed;
    uint8_t passed_det;
    unsigned long long time;
    struct c_seed_t_ *next; // for building a list.
} c_seed_t;

typedef struct update_seed_t_ {
    char *recv_fname;
    int update; // Update or delete. If the update value is 1, else delete.
}update_seed_t;

#ifdef __cplusplus
#include <string>


class Seed {
public:
    bool evaluated = false;
    c_seed_t seed_info;
public:
    Seed() {}
    ~Seed() {if(seed_info.seed_bin != nullptr) delete seed_info.seed_bin;}
    void init();
    uint8_t* get_binary() { return seed_info.seed_bin; }
    bool update_int_value(std::string key, int value);
    bool is_checked(){ return evaluated; }
    c_seed_t * c_seed();
};



extern "C" {
#endif

//socket ---
#define RECV_MAX_SIZE 30
typedef struct unf_proto_t_ {
    u32 fuzzer_id;
    uint8_t version;
    uint8_t cmd;
    int bits_loc;
    int havoc_score;
    int splicing;
    u8 cur_bits[8];
    char data[RECV_MAX_SIZE];
} unf_proto_t;

enum {
    REQUEST_NEW_SEEDS = 1,
    REQUEST_UPDATE = 2,
    REPLY_SEEDS = 3,
    UPDATE_BITS = 4
};
// --- socket

struct Binary {
    int flag;
    int length; //The total size of the seed
    uint8_t *seed_bin;
    int bin_len; //The size of the seed_bin
    char *name;
};

typedef struct fuzz_task_t_ {
    char seed_name[MAX_SEED_NAME_LEN];
    int havoc_score;
    int splicing;
} fuzz_task_t;


c_seed_t *consume_se_task();
void produce_se_task(c_seed_t *t);

void produce_seed_update(char *fname, int update);
update_seed_t *consume_seed_update();

void produce_bitmap_update(u8 *global_vir);
u8 * consume_bitmap_update();

/*init interfaces*/
int init_unifuzz( char* db_name,  char* db_site, char* target_name, char* local_binary_name);

/*control functions*/
void master_main_loop(char *master_ip, int master_port);

/*energe assignment*/
int get_a_fuzz_task(fuzz_task_t* ft);
int get_a_similar_fuzz_task(fuzz_task_t* ft, u32 fid);

/*update_global_bitmap*/
void update_global_bits(u64* cur_bits, int location);


/*seed evaluation*/
void do_evaluate_seed(c_seed_t * seedEvalTask);

void check_and_evaluate(); //Get seed information from the seed table from the database and join the queue
void start_evaluate_thread();
void notify_seeds();
//void produce_se_task(c_seed_t *t); //TODO: struct SeedEvalTask
//c_seed_t *consume_se_task(); //Take values from the queue, get seed information in the seed table.


int init_client_socket(char *master_ip, int master_port);
int init_server_socket(char *master_ip, int master_port);
int request_fuzzing_task(char *master_ip, int master_port, fuzz_task_t* ft);
int handle_recv_data(unf_proto_t* recv_buf, unf_proto_t* reply);

#ifdef __cplusplus
}
#endif


#endif