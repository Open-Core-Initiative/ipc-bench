# TunBench

[![GitHub license](https://img.shields.io/github/license/mashape/apistatus.svg?style=flat-square)](http://goldsborough.mit-license.org)

Benchmarks for Shared Memory and Tuntap inter-process-communication. Aim of the software is to measure sequential throughput. It sends a single message forth _and_ back (i.e., *ping pong*) between two processes and measures the transfer rate.


## Flowchart

<p  align="center">

<img  src="https://github.com/PrajvalRaval/ipc-bench/assets/41849970/2802a83f-a086-4444-abee-7ed93b396105">

</p>



## Usage

### Creating Tuntap Tunnels

To start using the app, we will first need to create two tuntap tunnels `tun0` & `tun1`.
You can execute the following code to do that:

```shell
sudo ip tuntap del tun0 mode tun
sudo ip tuntap add name tun0 mode tun user $USER
sudo ip link set tun0 up
sudo ip addr add 192.0.2.1 peer 192.0.2.2 dev tun0

sudo ip tuntap del tun1 mode tun
sudo ip tuntap add name tun1 mode tun user $USER
sudo ip link set tun1 up
sudo ip addr add 192.0.3.1 peer 192.0.3.2 dev tun1

sudo iptables -A FORWARD -i tun0 -s 192.0.2.2 -j ACCEPT
sudo iptables -A FORWARD -i tun1 -s 192.0.3.2 -j ACCEPT

sudo iptables -A FORWARD -o tun0 -d 192.0.2.2 -j ACCEPT
sudo iptables -A FORWARD -o tun1 -d 192.0.3.2 -j ACCEPT

sudo sysctl -w net.ipv4.ip_forward=1
```

This should configure two tunnels, you can check by running:

```shell
ifconfig -a
```

### Running the benchmarks

Install some required packages on Ubuntu:

```shell
sudo apt-get install pkg-config, libzmqpp-dev
```

Clone the project:

```shell
git clone https://github.com/Open-Core-Initiative/tunbench.git
```

You can build the project and all necessary executables using CMake. The following commands (executed from the root folder) should do the trick:

```shell
mkdir build
cd build
cmake ..
make
```

This will generate a `build/source` folder, holding further directories.

Simply execute the program named after the folder `build/source/shm/shm`.

```shell
sudo ./source/shm/shm -c 1000000 -s 1024
```

* `-c <count>`: How many messages to send between the server and client. Defaults to 1000.
* `-s <size>`: The size of individual messages. Defaults to 1000.

This will start a new server and client process, run benchmarks and print results to `stdout`.For example, running `build/source/shm/shm` outputs:

```
============ RESULTS ================
Message size:       256
Message count:      100
Total duration:     4.9320960	ms
Average duration:   49.229	us
Minimum duration:   43.520	us
Maximum duration:   177.408	us
Standard deviation: 16.557	us
Message rate:       20275.0000000	msg/s

===== TRANSFER RATES (BYTES) ========
Transfer rate:       81100.0000000	KB/s
Transfer rate:       79.1992188	MB/s
Transfer rate:       0.0773430	GB/s

===== TRANSFER RATES (BITS) ========
Transfer rate:       648800.0000000	Kb/s
Transfer rate:       633.5937500	Mb/s
Transfer rate:       0.6187439	Gb/s
=====================================
```
## Projects reffered

[goldsborough/ipc-bench](https://github.com/goldsborough/ipc-bench)
[pjg11/tuntcp](https://github.com/pjg11/tuntcp.git)