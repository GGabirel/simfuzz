#include <bson/bson.h>
#include <mongoc/mongoc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "alloc-inl.h"
#include "data-engine-ext.h"
#define MAP_SIZE_POW2       16
#define MAP_SIZE            (1 << MAP_SIZE_POW2)

#define FF(_b)  (0xff << ((_b) << 3))

/* Get unix time in milliseconds */

static u64 get_cur_time(void) {

  struct timeval tv;
  struct timezone tz;

  gettimeofday(&tv, &tz);

  return (tv.tv_sec * 1000ULL) + (tv.tv_usec / 1000);

}

/* Count the number of non-255 bytes set in the bitmap. Used strictly for the
   status screen, several calls per second or so. */

static u32 count_non_255_bytes2(u8* mem) {
  u32* ptr = (u32*)mem;
  u32  i   = (MAP_SIZE >> 2);
  u32  ret = 0;
  while (i--) {
    u32 v = *(ptr++);
    if (v == 0xffffffff) continue;
    if ((v & FF(0)) != FF(0)) ret++;
    if ((v & FF(1)) != FF(1)) ret++;
    if ((v & FF(2)) != FF(2)) ret++;
    if ((v & FF(3)) != FF(3)) ret++;
  }
  return ret;
}
static u32 count_non_255_bytes3(const uint8_t* mem) {
  u32* ptr = (u32*)mem;
  u32  i   = (MAP_SIZE >> 2);
  u32  ret = 0;
  while (i--) {
    u32 v = *(ptr++);
    if (v == 0xffffffff) continue;
    if ((v & FF(0)) != FF(0)) ret++;
    if ((v & FF(1)) != FF(1)) ret++;
    if ((v & FF(2)) != FF(2)) ret++;
    if ((v & FF(3)) != FF(3)) ret++;
  }
  return ret;
}

mongoc_client_t* mongo_conn_init(char *url_site);
void mongo_conn_stop(mongoc_client_t *client);
void operate(u8*virgin_map, const uint8_t*new_bits);

void load_mongo_binary(mongoc_client_t *client, u8 *target_name, u8 *binary_name);
void insert_mongo_bitmap(mongoc_client_t *client, u8 *target_name, u8 *virgin_b);
u8 read_mongo_bitmap(mongoc_client_t *client, u8 *target_name, u8 *type_name,  u8 *virgin_b);
u8 update_mongo_seed(mongoc_client_t *client, u8 *target_name, u8* fname, u8* seed, u64 len);
void insert_mongo_seed(mongoc_client_t *client, u8 * target_name, u8 * type_name, u8 *const fname, u8 *const mem,  u32 len, u64 time, u8 *const describe);
void insert_mongo_seed_in_queue(mongoc_client_t *client, u8 * target_name, u8 *const fname, u8 *const mem,  u32 len, u32 depth, u32 handicap, u64 time, u8 *const describe);
u8 *read_mongo_seed(mongoc_client_t *client, u8 *target_name,  const u8* fname, u32 len);
void insert_tmout_bitmap(mongoc_client_t *client, u8 *target_name, u8 *virgin_b);
void insert_crash_bitmap(mongoc_client_t *client, u8 *target_name, u8 *virgin_b);
