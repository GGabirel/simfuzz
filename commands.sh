# 编译
rm -rf build/*
cd build
cmake ..
make -j24

# 测试

build/bin/pfcon -u 192.168.252.5:27017 -i ./experiment/diff_corpus -b ~/programs/target_pcre -e 7
nohup build/bin/master-node -u 192.168.252.5:27017 -o ~/test0/output_m -b target_pcre7 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_m @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c1 -u 192.168.252.5:27017 -b target_pcre7 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c1 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c2 -u 192.168.252.5:27017 -b target_pcre7 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c2 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c3 -u 192.168.252.5:27017 -b target_pcre7 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c3 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c4 -u 192.168.252.5:27017 -b target_pcre7 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c4 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c5 -u 192.168.252.5:27017 -b target_pcre7 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c5 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c6 -u 192.168.252.5:27017 -b target_pcre7 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c6 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c7 -u 192.168.252.5:27017 -b target_pcre7 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c7 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c8 -u 192.168.252.5:27017 -b target_pcre7 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c8 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c9 -u 192.168.252.5:27017 -b target_pcre7 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c9 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c10 -u 192.168.252.5:27017 -b target_pcre7 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c10 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c11 -u 192.168.252.5:27017 -b target_pcre7 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c11 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c12 -u 192.168.252.5:27017 -b target_pcre7 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c12 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c13 -u 192.168.252.5:27017 -b target_pcre7 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c13 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c14 -u 192.168.252.5:27017 -b target_pcre7 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c14 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c15 -u 192.168.252.5:27017 -b target_pcre7 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c15 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c16 -u 192.168.252.5:27017 -b target_pcre7 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c16 @@ > /dev/null 2>&1 &

# 清理
ps aux | grep "build/bin/afl-fuzz -o /home/gabriel/test0/output_c.* -u 192.168.252.5:27017 -b target_pcre7 -l 192.168.252.6 -p 12345 -m none /home/gabriel/programs/target_pcre_c.* @@" | awk '{print $2}' | xargs kill -9

ps aux | grep "build/bin/master-node -u 192.168.252.5:27017 -o /home/gabriel/test0/output_m -b target_pcre7 -l 192.168.252.6 -p 12345 -m none /home/gabriel/programs/target_pcre_m @@" | awk '{print $2}' | xargs kill -9

sudo pkill -f check_mini.py

rm -rf ~/test0/output_* ~/programs/target_pcre_*
sudo rm -rf mongodb_monitor.log trace_mini_*

# on db server
mongo mongodb://127.0.0.1:27017/admin --eval "db.getSiblingDB('target_pcre7').dropDatabase()"
