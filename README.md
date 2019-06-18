# Shannon Direct-IO PCIe device driver

## Building the kernel module in Docker (e.g. for testing)

```
# Start a Docker container for clean builds
docker run -it -v ${PWD}/..:/build -w /build/shannon-direct-io-pcie-driver ubuntu:trusty bash

# Install build tools, includes mk-build-deps
apt update && apt install devscripts equivs --no-install-recommends --yes

# Automatically install all build dependencies from debian/control
mk-build-deps debian/control -r -i

# Install kernel headers
# Works only if Docker host and container kernel pair up
apt install linux-headers-$(uname -r) --yes

# Build package
dpkg-buildpackage -us -uc

# View result
ls -la ../
dpkg-deb -c ../*.deb

# Exit Docker
exit

# Ensure any files written by Docker are again user owned
sudo chown -R $USER ../
```

## Installing the Shannon driver as a DKMS module

1. Copy source to `/usr/src`:

```
sudo cp shannon-module_3.3.0 /usr/src/shannon-3.3.0
```

2. Build and install:

```
$ sudo dkms add -m shannon -v 3.3.0

Creating symlink /var/lib/dkms/shannon/3.3.0/source ->
                /usr/src/shannon-3.3.0

DKMS: add completed.

$ sudo dkms build -m shannon -v 3.3.0

Kernel preparation unnecessary for this kernel.  Skipping...

Building module:
cleaning build area....
KERNEL_TREE=/lib/modules/4.15.0-48-generic/build make modules.....
cleaning build area....

DKMS: build completed.

$ sudo dkms install -m shannon -v 3.3.0
```

3. Verify:

```
$ ls /var/lib/dkms/shannon/3.3.0/
4.15.0-48-generic  4.15.0-51-generic  source

$ dkms status
shannon, 3.3.0, 4.15.0-48-generic, x86_64: installed
shannon, 3.3.0, 4.15.0-51-generic, x86_64: installed

$ lsmod | grep shannon
shannon 675840 0
```
