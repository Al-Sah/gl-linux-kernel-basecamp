# [exercise08](https://gl-khpi.gitlab.io/exercises/exercise08/) 


### Getting the kernel sources.

Go to [kernel.org](https://www.kernel.org/), download tarball it and open

Prepare build directory: (Run following commands)

    export BUILD_KERNEL=$(pwd)/x01
    mkdir x01; cd x01
    make ARCH=i386 O=${BUILD_KERNEL} defconfig
    make menuconfig
    make -j4
    
### Configuring and build kernel

in kernel build directory: (Run following commands)

    make menuconfig
    make -j4


### Configuring and build buildroot

and again, run following commands:

    git clone git://git.buildroot.net/buildroot
    cd buildroot
    export BUILD_ROOTFS=$(pwd)/x01
    mkdir x01; cd x01
    make O=${BUILD_ROOTFS} qemu_x86_defconfig
    cd ${BUILD_ROOTFS}

    ** make menuconfig **


### Launching

    sudo qemu-system-i386 \
    -kernel ${BUILD_KERNEL}/arch/i386/boot/bzImage \
    -append "root=/dev/sda console=ttyS0" \
    -drive format=raw,file=${BUILD_ROOTFS}/images/rootfs.ext3 \
    -nic user,hostfwd=tcp::8022-:22 &

    # Connecting to VM via ssh
    ssh -p 8022 user@localhost



## Extra (Display complete information about the built system)



| Architecture:                   | x86_64                                      |
| ------------------------------- | ------------------------------------------- |
| CPU op-mode(s):                 | 32-bit, 64-bit                              |
| Address sizes:                  | 43 bits physical, 48 bits virtual           |
| CPU(s):                         | 8                                           |
| On-line CPU(s) list:            | 0-7                                         |
| Thread(s) per core:             | 2                                           |
| Cores:                          | 4                                           |
| Vendor ID:                      | AuthenticAMD                                |
| Model name:                     | AMD Ryzen 5 3400G with Radeon Vega Graphics
| Stepping:                       | 1
| Frequency boost:                | enabled
| Virtualization:                 | AMD-V

    Other characteristics
| Name                            | value                                  |
|-------------------------------  | -----------------------------------    |
| GNU Make                        | 4.2.1 Built for x86_64-pc-linux-gnu   
| gcc version                     | 9.3.0
| Ubuntu version                  | 9.3.0-17ubuntu1~20.04




