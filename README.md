# Shannon Direct-IO PCIe device driver

Build:
```
# Need to be one step above source folder
cd ../

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
