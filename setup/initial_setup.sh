
# update & install libs
apt update
DEBIAN_FRONTEND=noninteractive apt install -y software-properties-common
add-apt-repository ppa:ubuntu-toolchain-r/test
apt update
DEBIAN_FRONTEND=noninteractive apt install -y build-essential cmake \
libomp-dev libgtest-dev libgoogle-glog-dev libtinfo-dev zlib1g-dev libedit-dev \
libxml2-dev llvm-8-dev g++-9 gcc-9 wget

apt-get install -y python3-pip
pip3 install --upgrade pip
apt-get -y install unzip vim git
pip3 install decorator attrs scipy numpy pytest onnx==1.9.0
pip install torch==1.8.0 torchvision==0.9.0

# Update gcc to 9.4
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 10
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 20
update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-7 10
update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 20
update-alternatives --install /usr/bin/cc cc /usr/bin/gcc 30
update-alternatives --set cc /usr/bin/gcc
update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 30
update-alternatives --set c++ /usr/bin/g++
update-alternatives --set gcc "/usr/bin/gcc-9"
update-alternatives --set g++ "/usr/bin/g++-9"

# Install onnx runtime
wget https://github.com/microsoft/onnxruntime/releases/download/v1.8.1/onnxruntime-linux-x64-1.8.1.tgz -O /tmp/onnxruntime.tar.gz
tar -xvzf /tmp/onnxruntime.tar.gz -C /tmp/
#sudo mv -R /tmp/onnxruntime-linux-x64-1.8.1/ /opt/
mv /tmp/onnxruntime-linux-x64-1.8.1/ /opt/

