Linux kernel DANGER
============

There are several guides for kernel developers and users. These guides can
be rendered in a number of formats, like HTML and PDF. Please read
Documentation/admin-guide/README.rst first.

In order to build the documentation, use ``make htmldocs`` or
``make pdfdocs``.  The formatted documentation can also be read online at:

    https://www.kernel.org/doc/html/latest/

There are various text files in the Documentation/ subdirectory,
several of them using the Restructured Text markup notation.

Please read the Documentation/process/changes.rst file, as it contains the
requirements for building and running the kernel, and information about
the problems which may result by upgrading your kernel.

# Omoshiroi Code Files

Headers

[arch/x86/include/uapi/asm/processor-flags.h](arch/x86/include/uapi/asm/processor-flags.h) - CPU Flags, like CR0, CR4

[arch/x86/include/asm/segment.h](arch/x86/include/asm/segment.h) - Segment Descriptors Definitions

[arch/x86/include/asm/pgtable_types.h](arch/x86/include/asm/pgtable_types.h) - Page Table Templates

[arch/x86/include/uapi/asm/setup.h](arch/x86/include/uapi/asm/setup.h) - My Hack Functions

[arch/x86/include/asm/ptrace.h](arch/x86/include/asm/ptrace.h) - Usermode/Kernelmode Partterns

Sources

[arch/x86/kernel/cpu/common.c](arch/x86/kernel/cpu/common.c) - Init some CPU Features

[arch/x86/kernel/setup.c](arch/x86/kernel/setup.c) - Early Boot Kernel Setup

[arch/x86/kernel/head_64.S](arch/x86/kernel/head_64.S) - Early CPU Setup

[arch/x86/kernel/head64.c](arch/x86/kernel/head64.c) - Early CPU Setup

[arch/x86/kernel/process_64.c](arch/x86/kernel/process_64.c) - Start Usermode Threads

[arch/x86/kernel/entry_64.S](arch/x86/kernel/entry_64.S) - syscall/sysret

[arch/x86/mm/fault.c](arch/x86/mm/fault.c) - Page Fault Handler

[fs/exec.c](fs/exec.c) - Start ELF Binaries from Kernel

[kernel/sched/core.c](kernel/sched/core.c) - Scheduler

# Build & Run on Ubuntu 24.04

```
apt update
apt install -y build-essential libncurses-dev bison flex libssl-dev libelf-dev bc dwarves git
cp /boot/config-$(uname -r) .config
make menuconfig
```

Then, disable ```CONFIG_SYSTEM_TRUSTED_KEYS``` and ```BTF```

```
-> Cryptographic API (CRYPTO [=y])
    -> Certificates for signature checking
        -> Provide system-wide ring of trusted keys (SYSTEM_TRUSTED_KEYRING)
            -> Additional X.509 keys for default system keyring (SYSTEM_TRUSTED_KEYS [=])

-> Kernel hacking
    -> Compile-time checks and compiler options
        -> Generate BTF typeinfo (DEBUG_INFO_BTF [=n])
```

Then you can 

```
make localmodconfig
make -j24
make modules_install
make install
update-grub
```

# x64 Hacking Status

- [] CR0 Write Protection Disable
- [x] Hack the user GDT to Ring 0
- [x] Disabled PTI
- [x] Disabled SMEP/SMAP
- [x] Hacked User Segment Descriptors to Ring 0
- [x] Hacked User Page Table Templates to Ring 0