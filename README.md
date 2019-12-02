#NewEnergyLB

This is a Load Balancer (LB) developed using the parallel programming model CHARM++. The LB applies DVFS (Dynamic Voltage and Frequency Scaling) techniques instead of tasks distribution (process of migrating objects between cores).

The proposed balancer obtains the weighted loads of the system and, based on this information, increase the clock frequency of the cores with higher loads and decrease the frequency of the other cores, accordingly to their respective loads.

**Make shure that you have installed everything that the CHARM++ need to run the LB properly.** This includes the installation of `cpufreq` module.

Here are some links that may help: 

- Installation and Usage: http://charm.cs.uiuc.edu/users/gupta/2012_CloudHPCLB_charm/doc/html/install/2.html
- Tutorial: http://charmplusplus.org/tutorial/
- Manual: https://charm.readthedocs.io/en/latest/charm++/manual.html

To validate the proposal, the following equipment/configuration was used:
A PC with Intel Core I7-4810MQ and Linux Mint OS with kernel version 4.13.0. The version of CHARM ++ used was 6.5.1 with g++ v5.4.0.

###Basic instalation

1. Install g++
```bash
$ apt-get install g++
$ apt-get install g++-multilib
$ apt-get install build-essential
```

2. Download and install CHARM++
```bash
$ git clone https://charm.cs.illinois.edu/gerrit/charm
```

3. Checkout to version 6.5.1
```bash
$ git checkout v6.5.1
```

4. Include all .C, .ci and .h files to `charm/src/ck-ldb`

5. Build
```bash
$ ./build charm++ multicore-linux64
$ echo 'export PATH=$PATH:~/charm/bin' >> /home/your_user/.bashrc
```

###Configuring cpufreq

To use the load balancer, the cpufreq `userspace` mode need to be enabled, and for that, first you need to disable the intel driver. So, while at the GRUB menu:

1. Choose the linux OS
2. Select option `e`
3. Inlcude this command line: `intel_pstate=disable`
4. Initialize the OS

With the OS initialized, open your terminal and enable the `userspace` profile for all your processor cores with the comand below:
```bash
$ cpufreq-set -g userspace -c 0
```
Ps.: do it for all your cores (-c0, -c 1, -c 2, ...).

###Testing the load balancer

1. Enter your CHARM++ folder 
Open Makefile_lb.sh in your text editor and include the name of the new load balancer(NewEnergyLB) in OTHER_LDBS. Save it.
2. Run this script (`$ ./Makefile_lb.sh`)
3. Run `$ make depends`
4. Run `$ make charm++ OPTS="-O2 -Wall"`
5. Enter the benchmarks folder
6. Run `$ make lb`
7. Enter the tests folder
8. Open run-lb.sh in your text editor. Change the selected load balancer to "E". Save it.
9. Run the script in sudo mode (`$ sudo ./run-lb.sh`)
10. Enter the results folder and get your results :)