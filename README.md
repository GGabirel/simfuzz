# SIMFUZZ: Conflict-Aware Parallel Fuzzing

SIMFUZZ is a parallel fuzzing framework that significantly improves fuzzing efficiency by mitigating task conflicts using path similarity–aware scheduling. It is designed to enhance traditional parallel fuzzing architectures by reducing redundant exploration and increasing coverage and crash discovery rates.

## 🔍 Motivation

Parallel fuzzing distributes test case generation across multiple fuzzing instances to improve throughput. However, current designs often suffer from **task conflicts**—multiple instances redundantly exploring similar execution paths—due to the lack of path-level awareness in seed scheduling. These inefficiencies limit overall effectiveness and waste computing resources.

## 🚀 Key Features

- **Conflict-Aware Scheduling**: Avoids redundant effort by analyzing path similarity among test inputs.
- **Branch-Level Seed Encoding**: Seeds are encoded as bitmaps representing branch-level coverage.
- **Incremental Clustering**: Dynamically groups similar seeds based on execution path overlap.
- **Two-Stage Scheduling**:
  - **Local Scheduling**: Assigns similar seeds to the same fuzzing instance.
  - **Global Prioritization**: Ensures high-potential seeds are fuzzed first across all instances.

## 📈 Evaluation Results

SIMFUZZ was evaluated on **19 real-world programs and benchmark targets**. Compared to a state-of-the-art baseline, it demonstrated:

- **6.7% increase** in average branch coverage.
- **3.9% reduction** in task conflicts.
- Discovery of significantly **more unique crashes** in several cases.
- **15 previously unknown vulnerabilities** found in widely-used software, all assigned **CVE identifiers**.

## 📜 Publications

This project is based on the research presented in:

> XXXXX

## 🛠️ Build & Usage

### 📆 Dependencies

#### 1. Install MongoDB

Enable remote access:

- Modify the `bind_ip` in `/etc/mongodb.conf`:
  ```bash
  bind_ip = 0.0.0.0
  ```

- Open port `27017` in `iptables`:
  ```bash
  iptables -A INPUT -p tcp -m state --state NEW -m tcp --dport 27017 -j ACCEPT
  ```

#### 2. Install BSON Library

```bash
apt-get install libmongoc-1.0-0
apt-get install libbson-1.0
sudo apt-get install cmake libssl-dev libsasl2-dev
```

#### 3. Install MongoDB C Driver

```bash
git clone https://github.com/mongodb/mongo-c-driver.git
cd mongo-c-driver
git checkout x.y.z  # Replace with desired release tag
python build/calc_release_version.py > VERSION_CURRENT
mkdir cmake-build
cd cmake-build
cmake -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF ..
make
sudo make install
```

---

### 🏗️ Building

#### On Ubuntu

Use `cmake` to build the project:

```bash
mkdir build
cd build
cmake ..
make
```

---

### 🎛️ Components

#### ✅ Controller

Upload testcases and the instrumented binary (e.g., `who`) to MongoDB:

```bash
build/bin/pfcon -u <MONGODB_IP>:27017 -i <INPUT_DIR> -b <TARGET> -e <VERSION_NUMBER>
```

#### ✅ Master Node

Create a new output directory and run the master node:

```bash
mkdir output
build/bin/master-node -u <DB_SERVER_IP>:27017 -o ./output -b <TASK_NAME> -l <MASTER_IP> -p <MASTER_PORT> -N <CLIENT_COUNT> ./LOCAL_PROGRAM_NAME @@
```

#### ✅ Client Node

We modified `afl-fuzz.c` and added `p-fuzz.c` to implement our fuzzing logic. You can use the client just like `afl-fuzz`.

```bash
mkdir output
build/bin/afl-fuzz -o output/ -u <DB_SERVER_IP>:27017 -b <TASK_NAME> -l <MASTER_IP> -p <MASTER_PORT> ./LOCAL_PROGRAM_NAME @@
```


## 🔒 Disclaimer

SIMFUZZ is intended for **research and educational use only**. Always obtain permission before fuzzing third-party software or services.


> This is only for artifact preview. We will release the source code right upon acceptance to support reproducibility and further research.
> 
> You can find the pre-compiled binaries under ./build/