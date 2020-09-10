# rt_aide

rt_aide is an ongoing project developing an all-in-one solution for real-time Linux environment

## Usage

To build the example:

1) setup-the environment and check what real-time linux approach the is running

```bash
source ./setup.bash
./scripts/check_kernel.sh # python 2
```
note that for Xenomai 3, users can select whether to use between Alchemy(native) and POSIX(posix) skin

2) compile and run
```bash
make
./start.sh
```