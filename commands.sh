# 编译
rm -rf build/*
cd build
cmake ..
make -j24

# 测试

build/bin/pfcon -u 192.168.252.5:27017 -i ./experiment/diff_corpus -b ~/programs/target_pcre -e 6
nohup build/bin/master-node -u 192.168.252.5:27017 -o ~/test0/output_m -b target_pcre6 -l 192.168.252.6 -p 12345 -N 16 -m none ~/programs/target_pcre_m @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c1 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c1 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c2 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c2 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c3 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c3 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c4 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c4 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c5 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c5 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c6 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c6 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c7 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c7 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c8 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c8 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c9 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c9 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c10 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c10 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c11 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c11 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c12 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c12 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c13 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c13 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c14 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c14 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c15 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c15 @@ > /dev/null 2>&1 &
nohup build/bin/afl-fuzz -o ~/test0/output_c16 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c16 @@ > /dev/null 2>&1 &

# 窗口

screen -S master -d -m build/bin/master-node -u 192.168.252.5:27017 -o ~/test0/output_m -b target_pcre6 -l 192.168.252.6 -p 12345 -N 16 -m none ~/programs/target_pcre_m @@
screen -S afl-fuzz-c1 -d -m build/bin/afl-fuzz -o ~/test0/output_c1 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c1 @@
screen -S afl-fuzz-c2 -d -m build/bin/afl-fuzz -o ~/test0/output_c2 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c2 @@
screen -S afl-fuzz-c3 -d -m build/bin/afl-fuzz -o ~/test0/output_c3 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c3 @@
screen -S afl-fuzz-c4 -d -m build/bin/afl-fuzz -o ~/test0/output_c4 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c4 @@
screen -S afl-fuzz-c5 -d -m build/bin/afl-fuzz -o ~/test0/output_c5 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c5 @@
screen -S afl-fuzz-c6 -d -m build/bin/afl-fuzz -o ~/test0/output_c6 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c6 @@
screen -S afl-fuzz-c7 -d -m build/bin/afl-fuzz -o ~/test0/output_c7 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c7 @@
screen -S afl-fuzz-c8 -d -m build/bin/afl-fuzz -o ~/test0/output_c8 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c8 @@
screen -S afl-fuzz-c9 -d -m build/bin/afl-fuzz -o ~/test0/output_c9 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c9 @@
screen -S afl-fuzz-c10 -d -m build/bin/afl-fuzz -o ~/test0/output_c10 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c10 @@
screen -S afl-fuzz-c11 -d -m build/bin/afl-fuzz -o ~/test0/output_c11 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c11 @@
screen -S afl-fuzz-c12 -d -m build/bin/afl-fuzz -o ~/test0/output_c12 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c12 @@
screen -S afl-fuzz-c13 -d -m build/bin/afl-fuzz -o ~/test0/output_c13 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c13 @@
screen -S afl-fuzz-c14 -d -m build/bin/afl-fuzz -o ~/test0/output_c14 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c14 @@
screen -S afl-fuzz-c15 -d -m build/bin/afl-fuzz -o ~/test0/output_c15 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c15 @@
screen -S afl-fuzz-c16 -d -m build/bin/afl-fuzz -o ~/test0/output_c16 -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none ~/programs/target_pcre_c16 @@

screen -ls
screen -r <session_name>
# detatch: ctrl + a + d
# kill: exit / ctrl + d / screen -S <session_name> -X quit

# 清理
ps aux | grep "build/bin/afl-fuzz -o /home/gabriel/test0/output_c.* -u 192.168.252.5:27017 -b target_pcre6 -l 192.168.252.6 -p 12345 -m none /home/gabriel/programs/target_pcre_c.* @@" | awk '{print $2}' | xargs kill -9

ps aux | grep "build/bin/master-node -u 192.168.252.5:27017 -o /home/gabriel/test0/output_m -b target_pcre6 -l 192.168.252.6 -p 12345 -m none /home/gabriel/programs/target_pcre_m @@" | awk '{print $2}' | xargs kill -9

sudo pkill -f check_mini.py

rm -rf ~/test0/output_* ~/programs/target_pcre_*
sudo rm -rf mongodb_monitor.log trace_mini_*

# on db server
mongo mongodb://127.0.0.1:27017/admin --eval "db.getSiblingDB('target_pcre6').dropDatabase()"
