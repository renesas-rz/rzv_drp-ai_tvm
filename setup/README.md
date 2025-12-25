# Installation

## Requirements

Requirements are listed below.

- OS : Ubuntu 22.04  
- Python : 3.10
- Common Packages : git, wget, unzip
- Additional Packages for Docker installation : docker
- Evaluation Board: RZ/V2L EVK, RZ/V2M EVK, RZ/V2MA EVK, RZ/V2H EVK or RZ/V2N EVK
- Related Software Version:
  - DRP-AI Translator
    - RZ/V2L, RZ/V2M, RZ/V2MA
      - [DRP-AI Translator V1.90 or later](https://www.renesas.com/us/en/products/microcontrollers-microprocessors/rz-arm-based-high-end-32-64-bit-mpus/drp-ai-translator)
      - DRP-AI_Translator-v1.90-Linux-x86_64-Install
    - RZ/V2H or RZ/V2N
      - [DRP-AI_Translator_i8 V1.11 or later](https://www.renesas.com/software-tool/drp-ai-translator-i8)
      - DRP-AI_Translator_i8-v1.11-Linux-x86_64-Install
  - SDK
    - RZ/V2L
      - [RZ/V2L AI SDK v5.00 or later](https://www.renesas.com/us/en/products/microcontrollers-microprocessors/rz-arm-based-high-end-32-64-bit-mpus/drp-ai-translator)
      - RTK0EF0160F05000SJ.zip
    - RZ/V2M
      - [RZ/V Verified Linux Package V3.0.7 or later](https://www.renesas.com/us/en/software-tool/rzv-verified-linux-package)
      - [DRP-AI Support Package V7.51 or later](https://www.renesas.com/us/en/products/microcontrollers-microprocessors/rz-arm-based-high-end-32-64-bit-mpusl/rzv2m-drp-ai-support-package)
      - SDK Installer (generated from the above packages)
    - RZ/V2MA
      - [RZ/V Verified Linux Package V3.0.7 or later](https://www.renesas.com/us/en/software-tool/rzv-verified-linux-package)
      - [DRP-AI Support Package V7.50 or later](https://www.renesas.com/us/en/products/microcontrollers-microprocessors/rz-arm-based-high-end-32-64-bit-mpus/rzv2ma-drp-ai-support-package)
      - SDK Installer (generated from the above packages)
    - RZ/V2H
      - [RZ/V2H AI SDK v5.20 or later](https://www.renesas.com/us/en/software-tool/rzv2h-ai-software-development-kit)
      - RTK0EF0180F05200SJ.zip
    - RZ/V2N
      - [RZ/V2N AI SDK v6.00 or later](https://www.renesas.com/us/en/software-tool/rzv2n-ai-software-development-kit)
      - RTK0EF0189F06000SJ.zip

You can install DRP-AI TVM[^1] either [directly on your host system](#installing-drp-ai-tvm1-mera2) or [using Docker](#installing-drp-ai-tvm1-with-docker-mera2).

## Installing DRP-AI TVM[^1] (Mera2)

This section describes how to install DRP-AI TVM directly on your host system without using Docker.

### 1. Preparation

Before installing DRP-AI TVM[^1], please follow the instructions below to install the software listed in [Requirements](#requirements).

#### Install Required Packages

```bash
# Update package list and install required packages
sudo apt-get update
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y python3-pip unzip
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y software-properties-common
sudo apt install -y ca-certificates gpg wget lsb-release
sudo add-apt-repository ppa:ubuntu-toolchain-r/test

# Add Kitware repository for latest CMake
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc | sudo gpg --dearmor > /tmp/kitware-archive-keyring.gpg
sudo cp /tmp/kitware-archive-keyring.gpg /usr/share/keyrings/kitware-archive-keyring.gpg
echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main" | sudo tee /etc/apt/sources.list.d/kitware.list

# Update package list again and install development tools
sudo DEBIAN_FRONTEND=noninteractive apt-get update --fix-missing
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential cmake
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y libgl1 libjpeg-dev
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y git vim llvm-14 file g++-13 gcc-13

# Set Python alternatives
sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 1
sudo update-alternatives --config python
```

#### Install DRP-AI Translator

Install the DRP-AI Translator that you have downloaded from the Renesas website:

```bash
# Set product type
export PRODUCT=V2H  # Change to V2L, V2M, V2MA, or V2N as needed

# Install DRP-AI Translator
if [ "$PRODUCT" = "V2L" ] || [ "$PRODUCT" = "V2M" ] || [ "$PRODUCT" = "V2MA" ]; then
  # For RZ/V2L, RZ/V2M, or RZ/V2MA
  chmod +x /tmp/DRP-AI_Translator-*-Install
  cd /opt
  sudo /tmp/DRP-AI_Translator-*-Install
else
  # For RZ/V2H or RZ/V2N
  chmod +x /tmp/DRP-AI_Translator_i8-*-Install
  cd /opt
  sudo /tmp/DRP-AI_Translator_i8-*-Install
fi
```

#### Install SDK for RZ/V

1. Download the appropriate SDK for your board:
   - RZ/V2L: [RZ/V2L AI SDK](https://www.renesas.com/us/en/products/microcontrollers-microprocessors/rz-arm-based-high-end-32-64-bit-mpus/drp-ai-translator)
   - RZ/V2M: [RZ/V Verified Linux Package V3.0.7 or later](https://www.renesas.com/us/en/software-tool/rzv-verified-linux-package) and [DRP-AI Support Package V7.51 or later](https://www.renesas.com/us/en/products/microcontrollers-microprocessors/rz-arm-based-high-end-32-64-bit-mpusl/rzv2m-drp-ai-support-package)
   - RZ/V2MA: [RZ/V Verified Linux Package V3.0.7 or later](https://www.renesas.com/us/en/software-tool/rzv-verified-linux-package) and [DRP-AI Support Package V7.50 or later](https://www.renesas.com/us/en/products/microcontrollers-microprocessors/rz-arm-based-high-end-32-64-bit-mpus/rzv2ma-drp-ai-support-package)
   - RZ/V2H: [RZ/V2H AI SDK](https://www.renesas.com/us/en/software-tool/rzv2h-ai-software-development-kit)
   - RZ/V2N: [RZ/V2N AI SDK](https://www.renesas.com/us/en/software-tool/rzv2n-ai-software-development-kit)

2. Install the SDK:

```bash
# Set SDK file path and prepare toolchain script based on product type
if [ "$PRODUCT" = "V2L" ]; then
  # For RZ/V2L
  SDK_FILE="/path/to/RTK0EF0160F05000SJ.zip"
  
elif [ "$PRODUCT" = "V2M" ] || [ "$PRODUCT" = "V2MA" ]; then
  # For RZ/V2M or RZ/V2MA
  # For RZ/V2M and RZ/V2MA, the SDK is built from DRP-AI Support Package and Linux Package
  # Use the SDK installer you generated according to the DRP-AI Support Package Release Note
  TOOLCHAIN_SCRIPT="/path/to/sdk-installer.sh"
  chmod +x ${TOOLCHAIN_SCRIPT}
  
elif [ "$PRODUCT" = "V2N" ]; then
  # For RZ/V2N
  SDK_FILE="/path/to/RTK0EF0189F06000SJ.zip"
  
else
  # For RZ/V2H
  SDK_FILE="/path/to/RTK0EF0180F05200SJ.zip"
fi

# Extract toolchain script for all boards except RZ/V2M and RZ/V2MA
if [ "$PRODUCT" != "V2M" ] && [ "$PRODUCT" != "V2MA" ]; then
  cd /tmp
  unzip $SDK_FILE
  TOOLCHAIN_SCRIPT=$(find ./ -name "*toolchain*${PRODUCT,,}*sh" -o -name "*${PRODUCT,,}*toolchain*.sh" | head -n 1)
  chmod +x ${TOOLCHAIN_SCRIPT}
fi

# Install SDK
sudo ${TOOLCHAIN_SCRIPT}

# Create symbolic link
CORTEX_PATH=$(find /opt -name "cortexa55-poky-linux")
if [ -n "$CORTEX_PATH" ]; then
  sudo ln -s ${CORTEX_PATH} $(dirname ${CORTEX_PATH})/../aarch64-poky-linux
fi
```

### 2. Install Python Dependencies

```bash
# Install Python packages
pip3 install --upgrade pip
pip3 install psutil numpy==1.26.4
pip3 install cython==3.0.11
pip3 install decorator attrs
pip3 install tensorflow==2.18.1 tflite tqdm
```

### 3. Clone the Repository

```bash
# Set TVM_ROOT directory
export TVM_ROOT=${HOME}/drp-ai_tvm

# Clone the repository
git clone --recursive https://github.com/renesas-rz/rzv_drp-ai_tvm.git ${TVM_ROOT}
```

### 4. Set Environment Variables

```bash
# Set SDK path
echo 'export SDK="'$(find /opt/ -name "sysroots")/../'"' >> ~/.bashrc

# Set DRP-AI Translator paths
if [ "$PRODUCT" = "V2L" ] || [ "$PRODUCT" = "V2M" ] || [ "$PRODUCT" = "V2MA" ]; then
  # For RZ/V2L, RZ/V2M, or RZ/V2MA
  echo 'export TRANSLATOR="/opt/DRP-AI_Translator/translator/"' >> ~/.bashrc
  echo 'export QUANTIZER="/opt/DRP-AI_Translator/drpAI_Quantizer/"' >> ~/.bashrc
else
  # For RZ/V2H or RZ/V2N
  echo 'export PYTHONPATH="/opt/DRP-AI_Translator_i8/drpAI_Quantizer/"' >> ~/.bashrc
  echo 'export TRANSLATOR="/opt/DRP-AI_Translator_i8/translator/"' >> ~/.bashrc
  echo 'export QUANTIZER="/opt/DRP-AI_Translator_i8/drpAI_Quantizer/"' >> ~/.bashrc
fi

echo 'export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$LIBRARY_PATH"' >> ~/.bashrc
echo 'export LIBRARY_PATH="$LD_LIBRARY_PATH"' >> ~/.bashrc
echo 'export PRODUCT="'${PRODUCT}'"' >> ~/.bashrc

# Apply changes to current shell
source ~/.bashrc
```

### 5. Setup DRP-AI TVM[^1] Environment

```bash
# Remove python3-yaml to avoid conflicts
sudo apt-get -y purge python3-yaml

# Install pip packages
cd ${TVM_ROOT}/obj/pip_package
pip3 install mera2_r*
pip3 install tvm-*
pip3 install mera2_c*

# Clone additional dependencies
cd ${TVM_ROOT}/3rdparty
git clone https://github.com/gabime/spdlog.git
git clone https://github.com/chriskohlhoff/asio.git

# Copy header files
cp ${TVM_ROOT}/setup/include/*.h ${TVM_ROOT}/tvm/include/tvm/runtime/
```

### 6. Verify Installation

After completing the installation steps, you can verify the installation by running the sample applications provided in the tutorials directory.

```bash
# Navigate to tutorials directory
cd ${TVM_ROOT}/tutorials
```

## Installing DRP-AI TVM[^1] with Docker (Mera2)
### 1. Preparation for Docker

Before installing DRP-AI TVM[^1] with Docker, please ensure you have Docker installed and running on your Ubuntu 22.04 system.

```sh
# Install required packages if not already installed
sudo apt update
sudo apt install -y wget unzip
```

### 2. Prepare Required Files

Before proceeding, ensure you have the following files in your working directory:

1. DRP-AI Translator installer (appropriate version for your board)
   - For RZ/V2L, RZ/V2M, or RZ/V2MA: Download from [DRP-AI Translator page](https://www.renesas.com/us/en/products/microcontrollers-microprocessors/rz-arm-based-high-end-32-64-bit-mpus/drp-ai-translator)
   - For RZ/V2H or RZ/V2N: Download from [DRP-AI Translator i8 page](https://www.renesas.com/software-tool/drp-ai-translator-i8)
2. SDK package (appropriate version for your board)
   - For RZ/V2L: Download from [RZ/V2L AI SDK page](https://www.renesas.com/us/en/products/microcontrollers-microprocessors/rz-arm-based-high-end-32-64-bit-mpus/drp-ai-translator)
   - For RZ/V2M: Build SDK using [RZ/V Verified Linux Package V3.0.7 or later](https://www.renesas.com/us/en/software-tool/rzv-verified-linux-package) and [DRP-AI Support Package V7.51 or later](https://www.renesas.com/us/en/products/microcontrollers-microprocessors/rz-arm-based-high-end-32-64-bit-mpusl/rzv2m-drp-ai-support-package)
   - For RZ/V2MA: Build SDK using [RZ/V Verified Linux Package V3.0.7 or later](https://www.renesas.com/us/en/software-tool/rzv-verified-linux-package) and [DRP-AI Support Package V7.50 or later](https://www.renesas.com/us/en/products/microcontrollers-microprocessors/rz-arm-based-high-end-32-64-bit-mpus/rzv2ma-drp-ai-support-package)
   - For RZ/V2H: Download from [RZ/V2H AI SDK page](https://www.renesas.com/us/en/software-tool/rzv2h-ai-software-development-kit)
   - For RZ/V2N: Download from [RZ/V2N AI SDK page](https://www.renesas.com/us/en/software-tool/rzv2n-ai-software-development-kit)
3. Dockerfile for your board

You can download the Dockerfile from the official repository:

```sh
# Set product type
export PRODUCT=V2H  # Change to V2L, V2M, V2MA, or V2N as needed

# Download Dockerfile
wget https://raw.githubusercontent.com/renesas-rz/rzv_drp-ai_tvm/main/Dockerfile -O Dockerfile
```

### 3. Prepare SDK Files

```sh
# Determine SDK filename based on product
if [ "$PRODUCT" = "V2L" ]; then
  SDK_FILE="RTK0EF0160F05000SJ.zip"
elif [ "$PRODUCT" = "V2N" ]; then
  SDK_FILE="RTK0EF0189F06000SJ.zip"
elif [ "$PRODUCT" = "V2H" ]; then
  SDK_FILE="RTK0EF0180F05200SJ.zip"
fi

# Extract toolchain script for all boards except RZ/V2M and RZ/V2MA
if [ "$PRODUCT" != "V2M" ] && [ "$PRODUCT" != "V2MA" ]; then
  # Extract toolchain script
  mkdir -p sdk_temp
  unzip -q ${SDK_FILE} -d sdk_temp
  TOOLCHAIN_SCRIPT=$(find sdk_temp -type f -name "*toolchain*${PRODUCT,,}*.sh" -o -name "*${PRODUCT,,}*toolchain*.sh" | head -n 1)

  if [ -n "$TOOLCHAIN_SCRIPT" ]; then
      cp "$TOOLCHAIN_SCRIPT" ./
      TOOLCHAIN_SCRIPT="./$(basename $TOOLCHAIN_SCRIPT)"
      echo "Toolchain script extracted: $(basename $TOOLCHAIN_SCRIPT)"
      chmod +x ${TOOLCHAIN_SCRIPT}
  else
      echo "Error: Could not find toolchain script in SDK"
      rm -rf sdk_temp
      exit 1
  fi

  # Clean up temporary directory
  rm -rf sdk_temp
else
  # For RZ/V2M or RZ/V2MA, use the SDK installer you generated
  TOOLCHAIN_SCRIPT="./sdk-installer.sh"  # Replace with your actual SDK installer filename
  echo "Using SDK installer for RZ/V2M or RZ/V2MA: ${TOOLCHAIN_SCRIPT}"
  chmod +x ${TOOLCHAIN_SCRIPT}
fi
```

### 4. Prepare Build Directory

To avoid copying unnecessary files during Docker build, create a separate build directory:

```sh
# Create build directory
mkdir -p docker_build

# Determine DRP-AI Translator filename based on product
if [ "$PRODUCT" = "V2L" ] || [ "$PRODUCT" = "V2M" ] || [ "$PRODUCT" = "V2MA" ]; then
  TRANSLATOR_FILE="DRP-AI_Translator-v1.90-Linux-x86_64-Install"
else
  TRANSLATOR_FILE="DRP-AI_Translator_i8-v1.11-Linux-x86_64-Install"
fi

# Copy only required files to build directory
cp Dockerfile docker_build/
cp ${TRANSLATOR_FILE} docker_build/
cp ${TOOLCHAIN_SCRIPT} docker_build/
```

### 5. Build Docker Image

```sh
cd docker_build
docker build -t drp-ai_tvm_${PRODUCT,,}_image_${USER} -f Dockerfile --build-arg PRODUCT=${PRODUCT} .
```

**Tip:** If you encounter an error like "404 Not Found," try adding the `--no-cache` option with "`docker build`".

### 6. Run Docker Container

```sh
docker run -it --name drp-ai_tvm_${PRODUCT,,}_container_${USER} drp-ai_tvm_${PRODUCT,,}_image_${USER}
```

### Special Instructions for RZ/V2M and RZ/V2MA

For RZ/V2M and RZ/V2MA, the SDK preparation process is different from other boards:

1. Download the *DRP-AI Support Package* from [Renesas Web Page](https://www.renesas.com/products/microcontrollers-microprocessors/rz-mpus/rzv-embedded-ai-mpus#software_amp_tools).  
   To use the *DRP-AI Support Package*, *Linux Package* is required.  
   *Linux Package* can be found in the page of *DRP-AI Support Package*.

2. **Build image/SDK** according to the *DRP-AI Support Package Release Note* to generate following files:  

    | Name | Filename |Usage|  
    |----|---|---|  
    | **SDK Installer** | poky-*.sh |Used when compiling model and building the application.|  
    | Kernel Image | Image-*.bin |Used when booting kernel on the RZ/V evaluation board.|  
    | Device Tree File | *.dtb |Used when booting kernel on the RZ/V evaluation board.|  
    | Root Filesystem | *.tar.bz2 |Used when booting kernel on the RZ/V evaluation board.|  
    | Bootloaders | i.e., *.bin,* .srec| See Linux Package Release Note for more details.<br>Used when booting the RZ/V evaluation board.|  

3. Use the generated SDK installer (poky-*.sh) for Docker build.

[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.