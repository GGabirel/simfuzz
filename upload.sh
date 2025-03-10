#!/bin/bash

# upload.sh: 运行 pfcon 命令上传数据
# 使用方法: ./upload.sh [number]
# 如果没有提供 [number]，则默认为 2

# 默认值为 2
NUMBER=${1:-2}

# 定义日志函数，带时间戳
log() {
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] $1"
}

# 目标命令
COMMAND="./build/bin/pfcon -u 192.168.252.5:27017 -i $HOME/programs/input_pcre -b $HOME/programs/target_pcre -e $NUMBER"

# 检查 pfcon 命令是否存在且可执行
if [ ! -x "./build/bin/pfcon" ]; then
    log "错误: ./build/bin/pfcon 不存在或不可执行。请检查路径和权限。" >&2
    exit 1
fi

# 检查输入目录是否存在
if [ ! -d "$HOME/programs/input_pcre" ]; then
    log "错误: 输入目录 $HOME/programs/input_pcre 不存在。" >&2
    exit 1
fi

# 运行命令
log "开始运行命令: $COMMAND"
$COMMAND

# 检查命令是否成功执行
if [ $? -eq 0 ]; then
    log "命令成功执行。"
else
    log "命令执行失败。" >&2
    exit 1
fi

exit 0
