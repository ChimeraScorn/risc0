# https://github.com/adambreland/cpp-simple_bazel_cpp_toolchain
# https://github.com/bazelbuild/rules_cc/blob/main/cc/private/toolchain/cc_toolchain_config.bzl

load("@bazel_tools//tools/cpp:unix_cc_toolchain_config.bzl", "cc_toolchain_config")

def pkg_path_from_label(label):
    if label.workspace_root:
        return label.workspace_root + "/" + label.package
    else:
        return label.package

def gcc_toolchain_config(name, sysroot, version):
    sysroot_path = pkg_path_from_label(Label(sysroot))

    tool_paths = {tool: "bin/riscv32-unknown-elf-{}".format(tool) for tool in [
        "ar",
        "cpp",
        "gcc",
        "gcov",
        "ld",
        "nm",
        "objdump",
        "strip",
    ]}
    tool_paths["llvm-cov"] = "bin/riscv32-unknown-elf-gcov"

    common_flags = [
        "-DRISCV=1",
        "-mabi=ilp32",
        "-march=rv32im",
        # A freestanding environment is one in which the standard library may not exist,
        # and program startup may not necessarily be at main.
        "-ffreestanding",
        "-fno-exceptions",
        "-fno-non-call-exceptions",
        "-fno-rtti",
        "-fno-strict-aliasing",
        "-fno-threadsafe-statics",
        "-fno-use-cxa-atexit",
    ]

    compile_flags = common_flags + [
        "-Wall",
        "-Wunused-but-set-parameter",
        "-Wno-free-nonheap-object",
        "-Wno-error=pragmas",
        "-Wno-unknown-pragmas",
        "-Wno-strict-aliasing",
    ]

    dbg_compile_flags = ["-g"]

    opt_compile_flags = [
        "-fdata-sections",
        "-ffunction-sections",
        "-g0",
        "-O2",
    ]

    cxx_flags = []

    link_flags = common_flags + [
        # Do not use the standard system startup files or libraries when linking.
        "-nostdlib",
        "-pass-exit-codes",
    ]

    opt_link_flags = [
        "-Wl,--gc-sections",
    ]

    unfiltered_compile_flags = [
        "-no-canonical-prefixes",
        "-fno-canonical-system-headers",
        "-Wno-builtin-macro-redefined",
        '-D__DATE__="redacted"',
        '-D__TIMESTAMP__="redacted"',
        '-D__TIME__="redacted"',
    ]

    cxx_builtin_include_directories = [
        "%sysroot%/riscv32-unknown-elf/include/c++/" + version,
        "%sysroot%/riscv32-unknown-elf/include/c++/" + version + "/riscv32-unknown-elf",
        "%sysroot%/riscv32-unknown-elf/include/c++/" + version + "/backward",
        "%sysroot%/lib/gcc/riscv32-unknown-elf/" + version + "/include",
        "%sysroot%/lib/gcc/riscv32-unknown-elf/" + version + "/include-fixed",
        "%sysroot%/riscv32-unknown-elf/include",
    ]

    # Source: https://cs.opensource.google/bazel/bazel/+/master:tools/cpp/unix_cc_toolchain_config.bzl
    cc_toolchain_config(
        name = name,
        cpu = "riscv32",
        compiler = "gcc",
        toolchain_identifier = "riscv32-unknown-elf-gcc",
        host_system_name = "local",
        target_system_name = "riscv32-unknown-elf",
        target_libc = "unknown",
        abi_version = "unknown",
        abi_libc_version = "unknown",
        cxx_builtin_include_directories = cxx_builtin_include_directories,
        tool_paths = tool_paths,
        compile_flags = compile_flags,
        dbg_compile_flags = dbg_compile_flags,
        opt_compile_flags = opt_compile_flags,
        cxx_flags = cxx_flags,
        link_flags = link_flags,
        # link_libs = link_libs,
        opt_link_flags = opt_link_flags,
        unfiltered_compile_flags = unfiltered_compile_flags,
        # coverage_compile_flags = coverage_compile_flags,
        # coverage_link_flags = coverage_link_flags,
        # supports_start_end_lib = supports_start_end_lib,
        builtin_sysroot = sysroot_path,
    )
