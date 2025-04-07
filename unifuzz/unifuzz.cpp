#include <assert.h>
#include <thread>
#include "unifuzz.h"
#include "util.h"
#include "data-engine.h"

using namespace unifuzz;
std::condition_variable cv;
std::mutex cv_m; // This mutex is used for three purposes:
// 1) to synchronize accesses to i
// 2) to synchronize accesses to std::cerr
// 3) for the condition variable cv
#define BUF_SIZE 100




bool Seed::update_int_value(std::string key, int value) {
    assert(false); //Fixme: implement it.
    return true;
}

void Seed::init() {
    evaluated = false;
    seed_info.seed_bin = nullptr;
}

c_seed_t *Seed::c_seed() {
    return &seed_info;
}


/*The function of evaluating the seed is implemented in the master-node .c*/
void evaluate_seed() {
    // printf("evaluate_seed \n");
    c_seed_t *sd_tmp = consume_se_task();
    c_seed_t *sd_cur;
    sd_cur = (c_seed_t *)malloc(sizeof(c_seed_t));
    sd_cur->recv_fname = (char *)malloc(RECV_MAX_SIZE * sizeof(char));
    strcpy(sd_cur->recv_fname, sd_tmp->recv_fname);
    sd_cur->len_bin = sd_tmp->len_bin;
    sd_cur->depth = sd_tmp->depth;
    sd_cur->handicap = sd_tmp->handicap;
    sd_cur->was_fuzzed = sd_tmp->was_fuzzed;
    sd_cur->passed_det = sd_tmp->passed_det;
    sd_cur->time = sd_tmp->time;
    int len_tmp = sd_cur->len_bin;
    sd_cur->seed_bin = (uint8_t *)malloc(len_tmp);
    memcpy(sd_cur->seed_bin, sd_tmp->seed_bin, len_tmp);
    do_evaluate_seed(sd_cur);
    free(sd_cur->seed_bin);
    sd_cur->seed_bin = NULL;
    free(sd_cur->recv_fname);
    sd_cur->recv_fname = NULL;
    free(sd_cur);
    sd_cur = NULL;

    // also free sd_cur from queue
    free(sd_tmp->seed_bin);
    sd_tmp->seed_bin = NULL;
    free(sd_tmp->recv_fname);
    sd_tmp->recv_fname = NULL;
    free(sd_tmp);
    sd_tmp = NULL;
};


void notify_evaluate_finished() {
    /*
     * TODO: call any call back function in master-node.c to update data.
     * e.g., cull_queue();
    */
}


void wait_seeds() {
    std::unique_lock<std::mutex> lk(cv_m);
    std::cerr << "wait_seeds Waiting... \n";
    cv.wait(lk);
}

void notify_seeds() {
    std::unique_lock<std::mutex> lck(cv_m);
    std::cout << "notify_seeds go... \n";
    cv.notify_all();
}


BlockingQueue<c_seed_t *> se_tasks; // Download seeds and assessments
BlockingQueue<update_seed_t *> seed_update; //Update seed
BlockingQueue<u8 *> bitmap_update; //Update seed


/* Get seed information from the seed table from the database,
 * including seed binary, seed name, was_fuzzed, passed_det, etc.
 * and join the queue.
 * The function of evaluating the seed is implemented in the master-node .c */

void check_and_evaluate() {
    // printf("check_and_evaluate\n");
    MongoDataEngine *data_engine = get_data_engine();
    assert(data_engine != nullptr);
    bson_t *query;
    int32_t checked = 0;
    query = BCON_NEW("checked", BCON_INT32(checked)); //Filter for tags
//    query =  bson_new();
    data_engine->reset_seed_iterator(query);
    while (true) {
        c_seed_t *sd_cur = data_engine->get_next_seed(nullptr);

        if (sd_cur == nullptr) {
//            std::cout << "sd_cur->status: nullptr" << std::endl;
            break;
        }
        printf("check_and_evaluate: fname: %s\n", sd_cur->recv_fname);
        produce_se_task(sd_cur);  //Add to queue
//        evaluate_seed();

//		sd_cur->update_int_value("check", 1);
    }

    notify_evaluate_finished();
}

