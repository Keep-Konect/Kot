echo -e "\e[32mCreating compilation dirs...\e[0m"

mkdir -p "Sysroot/include/kot/"
mkdir -p "Sysroot/lib/"
mkdir -p "Bin/Modules/"

cp -p "Build/Starter.cfg" "Bin/Modules/Starter.cfg"

echo -e "\e[32mCompiling kot libraries...\e[0m"

cd "Sources/"

# libc

sudo make -C "Libs/libc/build"

# abi

sudo make -C "Libs/abi/build"

echo -e "\e[32mCompiling kot drivers...\e[0m"

# Drivers
sudo make -C "Modules/Drivers/PS2/Build"
sudo make -C "Modules/Drivers/VGA/Build"

echo -e "\e[32mCompiling kot modules...\e[0m"

# Services
sudo make -C "Modules/Services/System/Build"
sudo make -C "Modules/Services/WM/Build"

# ramfs

cd "../"
sudo bash "./Tools/BuildRamFS.sh"

# kernel

cd "./Sources/Kernel/"
echo -e "\e[32mBuilding kernel...\e[0m"

make all