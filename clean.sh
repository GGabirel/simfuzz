#!/bin/bash

# 1. 查找所有运行中的 afl-fuzz 进程，并提取 PID（排除 grep 进程本身）
pids=$(ps -ef | grep afl-fuzz | grep -v grep | awk '{print $2}')

if [ -n "$pids" ]; then
    echo "找到以下 afl-fuzz 进程的 PID: $pids"
    # 2. 强制终止这些 PID
    kill -9 $pids
    echo "已杀死所有 afl-fuzz 进程。"
else
    echo "未找到 afl-fuzz 进程。"
fi

# 3. 查找所有包含 'master-node' 的进程，并提取 PID（排除 grep 进程本身），同上终止
master_pids=$(ps -ef | grep master-node | grep -v grep | awk '{print $2}')

if [ -n "$master_pids" ]; then
    echo "找到以下 master-node 进程的 PID: $master_pids"
    kill -9 $master_pids
    echo "成功终止所有 'master-node' 进程。"
else
    echo "未找到 'master-node' 进程。"
fi

# 4. 删除指定的文件和目录
echo "正在删除 ~/test0/output_* 和 ~/programs/target_pcre_* ..."
rm -rf ~/test0/output_* ~/programs/target_pcre_*

# 检查删除操作是否成功
if [ $? -eq 0 ]; then
    echo "成功删除指定的文件和目录。"
else
    echo "删除文件和目录时出错。"
fi

echo "所有任务已完成。"