void get_seeds_to_queue(){
    // printf("get_seeds_to_queue\n");
    MongoDataEngine *data_engine = get_data_engine();
    assert(data_engine != nullptr);

    data_engine->get_check_to_queue(nullptr);

    notify_evaluate_finished();
}

/* The thread body to evaluate seeds */
[[noreturn]] void *seed_evaluate_thread() {
    printf("seed_evaluate_thread\n");
    while (true) {
        // printf("seed_evaluate_thread\n");
        evaluate_seed();
    }
}

[[noreturn]] void *seeds_thread(){
    printf("seeds_thread\n");
    while(true){
        // printf("seeds_thread\n");
        wait_seeds();
        get_seeds_to_queue();
    }
}

[[noreturn]] void *do_update_seed_thread(){
    printf("do_update_seed_thread\n");
    while(true){
        printf("do_update_seed_thread\n");
//        wait_seeds();
        update_seed_t *sd_cur = consume_seed_update();
        // Update or delete. If the update value is 1, else delete.
        if(sd_cur->update == 1){
            update_seed_check_data_engine_c(sd_cur->recv_fname);
        }else{
            int32_t checked = 0;
            delete_seed_check_fname_data_engine_c(checked, sd_cur->recv_fname);
        }
    }
}

[[noreturn]] void *do_update_bitmap(){
    printf("do_update_bitmap\n");
    while(true){
        u8 * global_vir = consume_bitmap_update();
        update_bitmap_to_data_engine_c(global_vir);
    }
}


/* C++ implementations */
std::thread &get_seeds_thread() {
    static std::thread t = std::thread(seeds_thread);
    return t;
}
std::thread &get_evaluate_thread() {
    static std::thread t = std::thread(seed_evaluate_thread);
    return t;
}
std::thread &update_seed_thread(){
    static std::thread t = std::thread(do_update_seed_thread);
    static std::thread t2 = std::thread(do_update_seed_thread);
    static std::thread t3 = std::thread(do_update_seed_thread);
    static std::thread t4 = std::thread(do_update_seed_thread);
    static std::thread t5 = std::thread(do_update_seed_thread);
    return t;
}

std::thread &update_bitmap_thread(){
    static std::thread t = std::thread(do_update_bitmap);
    return t;
}

////////////////////////////////////////////////////////Implementations in C /////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif


void produce_se_task(c_seed_t *sd_cur) {
    se_tasks.Push(sd_cur);
    printf("se_tasks_queue: size: %ld push()\n", se_tasks.Size());
}

c_seed_t *consume_se_task() {
    c_seed_t *sd_cur = se_tasks.Pop();
    printf("se_tasks_queue: size: %ld pop()\n", se_tasks.Size());
    return sd_cur;
}

void produce_seed_update(char *fname, int update) {
    update_seed_t *sd_cur;
    sd_cur = (update_seed_t *)malloc(sizeof(update_seed_t));
    sd_cur->recv_fname = (char *)malloc(RECV_MAX_SIZE * sizeof(char));
    strcpy(sd_cur->recv_fname, fname);
    // sd_cur->recv_fname = fname;
    sd_cur->update = update;
    seed_update.Push(sd_cur);
    printf("seed_update_queue: size: %ld push()\n", seed_update.Size());
}

update_seed_t *consume_seed_update() {
    update_seed_t *sd_cur = seed_update.Pop();
    printf("seed_update_queue: size: %ld pop()\n", seed_update.Size());
    return sd_cur;
}

void produce_bitmap_update(u8 *global_vir){
    bitmap_update.Push(global_vir);
    printf("bitmap_update_queue: size: %ld push()\n", bitmap_update.Size());
}

u8 * consume_bitmap_update(){
    u8 *global_vir = bitmap_update.Pop();
    printf("bitmap_update_queue: size: %ld pop()\n", bitmap_update.Size());
    return global_vir;
}

void start_evaluate_thread() {
    get_seeds_thread();
    get_evaluate_thread();
    update_seed_thread();
    update_bitmap_thread();  //update_bitmap global_vir
}

uint32_t num_fuzzers = 0;
u32 client_fuzzer_id = 0;
u32 curr_assigned_fuzzer_id = 1; // start from 1 for noise fuzzers

//#define REQUEST_UPDATE  "update"
int handle_recv_data(unf_proto_t* recv_buf, unf_proto_t* reply){
    int ret = 0;
    switch (recv_buf->cmd) {
        case REQUEST_NEW_SEEDS:
            /*request seed*/
            printf("REQUEST_NEW_SEEDS: \n");

            if (recv_buf->fuzzer_id == 0) // client uninitialized
                reply->fuzzer_id = curr_assigned_fuzzer_id++;
            else
                reply->fuzzer_id = recv_buf->fuzzer_id;
            
            assert(curr_assigned_fuzzer_id <= num_fuzzers + 1);

            fuzz_task_t ft;
            if(get_a_similar_fuzz_task(&ft, reply->fuzzer_id)){
                strncpy(reply->data, ft.seed_name, sizeof(reply->data));
                reply->splicing = ft.splicing;
                reply->havoc_score = ft.havoc_score;
                ret = sizeof(reply->data);
                printf("send similar seed ft.seed_name: %s to fuzzer %u\n", ft.seed_name, reply->fuzzer_id);     
            } else if(get_a_fuzz_task(&ft, reply->fuzzer_id)){
                strncpy(reply->data, ft.seed_name, sizeof(reply->data));
                reply->splicing = ft.splicing;
                reply->havoc_score = ft.havoc_score;
                ret = sizeof(reply->data);
                printf("send ft.seed_name: %s to fuzzer %u\n", ft.seed_name, reply->fuzzer_id);
            }

            break;

        case REQUEST_UPDATE:
            /*request evaluation*/
            printf("REQUEST_UPDATE\n");
            notify_seeds();
            ret = 0;
            break;

        case UPDATE_BITS:
            /*update global bits*/
            printf("UPDATE_BITS\n");
            update_global_bits( (u64*)recv_buf->cur_bits, recv_buf->bits_loc);
            ret = 0;
            break;

        default:
            fprintf(stderr, "Unkonwn requeset. ");
            ret = 0;
            break;
    }
    return ret;

}



//Initialize socket
int init_server_socket(char *master_ip, int master_port){
    /*init socket*/
    int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serv_sock == -1) {
        printf("create socket error\n");
        return -1;
    }
    printf("create socket ok,serv_sock=%d\n", serv_sock);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    printf("ip:%s\n", master_ip);
    serv_addr.sin_addr.s_addr = inet_addr(master_ip);
    serv_addr.sin_port = htons(master_port);
    int ret = bind(serv_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (ret == -1) {
        printf("bind error\n");
        return -1;
    }
    printf("bind ok\n");
    ret = listen(serv_sock, 20);
    if (ret == -1) {
        printf("listen error\n");
        return -1;
    }
    printf("listen ok\n");
    /*init socket end*/
    return serv_sock;

}

//Initialize socket
int init_client_socket(char *master_ip, int master_port){
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr2;
    memset(&serv_addr2, 0, sizeof(serv_addr2));
    serv_addr2.sin_family = AF_INET;
    serv_addr2.sin_addr.s_addr = inet_addr(master_ip);
    serv_addr2.sin_port = htons(master_port);

    connect(sock, (struct sockaddr *) &serv_addr2, sizeof(serv_addr2));
//    printf("socket: %d \n",sock);
    return sock;
}

int request_fuzzing_task(char *master_ip, int master_port, fuzz_task_t* ft){
    int sock = init_client_socket(master_ip, master_port);
    unf_proto_t send_buf, recv_buf;
    send_buf.cmd = REQUEST_NEW_SEEDS;
    send_buf.version = 1;
    send_buf.fuzzer_id = client_fuzzer_id;
    memset((&send_buf)->data, 0, RECV_MAX_SIZE);

    send(sock, &send_buf, sizeof(unf_proto_t), 0);
    int ret = recv(sock, &recv_buf, sizeof(unf_proto_t), 0);

    if( ret == -1 || ret == 0){
        fprintf(stderr, "recv error.\n" );
        close(sock);
        return -1;
    }
    
    if(recv_buf.cmd != REPLY_SEEDS){
        printf("reply_seeds failed.");
        close(sock);
        return -1;
    }

    if (client_fuzzer_id == 0) {
        client_fuzzer_id = recv_buf.fuzzer_id;
        printf("client fuzzer id: %u\n", client_fuzzer_id);
    }

    strcpy(ft->seed_name, recv_buf.data);
    ft->havoc_score = recv_buf.havoc_score;
    ft->splicing = recv_buf.splicing;
    printf("client recv seed_name: %s\n", ft->seed_name);

    close(sock);
    return 0;
}

void master_main_loop(char *master_ip, int master_port){
    int serv_sock = init_server_socket(master_ip, master_port);
    if(serv_sock == -1){
        printf("init_server_socket error\n");
        exit(0);
    }else{
        printf("init_server_socket ok\n");
    };
    struct sockaddr_in stClient;
    socklen_t len = sizeof(struct sockaddr_in);

    //char out_buff[BUF_SIZE];
    unf_proto_t recv_buf;
    unf_proto_t send_buf;

    fd_set stFdr;
    FD_ZERO(&stFdr);
    FD_SET(serv_sock, &stFdr);
    int MaxFd = serv_sock;
    //pthread_t tidp;

    int ret;
    start_evaluate_thread(); //The calling thread reads the seed from mongo and writes to the SeedEvalTask queue.
    while (1) {
        fd_set stFdrTmp = stFdr;
//      struct timeval sock_tv;
//      sock_tv.tv_sec = 10;
//      sock_tv.tv_usec = 10;
        ret = select(MaxFd + 1, &stFdrTmp, NULL, NULL, NULL);
//      printf("MaxFd: %d\n", MaxFd);
        if (ret < 0 ) { // Changed from ret <= 0 to ret < 0
            if (errno == 4) { // EINTR
                // printf("error MaxFd: %d", MaxFd);
                fprintf(stderr, "select interrupted by signal, continuing.\n");
                continue;
            } else {
                // printf("select error:%d\n", ret);
                // printf("%s\n", strerror(errno));
                // printf("%d\n", errno);
                continue;
            }
        } else if (ret == 0) {
            // printf("select time out error:%d\n", ret);
            // printf("%s\n", strerror(errno));
            // printf("%d\n", errno);
            continue;

        }
        // printf("select ok,ret=%d\n", ret);
        int i = 0;
        for (; i < MaxFd + 1; i++){
            if(FD_ISSET(i, &stFdrTmp)){
                if(i == serv_sock){
                    int iClient = accept(serv_sock, (struct sockaddr *)&stClient, &len);
                    if (iClient == -1){
                        continue;
                    }
                    // printf("iclient=%d\n", iClient);
                    FD_SET(iClient, &stFdr);
                    if(MaxFd < iClient){
                        MaxFd = iClient;
                    }
                }
                else{
                    /*receive request*/
                    ret = recv(i, &recv_buf, sizeof(unf_proto_t), 0);
                    // printf("ret:%d\n", ret);
                    if(ret > 0){
                        int reply_len = handle_recv_data(&recv_buf, &send_buf);
                        if(reply_len > 0){
                            send_buf.version = 1;
                            send_buf.cmd = REPLY_SEEDS;
                            send(i, &send_buf, sizeof(unf_proto_t), 0);
                        //printf("send to client: %s\n",send_buf->data);
                        }
                    }
                    else{
                        close(i);
                        FD_CLR(i, &stFdr);
                    }
                }
            }
        }

    //if (!stop_soon && exit_1) stop_soon = 2;
//    if (stop_soon) break;
    }

}


/* return 0 on sucess, -1 on failed.*/
int init_unifuzz( char* db_name,  char* db_site, char* target_name, char* local_binary_name) {
    if (db_site == nullptr || db_site[0] == 0){
        db_site =  "mongodb://127.0.0.1:27017";
    }
    if (target_name == nullptr){
        target_name = "yinqidi";
    }
    
    int ret_init_data_engine = init_data_engine_c("mongo", db_site, target_name, "seed");
    if(ret_init_data_engine == 1){
        fprintf(stderr, "the binary doesn't exit");
        return -1;
    }
    printf("mongodb_singleton_init\n");

      ///* Download the to-be-fuzzed binary */
    if(local_binary_name == nullptr) local_binary_name = target_name;
    download_target_binary_data_engine_c(local_binary_name);

    return 0;
}






#ifdef __cplusplus
}
#endif


